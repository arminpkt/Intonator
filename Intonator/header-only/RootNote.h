//
// Created by Vos on 30/09/2025.
//

#pragma once
#include "Note.h"

class RootNote : public Note {
public:
    RootNote(float freq, int s, int e)
        : Note(freq, s, e) {
        // children vector default constructed empty
    }
};