//
// Created by Vos on 15/11/2025.
//

#include "Grid2D.h"

// order of operations:
// implement Animator objects
// test
// make clickable during shift
// test

Grid2D::Grid2D(int rows, int cols, Fraction horizontal, Fraction vertical, float refFreq,
    UnTETeredAudioProcessor& processor, juce::VBlankAnimatorUpdater& updater)
    : numRows(rows), numCols(cols), intervalHorizontal(horizontal),
    intervalVertical(vertical), refCoordinatesCellTarget({(rows-1)/2, (cols-1)/2}),
    refNote(std::make_unique<RootNote>(refFreq, 0, 0)), processorRef(processor),
    gridTranspositionAnimator(
        juce::ValueAnimatorBuilder{}
            .withOnStartCallback([this]{
                auto refCellActual = getCellBounds(refCoordinatesCellActual);
                auto refCellTarget = getCellBounds(refCoordinatesCellTarget);
                auto refCoordinatesPxActual = refCellActual.getCentre();
                auto refCoordinatesPxTarget = refCellTarget.getCentre();
                paintingOffsetInitial = refCoordinatesPxTarget - refCoordinatesPxActual;
            })
            .withValueChangedCallback([this](auto value){
                paintingOffset = juce::makeAnimationLimits(
                    paintingOffsetInitial, {0, 0}
                ).lerp(value);
                repaint();
            })
            .withEasing(juce::Easings::createEaseInOut())
            .withDurationMs(600)
            .build()
    ) {
    currentCellStates.resize(numRows, std::vector(numCols, false));
    nextCellStates.resize(numRows, std::vector(numCols, false));
    updater.addAnimator(gridTranspositionAnimator);

    setWantsKeyboardFocus(true);
}

void Grid2D::paint(juce::Graphics& g) {
    for (int row = -numRows; row < numRows; ++row) {
        for (int col = -numCols; col < numCols; ++col) {
            auto cell = getCellBounds({row, col});

            auto cellColour = getColourForCoordinates({row, col});

            g.setColour(cellColour);
            g.fillRect(cell);

            // Draw dots in currently active cells
            if (row > 0 && col > 0 && currentCellStates[row][col]) {
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
    auto pos = event.getPosition();

    auto cell = getCellFromPx(pos);
    auto row = cell.y;
    auto col = cell.x;

    if (row < numRows && col < numCols)
    {
        nextCellStates[row][col] = !nextCellStates[row][col];
        updateNextActive();
        repaint();
    }
}

juce::Point<int> Grid2D::getCellFromPx(const juce::Point<int>& px) const {
    auto px_float = px.toFloat();
    auto bounds = getLocalBounds().toFloat();
    float cellWidth = bounds.getWidth() / static_cast<float>(numCols);
    float cellHeight = bounds.getHeight() / static_cast<float>(numRows);

    int col = static_cast<int>(px_float.x / cellWidth);
    int row = static_cast<int>(px_float.y / cellHeight);

    return {col, row};
}

bool Grid2D::keyPressed(const juce::KeyPress& key) {
    if (key == juce::KeyPress('r')) {
        activateTransition();
        return true;
    }

    return false;
}

void Grid2D::activateTransition() {
    for (size_t i = 0; i < currentActiveNotesOrdered.size(); ++i) {
        Note* note = currentActiveNotesOrdered[i];

        auto noteOff = juce::MidiMessage::noteOff(
            static_cast<int>(i + 2), note->getRoundedMidiValue());
        processorRef.midiBuffer.addEvent(noteOff, 10);
    }

    for (size_t i = 0; i < nextActiveNotesOrdered.size(); ++i) {
        Note* note = nextActiveNotesOrdered[i];

        auto noteOn = juce::MidiMessage::noteOn(
            static_cast<int>(i + 2), note->getRoundedMidiValue(),
            static_cast<juce::uint8>(100));
        processorRef.midiBuffer.addEvent(noteOn, 20);

        auto pitchBendValue = note->getPitchBendValue();
        auto pitchBend = juce::MidiMessage::pitchWheel(
            static_cast<int>(i + 2), pitchBendValue);
        processorRef.midiBuffer.addEvent(pitchBend, 30);
    }

    currentActiveNotes = std::move(nextActiveNotes);
    currentActiveNotesOrdered = std::move(nextActiveNotesOrdered);
    currentCellStates = std::move(nextCellStates);
    nextCellStates.resize(numRows, std::vector(numCols, false));

    refCoordinatesCellActual = calculateCenterOfGravityCell();
    refNote = generateNote(refCoordinatesCellActual);

    gridTranspositionAnimator.start();
}

juce::Colour Grid2D::getColourForPitchClass(PitchClass pitchClass, bool selected) {
    float hue = pitchClass.value / 12.0f;
    auto colour = juce::Colour::fromHSV(hue, 0.4f, 0.7f, 1.0f);
    if (selected) {
        colour = colour.withBrightness(0.9f).withSaturation(0.25f);
    }

    return colour;
}

juce::Rectangle<int> Grid2D::getCellBounds(juce::Point<int> coordinates) const {
    auto bounds = getLocalBounds().toFloat();
    float cellWidth = bounds.getWidth() / numCols;
    float cellHeight = bounds.getHeight() / numRows;

    int x = static_cast<int>(coordinates.x * cellWidth) + paintingOffset.x;
    int y = static_cast<int>(coordinates.y * cellHeight) + paintingOffset.y;
    int w = static_cast<int>(cellWidth);
    int h = static_cast<int>(cellHeight);

    return {x, y, w, h};
}

juce::Point<int> Grid2D::mirrorY(juce::Point<int> point) const {
    return {point.x, static_cast<int>(numRows) - point.y - 1};
}

juce::Point<int> Grid2D::mirrorYPx(juce::Point<int> point) const {
    return {point.x, getHeight() - point.y - 1};
}

std::unique_ptr<Note> Grid2D::generateNote(juce::Point<int> coordinates) const {
    auto mirrored = mirrorY(coordinates);

    Fraction ratioToRef(1, 1);
    for (int x = refCoordinatesCellTarget.x; x < mirrored.x; ++x)
        ratioToRef = ratioToRef * intervalHorizontal;
    for (int x = refCoordinatesCellTarget.x; x > mirrored.x; --x)
        ratioToRef = ratioToRef / intervalHorizontal;
    for (int y = refCoordinatesCellTarget.y; y < mirrored.y; ++y)
        ratioToRef = ratioToRef * intervalVertical;
    for (int y = refCoordinatesCellTarget.y; y > mirrored.y; --y)
        ratioToRef = ratioToRef / intervalVertical;

    return std::make_unique<ChildNote>(*refNote, ratioToRef, 0, 0);
}

juce::Colour Grid2D::getColourForCoordinates(juce::Point<int> coordinates) const {
    auto note = generateNote(coordinates);
    PitchClass pitchClass = note->getPitchClass();

    if (coordinates.y < 0 || coordinates.y >= numRows || coordinates.x < 0 || coordinates.x >= numCols)
        return getColourForPitchClass(pitchClass, false);

    return getColourForPitchClass(pitchClass, nextCellStates[coordinates.y][coordinates.x]);
}

void Grid2D::updateNextActive() {
    nextActiveNotes.clear();
    nextActiveNotesOrdered.clear();
    for (int row = 0; row < numRows; ++row)
        for (int col = 0; col < numCols; ++col)
            if (nextCellStates[row][col]) {
                auto note = generateNote({row, col});
                nextActiveNotesOrdered.push_back(note.get());
                nextActiveNotes.push_back(std::move(note));
            }

    optimiseTransition(currentActiveNotesOrdered, nextActiveNotesOrdered);
}

juce::Point<int> Grid2D::calculateCenterOfGravityCell() const {
    std::vector<juce::Point<int>> centers;
    for (int row = 0; row < numRows; ++row)
        for (int col = 0; col < numCols; ++col)
            if (currentCellStates[row][col])
                centers.push_back(getCellBounds({row, col}).getCentre());

    juce::Point<float> sum;
    for (const auto& center : centers)
        sum += center.toFloat();

    juce::Point<float> centerFloat = sum / centers.size();
    centerFloat -= {0.1f, 0.1f};

    juce::Point<int> center = centerFloat.roundToInt();

    auto cell = getCellFromPx(center);

    return cell;
}