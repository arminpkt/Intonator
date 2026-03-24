//
// Created by Armin Peukert on 17.03.26.
//

#pragma once

#include <vector>

struct StoredPianoNote
{
    bool isChild = false;

    int start = 0;
    int end = 0;

    // for root notes
    double frequency = 440.0;

    // for child notes
    int parentIndex = -1;
    int ratioNum = 1;
    int ratioDen = 1;
    double irratio = 1.0;
};

struct PianoRollState
{
    std::vector<StoredPianoNote> notes;
    int octaveHeightPx = 120;
    int barWidthPx = 120;
    double freqBottomScreen = 55.0;
    float barLeftScreen = 0.0f;
};