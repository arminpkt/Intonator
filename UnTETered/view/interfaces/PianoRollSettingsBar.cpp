//
// Created by Vos de Mens on 14/05/2026.
//

#include "PianoRollSettingsBar.h"

#include <utility>

#include "IntervalPresets.h"

PianoRollSettingsBar::PianoRollSettingsBar(
    std::function<void()> handleLockY,
    std::function<void()> handleLockRef,
    std::function<void()> handleVals,
    std::function<void()> handleCustomVals,
    std::function<void()> handleMonitoring
    ) : handleLockYChange(std::move(handleLockY)),
        handleLockRefChange(std::move(handleLockRef)),
        handleIntervalsChange(std::move(handleVals)),
        handleCustomIntervalsChange(std::move(handleCustomVals)),
        handleMonitoringChange(std::move(handleMonitoring)) {
    initialiseLockY();
    initialiseLockRef();
    initialiseIntervals();
    initialiseCustomIntervals();
    initialiseMonitoring();
}

void PianoRollSettingsBar::initialiseLockY() {
    addAndMakeVisible(lockYToggle);
    lockYToggle.onStateChange = handleLockYChange;
    lockYToggle.setHelpText("hoi");
}

void PianoRollSettingsBar::initialiseLockRef() {
    addAndMakeVisible(lockRefToggle);
    lockRefToggle.onStateChange = handleLockRefChange;
}

void PianoRollSettingsBar::initialiseIntervals() {
    addAndMakeVisible(intervalsComboBox);
    intervalsComboBox.onChange = handleIntervalsChange;

    intervalsComboBox.addItem("7-limit", SEVEN_LIMIT_ID);
    intervalsComboBox.addItem("custom", CUSTOM_INTERVALS_ID);

    intervalsComboBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromFloatRGBA(0, 0, 0, 0));
    intervalsComboBox.setColour(juce::ComboBox::outlineColourId, juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
    intervalsComboBox.setColour(juce::ComboBox::arrowColourId, juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
    intervalsComboBox.setColour(juce::ComboBox::textColourId, juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 1.f));
}

void PianoRollSettingsBar::initialiseCustomIntervals() {
    addAndMakeVisible(customIntervalsField);
    customIntervalsField.setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromFloatRGBA(0, 0, 0, 0));
    customIntervalsField.setColour(juce::TextEditor::textColourId, juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 1.f));
    customIntervalsField.setColour(juce::TextEditor::outlineColourId, juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
    customIntervalsField.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
    customIntervalsField.setColour(juce::TextEditor::highlightColourId, juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
    customIntervalsField.setColour(juce::TextEditor::highlightedTextColourId, juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
    customIntervalsField.setColour(juce::TextEditor::shadowColourId, juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
    customIntervalsField.onTextChange = handleCustomIntervalsChange;
}

void PianoRollSettingsBar::initialiseMonitoring() {
    addAndMakeVisible(monitoringToggle);
    monitoringToggle.setToggleState(false, juce::dontSendNotification);
    monitoringToggle.onClick = handleMonitoringChange;
}

void PianoRollSettingsBar::setCustomIntervalsVisibility(bool visible) {
    customIntervalsField.setVisible(visible);
}

bool PianoRollSettingsBar::getLockY() const {
    return lockYToggle.getToggleState();
}

bool PianoRollSettingsBar::getLockRef() const {
    return lockRefToggle.getToggleState();
}

int PianoRollSettingsBar::getIntervals() const {
    return intervalsComboBox.getSelectedId();
}

std::vector<Fraction> PianoRollSettingsBar::getCustomIntervals() const {
    return customIntervalsField.getFractions();
}

bool PianoRollSettingsBar::isMonitoringEnabled() const {
    return monitoringToggle.getToggleState();
}

void PianoRollSettingsBar::setLockY(bool lockY, bool sendNotification) {
    if (sendNotification)
        lockYToggle.setToggleState(lockY, juce::sendNotification);
    else
        lockYToggle.setToggleState(lockY, juce::dontSendNotification);
}

void PianoRollSettingsBar::setLockRef(bool lockRef, bool sendNotification) {
    if (sendNotification)
        lockRefToggle.setToggleState(lockRef, juce::sendNotification);
    else
        lockRefToggle.setToggleState(lockRef, juce::dontSendNotification);
}

void PianoRollSettingsBar::setIntervals(int id) {
    intervalsComboBox.setSelectedId(id);
}

void PianoRollSettingsBar::setCustomIntervals(const std::vector<Fraction>& fractions) {
    customIntervalsField.setFractions(fractions);
}

void PianoRollSettingsBar::setMonitoringEnabled(const bool enabled)
{
    monitoringToggle.setToggleState(enabled, juce::dontSendNotification);
}

void PianoRollSettingsBar::resized() {
    auto bounds = getLocalBounds().reduced(MARGIN);
    lockYToggle.setBounds(bounds.removeFromLeft(70));
    bounds.removeFromLeft(MARGIN);
    lockRefToggle.setBounds(bounds.removeFromLeft(80));
    bounds.removeFromLeft(MARGIN);
    monitoringToggle.setBounds(bounds.removeFromLeft(90));
    bounds.removeFromRight(MARGIN);
    intervalsComboBox.setBounds(bounds.removeFromLeft(100));
    bounds.removeFromLeft(MARGIN);
    customIntervalsField.setBounds(bounds);
}