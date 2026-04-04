//
// Created by Armin Peukert on 17.03.26.
//

#pragma once

#include "PianoRollState.h"
#include <juce_data_structures/juce_data_structures.h>

class PianoRollStateSerialiser
{
public:
    static juce::Identifier treeType();

    static juce::ValueTree toValueTree(const PianoRollState& state);
    static PianoRollState fromValueTree(const juce::ValueTree& tree,
                                        float pitchBendRange = 2.0f);
};
