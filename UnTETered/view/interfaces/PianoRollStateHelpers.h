//
// Created by Armin Peukert on 17.03.26.
//

#pragma once

#include "../../logic/NoteRegion.h"
#include "PianoRollState.h"
#include "../../logic/Monzo.h"

namespace PianoRollStateHelpers {
    PianoRollState makeStateFromNoteRegion(const NoteRegion& region);
    NoteRegion makeNoteRegionFromState(const PianoRollState& state);
    std::array<int, Primes::PrimeCount> makePrimePowersFromVar(juce::var var);
    juce::var makeVarFromPrimePowers(std::array<int, Primes::PrimeCount> var);
    juce::String makeStringFromIntervals(const std::vector<std::pair<int,int>>& intervals);
    std::vector<std::pair<int,int>> makeIntervalsFromString(const juce::String& str);
    std::tuple<int, int, int> makeTupleFromColour(const juce::Colour& colour);
    juce::Colour makeColourFromTuple(const std::tuple<int, int, int>& tuple);
}