//
// Created by Vos on 15/11/2025.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_animation/juce_animation.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

#include "../../logic/util.h"
#include "../../logic/Fraction.h"
#include "../../logic/Note.h"
#include "../../PluginProcessor.h"

using Point = juce::Point<int>;
using PointF = juce::Point<float>;
using Rect = juce::Rectangle<int>;
using RectF = juce::Rectangle<float>;
using Kernel = std::vector<std::vector<std::unique_ptr<Note>>>;

struct PointHash {
    std::size_t operator()(const Point& p) const {
        return std::hash<int>{}(p.x) ^ (std::hash<int>{}(p.y) << 1);
    }
};

using PointSet = std::unordered_set<Point, PointHash>;

static float MAX_FREQ_MIDDLE = 500;
static float MIN_FREQ_MIDDLE = 100;

class Grid2D: public juce::Component {
public:
    Grid2D(const Point& dim, const Fraction& horizontal, const Fraction& vertical, double freqOr,
    UnTETeredAudioProcessor& proc, juce::VBlankAnimatorUpdater& updater);
    ~Grid2D() override = default;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    bool keyPressed(const juce::KeyPress& event) override;

    void activateTransition();
    void calibrateGrid();
    void octavateGridDown();
    void octavateGridUp();

    static juce::Colour getColourForPitchClass(PitchClass pitchClass, bool selected);
    static Kernel createEmptyKernel(Point dim);

private:
    UnTETeredAudioProcessor& processor;
    const Point dimScreenCells;
    const Rect boundsScreenCells;
    const Point dimKernelCells;
    const Rect boundsKernelCells;
    const Fraction intervalHorizontal;
    const Fraction intervalVertical;
    RootNote noteOrigin;
    const Point middleCellScreen;
    Point offsetFromOriginGrid;
    std::array<std::pair<PointSet, char>, 91> saves;

    Kernel kernel;
    PointSet activeCellsGrid;
    PointSet selectedCellsGrid;
    std::vector<Note*> activeNotes;

    juce::Animator gridTranspositionAnimator;
    Point paintingOffsetPxInitial;
    Point paintingOffsetPx;

    Rect getPxBoundsFromCellScreen(const Point& inputCellScreen) const;
    Rect getPxBoundsFromCellKernel(const Point& inputCellKernel) const;
    PointF getDimCellPxFloat() const;
    Point getCellScreenFromPx(const Point& inputPx) const;
    Point getCellKernelFromPx(const Point& inputPx) const;
    Point getCellGridFromPx(const Point& inputPx) const;
    Point getCellKernelFromScreen(const Point& inputCellScreen) const;
    Point getCellScreenFromKernel(const Point& inputCellKernel) const;
    Point getCellGridFromScreen(const Point& inputCellScreen) const;
    Point getCellScreenFromGrid(const Point& inputCellGrid) const;
    Point getCellGridFromKernel(const Point& inputCellKernel) const;
    Point getCellKernelFromGrid(const Point& inputCellGrid) const;
    Note* getNoteFromScreen(Point point, bool cond);
    Note* getNoteFromKernel(const Point& cellKernel, bool reset);
    Note* getNoteFromGrid(const Point& cellGrid, bool reset);
    juce::Colour getColourForCellKernel(const Point& cellKernel);
    Rect mirrorYPx(Rect rect) const;
    Point calculateCenterOfGravityOffsetCell() const;
    void transposeGrid();
    Point mirrorYPx(Point point) const;
};
