//
// Created by Vos on 09/10/2025.
//

#include <unordered_set>

#include "PianoRoll.h"
#include "PianoRollStateHelpers.h"
#include "IntervalPresets.h"
#include "../../logic/util.h"

PianoRoll::PianoRoll(UnTETeredAudioProcessor& proc)
    : processor(proc),
      settingsBar(PianoRollSettingsBar(
          [this] { handleLockYChanged(); },
          [this] { handleLockRefChanged(); },
          [this] { handleIntervalsChanged(); },
          [this] { handleCustomIntervalsChanged(); },
          [this] { handleMonitoringChanged(); })) {
    setWantsKeyboardFocus(true);
    pullStateFromProcessorAndRebuild();
    startTimerHz(30);
    addAndMakeVisible(settingsBar);
}

PianoRoll::~PianoRoll() {
    stopAllPreviews();
}

void PianoRoll::paint(juce::Graphics& g) {
    drawBackground(g, getNoteCanvasBounds());
    drawIntervals(g);
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

void PianoRoll::fillRect(juce::Graphics& g, const Rect& rect) { g.fillRect(rect); }
void PianoRoll::drawRect(juce::Graphics& g, const Rect& rect) { g.drawRect(rect); }

void PianoRoll::drawText(const juce::String& text, const Rect& bounds, const juce::Graphics& g,
    juce::Justification justification = juce::Justification::centredLeft) {
    auto actualBounds = bounds.withTrimmedLeft(4).withTrimmedRight(4).withTrimmedTop(2).withTrimmedBottom(2);
    g.drawFittedText(text, actualBounds, justification, 1);
}

void PianoRoll::drawBackground(juce::Graphics& g, const Rect& bounds) const {
    auto [t, l, b, r, w, h] = getTLBRWH(bounds);
    for (int y = t; y < b; y++) {
        auto colour = juce::Colour::fromHSV(getHueFromYPx(y), 0.2f, 0.7f, 1.0f);
        g.setColour(colour);
        fillRect(g, {l, y, w, 1});
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
    if (gridTripletted) subDivs = subDivs * Fraction{3, 2};
    return subDivs;
}

int PianoRoll::getNrOfSubDivs() const {
    return getSubDivsFraction().getNumeratorAndDenominator().first;
}

void PianoRoll::drawNotes(juce::Graphics& g) const {
    for (auto& note : noteRegion.notes)
        drawNote(note.get(), NOTE_BASE_COLOUR, NOTE_OUTLINE_COLOUR, g);

    if (lockRefSetting)
        drawNote(lockedNoteReference, SELECTED_BASE_COLOUR.withMultipliedSaturation(.5), SELECTED_OUTLINE_COLOUR, g);

    if (notesSelected.size() == 1) {
        auto* noteSelected = notesSelected[0];
        for (auto& note : noteRegion.notes) {
            if (note.get() == noteSelected || !note->isFamiliarWith(noteSelected)) continue;
            drawNote(note.get(), FAMILY_BASE_COLOUR, FAMILY_OUTLINE_COLOUR, g);
            g.setColour(FAMILY_RATIO_TEXT_COLOUR);
            drawText((note->ratio / noteSelected->ratio).toString(), getNoteBounds(note.get()), g);
        }
        drawNote(noteSelected, SELECTED_BASE_COLOUR, SELECTED_OUTLINE_COLOUR, g);
    }

    if (notesSelected.size() > 1) {
        auto intRatios = getIntRatios(notesSelected);
        auto oddified = getIntRatios(notesSelected, true);
        for (size_t i = 0; i < notesSelected.size(); ++i) {
            auto* noteSelected = notesSelected[i];
            drawNote(noteSelected, MULT_SELECTED_BASE_COLOUR, MULT_SELECTED_OUTLINE_COLOUR, g);
            if (intRatios) {
                g.setColour(INT_RATIO_TEXT_COLOUR);
                auto boundsLeft = getNoteBounds(noteSelected);
                auto boundsRight = boundsLeft.removeFromRight(boundsLeft.getWidth()/2);
                drawText(juce::String(intRatios.value()[i]), boundsLeft, g);
                drawText("(" + juce::String(oddified.value()[i]) + ")", boundsRight, g, juce::Justification::right);
            }
        }
    }
}

void PianoRoll::drawNote(const Note* note, const juce::Colour& baseColour, const juce::Colour& outlineColour, juce::Graphics& g) const {
    auto colour = (note == noteHighlighted) ? baseColour.brighter() : baseColour;
    g.setColour(colour);
    fillRect(g, getNoteBounds(note));
    g.setColour(outlineColour);
    drawRect(g, getNoteBounds(note));
}

void PianoRoll::drawIntervals(juce::Graphics& g) const {
    if (!referenceExists()) return;
    for (auto& ratio : intervals) {
        auto bounds = getIntervalBounds(ratio).value();
        auto baseColour = (intervalHighlighted && ratio == intervalHighlighted.value())
                          ? INTERVAL_BASE_COLOUR.brighter() : INTERVAL_BASE_COLOUR;
        g.setColour(baseColour);
        fillRect(g, bounds);
        g.setColour(INTERVAL_TEXT_COLOUR);
        juce::String text = ratio.toString();
        drawText(text, bounds, g);
        drawText(text, bounds, g, juce::Justification::centredRight);
    }
}

void PianoRoll::drawRectDragged(juce::Graphics& g) const {
    if (draggedRect) {
        g.setColour(DRAGGED_RECT_BASE_COLOUR);
        fillRect(g, draggedRect.value());
        g.setColour(DRAGGED_RECT_OUTLINE_COLOUR);
        drawRect(g, draggedRect.value());
    }
}

void PianoRoll::drawOrientationBar(juce::Graphics& g) const {
    auto orientationBarBounds = getOrientationBarBounds();
    drawBackground(g, orientationBarBounds);
    drawBarLines(g, orientationBarBounds, false);

    drawDividerBeneath(g, orientationBarBounds);

    int initial = static_cast<int>(std::floor(barLeftScreen));
    int last    = static_cast<int>(std::floor(barLeftScreen + static_cast<float>(orientationBarBounds.getWidth()) / barWidthPxF));
    for (int bar = initial; bar <= last; ++bar) {
        auto x = getXPxFromBar(static_cast<float>(bar));
        drawText(juce::String(bar + 1), orientationBarBounds.withX(x).withWidth(static_cast<int>(barWidthPxF)), g);
    }
}

void PianoRoll::drawDividerBeneath(juce::Graphics& g, Rect bounds) const {
    auto [t, l, b, r, w, h] = getTLBRWH(bounds.toFloat());
    g.setColour(BAR_LINE_COLOUR);
    g.drawHorizontalLine(static_cast<int>(b), l, r);
}

void PianoRoll::drawPlayhead(juce::Graphics& g) const {
    auto [t, l, b, r, w, h] = getTLBRWH(getCanvasBounds());
    g.setColour(PLAYHEAD_COLOUR);
    g.drawVerticalLine(getXPxFromBar(playheadBarPos), static_cast<float>(t), static_cast<float>(b));
}

void PianoRoll::drawPasteCursorHandle(juce::Graphics& g) const {
    auto [t, l, b, r, w, h] = getTLBRWH(getOrientationBarBounds().toFloat());
    int xPx = getXPxFromBar(pasteCursorBarPos);

    constexpr float halfW = 5.0f;
    constexpr float triH  = 7.0f;
    juce::Path triangle;
    triangle.startNewSubPath(static_cast<float>(xPx),          b);
    triangle.lineTo          (static_cast<float>(xPx) - halfW, b - triH);
    triangle.lineTo          (static_cast<float>(xPx) + halfW, b - triH);
    triangle.closeSubPath();
    g.setColour(PASTE_CURSOR_COLOUR);
    g.fillPath(triangle);
}

void PianoRoll::drawPasteCursorLine(juce::Graphics& g) const {
    auto [t, l, b, r, w, h] = getTLBRWH(getNoteCanvasBounds());
    float alpha = 0.25f + ((std::sin(blinkPhase) + 1.0f) * 0.5f) * 0.75f;
    g.setColour(PASTE_CURSOR_COLOUR.withAlpha(alpha));
    g.drawVerticalLine(getXPxFromBar(pasteCursorBarPos), static_cast<float>(t), static_cast<float>(b));
}

void PianoRoll::drawSettingsBackground(juce::Graphics& g) const {
    auto settingsBarBounds = getSettingsBarBounds();
    drawBackground(g, settingsBarBounds);
    g.setColour(SETTINGS_BACKGROUND_COLOUR);
    fillRect(g, settingsBarBounds);
    drawDividerBeneath(g, settingsBarBounds);
}

void PianoRoll::resized() {
    settingsBar.setBounds(getSettingsBarBounds());
}

Rect PianoRoll::getCanvasBounds() const {
    auto bounds = getLocalBounds();
    bounds.removeFromTop(SETTINGS_BAR_HEIGHT);
    return bounds;
}
Rect PianoRoll::getSettingsBarBounds() const  { return getLocalBounds().removeFromTop(SETTINGS_BAR_HEIGHT); }
Rect PianoRoll::getNoteCanvasBounds() const   { auto b = getCanvasBounds(); b.removeFromTop(ORIENTATION_BAR_HEIGHT); return b; }
Rect PianoRoll::getOrientationBarBounds() const { return getCanvasBounds().removeFromTop(ORIENTATION_BAR_HEIGHT); }

float PianoRoll::getHueFromYPx(int y) const { return getHueFromFreq(getFreqFromYPx(y)); }
float PianoRoll::getHueFromFreq(double freq) { Note note{freq, {1, 1}, 1, 0, 0}; return note.getHue(); }
double PianoRoll::getFreqFromYPx(int y) const { return getFreqFromYPxF(static_cast<float>(y)); }

double PianoRoll::getFreqFromYPxF(float y) const {
    double nrOfOctaves = static_cast<double>(mirrorYPx(y)) / static_cast<double>(octaveHeightPxF);
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

bool PianoRoll::referenceExists() const { return getReferenceRefFreqRatioIrratio().has_value(); }

std::optional<Fraction> PianoRoll::getIntervalAt(Point px) const {
    if (!referenceExists()) return std::nullopt;
    for (auto& ratio : intervals) {
        auto boundsOpt = getIntervalBounds(ratio);
        if (boundsOpt && boundsOpt.value().expanded(3).contains(px))
            return ratio;
    }
    return std::nullopt;
}

Rect PianoRoll::getNoteBounds(const Note* note) const {
    int noteH = static_cast<int>(NOTE_HEIGHT_PER_OCTAVE * octaveHeightPxF);
    return { getXPxFromBar(note->start),
             getYPxFromFreq(note->getFrequency()) - noteH / 2,
             getXPxFromBar(note->end) - getXPxFromBar(note->start),
             noteH };
}

std::optional<Rect> PianoRoll::getIntervalBounds(Fraction ratio) const {
    if (!referenceExists()) return std::nullopt;
    auto [t, l, b, r, w, h] = getTLBRWH(getNoteCanvasBounds());
    int noteH = static_cast<int>(NOTE_HEIGHT_PER_OCTAVE * octaveHeightPxF);
    int y = getYPxFromFreq(getReferenceFrequency().value() * static_cast<double>(ratio)) - noteH / 2;
    return Rect{l, y, r, noteH};
}

std::optional<std::tuple<double, Fraction, double>> PianoRoll::getReferenceRefFreqRatioIrratio() const {
    if (lockRefSetting)
        return std::make_tuple(lockedNoteReference->referenceFrequency,
                               lockedNoteReference->ratio,
                               lockedNoteReference->irratio);
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

std::vector<double> PianoRoll::getIntervalFrequencies(Note* note) const {
    std::vector<double> out;
    out.reserve(intervals.size());
    for (auto& f : intervals)
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
        if (note == notesSelected[i]) return i;
    return std::nullopt;
}

void PianoRoll::unselectNote(const Note* note) {
    auto index = indexOfSelection(note);
    if (index)
        notesSelected.erase(notesSelected.begin() + static_cast<long>(index.value()));
}

void PianoRoll::startNotePreview(const Note* note) {
    if (!monitoringEnabled) return;

    int channel = PREVIEW_CHANNEL_START;

    if (static_cast<int>(activePreviews.size()) < PREVIEW_CHANNEL_COUNT) {
        for (int c = PREVIEW_CHANNEL_START; c < PREVIEW_CHANNEL_START + PREVIEW_CHANNEL_COUNT; ++c) {
            bool inUse = false;
            for (const auto& p : activePreviews)
                if (p.channel == c) { inUse = true; break; }
            if (!inUse) { channel = c; break; }
        }
    } else {
        auto oldest = std::min_element(activePreviews.begin(), activePreviews.end(),
            [](const ActivePreview& a, const ActivePreview& b) {
                return a.countdown < b.countdown;
            });
        processor.addPreviewMessages({juce::MidiMessage::noteOff(oldest->channel, oldest->midiNote)});
        channel = oldest->channel;
        activePreviews.erase(oldest);
    }

    processor.addPreviewMessages({juce::MidiMessage::pitchWheel(channel, note->getPitchBendValue(UnTETeredAudioProcessor::PITCH_BEND_RANGE)),
                                        juce::MidiMessage::noteOn(channel,
                                        note->getRoundedMidiValue(),static_cast<juce::uint8>(100))});

    activePreviews.push_back({ note->getRoundedMidiValue(), channel, PREVIEW_DURATION_TICKS });
}

void PianoRoll::stopAllPreviews() {
    for (const auto& p : activePreviews)
        processor.addPreviewMessages({juce::MidiMessage::noteOff(p.channel, p.midiNote)});
    activePreviews.clear();
}

void PianoRoll::handleMonitoringChanged() {
    monitoringEnabled = settingsBar.isMonitoringEnabled();
    if (!monitoringEnabled)
        stopAllPreviews();
    pushNoteStateToProcessor();
}

void PianoRoll::mouseDown(const juce::MouseEvent& event) {
    Point posPx = event.getPosition();

    if (getOrientationBarBounds().contains(posPx)) {
        pasteCursorBarPos = getBarSubRoundedFromXPx(posPx.getX());
        isDraggingPasteCursor = true;
        return;
    }

    previewedDuringCurrentDrag.clear();

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
        pasteCursorBarPos = std::max(0.0f, getBarSubRoundedFromXPx(currentPos.getX()));
        return;
    }

    Point mouseDownPos = event.getMouseDownPosition();
    if (!noteClicked || event.mods.isShiftDown()) {
        if (!event.mods.isShiftDown()) notesSelected.clear();
        dragRectangle(mouseDownPos, currentPos);
    } else {
        moveExtendShrinkNotes(mouseDownPos, currentPos);
    }
}

void PianoRoll::mouseMove(const juce::MouseEvent& event) {
    noteHighlighted = nullptr;
    intervalHighlighted.reset();
    auto position = event.getPosition();
    if (auto* noteAt = getNoteAt(position))
        noteHighlighted = noteAt;
    else if (auto intervalAt = getIntervalAt(position))
        intervalHighlighted = intervalAt;
}

void PianoRoll::mouseMagnify(const juce::MouseEvent& event, const float scaleFactor) {
    auto mousePos = event.getPosition();
    if (event.mods.isShiftDown()) {
        auto pxFromTop = mousePos.getY();
        freqBottomScreen = getFreqFromYPx(pxFromTop);
        octaveHeightPxF *= scaleFactor;
        freqBottomScreen = getFreqFromYPx(mirrorYPx(pxFromTop, 0));
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

void PianoRoll::handleSingleClick(const Point px) {
    if (auto* note = getNoteAt(px)) {
        if (!indexOfSelection(note)) notesSelected.clear();
        selectNote(note, px);

        dragStartOffsetPx = px.getX() - getXPxFromBar(note->start);
        selectedNotesStartsEnds.clear();
        for (const auto& n : notesSelected) selectedNotesStartsEnds.emplace_back(n->start, n->end);
        selectedNotesRefFreqs.clear();
        for (const auto& n : notesSelected) selectedNotesRefFreqs.push_back(n->referenceFrequency);

        noteClicked = note;
        undoSnapshotTakenForCurrentDrag = false;
        noteWasDraggedThisGesture       = false;

        startNotePreview(note);
        return;
    }

    noteClicked = nullptr;

    if (getIntervalAt(px)) {
        pasteCursorBarPos = getBarSubFromXPx(px.getX());
        return;
    }

    notesSelected.clear();
    pasteCursorBarPos = getBarSubFromXPx(px.getX());
}

void PianoRoll::handleDoubleClick(const Point px) {
    auto barSub           = getBarSubFromXPx(px.getX());
    auto intervalAt = getIntervalAt(px);
    auto* noteAt          = getNoteAt(px);

    if (noteAt) { deleteNote(noteAt); return; }

    if (intervalAt) {
        auto [refFreq, ratio, irratio] = getReferenceRefFreqRatioIrratio().value();
        addNoteWithRefFreq(refFreq, ratio * intervalAt.value(), irratio, barSub, barSub + 1);
        return;
    }

    addNoteWithoutReference(getFreqFromYPx(px.getY()), barSub, barSub + 1);
}

void PianoRoll::handleShiftSingleClick(const Point px) {
    if (auto* note = getNoteAt(px)) {
        selectNote(note, px, true);
        startNotePreview(note);
    }
}

void PianoRoll::dragRectangle(const Point mouseDownPos, const Point currentPos) {
    draggedRect = Rect{mouseDownPos, currentPos};
    if (draggedRect->getWidth() == 0) draggedRect->setWidth(1);
    for (auto& note : noteRegion.notes) {
        if (!indexOfSelection(note.get()).has_value() && getNoteBounds(note.get()).intersects(draggedRect.value())) {
            notesSelected.push_back(note.get());

            if (previewedDuringCurrentDrag.insert(note.get()).second)
                startNotePreview(note.get());
        }
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
    if (!lockYSetting) {
        double freqFactor = getFreqFromYPx(currentPos.getY()) / getFreqFromYPx(mouseDownPos.getY());
        for (size_t i = 0; i < notesSelected.size(); i++)
            notesSelected[i]->referenceFrequency = selectedNotesRefFreqs[i] * freqFactor;
        return;
    }

    if (!lockRefSetting) return;

    for (auto* s : notesSelected) if (s == lockedNoteReference) return;

    auto interval = getIntervalAt(currentPos);
    if (!interval) return;

    auto [refFreq, ratio, irratio] = getReferenceRefFreqRatioIrratio().value();
    double   refFreqFactor = refFreq / noteClicked->referenceFrequency;
    Fraction ratioFactor   = ratio   / noteClicked->ratio;
    double   irratioFactor = irratio / noteClicked->irratio;

    for (auto* note : notesSelected) {
        note->referenceFrequency *= refFreqFactor;
        note->ratio = note->ratio * ratioFactor * interval.value();
        note->irratio *= irratioFactor;
    }
}

bool PianoRoll::keyPressed(const juce::KeyPress& key) {
    auto code = static_cast<size_t>(key.getKeyCode());

    if (code == 'Z' && key.getModifiers().isCommandDown()) {
        if (key.getModifiers().isShiftDown()) redo(); else undo();
        return true;
    }
    if (key == juce::KeyPress::backspaceKey) { deleteSelection(); return true; }

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
    if (code == 'Y') { toggleLockYSetting();    return true; }
    if (code == 'R') { toggleLockRefSetting();  return true; }

    return false;
}

void PianoRoll::toggleLockYSetting() {
    settingsBar.setLockY(!lockYSetting, true);
}

void PianoRoll::setLockY(bool lockY) {
    lockYSetting = lockY;
    settingsBar.setLockY(lockY);
}

void PianoRoll::toggleLockRefSetting() {
    if (!lockRefSetting) {
        settingsBar.setLockRef(true, true);
        return;
    }

    if (notesSelected.size() == 1 && notesSelected[0] != lockedNoteReference) {
        settingsBar.setLockRef(false, true);
        settingsBar.setLockRef(true, true);
        return;
    }

    if (notesSelected.size() == 1 && notesSelected[0] == lockedNoteReference) {
        notesSelected.clear();
    }

    settingsBar.setLockRef(false, true);
}

void PianoRoll::setLockRef(bool lockRef) {
    lockRefSetting = lockRef;
    settingsBar.setLockRef(lockRef);
}

void PianoRoll::addNoteWithoutReference(double frequency, float start, float end) {
    pushUndoSnapshot();
    noteRegion.addNoteWithoutReference(frequency, start, end);
    pushNoteStateToProcessor();
    if (!noteRegion.notes.empty())
        startNotePreview(noteRegion.notes.back().get());
}

void PianoRoll::addNoteWithRefFreq(double refFreq, Fraction ratio, double irratio, float start, float end) {
    pushUndoSnapshot();
    noteRegion.addNoteWithRefFreq(refFreq, ratio, irratio, start, end);
    pushNoteStateToProcessor();
    if (!noteRegion.notes.empty())
        startNotePreview(noteRegion.notes.back().get());
}

void PianoRoll::deleteNote(Note* note, bool pushState) {
    if (pushState)
        pushUndoSnapshot();

    noteRegion.deleteNote(note);
    unselectNote(note);
    if (note == lockedNoteReference && lockRefSetting)
        setLockRef(false);

    if (pushState)
        pushNoteStateToProcessor();
}

void PianoRoll::deleteSelection() {
    if (notesSelected.empty()) return;

    pushUndoSnapshot();
    while (!notesSelected.empty())
        deleteNote(notesSelected.back(), false);
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
        if (lockRefSetting && note == lockedNoteReference)
            setLockRef(false);
        notesSelected.pop_back();
    }
    pushNoteStateToProcessor();
}

void PianoRoll::pasteClipboard() {
    if (clipboard.empty()) return;

    float earliestStart = clipboard.front()->start;
    for (const auto& n : clipboard) if (n->start < earliestStart) earliestStart = n->start;
    float latestEnd = clipboard.front()->end;
    for (const auto& n : clipboard) if (n->end > latestEnd) latestEnd = n->end;

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
    for (const auto* n : notesSelected) if (n->start < earliestStart) earliestStart = n->start;
    float latestEnd = notesSelected.front()->end;
    for (const auto* n : notesSelected) if (n->end > latestEnd) latestEnd = n->end;

    pushUndoSnapshot();

    std::vector<Note*> originalSelection = notesSelected;
    std::unordered_set<Note*> existingNotes;
    for (const auto& n : noteRegion.notes) existingNotes.insert(n.get());

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
    for (auto& note : noteRegion.notes) notesSelected.push_back(note.get());
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

void PianoRoll::handleLockYChanged() {
    lockYSetting = settingsBar.getLockY();
    pushNoteStateToProcessor();
}

void PianoRoll::handleLockRefChanged() {
    bool newLockRef = settingsBar.getLockRef();

    if (newLockRef) {
        if (notesSelected.size() == 1) lockedNoteReference = notesSelected[0];
        else newLockRef = false;
    }

    setLockRef(newLockRef);
    pushNoteStateToProcessor();
}

void PianoRoll::handleIntervalsChanged() {
    intervalsSetting = settingsBar.getIntervals();

    if (intervalsSetting == CUSTOM_INTERVALS_ID) {
        intervals = customIntervals;
        settingsBar.setCustomIntervalsVisibility(true);
    }
    else {
        intervals = getIntervalsByID(intervalsSetting);
        settingsBar.setCustomIntervalsVisibility(false);
    }

    pushNoteStateToProcessor();
}

void PianoRoll::handleCustomIntervalsChanged() {
    customIntervals = settingsBar.getCustomIntervals();
    if (intervalsSetting == CUSTOM_INTERVALS_ID)
        intervals = customIntervals;
    pushNoteStateToProcessor();
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

    // Tick down each active preview independently; send note-off when it expires.
    for (auto it = activePreviews.begin(); it != activePreviews.end(); ) {
        if (--(it->countdown) <= 0) {
            processor.addPreviewMessages({juce::MidiMessage::noteOff(it->channel, it->midiNote)});
            it = activePreviews.erase(it);
        } else {
            ++it;
        }
    }

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

    // Settings Bar
    // reference=lockNote can't survive serialization (pointer is gone), so fall back
    lockYSetting      = state.lockY;
    lockRefSetting    = false;
    monitoringEnabled = state.monitoringEnabled;
    intervalsSetting  = state.intervalsSetting;

    settingsBar.setLockY(lockYSetting);
    settingsBar.setLockRef(lockRefSetting);
    settingsBar.setMonitoringEnabled(monitoringEnabled);
    settingsBar.setIntervals(intervalsSetting);

    if (!state.customIntervals.empty())
    {
        customIntervals.clear();
        for (const auto& [num, den] : state.customIntervals)
            customIntervals.emplace_back( num, den );
        settingsBar.setCustomIntervals(customIntervals);
    }
    else {
        settingsBar.setCustomIntervals(SEVEN_LIMIT);
    }
}

void PianoRoll::pushNoteStateToProcessor() const {
    PianoRollState state    = makeStateFromNoteRegion(noteRegion);
    state.octaveHeightPxF   = octaveHeightPxF;
    state.barWidthPxF       = barWidthPxF;
    state.freqBottomScreen  = freqBottomScreen;
    state.barLeftScreen     = barLeftScreen;

    state.lockY             = lockYSetting;
    state.monitoringEnabled = monitoringEnabled;
    state.intervalsSetting  = intervalsSetting;
    state.customIntervals.clear();
    for (const auto& f : customIntervals)
    {
        auto [num, den] = f.getNumeratorAndDenominator();
        state.customIntervals.emplace_back(num, den);
    }
    processor.setPianoRollState(state);
}

void PianoRoll::pushViewportToProcessor() const {
    processor.setPianoRollViewState(octaveHeightPxF, barWidthPxF, freqBottomScreen, barLeftScreen);
}

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
    state.notes = std::move(undoStack.back()); undoStack.pop_back();
    state.octaveHeightPxF  = octaveHeightPxF;
    state.barWidthPxF      = barWidthPxF;
    state.freqBottomScreen = freqBottomScreen;
    state.barLeftScreen    = barLeftScreen;
    noteRegion = makeNoteRegionFromState(state);
    notesSelected.clear();
    pushNoteStateToProcessor();
}

void PianoRoll::redo() {
    if (redoStack.empty()) return;
    undoStack.push_back(makeStateFromNoteRegion(noteRegion).notes);
    PianoRollState state;
    state.notes = std::move(redoStack.back()); redoStack.pop_back();
    state.octaveHeightPxF  = octaveHeightPxF;
    state.barWidthPxF      = barWidthPxF;
    state.freqBottomScreen = freqBottomScreen;
    state.barLeftScreen    = barLeftScreen;
    noteRegion = makeNoteRegionFromState(state);
    notesSelected.clear();
    pushNoteStateToProcessor();
}