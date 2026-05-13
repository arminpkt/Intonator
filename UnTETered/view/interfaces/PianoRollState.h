//
// Created by Armin Peukert on 17.03.26.
//

#pragma once

#include <vector>
#include <juce_core/juce_core.h>
#include "../../logic/Monzo.h"


// Every drawable note in the piano roll is a child of exactly one matriarch.
struct StoredPianoNote
{
    double ref            = 0.0;
    float  start          = 0.0f;
    float  end            = 0.0f;
    std::array<int, primes::PrimeCount> primePowers{};
    double irratio        = 1.0;
};

struct PianoRollState
{
    std::vector<StoredPianoNote> notes;

    float  octaveHeightPxF  = 120.0f;
    float  barWidthPxF      = 120.0f;
    double freqBottomScreen = 55.0;
    float  barLeftScreen    = 0.0f;
};