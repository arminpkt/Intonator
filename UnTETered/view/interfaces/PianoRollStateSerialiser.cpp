//
// Created by Armin Peukert on 17.03.26.
//

#include "PianoRollStateSerialiser.h"

#include "PianoRollStateHelpers.h"

namespace PianoRollStateIds
{
    // Root tree
    static const juce::Identifier pianoRoll        { "PIANO_ROLL" };

    // Viewport properties
    static const juce::Identifier octaveHeightPx   { "octaveHeightPx" };
    static const juce::Identifier barWidthPx        { "barWidthPx" };
    static const juce::Identifier freqBottomScreen  { "freqBottomScreen" };
    static const juce::Identifier barLeftScreen     { "barLeftScreen" };

    // Matriarch list
    static const juce::Identifier matriarchs        { "MATRIARCHS" };
    static const juce::Identifier matriarch         { "MATRIARCH" };
    static const juce::Identifier frequency         { "frequency" };

    // Note list – every entry is a ChildNote
    static const juce::Identifier notes             { "NOTES" };
    static const juce::Identifier note              { "NOTE" };
    static const juce::Identifier start             { "start" };
    static const juce::Identifier end               { "end" };
    static const juce::Identifier matriarchIndex    { "matriarchIndex" };
    static const juce::Identifier primePowers       { "primePowers" };
    static const juce::Identifier irratio           { "irratio" };
}

// ---------------------------------------------------------------------------

juce::Identifier PianoRollStateSerialiser::treeType()
{
    return PianoRollStateIds::pianoRoll;
}

// ---------------------------------------------------------------------------

juce::ValueTree PianoRollStateSerialiser::toValueTree(const PianoRollState& state)
{
    juce::ValueTree tree(treeType());

    // Viewport
    tree.setProperty(PianoRollStateIds::octaveHeightPx,   state.octaveHeightPxF,   nullptr);
    tree.setProperty(PianoRollStateIds::barWidthPx,        state.barWidthPxF,       nullptr);
    tree.setProperty(PianoRollStateIds::freqBottomScreen,  state.freqBottomScreen,  nullptr);
    tree.setProperty(PianoRollStateIds::barLeftScreen,     state.barLeftScreen,     nullptr);

    // Matriarchs
    juce::ValueTree matriarchsTree(PianoRollStateIds::matriarchs);
    for (const auto& m : state.matriarchs)
    {
        juce::ValueTree mTree(PianoRollStateIds::matriarch);
        mTree.setProperty(PianoRollStateIds::frequency, m.frequency, nullptr);
        matriarchsTree.addChild(mTree, -1, nullptr);
    }
    tree.addChild(matriarchsTree, -1, nullptr);

    // Notes
    juce::ValueTree notesTree(PianoRollStateIds::notes);

    for (const auto& n : state.notes)
    {
        auto primePowers = makeVarFromPrimePowers(n.primePowers);
        juce::ValueTree noteTree(PianoRollStateIds::note);
        noteTree.setProperty(PianoRollStateIds::start,          n.start,          nullptr);
        noteTree.setProperty(PianoRollStateIds::end,            n.end,            nullptr);
        noteTree.setProperty(PianoRollStateIds::matriarchIndex, n.matriarchIndex, nullptr);
        noteTree.setProperty(PianoRollStateIds::primePowers,      primePowers,      nullptr);
        noteTree.setProperty(PianoRollStateIds::irratio,        n.irratio,        nullptr);
        notesTree.addChild(noteTree, -1, nullptr);
    }
    tree.addChild(notesTree, -1, nullptr);

    return tree;
}

// ---------------------------------------------------------------------------

PianoRollState PianoRollStateSerialiser::fromValueTree(const juce::ValueTree& tree,
                                                        float /*pitchBendRange*/)
{
    PianoRollState state;

    if (!tree.isValid() || !tree.hasType(treeType()))
        return state;

    // Viewport
    state.octaveHeightPxF =
        static_cast<float>(tree.getProperty(PianoRollStateIds::octaveHeightPx, 120.0));
    state.barWidthPxF =
        static_cast<float>(tree.getProperty(PianoRollStateIds::barWidthPx, 120.0));
    state.freqBottomScreen =
        static_cast<double>(tree.getProperty(PianoRollStateIds::freqBottomScreen, 55.0));
    state.barLeftScreen =
        static_cast<float>(static_cast<double>(
            tree.getProperty(PianoRollStateIds::barLeftScreen, 0.0)));

    // Matriarchs
    const auto matriarchsTree = tree.getChildWithName(PianoRollStateIds::matriarchs);
    if (matriarchsTree.isValid())
    {
        state.matriarchs.clear();
        state.matriarchs.reserve(static_cast<size_t>(matriarchsTree.getNumChildren()));

        for (int i = 0; i < matriarchsTree.getNumChildren(); ++i)
        {
            const auto mTree = matriarchsTree.getChild(i);
            if (!mTree.hasType(PianoRollStateIds::matriarch))
                continue;

            StoredMatriarch m;
            m.frequency = static_cast<double>(
                mTree.getProperty(PianoRollStateIds::frequency, 440.0));
            state.matriarchs.push_back(m);
        }
    }

    // Notes
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
            n.start          = static_cast<float>    (noteTree.getProperty(PianoRollStateIds::start,          0.0));
            n.end            = static_cast<float>    (noteTree.getProperty(PianoRollStateIds::end,            0.0));
            n.matriarchIndex = static_cast<int>      (noteTree.getProperty(PianoRollStateIds::matriarchIndex, -1));
            n.primePowers    = makePrimePowersFromVar(noteTree.getProperty(PianoRollStateIds::primePowers,    {}));
            n.irratio        = static_cast<double>   (noteTree.getProperty(PianoRollStateIds::irratio,        1.0));

            state.notes.push_back(n);
        }
    }

    return state;
}