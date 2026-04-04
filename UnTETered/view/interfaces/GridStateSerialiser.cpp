//
// Created by Armin Peukert on 17.03.26.
//

#include "GridStateSerialiser.h"

namespace GridStateIds
{
    static const juce::Identifier grid      { "GRID" };
    static const juce::Identifier pointNode { "C" };

    static const juce::Identifier originHz  { "originHz" };
    static const juce::Identifier offsetX   { "offsetX" };
    static const juce::Identifier offsetY   { "offsetY" };

    static const juce::Identifier active    { "ACTIVE" };
    static const juce::Identifier selected  { "SELECTED" };

    static const juce::Identifier saves     { "SAVES" };
    static const juce::Identifier slot      { "S" };
    static const juce::Identifier cells     { "CELLS" };

    static const juce::Identifier index     { "i" };
    static const juce::Identifier mode      { "mode" };
    static const juce::Identifier x         { "x" };
    static const juce::Identifier y         { "y" };
}

juce::Identifier GridStateSerialiser::treeType()
{
    return GridStateIds::grid;
}

juce::ValueTree GridStateSerialiser::toValueTree(const GridState& state)
{
    juce::ValueTree tree(treeType());

    tree.setProperty(GridStateIds::originHz, state.originFreqHz, nullptr);
    tree.setProperty(GridStateIds::offsetX,  state.offsetX,     nullptr);
    tree.setProperty(GridStateIds::offsetY,  state.offsetY,     nullptr);

    writePointSet(tree, GridStateIds::active,   state.activeCells);
    writePointSet(tree, GridStateIds::selected, state.selectedCells);

    tree.addChild(writeSaveSlots(state.saves), -1, nullptr);

    return tree;
}

GridState GridStateSerialiser::fromValueTree(const juce::ValueTree& tree)
{
    GridState state;

    if (!tree.isValid() || !tree.hasType(treeType()))
        return state;

    state.originFreqHz = static_cast<double>(tree.getProperty(GridStateIds::originHz, 220.0));
    state.offsetX      = static_cast<int>(tree.getProperty(GridStateIds::offsetX, 0));
    state.offsetY      = static_cast<int>(tree.getProperty(GridStateIds::offsetY, 0));

    state.activeCells   = readPointSet(tree, GridStateIds::active);
    state.selectedCells = readPointSet(tree, GridStateIds::selected);

    readSaveSlots(tree.getChildWithName(GridStateIds::saves), state.saves);

    return state;
}

void GridStateSerialiser::writePointSet(juce::ValueTree& parent,
                                        const juce::Identifier& childName,
                                        const PointSet& set)
{
    juce::ValueTree node(childName);

    for (const auto& p : set)
    {
        juce::ValueTree point(GridStateIds::pointNode);
        point.setProperty(GridStateIds::x, p.x, nullptr);
        point.setProperty(GridStateIds::y, p.y, nullptr);
        node.addChild(point, -1, nullptr);
    }

    parent.addChild(node, -1, nullptr);
}

PointSet GridStateSerialiser::readPointSet(const juce::ValueTree& parent,
                                           const juce::Identifier& childName)
{
    PointSet out;
    const auto node = parent.getChildWithName(childName);

    if (!node.isValid())
        return out;

    for (int i = 0; i < node.getNumChildren(); ++i)
    {
        const auto point = node.getChild(i);
        out.insert({
            static_cast<int>(point.getProperty(GridStateIds::x, 0)),
            static_cast<int>(point.getProperty(GridStateIds::y, 0))
        });
    }

    return out;
}

juce::ValueTree GridStateSerialiser::writeSaveSlots(const std::array<SaveSlotState, 91>& saves)
{
    juce::ValueTree savesTree(GridStateIds::saves);

    for (int i = 0; i < static_cast<int>(saves.size()); ++i)
    {
        const auto& src = saves[static_cast<size_t>(i)];

        juce::ValueTree slot(GridStateIds::slot);
        slot.setProperty(GridStateIds::index, i, nullptr);

        const juce::String modeString =
            src.mode != 0 ? juce::String::charToString(src.mode) : juce::String();

        slot.setProperty(GridStateIds::mode, modeString, nullptr);

        juce::ValueTree cells(GridStateIds::cells);

        for (const auto& p : src.screenCells)
        {
            juce::ValueTree point(GridStateIds::pointNode);
            point.setProperty(GridStateIds::x, p.x, nullptr);
            point.setProperty(GridStateIds::y, p.y, nullptr);
            cells.addChild(point, -1, nullptr);
        }

        slot.addChild(cells, -1, nullptr);
        savesTree.addChild(slot, -1, nullptr);
    }

    return savesTree;
}

void GridStateSerialiser::readSaveSlots(const juce::ValueTree& savesTree,
                                        std::array<SaveSlotState, 91>& saves)
{
    if (!savesTree.isValid())
        return;

    for (int i = 0; i < savesTree.getNumChildren(); ++i)
    {
        const auto slot = savesTree.getChild(i);
        if (!slot.hasType(GridStateIds::slot))
            continue;

        const int index = static_cast<int>(slot.getProperty(GridStateIds::index, -1));
        if (index < 0 || index >= static_cast<int>(saves.size()))
            continue;

        auto& dst = saves[static_cast<size_t>(index)];

        const auto modeStr = slot.getProperty(GridStateIds::mode, "").toString();
        dst.mode = modeStr.isNotEmpty() ? modeStr[0] : 0;

        dst.screenCells.clear();

        const auto cells = slot.getChildWithName(GridStateIds::cells);
        if (!cells.isValid())
            continue;

        dst.screenCells.reserve(static_cast<size_t>(cells.getNumChildren()));

        for (int j = 0; j < cells.getNumChildren(); ++j)
        {
            const auto point = cells.getChild(j);
            dst.screenCells.push_back({
                static_cast<int>(point.getProperty(GridStateIds::x, 0)),
                static_cast<int>(point.getProperty(GridStateIds::y, 0))
            });
        }
    }
}