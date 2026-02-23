#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <unordered_set>
#include <array>
#include <vector>
#include <functional>
#include <set>

using Point = juce::Point<int>;

struct PointHash {
    std::size_t operator()(const Point& p) const {
        return std::hash<int>{}(p.x) ^ (std::hash<int>{}(p.y) << 1);
    }
};
using PointSet = std::unordered_set<Point, PointHash>;

struct KeyBinding
{
    int keyCode = 0;
    int mods = 0; // bitmask (shift/ctrl/alt/cmd)
    std::vector<juce::Point<int>> cells; // chord payload as grid cells
};

struct SaveSlotState
{
    char mode = 0;                 // 0 = empty, 's' or 'a'
    std::vector<Point> screenCells; // your saves store SCREEN cells
};

struct GridState
{
    double originFreqHz = 220.0;
    int offsetX = 0;
    int offsetY = 0;
    PointSet activeCells;
    PointSet selectedCells;

    std::array<SaveSlotState, 91> saves;
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
    GridState getGridState() const;
    void setGridState(const GridState& s);
    void updateGridState(std::function<void(GridState&)> fn);

    //==============================================================================
    juce::MidiBuffer midiBuffer;

private:
    // Keep track of currently active notes per channel
    std::set<std::pair<int, int>> activeNotes; // pair<channel, note>
    double currentSamplePosition = 0.0;           // Tracks playback position
    double lastHostPosition = 0.0; // in samples
    int currentEventIndex = 0; // Tracks which MIDI event comes next

    mutable juce::CriticalSection gridStateLock;
    GridState gridState;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UnTETeredAudioProcessor)
};
