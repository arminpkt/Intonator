//
// Created by Vos de Mens on 28/09/2026.
//

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class InstructionsField : public juce::Component {
public:
    std::vector<std::pair<juce::String, juce::String>> CONTROLS = {
        {"Double click", "Create or delete a note"},
        {"Option+click a note", "Set note as a reference"},
        {"Cmd+C / Ctrl+C", "Copy selection"},
        {"Cmd+X / Ctrl+X", "Cut selection"},
        {"Cmd+V / Ctrl+V", "Paste"},
        {"Cmd+D / Ctrl+D", "Duplicate selection"},
        {"Cmd+A / Ctrl+A", "Select all"},
        {"Cmd+Z / Ctrl+Z", "Undo"},
        {"Cmd+Shift+Z / Ctrl+Shift+Z", "Redo"},
        {"Cmd+1 / Ctrl+1", "Narrow grid"},
        {"Cmd+2 / Ctrl+2", "Widen grid"},
        {"Cmd+3 / Ctrl+3", "Triplet grid"},
        {"Backspace", "Delete selection"},
        {"Option+Up", "Move selection an octave up"},
        {"Option+Down", "Move selection an octave down"},
        {"Y", "Toggle lock Y setting"},
        {"I", "Toggle info setting"},
        {"T", "Round selection to 12TET"}
    };

    void paint (juce::Graphics& g) override {
        g.setColour({50, 50, 50});
        g.fillRect(0, 0, getWidth(), getHeight());
        g.setColour(juce::Colours::lightgrey);
        int y = 50;
        int increment = 30;
        for (auto [key, entry] : CONTROLS) {
            g.drawHorizontalLine(y, 50, 450);
            g.drawText(key, 60, y, 180, increment, juce::Justification::centredLeft, true);
            g.drawText(entry, 260, y, 180, increment, juce::Justification::centredLeft, true);
            y += increment;
        }
        g.drawHorizontalLine(y, 50, 450);
        g.drawVerticalLine(50, 50, y);
        g.drawVerticalLine(250, 50, y);
        g.drawVerticalLine(450, 50, y);
    }
};

class InstructionsWindow : public juce::DialogWindow {

    InstructionsField background{};

public:
    InstructionsWindow()
        : DialogWindow("Controls",
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
        centreWithSize(500, 670);
        enterModalState();
        setVisible(true);
        setAlwaysOnTop(true);
    }

    void close() {
        setVisible(false);
    }
};