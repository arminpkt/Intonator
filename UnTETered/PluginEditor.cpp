#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
UnTETeredAudioProcessorEditor::UnTETeredAudioProcessorEditor (UnTETeredAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), animatorUpdater(juce::VBlankAnimatorUpdater(this)) {
    // addAndMakeVisible(grid2D);
    addAndMakeVisible(pianoRoll);

    setSize (1200, 400);
    setResizable(true, true);

    processorRef.addChangeListener(this);
}

UnTETeredAudioProcessorEditor::~UnTETeredAudioProcessorEditor() {
    processorRef.removeChangeListener(this);
};

void UnTETeredAudioProcessorEditor::resized() {
    clipResize();
    auto area = getLocalBounds();
    pianoRoll.setBounds(area);
}

void UnTETeredAudioProcessorEditor::clipResize() {
    auto bounds = getBounds();
    if (bounds.getWidth() < 200)
        setBounds(bounds.withWidth(200));
    if (bounds.getHeight() < 200)
        setBounds(bounds.withHeight(200));
}

void UnTETeredAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster *source) {
    if (source != &processorRef)
        return;

    pianoRoll.pullStateFromProcessorAndRebuild();
    grid2D.pullStateFromProcessorAndRebuild();
}
