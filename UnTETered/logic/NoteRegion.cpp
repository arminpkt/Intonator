//
// Created by Armin Peukert on 28.10.25.
//

#include "NoteRegion.h"
#include <algorithm>

void NoteRegion::addRootNote(double frequency, float start, float end) {
    notes.push_back(std::make_unique<RootNote>(frequency, start, end));
    calculateMidiMessages();
}

void NoteRegion::addChildNote(Note* parent, Fraction ratio, double irratio, float start, float end) {
    notes.push_back(std::make_unique<ChildNote>(parent, ratio, irratio, start, end));
    calculateMidiMessages();
}

void NoteRegion::addNote(std::unique_ptr<Note>* note) {
    notes.push_back(std::move(*note));
}

void NoteRegion::deleteNote(Note* note) {
    if (!note)
        return;

    ChildNote* asChild = nullptr;
    RootNote* asRoot = nullptr;
    try {
        asChild = dynamic_cast<ChildNote*>(note);
        asRoot = dynamic_cast<RootNote*>(note);
    }
    catch (const std::exception& e) {
        DBG("oop");
    }
    if (asChild) {
        asChild->abandonChildren();
    }
    if (asRoot)
        for (auto& child : asRoot->children) {
            auto newRoot = std::make_unique<RootNote>(child->frequency, child->start, child->end);
            newRoot->children = std::move(child->children);
            notes.push_back(std::move(newRoot));
        }

    for (size_t i = 0; i < notes.size(); ++i)
        if (notes[i].get() == note) {
            notes.erase(notes.begin() + static_cast<long int>(i));
            return;
        }
}

void NoteRegion::calculateMidiMessages(const float pitchBendRange) {
    midiMessages.clear();
    noteEvents.clear();
    noteEvents.reserve(notes.size());

    // Build one NoteEvent per note
    for (const auto& notePtr : notes)
    {
        const Note* note = notePtr.get();
        if (!note)
            continue;

        const int midiNoteNumber = note->getRoundedMidiValue();
        const int pitchBendValue = note->getPitchBendValue(pitchBendRange);

        NoteEvent event
        {
            note->start,
            note->end,
            juce::MidiMessage::noteOn(1, midiNoteNumber, static_cast<juce::uint8>(100)),
            juce::MidiMessage::pitchWheel(1, pitchBendValue),
            juce::MidiMessage::noteOff(1, midiNoteNumber)
        };

        event.noteOn.setTimeStamp(note->start);
        event.pitchBend.setTimeStamp(note->start);
        event.noteOff.setTimeStamp(note->end);

        noteEvents.push_back(std::move(event));
    }

    // Sort notes by start time
    std::sort(noteEvents.begin(), noteEvents.end(),
              [](const NoteEvent& a, const NoteEvent& b)
              {
                  return a.startTime < b.startTime;
              });

    channelPool.reset();

    // Active notes: pair of (end time, channel)
    std::vector<std::pair<double, int>> activeChannels;

    for (auto& event : noteEvents)
    {
        // Release all channels whose notes have already ended
        for (auto it = activeChannels.begin(); it != activeChannels.end();)
        {
            if (it->first <= event.startTime)
            {
                channelPool.release(it->second);
                it = activeChannels.erase(it);
            }
            else
            {
                ++it;
            }
        }

        auto channelOptional = channelPool.acquire();
        if (!channelOptional.has_value())
            throw std::out_of_range("No free channel");

        const int channel = *channelOptional;

        event.noteOn.setChannel(channel);
        event.pitchBend.setChannel(channel);
        event.noteOff.setChannel(channel);

        activeChannels.push_back({ event.endTime, channel });
    }

    // Flatten all note events into midiMessages
    midiMessages.reserve(noteEvents.size() * 3);

    for (const auto& event : noteEvents)
    {
        midiMessages.push_back(event.noteOn);
        midiMessages.push_back(event.pitchBend);
        midiMessages.push_back(event.noteOff);
    }

    // Sort final messages by timestamp
    std::sort(midiMessages.begin(), midiMessages.end(),
              [](const juce::MidiMessage& a, const juce::MidiMessage& b)
              {
                  if (a.getTimeStamp() != b.getTimeStamp())
                      return a.getTimeStamp() < b.getTimeStamp();

                  // Optional tie-breaker:
                  // pitch bend before note on before note off at same timestamp
                  if (a.isPitchWheel() != b.isPitchWheel())
                      return a.isPitchWheel();

                  if (a.isNoteOn() != b.isNoteOn())
                      return a.isNoteOn();

                  return false;
              });
}

void NoteRegion::useAsParentToCreate(Note* note, Fraction ratio, float start, float end) {
    auto child = std::make_unique<ChildNote>(note, ratio, start, end);
    note->children.insert(child.get());
    notes.push_back(std::move(child));
}

void NoteRegion::useAsParentToCreate(Note* note, float irratio, float start, float end) {
    auto child = std::make_unique<ChildNote>(note, irratio, start, end);
    note->children.insert(child.get());
    notes.push_back(std::move(child));
}