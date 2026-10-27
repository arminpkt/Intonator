//
// Created by Vos on 09/10/2025.
//

#include <unordered_set>

#include "PianoRoll.h"

#include "PianoRollInstructionsWindow.h"
#include "PianoRollStateHelpers.h"
#include "IntervalPresets.h"
#include "../../logic/util.h"
#include "../../CrashLog.h"

PianoRoll::PianoRoll(UnTETeredAudioProcessor& proc)
    : processor(proc),
      settingsBar(PianoRollSettingsBar(
          [this] { handleSnapYChanged(); },
          [this] { handleAbsoluteInfoChanged(); },
          [this] { handleMonitoringChanged(); },
          [this] { handleIntervalsChanged(); },
          [this] { handleCustomIntervalsChanged(); },
          [this] { handleSettingsClicked(); },
          [this] { handleInstructionsClicked(); },
          [this] (const juce::MouseEvent& e) { mouseMove(e); })),
      settingsWindow(
          [this]{ handleColourSchemeChanged(); },
          [this]{ handleGradientColourChanged(); },
          [this]{ handlePitchbendRangeChanged(); },
          [this]{ handleConcertPitchChanged(); },
          this) {
    Component::setName("PianoRoll");
    setWantsKeyboardFocus(true);
    pullStateFromProcessorAndRebuild();
    addAndMakeVisible(settingsBar);
    instructionsWindow.setVisible(false);
    settingsWindow.setVisible(false);
    setLookAndFeel(&lookAndFeel);
    startTimerHz(30);
}

PianoRoll::~PianoRoll() {
    stopAllPreviews();
}

void PianoRoll::paint(juce::Graphics& g) {
    if (isInvalidState()) {
        drawInvalidScreen(g);
        return;
    }

    drawBackground(g, getNoteCanvasBounds());
    drawIntervals(g);
    drawBarLines(g, getNoteCanvasBounds(), true);
    drawNotes(g);
    drawRectDragged(g);
    drawPasteCursorLine(g);
    drawOrientationBar(g);
    drawPlayhead(g);
    drawPlayheadHandle(g);
    drawSettingsBackground(g);
}

void PianoRoll::fillRect(juce::Graphics& g, const Rect& rect, const juce::Colour colour) {
    g.setColour(colour);
    g.fillRect(rect);
}

void PianoRoll::drawRect(juce::Graphics& g, const Rect& rect, const juce::Colour colour) {
    g.setColour(colour);
    g.drawRect(rect);
}

void PianoRoll::drawText(juce::Graphics& g, const juce::String& text, const Rect& bounds, const juce::Colour colour,
    juce::Justification justification = juce::Justification::centredLeft) {
    auto actualBounds = bounds.withTrimmedLeft(4).withTrimmedRight(4).withTrimmedTop(2).withTrimmedBottom(2);
    g.setColour(colour);
    g.drawFittedText(text, actualBounds, justification, 1);
}

void PianoRoll::drawBackground(juce::Graphics& g, const Rect& bounds) const {
    auto [t, l, b, r, w, h] = Util::getTLBRWH(bounds);
    for (int y = t; y < b; y++) {
        auto colour = getBackgroundLineColour(y);
        fillRect(g, {l, y, w, 1}, colour);
    }
}

juce::Colour PianoRoll::getBackgroundLineColour(int y) const {
    float hue = getHueFromYPx(y);
    if (colourSchemeSetting == PianoRollSettingsWindow::RAINBOW_ID)
        return juce::Colour::fromHSV(hue, 0.2f, 0.7f, 1.f);
    if (colourSchemeSetting == PianoRollSettingsWindow::GRADIENT_ID) {
        return gradientBackground.getColourAtPosition(hue).withSaturation(0.2f).withBrightness(0.7f).withAlpha(1.f);
    }
    return INVALID_BACKGROUND_COLOUR;
}

void PianoRoll::drawBarLines(juce::Graphics& g, const Rect& bounds, bool drawSubDivs) const {
    auto [t, l, b, r, w, h] = Util::getTLBRWH(bounds.toFloat());
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
    auto intRatios = Util::getIntRatios(notesSelected);

    for (auto& note : noteRegion.notes) {
        auto [bodyColour, outlineColour, textColour] = getNoteColours(note.get());
        juce::String text = getNoteText(note.get(), intRatios);

        auto bounds = getNoteBounds(note.get());
        fillRect(g, bounds, bodyColour);
        drawRect(g, bounds, outlineColour);
        drawText(g, text, bounds, textColour);
    }
}

void PianoRoll::drawIntervals(juce::Graphics& g) const {
    if (!lockedNoteReference) return;
    for (auto& ratio : intervals) {
        auto bounds = getIntervalBounds(ratio).value();

        auto intervalColour = getIntervalColour(ratio);

        fillRect(g, bounds, intervalColour);
        juce::String text = ratio.toString();
        drawText(g, text, bounds, INTERVAL_TEXT_COLOUR);
        drawText(g, text, bounds, INTERVAL_TEXT_COLOUR, juce::Justification::centredRight);
    }
}

juce::Colour PianoRoll::getIntervalColour(Fraction interval) const {
    auto intervalColour = (intervalHighlighted && interval == intervalHighlighted.value())
                          ? INTERVAL_BODY_COLOUR.brighter() : INTERVAL_BODY_COLOUR;
    if (interval.isOctavial())
        intervalColour = intervalColour.withMultipliedAlpha(1.5);

    return intervalColour;
}

std::tuple<juce::Colour, juce::Colour, juce::Colour> PianoRoll::getNoteColours(Note* note, bool highlight) const {
    juce::Colour body = BASE_BODY_COLOUR;
    juce::Colour outline = BASE_OUTLINE_COLOUR;
    juce::Colour text = BASE_TEXT_COLOUR;

    if (lockedNoteReference && note == lockedNoteReference) {
        body = LOCKED_REF_BODY_COLOUR;
        outline = LOCKED_REF_OUTLINE_COLOUR;
        text = LOCKED_REF_TEXT_COLOUR;
    }
    else if (lockedNoteReference && note->isFamiliarWith(lockedNoteReference)) {
        body = FAMILY_BODY_COLOUR;
        outline = FAMILY_OUTLINE_COLOUR;
        text = FAMILY_RATIO_TEXT_COLOUR;
    }

    if (indexOfSelection(note))
        outline = MULT_SELECTED_OUTLINE_COLOUR;

    if (highlight && note == noteHighlighted) {
        body = body.brighter();
        text = text.brighter();
    }

    return std::make_tuple(body, outline, text);
}

juce::String PianoRoll::getNoteText(const Note* note, std::optional<std::vector<int>> intRatios) const {
    if (lockedNoteReference && note == lockedNoteReference)
        return "";

    if (lockedNoteReference && note->isFamiliarWith(lockedNoteReference))
        return (note->ratio / lockedNoteReference->ratio).toString();

    auto i = indexOfSelection(note);
    if (i && !lockedNoteReference && intRatios && intRatios.value().size() > 1)
        return std::to_string(intRatios.value()[i.value()]);

    if (absoluteInfoSetting)
        return note->getAbsoluteInfo(concertPitch);

    return "";
}

void PianoRoll::drawRectDragged(juce::Graphics& g) const {
    if (draggedRect) {
        fillRect(g, draggedRect.value(), DRAGGED_RECT_BODY_COLOUR);
        drawRect(g, draggedRect.value(), DRAGGED_RECT_OUTLINE_COLOUR);
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
        drawText(g, juce::String(bar + 1),
            orientationBarBounds.withX(x).withWidth(static_cast<int>(barWidthPxF)), BAR_LINE_COLOUR);
    }
}

void PianoRoll::drawDividerBeneath(juce::Graphics& g, Rect bounds) const {
    auto [t, l, b, r, w, h] = Util::getTLBRWH(bounds.toFloat());
    g.setColour(BAR_LINE_COLOUR);
    g.drawHorizontalLine(static_cast<int>(b), l, r);
}

void PianoRoll::drawPlayhead(juce::Graphics& g) const {
    auto [t, l, b, r, w, h] = Util::getTLBRWH(getNoteCanvasBounds());
    g.setColour(PLAYHEAD_COLOUR);
    float playheadPos = (draggedPlayheadHandle && !cachedIsPlaying) ? internalPlayheadHandleBarPos : hostPlayheadBarPos;
    g.drawVerticalLine(getXPxFromBar(playheadPos), static_cast<float>(t), static_cast<float>(b));
}

void PianoRoll::drawPlayheadHandle(juce::Graphics &g) const {
    auto [t, l, b, r, w, h] = Util::getTLBRWH(getOrientationBarBounds().toFloat());
    int xPx = internalPlayheadDetached ? getXPxFromBar(internalPlayheadHandleBarPos)
                                        : getXPxFromBar(hostPlayheadHandleBarPos);

    constexpr float halfW = 5.0f;
    constexpr float triH  = 7.0f;
    juce::Path triangle;
    triangle.startNewSubPath(static_cast<float>(xPx + 0.5), b);
    triangle.lineTo          (static_cast<float>(xPx) - halfW, b - triH);
    triangle.lineTo          (static_cast<float>(xPx) + halfW, b - triH);
    triangle.closeSubPath();
    g.setColour(PLAYHEAD_COLOUR);
    g.fillPath(triangle);
}

void PianoRoll::drawPasteCursorHandle(juce::Graphics& g) const {
    auto [t, l, b, r, w, h] = Util::getTLBRWH(getOrientationBarBounds().toFloat());
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
    auto [t, l, b, r, w, h] = Util::getTLBRWH(getNoteCanvasBounds());
    float alpha = 0.25f + ((std::sin(blinkPhase) + 1.0f) * 0.5f) * 0.75f;
    g.setColour(PASTE_CURSOR_COLOUR.withAlpha(alpha));
    g.drawVerticalLine(getXPxFromBar(pasteCursorBarPos), static_cast<float>(t), static_cast<float>(b));
}

void PianoRoll::drawSettingsBackground(juce::Graphics& g) const {
    auto settingsBarBounds = getSettingsBarBounds();
    drawBackground(g, settingsBarBounds);
    fillRect(g, settingsBarBounds, SETTINGS_BAR_BACKGROUND_COLOUR);
    drawDividerBeneath(g, settingsBarBounds);
}

void PianoRoll::drawInvalidScreen(juce::Graphics& g) const {
    auto bounds = getLocalBounds();
    drawRect(g, bounds, INVALID_BACKGROUND_COLOUR);

    drawText(g, "Something went wrong, please send us the log file so we can fix this bug.\n"
             "On Windows, this file might be located at \\Documents and Settings\\username\\Application Data\\UnTETered\n"
             "On MacOS, this file might be located at ~/Library/UnTETered",
             bounds, INVALID_TEXT_COLOUR, juce::Justification::centred);
}

void PianoRoll::resized() {
    clipMagnification(0, 0);
    clipScreenEdges();
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
float PianoRoll::getHueFromFreq(double freq) const {
    Note note{freq, {1, 1}, 1, 0, 0};
    return note.getHue();
}
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
    for (auto note : notesSelected) {
        float barExact = getBarExactFromXPx(px.getX());
        if (barExact < note->start || barExact > note->end)
            continue;
        if (getNoteBounds(note).expanded(3).contains(px))
            return note;
    }
    for (auto& note : noteRegion.notes) {
        float barExact = getBarExactFromXPx(px.getX());
        if (barExact < note->start || barExact > note->end)
            continue;
        if (getNoteBounds(note.get()).expanded(3).contains(px))
            return note.get();
    }
    return nullptr;
}

std::optional<Fraction> PianoRoll::getIntervalAt(Point px) const {
    if (!lockedNoteReference) return std::nullopt;
    for (auto& ratio : intervals) {
        auto boundsOpt = getIntervalBounds(ratio);
        if (boundsOpt && boundsOpt.value().expanded(3).contains(px))
            return ratio;
    }
    return std::nullopt;
}

std::optional<Fraction> PianoRoll::getClosestInterval(Point px) {
    if (!lockedNoteReference)
        return std::nullopt;

    int smallestDistance = std::numeric_limits<int>::max();
    std::optional<Fraction> closestInterval{};
    for (auto& interval : intervals) {
        auto bounds = getIntervalBounds(interval).value();
        auto distance = std::abs(px.getY() - bounds.getCentreY());
        if (distance < 20 && distance < smallestDistance) {
            smallestDistance = distance;
            closestInterval = interval;
        }
    }

    if (closestInterval)
        return closestInterval;

    return getIntervalAt(px);
}

Rect PianoRoll::getNoteBounds(const Note* note) const {
    int noteH = getNoteHeight();
    return { getXPxFromBar(note->start),
             getYPxFromFreq(note->getFrequency()) - noteH / 2,
             getXPxFromBar(note->end) - getXPxFromBar(note->start),
             noteH };
}

int PianoRoll::getNoteHeight() const {
    return static_cast<int>(NOTE_HEIGHT_PER_OCTAVE * octaveHeightPxF);
}

std::optional<Rect> PianoRoll::getIntervalBounds(Fraction ratio) const {
    if (!lockedNoteReference) return std::nullopt;
    auto [t, l, b, r, w, h] = Util::getTLBRWH(getNoteCanvasBounds());
    int noteH = static_cast<int>(NOTE_HEIGHT_PER_OCTAVE * octaveHeightPxF);
    int y = getYPxFromFreq(lockedNoteReference->getFrequency() * static_cast<double>(ratio)) - noteH / 2;
    return Rect{l, y, r, noteH};
}

std::optional<std::tuple<double, Fraction, double>> PianoRoll::getReferenceRefFreqRatioIrratio() const {
    if (!lockedNoteReference)
        return std::nullopt;
    return std::make_tuple(
        lockedNoteReference->referenceFrequency,
        lockedNoteReference->ratio,
        lockedNoteReference->irratio
        );
}

std::vector<double> PianoRoll::getIntervalFrequencies(Note* note) const {
    std::vector<double> out;
    out.reserve(intervals.size());
    for (auto& f : intervals)
        out.push_back(note->getFrequency() * static_cast<double>(f));
    return out;
}

void PianoRoll::selectNote(Note* note, Point clickedPos, bool invertIfSelected) {
    CrashLog::LOG_SCOPE();
    logNote(note);

    if (indexOfSelection(note)) {
        if (invertIfSelected) {
            unselectNote(note);
            return;
        }
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
    CrashLog::LOG_SCOPE();
    auto index = indexOfSelection(note);
    if (index)
        notesSelected.erase(notesSelected.begin() + static_cast<long>(index.value()));
}

void PianoRoll::setReference(Note* note) {
    CrashLog::LOG_SCOPE();
    logNote(note);

    lockedNoteReference = note;
}

void PianoRoll::removeReference() {
    CrashLog::LOG_SCOPE();

    lockedNoteReference = nullptr;
}

void PianoRoll::startNotePreview(const Note* note) {
    if (!monitoringSetting) return;

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

    processor.addPreviewMessages({
        juce::MidiMessage::pitchWheel(channel, note->getPitchBendValue(pitchbendRange)),
        juce::MidiMessage::noteOn(channel, note->getRoundedMidiValue(), static_cast<juce::uint8>(100))});

    activePreviews.push_back({ note->getRoundedMidiValue(), channel, PREVIEW_DURATION_TICKS });
}

void PianoRoll::stopAllPreviews() {
    for (const auto& p : activePreviews)
        processor.addPreviewMessages({juce::MidiMessage::noteOff(p.channel, p.midiNote)});
    activePreviews.clear();
}

void PianoRoll::mouseDown(const juce::MouseEvent& event) {
    if (invalidState)
        return;

    CrashLog::LOG_SCOPE();
    Point posPx = event.getPosition();

    if (getOrientationBarBounds().contains(posPx)) {
        if (!hostTransportControllable)
            return;
        internalPlayheadBarPos = getBarSubRoundedFromXPx(posPx.getX());
        internalPlayheadHandleBarPos = internalPlayheadBarPos;
        processor.requestHostPlayheadJumpToBar(internalPlayheadBarPos);
        playheadHandleClicked = true;
        internalPlayheadDetached = true;
        internalPlayheadChanged = true;
        return;
    }

    previewedDuringCurrentDrag.clear();

    int nrOfClicks = (event.getNumberOfClicks() - 1) % 2 + 1;
    if (event.mods.isShiftDown() && nrOfClicks == 1)
        handleShiftSingleClick(posPx);
    else if (event.mods.isAltDown() && nrOfClicks == 1)
        handleOptionSingleClick(posPx);
    else if (nrOfClicks == 1)
        handleSingleClick(posPx);
    else if (nrOfClicks == 2)
        handleDoubleClick(posPx);
}

void PianoRoll::mouseDrag(const juce::MouseEvent& event) {
    if (invalidState)
        return;

    Point currentPos = event.getPosition();

    if (playheadHandleClicked) {
        //pasteCursorBarPos = std::max(0.0f, getBarSubRoundedFromXPx(currentPos.getX()));
        internalPlayheadBarPos = std::max(0.0f, getBarSubRoundedFromXPx(currentPos.getX()));
        internalPlayheadHandleBarPos = internalPlayheadBarPos;
        draggedPlayheadHandle = true;
        return;
    }

    Point mouseDownPos = event.getMouseDownPosition();
    if (!noteClicked || event.mods.isShiftDown()) {
        if (!event.mods.isShiftDown()) notesSelected.clear();
        dragRectangle(mouseDownPos, currentPos);
    } else if (!event.mods.isAltDown()) {
        moveExtendShrinkNotes(mouseDownPos, currentPos);
    }
}

void PianoRoll::mouseMove(const juce::MouseEvent& event) {
    if (invalidState)
        return;

    noteHighlighted = nullptr;
    intervalHighlighted.reset();
    auto position = event.getPosition();

    if (getNoteCanvasBounds().contains(position)) {
        if (auto* noteAt = getNoteAt(position))
            noteHighlighted = noteAt;
        else if (auto closestInterval = getClosestInterval(position))
            intervalHighlighted = closestInterval;
    }

    setTooltipColours();
}

void PianoRoll::mouseMagnify(const juce::MouseEvent& event, const float scaleFactor) {
    if (invalidState)
        return;

    auto mousePos = event.getPosition();
    auto pxFromTop = mousePos.getY();
    auto mouseX = mousePos.getX();
    if (event.mods.isShiftDown()) {
        freqBottomScreen = getFreqFromYPx(pxFromTop);
        octaveHeightPxF *= scaleFactor;
        freqBottomScreen = getFreqFromYPx(mirrorYPx(pxFromTop, 0));
    } else {
        barLeftScreen = getBarExactFromXPx(mouseX);
        barWidthPxF  *= scaleFactor;
        barLeftScreen = getBarExactFromXPx(-mouseX);
    }
    clipMagnification(mouseX, pxFromTop);
    clipScreenEdges();
    pushViewportToProcessor();
}

void PianoRoll::zoomX(const float scaleFactor, const int mouseX) {
    barLeftScreen = getBarExactFromXPx(mouseX);
    barWidthPxF  *= scaleFactor;
    barLeftScreen = getBarExactFromXPx(-mouseX);
}

void PianoRoll::zoomY(const float scaleFactor, const int pxFromTop) {
    freqBottomScreen = getFreqFromYPx(pxFromTop);
    octaveHeightPxF *= scaleFactor;
    freqBottomScreen = getFreqFromYPx(mirrorYPx(pxFromTop, 0));
}

void PianoRoll::mouseUp(const juce::MouseEvent& _) {
    if (invalidState)
        return;

    CrashLog::LOG_SCOPE();

    playheadHandleClicked = false;
    if (draggedPlayheadHandle) {
        draggedPlayheadHandle = false;
        processor.requestHostPlayheadJumpToBar(internalPlayheadBarPos);
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

    if (noteHighlighted)
        newNoteLength = noteHighlighted->end - noteHighlighted->start;
}

void PianoRoll::mouseWheelMove(const juce::MouseEvent& _, const juce::MouseWheelDetails& wheel) {
    scroll({wheel.deltaX, wheel.deltaY});
}

void PianoRoll::scroll(const PointF deltaXY) {
    if (invalidState)
        return;

    const PointF scaled = deltaXY * SCROLL_FACTOR;
    barLeftScreen    = getBarExactFromXPxF(-scaled.getX());
    freqBottomScreen = getFreqFromYPxF(mirrorYPx(scaled.getY()));
    clipScreenEdges();
    pushViewportToProcessor();
}

void PianoRoll::clipMagnification(const int mouseX, const int pxFromTop) {
    if (barWidthPxF < LOWEST_ALLOWED_BAR_WIDTH)
        zoomX(LOWEST_ALLOWED_BAR_WIDTH / barWidthPxF, mouseX);

    if (barWidthPxF > HIGHEST_ALLOWED_BAR_WIDTH)
        zoomX(HIGHEST_ALLOWED_BAR_WIDTH / barWidthPxF, mouseX);

    float lowestAllowedOctaveHeight = getNoteCanvasBounds().toFloat().getHeight() * 12 / 128;
    if (octaveHeightPxF < lowestAllowedOctaveHeight)
        zoomY(lowestAllowedOctaveHeight / octaveHeightPxF, pxFromTop);

    if (octaveHeightPxF > HIGHEST_ALLOWED_OCTAVE_HEIGHT)
        zoomY(HIGHEST_ALLOWED_OCTAVE_HEIGHT / octaveHeightPxF, pxFromTop);
}

void PianoRoll::clipScreenEdges() {
    if (barLeftScreen < 0.0f)
        barLeftScreen = 0.0f;

    auto halfNoteH = getNoteHeight() / 2;
    auto freqMarginFactor = getFreqFromYPx(0) / getFreqFromYPx(halfNoteH);

    if (freqBottomScreen < LOWEST_ALLOWED_FREQ / freqMarginFactor)
        freqBottomScreen = LOWEST_ALLOWED_FREQ / freqMarginFactor;

    auto topScreenYPx = getNoteCanvasBounds().getTopLeft().getY();
    auto freqTopScreen = getFreqFromYPx(topScreenYPx);
    if (freqTopScreen > HIGHEST_ALLOWED_FREQ * freqMarginFactor)
        freqBottomScreen /= (freqTopScreen / HIGHEST_ALLOWED_FREQ / freqMarginFactor);
}

void PianoRoll::handleSingleClick(const Point px) {
    CrashLog::LOG_SCOPE();
    if (auto* note = getNoteAt(px)) {
        if (!indexOfSelection(note)) notesSelected.clear();
        selectNote(note, px);

        dragStartOffsetPx = px.getX() - getXPxFromBar(note->start);
        updateSelectedNotesSnapshot();

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

void PianoRoll::updateSelectedNotesSnapshot() {
    selectedNotesStartsEnds.clear();
    for (const auto& n : notesSelected) selectedNotesStartsEnds.emplace_back(n->start, n->end);
    selectedNotesRefFreqs.clear();
    for (const auto& n : notesSelected) selectedNotesRefFreqs.push_back(n->referenceFrequency);
}

void PianoRoll::handleDoubleClick(const Point px) {
    CrashLog::LOG_SCOPE();
    auto barSub= getBarSubFromXPx(px.getX());
    auto* noteAt = getNoteAt(px);

    if (noteAt) {
        deleteNote(noteAt);
        return;
    }

    if (auto closestInterval = getClosestInterval(px)) {
        auto [refFreq, ratio, irratio] = getReferenceRefFreqRatioIrratio().value();
        addNoteWithRefFreq(refFreq, ratio * closestInterval.value(), irratio, barSub, barSub + newNoteLength);
        return;
    }

    addNoteWithoutReference(getFreqFromYPx(px.getY()), barSub, barSub + newNoteLength);
}

void PianoRoll::handleShiftSingleClick(const Point px) {
    CrashLog::LOG_SCOPE();
    if (auto* note = getNoteAt(px)) {
        selectNote(note, px, true);
        startNotePreview(note);
    }
}

void PianoRoll::handleOptionSingleClick(const Point px) {
    CrashLog::LOG_SCOPE();
    if (auto* note = getNoteAt(px)) {
        if (note == lockedNoteReference)
            removeReference();
        else
            setReference(note);
    }
    else
        removeReference();
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
        if (dragLeftSideSelectedNote)
            n->start = start + dBar;
        else if (dragRightSideSelectedNote)
            n->end = end + dBar;
        else {
            n->start = start + dBar;
            n->end = end + dBar;
        }
    }

    for (auto* note : notesSelected) {
        if (note->start < 0) {
            moveExtendShrinkHorizontally(dX - static_cast<int>(note->start * barWidthPxF));
            return;
        }
        if (note->end <= note->start && dragRightSideSelectedNote) {
            moveExtendShrinkHorizontally(
                dX - static_cast<int>((note->end-note->start - 1/static_cast<float>(getNrOfSubDivs())) * barWidthPxF));
            return;
        }
        if (note->end <= note->start && dragLeftSideSelectedNote) {
            moveExtendShrinkHorizontally(
                dX + static_cast<int>((note->end-note->start - 1/static_cast<float>(getNrOfSubDivs())) * barWidthPxF));
            return;
        }
    }
}

void PianoRoll::moveVertically(const Point currentPos, const Point mouseDownPos) {
    // Move freely if not locked
    if (!snapYSetting) {
        double freqFactor = getFreqFromYPx(currentPos.getY()) / getFreqFromYPx(mouseDownPos.getY());
        moveVerticallyFreely(freqFactor);
        return;
    }

    // Don't move if no locked reference
    if (!lockedNoteReference) return;

    // Don't move if locked reference is part of selection
    for (auto* s : notesSelected) if (s == lockedNoteReference) return;

    // Don't move if there is no valid place to move to
    auto interval = getIntervalAt(currentPos);
    if (!interval) return;

    // Move relative to reference
    moveVerticallyRelativeToReference(interval.value());
}

void PianoRoll::moveVerticallyFreely(double freqFactor) {
    for (size_t i = 0; i < notesSelected.size(); i++)
        notesSelected[i]->referenceFrequency = selectedNotesRefFreqs[i] * freqFactor;

    for (auto* note : notesSelected) {
        if (note->getFrequency() > HIGHEST_ALLOWED_FREQ) {
            moveVerticallyFreely(freqFactor * (HIGHEST_ALLOWED_FREQ * 0.999999) / note->getFrequency());
            return;
        }
        if (note->getFrequency() < LOWEST_ALLOWED_FREQ) {
            moveVerticallyFreely(freqFactor * (LOWEST_ALLOWED_FREQ * 1.000001) / note->getFrequency());
            return;
        }
    }
}

void PianoRoll::moveVerticallyRelativeToReference(const Fraction& interval) const {
    auto refFreq   = lockedNoteReference->referenceFrequency;
    auto ratio    = lockedNoteReference->ratio;
    auto irratio   = lockedNoteReference->irratio;
    double refFreqFactor = refFreq / noteClicked->referenceFrequency;
    Fraction ratioFactor = ratio   / noteClicked->ratio;
    double irratioFactor = irratio / noteClicked->irratio;

    for (auto* note : notesSelected) {
        Note copy = *note;
        copy.referenceFrequency *= refFreqFactor;
        copy.ratio = copy.ratio * ratioFactor * interval;
        copy.irratio *= irratioFactor;
        if (copy.getFrequency() > HIGHEST_ALLOWED_FREQ || copy.getFrequency() < LOWEST_ALLOWED_FREQ)
            return;
    }

    for (auto* note : notesSelected) {
        note->referenceFrequency *= refFreqFactor;
        note->ratio = note->ratio * ratioFactor * interval;
        note->irratio *= irratioFactor;
    }
}

bool PianoRoll::keyPressed(const juce::KeyPress& key) {
    if (invalidState)
        return false;

    CrashLog::LOG_SCOPE();

    auto code = static_cast<size_t>(key.getKeyCode());
    CrashLog::log(juce::String(code));

    if (key == juce::KeyPress::spaceKey) {
        if (!hostTransportControllable)
            return false;
        float playheadPos = internalPlayheadDetached ? internalPlayheadBarPos : hostPlayheadBarPos;
        processor.requestHostPlaybackStartStop(playheadPos);
        return true;
    }

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

    if (code == 'X' && key.getModifiers().isCommandDown()) { cutSelection();              return true; }
    if (code == 'C' && key.getModifiers().isCommandDown()) { copySelectionToClipboard();  return true; }
    if (code == 'V' && key.getModifiers().isCommandDown()) { pasteClipboard();            return true; }
    if (code == 'D' && key.getModifiers().isCommandDown()) { duplicate();                 return true; }
    if (code == 'A' && key.getModifiers().isCommandDown()) { selectAll();                 return true; }
    if (code == '1' && key.getModifiers().isCommandDown()) { narrowGrid();                return true; }
    if (code == '2' && key.getModifiers().isCommandDown()) { widenGrid();                 return true; }
    if (code == '3' && key.getModifiers().isCommandDown()) { tripletGrid();               return true; }
    if (code == 'Y')                                       { toggleSnapYSetting();        return true; }
    if (code == 'M')                                       { toggleMonitoringSetting();   return true; }
    if (code == 'I')                                       { toggleAbsoluteInfoSetting(); return true; }
    if (code == 'T')                                       { roundTo12TET();              return true; }
    return false;
}

void PianoRoll::toggleSnapYSetting() {
    settingsBar.setSnapY(!snapYSetting, true);
}

void PianoRoll::setSnapY(bool snapY) {
    snapYSetting = snapY;
    settingsBar.setSnapY(snapY);
}

void PianoRoll::toggleMonitoringSetting() {
    settingsBar.setMonitoring(!monitoringSetting, true);
}

void PianoRoll::setMonitoring(bool monitoring) {
    monitoringSetting = monitoring;
    settingsBar.setMonitoring(monitoring, true);
}

void PianoRoll::toggleAbsoluteInfoSetting() {
    settingsBar.setAbsoluteInfo(!absoluteInfoSetting, true);
}

void PianoRoll::setAbsoluteInfo(bool absoluteInfo) {
    absoluteInfoSetting = absoluteInfo;
    settingsBar.setAbsoluteInfo(absoluteInfo);
}

void PianoRoll::roundTo12TET() {
    CrashLog::LOG_SCOPE();
    pushUndoSnapshot();
    for (auto& note : notesSelected) {
        note->roundTo12TET();
    }
    pushNoteStateToProcessor();
}

void PianoRoll::addNoteWithoutReference(double frequency, float start, float end) {
    CrashLog::LOG_SCOPE();
    pushUndoSnapshot();
    noteRegion.addNoteWithoutReference(frequency, start, end);
    logNote(noteRegion.notes.back().get());
    pushNoteStateToProcessor();
    if (!noteRegion.notes.empty())
        startNotePreview(noteRegion.notes.back().get());
}

void PianoRoll::addNoteWithRefFreq(double refFreq, Fraction ratio, double irratio, float start, float end) {
    CrashLog::LOG_SCOPE();
    pushUndoSnapshot();
    noteRegion.addNoteWithRefFreq(refFreq, ratio, irratio, start, end);
    logNote(noteRegion.notes.back().get());
    pushNoteStateToProcessor();
    if (!noteRegion.notes.empty())
        startNotePreview(noteRegion.notes.back().get());
}

void PianoRoll::deleteNote(Note* note, bool pushState) {
    CrashLog::LOG_SCOPE();
    logNote(note);

    if (pushState)
        pushUndoSnapshot();

    noteRegion.deleteNote(note);
    unselectNote(note);
    if (note == lockedNoteReference)
        removeReference();

    if (note == noteHighlighted)
        noteHighlighted = nullptr;

    if (pushState)
        pushNoteStateToProcessor();
}

void PianoRoll::deleteSelection() {
    CrashLog::LOG_SCOPE();
    if (notesSelected.empty()) return;

    pushUndoSnapshot();
    while (!notesSelected.empty())
        deleteNote(notesSelected.back(), false);
    pushNoteStateToProcessor();
}

void PianoRoll::copySelectionToClipboard() {
    CrashLog::LOG_SCOPE();
    clipboard.clear();
    for (const auto* note : notesSelected)
        clipboard.push_back(std::make_unique<Note>(
            note->referenceFrequency, note->ratio, note->irratio, note->start, note->end));
}

void PianoRoll::cutSelection() {
    CrashLog::LOG_SCOPE();
    if (notesSelected.empty()) return;
    pushUndoSnapshot();
    copySelectionToClipboard();
    deleteSelection();
    pushNoteStateToProcessor();
}

void PianoRoll::pasteClipboard() {
    CrashLog::LOG_SCOPE();
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
    CrashLog::LOG_SCOPE();
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

void PianoRoll::applyIntervalsSetting() {
    if (intervalsSetting == STANDARD_ID) {
        intervals = Util::expandIntervals(getIntervalsByID(intervalsSetting));
        settingsBar.setCustomIntervalsVisibility(false);
    }
    if (intervalsSetting == CUSTOM_INTERVALS_ID) {
        intervals = customIntervals;
        settingsBar.setCustomIntervalsVisibility(true);
    }
}

void PianoRoll::applyGradientSetting() {
    gradientBackground.clearColours();
    gradientBackground.addColour(0, gradientColour1);
    gradientBackground.addColour(0.5, gradientColour2);
    gradientBackground.addColour(1, gradientColour1);
}

void PianoRoll::handleSnapYChanged() {
    snapYSetting = settingsBar.getSnapY();
    pushNoteStateToProcessor();
}

void PianoRoll::handleAbsoluteInfoChanged() {
    absoluteInfoSetting = settingsBar.getAbsoluteInfo();
    pushNoteStateToProcessor();
}

void PianoRoll::handleMonitoringChanged() {
    monitoringSetting = settingsBar.getMonitoring();
    if (!monitoringSetting)
        stopAllPreviews();
    pushNoteStateToProcessor();
}

void PianoRoll::handleIntervalsChanged() {
    intervalsSetting = settingsBar.getIntervals();
    applyIntervalsSetting();
    pushNoteStateToProcessor();
}

void PianoRoll::handleCustomIntervalsChanged() {
    customIntervals = settingsBar.getCustomIntervals();
    applyIntervalsSetting();
    pushNoteStateToProcessor();
}

void PianoRoll::handleSettingsClicked() {
    settingsWindow.open();
}

void PianoRoll::handleInstructionsClicked() {
    instructionsWindow.open();
}

void PianoRoll::handleColourSchemeChanged() {
    auto value = settingsWindow.getColourSchemeId();
    colourSchemeSetting = value;
    settingsWindow.placeComponents();
}

void PianoRoll::handleGradientColourChanged() {
    auto [c1, c2] = settingsWindow.getGradientColours();
    gradientColour1 = c1;
    gradientColour2 = c2;
    applyGradientSetting();
}

void PianoRoll::handlePitchbendRangeChanged() {
    auto value = settingsWindow.getPitchbendRange();
    pitchbendRange = juce::jlimit(0.5f, 48.f, value);
    settingsWindow.setPitchbendRange(pitchbendRange);
}

void PianoRoll::handleConcertPitchChanged() {
    auto value = settingsWindow.getConcertPitch();
    concertPitch = juce::jlimit(400.f, 480.f, value);
    settingsWindow.setConcertPitch(concertPitch);
}

void PianoRoll::changeListenerCallback(juce::ChangeBroadcaster* _) {
    handleGradientColourChanged();
}

void PianoRoll::timerCallback() {
    hostTransportControllable = processor.canControlHostTransport();
    if (!hostTransportControllable)
        internalPlayheadDetached = false;   // playhead always follows the host
    const auto& ts = processor.getTransportState();
    cachedPpqPosition = ts.ppqPosition.load(std::memory_order_relaxed);
    cachedNumerator   = ts.numerator.load(std::memory_order_relaxed);
    cachedDenominator = ts.denominator.load(std::memory_order_relaxed);
    cachedIsPlaying = ts.isPlaying.load(std::memory_order_relaxed);

    hostPlayheadBarPos = static_cast<float>(
        TimelineHelpers::ppqToBar(cachedPpqPosition, cachedNumerator, cachedDenominator));
    if ((hostPlayheadBarPos != oldHostPlayheadBarPos) && (hostPlayheadBarPos != internalPlayheadBarPos) && !cachedIsPlaying && !wasPlaying) {
            internalPlayheadDetached = false;
            hostPlayheadHandleBarPos = hostPlayheadBarPos;
    }

    oldHostPlayheadBarPos = hostPlayheadBarPos;
    wasPlaying = cachedIsPlaying;

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
    CrashLog::LOG_SCOPE();
    const juce::ScopedValueSetter<bool> guard(isLoadingState, true);

    const auto state = processor.getPianoRollState();
    octaveHeightPxF  = state.octaveHeightPxF;
    barWidthPxF      = state.barWidthPxF;
    freqBottomScreen = state.freqBottomScreen;
    barLeftScreen    = state.barLeftScreen;
    noteRegion       = PianoRollStateHelpers::makeNoteRegionFromState(state);

    notesSelected.clear();
    selectedNotesStartsEnds.clear();
    selectedNotesRefFreqs.clear();
    removeReference();
    noteHighlighted = nullptr;
    noteClicked = nullptr;
    draggedRect.reset();

    undoStack.clear();
    redoStack.clear();
    undoSnapshotTakenForCurrentDrag = false;
    noteWasDraggedThisGesture = false;

    // Settings Bar
    snapYSetting        = state.snapY;
    absoluteInfoSetting = state.absoluteInfo;
    monitoringSetting   = state.monitoringEnabled;
    intervalsSetting    = state.intervalsSetting;

    settingsBar.setSnapY(snapYSetting);
    settingsBar.setAbsoluteInfo(absoluteInfoSetting);
    settingsBar.setMonitoring( monitoringSetting);
    settingsBar.setIntervals(intervalsSetting);

    customIntervals.clear();
    if (!state.customIntervals.empty()) {
        for (const auto& [num, den] : state.customIntervals)
            customIntervals.emplace_back( num, den );
    }
    else
        customIntervals = STANDARD;

    settingsBar.setCustomIntervals(customIntervals);
    applyIntervalsSetting();

    // Settings Window
    colourSchemeSetting = state.colourScheme;
    gradientColour1 = PianoRollStateHelpers::makeColourFromTuple(state.gradientColour1);
    gradientColour2 = PianoRollStateHelpers::makeColourFromTuple(state.gradientColour2);
    applyGradientSetting();
    pitchbendRange = state.pitchBendRange;
    concertPitch = state.concertPitch;

    settingsWindow.setColourScheme(colourSchemeSetting);
    settingsWindow.setGradientColours(gradientColour1, gradientColour2);
    settingsWindow.setPitchbendRange(pitchbendRange);
    settingsWindow.setConcertPitch(concertPitch);

    repaint();
}

void PianoRoll::pushNoteStateToProcessor() const {
    CrashLog::LOG_SCOPE();
    if (isLoadingState)
        return;
    PianoRollState state    = PianoRollStateHelpers::makeStateFromNoteRegion(noteRegion);
    state.octaveHeightPxF   = octaveHeightPxF;
    state.barWidthPxF       = barWidthPxF;
    state.freqBottomScreen  = freqBottomScreen;
    state.barLeftScreen     = barLeftScreen;

    state.snapY             = snapYSetting;
    state.absoluteInfo      = absoluteInfoSetting;
    state.monitoringEnabled = monitoringSetting;
    state.intervalsSetting  = intervalsSetting;
    state.customIntervals.clear();
    for (const auto& f : customIntervals) {
        auto [num, den] = f.getNumeratorAndDenominator();
        state.customIntervals.emplace_back(num, den);
    }

    state.colourScheme      = colourSchemeSetting;
    state.gradientColour1   = PianoRollStateHelpers::makeTupleFromColour(gradientColour1);
    state.gradientColour2   = PianoRollStateHelpers::makeTupleFromColour(gradientColour2);
    state.pitchBendRange    = pitchbendRange;
    state.concertPitch      = concertPitch;

    processor.setPianoRollState(state);
    processor.markStateDirtyForHost();
}

void PianoRoll::pushViewportToProcessor() const {
    processor.setPianoRollViewState(octaveHeightPxF, barWidthPxF, freqBottomScreen, barLeftScreen);
}

void PianoRoll::pushUndoSnapshot() {
    CrashLog::LOG_SCOPE();
    undoStack.push_back(PianoRollStateHelpers::makeStateFromNoteRegion(noteRegion).notes);
    if (undoStack.size() > static_cast<size_t>(MAX_UNDO_STEPS))
        undoStack.erase(undoStack.begin());
    redoStack.clear();
}

void PianoRoll::undo() {
    CrashLog::LOG_SCOPE();
    if (undoStack.empty()) return;
    redoStack.push_back(PianoRollStateHelpers::makeStateFromNoteRegion(noteRegion).notes);
    PianoRollState state;
    state.notes = std::move(undoStack.back()); undoStack.pop_back();
    state.octaveHeightPxF  = octaveHeightPxF;
    state.barWidthPxF      = barWidthPxF;
    state.freqBottomScreen = freqBottomScreen;
    state.barLeftScreen    = barLeftScreen;
    noteRegion = PianoRollStateHelpers::makeNoteRegionFromState(state);
    notesSelected.clear();
    lockedNoteReference = nullptr;
    noteHighlighted = nullptr;
    pushNoteStateToProcessor();
}

void PianoRoll::redo() {
    CrashLog::LOG_SCOPE();
    if (redoStack.empty()) return;
    undoStack.push_back(PianoRollStateHelpers::makeStateFromNoteRegion(noteRegion).notes);
    PianoRollState state;
    state.notes = std::move(redoStack.back()); redoStack.pop_back();
    state.octaveHeightPxF  = octaveHeightPxF;
    state.barWidthPxF      = barWidthPxF;
    state.freqBottomScreen = freqBottomScreen;
    state.barLeftScreen    = barLeftScreen;
    noteRegion = PianoRollStateHelpers::makeNoteRegionFromState(state);
    notesSelected.clear();
    lockedNoteReference = nullptr;
    noteHighlighted = nullptr;
    pushNoteStateToProcessor();
}

bool PianoRoll::exists(const Note* note) const {
    for (auto& existing : noteRegion.notes)
        if (existing.get() == note)
            return true;
    return false;
}

bool PianoRoll::isInvalidState() {
    if (invalidState) return true;

    for (auto selected : notesSelected) {
        if (!selected) {
            CrashLog::log("selected note is nullptr");
            invalidState = true;
            continue;
        }
        if (!exists(selected)) {
            CrashLog::log("selected note doesn't exist");
            invalidState = true;
        }
    }

    if (lockedNoteReference && !exists(lockedNoteReference)) {
        CrashLog::log("reference doesn't exist");
        invalidState = true;
    }

    if (noteHighlighted && !exists(noteHighlighted)){
        CrashLog::log("highlighted note doesn't exist");
        invalidState = true;
    }

    return invalidState;
}

void PianoRoll::setTooltipColours() {
    lookAndFeel.setColour(juce::TooltipWindow::backgroundColourId, TOOLTIP_BODY_COLOUR);
    lookAndFeel.setColour(juce::TooltipWindow::outlineColourId, TOOLTIP_OUTLINE_COLOUR);
    lookAndFeel.setColour(juce::TooltipWindow::textColourId, TOOLTIP_TEXT_COLOUR);

    if (noteHighlighted) {
        auto [bodyColour, outlineColour, textColour] = getNoteColours(noteHighlighted, false);
        lookAndFeel.setColour(juce::TooltipWindow::backgroundColourId, bodyColour);
        lookAndFeel.setColour(juce::TooltipWindow::outlineColourId, outlineColour);
        lookAndFeel.setColour(juce::TooltipWindow::textColourId, textColour);
    }
}

juce::String PianoRoll::getTooltip() {
    setTooltipColours();

    if (noteHighlighted) {
        auto tooltipText = noteHighlighted->getAbsoluteInfo(concertPitch);
        if (lockedNoteReference && noteHighlighted->isFamiliarWith(lockedNoteReference)) {
            auto interval = noteHighlighted->ratio / lockedNoteReference->ratio;
            tooltipText += ":   " + getIntervalTooltipText(interval);
        }

        return tooltipText;
    }

    if (intervalHighlighted)
        return getIntervalTooltipText(intervalHighlighted.value());

    return {};
}

juce::String PianoRoll::getIntervalTooltipText(Fraction& interval) {
    auto tooltipText = interval.toString() + " (";
    if (auto name = interval.getName())
        tooltipText += name.value() + ", ";
    tooltipText += juce::String(interval.getSizeInCents()) + "ct)";
    return tooltipText;
}

void PianoRoll::logNote(const Note* note) const {
    CrashLog::log(note ? note->getAbsoluteInfo(concertPitch) : "NULLPTR NOTE");
}
