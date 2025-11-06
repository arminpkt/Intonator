//
// Created by Armin Peukert on 28.10.25.
//

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <vector>
#include <utility>
#include "header-only/Note.h"
#include "header-only/ChildNote.h"
#include "header-only/RootNote.h"

class NoteRegion {
public:
    std::vector<std::unique_ptr<Note>> notes;
    std::vector<juce::MidiMessage> midiMessages;

    void addNote(float frequency, int start, int end);
    void calculateMidiMessages(); // not const!
};
