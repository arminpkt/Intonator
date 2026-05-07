//
// Created by Vos on 09/10/2025.
//

#pragma once

#include "../Types.h"
#include "../../logic/NoteRegion.h"
#include "../../PluginProcessor.h"
#include "../../logic/TimelineHelpers.h"

class PianoRoll : public juce::Component, juce::Timer {
public:
    const float NOTE_HEIGHT_PER_OCTAVE = 1.0f/20.0f;
    const float SCROLL_FACTOR = 60.0f;
    explicit PianoRoll(UnTETeredAudioProcessor& proc);
    void initialisePotentialRatios();
    ~PianoRoll() override = default;

    void paint(juce::Graphics& g) override;
    static void fillRect(const Rect& rect, juce::Graphics& g);
    static void drawRect(const Rect& rect, juce::Graphics& g);
    static void drawText(const juce::String& text, const Rect& bounds, const juce::Graphics& g);
    void drawBackground(juce::Graphics& g) const;
    void drawBarLines(juce::Graphics& g) const;
    int getNrOfSubDivs() const;
    void drawNotes(juce::Graphics& g) const;
    void drawNote(const ChildNote* note, juce::Colour baseColour, juce::Colour edgeColour, juce::Graphics& g) const;
    void drawPotentialRatios(juce::Graphics& g) const;
    void drawRectDragged(juce::Graphics& g) const;
    void drawPlayhead(juce::Graphics& g) const;
    float getHueFromYPx(int y) const;
    static float getHueFromFreq(double freq) ;
    double getFreqFromYPx(int y, bool ignoreFreqBottomScreen = false) const;
    double getFreqFromYPxF(float y, bool ignoreFreqBottomScreen = false) const;
    int getYPxFromFreq(double freq) const;
    float getBarExactFromXPx(int x, bool ignoreBarLeft = false) const;
    float getBarExactFromXPxF(float x, bool ignoreBarLeft = false) const;
    float getBarSubFromXPx(int px, bool ignoreBarLeft = false) const;
    float getBarSubRoundedFromXPx(int px, bool ignoreBarLeft = false) const;
    int getBarFloorFromXPx(int px, bool ignoreBarLeft = false) const;
    int getXPxFromBar(float bar) const;
    ChildNote* getNoteAt(Point px) const;
    std::optional<Fraction> getPotentialRatioAt(Point px) const;
    Rect getNoteBounds(const ChildNote* note) const;
    std::optional<Rect> getPotentialRatioBounds(Fraction ratio) const;
    std::vector<double> getPotentialFrequencies(ChildNote* note) const;
    void selectNote(ChildNote* note, Point clickedPos, bool invertIfSelected);
    std::optional<size_t> indexOfSelection(const ChildNote* note) const;
    void unselectNote(const ChildNote* note);

    /** Mirrors y coordinate interpreted as px value around the axis.
     *
     * @param y     Px value to mirror
     * @param axis  0 = bottom, 1 = top, 0.5 = middle = default
     */
    float mirrorYPx(float y, float axis = 0.5f) const;
    int mirrorYPx(int y, float axis = 0.5f) const;
    Point mirrorYPx(Point point, float axis = 0.5f) const;
    Rect mirrorYPx(Rect rect, float axis = 0.5f) const;

    void setPlayheadPosFromPoint(Point);

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseMagnify(const juce::MouseEvent& event, float scaleFactor) override;
    void dragRectangle(Point mouseDownPos, Point currentPos);
    void moveExtendShrinkNotes(Point mouseDownPos, Point currentPos) const;
    void moveExtendShrinkHorizontally(int dX) const;
    void moveVertically(Point mouseDownPos) const;
    void mouseUp(const juce::MouseEvent& _) override;
    void mouseWheelMove(const juce::MouseEvent& _, const juce::MouseWheelDetails& wheel) override;
    void scroll(PointF deltaXY);
    void clipScreenEdges();
    void handleSingleClick(Point px);
    void handleDoubleClick(Point px);
    void handleShiftSingleClick(Point px);
    bool keyPressed(const juce::KeyPress& key) override;
    void addNoteWithoutReference(double frequency, float start, float end);
    void addNoteWithReference(ChildNote* parent, Fraction ratio, double irratio, float start, float end);
    void deleteNote(ChildNote* note);
    void deleteSelection();

    void timerCallback() override;

private:
    void pullStateFromProcessorAndRebuild();
    void pushStateToProcessor() const;

    UnTETeredAudioProcessor& processor;
    NoteRegion noteRegion;
    float octaveHeightPxF = 200;
    float barWidthPxF = 100;
    double freqBottomScreen = 200;
    float barLeftScreen = 0;
    float playheadBarPos = 0;
    std::vector<Fraction> potentialRatios;
    std::vector<ChildNote*> notesSelected;
    ChildNote* noteHighlighted{};
    std::optional<Fraction> potentialRatioHighlighted;
    int dragStartOffsetPx{};
    std::vector<std::pair<float, float>> selectedNotesStartsEnds;
    bool clickedNote = false;
    bool dragRightSideSelectedNote = false;
    bool dragLeftSideSelectedNote = false;
    std::optional<Rect> draggedRect;

    double cachedPpqPosition = 0.0;
    int cachedNumerator = 4;
    int cachedDenominator = 4;
};
