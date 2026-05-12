//
// Created by Armin Peukert on 17.03.26.
//

#pragma once

#include <vector>
#include <juce_core/juce_core.h>
#include "../../logic/Monzo.h"

// A reference-frequency anchor. Never drawn on the piano roll itself;
// exists only so ChildNotes have something to hang their ratios off.
struct StoredMatriarch
{
    double frequency = 440.0;
};

// Every drawable note in the piano roll is a child of exactly one matriarch.
struct StoredPianoNote
{
    float  start          = 0.0f;
    float  end            = 0.0f;
    int    matriarchIndex = -1;      // index into PianoRollState::matriarchs
    std::array<int, primes::PrimeCount> primePowers{};
    double irratio        = 1.0;
};

struct PianoRollState
{
    std::vector<StoredMatriarch> matriarchs;
    std::vector<StoredPianoNote> notes;

    float  octaveHeightPxF  = 120.0f;
    float  barWidthPxF      = 120.0f;
    double freqBottomScreen = 55.0;
    float  barLeftScreen    = 0.0f;
};