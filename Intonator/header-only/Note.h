//
// Created by Vos on 30/09/2025.
//

#pragma once
#include <vector>
#include <memory>

#include "ChildNote.h"

class Note {
public:
    float frequency;
    int start;
    int end;
    std::vector<std::unique_ptr<ChildNote>> children;

    Note(float freq, int s, int e)
        : frequency(freq), start(s), end(e) {}

    virtual ~Note() = default;
};
