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


struct ChildNote;

struct Note {
    double frequency;
    int start;
    int end;
    std::unordered_set<ChildNote*> children;

    Note(const double freq, const int s, const int e)
        : frequency(freq), start(s), end(e) {}

    /** Computes the interval between f and this note's frequency in semitones.
     *
     * @param f     Reference frequency in Hz
     */
    [[nodiscard]] double getDistanceFrom(const double f) const {
        double ratio = frequency / f;
        double ratio_log = std::log2(ratio);
        return ratio_log * 12;
    }

    /** Computes the interval between the input's frequency and this note's frequency in semitones.
     *
     * @param note  Reference note
     */
    [[nodiscard]] double getDistanceFrom(const Note& note) const {
        return getDistanceFrom(note.frequency);
    }

    // Computes the MIDI value if MIDI were continuous.
    [[nodiscard]] double getPitch() const {
        double distanceFromA440 = getDistanceFrom(440);
        double pitch = distanceFromA440 + 69;
        return pitch;
    }

    // Computes the continuous pitch class, where A -> 0, Bb -> 1, ...
    [[nodiscard]] PitchClass getPitchClass() const {
        double pitch = getPitch();
        return {pitch};
    }

    // Computes the closest MIDI value for this note.
    [[nodiscard]] int getRoundedMidiValue() const {
        double roundedMidiValue = std::round(getPitch());
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
    [[nodiscard]] double getPitchBendInSemitonesWRT(const int midiNoteValue) const {
        double offsetInSemitones = getPitch() - static_cast<double>(midiNoteValue);
        return offsetInSemitones;
    }

    /** Computes the pitch bend value from the rounded MIDI value for this note.
     *
     * @param bendRange     The range of the pitchbend in semitones
     */
    [[nodiscard]] juce::uint16 getPitchBendValue(const double bendRange = .5) const {
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
    [[nodiscard]] juce::uint16 getPitchBendValueWRT(const int midiNoteValue, const double bendRange = .5) const {
        double offsetInSemitones = getPitchBendInSemitonesWRT(midiNoteValue);
        double bendRatio = offsetInSemitones / bendRange;
        double bendValueCont = 8192 + 8192 * bendRatio;
        double roundedBendValue = std::round(bendValueCont);
        return static_cast<juce::uint16>(roundedBendValue);
    }

    /** Moves this note up or down by octaves to be closest to other.
     *
     * @param other     The note to move this note close to
     */
    void octavateClosestTo(const Note& other) {
        double distanceInSemitones = getDistanceFrom(other);
        double distanceInOctaves = distanceInSemitones / 12;
        int numberOfOctavesToOctavate = -static_cast<int>(std::round(distanceInOctaves));
        DBG(distanceInSemitones);
        DBG(distanceInOctaves);
        DBG(numberOfOctavesToOctavate);
        if (numberOfOctavesToOctavate > 0) {
            int num = static_cast<int>(std::pow(2, numberOfOctavesToOctavate));
            *this *= {num, 1};
        } else {
            int den = static_cast<int>(std::pow(2, -numberOfOctavesToOctavate));
            *this *= {1, den};
        }
    }

    bool operator<(const Note& other) const {
        return frequency < other.frequency;
    }

    virtual ~Note() = default;
    virtual void recalculate() = 0;
    virtual Note& operator*=(const Fraction& f) = 0;
    virtual Note& operator/=(const Fraction& f) = 0;
    virtual Note& operator*=(const double& i) = 0;
    virtual Note& operator/=(const double& i) = 0;
    Note& operator*=(const int& i) {
        *this *= Fraction(i, 1);
        return *this;
    }
    Note& operator/=(const int& i) {
        *this /= Fraction(i, 1);
        return *this;
    }
};

struct ChildNote : Note {
    Note* parent;
    Fraction ratio;
    double irratio;

    ChildNote(Note& p, const Fraction& r, const int s, const int e)
    : ChildNote(p, r, 1, s, e) {}

    ChildNote(Note& p, const double i, const int s, const int e)
    : ChildNote(p, {1, 1}, i, s, e) {}

    ChildNote(Note& p, const Fraction& r, const double i, const int s, const int e)
    : Note(p.frequency * static_cast<double>(r) * i, s, e), parent(&p), ratio(r), irratio(i) {
        p.children.insert(this);
    }

    ~ChildNote() override {
        parent->children.erase(this);
    }

    void recalculate() override {
        frequency = parent->frequency * static_cast<double>(ratio) * irratio;
        for (const auto& note : children)
            note->recalculate();
    }

    Note& operator*=(const Fraction& f) override {
        ratio = ratio * f;
        recalculate();
        return *this;
    }
    Note& operator/=(const Fraction& f) override {
        ratio = ratio / f;
        recalculate();
        return *this;
    }
    Note& operator*=(const double& i) override {
        irratio *= i;
        recalculate();
        return *this;
    }
    Note& operator/=(const double& i) override {
        irratio /= i;
        recalculate();
        return *this;
    }
};

class RootNote : public Note {
public:
    RootNote(const double freq, const int s, const int e)
        : Note(freq, s, e) {
    }

    void recalculate() override {
        for (const auto& note : children)
            note->recalculate();
    }

    // Compound assignment
    Note& operator*=(const Fraction& f) override {
        frequency *= static_cast<double>(f);
        recalculate();
        return *this;
    }
    Note& operator/=(const Fraction& f) override {
        frequency /= static_cast<double>(f);
        recalculate();
        return *this;
    }
    Note& operator*=(const double& i) override {
        frequency *= i;
        recalculate();
        return *this;
    }
    Note& operator/=(const double& i) override {
        frequency /= i;
        recalculate();
        return *this;
    }
};