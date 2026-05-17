//
// Created by Armin Peukert on 28.10.25.
//

#include "NoteRegion.h"
#include <algorithm>

void NoteRegion::addNoteWithoutReference(double frequency, float start, float end) {
    addNoteWithRefFreq(frequency, {1, 1}, 1, start, end);
}

void NoteRegion::addNoteWithRefNote(Note* reference, Fraction ratio, double irratio, float start, float end) {
    Fraction ratioTotal = ratio * reference->ratio;
    double irratioTotal = irratio * reference->irratio;
    addNoteWithRefFreq(reference->referenceFrequency, ratioTotal, irratioTotal, start, end);
}

void NoteRegion::addNoteWithRefFreq(double refFreq, Fraction ratio, double irratio, double start, double end) {
    notes.push_back(std::make_unique<Note>(refFreq, ratio, irratio, start, end));
}

void NoteRegion::deleteNote(Note* note) {
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
    for (const auto& note : notes)
    {
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

        activeChannels.emplace_back( event.endTime, channel );
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

               return midiEventPriority(a) < midiEventPriority(b);
           });
}

int NoteRegion::midiEventPriority(const juce::MidiMessage& m)
{
    if (m.isNoteOff())    return 0;
    if (m.isPitchWheel()) return 1;
    if (m.isNoteOn())     return 2;
    return 3;
}