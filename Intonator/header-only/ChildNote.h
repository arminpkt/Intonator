//
// Created by Vos on 30/09/2025.
//

#pragma once
#include "Note.h"

class ChildNote : public Note {
public:
    // Non-owning raw pointer to parent, just for calculation
    const Note* parent;

    ChildNote(const Note* p, float ratio, int s, int e)
        : Note(p->frequency * ratio, s, e), parent(p) {
        // children vector default constructed empty
    }
};