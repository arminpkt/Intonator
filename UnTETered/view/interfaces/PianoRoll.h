//
// Created by Vos on 09/10/2025.
//

#pragma once

#include "../Types.h"
#include "../../logic/NoteRegion.h"
#include "../../PluginProcessor.h"

class PianoRoll : public juce::Component {
public:
    float NOTE_HEIGHT_PER_OCTAVE = 1.0f/24.0f;
    explicit PianoRoll(UnTETeredAudioProcessor& proc);
    void initialisePotentialRatios();
    ~PianoRoll() override = default;

    void paint(juce::Graphics& g) override;
    void fillRect(Rect& rect, juce::Graphics& g) const;
    void drawRect(Rect& rect, juce::Graphics& g) const;
    void drawText(juce::String& text, Rect& bounds, juce::Graphics& g) const;
    void drawBackground(juce::Graphics& g) const;
    void drawBarLines(juce::Graphics& g) const;
    int getNrOfSubDivs() const;
    void drawNotes(juce::Graphics& g) const;
    void drawPotentialRatios(juce::Graphics& g) const;
    void drawRectDragged(juce::Graphics& g) const;
    float getHueFromYPx(int y) const;
    static float getHueFromFreq(double freq) ;
    double getFreqFromYPx(int y) const;
    int getYPxFromFreq(double freq) const;
    float getBarExactFromXPx(int px) const;
    int getXPxFromBar(float bar) const;
    float getBarSubFromXPx(int px) const;
    int getBarFloorFromXPx(int px) const;
    Note* getNoteAt(Point px) const;
    std::optional<Fraction> getPotentialRatioAt(Point px) const;
    Rect getNoteBounds(Note* note) const;
    std::optional<Rect> getPotentialRatioBounds(Fraction ratio) const;
    std::vector<double> getPotentialFrequencies(Note* note) const;
    void selectNote(Note* note, Point clickedPos);
    void selectPotentialRatio(Fraction ratio);
    void unselectNote();
    int mirrorYPx(int y) const;
    Point mirrorYPx(Point point) const;
    Rect mirrorYPx(Rect rect) const;

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& _) override;
    void handleSingleClick(Point px);
    void handleDoubleClick(Point px);
    bool keyPressed(const juce::KeyPress& key) override;
    void deleteSelection();

private:
    UnTETeredAudioProcessor& processor;
    NoteRegion noteRegion;
    int octaveHeightPx = 200;
    int barWidthPx = 100;
    double freqBottomScreen = 200;
    float barLeftScreen = 0;
    std::vector<Fraction> potentialRatios;
    Note* selectedNote{};
    float selectedNoteStart{};
    float selectedNoteEnd{};
    bool dragRightSideSelectedNote = false;
    bool dragLeftSideSelectedNote = false;
    std::vector<Note*> selectedNotesDragged;
    std::optional<Rect> draggedRect;
    std::optional<Fraction> selectedPotentialRatio;
};
