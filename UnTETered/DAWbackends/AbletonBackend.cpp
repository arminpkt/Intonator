#include "AbletonBackend.h"

AbletonBackend::AbletonBackend() {
    if (!sender.connect("127.0.0.1", OSC_PORT)) {
        DBG("OSC Connection Failed");
    }
    // Port 0 = let the OS pick a free port, so several plugin instances don't collide
    if (replySocket.bindToPort(0, "127.0.0.1")) {
        replyPort = replySocket.getBoundPort();
        if (receiver.connectToSocket(replySocket))
            receiver.addListener(this);
        else
            DBG("OSC: could not attach receiver to reply socket");
    } else {
        DBG("OSC: could not bind reply socket");
    }

    sendPing();
    startTimer(PING_INTERVAL_MS);
}

AbletonBackend::~AbletonBackend() {
    stopTimer();
    receiver.removeListener(this);
    receiver.disconnect();
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

bool AbletonBackend::canControlTransport() const noexcept {
    if (!pongReceived.load(std::memory_order_relaxed))
        return false;
    // unsigned subtraction also survives the 32-bit counter wrapping around
    return juce::Time::getMillisecondCounter() - lastPongMs.load(std::memory_order_relaxed) < SCRIPT_TIMEOUT_MS;
}

void AbletonBackend::oscMessageReceived(const juce::OSCMessage& message) {
    if (message.getAddressPattern().toString() != "/untetered/pong")
        return;
    lastPongMs.store(juce::Time::getMillisecondCounter(), std::memory_order_relaxed);
    pongReceived.store(true, std::memory_order_relaxed);
}

void AbletonBackend::timerCallback() { sendPing(); }

void AbletonBackend::sendPing() noexcept {
    if (replyPort > 0)
        sender.send("/untetered/ping", static_cast<juce::int32>(replyPort));
}