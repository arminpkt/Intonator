#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

struct TransportState
{
    std::atomic<bool>   isPlaying { false };
    std::atomic<bool>   isRecording { false };
    std::atomic<double> bpm { 120.0 };
    std::atomic<double> ppqPosition { 0.0 };
    std::atomic<int64_t> timeInSamples { 0 };
    std::atomic<double> sampleRate { 44100.0 };
    std::atomic<int>    numerator { 4 };
    std::atomic<int>    denominator { 4 };
};

struct HostSeekRequest
{
    std::atomic<bool> pending { false };
    std::atomic<double> targetBar { 0.0 };
};

//==============================================================================
class UnTETeredAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    UnTETeredAudioProcessor();
    ~UnTETeredAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    const TransportState& getTransportState();
    void requestHostSeekToBar(double targetBar) noexcept;

    //==============================================================================
    juce::MidiBuffer midiBuffer;

private:
    void setTransportStateFromHost(juce::AudioPlayHead*);
    void playMidi(juce::AudioPlayHead* playHead, juce::MidiBuffer& midiMessages);
    // Keep track of currently active notes per channel
    std::set<std::pair<int, int>> activeNotes; // pair<channel, note>
    double currentSamplePosition = 0.0;           // Tracks playback position
    double lastHostPosition = 0.0; // in samples
    int currentEventIndex = 0; // Tracks which MIDI event comes next
    TransportState transportState;
    HostSeekRequest hostSeekRequest;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UnTETeredAudioProcessor)
};
