//
// Created by Vos on 09/10/2025.
//
#include "PianoRollStateHelpers.h"

#include "PianoRoll.h"

#include "../../logic/util.h"
#include <unordered_set>

PianoRoll::PianoRoll(UnTETeredAudioProcessor& proc)
    : processor(proc),
      settingsBar(PianoRollSettingsBar(
          [this] { handleLockYChanged(); },
          [this] { handleReferenceChanged(); },
          [this] { handlePotentialRatiosChanged(); })) {
    initialisePotentialRatios();
    setWantsKeyboardFocus(true);
    pullStateFromProcessorAndRebuild();
    startTimerHz(30);
    addAndMakeVisible(settingsBar);
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
    settingsBar.setLockY(locked);
    settingsBar.setReference(selectedNote);
    settingsBar.setPotentialRatios(potentialRatios);
}

void PianoRoll::paint(juce::Graphics& g) {
    drawBackground(g, getNoteCanvasBounds());
    drawPotentialRatios(g);
    drawBarLines(g, getNoteCanvasBounds(), true);
    drawNotes(g);
    drawRectDragged(g);
    if (isDraggingPasteCursor)
        drawPasteCursorLine(g);
    drawOrientationBar(g);
    drawPlayhead(g);
    drawPasteCursorHandle(g);
    drawSettingsBackground(g);
}

void PianoRoll::fillRect(const Rect& rect, juce::Graphics& g) {
    g.fillRect(rect);
}

void PianoRoll::drawRect(const Rect& rect, juce::Graphics& g) {
    g.drawRect(rect);
}

void PianoRoll::drawText(const juce::String& text, const Rect& bounds, const juce::Graphics& g,
    juce::Justification justification = juce::Justification::centredLeft) {
    auto actualBounds = bounds.withTrimmedLeft(4).withTrimmedRight(4).withTrimmedTop(2).withTrimmedBottom(2);
    g.drawFittedText(text, actualBounds, justification, 1);
}

void PianoRoll::drawBackground(juce::Graphics& g, const Rect& bounds) const {
    auto [t, l, b, r, w, h] = getTLBRWH(bounds);
    for (int y = t; y < b; y++) {
        float hue = getHueFromYPx(y);
        auto colour = juce::Colour::fromHSV(hue, 0.2f, 0.7f, 1.0f);
        g.setColour(colour);
        fillRect({l, y, w, 1}, g);
    }
}

void PianoRoll::drawBarLines(juce::Graphics& g, const Rect& bounds, bool drawSubDivs) const {
    auto [t, l, b, r, w, h] = getTLBRWH(bounds.toFloat());
    float firstBar = (ceil(barLeftScreen) - barLeftScreen - 1) * static_cast<float>(barWidthPxF);

    if (drawSubDivs) {
        g.setColour(SUB_DIV_LINE_COLOUR);
        for (float x = firstBar; x < w; x += barWidthPxF / static_cast<float>(getNrOfSubDivs())) // NOLINT(*-flp30-c)
            g.drawVerticalLine(static_cast<int>(x), t, b);
    }

    g.setColour(BAR_LINE_COLOUR);
    firstBar = (ceil(barLeftScreen) - barLeftScreen) * static_cast<float>(barWidthPxF);
    for (float x = firstBar; x < w; x += barWidthPxF) // NOLINT(*-flp30-c)
        g.drawVerticalLine(static_cast<int>(x), t, b);
}

Fraction PianoRoll::getSubDivsFraction() const {
    auto num = cachedNumerator > 0 ? cachedNumerator : 4;
    auto subDivs = num * extraGridResolution;
    if (gridTripletted)
        subDivs = subDivs * Fraction{3, 2};
    return subDivs;
}

int PianoRoll::getNrOfSubDivs() const {
    return getSubDivsFraction().getNumeratorAndDenominator().first;
}

void PianoRoll::drawNotes(juce::Graphics& g) const {
    for (auto& note : noteRegion.notes)
        drawNote(note.get(), NOTE_BASE_COLOUR, NOTE_OUTLINE_COLOUR, g);

    if (referenceSetting == lockNote)
        drawNote(lockedNoteReference, SELECTED_BASE_COLOUR.withMultipliedSaturation(.5), SELECTED_OUTLINE_COLOUR, g);

    if (notesSelected.size() == 1) {
        auto noteSelected = notesSelected[0];
        for (auto& note : noteRegion.notes) {
            if (note.get() == noteSelected || !note->isFamiliarWith(noteSelected))
                continue;
            drawNote(note.get(), FAMILY_BASE_COLOUR, FAMILY_OUTLINE_COLOUR, g);
            auto boundsChild = getNoteBounds(note.get());
            g.setColour(FAMILY_RATIO_TEXT_COLOUR);
            drawText((note->ratio / noteSelected->ratio).toString(), boundsChild, g);
        }
        drawNote(noteSelected, SELECTED_BASE_COLOUR, SELECTED_OUTLINE_COLOUR, g);
    }

    if (notesSelected.size() > 1) {
        auto intRatios = getIntRatios(notesSelected);
        for (size_t i = 0; i < notesSelected.size(); ++i) {
            auto noteSelected = notesSelected[i];
            drawNote(noteSelected, MULT_SELECTED_BASE_COLOUR, MULT_SELECTED_OUTLINE_COLOUR, g);
            if (intRatios) {
                g.setColour(INT_RATIO_TEXT_COLOUR);
                drawText(juce::String(std::to_string(intRatios.value()[i])), getNoteBounds(noteSelected), g);
            }
        }
    }
}

void PianoRoll::drawNote(const Note* note, const juce::Colour& baseColour, const juce::Colour& outlineColour, juce::Graphics& g) const {
    Rect noteBounds = getNoteBounds(note);
    auto colour = (note == noteHighlighted) ? baseColour.brighter() : baseColour;
    g.setColour(colour);
    fillRect(noteBounds, g);
    g.setColour(outlineColour);
    drawRect(noteBounds, g);
}

void PianoRoll::drawPotentialRatios(juce::Graphics& g) const {
    if (!referenceExists())
        return;
    for (auto& ratio : potentialRatios) {
        auto bounds = getPotentialRatioBounds(ratio).value();
        auto baseColour = (potentialRatioHighlighted && ratio == potentialRatioHighlighted.value())
                          ? POTENTIAL_RATIO_BASE_COLOUR.brighter()
                          : POTENTIAL_RATIO_BASE_COLOUR;
        g.setColour(baseColour);
        fillRect(bounds, g);
        g.setColour(POTENTIAL_RATIO_TEXT_COLOUR);
        juce::String text = ratio.toString();
        drawText(text, bounds, g);
        drawText(text, bounds, g, juce::Justification::centredRight);
    }
}

void PianoRoll::drawRectDragged(juce::Graphics& g) const {
    if (draggedRect) {
        g.setColour(DRAGGED_RECT_BASE_COLOUR);
        g.fillRect(draggedRect.value());
        g.setColour(DRAGGED_RECT_OUTLINE_COLOUR);
        g.drawRect(draggedRect.value());
    }
}

void PianoRoll::drawOrientationBar(juce::Graphics& g) const {
    auto orientationBarBounds = getOrientationBarBounds();
    drawBackground(g, orientationBarBounds);
    drawBarLines(g, orientationBarBounds, false);

    auto [t, l, b, r, w, h] = getTLBRWH(orientationBarBounds.toFloat());
    g.setColour(BAR_LINE_COLOUR);
    g.drawHorizontalLine(static_cast<int>(b), l, r);

    int initial = static_cast<int>(std::floor(barLeftScreen));
    int last = static_cast<int>(std::floor(barLeftScreen + w / barWidthPxF));
    for (int bar = initial; bar <= last; ++bar) {
        auto x = getXPxFromBar(static_cast<float>(bar));
        Rect textBounds = orientationBarBounds.withX(x).withWidth(static_cast<int>(barWidthPxF));
        drawText(juce::String(bar + 1), textBounds, g);
    }
}

void PianoRoll::drawPlayhead(juce::Graphics& g) const {
    auto [t, l, b, r, w, h] = getTLBRWH(getCanvasBounds());
    g.setColour(PLAYHEAD_COLOUR);
    g.drawVerticalLine(getXPxFromBar(playheadBarPos), static_cast<float>(t), static_cast<float>(b));
}

void PianoRoll::drawPasteCursorHandle(juce::Graphics& g) const {
    auto bar = getOrientationBarBounds();
    auto [t, l, b, r, w, h] = getTLBRWH(bar.toFloat());

    int xPx = getXPxFromBar(pasteCursorBarPos);

    constexpr float halfW = 5.0f;
    constexpr float triH  = 7.0f;
    float tipY  = b;
    float baseY = b - triH;

    juce::Path triangle;
    triangle.startNewSubPath(static_cast<float>(xPx),          tipY);
    triangle.lineTo          (static_cast<float>(xPx) - halfW, baseY);
    triangle.lineTo          (static_cast<float>(xPx) + halfW, baseY);
    triangle.closeSubPath();

    g.setColour(PASTE_CURSOR_COLOUR);
    g.fillPath(triangle);
}

void PianoRoll::drawPasteCursorLine(juce::Graphics& g) const {
    auto [t, l, b, r, w, h] = getTLBRWH(getNoteCanvasBounds());
    int xPx = getXPxFromBar(pasteCursorBarPos);

    float alpha = (std::sin(blinkPhase) + 1.0f) * 0.5f;
    alpha = 0.25f + alpha * 0.75f;

    g.setColour(PASTE_CURSOR_COLOUR.withAlpha(alpha));
    g.drawVerticalLine(xPx, static_cast<float>(t), static_cast<float>(b));
}

void PianoRoll::drawSettingsBackground(juce::Graphics& g) const {
    g.setColour(SETTINGS_BACKGROUND_COLOUR);
    g.fillRect(getSettingsBarBounds());
}

void PianoRoll::resized() {
    settingsBar.setBounds(getSettingsBarBounds());
}

Rect PianoRoll::getCanvasBounds() const {
    auto bounds = getLocalBounds();
    bounds.removeFromTop(SETTINGS_BAR_HEIGHT);
    return bounds;
}

Rect PianoRoll::getSettingsBarBounds() const {
    return getLocalBounds().removeFromTop(SETTINGS_BAR_HEIGHT);
}

Rect PianoRoll::getNoteCanvasBounds() const {
    auto bounds = getCanvasBounds();
    bounds.removeFromTop(ORIENTATION_BAR_HEIGHT);
    return bounds;
}

Rect PianoRoll::getOrientationBarBounds() const {
    return getCanvasBounds().removeFromTop(ORIENTATION_BAR_HEIGHT);
}

float PianoRoll::getHueFromYPx(int y) const {
    return getHueFromFreq(getFreqFromYPx(y));
}

float PianoRoll::getHueFromFreq(double freq) {
    Note note{freq, {1, 1}, 1, 0, 0};
    return note.getHue();
}

double PianoRoll::getFreqFromYPx(int y) const {
    return getFreqFromYPxF(static_cast<float>(y));
}

double PianoRoll::getFreqFromYPxF(float y) const {
    float mirrored = mirrorYPx(y);
    double nrOfOctaves = static_cast<double>(mirrored) / static_cast<double>(octaveHeightPxF);
    return std::pow(2.0, nrOfOctaves) * freqBottomScreen;
}

int PianoRoll::getYPxFromFreq(double freq) const {
    double nrOfOctaves = std::log2(freq / freqBottomScreen);
    return static_cast<int>(mirrorYPx(static_cast<float>(nrOfOctaves * octaveHeightPxF)));
}

float PianoRoll::getBarExactFromXPx(int x, bool ignoreBarLeftScreen) const {
    return getBarExactFromXPxF(static_cast<float>(x), ignoreBarLeftScreen);
}

float PianoRoll::getBarExactFromXPxF(float x, bool ignoreBarLeftScreen) const {
    float raw = x / static_cast<float>(barWidthPxF);
    return ignoreBarLeftScreen ? raw : barLeftScreen + raw;
}

float PianoRoll::getBarSubFromXPx(int px, bool ignoreBarLeftScreen) const {
    float exact = getBarExactFromXPx(px, ignoreBarLeftScreen);
    auto n = static_cast<float>(getNrOfSubDivs());
    return static_cast<float>(static_cast<int>(exact * n)) / n;
}

float PianoRoll::getBarSubRoundedFromXPx(int px, bool ignoreBarLeftScreen) const {
    float exact = getBarExactFromXPx(px, ignoreBarLeftScreen);
    auto n = static_cast<float>(getNrOfSubDivs());
    return std::round(exact * n) / n;
}

int PianoRoll::getBarFloorFromXPx(int px, bool ignoreBarLeftScreen) const {
    return static_cast<int>(getBarExactFromXPx(px, ignoreBarLeftScreen));
}

int PianoRoll::getXPxFromBar(float bar) const {
    return static_cast<int>((bar - barLeftScreen) * static_cast<float>(barWidthPxF));
}

float PianoRoll::mirrorYPx(float y, float axis) const {
    return static_cast<float>(getHeight()) * 2.0f * (1.0f - axis) - y;
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

Note* PianoRoll::getNoteAt(Point px) {
    for (auto& note : noteRegion.notes)
        if (getNoteBounds(note.get()).expanded(3).contains(px))
            return note.get();
    return nullptr;
}

bool PianoRoll::referenceExists() const {
    return getReferenceRefFreqRatioIrratio().has_value();
}

std::optional<Fraction> PianoRoll::getPotentialRatioAt(Point px) const {
    if (!referenceExists())
        return std::nullopt;
    for (auto& ratio : potentialRatios) {
        auto boundsOpt = getPotentialRatioBounds(ratio);
        if (boundsOpt && boundsOpt.value().expanded(3).contains(px))
            return ratio;
    }
    return std::nullopt;
}

Rect PianoRoll::getNoteBounds(const Note* note) const {
    int startPx = getXPxFromBar(note->start);
    int endPx   = getXPxFromBar(note->end);
    int noteH   = static_cast<int>(NOTE_HEIGHT_PER_OCTAVE * octaveHeightPxF);
    int y       = getYPxFromFreq(note->getFrequency()) - noteH / 2;
    return {startPx, y, endPx - startPx, noteH};
}

std::optional<Rect> PianoRoll::getPotentialRatioBounds(Fraction ratio) const {
    if (!referenceExists())
        return std::nullopt;
    auto [t, l, b, r, w, h] = getTLBRWH(getNoteCanvasBounds());
    double freqPotential = getReferenceFrequency().value() * static_cast<double>(ratio);
    int noteH = static_cast<int>(NOTE_HEIGHT_PER_OCTAVE * octaveHeightPxF);
    int y = getYPxFromFreq(freqPotential) - noteH / 2;
    return Rect{l, y, r, noteH};
}

std::optional<std::tuple<double, Fraction, double>> PianoRoll::getReferenceRefFreqRatioIrratio() const {
    if (referenceSetting == lockNote)
        return std::make_tuple(lockedNoteReference->referenceFrequency,
                               lockedNoteReference->ratio,
                               lockedNoteReference->irratio);
    if (referenceSetting == customRef) {
        auto [f, r, ir] = customReference.value();
        return std::make_tuple(f, r, ir);
    }
    if (notesSelected.size() == 1) {
        auto* n = notesSelected[0];
        return std::make_tuple(n->referenceFrequency, n->ratio, n->irratio);
    }
    return {};
}

std::optional<double> PianoRoll::getReferenceFrequency() const {
    auto v = getReferenceRefFreqRatioIrratio();
    if (!v) return {};
    auto [refFreq, ratio, irratio] = *v;
    return refFreq * static_cast<double>(ratio) * irratio;
}

std::vector<double> PianoRoll::getPotentialFrequencies(Note* note) const {
    std::vector<double> out;
    out.reserve(potentialRatios.size());
    for (auto& f : potentialRatios)
        out.push_back(note->getFrequency() * static_cast<double>(f));
    return out;
}

void PianoRoll::selectNote(Note* note, Point clickedPos, bool invertIfSelected) {
    if (indexOfSelection(note)) {
        if (invertIfSelected) unselectNote(note);
    } else {
        notesSelected.push_back(note);
    }
    dragLeftSideSelectedNote  = std::abs(clickedPos.x - getXPxFromBar(note->start)) < 5;
    dragRightSideSelectedNote = std::abs(clickedPos.x - getXPxFromBar(note->end))   < 5;
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
        notesSelected.erase(notesSelected.begin() + static_cast<long>(index.value()));
}

// =============================================================================
// Mouse events
// =============================================================================

void PianoRoll::mouseDown(const juce::MouseEvent& event) {
    Point posPx = event.getPosition();

    if (getOrientationBarBounds().contains(posPx)) {
        pasteCursorBarPos = getBarSubRoundedFromXPx(posPx.getX());
        isDraggingPasteCursor = true;
        return;
    }

    int nrOfClicks = (event.getNumberOfClicks() - 1) % 2 + 1;
    if (event.mods.isShiftDown() && nrOfClicks == 1)
        handleShiftSingleClick(posPx);
    else if (nrOfClicks == 1)
        handleSingleClick(posPx);
    else if (nrOfClicks == 2)
        handleDoubleClick(posPx);
}

void PianoRoll::mouseDrag(const juce::MouseEvent& event) {
    Point currentPos = event.getPosition();

    if (isDraggingPasteCursor) {
        pasteCursorBarPos = getBarSubRoundedFromXPx(currentPos.getX());
        pasteCursorBarPos = std::max(0.0f, pasteCursorBarPos);
        return;
    }

    Point mouseDownPos = event.getMouseDownPosition();

    if (!noteClicked || event.mods.isShiftDown()) {
        if (!event.mods.isShiftDown())
            notesSelected.clear();
        dragRectangle(mouseDownPos, currentPos);
    } else {
        moveExtendShrinkNotes(mouseDownPos, currentPos);
    }
}

void PianoRoll::mouseMove(const juce::MouseEvent& event) {
    noteHighlighted = nullptr;
    potentialRatioHighlighted.reset();
    auto position = event.getPosition();
    if (auto* noteAt = getNoteAt(position))
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
    } else {
        barLeftScreen = getBarExactFromXPx(mousePos.getX());
        barWidthPxF  *= scaleFactor;
        barLeftScreen = getBarExactFromXPx(-mousePos.getX());
    }
    clipScreenEdges();
    pushViewportToProcessor();
}

void PianoRoll::mouseUp(const juce::MouseEvent& _) {
    if (isDraggingPasteCursor) {
        isDraggingPasteCursor = false;
        return;
    }

    dragLeftSideSelectedNote  = false;
    dragRightSideSelectedNote = false;
    draggedRect.reset();

    if (noteWasDraggedThisGesture) {
        pushNoteStateToProcessor();
        noteWasDraggedThisGesture = false;
    }

    undoSnapshotTakenForCurrentDrag = false;
}

void PianoRoll::mouseWheelMove(const juce::MouseEvent& _, const juce::MouseWheelDetails& wheel) {
    scroll({wheel.deltaX, wheel.deltaY});
}

void PianoRoll::scroll(const PointF deltaXY) {
    const PointF scaled = deltaXY * SCROLL_FACTOR;
    barLeftScreen    = getBarExactFromXPxF(-scaled.getX());
    freqBottomScreen = getFreqFromYPxF(mirrorYPx(scaled.getY()));
    clipScreenEdges();
    pushViewportToProcessor();
}

void PianoRoll::clipScreenEdges() {
    if (barLeftScreen   < 0.0f) barLeftScreen   = 0.0f;
    if (freqBottomScreen < 10.0) freqBottomScreen = 10.0;
}

// =============================================================================
// Click handlers
// =============================================================================

void PianoRoll::handleSingleClick(const Point px) {
    if (auto* note = getNoteAt(px)) {
        if (!indexOfSelection(note))
            notesSelected.clear();
        selectNote(note, px);

        dragStartOffsetPx = px.getX() - getXPxFromBar(note->start);
        selectedNotesStartsEnds.clear();
        for (const auto& n : notesSelected)
            selectedNotesStartsEnds.emplace_back(n->start, n->end);
        selectedNotesRefFreqs.clear();
        for (const auto& n : notesSelected)
            selectedNotesRefFreqs.push_back(n->referenceFrequency);

        noteClicked = note;
        undoSnapshotTakenForCurrentDrag = false;
        noteWasDraggedThisGesture       = false;
        return;
    }

    noteClicked = nullptr;
    notesSelected.clear();
    pasteCursorBarPos = getBarSubFromXPx(px.getX());
}

void PianoRoll::handleDoubleClick(const Point px) {
    auto barSub           = getBarSubFromXPx(px.getX());
    auto potentialRatioAt = getPotentialRatioAt(px);
    auto* noteAt          = getNoteAt(px);

    if (noteAt) {
        deleteNote(noteAt);
        return;
    }

    if (potentialRatioAt) {
        auto [refFreq, ratio, irratio] = getReferenceRefFreqRatioIrratio().value();
        addNoteWithRefFreq(refFreq, ratio * potentialRatioAt.value(), irratio, barSub, barSub + 1);
        return;
    }

    addNoteWithoutReference(getFreqFromYPx(px.getY()), barSub, barSub + 1);
}

void PianoRoll::handleShiftSingleClick(const Point px) {
    if (auto* note = getNoteAt(px))
        selectNote(note, px, true);
}

void PianoRoll::dragRectangle(const Point mouseDownPos, const Point currentPos) {
    draggedRect = Rect{mouseDownPos, currentPos};
    if (draggedRect->getWidth() == 0)
        draggedRect->setWidth(1);
    for (auto& note : noteRegion.notes) {
        bool alreadySelected = indexOfSelection(note.get()).has_value();
        if (!alreadySelected && getNoteBounds(note.get()).intersects(draggedRect.value()))
            notesSelected.push_back(note.get());
    }
}

void PianoRoll::moveExtendShrinkNotes(const Point mouseDownPos, const Point currentPos) {
    if (!undoSnapshotTakenForCurrentDrag) {
        pushUndoSnapshot();
        undoSnapshotTakenForCurrentDrag = true;
    }
    moveExtendShrinkHorizontally(currentPos.getX() - mouseDownPos.getX());
    moveVertically(currentPos, mouseDownPos);
    noteWasDraggedThisGesture = true;
}

void PianoRoll::moveExtendShrinkHorizontally(const int dX) const {
    float dBar = getBarSubRoundedFromXPx(dX, true);
    for (size_t i = 0; i < notesSelected.size(); i++) {
        auto* n = notesSelected[i];
        auto [start, end] = selectedNotesStartsEnds[i];
        if (dragLeftSideSelectedNote)       n->start = start + dBar;
        else if (dragRightSideSelectedNote) n->end   = end   + dBar;
        else { n->start = start + dBar; n->end = end + dBar; }
    }
}

void PianoRoll::moveVertically(const Point currentPos, const Point mouseDownPos) const {
    if (lockYSetting == continuous) {
        double freqFactor = getFreqFromYPx(currentPos.getY()) / getFreqFromYPx(mouseDownPos.getY());
        for (size_t i = 0; i < notesSelected.size(); i++)
            notesSelected[i]->referenceFrequency = selectedNotesRefFreqs[i] * freqFactor;
        return;
    }

    if (lockYSetting == snap) {
        if (referenceSetting == selectedNote) return;
        if (referenceSetting == lockNote)
            for (auto* s : notesSelected)
                if (s == lockedNoteReference) return;

        auto potentialRatio = getPotentialRatioAt(currentPos);
        if (!potentialRatio) return;

        auto [refFreq, ratio, irratio] = getReferenceRefFreqRatioIrratio().value();
        double   refFreqFactor = refFreq / noteClicked->referenceFrequency;
        Fraction ratioFactor   = ratio   / noteClicked->ratio;
        double   irratioFactor = irratio / noteClicked->irratio;

        for (auto* note : notesSelected) {
            note->referenceFrequency *= refFreqFactor;
            note->ratio = note->ratio * ratioFactor * potentialRatio.value();
            note->irratio *= irratioFactor;
        }
    }
}

bool PianoRoll::keyPressed(const juce::KeyPress& key) {
    auto code = static_cast<size_t>(key.getKeyCode());

    if (code == 'Z' && key.getModifiers().isCommandDown()) {
        if (key.getModifiers().isShiftDown()) redo(); else undo();
        return true;
    }

    if (key == juce::KeyPress::backspaceKey) {
        deleteSelection();
        return true;
    }

    if (code == static_cast<size_t>(juce::KeyPress::upKey) && key.getModifiers().isAltDown()) {
        pushUndoSnapshot();
        for (auto* n : notesSelected) n->ratio = n->ratio * 2;
        pushNoteStateToProcessor();
        return true;
    }
    if (code == static_cast<size_t>(juce::KeyPress::downKey) && key.getModifiers().isAltDown()) {
        pushUndoSnapshot();
        for (auto* n : notesSelected) n->ratio = n->ratio / 2;
        pushNoteStateToProcessor();
        return true;
    }

    if (code == 'X' && key.getModifiers().isCommandDown()) { cutSelection();             return true; }
    if (code == 'C' && key.getModifiers().isCommandDown()) { copySelectionToClipboard(); return true; }
    if (code == 'V' && key.getModifiers().isCommandDown()) { pasteClipboard();           return true; }
    if (code == 'D' && key.getModifiers().isCommandDown()) { duplicate();                return true; }
    if (code == 'A' && key.getModifiers().isCommandDown()) { selectAll();                return true; }
    if (code == '1' && key.getModifiers().isCommandDown()) { narrowGrid();               return true; }
    if (code == '2' && key.getModifiers().isCommandDown()) { widenGrid();                return true; }
    if (code == '3' && key.getModifiers().isCommandDown()) { tripletGrid();              return true; }
    if (code == 'L') { incrementLockYSetting();    return true; }
    if (code == 'R') { incrementReferenceSetting(); return true; }

    return false;
}

void PianoRoll::incrementLockYSetting() {
    settingsBar.setLockY(static_cast<LockY>((lockYSetting + 1) % 3));
}

void PianoRoll::incrementReferenceSetting() {
    if (referenceSetting == lockNote && notesSelected.size() == 1 && notesSelected[0] != lockedNoteReference) {
        lockedNoteReference = notesSelected[0];
        return;
    }
    settingsBar.setReference(static_cast<Reference>((referenceSetting + 1) % 2));
}

void PianoRoll::addNoteWithoutReference(double frequency, float start, float end) {
    pushUndoSnapshot();
    noteRegion.addNoteWithoutReference(frequency, start, end);
    pushNoteStateToProcessor();
}

void PianoRoll::addNoteWithRefFreq(double refFreq, Fraction ratio, double irratio, float start, float end) {
    pushUndoSnapshot();
    noteRegion.addNoteWithRefFreq(refFreq, ratio, irratio, start, end);
    pushNoteStateToProcessor();
}

void PianoRoll::deleteNote(Note* note) {
    pushUndoSnapshot();
    noteRegion.deleteNote(note);
    unselectNote(note);
    if (note == lockedNoteReference && referenceSetting == lockNote)
        settingsBar.setReference(selectedNote);
    pushNoteStateToProcessor();
}

void PianoRoll::deleteSelection() {
    if (notesSelected.empty()) return;
    pushUndoSnapshot();
    while (!notesSelected.empty()) {
        auto* note = notesSelected.back();
        noteRegion.deleteNote(note);
        if (note == lockedNoteReference && referenceSetting == lockNote)
            settingsBar.setReference(selectedNote);
        notesSelected.pop_back();
    }
    pushNoteStateToProcessor();
}

void PianoRoll::copySelectionToClipboard() {
    clipboard.clear();
    for (const auto* note : notesSelected)
        clipboard.push_back(std::make_unique<Note>(
            note->referenceFrequency, note->ratio, note->irratio, note->start, note->end));
}

void PianoRoll::cutSelection() {
    if (notesSelected.empty()) return;
    pushUndoSnapshot();
    copySelectionToClipboard();
    while (!notesSelected.empty()) {
        auto* note = notesSelected.back();
        noteRegion.deleteNote(note);
        if (note == lockedNoteReference && referenceSetting == lockNote)
            settingsBar.setReference(selectedNote);
        notesSelected.pop_back();
    }
    pushNoteStateToProcessor();
}

void PianoRoll::pasteClipboard() {
    if (clipboard.empty()) return;

    float earliestStart = clipboard.front()->start;
    for (const auto& n : clipboard)
        if (n->start < earliestStart) earliestStart = n->start;

    float latestEnd = clipboard.front()->end;
    for (const auto& n : clipboard)
        if (n->end > latestEnd) latestEnd = n->end;

    pushUndoSnapshot();
    notesSelected.clear();

    for (const auto& n : clipboard) {
        float start = pasteCursorBarPos + n->start - earliestStart;
        float end   = start + n->end - n->start;
        noteRegion.addNoteWithRefFreq(n->referenceFrequency, n->ratio, n->irratio, start, end);
        notesSelected.push_back(noteRegion.notes.back().get());
    }

    pasteCursorBarPos += latestEnd - earliestStart;
    pushNoteStateToProcessor();
}

void PianoRoll::duplicate() {
    if (notesSelected.empty()) return;

    float earliestStart = notesSelected.front()->start;
    for (const auto* n : notesSelected)
        if (n->start < earliestStart) earliestStart = n->start;

    float latestEnd = notesSelected.front()->end;
    for (const auto* n : notesSelected)
        if (n->end > latestEnd) latestEnd = n->end;

    pushUndoSnapshot();

    std::vector<Note*> originalSelection = notesSelected;

    std::unordered_set<Note*> existingNotes;
    for (const auto& n : noteRegion.notes)
        existingNotes.insert(n.get());

    for (const auto* n : originalSelection) {
        float start = latestEnd + n->start - earliestStart;
        float end   = start + n->end - n->start;
        noteRegion.addNoteWithRefFreq(n->referenceFrequency, n->ratio, n->irratio, start, end);
    }

    notesSelected.clear();
    for (const auto& n : noteRegion.notes)
        if (existingNotes.find(n.get()) == existingNotes.end())
            notesSelected.push_back(n.get());

    pushNoteStateToProcessor();
}

void PianoRoll::selectAll() {
    notesSelected.clear();
    for (auto& note : noteRegion.notes)
        notesSelected.push_back(note.get());
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

// =============================================================================
// Settings bar handlers
// =============================================================================

void PianoRoll::handleLockYChanged() {
    lockYSetting = settingsBar.getLockY();
}

void PianoRoll::handleReferenceChanged() {
    switch (settingsBar.getReference()) {
        case selectedNote: break;
        case lockNote:
            if (notesSelected.size() == 1) lockedNoteReference = notesSelected[0];
            else settingsBar.setReference(selectedNote);
            break;
        case customRef:
            throw std::invalid_argument("not implemented");
    }
    referenceSetting = settingsBar.getReference();
}

void PianoRoll::handlePotentialRatiosChanged() {
    potentialRatios = settingsBar.getPotentialRatios();
}

void PianoRoll::timerCallback() {
    const auto& ts = processor.getTransportState();

    cachedPpqPosition = ts.ppqPosition.load(std::memory_order_relaxed);
    cachedNumerator   = ts.numerator.load(std::memory_order_relaxed);
    cachedDenominator = ts.denominator.load(std::memory_order_relaxed);

    playheadBarPos = static_cast<float>(
        TimelineHelpers::ppqToBar(cachedPpqPosition, cachedNumerator, cachedDenominator));

    blinkPhase += juce::MathConstants<float>::pi / 22.5f;
    if (blinkPhase > juce::MathConstants<float>::twoPi)
        blinkPhase -= juce::MathConstants<float>::twoPi;

    repaint();
}

void PianoRoll::pullStateFromProcessorAndRebuild() {
    const auto state = processor.getPianoRollState();
    octaveHeightPxF  = state.octaveHeightPxF;
    barWidthPxF      = state.barWidthPxF;
    freqBottomScreen = state.freqBottomScreen;
    barLeftScreen    = state.barLeftScreen;
    noteRegion       = makeNoteRegionFromState(state);
    notesSelected.clear();
    draggedRect.reset();
}

void PianoRoll::pushNoteStateToProcessor() const {
    PianoRollState state   = makeStateFromNoteRegion(noteRegion);
    state.octaveHeightPxF  = octaveHeightPxF;
    state.barWidthPxF      = barWidthPxF;
    state.freqBottomScreen = freqBottomScreen;
    state.barLeftScreen    = barLeftScreen;
    processor.setPianoRollState(state);
}

void PianoRoll::pushViewportToProcessor() const {
    processor.setPianoRollViewState(octaveHeightPxF, barWidthPxF,
                                    freqBottomScreen, barLeftScreen);
}

// =============================================================================
// Undo / Redo
// =============================================================================

void PianoRoll::pushUndoSnapshot() {
    undoStack.push_back(makeStateFromNoteRegion(noteRegion).notes);
    if (undoStack.size() > static_cast<size_t>(MAX_UNDO_STEPS))
        undoStack.erase(undoStack.begin());
    redoStack.clear();
}

void PianoRoll::undo() {
    if (undoStack.empty()) return;
    redoStack.push_back(makeStateFromNoteRegion(noteRegion).notes);
    PianoRollState state;
    state.notes            = std::move(undoStack.back());
    state.octaveHeightPxF  = octaveHeightPxF;
    state.barWidthPxF      = barWidthPxF;
    state.freqBottomScreen = freqBottomScreen;
    state.barLeftScreen    = barLeftScreen;
    undoStack.pop_back();
    noteRegion = makeNoteRegionFromState(state);
    notesSelected.clear();
    pushNoteStateToProcessor();
}

void PianoRoll::redo() {
    if (redoStack.empty()) return;
    undoStack.push_back(makeStateFromNoteRegion(noteRegion).notes);
    PianoRollState state;
    state.notes            = std::move(redoStack.back());
    state.octaveHeightPxF  = octaveHeightPxF;
    state.barWidthPxF      = barWidthPxF;
    state.freqBottomScreen = freqBottomScreen;
    state.barLeftScreen    = barLeftScreen;
    redoStack.pop_back();
    noteRegion = makeNoteRegionFromState(state);
    notesSelected.clear();
    pushNoteStateToProcessor();
}