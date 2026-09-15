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


const juce::String NOTE_NAMES[] = {
    "A", "Bb", "B", "C", "Db", "D", "Eb", "E", "F", "F#", "G", "Ab"
};

struct Note {
    double referenceFrequency;

    Fraction ratio;
    double irratio;
    float start;
    float end;

    Note(const double ref, Fraction r, double i, const float s, const float e)
        : referenceFrequency(ref), ratio(r), irratio(i), start(s), end(e) {}

    /** Computes the interval between f and this note's frequency in semitones.
     *
     * @param f     Reference frequency in Hz
     */
    [[nodiscard]] double getDistanceFrom(const double f) const {
        return getDistance(f, getFrequency());
    }

    /** Computes the interval between the input's frequency and this note's frequency in semitones.
     *
     * @param note  Reference note
     */
    [[nodiscard]] double getDistanceFrom(const Note& note) const {
        return getDistanceFrom(note.getFrequency());
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

    // Computes cents from closest 12TET note, range -50 to 50.
    [[nodiscard]] int getCentOffset() const {
        auto pitch = getPitch();
        auto rounded = std::round(pitch);
        auto offset = pitch - rounded;
        return static_cast<int>(offset * 100);
    }

    /** Computer the pitchbend offset in semitones with respect to the input MIDI note value.
     *
     * @param midiNoteValue     The note from which the offset is calculated
     * @return                  The distance in semitones
     */
    [[nodiscard]] double getPitchBendInSemitonesWRT(const int midiNoteValue) const {
        double offsetInSemitones = getPitch() - static_cast<double>(midiNoteValue);
        return offsetInSemitones;
    }

    /** Computes the pitchbend value from the rounded MIDI value for this note.
     *
     * @param bendRange     The range of the pitchbend in semitones
     */
    [[nodiscard]] juce::uint16 getPitchBendValue(const double bendRange) const {
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
    [[nodiscard]] juce::uint16 getPitchBendValueWRT(const int midiNoteValue, const double bendRange) const {
        double offsetInSemitones = getPitchBendInSemitonesWRT(midiNoteValue);
        double bendRatio = offsetInSemitones / bendRange;
        double bendValueCont = 8192 + 8192 * bendRatio;
        double roundedBendValue = std::round(bendValueCont);
        return static_cast<juce::uint16>(roundedBendValue);
    }

    /** Moves this note up or down by octaves to be closest to `other`.
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

    void roundReferenceTo12TET() {
        auto semitonesTo440 = getDistance(referenceFrequency, 440);
        auto semitonesToClosest12TET = semitonesTo440 - std::round(semitonesTo440);
        auto factor = std::pow(2, semitonesToClosest12TET / 12);
        referenceFrequency *= factor;
    }

    [[nodiscard]] float getHue() const {
        auto pitchClass = getPitchClass();
        float hue = static_cast<float>(pitchClass.value) / 12.f;
        return hue;
    }

    [[nodiscard]] double getFrequency() const {
        return static_cast<double>(ratio) * referenceFrequency;
    }

    [[nodiscard]] juce::String getNoteName() const {
        auto pc = getPitchClass();
        auto index = pc.getRoundedValue() % 12;
        return NOTE_NAMES[index];
    }

    [[nodiscard]] juce::String getAbsoluteInfo() const {
        juce::String noteName = getNoteName();
        auto centOffset = getCentOffset();
        juce::String connector = centOffset >= 0 ? " + " : " - ";
        juce::String centString = juce::String{std::abs(centOffset)} + "ct";

        return noteName + connector + centString;
    }

    bool isFamiliarWith(const Note* note) const {
        return std::abs(note->referenceFrequency - referenceFrequency) < 0.000001;
    }

    bool operator<(const Note& other) const {
        return getFrequency() < other.getFrequency();
    }

    Note& operator*=(const Fraction& f) {
        ratio = ratio * f;
        return *this;
    }
    Note& operator/=(const Fraction& f) {
        ratio = ratio / f;
        return *this;
    }
    Note& operator*=(const double& i) {
        irratio *= i;
        return *this;
    }
    Note& operator/=(const double& i) {
        irratio /= i;
        return *this;
    }
    Note& operator*=(const int& i) {
        *this *= Fraction(i, 1);
        return *this;
    }
    Note& operator/=(const int& i) {
        *this /= Fraction(i, 1);
        return *this;
    }

    [[nodiscard]] static double getDistance(const double from, const double to) {
        double ratio = to / from;
        double ratioLog = std::log2(ratio);
        return ratioLog * 12;
    }
};