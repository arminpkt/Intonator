//
// Created by Vos on 09/10/2025.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class PianoRoll : public juce::Component
{
public:
    PianoRoll(size_t rows, size_t cols);
    ~PianoRoll() override = default;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;

private:
    size_t numRows;
    size_t numCols;
    std::vector<std::vector<bool>> cellStates; // true = active (colored), false = inactive
    const std::unordered_set<int> whiteNotes = {0, 2, 4, 5, 7, 9, 11};

    juce::Rectangle<int> getCellBounds(size_t row, size_t col) const;
};
