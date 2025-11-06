//
// Created by Vos on 30/09/2025.
//

#pragma once
#include <cmath>
#include <vector>
#include <memory>


class ChildNote;

class Note {
public:
    float frequency;
    int start;
    int end;
    std::vector<std::unique_ptr<ChildNote>> children;

    Note(float freq, int s, int e)
        : frequency(freq), start(s), end(e) {}

    virtual ~Note() = default;
    virtual void dummy() = 0;

    /** Computes the interval between f and this note's frequency in semitones.
     *
     * @param f     Reference frequency in Hz
     */
    float getDistanceFrom(const float f) const {
        float ratio = frequency / f;
        float ratio_log = std::log2(ratio);
        return ratio_log * 12;
    }

    // Computes the MIDI value if MIDI were continuous.
    float getPitch() const {
        float distanceFromA440 = getDistanceFrom(440);
        float pitch = distanceFromA440 - 69;
        return pitch;
    }

    // Computes the continuous pitch class, where A -> 0, Bb -> 1, ...
    float getPitchClass() const {
        float pitch = getPitch();
        return std::fmodf(pitch, 12);
    }

    // Computes the closest MIDI value for this note.
    int getRoundedMidiValue() const {
        float roundedMidiValue = std::round(getPitch());
        return static_cast<int>(roundedMidiValue);
    }

    /** Computes the pitch bend value from the rounded MIDI value for this note.
     *
     * @param bendRange     The pitch bend range in semitones
     */
    int getPitchBendValue(const float bendRange = 2) const {
        float offsetInSemitones = getPitch() - static_cast<float>(getRoundedMidiValue());
        float bendRatio = offsetInSemitones / bendRange;
        float bendValueCont = 8192 + 8192 * bendRatio;
        float roundedBendValue = std::round(bendValueCont);
        return static_cast<int>(roundedBendValue);
    }

    bool operator<(const Note& other) const {
        return frequency < other.frequency;
    }
};

class ChildNote : public Note {
public:
    const Note* parent;
    float ratio;

    ChildNote(const Note& p, float r, int s, int e)
        : Note(p.frequency * r, s, e), parent(&p), ratio(r) {}
};

class RootNote : public Note {
public:
    RootNote(float freq, int s, int e)
        : Note(freq, s, e) {
    }
    void dummy() override {}
};