//
// Created by Vos on 09/10/2025.
//

#include "PianoRoll.h"

PianoRoll::PianoRoll(UnTETeredAudioProcessor& proc) : processor(proc) {
    initialisePotentialRatios();
    setWantsKeyboardFocus(true);
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
    drawBackground(g);
    drawPotentialRatios(g);
    drawBarLines(g);
    drawNotes(g);
    drawRectDragged(g);
    drawPlayhead(g);
}

void PianoRoll::fillRect(Rect& rect, juce::Graphics& g) {
    g.fillRect(rect);
}

void PianoRoll::drawRect(Rect& rect, juce::Graphics& g) {
    g.drawRect(rect);
}

void PianoRoll::drawText(juce::String& text, Rect& bounds, juce::Graphics& g) {
    g.drawSingleLineText(text, bounds.getX() + 5, (bounds).getBottom());
}

void PianoRoll::drawBackground(juce::Graphics& g) const {
    auto bounds = getLocalBounds();
    for (int y = 0; y < bounds.getHeight(); y++) {
        float hue = getHueFromYPx(y);
        auto colour = juce::Colour::fromHSV(hue, 0.5f, 0.7f, 1.0f);
        g.setColour(colour);
        Rect rect{0, y, bounds.getWidth(), 1};
        fillRect(rect, g);
    }
}

void PianoRoll::drawBarLines(juce::Graphics& g) const {
    auto bounds = getLocalBounds();
    g.setColour(juce::Colour::fromRGB(100, 100, 100));
    int firstBar = static_cast<int>((ceil(barLeftScreen) - barLeftScreen - 1) * static_cast<float>(barWidthPx));
    for (int x = firstBar; x < bounds.getWidth(); x += barWidthPx/getNrOfSubDivs()) {
        g.drawVerticalLine(x, 0, static_cast<float>(bounds.getHeight()));
    }

    g.setColour(juce::Colour::fromRGB(50, 50, 50));
    firstBar = static_cast<int>((ceil(barLeftScreen) - barLeftScreen) * static_cast<float>(barWidthPx));
    for (int x = firstBar; x < bounds.getWidth(); x += barWidthPx) {
        g.drawVerticalLine(x, 0, static_cast<float>(bounds.getHeight()));
    }
}

int PianoRoll::getNrOfSubDivs() const {
    return cachedNumerator > 0 ? cachedNumerator : 4;
}

void PianoRoll::drawNotes(juce::Graphics& g) const {
    for (auto& note : noteRegion.notes) {
        Rect bounds = getNoteBounds(note.get());
        g.setColour(juce::Colour::fromRGB(100, 100, 100));
        fillRect(bounds, g);
        g.setColour(juce::Colour::fromRGB(50, 50, 50));
        drawRect(bounds, g);
    }

    if (selectedNote) {
        // draw selectedNote over
        Rect boundsSelected = getNoteBounds(selectedNote);
        g.setColour(juce::Colour::fromRGB(50, 200, 50));
        fillRect(boundsSelected, g);
        g.setColour(juce::Colour::fromRGB(50, 50, 50));
        drawRect(boundsSelected, g);

        // draw children over
        for (auto& child : selectedNote->children) {
            Rect boundsChild = getNoteBounds(child);
            g.setColour(juce::Colour::fromRGB(50, 50, 200));
            fillRect(boundsChild, g);
            g.setColour(juce::Colour::fromRGB(50, 50, 50));
            drawRect(boundsChild, g);
        }

        if (auto asChild = dynamic_cast<ChildNote*>(selectedNote)) {
            Rect boundsParent = getNoteBounds(asChild->parent);
            g.setColour(juce::Colour::fromRGB(200, 50, 50));
            fillRect(boundsParent, g);
            g.setColour(juce::Colour::fromRGB(50, 50, 50));
            drawRect(boundsParent, g);
        }
    }

    for (auto& note : selectedNotesDragged) {
        Rect boundsSelectedDragged = getNoteBounds(note);
        g.setColour(juce::Colour::fromRGB(205, 205, 205));
        drawRect(boundsSelectedDragged, g);
    }

    for (auto& note : noteRegion.notes) {

    }
}

void PianoRoll::drawPotentialRatios(juce::Graphics& g) const {
    if (!selectedNote)
        return;
    for (auto& ratio : potentialRatios) {
        auto bounds = getPotentialRatioBounds(ratio).value();
        g.setColour(juce::Colour::fromRGBA(50, 50, 50, 128));
        fillRect(bounds, g);
        g.setColour(juce::Colour::fromRGB(50, 50, 50));
        juce::String text = ratio.toString();
        drawText(text, bounds, g);
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

void PianoRoll::drawPlayhead(juce::Graphics &g) const {
    auto bounds = getLocalBounds();
    auto playheadX = getXPxFromBar(playheadBarPos);
    g.setColour(juce::Colour::fromRGB(255, 255, 255));
    g.drawVerticalLine(playheadX, 0, static_cast<float>(bounds.getHeight()));
}


float PianoRoll::getHueFromYPx(int y) const {
    double freq = getFreqFromYPx(y);
    return getHueFromFreq(freq);
}

float PianoRoll::getHueFromFreq(double freq) {
    RootNote note{freq, 0, 0};
    return note.getHue();
}

double PianoRoll::getFreqFromYPx(int y) const {
    return getFreqFromYPxF(static_cast<float>(y));
}

double PianoRoll::getFreqFromYPxF(float y) const {
    float mirrored = mirrorYPx(y);
    double nrOfOctaves = static_cast<double>(mirrored)/static_cast<double>(octaveHeightPx);
    double freqFactor = std::pow(2, nrOfOctaves);
    return freqFactor * freqBottomScreen;
}

int PianoRoll::getYPxFromFreq(double freq) const {
    double freqFactor = freq / freqBottomScreen;
    double nrOfOctaves = std::log2(freqFactor);
    double yPx = nrOfOctaves * octaveHeightPx;
    float mirrored = mirrorYPx(static_cast<float>(yPx));
    return static_cast<int>(mirrored);
}

float PianoRoll::getBarExactFromXPx(int x) const {
    return getBarExactFromXPxF(static_cast<float>(x));
}

float PianoRoll::getBarExactFromXPxF(float x) const {
    return barLeftScreen + x / static_cast<float>(barWidthPx);
}

int PianoRoll::getXPxFromBar(float bar) const {
    return static_cast<int>((bar - barLeftScreen) * static_cast<float>(barWidthPx));
}

float PianoRoll::getBarSubFromXPx(int px) const {
    float exact = getBarExactFromXPx(px);
    auto nrOfSubDivs = static_cast<float>(getNrOfSubDivs());
    return static_cast<float>(static_cast<int>(exact * nrOfSubDivs)) / nrOfSubDivs;
}

int PianoRoll::getBarFloorFromXPx(int px) const {
    float exact = getBarExactFromXPx(px);
    return static_cast<int>(exact);
}

Note* PianoRoll::getNoteAt(Point px) const {
    for (auto& note : noteRegion.notes) {
        Rect bounds = getNoteBounds(note.get()).expanded(3);
        if (bounds.contains(px))
            return note.get();
    }
    return nullptr;
}

std::optional<Fraction> PianoRoll::getPotentialRatioAt(Point px) const {
    if (!selectedNote)
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

Rect PianoRoll::getNoteBounds(Note* note) const {
    int startPx = getXPxFromBar(note->start);
    int endPx = getXPxFromBar(note->end);
    int noteWidth = (endPx - startPx);
    int noteHeight = static_cast<int>(NOTE_HEIGHT_PER_OCTAVE*static_cast<float>(octaveHeightPx));
    int y = getYPxFromFreq(note->frequency) - noteHeight/2;
    Rect rect{startPx, y, noteWidth, noteHeight};
    return rect;
}

std::optional<Rect> PianoRoll::getPotentialRatioBounds(Fraction ratio) const {
    auto localBounds = getLocalBounds();
    if (!selectedNote)
        return std::nullopt;
    double freqPotential = selectedNote->frequency * static_cast<double>(ratio);
    int yMid = getYPxFromFreq(freqPotential);
    int noteHeight = static_cast<int>(NOTE_HEIGHT_PER_OCTAVE*static_cast<float>(octaveHeightPx));
    int y = yMid - noteHeight/2;
    Rect rect{0, y, localBounds.getWidth(), noteHeight};
    return rect;
}

std::vector<double> PianoRoll::getPotentialFrequencies(Note* note) const {
    std::vector<double> potentialFreqs;
    potentialFreqs.reserve(potentialRatios.size());
    for (auto& f : potentialRatios)
        potentialFreqs.push_back(note->frequency * static_cast<double>(f));
    return potentialFreqs;
}

void PianoRoll::selectNote(Note* note, Point clickedPos) {
    selectedNote = note;
    selectedNoteStart = note->start;
    selectedNoteEnd = note->end;
    selectedPotentialRatio.reset();
    dragLeftSideSelectedNote = false;
    dragRightSideSelectedNote = false;
    if (abs(clickedPos.x-getXPxFromBar(note->end)) < 5)
        dragRightSideSelectedNote = true;
    if (abs(clickedPos.x-getXPxFromBar(note->start)) < 5)
        dragLeftSideSelectedNote = true;
    selectedNotesDragged.clear();
}

void PianoRoll::selectPotentialRatio(Fraction ratio) {
    selectedPotentialRatio = ratio;
}

void PianoRoll::unselectNote() {
    selectedNote = nullptr;
    selectedPotentialRatio.reset();
    selectedNotesDragged.clear();
}

float PianoRoll::mirrorYPx(float y) const {
    return static_cast<float>(getHeight()) - y;
}

Point PianoRoll::mirrorYPx(Point point) const {
    return {point.x, getHeight() - point.y - 1};
}

Rect PianoRoll::mirrorYPx(Rect rect) const {
    return {rect.getX(), getHeight() - rect.getY() - 1 - rect.getHeight(), rect.getWidth(), rect.getHeight()};
}

void PianoRoll::setPlayheadPosFromPoint(Point point) {
    float bar = getBarSubFromXPx(point.x);
    playheadBarPos = bar;
}


void PianoRoll::mouseDown(const juce::MouseEvent& event) {
    Point posPx = event.getPosition();
    int nrOfClicks = (event.getNumberOfClicks()-1)%2+1;
    if (nrOfClicks == 1)
        handleSingleClick(posPx);
    if (nrOfClicks == 2)
        handleDoubleClick(posPx);
    repaint();
}

void PianoRoll::mouseDrag(const juce::MouseEvent& event) {
    if (selectedNote && !selectedPotentialRatio) {
        int dX = event.getDistanceFromDragStartX();
        float dBar = getBarSubFromXPx(dX);
        if (!dragRightSideSelectedNote)
            selectedNote->start = selectedNoteStart + dBar;
        if (!dragLeftSideSelectedNote)
            selectedNote->end = selectedNoteEnd + dBar;
    }
    else {
        unselectNote();
        selectedNotesDragged.clear();
        Point start = event.getMouseDownPosition();
        Point offset = event.getPosition();
        draggedRect = {start, offset};
        for (auto& note : noteRegion.notes) {
            Note* ptr = note.get();
            Rect boundsNote = getNoteBounds(ptr);
            if (boundsNote.intersects(draggedRect.value()))
                selectedNotesDragged.push_back(ptr);
        }
    }
    repaint();
}

void PianoRoll::mouseUp(const juce::MouseEvent& _) {
    dragLeftSideSelectedNote = false;
    dragRightSideSelectedNote = false;
    draggedRect.reset();
    repaint();
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
    if (barLeftScreen < 0)
        barLeftScreen = 0;
    if (freqBottomScreen < 10)
        freqBottomScreen = 10;
    repaint();
}

void PianoRoll::handleSingleClick(Point px) {
    if (auto note = getNoteAt(px)) {
        selectNote(note, px);
    } else if (auto potentialChild = getPotentialRatioAt(px)) {
        selectPotentialRatio(potentialChild.value());
    } else {
        unselectNote();
        setPlayheadPosFromPoint(px);
    }
}

void PianoRoll::handleDoubleClick(Point px) {
    auto barSub = getBarSubFromXPx(px.getX());
    if (selectedPotentialRatio) {
        noteRegion.addChildNote(selectedNote, selectedPotentialRatio.value(), 1, barSub, barSub+1);
        return;
    }
    if (selectedNote) {
        noteRegion.deleteNote(selectedNote);
        unselectNote();
        return;
    }
    auto freq = getFreqFromYPx(px.getY());
    noteRegion.addRootNote(freq, barSub, barSub + 1);
}

bool PianoRoll::keyPressed(const juce::KeyPress& key) {
    if (key == juce::KeyPress::backspaceKey) {
        deleteSelection();
        repaint();
        return true;
    }
    return false;
}

void PianoRoll::deleteSelection() {
    if (selectedNote) {
        noteRegion.deleteNote(selectedNote);
        unselectNote();
    }

    for (auto& note : selectedNotesDragged) {
        if (dynamic_cast<ChildNote*>(note)) {
            noteRegion.deleteNote(note);
            note = nullptr;
        }
    }

    for (auto note : selectedNotesDragged)
        noteRegion.deleteNote(note);

    selectedNotesDragged.clear();
}

void PianoRoll::timerCallback() {
    const auto& transportState = processor.getTransportState();

    cachedPpqPosition = transportState.ppqPosition.load(std::memory_order_relaxed);
    cachedNumerator = transportState.numerator.load(std::memory_order_relaxed);
    cachedDenominator = transportState.denominator.load(std::memory_order_relaxed);

    DBG(cachedPpqPosition + cachedNumerator + cachedDenominator);
    playheadBarPos = static_cast<float>(ppqToBar(cachedPpqPosition, cachedNumerator, cachedDenominator));

    DBG(playheadBarPos);

    repaint();
}

double PianoRoll::getQuarterNotesPerBar(int numerator, int denominator) {
    return denominator > 0
        ? 4.0 * static_cast<double>(numerator) / static_cast<double>(denominator)
        : 4.0;
}

double PianoRoll::ppqToBar(double ppq, int numerator, int denominator) {
    const auto qNPerBar = getQuarterNotesPerBar(numerator, denominator);
    return qNPerBar > 0.0 ? ppq / qNPerBar : 0.0;
}