#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
UnTETeredAudioProcessorEditor::UnTETeredAudioProcessorEditor (UnTETeredAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), animatorUpdater(juce::VBlankAnimatorUpdater(this)) {
    // addAndMakeVisible(grid2D);
    addAndMakeVisible(pianoRoll);

    setSize (1200, 400);
    setResizable(true, true);
}

UnTETeredAudioProcessorEditor::~UnTETeredAudioProcessorEditor() = default;


void UnTETeredAudioProcessorEditor::resized() {
    auto area = getLocalBounds();
    pianoRoll.setBounds(area);
}
