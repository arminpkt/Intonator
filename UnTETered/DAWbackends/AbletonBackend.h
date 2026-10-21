#pragma once
#include "DAWBackend.h"
#include <juce_osc/juce_osc.h>

class AbletonBackend : public DAWBackend,
                       private juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>,
                       private juce::Timer {
    public:
        AbletonBackend();
        ~AbletonBackend() override;

        void requestPlayheadJump(double targetBeat) noexcept override;
        void requestPlay(double targetBeat) noexcept override;
        void requestStop() noexcept override;
        const char* getName() const noexcept override;
        bool canControlTransport() const noexcept override;

    private:
        void oscMessageReceived(const juce::OSCMessage& message) override;
        void timerCallback() override;
        void sendPing() noexcept;

        static constexpr int OSC_PORT = 11420;
        static constexpr int PING_INTERVAL_MS = 1000;
        static constexpr juce::uint32 SCRIPT_TIMEOUT_MS = 3000;

        juce::OSCSender sender;
        juce::DatagramSocket replySocket { false };   // declared before receiver: receiver must be destroyed first
        juce::OSCReceiver receiver;
        int replyPort = -1;

        std::atomic<bool>         pongReceived { false };
        std::atomic<juce::uint32> lastPongMs   { 0 };
};