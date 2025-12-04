//
// Created by Armin Peukert on 28.10.25.
//

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <vector>

#include "header-only/ChannelPool.h"
#include "header-only/Note.h"

class NoteRegion {
public:
    std::vector<std::unique_ptr<Note>> notes;
    std::vector<juce::MidiMessage> midiMessages;

    void addNote(float frequency, int start, int end);
    void calculateMidiMessages(float pitchBendRange = 2); // not const!
    void useAsParentToCreate(Note* note, Fraction ratio);
    void useAsParentToCreate(Note* note, float irratio);

private:
    ChannelPool channelPool;
};
