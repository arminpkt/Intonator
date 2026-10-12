#include "LogicBackend.h"

#include "../PluginProcessor.h"

LogicBackend::LogicBackend(const TransportState& tState) : DAWBackend(), transportState(tState) {
    midiOutput = juce::MidiOutput::createNewDevice("UnTETered Sync");
    if (midiOutput) {
        DBG("LogicBackend: created MIDI port ‘UnTETered Sync");
    } else {
        DBG("LogicBackend: could not create virtual MIDI port, nothing will be sent");
    }
}

void LogicBackend::requestPlayheadJump(double targetBeat) noexcept {
    sendMessage(beatsToFullFrameMessage(targetBeat).value());
}

void LogicBackend::requestPlay(double targetBeat) noexcept {
    auto fullFrame = beatsToFullFrameMessage(targetBeat);
    if (!fullFrame) {
        DBG("LogicBackend: Play skipped, no valid start position");
        return;
    }

    sendMessage(*fullFrame);
    sendMessage(juce::MidiMessage::midiMachineControlCommand(juce::MidiMessage::mmc_play));
    DBG("LogicBackend: Play");
}

void LogicBackend::requestStop() noexcept {
    sendMessage(juce::MidiMessage::midiMachineControlCommand(juce::MidiMessage::mmc_stop));
    DBG("LogicBackend: Stop");
}

const char* LogicBackend::getName() const noexcept  { return "Logic"; }

void LogicBackend::sendMessage(const juce::MidiMessage &message) noexcept {
    if (midiOutput) {
        midiOutput->sendMessageNow(message);
    } else {
        DBG("LogicBackend: no MIDI port, message dropped");
    }
}

std::optional<juce::MidiMessage> LogicBackend::beatsToFullFrameMessage(double targetBeat) noexcept {
    double currentBpm = transportState.bpm.load(std::memory_order_relaxed);
    double currentSampleRate = transportState.sampleRate.load(std::memory_order_relaxed);
    if (currentSampleRate <= 0.0 || currentBpm <= 0.0) {
        DBG("LogicBackend: cannot convert, sampleRate = " << currentSampleRate
            << ", bpm = " << currentBpm);
        return std::nullopt;
    }
    double currentTimeSeconds = transportState.timeInSamples.load(std::memory_order_relaxed)
                                    / currentSampleRate;
    double currentPositionBeats = transportState.ppqPosition.load(std::memory_order_relaxed);
    double targetTimeSeconds = currentTimeSeconds + (targetBeat - currentPositionBeats) * 60.0 / currentBpm;
    double totalSeconds = std::max(0.0, targetTimeSeconds + SMPTE_OFFSET_SECONDS);

    int wholeSeconds = static_cast<int>(totalSeconds);
    int targetHours = std::min(wholeSeconds / 3600, 23);
    int targetMinutes = wholeSeconds % 3600 / 60;
    int targetSeconds = wholeSeconds % 60;
    int targetFrames = std::min(static_cast<int>((totalSeconds - wholeSeconds) * FRAME_RATE),
                        static_cast<int>(FRAME_RATE) - 1);

    DBG("LogicBackend: target beat " << targetBeat
        << ", now at beat " << currentPositionBeats
        << " / " << currentTimeSeconds << " s, bpm " << currentBpm
        << " -> " << juce::String(targetHours).paddedLeft('0', 2)
        << ":"    << juce::String(targetMinutes).paddedLeft('0', 2)
        << ":"    << juce::String(targetSeconds).paddedLeft('0', 2)
        << ":"    << juce::String(targetFrames).paddedLeft('0', 2));

    return juce::MidiMessage::fullFrame(targetHours, targetMinutes, targetSeconds, targetFrames,
        FRAME_RATE_ENUM);
}
