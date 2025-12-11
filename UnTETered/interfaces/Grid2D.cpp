//
// Created by Vos on 15/11/2025.
//

#include "Grid2D.h"

Grid2D::Grid2D(size_t rows, size_t cols, Fraction horizontal, Fraction vertical,
    float freqBL, UnTETeredAudioProcessor& processor)
    : numRows(rows), numCols(cols), intervalHorizontal(horizontal),
        intervalVertical(vertical), frequencyBottomLeft(freqBL), processorRef(processor)
{
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

            auto note = generateNote(row, col);
            PitchClass pitchClass = note->getPitchClass();

            juce::Colour cellColour = getColourForPitchClass(pitchClass, nextCellStates[row][col]);

            g.setColour(cellColour);
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

std::unique_ptr<Note> Grid2D::generateNote(const size_t row, const size_t col) const {
    auto rowFlipped = numRows - row - 1;
    Fraction ratioToBL(1, 1);
    for (size_t i = 0; i < col; ++i)
        ratioToBL *= intervalHorizontal;
    for (size_t i = 0; i < rowFlipped; ++i)
        ratioToBL *= intervalVertical;

    float frequency = frequencyBottomLeft * ratioToBL.toFloat();
    return std::make_unique<RootNote>(frequency, 0, 0);
}

juce::Colour Grid2D::getColourForPitchClass(PitchClass pitchClass, bool selected) {
    float hue = pitchClass.value / 12.0f;
    auto colour = juce::Colour::fromHSV(hue, 0.4f, 0.7f, 1.0f);
    if (selected) {
        colour = colour.withBrightness(0.9f).withSaturation(0.25f);
    }
    return colour;
}

juce::Rectangle<int> Grid2D::getCellBounds(const size_t row, const size_t col) const {
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
    nextActiveNotesOrdered.clear();
    for (size_t row = 0; row < numRows; ++row)
        for (size_t col = 0; col < numCols; ++col)
            if (nextCellStates[row][col]) {
                auto note = generateNote(row, col);
                nextActiveNotesOrdered.push_back(note.get());
                nextActiveNotes.push_back(std::move(note));
            }

    optimiseTransition(currentActiveNotesOrdered, nextActiveNotesOrdered);
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

        juce::uint16 pitchBendValue = note->getPitchBendValue();
        auto pitchBend = juce::MidiMessage::pitchWheel(
            static_cast<int>(i + 2), pitchBendValue);
        processorRef.midiBuffer.addEvent(pitchBend, 30);
    }

    currentActiveNotes = std::move(nextActiveNotes);
    currentActiveNotesOrdered = std::move(nextActiveNotesOrdered);
    currentCellStates = std::move(nextCellStates);
    nextCellStates.resize(numRows, std::vector<bool>(numCols, false));

    repaint();
}