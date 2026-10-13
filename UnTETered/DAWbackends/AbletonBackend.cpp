#include "AbletonBackend.h"

AbletonBackend::AbletonBackend() {
    if (!sender.connect("127.0.0.1", OSC_PORT)) {
        DBG("OSC Connection Failed");
    }
}

void AbletonBackend::requestPlayheadJump(double targetBeat) noexcept {
    if (!sender.send("/untetered/song_time", (float) targetBeat)) {
        DBG("Could not send OSC message");
    }
}

void AbletonBackend::requestPlay(double targetBeat) noexcept {
    if (!sender.send("/untetered/play", (float)targetBeat)) {
        DBG("Could not send OSC message");
    }
}

void AbletonBackend::requestStop() noexcept {
    if (!sender.send ("/untetered/stop")) {
        DBG("Could not send OSC message");
    }
}

const char* AbletonBackend::getName() const noexcept  { return "Ableton";}