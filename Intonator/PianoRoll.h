//
// Created by Vos on 09/10/2025.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class PianoRoll : public juce::Component
{
public:
    PianoRoll(int rows, int cols);
    ~PianoRoll() override = default;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;

private:
    int numRows;
    int numCols;
    std::vector<std::vector<bool>> cellStates; // true = active (colored), false = inactive
    const std::unordered_set<int> allowedNotes = {0, 2, 4, 5, 7, 9, 11};

    juce::Rectangle<int> getCellBounds(int row, int col) const;
};
