//
// Created by Vos on 09/10/2025.
//

#include "PianoRoll.h"

PianoRoll::PianoRoll(UnTETeredAudioProcessor& proc) : processor(proc) {
    initialisePotentialRatios();
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
}

void PianoRoll::drawBackground(juce::Graphics& g) const {
    auto bounds = getLocalBounds();
    for (int y = 0; y < bounds.getHeight(); y++) {
        float hue = getHueFromYPx(y);
        auto colour = juce::Colour::fromHSV(hue, 0.5f, 0.7f, 1.0f);
        g.setColour(colour);
        Rect rect{0, y, bounds.getWidth(), 1};
        g.fillRect(rect);
    }
}

void PianoRoll::drawBarLines(juce::Graphics& g) const {
    auto bounds = getLocalBounds();
    g.setColour(juce::Colour::fromRGB(100, 100, 100));
    int firstBar = static_cast<int>((ceil(barLeftScreen) - barLeftScreen - 1) * static_cast<float>(barWidthPx));
    for (int x = firstBar; x < bounds.getWidth(); x += barWidthPx/getNrOfSubDivs()) {
        Rect rect{x, 0, 1, bounds.getHeight()};
        g.fillRect(rect);
    }

    g.setColour(juce::Colour::fromRGB(50, 50, 50));
    firstBar = static_cast<int>((ceil(barLeftScreen) - barLeftScreen) * static_cast<float>(barWidthPx));
    for (int x = firstBar; x < bounds.getWidth(); x += barWidthPx) {
        Rect rect{x, 0, 1, bounds.getHeight()};
        g.fillRect(rect);
    }
}

int PianoRoll::getNrOfSubDivs() const {
    auto playHead = processor.getPlayHead();
    int nrOfSubDivs = playHead ? playHead->getPosition()->getTimeSignature()->numerator : 4;
    return nrOfSubDivs;
}

void PianoRoll::drawNotes(juce::Graphics& g) const {
    for (auto& note : noteRegion.notes) {
        Rect bounds = getNoteBounds(note.get());
        g.setColour(juce::Colour::fromRGB(100, 100, 100));
        g.fillRect(bounds);
        g.setColour(juce::Colour::fromRGB(50, 50, 50));
        g.drawRect(bounds);
    }

    if (selectedNote) {
        // draw selectedNote over
        Rect boundsSelected = getNoteBounds(selectedNote);
        g.setColour(juce::Colour::fromRGB(50, 200, 50));
        g.fillRect(boundsSelected);
        g.setColour(juce::Colour::fromRGB(50, 50, 50));
        g.drawRect(boundsSelected);

        // draw children over
        for (auto& child : selectedNote->children) {
            Rect boundsChild = getNoteBounds(child);
            g.setColour(juce::Colour::fromRGB(50, 50, 200));
            g.fillRect(boundsChild);
            g.setColour(juce::Colour::fromRGB(50, 50, 50));
            g.drawRect(boundsChild);
        }

        if (auto ptr = dynamic_cast<ChildNote*>(selectedNote)) {
            Rect boundsParent = getNoteBounds(ptr->parent);
            g.setColour(juce::Colour::fromRGB(200, 50, 50));
            g.fillRect(boundsParent);
            g.setColour(juce::Colour::fromRGB(50, 50, 50));
            g.drawRect(boundsParent);
        }
    }
}

void PianoRoll::drawPotentialRatios(juce::Graphics& g) const {
    if (!selectedNote)
        return;
    for (auto& ratio : potentialRatios) {
        auto bounds = getPotentialRatioBounds(ratio).value();
        g.setColour(juce::Colour::fromRGBA(50, 50, 50, 128));
        g.fillRect(bounds);
        g.setColour(juce::Colour::fromRGB(50, 50, 50));
        juce::String text = ratio.toString();
        g.drawSingleLineText(text, bounds.getX() + 5, bounds.getBottom());
    }
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
    int mirrored = mirrorYPx(y);
    double nrOfOctaves = static_cast<double>(mirrored)/static_cast<double>(octaveHeightPx);
    double freqFactor = std::pow(2, nrOfOctaves);
    return freqFactor * freqBottomScreen;
}

int PianoRoll::getYPxFromFreq(double freq) const {
    double freqFactor = freq / freqBottomScreen;
    double nrOfOctaves = std::log2(freqFactor);
    return mirrorYPx(static_cast<int>(nrOfOctaves * octaveHeightPx));
}

float PianoRoll::getBarExactFromXPx(int px) const {
    return barLeftScreen + static_cast<float>(px) / static_cast<float>(barWidthPx);
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
        Rect bounds = getNoteBounds(note.get());
        if (bounds.contains(px))
            return note.get();
    }
    return nullptr;
}

std::optional<Fraction> PianoRoll::getPotentialRatioAt(Point px) const {
    if (!selectedNote)
        return std::nullopt;
    for (auto& ratio : potentialRatios) {
        auto bounds = getPotentialRatioBounds(ratio);
        if (bounds && bounds.value().contains(px))
            return ratio;
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

void PianoRoll::selectNote(Note* note) {
    selectedNote = note;
    selectedPotentialRatio.reset();
}

void PianoRoll::selectPotentialRatio(Fraction ratio) {
    selectedPotentialRatio = ratio;
}

void PianoRoll::unselectNote() {
    selectedNote = nullptr;
    selectedPotentialRatio.reset();
}

int PianoRoll::mirrorYPx(int y) const {
    return getHeight() - y;
}

Point PianoRoll::mirrorYPx(Point point) const {
    return {point.x, getHeight() - point.y - 1};
}

Rect PianoRoll::mirrorYPx(Rect rect) const {
    return {rect.getX(), getHeight() - rect.getY() - 1 - rect.getHeight(), rect.getWidth(), rect.getHeight()};
}

void PianoRoll::mouseDown(const juce::MouseEvent& event) {
    Point posPx = event.getPosition();
    int nrOfClicks = event.getNumberOfClicks();
    if (nrOfClicks == 1)
        handleSingleClick(posPx);
    if (nrOfClicks == 2)
        handleDoubleClick(posPx);
    repaint();
}

void PianoRoll::handleSingleClick(Point px) {
    if (auto note = getNoteAt(px))
        selectNote(note);
    else if (auto potentialChild = getPotentialRatioAt(px))
        selectPotentialRatio(potentialChild.value());
    else
        unselectNote();
}

void PianoRoll::handleDoubleClick(Point px) {
    auto barSub = getBarSubFromXPx(px.getX());
    if (selectedPotentialRatio) {
        noteRegion.addChildNote(selectedNote, selectedPotentialRatio.value(), 1, barSub, barSub+1);
        return;
    }
    if (selectedNote) {
        noteRegion.deleteNote(selectedNote);
        return;
    }
    auto freq = getFreqFromYPx(px.getY());
    noteRegion.addRootNote(freq, barSub, barSub + 1);
}


