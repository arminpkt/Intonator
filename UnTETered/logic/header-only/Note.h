//
// Created by Vos on 30/09/2025.
//

#pragma once

#include <cmath>
#include <vector>
#include <stdexcept>
#include <juce_audio_processors/juce_audio_processors.h>

#include "Fraction.h"
#include "PitchClass.h"


class ChildNote;

class Note {
public:
    float frequency;
    int start;
    int end;
    std::vector<ChildNote*> children;

    Note(const float freq, const int s, const int e)
        : frequency(freq), start(s), end(e) {}

    /** Computes the interval between f and this note's frequency in semitones.
     *
     * @param f     Reference frequency in Hz
     */
    float getDistanceFrom(const float f) const {
        float ratio = frequency / f;
        float ratio_log = std::log2(ratio);
        return ratio_log * 12;
    }

    /** Computes the interval between the input's frequency and this note's frequency in semitones.
     *
     * @param note  Reference note
     */
    float getDistanceFrom(const Note& note) const {
        return getDistanceFrom(note.frequency);
    }

    // Computes the MIDI value if MIDI were continuous.
    float getPitch() const {
        float distanceFromA440 = getDistanceFrom(440);
        float pitch = distanceFromA440 + 69;
        return pitch;
    }

    // Computes the continuous pitch class, where A -> 0, Bb -> 1, ...
    PitchClass getPitchClass() const {
        float pitch = getPitch();
        return {pitch};
    }

    // Computes the closest MIDI value for this note.
    int getRoundedMidiValue() const {
        float roundedMidiValue = std::round(getPitch());
        if (roundedMidiValue < 0 || roundedMidiValue > 127) {
            throw std::out_of_range("note out of midi range");
        }
        return static_cast<int>(roundedMidiValue);
    }

    /** Computer the pitchend offset in semitones with respect to the input MIDI note value.
     *
     * @param midiNoteValue     The note from which the offset is calculated
     * @return                  The distance in semitones
     */
    float getPitchBendInSemitonesWRT(const int midiNoteValue) const {
        float offsetInSemitones = getPitch() - static_cast<float>(midiNoteValue);
        return offsetInSemitones;
    }

    /** Computes the pitch bend value from the rounded MIDI value for this note.
     *
     * @param bendRange     The range of the pitchbend in semitones
     */
    juce::uint16 getPitchBendValue(const float bendRange = .5) const {
        int roundedMidiValue = getRoundedMidiValue();
        return getPitchBendValueWRT(roundedMidiValue, bendRange);
    }

    /** Computes the pitchbend value to get this note, if it were constructed through pitchbend
     *      from the provided midi note value.
     *
     * @param midiNoteValue     The note from which the pitchbend is calculated
     * @param bendRange         The range of the pitchbend in semitones
     * @return
     */
    juce::uint16 getPitchBendValueWRT(const int midiNoteValue, const float bendRange = .5) const {
        float offsetInSemitones = getPitchBendInSemitonesWRT(midiNoteValue);
        float bendRatio = offsetInSemitones / bendRange;
        float bendValueCont = 8192 + 8192 * bendRatio;
        float roundedBendValue = std::round(bendValueCont);
        return static_cast<juce::uint16>(roundedBendValue);
    }

    /** Moves this note up or down by octaves to be closest to other.
     *
     * @param other     The note to move this note close to
     */
    void octavateClosestTo(const Note& other) {
        float distanceInSemitones = getDistanceFrom(other);
        float distanceInOctaves = distanceInSemitones / 12;
        int numberOfOctavesToOctavate = -static_cast<int>(std::round(distanceInOctaves));
        DBG(distanceInSemitones);
        DBG(distanceInOctaves);
        DBG(numberOfOctavesToOctavate);
        Fraction f(1, 1);
        if (numberOfOctavesToOctavate > 0) {
            int num = static_cast<int>(std::pow(2, numberOfOctavesToOctavate));
            f = Fraction(num, 1);
        } else {
            int den = static_cast<int>(std::pow(2, -numberOfOctavesToOctavate));
            f = Fraction(1, den);
        }

        *this *= f;
    }

    bool operator<(const Note& other) const {
        return frequency < other.frequency;
    }

    virtual ~Note() = default;
    virtual void recalculate() = 0;
    virtual Note& operator*=(const Fraction& f) = 0;
    virtual Note& operator/=(const Fraction& f) = 0;
    virtual Note& operator*=(const float& i) = 0;
    virtual Note& operator/=(const float& i) = 0;
    Note& operator*=(const int& i) {
        *this *= Fraction(i, 1);
        return *this;
    }
    Note& operator/=(const int& i) {
        *this /= Fraction(i, 1);
        return *this;
    }
};

class ChildNote : public Note {
public:
    const Note* parent;
    Fraction ratio;
    float irratio;

    ChildNote(Note& p, Fraction r, int s, int e)
    : Note(p.frequency * r.toFloat(), s, e), parent(&p), ratio(r), irratio(1) {}

    ChildNote(Note& p, float i, int s, int e)
    : Note(p.frequency * i, s, e), parent(&p), ratio(Fraction(1, 1)), irratio(i) {}

    ChildNote(Note& p, Fraction r, float i, int s, int e)
    : Note(p.frequency * r.toFloat() * i, s, e), parent(&p), ratio(r), irratio(i) {}


    void recalculate() override {
        frequency = parent->frequency * ratio.toFloat() * irratio;
        for (const auto& note : children)
            note->recalculate();
    }

    // Compound assignment
    Note& operator*=(const Fraction& f) override {
        ratio *= f;
        recalculate();
        return *this;
    }
    Note& operator/=(const Fraction& f) override {
        ratio /= f;
        recalculate();
        return *this;
    }
    Note& operator*=(const float& i) override {
        irratio *= i;
        recalculate();
        return *this;
    }
    Note& operator/=(const float& i) override {
        irratio /= i;
        recalculate();
        return *this;
    }
};

class RootNote : public Note {
public:
    RootNote(float freq, int s, int e)
        : Note(freq, s, e) {
    }

    void recalculate() override {
        for (const auto& note : children)
            note->recalculate();
    }

    // Compound assignment
    Note& operator*=(const Fraction& f) override {
        frequency *= f.toFloat();
        recalculate();
        return *this;
    }
    Note& operator/=(const Fraction& f) override {
        frequency /= f.toFloat();
        recalculate();
        return *this;
    }
    Note& operator*=(const float& i) override {
        frequency *= i;
        recalculate();
        return *this;
    }
    Note& operator/=(const float& i) override {
        frequency /= i;
        recalculate();
        return *this;
    }
};