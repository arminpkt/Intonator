#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
UnTETeredAudioProcessorEditor::UnTETeredAudioProcessorEditor (UnTETeredAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), animatorUpdater(juce::VBlankAnimatorUpdater(this)) {
    addAndMakeVisible(textInput);
    addAndMakeVisible(grid2D);

    setSize (400, 400 + TextInput::getHeight());
}

UnTETeredAudioProcessorEditor::~UnTETeredAudioProcessorEditor() = default;


void UnTETeredAudioProcessorEditor::resized() {
    auto area = getLocalBounds();
    textInput.setBounds(area.removeFromTop(TextInput::getHeight()));
    DBG(area.toString());
    grid2D.setBounds(area);
}
