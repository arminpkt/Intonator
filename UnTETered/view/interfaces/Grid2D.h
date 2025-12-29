//
// Created by Vos on 15/11/2025.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_animation/juce_animation.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

#include "../../logic/util.h"
#include "../../logic/NoteRegion.h"
#include "../../logic/header-only/Fraction.h"
#include "../../PluginProcessor.h"

class Grid2D: public juce::Component
{
    public:
    Grid2D(int rows, int cols, Fraction horizontal, Fraction vertical, float refFreq,
    UnTETeredAudioProcessor& processor, juce::VBlankAnimatorUpdater& updater);
    ~Grid2D() override = default;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    bool keyPressed(const juce::KeyPress& event) override;

    void activateTransition();
    void addAnimatorToUpdater(juce::AnimatorUpdater& updater) const;

    static juce::Colour getColourForPitchClass(PitchClass pitchClass, bool selected);

    private:
    int numRows;
    int numCols;
    Fraction intervalHorizontal;
    Fraction intervalVertical;
    juce::Point<int> const refCoordinatesCellTarget;
    juce::Point<int> refCoordinatesCellActual;
    juce::Point<int> paintingOffsetInitial;
    juce::Point<int> paintingOffset;
    std::unique_ptr<Note> refNote;

    std::vector<std::vector<bool>> currentCellStates;
    std::vector<std::vector<bool>> nextCellStates;
    std::vector<std::unique_ptr<Note>> currentActiveNotes;
    std::vector<std::unique_ptr<Note>> nextActiveNotes;
    std::vector<Note*> currentActiveNotesOrdered;
    std::vector<Note*> nextActiveNotesOrdered;
    UnTETeredAudioProcessor& processorRef;

    juce::Animator gridTranspositionAnimator;

    juce::Rectangle<int> getCellBounds(juce::Point<int> coordinates) const;
    juce::Point<int> getCellFromPx(const juce::Point<int>& px) const;
    juce::Point<int> mirrorY(juce::Point<int> point) const;
    juce::Point<int> mirrorYPx(juce::Point<int> point) const;
    std::unique_ptr<Note> generateNote(juce::Point<int> coordinates) const;
    juce::Colour getColourForCoordinates(juce::Point<int> coordinates) const;
    void updateNextActive();
    juce::Point<int> calculateCenterOfGravityCell() const;

    std::vector<juce::MidiMessage> midiMessages;
};
