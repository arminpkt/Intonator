//
// Created by Armin Peukert on 28.10.25.
//

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "header-only/Note.h"
#include "header-only/ChildNote.h"

class NoteRegion {
public:
    std::vector<Note*> notes;
    std::vector<juce::MidiMessage*> midiMessages;

    void updateMidiMessages();
};
