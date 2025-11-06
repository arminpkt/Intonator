//
// Created by Vos on 06/11/2025.
//

#include "HardcodedChord.h"

NoteRegion initMyGlobal() {
    NoteRegion obj{};
    obj.addNote(400, 0, 1000);
    obj.addNote(500, 0, 1000);
    obj.addNote(600, 0, 1000);
    obj.addNote(700, 0, 1000);
    return obj;
}

NoteRegion globalNoteRegion = initMyGlobal();