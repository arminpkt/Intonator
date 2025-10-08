//
// Created by Vos on 30/09/2025.
//

#pragma once
#include "Fraction.h"
#include "Note.h"
#include "RootNote.h"

class ChildNote : public Note {
public:
    const Note* parent;
    const RootNote* root;
    Fraction ratio;
    Fraction ratioToRoot;

private:
    static Fraction computeRatioToRoot(const Note& p, Fraction r, const RootNote*& rootPtr) {
        if (const auto rp = dynamic_cast<const RootNote*>(&p)) {
            rootPtr = rp;
            return r;
        } else if (const auto cp = dynamic_cast<const ChildNote*>(&p)) {
            rootPtr = cp->root;
            return r * cp->ratioToRoot;
        } else {
            throw std::invalid_argument("Parent must be RootNote or ChildNote");
        }
    }

    ChildNote(const Note& p, Fraction r, int s, int e)
        : Note(0, s, e), parent(&p), ratio(r), ratioToRoot(computeRatioToRoot(p, r, root)) {
        if (const auto rp = dynamic_cast<const RootNote*>(&p)) {
            root = rp;
            ratioToRoot = r;
        } else if (const auto cp = dynamic_cast<const ChildNote*>(&p)) {
            root = cp->root;
            ratioToRoot = r * cp->ratioToRoot;
        } else {
            throw std::invalid_argument("Parent must be RootNote or ChildNote");
        }

        frequency = root->frequency * ratioToRoot.toFloat();
    }
};
