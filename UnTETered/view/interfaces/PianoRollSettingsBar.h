//
// Created by Vos de Mens on 13/05/2026.
//

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "FractionsField.h"

class PianoRollSettingsBar : public juce::Component {
public:
    const int MARGIN = 5;

    explicit PianoRollSettingsBar(
        std::function<void()> handleSnapY,
        std::function<void()> handleAbsoluteInfo,
        std::function<void()> handleMonitoring,
        std::function<void()> handleVals,
        std::function<void()> handleCustomVals,
        std::function<void()> handleSettings,
        std::function<void()> handleInstructions,
        std::function<void(const juce::MouseEvent&)> mouseMoveParent
        );

    bool getSnapY() const;
    bool getAbsoluteInfo() const;
    bool getMonitoring() const;
    int getIntervals() const;
    std::vector<Fraction> getCustomIntervals() const;

    void setSnapY(bool snapY, bool sendNotification = false);
    void setAbsoluteInfo(bool absoluteInfo, bool sendNotification = false);
    void setMonitoring(bool monitoring, bool sendNotification = false);
    void setIntervals(int id);
    void setCustomIntervals(const std::vector<Fraction>& fractions);

    void setCustomIntervalsVisibility(bool visible);

    void mouseMove(const juce::MouseEvent& event) override;

private:
    juce::ToggleButton snapYToggle { "snap Y" };
    juce::ToggleButton absoluteInfoToggle {"info"};
    juce::ToggleButton monitoringToggle { "monitor" };
    juce::ComboBox intervalsComboBox;
    FractionsField customIntervalsField;
    juce::TextButton settingsButton { juce::String::fromUTF8(u8"⚙") };
    juce::TextButton instructionsButton { "?" };

    std::function<void()> handleSnapYChange;
    std::function<void()> handleAbsoluteInfoChange;
    std::function<void()> handleMonitoringChange;
    std::function<void()> handleIntervalsChange;
    std::function<void()> handleCustomIntervalsChange;
    std::function<void()> handleSettingsClick;
    std::function<void()> handleInstructionsClick;
    std::function<void(const juce::MouseEvent&)> handleMouseMove;

    void initialiseSnapY();
    void initialiseAbsoluteInfo();
    void initialiseMonitoring();
    void initialiseIntervals();
    void initialiseCustomIntervals();
    void initialiseSettings();
    void initialiseInstructions();

    void resized() override;
};