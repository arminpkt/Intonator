#pragma once

#include "view/interfaces/PianoRoll.h"
#include "PluginProcessor.h"
#include "view/interfaces/Grid2D.h"
#include "view/interfaces/TextInput.h"

//==============================================================================
class UnTETeredAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                            private juce::ChangeListener
{
public:
    explicit UnTETeredAudioProcessorEditor (UnTETeredAudioProcessor&);
    ~UnTETeredAudioProcessorEditor() override;

    //==============================================================================
    void resized() override;

    bool keyPressed (const juce::KeyPress& key) override;

private:
    void changeListenerCallback(juce::ChangeBroadcaster *source) override;
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    UnTETeredAudioProcessor& processorRef;
    juce::VBlankAnimatorUpdater animatorUpdater;
    TextInput textInput{processorRef};
    Grid2D grid2D{{10, 10}, {5, 4}, {3, 2}, 220, processorRef, animatorUpdater};
    PianoRoll pianoRoll{processorRef};
    std::unique_ptr<juce::Component> activationOverlay;
    juce::TextButton licenseButton { "License" };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UnTETeredAudioProcessorEditor)
};
