//
// Created by Vos on 30/09/2025.
//

#pragma once
#include "Note.h"
#include "RootNote.h"

class ChildNote : public Note {
public:
    const Note* parent;
    float ratio;

    ChildNote(const Note& p, float r, int s, int e)
        : Note(p.frequency * r, s, e), parent(&p), ratio(r) {}
};
