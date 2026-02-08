#pragma once

#include "PianoRoll.h"
#include "PluginProcessor.h"
#include "view/interfaces/Grid2D.h"
#include "view/interfaces/TextInput.h"

//==============================================================================
class UnTETeredAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit UnTETeredAudioProcessorEditor (UnTETeredAudioProcessor&);
    ~UnTETeredAudioProcessorEditor() override;

    //==============================================================================
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    UnTETeredAudioProcessor& processorRef;
    juce::VBlankAnimatorUpdater animatorUpdater;
    TextInput textInput{processorRef};
    Grid2D grid2D{{10, 10}, {5, 4}, {3, 2}, 220, processorRef, animatorUpdater};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UnTETeredAudioProcessorEditor)
};
