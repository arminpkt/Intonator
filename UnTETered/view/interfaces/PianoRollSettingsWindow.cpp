//
// Created by Vos de Mens on 20/10/2026.
//

#include "PianoRollSettingsWindow.h"


PianoRollSettingsWindow::PianoRollSettingsWindow(
        std::function<void()> handleColourScheme,
        std::function<void()> handleGradientColour,
        std::function<void()> handlePitchbendRange,
        std::function<void()> handleConcertPitch,
        juce::ChangeListener* ref
        )
        : DialogWindow("Settings", {50, 50, 50}, true) {
    handleColourSchemeChanged = std::move(handleColourScheme);
    handleGradientColourSchemeChanged = std::move(handleGradientColour);
    handlePitchbendRangeChanged = std::move(handlePitchbendRange);
    handleConcertPitchChanged = std::move(handleConcertPitch);

    initialiseLookAndFeel();
    setLookAndFeel(&lookAndFeel);

    setContentOwned(&content, true);
    initialiseComponents(ref);

    centreWithSize(2*(MARGIN + COLUMN_WIDTH), 670);
    enterModalState();
    setResizable(false, false);

    placeComponents();
}

void PianoRollSettingsWindow::closeButtonPressed() {
    close();
}

void PianoRollSettingsWindow::open() {
    centreWithSize(getWidth(), getHeight());
    enterModalState();
    setVisible(true);
    setAlwaysOnTop(true);
}

void PianoRollSettingsWindow::close() {
    setVisible(false);
}

void PianoRollSettingsWindow::initialiseLookAndFeel() {
    juce::Colour lineColour = {200, 200, 200};
    juce::Colour transparent = juce::Colour::fromRGBA(0, 0, 0, 0);

    lookAndFeel.setColour(juce::Label::textColourId, lineColour);

    lookAndFeel.setColour(juce::ComboBox::outlineColourId, lineColour);
    lookAndFeel.setColour(juce::ComboBox::textColourId, lineColour);
    lookAndFeel.setColour(juce::ComboBox::backgroundColourId, transparent);

    // lookAndFeel.setColour(juce::Slider::textBoxOutlineColourId, lineColour);
    // lookAndFeel.setColour(juce::Slider::rotarySliderOutlineColourId, lineColour);
    // lookAndFeel.setColour(juce::Slider::textBoxTextColourId, lineColour);
    // lookAndFeel.setColour(juce::Slider::backgroundColourId, transparent);
    // lookAndFeel.setColour(juce::Slider::textBoxBackgroundColourId, transparent);
    // lookAndFeel.setColour(juce::Slider::rotarySliderFillColourId, transparent);

    lookAndFeel.setColour(juce::TextEditor::outlineColourId, lineColour);
    lookAndFeel.setColour(juce::TextEditor::textColourId, lineColour);
    lookAndFeel.setColour(juce::TextEditor::backgroundColourId, transparent);
}

void PianoRollSettingsWindow::initialiseComponents(juce::ChangeListener* ref) {
    content.addAndMakeVisible(colourSchemeLabel);
    content.addAndMakeVisible(colourSchemeComboBox);
    content.addAndMakeVisible(gradientColourLabel1);
    content.addAndMakeVisible(gradientColourLabel2);
    content.addAndMakeVisible(gradientColourSelector1);
    content.addAndMakeVisible(gradientColourSelector2);
    content.addAndMakeVisible(pitchbendRangeLabel);
    content.addAndMakeVisible(pitchbendRangeField);
    content.addAndMakeVisible(concertPitchLabel);
    content.addAndMakeVisible(concertPitchField);

    colourSchemeLabel.setText("Colour scheme", juce::dontSendNotification);
    colourSchemeComboBox.addItem("Rainbow", RAINBOW_ID);
    colourSchemeComboBox.addItem("Gradient", GRADIENT_ID);
    colourSchemeComboBox.onChange = handleColourSchemeChanged;

    gradientColourLabel1.setText("Colour for A", juce::dontSendNotification);
    gradientColourLabel2.setText("Colour for D#", juce::dontSendNotification);
    gradientColourSelector1.addChangeListener(ref);
    gradientColourSelector2.addChangeListener(ref);

    pitchbendRangeLabel.setText("Pitchbend range", juce::dontSendNotification);
    pitchbendRangeField.onFocusLost = handlePitchbendRangeChanged;

    concertPitchLabel.setText("Concert pitch", juce::dontSendNotification);
    concertPitchField.onFocusLost = handleConcertPitchChanged;
}

void PianoRollSettingsWindow::placeComponents() {
    auto bounds = getLocalBounds().reduced(MARGIN);

    auto colourSchemeLine = bounds.removeFromTop(LINE_HEIGHT);
    colourSchemeLabel.setBounds(colourSchemeLine.removeFromLeft(COLUMN_WIDTH));
    colourSchemeComboBox.setBounds(colourSchemeLine.removeFromLeft(COLUMN_WIDTH));
    bounds.removeFromTop(LINE_SPACING);

    if (colourSchemeComboBox.getSelectedId() == GRADIENT_ID) {
        gradientColourLabel1.setVisible(true);
        gradientColourLabel2.setVisible(true);
        gradientColourSelector1.setVisible(true);
        gradientColourSelector2.setVisible(true);
        auto gradientLine1 = bounds.removeFromTop(COLOUR_LINE_HEIGHT);
        gradientColourLabel1.setBounds(gradientLine1.removeFromLeft(COLUMN_WIDTH));
        gradientColourSelector1.setBounds(gradientLine1.removeFromLeft(COLUMN_WIDTH));
        bounds.removeFromTop(LINE_SPACING);
        auto gradientLine2 = bounds.removeFromTop(COLOUR_LINE_HEIGHT);
        gradientColourLabel2.setBounds(gradientLine2.removeFromLeft(COLUMN_WIDTH));
        gradientColourSelector2.setBounds(gradientLine2.removeFromLeft(COLUMN_WIDTH));
        bounds.removeFromTop(LINE_SPACING);
    } else {
        gradientColourLabel1.setVisible(false);
        gradientColourLabel2.setVisible(false);
        gradientColourSelector1.setVisible(false);
        gradientColourSelector2.setVisible(false);
    }

    auto pitchbendRangeLine = bounds.removeFromTop(LINE_HEIGHT);
    pitchbendRangeLabel.setBounds(pitchbendRangeLine.removeFromLeft(COLUMN_WIDTH));
    pitchbendRangeField.setBounds(pitchbendRangeLine.removeFromRight(FIELD_WIDTH));
    bounds.removeFromTop(LINE_SPACING);

    auto concertPitchLine = bounds.removeFromTop(LINE_HEIGHT);
    concertPitchLabel.setBounds(concertPitchLine.removeFromLeft(COLUMN_WIDTH));
    concertPitchField.setBounds(concertPitchLine.removeFromRight(FIELD_WIDTH));
    bounds.removeFromTop(LINE_SPACING);

    // snapYToggle.setBounds(bounds.removeFromLeft(70));
    // bounds.removeFromLeft(MARGIN);
    // absoluteInfoToggle.setBounds(bounds.removeFromLeft(60));
    // bounds.removeFromLeft(MARGIN);
    // monitoringToggle.setBounds(bounds.removeFromLeft(90));
    // bounds.removeFromRight(MARGIN);
    // intervalsComboBox.setBounds(bounds.removeFromLeft(100));
    // bounds.removeFromLeft(MARGIN);
    // instructionsButton.setBounds(bounds.removeFromRight(20));
    // bounds.removeFromRight(MARGIN);
    // customIntervalsField.setBounds(bounds);
}

int PianoRollSettingsWindow::getColourSchemeId() const {
    return colourSchemeComboBox.getSelectedId();
}

std::pair<juce::Colour, juce::Colour> PianoRollSettingsWindow::getGradientColours() const {
    auto colour1 = gradientColourSelector1.getCurrentColour();
    auto colour2 = gradientColourSelector2.getCurrentColour();
    return std::make_pair(colour1, colour2);
}

float PianoRollSettingsWindow::getPitchbendRange() const {
    return pitchbendRangeField.getFloatValue();
}

float PianoRollSettingsWindow::getConcertPitch() const {
    return concertPitchField.getFloatValue();
}

void PianoRollSettingsWindow::setColourScheme(int colourSchemeId) {
    colourSchemeComboBox.setSelectedId(colourSchemeId);
}

void PianoRollSettingsWindow::setGradientColours(juce::Colour gradientColour1, juce::Colour gradientColour2) {
    gradientColourSelector1.setCurrentColour(gradientColour1);
    gradientColourSelector2.setCurrentColour(gradientColour2);
}

void PianoRollSettingsWindow::setPitchbendRange(float pitchbendRange) {
    pitchbendRangeField.setFloatValue(pitchbendRange);
}

void PianoRollSettingsWindow::setConcertPitch(float concertPitch) {
    concertPitchField.setFloatValue(concertPitch);
}

