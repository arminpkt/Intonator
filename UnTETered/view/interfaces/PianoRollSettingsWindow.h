//
// Created by Vos de Mens on 20/10/2026.
//

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "NumericField.h"
#include "TrimmedSlider.h"



class PianoRollSettingsWindow : public juce::DialogWindow {
public:
    static constexpr int MARGIN = 20;
    static constexpr int COLUMN_WIDTH = 200;
    static constexpr int FIELD_WIDTH = 70;
    static constexpr int LINE_HEIGHT = 20;
    static constexpr int COLOUR_LINE_HEIGHT = 100;
    static constexpr int LINE_SPACING = 5;

    static constexpr int RAINBOW_ID = 1;
    static constexpr int GRADIENT_ID = 2;

    juce::LookAndFeel_V4 lookAndFeel;

    PianoRollSettingsWindow(
        std::function<void()> handleColourScheme,
        std::function<void()> handleGradientColour,
        std::function<void()> handlePitchbendRange,
        std::function<void()> handleConcertPitch,
        juce::ChangeListener* ref
        );

    void open();
    void close();
    void initialiseLookAndFeel();
    void initialiseComponents(juce::ChangeListener* ref);
    void placeComponents();

    int getColourSchemeId() const;
    std::pair<juce::Colour, juce::Colour> getGradientColours() const;
    float getPitchbendRange() const;
    float getConcertPitch() const;

    void setColourScheme(int colourSchemeId);
    void setGradientColours(juce::Colour gradientColour1, juce::Colour gradientColour2);
    void setPitchbendRange(float pitchbendRange);
    void setConcertPitch(float concertPitch);

    void closeButtonPressed() override;

    Component content{};

    juce::Label colourSchemeLabel;
    juce::ComboBox colourSchemeComboBox;

    juce::Label gradientColourLabel1;
    juce::Label gradientColourLabel2;
    juce::ColourSelector gradientColourSelector1{juce::ColourSelector::ColourSelectorOptions::showColourspace};
    juce::ColourSelector gradientColourSelector2{juce::ColourSelector::ColourSelectorOptions::showColourspace};

    juce::Label pitchbendRangeLabel;
    NumericField pitchbendRangeField;

    juce::Label concertPitchLabel;
    NumericField concertPitchField;

    std::function<void()> handleColourSchemeChanged;
    std::function<void()> handleGradientColourSchemeChanged;
    std::function<void()> handlePitchbendRangeChanged;
    std::function<void()> handleConcertPitchChanged;
};
