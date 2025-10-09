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

    juce::Rectangle<int> getCellBounds(int row, int col) const;
};
