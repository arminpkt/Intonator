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
    std::vector<std::unique_ptr<RootNote>> matriarchs;
    std::vector<std::unique_ptr<ChildNote>> notes;
    std::vector<NoteEvent> noteEvents;
    std::vector<juce::MidiMessage> midiMessages;

    void addNoteWithoutReference(double frequency, float start, float end);
    void addNoteWithReference(ChildNote* reference, Fraction ratio, double irratio, float start, float end);
    void addNoteWithMatriarch(RootNote* matriarch, Fraction ratio, double irratio, double start, double end);
    void deleteNote(ChildNote* note);
    void calculateMidiMessages(float pitchBendRange = 2); // not const!

private:
    static int midiEventPriority(const juce::MidiMessage& m);
    void deleteNoteUnsafe(const Note* note);
    ChannelPool channelPool;
};
