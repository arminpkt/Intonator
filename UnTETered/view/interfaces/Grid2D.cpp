//
// Created by Vos on 15/11/2025.
//

#include "Grid2D.h"

// order of operations:
// implement Animator objects
// test
// make clickable during shift
// test

Grid2D::Grid2D(const Point& dim, const Fraction& horizontal, const Fraction& vertical,
        const double freqOr, UnTETeredAudioProcessor& proc, juce::VBlankAnimatorUpdater& updater) :
    processor(proc), dimScreenCells(dim), boundsScreenCells(0, 0, dim.x, dim.y),
    dimKernelCells(3*dim), boundsKernelCells(0, 0, 3*dim.x, 3*dim.y),
    intervalHorizontal(horizontal), intervalVertical(vertical), noteOrigin(RootNote(freqOr, 0, 0)),
    middleCellScreen((dim-Point{1, 1})/2), offsetFromOriginGrid(Point(0, 0)),
    gridTranspositionAnimator(
        juce::ValueAnimatorBuilder{}
        .withValueChangedCallback([this](auto value){
          paintingOffsetPx = juce::makeAnimationLimits(
              paintingOffsetPxInitial, {0, 0}
          ).lerp(value);
          repaint();
        })
        .withOnCompleteCallback([this] {repaint();})
        .withEasing(juce::Easings::createEaseInOut())
        .withDurationMs(600)
        .build()
    ) {
        updater.addAnimator(gridTranspositionAnimator);
        kernel = createEmptyKernel(dimKernelCells);

        setWantsKeyboardFocus(true);
    }

// deze kan slimmer, eerst alles tekenen, daarna de geselecteerde en actieve cellen veranderen (scheelt niks)
void Grid2D::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    auto [xCellKernelMin, yCellKernelMin] = getCellKernelFromPx({0, 0});
    auto [xCellKernelMax, yCellKernelMax] = getCellKernelFromPx({bounds.getWidth(), bounds.getHeight()});
    for (int y = yCellKernelMin - 1; y <= yCellKernelMax; ++y) {
        for (int x = xCellKernelMin - 1; x <= xCellKernelMax; ++x) {
            Point cellKernel = {x, y};
            Point cellGrid = getCellGridFromKernel(cellKernel);
            auto pxBoundsDraw = getPxBoundsFromCellKernel(cellKernel);
            auto cellColour = getColourForCellKernel(cellKernel);

            g.setColour(cellColour);
            g.fillRect(pxBoundsDraw);

            // Draw dots in currently active cells
            if (activeCellsGrid.find(cellGrid) != activeCellsGrid.end()) {
                g.setColour(juce::Colours::red);
                auto pxBoundsDrawFloat = pxBoundsDraw.toFloat();
                auto circleBounds = pxBoundsDraw.toFloat().expanded(
                    -.35f * pxBoundsDrawFloat.getWidth(), -.35f * pxBoundsDrawFloat.getHeight());
                g.fillEllipse(circleBounds);
            }

            // Draw cell border
            g.setColour(juce::Colours::black);
            g.drawRect(pxBoundsDraw, 1.0f);
        }
    }
}

void Grid2D::mouseDown(const juce::MouseEvent& event) {
    Point posPx = event.getPosition();
    Point mirrored = mirrorYPx(posPx);
    Point cellGrid = getCellGridFromPx(mirrored);

    if (selectedCellsGrid.find(cellGrid) != selectedCellsGrid.end())
        selectedCellsGrid.erase(cellGrid);
    else
        selectedCellsGrid.insert(cellGrid);

    repaint();
}

bool Grid2D::keyPressed(const juce::KeyPress& key) {
    if (key == juce::KeyPress::spaceKey) {
        activateTransition();
        return true;
    }
    if (key == juce::KeyPress::backspaceKey) {
        selectedCellsGrid.clear();
        repaint();
        return true;
    }
    auto code = static_cast<size_t>(key.getKeyCode());
    DBG(code);
    if (code >= 65 && code <= 90) {
        if (key.getModifiers().isShiftDown()) {
            if (!selectedCellsGrid.empty()) {
                saves[code] = {{}, 's'};
                for (auto& selectedGrid : selectedCellsGrid)
                    saves[code].first.insert(getCellScreenFromGrid(selectedGrid));
            }
            return true;
        }
        if (key.getModifiers().isAltDown()) {
            if (!selectedCellsGrid.empty()) {
                saves[code] = {{}, 'a'};
                for (auto& selectedGrid : selectedCellsGrid)
                    saves[code].first.insert(getCellScreenFromGrid(selectedGrid));
            }
            return true;
        }
        selectedCellsGrid.clear();
        for (auto& save : saves[code].first)
            selectedCellsGrid.insert(getCellGridFromScreen(save));
        if (saves[code].second == 's') {
            repaint();
            return true;
        }
        if (saves[code].second == 'a') {
            activateTransition();
            return true;
        }
    }
    return false;
}

void Grid2D::activateTransition() {
    for (size_t i = 0; i < activeNotes.size(); ++i) {
        const Note* note = activeNotes[i];

        auto noteOff = juce::MidiMessage::noteOff(
            static_cast<int>(i + 2), note->getRoundedMidiValue());
        processor.midiBuffer.addEvent(noteOff, 10);
    }

    bool sameSize = activeCellsGrid.size() == selectedCellsGrid.size();

    if (!sameSize)
        calibrateGrid();

    std::vector<Note*> nextActiveNotes;
    for (auto& cellGrid : selectedCellsGrid) {
        Note* note = getNoteFromGrid(cellGrid, !sameSize);
        nextActiveNotes.push_back(note);
    }
    optimiseTransition(activeNotes, nextActiveNotes);

    for (size_t i = 0; i < nextActiveNotes.size(); ++i) {
        const Note* note = nextActiveNotes[i];

        auto noteOn = juce::MidiMessage::noteOn(
            static_cast<int>(i + 2), note->getRoundedMidiValue(),
            static_cast<juce::uint8>(100));
        processor.midiBuffer.addEvent(noteOn, 20);

        auto pitchBendValue = note->getPitchBendValue();
        auto pitchBend = juce::MidiMessage::pitchWheel(
            static_cast<int>(i + 2), pitchBendValue);
        processor.midiBuffer.addEvent(pitchBend, 30);
    }

    activeNotes = std::move(nextActiveNotes);
    activeCellsGrid = std::move(selectedCellsGrid);

    transposeGrid();
}

void Grid2D::calibrateGrid() {
    while (getNoteFromScreen(middleCellScreen, true)->frequency > MAX_FREQ_MIDDLE)
        octavateGridDown();
    while (getNoteFromScreen(middleCellScreen, true)->frequency < MIN_FREQ_MIDDLE)
        octavateGridUp();
}

void Grid2D::octavateGridDown() {
    noteOrigin /= 2;
}

void Grid2D::octavateGridUp() {
    noteOrigin *= 2;
}

juce::Colour Grid2D::getColourForNote(Note* note, bool selected) {
    float hue = note->getHue();
    auto colour = juce::Colour::fromHSV(hue, 0.4f, 0.7f, 1.0f);
    if (selected)
        colour = colour.withBrightness(0.9f).withSaturation(0.25f);

    return colour;
}

Kernel Grid2D::createEmptyKernel(Point dim) {
    auto x = static_cast<size_t>(dim.x);
    auto y = static_cast<size_t>(dim.y);

    Kernel emptyKernel;
    emptyKernel.resize(y);
    for (auto & row : emptyKernel)
        row.resize(x);
    return emptyKernel;
}

Rect Grid2D::getPxBoundsFromCellScreen(const Point& inputCellScreen) const {
    PointF dimCellPxFloat = getDimCellPxFloat();
    Point posPx = (inputCellScreen.toFloat() * dimCellPxFloat).roundToInt() + paintingOffsetPx;
    Point dimCellPx = dimCellPxFloat.roundToInt();
    Rect pxBounds = {posPx.x, posPx.y, dimCellPx.x, dimCellPx.y};
    Rect localBounds = getLocalBounds();
    if (pxBounds.getX() < 0)
        pxBounds.removeFromLeft(pxBounds.getX());
    if (pxBounds.getY() < 0)
        pxBounds.removeFromTop(pxBounds.getY());
    if (pxBounds.getRight() > localBounds.getWidth())
        pxBounds.removeFromRight(pxBounds.getRight() - localBounds.getWidth());
    if (pxBounds.getBottom() > localBounds.getHeight())
        pxBounds.removeFromBottom(pxBounds.getBottom() - localBounds.getHeight());

    Rect mirrored = mirrorYPx(pxBounds);
    return mirrored;
}

Rect Grid2D::getPxBoundsFromCellKernel(const Point& inputCellKernel) const {
    Point cellScreen = getCellScreenFromKernel(inputCellKernel);
    return getPxBoundsFromCellScreen(cellScreen);
}

PointF Grid2D::getDimCellPxFloat() const {
    RectF boundsScreenPxFloat = getLocalBounds().toFloat();
    PointF dimScreenPxFloat = PointF(boundsScreenPxFloat.getWidth(), boundsScreenPxFloat.getHeight());
    PointF dimScreenCellsFloat = dimScreenCells.toFloat();
    PointF dimCellPxFloat = dimScreenPxFloat / dimScreenCellsFloat;
    return dimCellPxFloat;
}

Point Grid2D::getCellScreenFromPx(const Point& inputPx) const {
    PointF dimCellPxFloat = getDimCellPxFloat();
    Point inputPxCorrected = inputPx - paintingOffsetPx;
    Point inputCellScreen = (inputPxCorrected.toFloat() / dimCellPxFloat).toInt();
    return inputCellScreen;
}

Point Grid2D::getCellKernelFromPx(const Point& inputPx) const {
    Point cellScreen = getCellScreenFromPx(inputPx);
    return getCellKernelFromScreen(cellScreen);
}

Point Grid2D::getCellGridFromPx(const Point& inputPx) const {
    Point cellScreen = getCellScreenFromPx(inputPx);
    return getCellGridFromScreen(cellScreen);
}

Point Grid2D::getCellKernelFromScreen(const Point& inputCellScreen) const {
    Point cellKernel = inputCellScreen + dimScreenCells;
    if (!boundsKernelCells.contains(cellKernel))
        throw std::invalid_argument("invalid kernel cell");
    return inputCellScreen + dimScreenCells;
}

Point Grid2D::getCellScreenFromKernel(const Point& inputCellKernel) const {
    return inputCellKernel - dimScreenCells;
}

Point Grid2D::getCellGridFromScreen(const Point& inputCellScreen) const {
    return inputCellScreen + offsetFromOriginGrid - middleCellScreen;
}

Point Grid2D::getCellScreenFromGrid(const Point& inputCellGrid) const {
    return inputCellGrid - offsetFromOriginGrid + middleCellScreen;
}

Point Grid2D::getCellGridFromKernel(const Point& inputCellKernel) const {
    Point cellScreen = getCellScreenFromKernel(inputCellKernel);
    return getCellGridFromScreen(cellScreen);
}

Point Grid2D::getCellKernelFromGrid(const Point& inputCellGrid) const {
    Point cellScreen = getCellScreenFromGrid(inputCellGrid);
    return getCellKernelFromScreen(cellScreen);
}

Note* Grid2D::getNoteFromScreen(Point point, bool reset = false) {
    Point cellKernel = getCellKernelFromScreen(point);
    return getNoteFromKernel(cellKernel, reset);
}

Note* Grid2D::getNoteFromKernel(const Point& cellKernel, bool reset = false) {
    if (!boundsKernelCells.contains(cellKernel))
        throw std::invalid_argument("invalid kernel cell");

    const auto x = static_cast<size_t>(cellKernel.x);
    const auto y = static_cast<size_t>(cellKernel.y);

    if (reset || !kernel[y][x]) {
        const Point cellGrid = getCellGridFromKernel(cellKernel);
        const Fraction ratioToRef = (intervalHorizontal ^ cellGrid.x) * (intervalVertical ^ cellGrid.y);
        kernel[y][x] = std::make_unique<ChildNote>(&noteOrigin, ratioToRef, 0, 0);
    }

    return kernel[y][x].get();
}

Note* Grid2D::getNoteFromGrid(const Point& cellGrid, bool reset = false) {
    Point cellKernel = getCellKernelFromGrid(cellGrid);
    return getNoteFromKernel(cellKernel, reset);
}

juce::Colour Grid2D::getColourForCellKernel(const Point& cellKernel) {
    Note* note = getNoteFromKernel(cellKernel, false);
    Point gridCell = getCellGridFromKernel(cellKernel);
    bool selected = selectedCellsGrid.find(gridCell) != selectedCellsGrid.end();
    return getColourForNote(note, selected);
}

void Grid2D::transposeGrid() {
    Point centerOfGravityOffset = calculateCenterOfGravityOffsetCell();
    Kernel newKernel = createEmptyKernel(dimKernelCells);

    auto widthKernel = static_cast<size_t>(dimKernelCells.x);
    auto heightKernel = static_cast<size_t>(dimKernelCells.y);
    for (size_t y = 0; y < heightKernel; ++y)
        for (size_t x = 0; x < widthKernel; ++x) {
            Point pOld = Point(static_cast<int>(x), static_cast<int>(y)) + centerOfGravityOffset;
            if (boundsKernelCells.contains(pOld)) {
                auto yOld = static_cast<size_t>(pOld.y);
                auto xOld = static_cast<size_t>(pOld.x);
                newKernel[y][x] = std::move(kernel[yOld][xOld]);
            }
        }
    kernel = std::move(newKernel);

    offsetFromOriginGrid += centerOfGravityOffset;
    paintingOffsetPxInitial = (getDimCellPxFloat() * centerOfGravityOffset).roundToInt();

    gridTranspositionAnimator.start();
}

Point Grid2D::mirrorYPx(Point point) const {
    return {point.x, getHeight() - point.y - 1};
}

Rect Grid2D::mirrorYPx(Rect rect) const {
    return {rect.getX(), getHeight() - rect.getY() - 1 - rect.getHeight(), rect.getWidth(), rect.getHeight()};
}

Point Grid2D::calculateCenterOfGravityOffsetCell() const {
    if (activeCellsGrid.empty())
        return {0, 0};

    Point sum;
    for (const auto& active : activeCellsGrid)
        sum += getCellScreenFromGrid(active);
    PointF centerOfGravityFloat = sum.toFloat() / activeCellsGrid.size();
    centerOfGravityFloat += {0.51f, 0.51f};

    PointF centerOfGridCell = dimScreenCells.toFloat()/2;

    Point offset = (centerOfGravityFloat - centerOfGridCell).roundToInt();
    return offset;
}