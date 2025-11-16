//
// Created by Vos on 15/11/2025.
//

#include "Grid2D.h"

Grid2D::Grid2D(size_t rows, size_t cols, Fraction horizontal, Fraction vertical,
    float freqBL, AudioPluginAudioProcessor& processor)
    : numRows(rows), numCols(cols), intervalHorizontal(horizontal),
        intervalVertical(vertical), frequencyBottomLeft(freqBL), processorRef(processor){
    currentCellStates.resize(numRows, std::vector<bool>(numCols, false));
    nextCellStates.resize(numRows, std::vector<bool>(numCols, false));
    setWantsKeyboardFocus(true);
}

void Grid2D::paint(juce::Graphics& g) {
    // Draw grid cells and colors
    for (size_t row = 0; row < numRows; ++row)
    {
        for (size_t col = 0; col < numCols; ++col)
        {
            auto cell = getCellBounds(row, col);

            // Fill color if cell is set to be active next
            if (nextCellStates[row][col])
                g.setColour(juce::Colours::skyblue);
            else
                g.setColour(juce::Colours::white);

            g.fillRect(cell);

            // Draw dots in currently active cells
            if (currentCellStates[row][col]) {
                g.setColour(juce::Colours::red);
                auto circleBounds = cell.toFloat().expanded(
                    -.35f * cell.getWidth(), -.35f * cell.getHeight());
                g.fillEllipse(circleBounds);
            }

            // Draw cell border
            g.setColour(juce::Colours::black);
            g.drawRect(cell, 1.0f);
        }
    }
}

void Grid2D::mouseDown(const juce::MouseEvent& event) {
    auto pos = event.getPosition().toFloat();
    auto bounds = getLocalBounds().toFloat();
    float cellWidth = bounds.getWidth() / static_cast<float>(numCols);
    float cellHeight = bounds.getHeight() / static_cast<float>(numRows);

    size_t col = pos.x / cellWidth;
    size_t row = pos.y / cellHeight;

    // Toggle cell state if within bounds
    if (row < numRows && col < numCols)
    {
        nextCellStates[row][col] = !nextCellStates[row][col];
        update();
        repaint();
    }
}

bool Grid2D::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress('r')) {
        activateTransition();
        return true;
    }

    return false;
}

std::unique_ptr<Note> Grid2D::generateNote(const size_t row, const size_t col) {
    auto rowFlipped = numRows - row - 1;
    Fraction ratioToBL(1, 1);
    for (size_t i = 0; i < col; ++i)
        ratioToBL *= intervalHorizontal;
    for (size_t i = 0; i < rowFlipped; ++i)
        ratioToBL *= intervalVertical;

    float frequency = frequencyBottomLeft * ratioToBL.toFloat();
    return std::make_unique<RootNote>(frequency, 0, 0);
}

juce::Rectangle<int> Grid2D::getCellBounds(size_t row, size_t col) const {
    auto bounds = getLocalBounds();
    float cellWidth = bounds.getWidth() / numCols;
    float cellHeight = bounds.getHeight() / numRows;

    int x = static_cast<int>(col * cellWidth);
    int y = static_cast<int>(row * cellHeight);
    int w = static_cast<int>(cellWidth);
    int h = static_cast<int>(cellHeight);

    return juce::Rectangle<int>(x, y, w, h);
}

void Grid2D::update() {
    nextActiveNotes.clear();
    optimisedNextNotes.clear();
    for (size_t row = 0; row < numRows; ++row)
        for (size_t col = 0; col < numCols; ++col)
            if (nextCellStates[row][col]) {
                auto note = generateNote(row, col);
                nextActiveNotes.push_back(std::move(note));
            }

    if (currentActiveNotes.size() == nextActiveNotes.size()) {
        std::vector<Note*> rawCurrent;
        for (auto& unique : currentActiveNotes)
            rawCurrent.push_back(unique.get());
        for (auto& unique : nextActiveNotes)
            optimisedNextNotes.push_back(unique.get());
        std::cout << "ja" << std::endl;
        optimiseTransition(rawCurrent, optimisedNextNotes);
    }
}

void Grid2D::activateTransition() {
    for (size_t i = 0; i < currentActiveNotes.size(); ++i)
        processorRef.midiBuffer.addEvent(juce::MidiMessage::noteOff(
            static_cast<int>(i + 2), currentActiveNotes[i]->getRoundedMidiValue()), 10);

    for (size_t i = 0; i < nextActiveNotes.size(); ++i) {
        Note* note = nextActiveNotes[i].get();

        auto noteOn = juce::MidiMessage::noteOn(
            static_cast<int>(i + 2), note->getRoundedMidiValue(),
            static_cast<juce::uint8>(100));
        processorRef.midiBuffer.addEvent(noteOn, 20);

        juce::uint16 pitchBendValue = juce::MidiMessage::pitchbendToPitchwheelPos(
            note->getPitchBendInSemitones(), 2);
        auto pitchBend = juce::MidiMessage::pitchWheel(
            static_cast<int>(i + 2), pitchBendValue);
        processorRef.midiBuffer.addEvent(pitchBend, 30);
    }

    currentActiveNotes = std::move(nextActiveNotes);
    currentCellStates = std::move(nextCellStates);
    nextCellStates.resize(numRows, std::vector<bool>(numCols, false));

    repaint();
}