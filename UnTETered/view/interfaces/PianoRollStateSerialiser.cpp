//
// Created by Armin Peukert on 17.03.26.
//

#include "PianoRollStateSerialiser.h"

namespace PianoRollStateIds
{
    static const juce::Identifier pianoRoll        { "PIANO_ROLL" };
    static const juce::Identifier octaveHeightPx   { "octaveHeightPx" };
    static const juce::Identifier barWidthPx       { "barWidthPx" };
    static const juce::Identifier freqBottomScreen { "freqBottomScreen" };
    static const juce::Identifier barLeftScreen    { "barLeftScreen" };

    static const juce::Identifier notes            { "NOTES" };
    static const juce::Identifier note             { "NOTE" };

    static const juce::Identifier isChild          { "isChild" };
    static const juce::Identifier start            { "start" };
    static const juce::Identifier end              { "end" };
    static const juce::Identifier frequency        { "frequency" };
    static const juce::Identifier parentIndex      { "parentIndex" };
    static const juce::Identifier ratioText        { "ratioText" };
    static const juce::Identifier irratio          { "irratio" };
}

juce::Identifier PianoRollStateSerialiser::treeType()
{
    return PianoRollStateIds::pianoRoll;
}

juce::ValueTree PianoRollStateSerialiser::toValueTree(const PianoRollState& state)
{
    juce::ValueTree tree(treeType());

    tree.setProperty(PianoRollStateIds::octaveHeightPx,   state.octaveHeightPx,   nullptr);
    tree.setProperty(PianoRollStateIds::barWidthPx,       state.barWidthPx,       nullptr);
    tree.setProperty(PianoRollStateIds::freqBottomScreen, state.freqBottomScreen, nullptr);
    tree.setProperty(PianoRollStateIds::barLeftScreen,    state.barLeftScreen,    nullptr);

    juce::ValueTree notesTree(PianoRollStateIds::notes);

    for (const auto& n : state.notes)
    {
        juce::ValueTree noteTree(PianoRollStateIds::note);
        noteTree.setProperty(PianoRollStateIds::isChild,     n.isChild,      nullptr);
        noteTree.setProperty(PianoRollStateIds::start,       n.start,        nullptr);
        noteTree.setProperty(PianoRollStateIds::end,         n.end,          nullptr);
        noteTree.setProperty(PianoRollStateIds::frequency,   n.frequency,    nullptr);
        noteTree.setProperty(PianoRollStateIds::parentIndex, n.parentIndex,  nullptr);
        noteTree.setProperty(PianoRollStateIds::ratioText,   n.ratioText,    nullptr);
        noteTree.setProperty(PianoRollStateIds::irratio,     n.irratio,      nullptr);

        notesTree.addChild(noteTree, -1, nullptr);
    }

    tree.addChild(notesTree, -1, nullptr);
    return tree;
}

PianoRollState PianoRollStateSerialiser::fromValueTree(const juce::ValueTree& tree,
                                                       float /*pitchBendRange*/)
{
    PianoRollState state;

    if (!tree.isValid() || !tree.hasType(treeType()))
        return state;

    state.octaveHeightPx =
        static_cast<int>(tree.getProperty(PianoRollStateIds::octaveHeightPx, 120));

    state.barWidthPx =
        static_cast<int>(tree.getProperty(PianoRollStateIds::barWidthPx, 120));

    state.freqBottomScreen =
        static_cast<double>(tree.getProperty(PianoRollStateIds::freqBottomScreen, 55.0));

    state.barLeftScreen =
        static_cast<float>(static_cast<double>(tree.getProperty(PianoRollStateIds::barLeftScreen, 0.0)));

    const auto notesTree = tree.getChildWithName(PianoRollStateIds::notes);
    if (notesTree.isValid())
    {
        state.notes.clear();
        state.notes.reserve(static_cast<size_t>(notesTree.getNumChildren()));

        for (int i = 0; i < notesTree.getNumChildren(); ++i)
        {
            const auto noteTree = notesTree.getChild(i);
            if (!noteTree.hasType(PianoRollStateIds::note))
                continue;

            StoredPianoNote n;
            n.isChild     = static_cast<bool>(noteTree.getProperty(PianoRollStateIds::isChild, false));
            n.start       = static_cast<float>(noteTree.getProperty(PianoRollStateIds::start, 0.0));
            n.end         = static_cast<float>(noteTree.getProperty(PianoRollStateIds::end, 0.0));
            n.frequency   = static_cast<double>(noteTree.getProperty(PianoRollStateIds::frequency, 440.0));
            n.parentIndex = static_cast<int>(noteTree.getProperty(PianoRollStateIds::parentIndex, -1));
            n.ratioText   = noteTree.getProperty(PianoRollStateIds::ratioText, "1/1").toString();
            n.irratio     = static_cast<double>(noteTree.getProperty(PianoRollStateIds::irratio, 1.0));

            state.notes.push_back(n);
        }
    }

    return state;
}