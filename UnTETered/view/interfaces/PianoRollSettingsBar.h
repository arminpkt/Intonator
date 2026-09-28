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
        std::function<void()> handleLockY,
        std::function<void()> handleAbsoluteInfo,
        std::function<void()> handleVals,
        std::function<void()> handleCustomVals,
        std::function<void()> handleMonitoring,
        std::function<void()> handleInstructions
        );

    bool getLockY() const;
    bool getAbsoluteInfo() const;
    int getIntervals() const;
    std::vector<Fraction> getCustomIntervals() const;
    bool getMonitoring() const;

    void setLockY(bool lockY, bool sendNotification = false);
    void setAbsoluteInfo(bool absoluteInfo, bool sendNotification = false);
    void setIntervals(int id);
    void setCustomIntervals(const std::vector<Fraction>& fractions);
    void setMonitoring(bool enabled);

    void setCustomIntervalsVisibility(bool visible);

private:
    juce::ToggleButton lockYToggle { "lock Y" };
    juce::ToggleButton absoluteInfoToggle {"info"};
    juce::ComboBox intervalsComboBox;
    FractionsField customIntervalsField;
    juce::ToggleButton monitoringToggle { "monitor" };
    juce::TextButton instructionsButton { "?" };

    std::function<void()> handleLockYChange;
    std::function<void()> handleAbsoluteInfoChange;
    std::function<void()> handleIntervalsChange;
    std::function<void()> handleCustomIntervalsChange;
    std::function<void()> handleMonitoringChange;
    std::function<void()> handleInstructionsClick;

    void initialiseLockY();
    void initialiseAbsoluteInfo();
    void initialiseIntervals();
    void initialiseCustomIntervals();
    void initialiseMonitoring();
    void initialiseInstructions();

    void resized() override;
};