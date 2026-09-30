//
// Created by Vos de Mens on 28/09/2026.
//

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class InstructionsField : public juce::Component {
public:
    std::vector<std::pair<juce::String, juce::String>> CONTROLS = {
        {"Double click", "Create a note"},
        {"Option+click a note", "Set the note as a reference."},
        {"Cmd+C / Ctrl+C", "Copy selection"},
        {"Cmd+X / Ctrl+X", "Cut selection"},
        {"Cmd+V / Ctrl+V", "Paste"},
        {"Cmd+Z / Ctrl+Z", "Undo"},
        {"Cmd+Shift+Z / Ctrl+Shift+Z", "Redo"},
        {"Cmd+D / Ctrl+D", "Duplicate selection"},
        {"Cmd+A / Ctrl+A", "Select all"},
        {"Cmd+1 / Ctrl+1", "Narrow grid"},
        {"Cmd+2 / Ctrl+2", "Widen grid"},
        {"Cmd+3 / Ctrl+3", "Triplet grid"},
        {"Backspace", "Delete selection"},
        {"Option+Up", "Move selection an octave up"},
        {"Option+Down", "Move selection an octave down"},
        {"Y", "Toggle lock Y setting"},
        {"I", "Toggle info setting"},
        {"T", "Round reference frequency of selection to 12TET"}
    };

    void paint (juce::Graphics& g) override {
        g.setColour(juce::Colours::darkgrey);
        g.fillRect(0, 0, getWidth(), getHeight());
        g.setColour(juce::Colours::lightgrey);
        g.drawText("Instructions", 0, 30, getWidth(), 60, juce::Justification::centred);
        g.drawMultiLineText("hoi\nhoi\nhallo", 100, 100, 150);
    }
};

class InstructionsWindow : public juce::DialogWindow {

    InstructionsField background{};

public:
    InstructionsWindow()
        : DialogWindow("Instructions",
                      juce::Colours::black,
                      true) {

        // Add any components you want
        setContentOwned(&background, true);

        // Set reasonable size
        centreWithSize(getWidth(), getHeight());
        enterModalState();
        setResizable(false, false);
    }

    void closeButtonPressed() override {
        close();
    }

    void open() {
        setCentrePosition(getBounds().getCentre());
        centreWithSize(500, 700);
        enterModalState();
        setVisible(true);
        setAlwaysOnTop(true);
    }

    void close() {
        setVisible(false);
    }
};