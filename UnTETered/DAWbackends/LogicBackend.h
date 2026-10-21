#pragma once
#include "DAWBackend.h"
#include <juce_audio_devices/juce_audio_devices.h>
#include "BackendLog.h"

struct TransportState;

class LogicBackend : public DAWBackend {
    public:
        LogicBackend(const TransportState& tState);

        void requestPlayheadJump(double targetBeat) noexcept override;
        void requestPlay(double targetBeat) noexcept override;
        void requestStop() noexcept override;
        const char* getName() const noexcept override;
        bool canControlTransport() const noexcept override;
    private:
        void sendMessage(const juce::MidiMessage& message) noexcept;
        std::optional<juce::MidiMessage> beatsToGotoMessage(double targetBeat) noexcept;
        static constexpr double SMPTE_OFFSET_SECONDS = 3600.0; // 1 hour
        static constexpr double FRAME_RATE = 25.0;
        static constexpr juce::MidiMessage::SmpteTimecodeType FRAME_RATE_ENUM = juce::MidiMessage::fps25;
        std::unique_ptr<juce::MidiOutput> midiOutput;
        const TransportState& transportState;
};
