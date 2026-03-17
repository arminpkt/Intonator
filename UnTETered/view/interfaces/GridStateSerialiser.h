//
// Created by Armin Peukert on 17.03.26.
//

#pragma once

#include "GridState.h"
#include <juce_data_structures/juce_data_structures.h>

class GridStateSerialiser
{
public:
    static juce::Identifier treeType();

    static juce::ValueTree toValueTree(const GridState& state);
    static GridState fromValueTree(const juce::ValueTree& tree);

private:
    static void writePointSet(juce::ValueTree& parent,
                              const juce::Identifier& childName,
                              const PointSet& set);

    static PointSet readPointSet(const juce::ValueTree& parent,
                                 const juce::Identifier& childName);

    static juce::ValueTree writeSaveSlots(const std::array<SaveSlotState, 91>& saves);
    static void readSaveSlots(const juce::ValueTree& savesTree,
                              std::array<SaveSlotState, 91>& saves);
};
