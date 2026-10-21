#pragma once
#include <juce_core/juce_core.h>

class DAWBackend {
    public:
        virtual ~DAWBackend() = default;
        virtual void requestPlayheadJump(double targetBeat) noexcept = 0;
        virtual void requestPlay(double targetBeat) noexcept = 0;
        virtual void requestStop() noexcept = 0;
        virtual const char* getName() const noexcept = 0;
        virtual bool canControlTransport() const noexcept = 0;

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
        bool canControlTransport() const noexcept override { return false; }
};