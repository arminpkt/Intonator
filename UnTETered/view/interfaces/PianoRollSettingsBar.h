//
// Created by Vos de Mens on 13/05/2026.
//

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "FractionsField.h"

class PianoRollSettingsBar : public juce::Component {
public:
    const int MARGIN = 5;
    const juce::Colour TRANSPARENT_COLOUR = juce::Colour::fromFloatRGBA(0, 0, 0, 0);
    const juce::Colour LINE_COLOUR = juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.4f);
    const juce::Colour TEXT_COLOUR = {255, 255, 255};

    explicit PianoRollSettingsBar(
        std::function<void()> handleLockY,
        std::function<void()> handleLockRef,
        std::function<void()> handleVals,
        std::function<void()> handleCustomVals,
        std::function<void()> handleMonitoring
        );

    bool getLockY() const;
    bool getLockRef() const;
    int getIntervals() const;
    std::vector<Fraction> getCustomIntervals() const;
    bool isMonitoringEnabled() const;

    void setLockY(bool lockY, bool sendNotification = false);
    void setLockRef(bool lockRef, bool sendNotification = false);
    void setIntervals(int id);
    void setCustomIntervals(const std::vector<Fraction>& fractions);
    void setMonitoringEnabled(bool enabled);

    void setCustomIntervalsVisibility(bool visible);

private:
    juce::ToggleButton lockYToggle { "lock Y" };
    juce::ToggleButton lockRefToggle { "lock ref" };
    juce::ComboBox intervalsComboBox;
    FractionsField customIntervalsField;
    juce::ToggleButton monitoringToggle { "monitor" };

    std::function<void()> handleLockYChange;
    std::function<void()> handleLockRefChange;
    std::function<void()> handleIntervalsChange;
    std::function<void()> handleCustomIntervalsChange;
    std::function<void()> handleMonitoringChange;

    void initialiseLockY();
    void initialiseLockRef();
    void initialiseIntervals();
    void initialiseCustomIntervals();
    void initialiseMonitoring();

    void resized() override;
};