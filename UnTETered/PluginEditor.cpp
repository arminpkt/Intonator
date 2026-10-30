#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
UnTETeredAudioProcessorEditor::UnTETeredAudioProcessorEditor (UnTETeredAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), animatorUpdater(juce::VBlankAnimatorUpdater(this)) {
    // addAndMakeVisible(grid2D);
    addAndMakeVisible(pianoRoll);

    activationOverlay = processorRef.license.createActivationUI();

    if (activationOverlay != nullptr)
    {
        addChildComponent (*activationOverlay);
    }

    setResizeLimits(200, 200, 20000, 20000);
    setResizable(true, true);
    setSize (1200, 400);

    processorRef.addChangeListener(this);
}

UnTETeredAudioProcessorEditor::~UnTETeredAudioProcessorEditor() {
    processorRef.removeChangeListener(this);
}

void UnTETeredAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();
    if (activationOverlay != nullptr)
        activationOverlay->setBounds (bounds);

    pianoRoll.setBounds(bounds);
}

bool UnTETeredAudioProcessorEditor::keyPressed(const juce::KeyPress &key) {
    const juce::KeyPress licenseShortcut('l', juce::ModifierKeys::commandModifier, 0);
    if (key == licenseShortcut && activationOverlay != nullptr) {
        if (activationOverlay->isVisible()) {
            if (processorRef.license.isLicensed())
                activationOverlay->setVisible(false);
        } else {
            activationOverlay->setVisible(true);
            activationOverlay->toFront(false);
        }
        return true;
    }
    return false;
}

void UnTETeredAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster *source) {
    if (source != &processorRef)
        return;

    pianoRoll.pullStateFromProcessorAndRebuild();
    grid2D.pullStateFromProcessorAndRebuild();
}
