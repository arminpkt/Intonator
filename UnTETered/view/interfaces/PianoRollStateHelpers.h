//
// Created by Armin Peukert on 17.03.26.
//

#pragma once

#include "../../logic/NoteRegion.h"
#include "PianoRollState.h"

PianoRollState makeStateFromNoteRegion(const NoteRegion& region);
NoteRegion makeNoteRegionFromState(const PianoRollState& state, float pitchBendRange = 2.0f);
