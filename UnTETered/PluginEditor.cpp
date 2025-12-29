#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
UnTETeredAudioProcessorEditor::UnTETeredAudioProcessorEditor (UnTETeredAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), animatorUpdater(juce::VBlankAnimatorUpdater(this))
{
    juce::ignoreUnused (processorRef);
    setSize (400, 400);
}

UnTETeredAudioProcessorEditor::~UnTETeredAudioProcessorEditor() = default;

//==============================================================================
void UnTETeredAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::darkgrey);
    addAndMakeVisible(grid2D);
}

void UnTETeredAudioProcessorEditor::resized()
{
    grid2D.setBounds(getLocalBounds());
}
