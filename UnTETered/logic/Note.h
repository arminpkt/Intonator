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
    float start;
    float end;
    std::vector<ChildNote*> children;

    Note(const double freq, const int s, const int e)
        : Note(freq, static_cast<float>(s), static_cast<float>(e)) {}

    Note(const double freq, const float s, const float e)
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
    [[nodiscard]] juce::uint16 getPitchBendValue(const double bendRange = 1) const {
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
    [[nodiscard]] juce::uint16 getPitchBendValueWRT(const int midiNoteValue, const double bendRange = 1) const {
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

    [[nodiscard]] float getHue() const {
        auto pitchClass = getPitchClass();
        float hue = static_cast<float>(pitchClass.value) / 12.f;
        return hue;
    }

    bool operator<(const Note& other) const {
        return frequency < other.frequency;
    }

    virtual ~Note() = default;
    virtual void recalculate() = 0;
    virtual std::vector<Note*> getAncestry() = 0;
    virtual std::optional<Fraction> getRatioToAncestor(Note* ancestor) = 0;
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

    void disown(const ChildNote* toDisown) {
        for (size_t i = 0; i < children.size(); ++i) {
            if (children[i] == toDisown) {
                children.erase(children.begin() + static_cast<long int>(i));
                return;
            }
        }
    }
};

struct ChildNote : Note {
    Note* parent;
    Fraction ratio;
    double irratio;

    ChildNote(Note* p, const Fraction& r) : ChildNote(p, r, p->start, p->end) {}

    ChildNote(Note* p, const Fraction& r, const float s, const float e)
    : ChildNote(p, r, 1, s, e) {}

    ChildNote(Note* p, const double i, const float s, const float e)
    : ChildNote(p, {1, 1}, i, s, e) {}

    ChildNote(Note* p, const Fraction& r, const double i, const float s, const float e)
    : Note(p->frequency * static_cast<double>(r) * i, s, e), parent(p), ratio(r), irratio(i) {
        p->children.push_back(this);
    }

    ~ChildNote() override {
        parent->disown(this);
    }

    // void abandonChildren() const {
    //     for (auto& child : children) {
    //         child->parent = parent;
    //         child->ratio = child->ratio * ratio;
    //         parent->children.push_back(child);
    //     }
    //     parent->disown(this);
    // }

    std::vector<Note*> getAncestry() override {
        std::vector<Note*> ancestry = {this};
        std::vector<Note*> parentAncestry = parent->getAncestry();
        ancestry.insert(ancestry.end(), parentAncestry.begin(), parentAncestry.end());
        return ancestry;
    }

    std::optional<Fraction> getRatioToAncestor(Note* ancestor) override {
        if (ancestor == this)
            return Fraction{1, 1};
        if (auto parentRatio = parent->getRatioToAncestor(ancestor))
            return ratio * parentRatio.value();
        return std::nullopt;
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

struct RootNote : Note {
    RootNote(const double freq, const float s, const float e)
        : Note(freq, s, e) {
    }

    void recalculate() override {
        for (const auto& note : children)
            note->recalculate();
    }

    std::vector<Note*> getAncestry() override {
        return {this};
    }

    std::optional<Fraction> getRatioToAncestor(Note* ancestor) override {
        if (this == ancestor)
            return Fraction{1, 1};
        return std::nullopt;
    }

    void setFrequency(const double f) {
        frequency = f;
        recalculate();
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