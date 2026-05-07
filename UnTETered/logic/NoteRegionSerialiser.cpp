//
// Created by Armin Peukert on 17.03.26.
//

#include "NoteRegionSerialiser.h"

namespace NoteRegionIds
{
    static const juce::Identifier region     { "NOTE_REGION" };
    static const juce::Identifier noteNode   { "NOTE" };

    static const juce::Identifier type       { "type" };
    static const juce::Identifier rootType   { "root" };
    static const juce::Identifier childType  { "child" };

    static const juce::Identifier frequency  { "frequency" };
    static const juce::Identifier start      { "start" };
    static const juce::Identifier end        { "end" };

    static const juce::Identifier parent     { "parent" };
    static const juce::Identifier ratioNum   { "ratioNum" };
    static const juce::Identifier ratioDen   { "ratioDen" };
    static const juce::Identifier irratio    { "irratio" };
}

juce::Identifier NoteRegionSerialiser::treeType()
{
    return NoteRegionIds::region;
}

int NoteRegionSerialiser::findNoteIndex(const std::vector<std::unique_ptr<RootNote>>& notes,
                                        const RootNote* target)
{
    for (int i = 0; i < static_cast<int>(notes.size()); ++i)
    {
        if (notes[static_cast<size_t>(i)].get() == target)
            return i;
    }

    return -1;
}

juce::ValueTree NoteRegionSerialiser::toValueTree(const NoteRegion& region)
{
    juce::ValueTree tree(treeType());

    for (const auto& notePtr : region.notes)
    {
        if (!notePtr)
            continue;

        const Note& note = *notePtr;
        juce::ValueTree noteTree(NoteRegionIds::noteNode);

        noteTree.setProperty(NoteRegionIds::start, note.start, nullptr);
        noteTree.setProperty(NoteRegionIds::end,   note.end,   nullptr);

        if (const auto* root = dynamic_cast<const RootNote*>(&note))
        {
            juce::ignoreUnused(root);
            noteTree.setProperty(NoteRegionIds::type, NoteRegionIds::rootType.toString(), nullptr);
            noteTree.setProperty(NoteRegionIds::frequency, note.frequency, nullptr);
        }
        else if (const auto* child = dynamic_cast<const ChildNote*>(&note))
        {
            noteTree.setProperty(NoteRegionIds::type, NoteRegionIds::childType.toString(), nullptr);

            const int parentIndex = findNoteIndex(region.matriarchs, child->parent);
            noteTree.setProperty(NoteRegionIds::parent, parentIndex, nullptr);

            const auto [num, den] = child->ratio.getNumeratorAndDenominator();
            noteTree.setProperty(NoteRegionIds::ratioNum, num, nullptr);
            noteTree.setProperty(NoteRegionIds::ratioDen, den, nullptr);
            noteTree.setProperty(NoteRegionIds::irratio,  child->irratio,            nullptr);
        }
        else
        {
            // Fallback: unknown Note subtype -> preserve effective note only as root
            noteTree.setProperty(NoteRegionIds::type, NoteRegionIds::rootType.toString(), nullptr);
            noteTree.setProperty(NoteRegionIds::frequency, note.frequency, nullptr);
        }

        tree.addChild(noteTree, -1, nullptr);
    }

    return tree;
}

NoteRegion NoteRegionSerialiser::fromValueTree(const juce::ValueTree& tree,
                                               float pitchBendRange)
{
    NoteRegion region;

    if (!tree.isValid() || !tree.hasType(treeType()))
        return region;

    // First pass: create notes in the same order as saved.
    for (int i = 0; i < tree.getNumChildren(); ++i)
    {
        const auto noteTree = tree.getChild(i);
        if (!noteTree.hasType(NoteRegionIds::noteNode))
            continue;

        const auto type  = noteTree.getProperty(NoteRegionIds::type, "").toString();
        const int start  = static_cast<int>(noteTree.getProperty(NoteRegionIds::start, 0));
        const int end    = static_cast<int>(noteTree.getProperty(NoteRegionIds::end, 0));

        if (type == NoteRegionIds::childType.toString())
        {
            // placeholder; replaced in second pass
            region.notes.push_back(nullptr);
        }
        else
        {
            throw std::invalid_argument("Unknown note");
        }
    }

    // Second pass: create child notes now that parents exist.
    for (int i = 0; i < tree.getNumChildren(); ++i)
    {
        const auto noteTree = tree.getChild(i);
        if (!noteTree.hasType(NoteRegionIds::noteNode))
            continue;

        const auto type = noteTree.getProperty(NoteRegionIds::type, "").toString();
        if (type != NoteRegionIds::childType.toString())
            continue;

        const int start = static_cast<int>(noteTree.getProperty(NoteRegionIds::start, 0));
        const int end   = static_cast<int>(noteTree.getProperty(NoteRegionIds::end, 0));

        const int parentIndex = static_cast<int>(noteTree.getProperty(NoteRegionIds::parent, -1));
        if (parentIndex < 0 || parentIndex >= static_cast<int>(region.notes.size()) || !region.notes[static_cast<size_t>(parentIndex)])
        {
            // fallback if parent missing
            region.notes[static_cast<size_t>(i)] = std::make_unique<RootNote>(440.0, start, end);
            continue;
        }

        Note& parent = *region.notes[static_cast<size_t>(parentIndex)];

        const int ratioNum = static_cast<int>(noteTree.getProperty(NoteRegionIds::ratioNum, 1));
        const int ratioDen = static_cast<int>(noteTree.getProperty(NoteRegionIds::ratioDen, 1));
        const double irratio =
            static_cast<double>(noteTree.getProperty(NoteRegionIds::irratio, 1.0));

        region.notes[static_cast<size_t>(i)] =
            std::make_unique<ChildNote>(&parent, Fraction(ratioNum, ratioDen), irratio, start, end);
    }

    region.calculateMidiMessages(pitchBendRange);
    return region;
}
