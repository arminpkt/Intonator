//
// Created by Armin Peukert on 17.03.26.
//

#include "PianoRollStateHelpers.h"

#include <unordered_map>

PianoRollState makeStateFromNoteRegion(const NoteRegion& region)
{
    PianoRollState state;
    state.notes.reserve(region.notes.size());

    std::unordered_map<const Note*, int> noteIndex;

    for (const auto& notePtr : region.notes)
    {
        const Note* note = notePtr.get();
        StoredPianoNote stored;

        stored.start = note->start;
        stored.end   = note->end;

        if (const auto* child = dynamic_cast<const ChildNote*>(note))
        {
            stored.isChild = true;
            stored.primePowers = child->ratio.getMonzo().primePowers;
            stored.irratio  = child->irratio;
            stored.parentIndex = -1; // fill later after all notes have indices
        }
        else
        {
            stored.isChild = false;
            stored.frequency = note->frequency;
        }

        noteIndex[note] = static_cast<int>(state.notes.size());
        state.notes.push_back(stored);
    }

    for (size_t i = 0; i < region.notes.size(); ++i)
    {
        const Note* note = region.notes[i].get();
        if (const auto* child = dynamic_cast<const ChildNote*>(note))
        {
            auto it = noteIndex.find(child->parent);
            if (it != noteIndex.end())
                state.notes[i].parentIndex = it->second;
        }
    }

    return state;
}

NoteRegion makeNoteRegionFromState(const PianoRollState& state, float pitchBendRange)
{
    NoteRegion region;
    std::vector<Note*> createdNotes;
    createdNotes.reserve(state.notes.size());

    for (const auto& stored : state.notes)
    {
        if (!stored.isChild)
        {
            auto root = std::make_unique<RootNote>(
                stored.frequency,
                stored.start,
                stored.end
            );

            createdNotes.push_back(root.get());
            region.notes.push_back(std::move(root));
        }
        else
        {
            createdNotes.push_back(nullptr);
        }
    }

    for (size_t i = 0; i < state.notes.size(); ++i)
    {
        const auto& stored = state.notes[i];
        if (!stored.isChild)
            continue;

        if (stored.parentIndex < 0
            || stored.parentIndex >= static_cast<int>(createdNotes.size())
            || createdNotes[static_cast<size_t>(stored.parentIndex)] == nullptr)
        {
            continue;
        }

        Note* parent = createdNotes[static_cast<size_t>(stored.parentIndex)];

        auto ratio = Fraction(Monzo(stored.primePowers));

        auto child = std::make_unique<ChildNote>(
            parent,
            ratio,
            stored.irratio,
            stored.start,
            stored.end
        );

        createdNotes[i] = child.get();
        region.notes.push_back(std::move(child));
    }

    region.calculateMidiMessages(pitchBendRange);
    return region;
}