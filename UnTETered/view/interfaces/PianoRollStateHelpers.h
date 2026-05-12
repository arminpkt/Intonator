//
// Created by Armin Peukert on 17.03.26.
//

#pragma once

#include "../../logic/NoteRegion.h"
#include "PianoRollState.h"
#include "../../logic/Monzo.h"

PianoRollState makeStateFromNoteRegion(const NoteRegion& region);
NoteRegion makeNoteRegionFromState(const PianoRollState& state, float pitchBendRange = 2.0f);
std::array<int, primes::PrimeCount> makePrimePowersFromVar(juce::var var);
juce::var makeVarFromPrimePowers(std::array<int, primes::PrimeCount> var);