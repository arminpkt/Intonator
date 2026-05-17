//
// Created by Vos on 09/10/2025.
//
#include "PianoRollStateHelpers.h"

#include "PianoRoll.h"

#include "../../logic/util.h"

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
    drawOrientationBar(g);
    drawPlayhead(g);
    drawSettingsBackground(g);
}

void PianoRoll::fillRect(const Rect& rect, juce::Graphics& g) {
    g.fillRect(rect);
}

void PianoRoll::drawRect(const Rect& rect, juce::Graphics& g) {
    g.drawRect(rect);
}

void PianoRoll::drawText(const juce::String& text, const Rect& bounds, const juce::Graphics& g,
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
        g.setColour(SUB_DIV_LINE_COLOUR);
        for (float x = firstBar; x < w; x += barWidthPxF / static_cast<float>(getNrOfSubDivs())) // NOLINT(*-flp30-c)
            g.drawVerticalLine(static_cast<int>(x), t, b);
    }

    // draw bar lines
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
    auto f = getSubDivsFraction();
    return f.getNumeratorAndDenominator().first;
}

void PianoRoll::drawNotes(juce::Graphics& g) const {
    for (auto& note : noteRegion.notes)
        drawNote(note.get(), NOTE_BASE_COLOUR, NOTE_OUTLINE_COLOUR, g);

    // draw locked reference note over
    if (referenceSetting == lockNote)
        drawNote(lockedNoteReference, SELECTED_BASE_COLOUR.withMultipliedSaturation(.5), SELECTED_OUTLINE_COLOUR, g);

    // draw selected note and family over
    if (notesSelected.size() == 1) {
        auto noteSelected = notesSelected[0];

        // draw family over
        for (auto& note : noteRegion.notes) {
            if (note.get() == noteSelected || !note->isFamiliarWith(noteSelected))
                continue;

            drawNote(note.get(), FAMILY_BASE_COLOUR, FAMILY_OUTLINE_COLOUR, g);

            auto boundsChild = getNoteBounds(note.get());
            g.setColour(FAMILY_RATIO_TEXT_COLOUR);
            auto ratioTotal = note->ratio / noteSelected->ratio;
            juce::String text = ratioTotal.toString();
            drawText(text, boundsChild, g);
        }

        // draw selectedNote over
        drawNote(noteSelected, SELECTED_BASE_COLOUR, SELECTED_OUTLINE_COLOUR, g);
    }

    // draw multiple selected notes over
    if (notesSelected.size() > 1) {
        auto intRatios = getIntRatios(notesSelected);
        for (size_t i = 0; i < notesSelected.size(); ++i) {
            auto noteSelected = notesSelected[i];
            drawNote(noteSelected, MULT_SELECTED_BASE_COLOUR, MULT_SELECTED_OUTLINE_COLOUR, g);
            if (intRatios) {
                auto boundsSelected = getNoteBounds(noteSelected);
                auto intRatio = intRatios.value()[i];
                juce::String text = std::to_string(intRatio);
                g.setColour(INT_RATIO_TEXT_COLOUR);
                drawText(text, boundsSelected, g);
            }
        }
    }
}

void PianoRoll::drawNote(const Note* note, const juce::Colour& baseColour, const juce::Colour& outlineColour, juce::Graphics& g) const {
    Rect noteBounds = getNoteBounds(note);
    auto colour = baseColour;
    if (note == noteHighlighted)
        colour = colour.brighter();
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
        auto baseColour = POTENTIAL_RATIO_BASE_COLOUR;
        if (potentialRatioHighlighted && ratio == potentialRatioHighlighted.value())
            baseColour = baseColour.brighter();
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

void PianoRoll::drawPlayhead(juce::Graphics &g) const {
    auto [t, l, b, r, w, h] = getTLBRWH(getCanvasBounds());
    auto playheadX = getXPxFromBar(playheadBarPos);
    g.setColour(PLAYHEAD_COLOUR);
    g.drawVerticalLine(playheadX, static_cast<float>(t), static_cast<float>(b));
}

void PianoRoll::drawSettingsBackground(juce::Graphics &g) const {
    g.setColour(SETTINGS_BACKGROUND_COLOUR);
    g.fillRect(getSettingsBarBounds());
}

void PianoRoll::resized() {
    settingsBar.setBounds(getSettingsBarBounds());
}

float PianoRoll::getHueFromYPx(int y) const {
    double freq = getFreqFromYPx(y);
    return getHueFromFreq(freq);
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
    double nrOfOctaves = static_cast<double>(mirrored)/static_cast<double>(octaveHeightPxF);
    double freqFactor = std::pow(2, nrOfOctaves);
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
        Rect bounds = getNoteBounds(note.get()).expanded(3);
        if (bounds.contains(px))
            return note.get();
    }
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
    if (!referenceExists())
        return std::nullopt;

    auto [t, l, b, r, w, h] = getTLBRWH(getNoteCanvasBounds());

    auto refFreq = getReferenceFrequency().value();

    double freqPotential = refFreq * static_cast<double>(ratio);
    int yMid = getYPxFromFreq(freqPotential);
    int noteHeight = static_cast<int>(NOTE_HEIGHT_PER_OCTAVE*static_cast<float>(octaveHeightPxF));
    int y = yMid - noteHeight/2;
    Rect rect{l, y, r, noteHeight};
    return rect;
}

std::optional<std::tuple<double, Fraction, double>> PianoRoll::getReferenceRefFreqRatioIrratio() const {
        if (referenceSetting == lockNote)
            return std::make_tuple(
                lockedNoteReference->referenceFrequency,
                lockedNoteReference->ratio,
                lockedNoteReference->irratio
                );
        if (referenceSetting == customRef) {
            auto [lockedRefFreq, lockedRatio, lockedIrratio] = customReference.value();
            return std::make_tuple(lockedRefFreq, lockedRatio, lockedIrratio);
        }
        if (notesSelected.size() == 1) {
            auto noteSelected = notesSelected[0];
            return std::make_tuple(
                noteSelected->referenceFrequency,
                noteSelected->ratio,
                noteSelected->irratio
                );
        }
    return {};
}

std::optional<double> PianoRoll::getReferenceFrequency() const {
    auto refFreqRatioIrratio = getReferenceRefFreqRatioIrratio();
    if (!refFreqRatioIrratio)
        return {};
    auto [refFreq, ratio, irratio] = refFreqRatioIrratio.value();
    return refFreq * static_cast<double>(ratio) * irratio;
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
    if (!noteClicked || event.mods.isShiftDown()) {
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
        Rect boundsNote = getNoteBounds(note.get());
        bool selected = false;
        for (const auto& noteSelected : notesSelected)
            if (note.get() == noteSelected)
                selected = true;
        if (boundsNote.intersects(draggedRect.value()) && !selected)
            notesSelected.push_back(note.get());
    }
}

void PianoRoll::moveExtendShrinkNotes(const Point mouseDownPos, const Point currentPos) const {
    moveExtendShrinkHorizontally(currentPos.getX() - mouseDownPos.getX());
    moveVertically(currentPos, mouseDownPos);
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

void PianoRoll::moveVertically(const Point currentPos, const Point mouseDownPos) const {
    if (lockYSetting == continuous) {
        double freqFactor = getFreqFromYPx(currentPos.getY()) / getFreqFromYPx(mouseDownPos.getY());
        for (size_t i = 0; i < notesSelected.size(); i++) {
            auto noteSelected = notesSelected[i];
            double refFreq = selectedNotesRefFreqs[i];
            noteSelected->referenceFrequency = refFreq * freqFactor;
        }
        return;
    }

    // Snapping only makes sense if the note in reference to which we snap isn't being moved
    if (lockYSetting == snap) {
        // If the reference IS the selected note, of course this doesn't work
        if (referenceSetting == selectedNote)
            return;

        // If the reference is a locked note, we have to check that it's not in our selection
        if (referenceSetting == lockNote)
            for (auto& selected : notesSelected)
                if (selected == lockedNoteReference)
                    return;

        auto potentialRatio = getPotentialRatioAt(currentPos);
        if (!potentialRatio)
            return;
        auto [refFreq, ratio, irratio] = getReferenceRefFreqRatioIrratio().value();
        double refFreqFactor = refFreq / noteClicked->referenceFrequency;
        Fraction ratioFactor = ratio / noteClicked->ratio;
        double irratioFactor = irratio / noteClicked->irratio;

        for (auto& note : notesSelected) {
            note->referenceFrequency *= refFreqFactor;
            note->ratio = note->ratio * ratioFactor * potentialRatio.value();
            note->irratio = note->irratio * irratioFactor;
        }
    }
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

        selectedNotesRefFreqs.clear();
        selectedNotesRefFreqs.reserve(notesSelected.size());
        for (const auto& noteSelected : notesSelected)
            selectedNotesRefFreqs.push_back(noteSelected->referenceFrequency);

        noteClicked = note;
        return;
    }
    noteClicked = nullptr;

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
        auto [refFreq, ratio, irratio] = getReferenceRefFreqRatioIrratio().value();
        addNoteWithRefFreq(refFreq, ratio * potentialRatioAt.value(), irratio, barSub, barSub+1);
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
    auto code = static_cast<size_t>(key.getKeyCode());

    if (key == juce::KeyPress::backspaceKey) {
        deleteSelection();
        return true;
    }

    if (code == static_cast<size_t>(juce::KeyPress::upKey) && key.getModifiers().isAltDown()) {
        for (auto& noteSelected : notesSelected)
            noteSelected->ratio = noteSelected->ratio * 2;
        return true;
    }
    if (code == static_cast<size_t>(juce::KeyPress::downKey) && key.getModifiers().isAltDown()) {
        for (auto& noteSelected : notesSelected)
            noteSelected->ratio = noteSelected->ratio / 2;
        return true;
    }

    if (code == 'C' && key.getModifiers().isCommandDown()) {
        copySelectionToClipboard();
        return true;
    }
    if (code == 'V' && key.getModifiers().isCommandDown()) {
        pasteClipboard();
        return true;
    }
    if (code == 'D' && key.getModifiers().isCommandDown()) {
        duplicate();
        return true;
    }
    if (code == 'A' && key.getModifiers().isCommandDown()) {
        selectAll();
        return true;
    }

    if (code == '1' && key.getModifiers().isCommandDown()) {
        narrowGrid();
        return true;
    }
    if (code == '2' && key.getModifiers().isCommandDown()) {
        widenGrid();
        return true;
    }
    if (code == '3' && key.getModifiers().isCommandDown()) {
        tripletGrid();
        return true;
    }

    if (code == 'L') {
        incrementLockYSetting();
        return true;
    }
    if (code == 'R') {
        incrementReferenceSetting();
        return true;
    }

    return false;
}

void PianoRoll::incrementLockYSetting() {
    LockY newLockYSetting = static_cast<LockY>((lockYSetting + 1) % 3);
    settingsBar.setLockY(newLockYSetting);
}

void PianoRoll::incrementReferenceSetting() {
    if (referenceSetting == lockNote && notesSelected.size() == 1 && notesSelected[0] != lockedNoteReference) {
        lockedNoteReference = notesSelected[0];
        return;
    }
    Reference newReferenceSetting = static_cast<Reference>((referenceSetting + 1) % 2);
    settingsBar.setReference(newReferenceSetting);
}

void PianoRoll::addNoteWithoutReference(double frequency, float start, float end) {
    noteRegion.addNoteWithoutReference(frequency, start, end);
    pushStateToProcessor();
}

void PianoRoll::addNoteWithRefFreq(double refFreq, Fraction ratio, double irratio, float start, float end) {
    noteRegion.addNoteWithRefFreq(refFreq, ratio, irratio, start, end);
    pushStateToProcessor();
}

void PianoRoll::deleteNote(Note* note) {
    noteRegion.deleteNote(note);
    unselectNote(note);
    if (note == lockedNoteReference && referenceSetting == lockNote)
        settingsBar.setReference(selectedNote);
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
        clipboard.push_back(std::make_unique<Note>(
            note->referenceFrequency,
            note->ratio,
            note->irratio,
            note->start,
            note->end
            ));
    }
}

void PianoRoll::pasteClipboard() {
    if (clipboard.empty())
        return;

    auto earliestStart = clipboard.front()->start;
    for (const auto& note : clipboard)
        if (note->start < earliestStart)
            earliestStart = note->start;

    auto latestEnd = clipboard.front()->end;
    for (const auto& note : clipboard)
        if (note->end > latestEnd)
            latestEnd = note->end;

    for (const auto& note : clipboard) {
        float start = playheadBarPos + note->start - earliestStart;
        float end = start + note->end - note->start;
        noteRegion.addNoteWithRefFreq(note->referenceFrequency, note->ratio, note->irratio, start, end);
    }

    playheadBarPos += latestEnd - earliestStart;

    pushStateToProcessor();
}

void PianoRoll::duplicate() {
    auto earliestStart = notesSelected.front()->start;
    for (const auto& note : notesSelected)
        if (note->start < earliestStart)
            earliestStart = note->start;

    auto latestEnd = notesSelected.front()->end;
    for (const auto& note : notesSelected)
        if (note->end > latestEnd)
            latestEnd = note->end;

    for (const auto& note : notesSelected) {
        float start = latestEnd + note->start - earliestStart;
        float end = start + note->end - note->start;
        noteRegion.addNoteWithRefFreq(note->referenceFrequency, note->ratio, note->irratio, start, end);
    }

    pushStateToProcessor();
}

void PianoRoll::selectAll() {
    notesSelected.clear();
    notesSelected.reserve(noteRegion.notes.size());
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

Rect PianoRoll::getCanvasBounds() const {
    auto bounds = getLocalBounds();
    bounds.removeFromTop(SETTINGS_BAR_HEIGHT);
    return bounds;
}

Rect PianoRoll::getSettingsBarBounds() const {
    auto bounds = getLocalBounds();
    return bounds.removeFromTop(SETTINGS_BAR_HEIGHT);
}

Rect PianoRoll::getNoteCanvasBounds() const {
    auto bounds = getCanvasBounds();
    bounds.removeFromTop(ORIENTATION_BAR_HEIGHT);
    return bounds;
}

Rect PianoRoll::getOrientationBarBounds() const {
    auto bounds = getCanvasBounds();
    return bounds.removeFromTop(ORIENTATION_BAR_HEIGHT);
}

void PianoRoll::handleLockYChanged() {
    lockYSetting = settingsBar.getLockY();
}

void PianoRoll::handleReferenceChanged() {
    switch (settingsBar.getReference()) {
        case selectedNote:
            break;

        case lockNote:
            if (notesSelected.size() == 1) {
                auto note = notesSelected[0];
                lockedNoteReference = note;
            }
            else
                settingsBar.setReference(selectedNote);
            break;

        case customRef:
            throw std::invalid_argument("not implemented");
            break;
    }
    referenceSetting = settingsBar.getReference();
}

void PianoRoll::handlePotentialRatiosChanged() {
    potentialRatios = settingsBar.getPotentialRatios();
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