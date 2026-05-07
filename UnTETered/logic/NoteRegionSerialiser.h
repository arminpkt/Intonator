//
// Created by Armin Peukert on 17.03.26.
//

#pragma once

#include "NoteRegion.h"
#include <juce_data_structures/juce_data_structures.h>

class NoteRegionSerialiser
{
public:
    static juce::Identifier treeType();

    static juce::ValueTree toValueTree(const NoteRegion& region);
    static NoteRegion fromValueTree(const juce::ValueTree& tree,
                                    float pitchBendRange = 2.0f);

private:
    static int findNoteIndex(const std::vector<std::unique_ptr<RootNote>>& notes,
                             const RootNote* target);
};
