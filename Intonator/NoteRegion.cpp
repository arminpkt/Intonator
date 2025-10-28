//
// Created by Armin Peukert on 28.10.25.
//

#include "NoteRegion.h"
#include <cmath> // for std::log2, std::round

void NoteRegion::updateMidiMessages()
{
    midiMessages.clear();

    for (const auto* note : notes)
    {
        if (note == nullptr)
            continue;

        // Convert frequency to MIDI note number (equal temperament)
        int midiNoteNumber = static_cast<int>(std::round(69.0 + 12.0 * std::log2(note->frequency / 440.0f)));

        // Basic safety: clamp to valid MIDI range
        midiNoteNumber = std::clamp(midiNoteNumber, 0, 127);

        // Create Note On message
        auto* noteOn = new juce::MidiMessage(juce::MidiMessage::noteOn(1, midiNoteNumber, (juce::uint8)100));
        noteOn->setTimeStamp(note->start);
        midiMessages.push_back(noteOn);

        // Create Note Off message
        auto* noteOff = new juce::MidiMessage(juce::MidiMessage::noteOff(1, midiNoteNumber));
        noteOff->setTimeStamp(note->end);
        midiMessages.push_back(noteOff);
    }

    // Sort MIDI messages by timestamp to ensure proper order
    std::sort(midiMessages.begin(), midiMessages.end(),
        [](const juce::MidiMessage* a, const juce::MidiMessage* b)
        {
            return a->getTimeStamp() < b->getTimeStamp();
        });
}