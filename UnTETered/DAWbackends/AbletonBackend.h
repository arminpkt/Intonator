#pragma once
#include "DAWBackend.h"
#include <juce_osc/juce_osc.h>

class AbletonBackend : public DAWBackend {
    public:
        AbletonBackend();
        void requestPlayheadJump(double targetBeat) noexcept override;
        void requestPlay(double targetBeat) noexcept override;
        void requestStop() noexcept override;
        const char* getName() const noexcept override;
    private:
        static constexpr int OSC_PORT = 11420;
        juce::OSCSender sender;
};