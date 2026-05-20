//
// Created by Vos de Mens on 14/05/2026.
//

#include "PianoRollSettingsBar.h"

#include <utility>

PianoRollSettingsBar::PianoRollSettingsBar(
    std::function<void()> handleLock,
    std::function<void()> handleRef,
    std::function<void()> handleFrac,
    std::function<void()> handleMonitoring
    ) : handleLockYChange(std::move(handleLock)),
        handleReferenceChange(std::move(handleRef)),
        handlePotentialRatiosChange(std::move(handleFrac)),
        handleMonitoringChange(std::move(handleMonitoring)) {
    initialiseLockY();
    initialiseReference();
    initialisePotentialRatios();
    initialiseMonitoring();
}

void PianoRollSettingsBar::initialiseLockY() {
    addAndMakeVisible(lockYComboBox);
    lockYComboBox.setColour(juce::ComboBox::backgroundColourId, COMBOBOX_BACKGROUND_COLOUR);
    lockYComboBox.addItem("lock", locked+1);
    lockYComboBox.addItem("snap", snap+1);
    lockYComboBox.addItem("free", continuous+1);
    lockYComboBox.onChange = handleLockYChange;
}

void PianoRollSettingsBar::initialiseReference() {
    addAndMakeVisible(referenceComboBox);
    referenceComboBox.setColour(juce::ComboBox::backgroundColourId, COMBOBOX_BACKGROUND_COLOUR);
    referenceComboBox.addItem("select", selectedNote+1);
    referenceComboBox.addItem("fixed", lockNote+1);
    referenceComboBox.onChange = handleReferenceChange;
}

void PianoRollSettingsBar::initialisePotentialRatios() {
    addAndMakeVisible(potentialRatiosField);
    potentialRatiosField.setColour(juce::TextEditor::backgroundColourId, COMBOBOX_BACKGROUND_COLOUR);
    potentialRatiosField.onTextChange = handlePotentialRatiosChange;
}

void PianoRollSettingsBar::initialiseMonitoring() {
    addAndMakeVisible(monitoringToggle);
    monitoringToggle.setToggleState(false, juce::dontSendNotification);
    monitoringToggle.onClick = handleMonitoringChange;
}

LockY PianoRollSettingsBar::getLockY() const {
    return static_cast<LockY>(lockYComboBox.getSelectedId() - 1);
}

Reference PianoRollSettingsBar::getReference() const {
    return static_cast<Reference>(referenceComboBox.getSelectedId() - 1);
}

std::vector<Fraction> PianoRollSettingsBar::getPotentialRatios() const {
    return potentialRatiosField.getFractions();
}

bool PianoRollSettingsBar::isMonitoringEnabled() const {
    return monitoringToggle.getToggleState();
}

void PianoRollSettingsBar::setLockY(const LockY lockY) {
    lockYComboBox.setSelectedId(lockY+1);
}

void PianoRollSettingsBar::setReference(const Reference reference) {
    referenceComboBox.setSelectedId(reference+1);
}

void PianoRollSettingsBar::setPotentialRatios(const std::vector<Fraction>& fractions) {
    potentialRatiosField.setFractions(fractions);
}

void PianoRollSettingsBar::setMonitoringEnabled(const bool enabled)
{
    monitoringToggle.setToggleState(enabled, juce::dontSendNotification);
}

void PianoRollSettingsBar::resized() {
    auto bounds = getLocalBounds().reduced(MARGIN);
    lockYComboBox.setBounds(bounds.removeFromLeft(70));
    bounds.removeFromLeft(MARGIN);
    referenceComboBox.setBounds(bounds.removeFromLeft(80));
    bounds.removeFromLeft(MARGIN);
    // Monitoring toggle sits at the right end.
    monitoringToggle.setBounds(bounds.removeFromRight(80));
    bounds.removeFromRight(MARGIN);
    potentialRatiosField.setBounds(bounds);
}