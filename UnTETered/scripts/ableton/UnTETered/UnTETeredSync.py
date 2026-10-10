"""UnTETered Sync - minimal Ableton Live Remote Script.

Receives OSC from the UnTETered plugin on localhost and controls Live's
transport. Handles the "play from position X" case. Live ignores positions
set while stopped (it starts from its insert marker or its last stop
position instead), so PLAY_MODE selects which strategy is used to make
playback start at X.

Messages (all positions in beats = quarter notes from the arrangement start):
    /untetered/ping      i:replyPort   -> replies /untetered/pong i:protocolVersion
    /untetered/play      f:beat        start playback from beat (or jump if already playing)
    /untetered/stop                    stop playback
    /untetered/song_time f:beat        move the playhead (exact jump while playing)
"""

import socket
import time

from _Framework.ControlSurface import ControlSurface

from .osc import parse_message, build_message

PROTOCOL_VERSION = 1
LISTEN_HOST = "127.0.0.1"
LISTEN_PORT = 11420           # different from AbletonOSC (11000/11001), so both can coexist
MAX_PACKETS_PER_TICK = 64     # safety cap so a flood of packets can't stall Live's UI tick

DEBUG = True                  # set to False to silence the debug output

# How playback is started from a position while Live is stopped:
#   "start_marker" - experiment 1: set Live's start marker (song.start_time),
#                    then start_playing. No jump needed if Live accepts it.
#   "same_call"    - experiment 2: continue_playing, then set the position
#                    immediately in the same handler call.
#   "listener"     - previous behaviour: continue_playing, jump once the
#                    is_playing listener reports that playback has started.
# Every mode keeps the listener jump as a safety net in case it didn't land.
PLAY_MODE = "start_marker"

# If Live is already within this many beats past the target when playback
# starts, the safety-net jump is skipped (Live is where it should be, and
# jumping back a few milliseconds would itself be audible).
ARRIVED_TOLERANCE_BEATS = 0.5


class UnTETeredSync(ControlSurface):

    def __init__(self, c_instance):
        super(UnTETeredSync, self).__init__(c_instance)

        self._t0 = time.monotonic()  # reference point for the debug timestamps

        # Target position (beats) to jump to once playback has actually started.
        self._pending_jump = None
        self._logged_listener_deferral = False

        self._handlers = {
            "/untetered/ping": self._handle_ping,
            "/untetered/play": self._handle_play,
            "/untetered/stop": self._handle_stop,
            "/untetered/song_time": self._handle_song_time,
        }

        self._socket = None
        self._open_socket()

        self.song().add_is_playing_listener(self._on_is_playing_changed)

        if self._socket is not None:
            self.show_message("UnTETered Sync: listening on port %d" % LISTEN_PORT)
            self._dbg("started, listening on port %d, PLAY_MODE=%s" % (LISTEN_PORT, PLAY_MODE))

    # ------------------------------------------------------------------ debug helpers

    def _dbg(self, text):
        if DEBUG:
            elapsed_ms = (time.monotonic() - self._t0) * 1000.0
            self.log_message("UnTETered Sync [%9.1f ms] %s" % (elapsed_ms, text))

    def _state(self):
        song = self.song()
        start_time = getattr(song, "start_time", None)
        start_text = "n/a" if start_time is None else "%.4f" % start_time
        return "playing=%s song_time=%.4f start_time=%s" % (
            song.is_playing, song.current_song_time, start_text)

    # ------------------------------------------------------------------ lifecycle

    def _open_socket(self):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock.setblocking(False)
            sock.bind((LISTEN_HOST, LISTEN_PORT))
            self._socket = sock
        except OSError as error:
            self._socket = None
            self.log_message("UnTETered Sync: could not open port %d: %s" % (LISTEN_PORT, error))
            self.show_message("UnTETered Sync: port %d is already in use" % LISTEN_PORT)

    def disconnect(self):
        self._dbg("disconnect")
        song = self.song()
        if song.is_playing_has_listener(self._on_is_playing_changed):
            song.remove_is_playing_listener(self._on_is_playing_changed)
        if self._socket is not None:
            self._socket.close()
            self._socket = None
        super(UnTETeredSync, self).disconnect()

    def update_display(self):
        """Called by Live on its UI tick (roughly every 100 ms)."""
        super(UnTETeredSync, self).update_display()
        self._poll_socket()

        # Fallback path: if the listener could not apply the jump directly,
        # apply it here on the next tick.
        if self._pending_jump is not None and self.song().is_playing:
            self._dbg("tick fallback: pending jump to %.4f | %s"
                      % (self._pending_jump, self._state()))
            self._apply_pending_jump()

    # ------------------------------------------------------------------ networking

    def _poll_socket(self):
        if self._socket is None:
            return
        for _ in range(MAX_PACKETS_PER_TICK):
            try:
                data, sender = self._socket.recvfrom(1024)
            except (BlockingIOError, InterruptedError):
                return  # nothing more to read this tick
            except OSError as error:
                self.log_message("UnTETered Sync: socket error: %s" % error)
                return

            message = parse_message(data)
            if message is None:
                self._dbg("RX unparseable packet from %s, %d bytes: %r"
                          % (sender, len(data), data[:48]))
                continue
            address, args = message
            self._dbg("RX %s %r | %s" % (address, args, self._state()))

            handler = self._handlers.get(address)
            if handler is None:
                self._dbg("   no handler for %s, ignored" % address)
                continue
            try:
                handler(args)
            except Exception as error:  # never let one bad message kill the script
                self.log_message("UnTETered Sync: error handling %s: %s" % (address, error))

    def _send(self, port, address, *args):
        if self._socket is None:
            return
        try:
            self._socket.sendto(build_message(address, *args), (LISTEN_HOST, port))
            self._dbg("TX %s %r -> port %d" % (address, args, port))
        except OSError as error:
            self.log_message("UnTETered Sync: could not send %s: %s" % (address, error))

    # ------------------------------------------------------------------ handlers

    @staticmethod
    def _beat_from_args(args):
        if not args:
            return None
        return max(0.0, float(args[0]))

    def _handle_ping(self, args):
        if args:
            self._send(int(args[0]), "/untetered/pong", PROTOCOL_VERSION)

    def _handle_play(self, args):
        song = self.song()
        beat = self._beat_from_args(args)

        if beat is None:
            self._dbg("   play: no position -> continue_playing")
            song.continue_playing()
            return

        if song.is_playing:
            # Already running: a plain jump lands exactly.
            self._dbg("   play: already playing -> jump to %.4f" % beat)
            song.current_song_time = beat
            self._dbg("   play: after jump | %s" % self._state())
            return

        # Stopped. The listener jump stays armed as a safety net in every mode;
        # it is skipped automatically if Live already started at the target.
        self._pending_jump = beat
        song.current_song_time = beat  # keeps Live's display in step while stopped

        if PLAY_MODE == "start_marker" and self._try_set_start_marker(beat):
            self._dbg("   play: start marker set -> start_playing")
            song.start_playing()
            self._dbg("   play: after start_playing | %s" % self._state())
            return

        self._dbg("   play: mode %s -> continue_playing" % PLAY_MODE)
        song.continue_playing()
        self._dbg("   play: after continue_playing | %s" % self._state())

        if PLAY_MODE == "same_call":
            song.current_song_time = beat
            self._dbg("   play: same-call jump to %.4f | %s" % (beat, self._state()))

    def _try_set_start_marker(self, beat):
        """Experiment 1: try to move Live's start marker. Returns True if Live accepted it."""
        song = self.song()
        if not hasattr(song, "start_time"):
            self._dbg("   start marker: song has no start_time property")
            return False
        try:
            song.start_time = beat
        except Exception as error:
            self._dbg("   start marker: Live refused (%s)" % error)
            return False
        accepted = abs(song.start_time - beat) < 1e-6
        self._dbg("   start marker: set to %.4f, Live reports %.4f -> %s"
                  % (beat, song.start_time, "accepted" if accepted else "ignored"))
        return accepted

    def _handle_stop(self, args):
        self._dbg("   stop: dropping pending jump %r, stop_playing" % self._pending_jump)
        self._pending_jump = None
        self.song().stop_playing()

    def _handle_song_time(self, args):
        beat = self._beat_from_args(args)
        if beat is not None:
            self._dbg("   song_time: set to %.4f" % beat)
            self.song().current_song_time = beat
            self._dbg("   song_time: after set | %s" % self._state())

    # ------------------------------------------------------------------ jump on playback start

    def _on_is_playing_changed(self):
        self._dbg("listener: is_playing changed | %s | pending=%r"
                  % (self._state(), self._pending_jump))
        if not self.song().is_playing:
            self._pending_jump = None  # stopped before the jump happened: drop it
            return
        if self._pending_jump is None:
            return
        # Fastest path: jump right inside the notification. Live may refuse
        # changes triggered from notifications; then update_display applies
        # the jump on the next tick instead.
        try:
            self._apply_pending_jump()
        except Exception as error:
            self._dbg("listener: jump refused, deferred to next tick (%s)" % error)
            if not self._logged_listener_deferral:
                self.log_message("UnTETered Sync: jump deferred to next tick (%s)" % error)
                self._logged_listener_deferral = True

    def _apply_pending_jump(self):
        target = self._pending_jump
        if target is None:
            return
        current = self.song().current_song_time
        if 0.0 <= current - target < ARRIVED_TOLERANCE_BEATS:
            # Live already started at the target: no jump, nothing to hear.
            self._pending_jump = None
            self._dbg("jump not needed: Live is at %.4f, target %.4f" % (current, target))
            return
        self.song().current_song_time = target
        self._pending_jump = None  # only cleared once the jump actually succeeded
        self._dbg("jump applied: %.4f -> %.4f | %s" % (current, target, self._state()))