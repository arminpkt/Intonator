//
// Created by Vos on 09/10/2025.
//

#include "PianoRoll.h"

PianoRoll::PianoRoll(size_t rows, size_t cols)
    : numRows(rows), numCols(cols) {
    cellStates.resize(numRows, std::vector<bool>(numCols, false));
}

void PianoRoll::paint(juce::Graphics& g) {
    // Draw grid cells and colors
    for (size_t row = 0; row < numRows; ++row)
    {
        for (size_t col = 0; col < numCols; ++col)
        {
            auto cell = getCellBounds(row, col);

            // Fill color if cell is active
            if (cellStates[row][col])
                g.setColour(juce::Colours::skyblue);
            else {
                if (whiteNotes.find(row % 12) != whiteNotes.end())
                    g.setColour(juce::Colours::white);
                else
                    g.setColour(juce::Colours::lightgrey);
            }

            g.fillRect(cell);

            // Draw cell border
            g.setColour(juce::Colours::black);
            g.drawRect(cell, 1.0f);
        }
    }
}

void PianoRoll::mouseDown(const juce::MouseEvent& event) {
    auto pos = event.getPosition();
    auto bounds = getLocalBounds();
    float cellWidth = bounds.getWidth() / (float)numCols;
    float cellHeight = bounds.getHeight() / (float)numRows;

    int col = pos.x / cellWidth;
    int row = pos.y / cellHeight;

    // Toggle cell state if within bounds
    if (row >= 0 && row < numRows && col >= 0 && col < numCols)
    {
        cellStates[row][col] = !cellStates[row][col];
        repaint();
    }
}

juce::Rectangle<int> PianoRoll::getCellBounds(size_t row, size_t col) const {
    auto bounds = getLocalBounds();
    float cellWidth = bounds.getWidth() / (float)numCols;
    float cellHeight = bounds.getHeight() / (float)numRows;

    int x = static_cast<int>(col * cellWidth);
    int y = static_cast<int>(row * cellHeight);
    int w = static_cast<int>(cellWidth);
    int h = static_cast<int>(cellHeight);

    return juce::Rectangle<int>(x, y, w, h);
}
