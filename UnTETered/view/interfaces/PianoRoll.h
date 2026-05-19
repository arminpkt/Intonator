//
// Created by Vos on 09/10/2025.
//

#pragma once

#include "PianoRollSettingsBar.h"
#include "../Types.h"
#include "../../logic/NoteRegion.h"
#include "../../PluginProcessor.h"
#include "../../logic/TimelineHelpers.h"

class PianoRoll : public juce::Component, juce::Timer {
public:
    const float NOTE_HEIGHT_PER_OCTAVE = 7.0f/120.0f;
    const float SCROLL_FACTOR = 60.0f;
    const int SETTINGS_BAR_HEIGHT = 30;
    const int ORIENTATION_BAR_HEIGHT = 15;
    const juce::Colour SUB_DIV_LINE_COLOUR = {100, 100, 100};
    const juce::Colour BAR_LINE_COLOUR = {50, 50, 50};
    const juce::Colour NOTE_BASE_COLOUR = {100, 100, 100};
    const juce::Colour NOTE_OUTLINE_COLOUR = {50, 50, 50};
    const juce::Colour FAMILY_BASE_COLOUR = {50, 50, 180};
    const juce::Colour FAMILY_OUTLINE_COLOUR = {50, 50, 50};
    const juce::Colour FAMILY_RATIO_TEXT_COLOUR = {170, 170, 170};
    const juce::Colour SELECTED_BASE_COLOUR = {50, 180, 50};
    const juce::Colour SELECTED_OUTLINE_COLOUR = {50, 50, 50};
    const juce::Colour MULT_SELECTED_BASE_COLOUR = {100, 100, 100};
    const juce::Colour MULT_SELECTED_OUTLINE_COLOUR = {200, 200, 200};
    const juce::Colour INT_RATIO_TEXT_COLOUR = {50, 50, 50};
    const juce::Colour POTENTIAL_RATIO_BASE_COLOUR = juce::Colour::fromRGBA(50, 50, 50, 80);
    const juce::Colour POTENTIAL_RATIO_TEXT_COLOUR = {50, 50, 50};
    const juce::Colour DRAGGED_RECT_BASE_COLOUR = juce::Colour::fromRGBA(50, 50, 50, 50);
    const juce::Colour DRAGGED_RECT_OUTLINE_COLOUR = {150, 200, 150};
    const juce::Colour PLAYHEAD_COLOUR = {255, 255, 255};
    const juce::Colour PASTE_CURSOR_COLOUR = {255, 160, 30};
    const juce::Colour SETTINGS_BACKGROUND_COLOUR = {70, 70, 70};
    const Rect NOTE_DOT_BOX = {0, 0, 4, 4};

    explicit PianoRoll(UnTETeredAudioProcessor& proc);
    ~PianoRoll() override;
    void initialisePotentialRatios();

private:
    void paint(juce::Graphics& g) override;
    static void fillRect(const Rect& rect, juce::Graphics& g);
    static void drawRect(const Rect& rect, juce::Graphics& g);
    static void drawText(const juce::String& text, const Rect& bounds, const juce::Graphics& g, juce::Justification justification);
    void drawBackground(juce::Graphics& g, const Rect& bounds) const;
    void drawBarLines(juce::Graphics& g, const Rect& bounds, bool drawSubDivs) const;
    Fraction getSubDivsFraction() const;
    int getNrOfSubDivs() const;
    void drawNotes(juce::Graphics& g) const;
    void drawNote(const Note* note, const juce::Colour& baseColour, const juce::Colour& outlineColour, juce::Graphics& g) const;
    void drawPotentialRatios(juce::Graphics& g) const;
    void drawRectDragged(juce::Graphics& g) const;
    void drawOrientationBar(juce::Graphics& g) const;
    void drawPlayhead(juce::Graphics& g) const;
    void drawPasteCursorHandle(juce::Graphics& g) const;
    void drawPasteCursorLine(juce::Graphics& g) const;
    void drawSettingsBackground(juce::Graphics& g) const;
    void resized() override;
    float getHueFromYPx(int y) const;
    static float getHueFromFreq(double freq);
    double getFreqFromYPx(int y) const;
    double getFreqFromYPxF(float y) const;
    int getYPxFromFreq(double freq) const;
    float getBarExactFromXPx(int x, bool ignoreBarLeft = false) const;
    float getBarExactFromXPxF(float x, bool ignoreBarLeft = false) const;
    float getBarSubFromXPx(int px, bool ignoreBarLeft = false) const;
    float getBarSubRoundedFromXPx(int px, bool ignoreBarLeft = false) const;
    int getBarFloorFromXPx(int px, bool ignoreBarLeft = false) const;
    int getXPxFromBar(float bar) const;
    Note* getNoteAt(Point px);
    bool referenceExists() const;
    std::optional<Fraction> getPotentialRatioAt(Point px) const;
    Rect getNoteBounds(const Note* note) const;
    std::optional<Rect> getPotentialRatioBounds(Fraction ratio) const;
    std::optional<std::tuple<double, Fraction, double>> getReferenceRefFreqRatioIrratio() const;
    std::optional<double> getReferenceFrequency() const;
    std::vector<double> getPotentialFrequencies(Note* note) const;
    void selectNote(Note* note, Point clickedPos, bool invertIfSelected = false);
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
    void moveExtendShrinkNotes(Point mouseDownPos, Point currentPos);
    void moveExtendShrinkHorizontally(int dX) const;
    void moveVertically(Point currentPos, Point mouseDownPos) const;
    void mouseUp(const juce::MouseEvent& _) override;
    void mouseWheelMove(const juce::MouseEvent& _, const juce::MouseWheelDetails& wheel) override;
    void scroll(PointF deltaXY);
    void clipScreenEdges();
    void handleSingleClick(Point px);
    void handleDoubleClick(Point px);
    void handleShiftSingleClick(Point px);
    bool keyPressed(const juce::KeyPress& key) override;
    void incrementLockYSetting();
    void incrementReferenceSetting();
    void addNoteWithoutReference(double frequency, float start, float end);
    void addNoteWithRefFreq(double refFreq, Fraction ratio, double irratio, float start, float end);
    void deleteNote(Note* note);
    void deleteSelection();
    void copySelectionToClipboard();
    void cutSelection();
    void pasteClipboard();
    void duplicate();
    void selectAll();
    void narrowGrid();
    void widenGrid();
    void tripletGrid();
    Rect getCanvasBounds() const;
    Rect getSettingsBarBounds() const;
    Rect getNoteCanvasBounds() const;
    Rect getOrientationBarBounds() const;

    void handleLockYChanged();
    void handleReferenceChanged();
    void handlePotentialRatiosChanged();
    void handleMonitoringChanged();

    void timerCallback() override;

    void pullStateFromProcessorAndRebuild();
    void pushNoteStateToProcessor() const;
    void pushViewportToProcessor() const;

    static constexpr int MAX_UNDO_STEPS = 50;
    std::vector<std::vector<StoredPianoNote>> undoStack;
    std::vector<std::vector<StoredPianoNote>> redoStack;
    void pushUndoSnapshot();
    void undo();
    void redo();
    bool undoSnapshotTakenForCurrentDrag = false;
    bool noteWasDraggedThisGesture = false;

    static constexpr int PREVIEW_CHANNEL_START  = 13;
    static constexpr int PREVIEW_CHANNEL_COUNT  = 4;
    static constexpr int PREVIEW_DURATION_TICKS = 10; // 1 s at 30 Hz

    struct ActivePreview {
        int midiNote;
        int channel;
        int countdown;
    };

    void startNotePreview(const Note* note);

    void stopAllPreviews();

    bool monitoringEnabled = false;
    std::vector<ActivePreview> activePreviews;
    std::unordered_set<Note*> previewedDuringCurrentDrag;

    // -------------------------------------------------------------------------
    UnTETeredAudioProcessor& processor;
    NoteRegion noteRegion;

    PianoRollSettingsBar settingsBar;
    LockY lockYSetting = locked;
    Reference referenceSetting = selectedNote;
    std::vector<Fraction> potentialRatios;

    float octaveHeightPxF = 200;
    float barWidthPxF = 100;
    double freqBottomScreen = 200;
    float barLeftScreen = 0;

    float playheadBarPos    = 0;
    float pasteCursorBarPos = 0;
    bool  isDraggingPasteCursor = false;
    float blinkPhase = 0.0f;

    std::vector<Note*> notesSelected;
    std::optional<std::tuple<double, Fraction, double>> customReference;
    Note* lockedNoteReference{};
    Note* noteHighlighted{};
    std::optional<Fraction> potentialRatioHighlighted;
    int dragStartOffsetPx{};
    std::vector<std::pair<float, float>> selectedNotesStartsEnds;
    std::vector<double> selectedNotesRefFreqs;
    Note* noteClicked{};
    bool dragRightSideSelectedNote = false;
    bool dragLeftSideSelectedNote  = false;
    std::optional<Rect> draggedRect;
    std::vector<std::unique_ptr<Note>> clipboard;
    Fraction extraGridResolution{1, 1};
    bool gridTripletted = false;

    double cachedPpqPosition = 0.0;
    int    cachedNumerator   = 4;
    int    cachedDenominator = 4;
};