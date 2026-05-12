//
// Created by Armin Peukert on 17.03.26.
//

#pragma once

#include <vector>
#include <juce_core/juce_core.h>
#include "../../logic/Monzo.h"

struct StoredPianoNote
{
    bool isChild = false;

    float start = 0.0;
    float end = 0.0;

    // for root notes
    double frequency = 440.0;

    // for child notes
    int parentIndex = -1;
    std::array<int, primes::PrimeCount> primePowers{};
    double irratio = 1.0;
};

struct PianoRollState
{
    std::vector<StoredPianoNote> notes;
    float octaveHeightPxF = 120;
    float barWidthPxF = 120;
    double freqBottomScreen = 55.0;
    float barLeftScreen = 0.0f;
};