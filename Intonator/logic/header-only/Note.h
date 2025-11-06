//
// Created by Vos on 30/09/2025.
//

#pragma once
#include <vector>
#include <memory>


class ChildNote;

class Note {
public:
    float frequency;
    int start;
    int end;
    std::vector<std::unique_ptr<ChildNote>> children;

    Note(float freq, int s, int e)
        : frequency(freq), start(s), end(e) {}

    virtual ~Note() = default;
    virtual void dummy() = 0;

    bool operator<(const Note& other) const {
        return frequency < other.frequency;
    }
};

class ChildNote : public Note {
public:
    const Note* parent;
    float ratio;

    ChildNote(const Note& p, float r, int s, int e)
        : Note(p.frequency * r, s, e), parent(&p), ratio(r) {}
};

class RootNote : public Note {
public:
    RootNote(float freq, int s, int e)
        : Note(freq, s, e) {
    }
    void dummy() override {}
};