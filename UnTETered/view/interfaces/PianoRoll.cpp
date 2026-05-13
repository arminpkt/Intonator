//
// Created by Vos on 09/10/2025.
//
#include "PianoRollStateHelpers.h"

#include "PianoRoll.h"

#include "../../logic/util.h"

PianoRoll::PianoRoll(UnTETeredAudioProcessor& proc) : processor(proc) {
    initialisePotentialRatios();
    setWantsKeyboardFocus(true);
    pullStateFromProcessorAndRebuild();
    startTimerHz(30);
}

void PianoRoll::initialisePotentialRatios() {
    potentialRatios = {
        {1, 1},
        {2, 1},
        {3, 2},
        {4, 3},
        {5, 4},
        {5, 3},
        {8, 5},
        {6, 5},
        {7, 4},
        {9, 8},
        {1, 2},
        {2, 3},
        {3, 4},
        {4, 5},
        {3, 5},
        {5, 8},
        {5, 6},
        {4, 7},
        {8, 9},
    };
}

void PianoRoll::paint(juce::Graphics& g) {
    drawBackground(g, getNoteCanvasBounds());
    drawPotentialRatios(g);
    drawBarLines(g, getNoteCanvasBounds(), true);
    drawNotes(g);
    drawRectDragged(g);
    drawOrientationBar(g);
    drawPlayhead(g);
}

void PianoRoll::fillRect(const Rect& rect, juce::Graphics& g) {
    g.fillRect(rect);
}

void PianoRoll::drawRect(const Rect& rect, juce::Graphics& g) {
    g.drawRect(rect);
}

void PianoRoll::drawText(
    const juce::String& text,
    const Rect& bounds,
    const juce::Graphics& g,
    juce::Justification justification = juce::Justification::centredLeft
    ) {
    auto actualBounds = bounds.withTrimmedLeft(4).withTrimmedRight(4).withTrimmedTop(2).withTrimmedBottom(2);
    g.drawFittedText(text, actualBounds, justification, 1);
}

void PianoRoll::drawBackground(juce::Graphics& g, const Rect& bounds) const {
    auto [t, l, b, r, w, h] = getTLBRWH(bounds);

    for (int y = t; y < b; y++) {
        float hue = getHueFromYPx(y);
        auto colour = juce::Colour::fromHSV(hue, 0.2f, 0.7f, 1.0f);
        g.setColour(colour);
        Rect rect = {l, y, w, 1};
        fillRect(rect, g);
    }
}

void PianoRoll::drawBarLines(juce::Graphics& g, const Rect& bounds, bool drawSubDivs) const {
    auto [t, l, b, r, w, h] = getTLBRWH(bounds.toFloat());
    float firstBar = (ceil(barLeftScreen) - barLeftScreen - 1) * static_cast<float>(barWidthPxF);

    // draw subdivision lines
    if (drawSubDivs) {
        g.setColour(juce::Colour::fromRGB(100, 100, 100));
        for (float x = firstBar; x < w; x += barWidthPxF/static_cast<float>(getNrOfSubDivs())) // NOLINT(*-flp30-c)
            g.drawVerticalLine(static_cast<int>(x), t, b);
    }

    // draw bar lines
    g.setColour(juce::Colour::fromRGB(50, 50, 50));
    firstBar = (ceil(barLeftScreen) - barLeftScreen) * static_cast<float>(barWidthPxF);
    for (float x = firstBar; x < w; x += barWidthPxF) // NOLINT(*-flp30-c)
        g.drawVerticalLine(static_cast<int>(x), t - static_cast<float>(ORIENTATION_BAR_HEIGHT), b);
}

Fraction PianoRoll::getSubDivsFraction() const {
    auto num = cachedNumerator > 0 ? cachedNumerator : 4;
    auto subDivs = num * extraGridResolution;
    if (gridTripletted)
        subDivs = subDivs * Fraction{3, 2};
    return subDivs;
}

int PianoRoll::getNrOfSubDivs() const {
    auto f = getSubDivsFraction();
    return f.getNumeratorAndDenominator().first;
}

void PianoRoll::drawNotes(juce::Graphics& g) const {
    for (auto& note : noteRegion.notes) {
        auto baseColour = juce::Colour::fromRGB(100, 100, 100);
        auto edgeColour = juce::Colour::fromRGB(50, 50, 50);
        drawNote(&note, baseColour, edgeColour, g);
    }

    if (notesSelected.size() == 1) {
        auto noteSelected = notesSelected[0];

        // draw family over
        for (auto& note : noteRegion.notes) {
            if (&note == noteSelected || !note.isFamiliarWith(noteSelected))
                continue;

            auto baseColourChild = juce::Colour::fromRGB(50, 50, 200);
            auto edgeColourChild = juce::Colour::fromRGB(50, 50, 50);
            drawNote(&note, baseColourChild, edgeColourChild, g);

            auto boundsChild = getNoteBounds(&note);
            g.setColour(juce::Colour::fromRGB(200, 200, 200));
            auto ratioTotal = note.ratio/noteSelected->ratio;
            juce::String text = ratioTotal.toString();
            drawText(text, boundsChild, g);
        }

        // draw selectedNote over
        auto baseColour = juce::Colour::fromRGB(50, 200, 50);
        auto edgeColour = juce::Colour::fromRGB(50, 50, 50);
        drawNote(noteSelected, baseColour, edgeColour, g);
    }

    else if (notesSelected.size() > 1) {
        auto intRatios = getIntRatios(notesSelected);
        for (size_t i = 0; i < notesSelected.size(); ++i) {
            auto noteSelected = notesSelected[i];
            auto baseColourSelected = juce::Colour::fromRGB(100, 100, 100);
            auto edgeColourSelected = juce::Colour::fromRGB(205, 205, 205);
            drawNote(noteSelected, baseColourSelected, edgeColourSelected, g);
            if (intRatios) {
                auto boundsSelected = getNoteBounds(noteSelected);
                auto intRatio = intRatios.value()[i];
                juce::String text = std::to_string(intRatio);
                g.setColour(juce::Colour::fromRGB(50, 50, 50));
                drawText(text, boundsSelected, g);
            }
        }
    }
}

void PianoRoll::drawNote(const Note* note, const juce::Colour baseColour, const juce::Colour edgeColour, juce::Graphics& g) const {
    Rect noteBounds = getNoteBounds(note);
    auto colour = baseColour;
    if (note == noteHighlighted)
        colour = colour.brighter();
    g.setColour(colour);
    fillRect(noteBounds, g);
    g.setColour(edgeColour);
    drawRect(noteBounds, g);
}

void PianoRoll::drawPotentialRatios(juce::Graphics& g) const {
    if (notesSelected.size() != 1)
        return;
    for (auto& ratio : potentialRatios) {
        auto bounds = getPotentialRatioBounds(ratio).value();
        auto baseColour = juce::Colour::fromRGBA(50, 50, 50, 128);
        if (potentialRatioHighlighted && ratio == potentialRatioHighlighted.value())
            baseColour = baseColour.brighter();
        g.setColour(baseColour);
        fillRect(bounds, g);
        g.setColour(juce::Colour::fromRGB(50, 50, 50));
        juce::String text = ratio.toString();
        drawText(text, bounds, g);
        drawText(text, bounds, g, juce::Justification::centredRight);
    }
}

void PianoRoll::drawRectDragged(juce::Graphics& g) const {
    if (draggedRect) {
        g.setColour(juce::Colour::fromRGBA(50, 50, 50, 50));
        g.fillRect(draggedRect.value());
        g.setColour(juce::Colour::fromRGB(150, 200, 150));
        g.drawRect(draggedRect.value());
    }
}

void PianoRoll::drawOrientationBar(juce::Graphics& g) const {
    auto orientationBarBounds = getOrientationBarBounds();
    drawBackground(g, orientationBarBounds);
    drawBarLines(g, orientationBarBounds, false);

    auto [t, l, b, r, w, h] = getTLBRWH(orientationBarBounds.toFloat());
    g.setColour(juce::Colour::fromRGB(50, 50, 50));
    g.drawHorizontalLine(static_cast<int>(b), l, r);

    int initial = static_cast<int>(std::floor(barLeftScreen));
    int last = static_cast<int>(std::floor(barLeftScreen + w / barWidthPxF));

    for (int bar = initial; bar <= last; ++bar) {
        auto x = getXPxFromBar(static_cast<float>(bar));
        Rect textBounds = orientationBarBounds.withX(x).withWidth(static_cast<int>(barWidthPxF));
        drawText(juce::String(bar + 1), textBounds, g);
    }
}

void PianoRoll::drawPlayhead(juce::Graphics &g) const {
    auto [t, l, b, r, w, h] = getTLBRWH(getLocalBounds());
    auto playheadX = getXPxFromBar(playheadBarPos);
    g.setColour(juce::Colour::fromRGB(255, 255, 255));
    g.drawVerticalLine(playheadX, static_cast<float>(t), static_cast<float>(b));
}


float PianoRoll::getHueFromYPx(int y) const {
    double freq = getFreqFromYPx(y);
    return getHueFromFreq(freq);
}

float PianoRoll::getHueFromFreq(double freq) {
    Note note{freq, {1, 1}, 1, 0, 0};
    return note.getHue();
}

double PianoRoll::getFreqFromYPx(int y, bool ignoreFreqBottomScreen) const {
    return getFreqFromYPxF(static_cast<float>(y), ignoreFreqBottomScreen);
}

double PianoRoll::getFreqFromYPxF(float y, bool ignoreFreqBottomScreen) const {
    float mirrored = mirrorYPx(y);
    double nrOfOctaves = static_cast<double>(mirrored)/static_cast<double>(octaveHeightPxF);
    double freqFactor = std::pow(2, nrOfOctaves);
    if (ignoreFreqBottomScreen)
        return freqFactor;
    return freqFactor * freqBottomScreen;
}

int PianoRoll::getYPxFromFreq(double freq) const {
    double freqFactor = freq / freqBottomScreen;
    double nrOfOctaves = std::log2(freqFactor);
    double yPx = nrOfOctaves * octaveHeightPxF;
    float mirrored = mirrorYPx(static_cast<float>(yPx));
    return static_cast<int>(mirrored);
}

float PianoRoll::getBarExactFromXPx(int x, bool ignoreBarLeftScreen) const {
    return getBarExactFromXPxF(static_cast<float>(x), ignoreBarLeftScreen);
}

float PianoRoll::getBarExactFromXPxF(float x, bool ignoreBarLeftScreen) const {
    float ignoring = x / static_cast<float>(barWidthPxF);
    if (ignoreBarLeftScreen)
        return ignoring;
    return barLeftScreen + ignoring;
}

float PianoRoll::getBarSubFromXPx(int px, bool ignoreBarLeftScreen) const {
    float exact = getBarExactFromXPx(px, ignoreBarLeftScreen);
    auto nrOfSubDivs = static_cast<float>(getNrOfSubDivs());
    return static_cast<float>(static_cast<int>(exact * nrOfSubDivs)) / nrOfSubDivs;
}

float PianoRoll::getBarSubRoundedFromXPx(int px, bool ignoreBarLeftScreen) const {
    float exact = getBarExactFromXPx(px, ignoreBarLeftScreen);
    auto nrOfSubDivs = static_cast<float>(getNrOfSubDivs());
    return std::round(exact * nrOfSubDivs) / nrOfSubDivs;
}

int PianoRoll::getBarFloorFromXPx(int px, bool ignoreBarLeftScreen) const {
    float exact = getBarExactFromXPx(px, ignoreBarLeftScreen);
    return static_cast<int>(exact);
}

int PianoRoll::getXPxFromBar(float bar) const {
    return static_cast<int>((bar - barLeftScreen) * static_cast<float>(barWidthPxF));
}

Note* PianoRoll::getNoteAt(Point px) {
    for (auto& note : noteRegion.notes) {
        Rect bounds = getNoteBounds(&note).expanded(3);
        if (bounds.contains(px))
            return &note;
    }
    return nullptr;
}

std::optional<Fraction> PianoRoll::getPotentialRatioAt(Point px) const {
    if (notesSelected.size() != 1)
        return std::nullopt;
    for (auto& ratio : potentialRatios) {
        auto boundsOpt = getPotentialRatioBounds(ratio);
        if (boundsOpt) {
            Rect bounds = boundsOpt.value().expanded(3);
            if (bounds.contains(px))
                return ratio;
        }
    }
    return std::nullopt;
}

Rect PianoRoll::getNoteBounds(const Note* note) const {
    int startPx = getXPxFromBar(note->start);
    int endPx = getXPxFromBar(note->end);
    int noteWidth = (endPx - startPx);
    int noteHeight = static_cast<int>(NOTE_HEIGHT_PER_OCTAVE*static_cast<float>(octaveHeightPxF));
    int y = getYPxFromFreq(note->getFrequency()) - noteHeight/2;
    Rect rect{startPx, y, noteWidth, noteHeight};
    return rect;
}

std::optional<Rect> PianoRoll::getPotentialRatioBounds(Fraction ratio) const {
    auto [t, l, b, r, w, h] = getTLBRWH(getNoteCanvasBounds());
    if (notesSelected.size() != 1)
        return std::nullopt;
    auto selectedNote = notesSelected[0];
    double freqPotential = selectedNote->getFrequency() * static_cast<double>(ratio);
    int yMid = getYPxFromFreq(freqPotential);
    int noteHeight = static_cast<int>(NOTE_HEIGHT_PER_OCTAVE*static_cast<float>(octaveHeightPxF));
    int y = yMid - noteHeight/2;
    Rect rect{l, y, r, noteHeight};
    return rect;
}

std::vector<double> PianoRoll::getPotentialFrequencies(Note* note) const {
    std::vector<double> potentialFreqs;
    potentialFreqs.reserve(potentialRatios.size());
    for (auto& f : potentialRatios)
        potentialFreqs.push_back(note->getFrequency() * static_cast<double>(f));
    return potentialFreqs;
}

void PianoRoll::selectNote(Note* note, Point clickedPos, bool invertIfSelected = false) {
    if (indexOfSelection(note)) {
        if (invertIfSelected)
            unselectNote(note);
    }
    else
        notesSelected.push_back(note);

    dragLeftSideSelectedNote = false;
    dragRightSideSelectedNote = false;
    if (abs(clickedPos.x-getXPxFromBar(note->end)) < 5)
        dragRightSideSelectedNote = true;
    if (abs(clickedPos.x-getXPxFromBar(note->start)) < 5)
        dragLeftSideSelectedNote = true;
}

std::optional<size_t> PianoRoll::indexOfSelection(const Note* note) const {
    for (size_t i = 0; i < notesSelected.size(); i++)
        if (note == notesSelected[i])
            return i;
    return std::nullopt;
}

void PianoRoll::unselectNote(const Note* note) {
    auto index = indexOfSelection(note);
    if (index)
        notesSelected.erase(notesSelected.begin() + static_cast<long int>(index.value()));
}

float PianoRoll::mirrorYPx(float y, float axis) const {
    return static_cast<float>(getHeight()) * 2 * (1 - axis) - y;
}

int PianoRoll::mirrorYPx(int y, float axis) const {
    return static_cast<int>(mirrorYPx(static_cast<float>(y), axis));
}

Point PianoRoll::mirrorYPx(Point point, float axis) const {
    return {point.x, mirrorYPx(point.getY(), axis)};
}

Rect PianoRoll::mirrorYPx(Rect rect, float axis) const {
    return {rect.getX(), mirrorYPx(rect.getY(), axis) - rect.getHeight(), rect.getWidth(), rect.getHeight()};
}

void PianoRoll::setPlayheadPosFromPoint(Point point) {
    float bar = getBarSubFromXPx(point.x);
    playheadBarPos = bar;
}

void PianoRoll::mouseDown(const juce::MouseEvent& event) {
    Point posPx = event.getPosition();
    int nrOfClicks = (event.getNumberOfClicks()-1)%2+1;
    if (event.mods.isShiftDown() && nrOfClicks == 1)
        handleShiftSingleClick(posPx);
    else if (nrOfClicks == 1)
        handleSingleClick(posPx);
    else if (nrOfClicks == 2)
        handleDoubleClick(posPx);
}

void PianoRoll::mouseDrag(const juce::MouseEvent& event) {
    Point mouseDownPos = event.getMouseDownPosition();
    Point currentPos = event.getPosition();

    // dragging rectangle to select
    if (!clickedNote || event.mods.isShiftDown()) {
        if (!event.mods.isShiftDown())
            notesSelected.clear();
        dragRectangle(mouseDownPos, currentPos);
    }

    // moving/extending/shrinking notes
    else
        moveExtendShrinkNotes(mouseDownPos, currentPos);
}

void PianoRoll::mouseMove(const juce::MouseEvent& event) {
    noteHighlighted = nullptr;
    potentialRatioHighlighted.reset();
    auto position = event.getPosition();
    if (auto noteAt = getNoteAt(position))
        noteHighlighted = noteAt;
    else if (auto potentialRatioAt = getPotentialRatioAt(position))
        potentialRatioHighlighted = potentialRatioAt;
}

void PianoRoll::mouseMagnify(const juce::MouseEvent& event, const float scaleFactor) {
    auto mousePos = event.getPosition();

    if (event.mods.isShiftDown()) {
        auto pxFromTop = mousePos.getY();
        auto pxFromTopFlippedInBottom = mirrorYPx(pxFromTop, 0);
        freqBottomScreen = getFreqFromYPx(pxFromTop);
        octaveHeightPxF *= scaleFactor;
        freqBottomScreen = getFreqFromYPx(pxFromTopFlippedInBottom);
    }
    else {
        barLeftScreen = getBarExactFromXPx(mousePos.getX());
        barWidthPxF *= scaleFactor;
        barLeftScreen = getBarExactFromXPx(-mousePos.getX());
    }
    clipScreenEdges();
    pushStateToProcessor();
}

void PianoRoll::dragRectangle(const Point mouseDownPos, const Point currentPos) {
    draggedRect = Rect{mouseDownPos, currentPos};
    if (draggedRect->getWidth() == 0)
        draggedRect->setWidth(1);
    for (auto& note : noteRegion.notes) {
        Rect boundsNote = getNoteBounds(&note);
        bool selected = false;
        for (const auto& noteSelected : notesSelected)
            if (&note == noteSelected)
                selected = true;
        if (boundsNote.intersects(draggedRect.value()) && !selected)
            notesSelected.push_back(&note);
    }
}

void PianoRoll::moveExtendShrinkNotes(const Point mouseDownPos, const Point currentPos) const {
    moveExtendShrinkHorizontally(currentPos.getX() - mouseDownPos.getX());
    moveVertically(currentPos);
    pushStateToProcessor();
}

void PianoRoll::moveExtendShrinkHorizontally(const int dX) const {
    float dBar = getBarSubRoundedFromXPx(dX, true);
    for (size_t i = 0; i < notesSelected.size(); i++) {
        auto noteSelected = notesSelected[i];
        auto [start, end] = selectedNotesStartsEnds[i];
        if (dragLeftSideSelectedNote)
            noteSelected->start = start + dBar;
        else if (dragRightSideSelectedNote)
            noteSelected->end = end + dBar;
        else {
            noteSelected->start = start + dBar;
            noteSelected->end = end + dBar;
        }
    }
}

void PianoRoll::moveVertically(const Point currentPos) const {
    //TODO
}

void PianoRoll::mouseUp(const juce::MouseEvent& _) {
    dragLeftSideSelectedNote = false;
    dragRightSideSelectedNote = false;
    draggedRect.reset();
    pushStateToProcessor();
}

void PianoRoll::mouseWheelMove(const juce::MouseEvent& _, const juce::MouseWheelDetails& wheel) {
    const PointF deltaXY = {wheel.deltaX, wheel.deltaY};
    scroll(deltaXY);
}

void PianoRoll::scroll(const PointF deltaXY) {
    const PointF scaled = deltaXY * SCROLL_FACTOR;
    const float xPx = -scaled.getX();
    const float yPx = mirrorYPx(scaled.getY());
    const float newBarLeftScreen = getBarExactFromXPxF(xPx);
    const double newFreqBottomScreen = getFreqFromYPxF(yPx);
    barLeftScreen = newBarLeftScreen;
    freqBottomScreen = newFreqBottomScreen;
    clipScreenEdges();
    pushStateToProcessor();
}

void PianoRoll::clipScreenEdges() {
    if (barLeftScreen < 0)
        barLeftScreen = 0;
    if (freqBottomScreen < 10)
        freqBottomScreen = 10;
}

void PianoRoll::handleSingleClick(const Point px) {
    if (auto note = getNoteAt(px)) {
        if (!indexOfSelection(note))
            notesSelected.clear();
        selectNote(note, px);

        dragStartOffsetPx = px.getX()-getXPxFromBar(note->start);
        selectedNotesStartsEnds.clear();
        selectedNotesStartsEnds.reserve(notesSelected.size());
        for (const auto& noteSelected : notesSelected)
            selectedNotesStartsEnds.emplace_back(noteSelected->start, noteSelected->end);
        clickedNote = true;
        return;
    }
    clickedNote = false;

    if (!getPotentialRatioAt(px)) {
        notesSelected.clear();
        return;
    }

    setPlayheadPosFromPoint(px);
}

void PianoRoll::handleDoubleClick(const Point px) {
    auto barSub = getBarSubFromXPx(px.getX());
    auto potentialRatioAt = getPotentialRatioAt(px);
    auto noteAt = getNoteAt(px);

    if (noteAt) {
        deleteNote(noteAt);
        return;
    }

    if (potentialRatioAt) {
        addNoteWithReference(notesSelected[0], potentialRatioAt.value(), 1, barSub, barSub+1);
        return;
    }

    auto freq = getFreqFromYPx(px.getY());
    addNoteWithoutReference(freq, barSub, barSub + 1);
}

void PianoRoll::handleShiftSingleClick(const Point px) {
    if (auto note = getNoteAt(px))
        selectNote(note, px, true);
}

bool PianoRoll::keyPressed(const juce::KeyPress& key) {
    if (key == juce::KeyPress::backspaceKey) {
        deleteSelection();
        return true;
    }

    auto code = static_cast<size_t>(key.getKeyCode());
    if (code >= 65 && code <= 90) {
        if (key.getKeyCode() == 'C' && key.getModifiers().isCommandDown()) {
            copySelectionToClipboard();
            return true;
        }
        if (key.getKeyCode() == 'V' && key.getModifiers().isCommandDown()) {
            pasteClipboard();
            return true;
        }
        if (key.getKeyCode() == 'D' && key.getModifiers().isCommandDown()) {
            copySelectionToClipboard();
            pasteClipboard();
            return true;
        }
    }

    if (code >= 49 && code <= 58) {
        if (key.getKeyCode() == '1' && key.getModifiers().isCommandDown()) {
            narrowGrid();
            return true;
        }
        if (key.getKeyCode() == '2' && key.getModifiers().isCommandDown()) {
            widenGrid();
            return true;
        }
        if (key.getKeyCode() == '3' && key.getModifiers().isCommandDown()) {
            tripletGrid();
            return true;
        }
    }

    return false;
}

void PianoRoll::addNoteWithoutReference(double frequency, float start, float end) {
    noteRegion.addNoteWithoutReference(frequency, start, end);
    pushStateToProcessor();
}

void PianoRoll::addNoteWithReference(Note* ref, Fraction ratio, double irratio, float start, float end) {
    noteRegion.addNoteWithRefNote(ref, ratio, irratio, start, end);
    pushStateToProcessor();
}

void PianoRoll::deleteNote(Note* note) {
    noteRegion.deleteNote(note);
    unselectNote(note);
    pushStateToProcessor();
}

void PianoRoll::deleteSelection() {
    while (!notesSelected.empty())
        deleteNote(notesSelected.back());
}

void PianoRoll::copySelectionToClipboard() {
    clipboard.clear();
    clipboard.reserve(notesSelected.size());
    for (const auto& note : notesSelected) {
        clipboard.push_back(*note);
    }
}

void PianoRoll::pasteClipboard() {
    if (clipboard.empty())
        return;

    auto earliestStart = clipboard.front().start;
    for (const auto& note : clipboard)
        if (note.start < earliestStart)
            earliestStart = note.start;

    auto latestEnd = clipboard.front().end;
    for (const auto& note : clipboard)
        if (note.end > latestEnd)
            latestEnd = note.end;

    for (const auto& note : clipboard) {
        float start = playheadBarPos + note.start - earliestStart;
        float end = start + note.end - note.start;
        noteRegion.addNoteWithRefFreq(note.referenceFrequency, note.ratio, note.irratio, start, end);
    }

    playheadBarPos += latestEnd - earliestStart;

    pushStateToProcessor();
}

void PianoRoll::narrowGrid() {
    if (getSubDivsFraction().getMonzo().primePowers[0] < 7)
        extraGridResolution = extraGridResolution * 2;
}

void PianoRoll::widenGrid() {
    if (getSubDivsFraction().getMonzo().primePowers[0] > 0)
        extraGridResolution = extraGridResolution / 2;
}

void PianoRoll::tripletGrid() {
    if (gridTripletted || getSubDivsFraction().getMonzo().primePowers[0] > 0)
        gridTripletted = !gridTripletted;
}

Rect PianoRoll::getNoteCanvasBounds() const {
    auto bounds = getLocalBounds();
    bounds.removeFromTop(ORIENTATION_BAR_HEIGHT);
    return bounds;
}

Rect PianoRoll::getOrientationBarBounds() const {
    auto bounds = getLocalBounds();
    return bounds.removeFromTop(ORIENTATION_BAR_HEIGHT);
}


void PianoRoll::timerCallback() {
    const auto& transportState = processor.getTransportState();

    cachedPpqPosition = transportState.ppqPosition.load(std::memory_order_relaxed);
    cachedNumerator = transportState.numerator.load(std::memory_order_relaxed);
    cachedDenominator = transportState.denominator.load(std::memory_order_relaxed);

    playheadBarPos = static_cast<float>(TimelineHelpers::ppqToBar(cachedPpqPosition, cachedNumerator, cachedDenominator));

    repaint();
}

void PianoRoll::pullStateFromProcessorAndRebuild() {
    const auto state = processor.getPianoRollState();

    octaveHeightPxF = state.octaveHeightPxF;
    barWidthPxF = state.barWidthPxF;
    freqBottomScreen = state.freqBottomScreen;
    barLeftScreen = state.barLeftScreen;

    noteRegion = makeNoteRegionFromState(state);

    notesSelected.clear();
    draggedRect.reset();
}

void PianoRoll::pushStateToProcessor() const {
    PianoRollState state = makeStateFromNoteRegion(noteRegion);
    state.octaveHeightPxF = octaveHeightPxF;
    state.barWidthPxF = barWidthPxF;
    state.freqBottomScreen = freqBottomScreen;
    state.barLeftScreen = barLeftScreen;

    processor.setPianoRollState(state);
}