//
// Created by Vos on 09/10/2025.
//

#pragma once

#include "../Types.h"
#include "../../logic/NoteRegion.h"
#include "../../PluginProcessor.h"

class PianoRoll : public juce::Component {
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
    void drawNote(const Note* note, juce::Colour baseColour, juce::Colour edgeColour, juce::Graphics& g) const;
    void drawPotentialRatios(juce::Graphics& g) const;
    void drawRectDragged(juce::Graphics& g) const;
    float getHueFromYPx(int y) const;
    static float getHueFromFreq(double freq) ;
    double getFreqFromYPx(int y) const;
    double getFreqFromYPxF(float y) const;
    int getYPxFromFreq(double freq) const;
    float getBarExactFromXPx(int x) const;
    float getBarExactFromXPxF(float x) const;
    int getXPxFromBar(float bar) const;
    float getBarSubFromXPx(int px) const;
    float getBarSubRoundedFromXPx(int px) const;
    int getBarFloorFromXPx(int px) const;
    Note* getNoteAt(Point px) const;
    std::optional<Fraction> getPotentialRatioAt(Point px) const;
    Rect getNoteBounds(const Note* note) const;
    std::optional<Rect> getPotentialRatioBounds(Fraction ratio) const;
    std::vector<double> getPotentialFrequencies(Note* note) const;
    void selectNote(Note* note, Point clickedPos, bool invertIfSelected);
    std::optional<size_t> indexOfSelection(const Note* note) const;
    void unselectNote(const Note* note);
    float mirrorYPx(float y, float axis = 0.5f) const;
    int mirrorYPx(int y, float axis = 0.5f) const;
    Point mirrorYPx(Point point, float axis = 0.5f) const;
    Rect mirrorYPx(Rect rect, float axis = 0.5f) const;

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseMagnify(const juce::MouseEvent& event, float scaleFactor) override;
    void dragRectangle(Point mouseDownPos, Point currentPos);
    void moveExtendShrinkNotes(Point mouseDownPos, Point currentPos) const;
    void mouseUp(const juce::MouseEvent& _) override;
    void mouseWheelMove(const juce::MouseEvent& _, const juce::MouseWheelDetails& wheel) override;
    void scroll(PointF deltaXY);
    void clipScreenEdges();
    void handleSingleClick(Point px);
    void handleDoubleClick(Point px);
    void handleShiftSingleClick(Point px);
    bool keyPressed(const juce::KeyPress& key) override;
    void deleteNote(Note* note);
    void deleteSelection();

private:
    UnTETeredAudioProcessor& processor;
    NoteRegion noteRegion;
    float octaveHeightPxF = 200;
    float barWidthPxF = 100;
    double freqBottomScreen = 200;
    float barLeftScreen = 0;
    std::vector<Fraction> potentialRatios;
    std::vector<Note*> notesSelected;
    Note* noteHighlighted{};
    std::optional<Fraction> potentialRatioHighlighted;
    int dragStartOffsetPx{};
    std::vector<std::pair<float, float>> selectedNotesStartsEnds;
    bool clickedNote = false;
    bool dragRightSideSelectedNote = false;
    bool dragLeftSideSelectedNote = false;
    std::optional<Rect> draggedRect;
};
