//
// Created by Vos de Mens on 05/05/2026.
//

#pragma once
#include "Note.h"

struct NoteFamily {
    public:
    double refFrequency;
    std::vector<Note> notes;

    explicit NoteFamily(Note* ref) : matriarch(std::make_unique<RootNote>(ref->frequency, 0, 0)) {}

    void addNote(Fraction frac, float start, float end) {
        childNotes.emplace_back(matriarch.get(), frac, start, end);
    }

    void deleteNote(Note* note) {}
};
