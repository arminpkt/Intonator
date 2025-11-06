//
// Created by Armin Peukert on 28.10.25.
//

#include "NoteRegion.h"
#include <cmath>
#include <algorithm>

void NoteRegion::addNote(float frequency, int start, int end)
{
    // Create and store a new RootNote
    notes.push_back(std::make_unique<RootNote>(frequency, start, end));

    // Automatically recalculate the MIDI messages
    calculateMidiMessages();
}

void NoteRegion::calculateMidiMessages()
{
    midiMessages.clear();

    for (const auto& notePtr : notes)
    {
        const Note* note = notePtr.get();
        if (!note)
            continue;

        // Convert frequency to MIDI note number (equal temperament)
        int midiNoteNumber = static_cast<int>(std::round(69.0 + 12.0 * std::log2(note->frequency / 440.0f)));
        midiNoteNumber = std::clamp(midiNoteNumber, 0, 127);

        // Create Note On message
        juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, midiNoteNumber, (juce::uint8)100);
        noteOn.setTimeStamp(note->start);
        midiMessages.push_back(noteOn);

        // Create Note Off message
        juce::MidiMessage noteOff = juce::MidiMessage::noteOff(1, midiNoteNumber);
        noteOff.setTimeStamp(note->end);
        midiMessages.push_back(noteOff);
    }

    // Sort by timestamp so they play in correct order
    std::sort(midiMessages.begin(), midiMessages.end(),
        [](const juce::MidiMessage& a, const juce::MidiMessage& b)
        {
            return a.getTimeStamp() < b.getTimeStamp();
        });
}
