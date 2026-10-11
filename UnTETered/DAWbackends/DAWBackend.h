#pragma once
#include <juce_core/juce_core.h>
/*
    DAWBackend

    Interface for controlling the host DAW's transport from the plugin.
    Each supported DAW (Ableton, Logic, ...) implements it in its own class;
    the processor holds exactly one backend, chosen at startup by the
    backend factory, and never needs to know which one it is.

    Rules for callers:
      - Positions are in beats (quarter notes from the start of the
        timeline) and never negative. The processor clamps before calling.
      - Only call from the message thread, never from the audio thread.

    Rules for implementations:
      - All functions are fire-and-forget: they must not block,
        and they must not throw. Errors are handled and logged internally.
      - Each backend owns its own connection (socket, MIDI port, ...),
        which is why backends cannot be copied.
*/

class DAWBackend {
    public:
        virtual ~DAWBackend() = default;
        virtual void requestPlayheadJump(double targetBeat) noexcept = 0;
        virtual void requestPlay(double targetBeat) noexcept = 0;
        virtual void requestStop() noexcept = 0;
        virtual const char* getName() const noexcept = 0;

    protected:
        DAWBackend() = default;
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DAWBackend)
};

class NullBackend : public DAWBackend {
    public:
        void requestPlayheadJump(double) noexcept override {}
        void requestPlay(double) noexcept override {}
        void requestStop() noexcept override {}
        const char* getName() const noexcept override { return "None";}
};