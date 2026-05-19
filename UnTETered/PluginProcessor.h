#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <unordered_set>
#include <array>
#include <vector>
#include <functional>
#include <set>
#include "view/interfaces/GridState.h"
#include "view/interfaces/GridStateSerialiser.h"
#include "view/interfaces/PianoRollState.h"
#include "view/interfaces/PianoRollStateSerialiser.h"
#include "view/interfaces/PianoRollStateHelpers.h"

struct PlaybackSequence
{
    std::vector<juce::MidiMessage> messages; // timestamps in bars, not samples
    double pitchBendRange = 2.0;
    bool valid = false;
};

struct TransportState
{
    std::atomic<bool>    isPlaying    { false };
    std::atomic<bool>    isRecording  { false };
    std::atomic<double>  bpm          { 120.0 };
    std::atomic<double>  ppqPosition  { 0.0 };
    std::atomic<int64_t> timeInSamples{ 0 };
    std::atomic<double>  sampleRate   { 44100.0 };
    std::atomic<int>     numerator    { 4 };
    std::atomic<int>     denominator  { 4 };
};

struct HostSeekRequest
{
    std::atomic<bool>   pending   { false };
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
    GridState getGridState() const;
    void setGridState(const GridState& s);
    void updateGridState(std::function<void(GridState&)> fn);

    PianoRollState getPianoRollState() const;

    void setPianoRollState(const PianoRollState& s);

    void setPianoRollViewState(float octaveHeightPxF,
                               float barWidthPxF,
                               double freqBottomScreen,
                               float barLeftScreen);

    void updatePianoRollState(std::function<void(PianoRollState&)> fn);

    //==============================================================================
    juce::MidiBuffer midiBuffer;

private:
    void setTransportStateFromHost(juce::AudioPlayHead*);
    void rebuildPlaybackSequence();
    void flushActiveNotes(juce::MidiBuffer& midiMessages);
    void playMidi(juce::MidiBuffer& midiMessages, int numSamples);

    // Keep track of currently active notes per channel
    std::set<std::pair<int, int>> activeNotes; // pair<channel, note>

    // The bar position we expected to be at the start of this block,
    // based on where last block ended. Used to detect loops and seeks.
    double lastExpectedBar = -1.0;

    mutable juce::CriticalSection gridStateLock;
    GridState gridState;

    mutable juce::CriticalSection pianoRollStateLock;
    PianoRollState pianoRollState;
    TransportState transportState;
    HostSeekRequest hostSeekRequest;

    mutable juce::CriticalSection playbackLock;
    PlaybackSequence playbackSequence;
    std::atomic<bool> playbackDirty { true };
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UnTETeredAudioProcessor)
};