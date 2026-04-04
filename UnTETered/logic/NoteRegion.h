//
// Created by Armin Peukert on 28.10.25.
//

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <vector>

#include "ChannelPool.h"
#include "Note.h"

struct NoteEvent {
    double startTime;
    double endTime;
    juce::MidiMessage noteOn;
    juce::MidiMessage pitchBend;
    juce::MidiMessage noteOff;
    int channel = 0;
};

class NoteRegion {
public:
    std::vector<std::unique_ptr<Note>> notes;
    std::vector<NoteEvent> noteEvents;
    std::vector<juce::MidiMessage> midiMessages;

    void addRootNote(double frequency, float start, float end);
    void addChildNote(Note* parent, Fraction ratio, double irratio, float start, float end);
    void addNote(std::unique_ptr<Note>* note);
    void deleteNote(Note* note);
    void calculateMidiMessages(float pitchBendRange = 2); // not const!
    void useAsParentToCreate(Note* note, Fraction ratio, float start, float end);
    void useAsParentToCreate(Note* note, float irratio, float start, float end);

private:
    static int midiEventPriority(const juce::MidiMessage& m);
    ChannelPool channelPool;
};
