//
// Created by Armin Peukert on 17.03.26.
//

#include "PianoRollStateHelpers.h"
#include <unordered_map>

// ---------------------------------------------------------------------------
// NoteRegion  →  PianoRollState
// ---------------------------------------------------------------------------
PianoRollState makeStateFromNoteRegion(const NoteRegion& region)
{
    PianoRollState state;

    // 1. Serialise every matriarch and build a reverse-lookup map so we can
    //    find a matriarch's index in O(1) when we process its children below.
    state.matriarchs.reserve(region.matriarchs.size());
    std::unordered_map<const RootNote*, int> matriarchIndex;
    matriarchIndex.reserve(region.matriarchs.size());

    for (const auto& mPtr : region.matriarchs)
    {
        matriarchIndex[mPtr.get()] = static_cast<int>(state.matriarchs.size());
        state.matriarchs.push_back({ mPtr->frequency });
    }

    // 2. Serialise every drawn note (always a ChildNote in the new layout).
    state.notes.reserve(region.notes.size());

    for (const auto& notePtr : region.notes)
    {
        const ChildNote* child = notePtr.get();

        StoredPianoNote sn;
        sn.start     = child->start;
        sn.end       = child->end;
        sn.ratioText = child->ratio.toString();
        sn.irratio   = child->irratio;

        // The parent must be one of the matriarchs – cast is safe by design.
        auto it = matriarchIndex.find(static_cast<const RootNote*>(child->parent));
        if (it != matriarchIndex.end())
            sn.matriarchIndex = it->second;
        // If the parent isn't found (shouldn't happen in a well-formed region)
        // matriarchIndex stays -1 and the note will be skipped on reload.

        state.notes.push_back(std::move(sn));
    }

    return state;
}

// ---------------------------------------------------------------------------
// PianoRollState  →  NoteRegion
// ---------------------------------------------------------------------------
NoteRegion makeNoteRegionFromState(const PianoRollState& state, float pitchBendRange)
{
    NoteRegion region;

    // 1. Recreate every matriarch.  start/end are 0 because matriarchs are
    //    pure reference-frequency objects, not drawable notes.
    region.matriarchs.reserve(state.matriarchs.size());
    for (const auto& sm : state.matriarchs)
        region.matriarchs.push_back(std::make_unique<RootNote>(sm.frequency, 0.0f, 0.0f));

    // 2. Recreate every drawn note as a ChildNote of its stored matriarch.
    region.notes.reserve(state.notes.size());
    for (const auto& sn : state.notes)
    {
        if (sn.matriarchIndex < 0
            || sn.matriarchIndex >= static_cast<int>(region.matriarchs.size()))
        {
            // Orphaned note – skip rather than crash.
            continue;
        }

        auto ratioOpt = Fraction::fromString(sn.ratioText);
        if (!ratioOpt.has_value())
            continue;

        RootNote* matriarch =
            region.matriarchs[static_cast<size_t>(sn.matriarchIndex)].get();

        region.notes.push_back(std::make_unique<ChildNote>(
            matriarch,
            ratioOpt.value(),
            sn.irratio,
            sn.start,
            sn.end
        ));
    }

    region.calculateMidiMessages(pitchBendRange);
    return region;
}