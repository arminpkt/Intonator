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

void NoteRegion::calculateMidiMessages(const float pitchBendRange)
{
    midiMessages.clear();

    std::unordered_map<juce::MidiMessage*, std::pair<juce::MidiMessage*, juce::MidiMessage*>> midiMessagesMap;

    for (const auto& notePtr : notes)
    {
        const Note* note = notePtr.get();
        if (!note)
            continue;

        // Convert frequency to MIDI
        const int midiNoteNumber = note->getRoundedMidiValue();
        const float pitchBendInSemitones = note->getPitchBendInSemitones();

        // Create Note On message
        juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, midiNoteNumber, static_cast<juce::uint8>(100));
        noteOn.setTimeStamp(note->start);
        midiMessages.push_back(noteOn);

        // Create Pitchbend message
        juce::MidiMessage pitchBend = juce::MidiMessage::pitchbendToPitchwheelPos(pitchBendInSemitones, pitchBendRange);
        pitchBend.setTimeStamp(note->start);
        midiMessages.push_back(pitchBend);

        // Create Note Off message
        juce::MidiMessage noteOff = juce::MidiMessage::noteOff(1, midiNoteNumber);
        noteOff.setTimeStamp(note->end);
        midiMessages.push_back(noteOff);

        midiMessagesMap[&noteOn] = std::make_pair(&pitchBend, &noteOff);
    }

    // Sort by timestamp so they play in correct order
    std::sort(midiMessages.begin(), midiMessages.end(),
        [](const juce::MidiMessage& a, const juce::MidiMessage& b)
        {
            return a.getTimeStamp() < b.getTimeStamp();
        });

    channelPool.reset();
    for (auto& midiMessage : midiMessages) {
        if (midiMessage.isNoteOn()) {
            auto channelOptional = channelPool.acquire();
            if (!channelOptional.has_value())
                throw std::out_of_range("No free channel");
            int channel = channelOptional.value();
            midiMessage.setChannel(channel);

            auto [pb, off] = midiMessagesMap[&midiMessage];
            pb->setChannel(channel);
            off->setChannel(channel);

        } else if (midiMessage.isNoteOff()) {
            int channel = midiMessage.getChannel();
            channelPool.release(channel);
        }
    }
}

void NoteRegion::useAsParentToCreate(Note* note, Fraction ratio) {
    auto child = std::make_unique<ChildNote>(*note, ratio, note->start, note->end);
    note->children.push_back(child.get());
    notes.push_back(std::move(child));
}

void NoteRegion::useAsParentToCreate(Note* note, float irratio) {
    auto child = std::make_unique<ChildNote>(*note, irratio, note->start, note->end);
    note->children.push_back(child.get());
    notes.push_back(std::move(child));
}