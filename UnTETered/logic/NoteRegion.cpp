//
// Created by Armin Peukert on 28.10.25.
//

#include "NoteRegion.h"
#include <algorithm>

void NoteRegion::addRootNote(double frequency, float start, float end) {
    notes.push_back(std::make_unique<RootNote>(frequency, start, end));
    // calculateMidiMessages();
}

void NoteRegion::addChildNote(Note* parent, Fraction ratio, double irratio, float start, float end) {
    notes.push_back(std::make_unique<ChildNote>(parent, ratio, irratio, start, end));
    // calculateMidiMessages();
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

    std::unordered_map<juce::MidiMessage*, std::pair<juce::MidiMessage*, juce::MidiMessage*>> midiMessagesMap;

    for (const auto& notePtr : notes)
    {
        const Note* note = notePtr.get();
        if (!note)
            continue;

        // Get relevant info from note
        const int midiNoteNumber = note->getRoundedMidiValue();
        const auto pitchBendValue = note->getPitchBendValue(pitchBendRange);

        // Create Note On message
        juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, midiNoteNumber, static_cast<juce::uint8>(100));
        noteOn.setTimeStamp(note->start);
        midiMessages.push_back(noteOn);

        // Create Pitchbend message
        juce::MidiMessage pitchBend = juce::MidiMessage::pitchWheel(1, pitchBendValue);
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
        [](const juce::MidiMessage& a, const juce::MidiMessage& b) {
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