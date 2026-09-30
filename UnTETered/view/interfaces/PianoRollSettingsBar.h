//
// Created by Vos de Mens on 13/05/2026.
//

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "FractionsField.h"

class PianoRollSettingsBar : public juce::Component {
public:
    static constexpr const char* INSTRUCTIONS_TEXT =
        "Double click anywhere to create a note.\n"
        "Option+click a note to set it as a reference.\n"
        "\nKeyboard controls:\n"
        "Cmd+C / Ctrl+C: Copy selection\n"
        "Cmd+X / Ctrl+X: Cut selection\n"
        "Cmd+V / Ctrl+V: Paste\n"
        "Cmd+Z / Ctrl+Z: Undo\n"
        "Cmd+Shift+Z / Ctrl+Shift+Z: Redo\n"
        "Cmd+D / Ctrl+D: Duplicate selection\n"
        "Cmd+A / Ctrl+A: Select all\n"
        "Cmd+1 / Ctrl+1: Narrow grid\n"
        "Cmd+2 / Ctrl+2: Widen grid\n"
        "Cmd+3 / Ctrl+3: Triplet grid\n"
        "Backspace: Delete selection\n"
        "Option+Up: Move selection an octave up\n"
        "Option+Down: Move selection an octave down\n"
        "Y: Toggle lock Y setting\n"
        "I: Toggle info setting\n"
        "T: Round reference frequency of selection to 12TET"
    ;
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
    juce::ToggleButton lockYToggle { "snap Y" };
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