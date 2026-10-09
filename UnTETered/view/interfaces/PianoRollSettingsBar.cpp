//
// Created by Vos de Mens on 14/05/2026.
//

#include "PianoRollSettingsBar.h"

#include <utility>

#include "IntervalPresets.h"

PianoRollSettingsBar::PianoRollSettingsBar(
    std::function<void()> handleSnapY,
    std::function<void()> handleAbsoluteInfo,
    std::function<void()> handleVals,
    std::function<void()> handleCustomVals,
    std::function<void()> handleMonitoring,
    std::function<void()> handleInstructions
    ) : handleSnapYChange(std::move(handleSnapY)),
        handleAbsoluteInfoChange(std::move(handleAbsoluteInfo)),
        handleIntervalsChange(std::move(handleVals)),
        handleCustomIntervalsChange(std::move(handleCustomVals)),
        handleMonitoringChange(std::move(handleMonitoring)),
        handleInstructionsClick(std::move((handleInstructions))) {
    initialiseSnapY();
    initialiseAbsoluteInfo();
    initialiseIntervals();
    initialiseCustomIntervals();
    initialiseMonitoring();
    initialiseInstructions();
}

void PianoRollSettingsBar::initialiseSnapY() {
    addAndMakeVisible(snapYToggle);
    snapYToggle.setTooltip ("Fixes the pitch of notes, so they can only be changed relative to a selected reference note.");
    snapYToggle.onStateChange = handleSnapYChange;
    snapYToggle.setHelpText("hoi");
}

void PianoRollSettingsBar::initialiseAbsoluteInfo() {
    addAndMakeVisible(absoluteInfoToggle);
    absoluteInfoToggle.setTooltip ("Notes will show their pitch.");
    absoluteInfoToggle.onStateChange = handleAbsoluteInfoChange;
}

void PianoRollSettingsBar::initialiseIntervals() {
    addAndMakeVisible(intervalsComboBox);
    intervalsComboBox.setTooltip ("Choose which intervals should be available.");
    intervalsComboBox.onChange = handleIntervalsChange;

    intervalsComboBox.addItem("standard", SEVEN_LIMIT_ID);
    intervalsComboBox.addItem("custom", CUSTOM_INTERVALS_ID);

    intervalsComboBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromFloatRGBA(0, 0, 0, 0));
    intervalsComboBox.setColour(juce::ComboBox::outlineColourId, juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
    intervalsComboBox.setColour(juce::ComboBox::arrowColourId, juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f));
    intervalsComboBox.setColour(juce::ComboBox::textColourId, juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 1.f));
}

void PianoRollSettingsBar::initialiseCustomIntervals() {
    addAndMakeVisible(customIntervalsField);
    customIntervalsField.setTooltip ("Enter intervals of your choice, written as a/b, separated by commas.");
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
    monitoringToggle.setTooltip ("Notes will sound when selected.");
    monitoringToggle.setToggleState(false, juce::dontSendNotification);
    monitoringToggle.onClick = handleMonitoringChange;
}

void PianoRollSettingsBar::initialiseInstructions() {
    addAndMakeVisible(instructionsButton);
    instructionsButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour::fromFloatRGBA(0, 0, 0, 0.3f));
    instructionsButton.setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colour::fromFloatRGBA(1, 1, 1, 1));
    instructionsButton.setColour(juce::TextButton::ColourIds::textColourOnId, juce::Colour::fromFloatRGBA(1, 1, 1, 1));
    instructionsButton.setTooltip("Opens instructions window.");
    instructionsButton.onClick = handleInstructionsClick;
}

void PianoRollSettingsBar::setCustomIntervalsVisibility(bool visible) {
    customIntervalsField.setVisible(visible);
}

bool PianoRollSettingsBar::getSnapY() const {
    return snapYToggle.getToggleState();
}

bool PianoRollSettingsBar::getAbsoluteInfo() const {
    return absoluteInfoToggle.getToggleState();
}

int PianoRollSettingsBar::getIntervals() const {
    return intervalsComboBox.getSelectedId();
}

std::vector<Fraction> PianoRollSettingsBar::getCustomIntervals() const {
    return customIntervalsField.getFractions();
}

bool PianoRollSettingsBar::getMonitoring() const {
    return monitoringToggle.getToggleState();
}

void PianoRollSettingsBar::setSnapY(bool snapY, bool sendNotification) {
    if (sendNotification)
        snapYToggle.setToggleState(snapY, juce::sendNotification);
    else
        snapYToggle.setToggleState(snapY, juce::dontSendNotification);
}

void PianoRollSettingsBar::setMonitoring(const bool monitoring, bool sendNotification) {
    if (sendNotification)
        monitoringToggle.setToggleState(monitoring, juce::sendNotification);
    else
        monitoringToggle.setToggleState(monitoring, juce::dontSendNotification);
}

void PianoRollSettingsBar::setAbsoluteInfo(bool absoluteInfo, bool sendNotification) {
    if (sendNotification)
        absoluteInfoToggle.setToggleState(absoluteInfo, juce::sendNotification);
    else
        absoluteInfoToggle.setToggleState(absoluteInfo, juce::dontSendNotification);
}

void PianoRollSettingsBar::setIntervals(int id) {
    intervalsComboBox.setSelectedId(id, juce::dontSendNotification);
}

void PianoRollSettingsBar::setCustomIntervals(const std::vector<Fraction>& fractions) {
    customIntervalsField.setFractions(fractions);
}

void PianoRollSettingsBar::resized() {
    auto bounds = getLocalBounds().reduced(MARGIN);
    snapYToggle.setBounds(bounds.removeFromLeft(70));
    bounds.removeFromLeft(MARGIN);
    absoluteInfoToggle.setBounds(bounds.removeFromLeft(60));
    bounds.removeFromLeft(MARGIN);
    monitoringToggle.setBounds(bounds.removeFromLeft(90));
    bounds.removeFromRight(MARGIN);
    intervalsComboBox.setBounds(bounds.removeFromLeft(100));
    bounds.removeFromLeft(MARGIN);
    instructionsButton.setBounds(bounds.removeFromRight(20));
    bounds.removeFromRight(MARGIN);
    customIntervalsField.setBounds(bounds);
}