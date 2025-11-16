//
// Created by Vos on 15/11/2025.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

#include "../logic/util.h"
#include "../logic/NoteRegion.h"
#include "../logic/header-only/Fraction.h"
#include "../PluginProcessor.h"

class Grid2D : public juce::Component
{
    public:
    Grid2D(size_t rows, size_t cols, Fraction horizontal, Fraction vertical,
        float freqBL, AudioPluginAudioProcessor& processor);
    ~Grid2D() override = default;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    bool keyPressed(const juce::KeyPress& event) override;

    void activateTransition();

    private:
    size_t numRows;
    size_t numCols;
    Fraction intervalHorizontal;
    Fraction intervalVertical;
    float frequencyBottomLeft;
    RootNote A440{440, 0, 0};

    juce::Rectangle<int> getCellBounds(size_t row, size_t col) const;

    std::vector<std::vector<bool>> currentCellStates;
    std::vector<std::vector<bool>> nextCellStates;
    std::vector<std::unique_ptr<Note>> currentActiveNotes;
    std::vector<std::unique_ptr<Note>> nextActiveNotes;
    std::vector<Note*> optimisedNextNotes;
    AudioPluginAudioProcessor& processorRef;

    std::unique_ptr<Note> generateNote(size_t row, size_t col);
    void update();

    std::vector<juce::MidiMessage> midiMessages;
};
