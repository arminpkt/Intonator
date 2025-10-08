//
// Created by Vos on 30/09/2025.
//

#pragma once
#include <vector>
#include <memory>

class Note {
public:
    float frequency;
    int start;
    int end;
    std::vector<std::unique_ptr<Note>> children;

    Note(float freq, int s, int e)
        : frequency(freq), start(s), end(e) {}

    virtual ~Note() = default;
};
