#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
UnTETeredAudioProcessorEditor::UnTETeredAudioProcessorEditor (UnTETeredAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), animatorUpdater(juce::VBlankAnimatorUpdater(this)) {
    addAndMakeVisible(pianoRoll);

    setSize (1200, 400);
}

UnTETeredAudioProcessorEditor::~UnTETeredAudioProcessorEditor() = default;


void UnTETeredAudioProcessorEditor::resized() {
    auto area = getLocalBounds();
    pianoRoll.setBounds(area);
}
