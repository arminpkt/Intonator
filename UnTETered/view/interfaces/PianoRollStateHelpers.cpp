//
// Created by Armin Peukert on 17.03.26.
//

#include "PianoRollStateHelpers.h"
#include <unordered_map>

// ---------------------------------------------------------------------------
// NoteRegion  →  PianoRollState
// ---------------------------------------------------------------------------
PianoRollState PianoRollStateHelpers::makeStateFromNoteRegion(const NoteRegion& region)
{
    PianoRollState state;

    state.notes.reserve(region.notes.size());

    for (const auto& notePtr : region.notes)
    {
        StoredPianoNote sn;
        sn.ref       = notePtr->referenceFrequency;
        sn.start     = notePtr->start;
        sn.end       = notePtr->end;
        sn.primePowers = notePtr->ratio.getMonzo().primePowers;
        sn.irratio   = notePtr->irratio;

        state.notes.push_back(sn);
    }

    return state;
}

// ---------------------------------------------------------------------------
// PianoRollState  →  NoteRegion
// ---------------------------------------------------------------------------
NoteRegion PianoRollStateHelpers::makeNoteRegionFromState(const PianoRollState& state, float pitchBendRange)
{
    NoteRegion region;

    region.notes.reserve(state.notes.size());
    for (const auto& sn : state.notes)
    {
        auto ratio = Fraction(Monzo(sn.primePowers));

        region.notes.push_back(std::make_unique<Note>(
            sn.ref,
            ratio,
            sn.irratio,
            sn.start,
            sn.end
        ));
    }

    region.calculateMidiMessages(pitchBendRange, state.concertPitch);
    return region;
}

std::array<int, primes::PrimeCount> PianoRollStateHelpers::makePrimePowersFromVar(juce::var var) {
    juce::String csvString = var.toString();

    juce::StringArray parts = juce::StringArray::fromTokens(csvString, ",", "\"");
    std::array<int, primes::PrimeCount> primePowers{};

    for (int i = 0; i < parts.size(); ++i)
        primePowers[static_cast<size_t>(i)] = parts[i].getIntValue();

    return primePowers;
}

juce::var PianoRollStateHelpers::makeVarFromPrimePowers(std::array<int, primes::PrimeCount> primePowers) {
    juce::StringArray stringParts;
    for (auto& val : primePowers)
    {
        stringParts.add(juce::String(val));
    }
    juce::String csvString = stringParts.joinIntoString(",");

    return {csvString};
}

juce::String PianoRollStateHelpers::makeStringFromIntervals(const std::vector<std::pair<int,int>>& intervals)
{
    juce::StringArray parts;
    for (const auto& [num, den] : intervals)
        parts.add(juce::String(num) + "/" + juce::String(den));
    return parts.joinIntoString(" ");
}

std::vector<std::pair<int,int>> PianoRollStateHelpers::makeIntervalsFromString(const juce::String& str)
{
    std::vector<std::pair<int,int>> out;
    auto parts = juce::StringArray::fromTokens(str, " ", "");
    for (const auto& part : parts)
    {
        auto sides = juce::StringArray::fromTokens(part, "/", "");
        if (sides.size() == 2)
            out.emplace_back(sides[0].getIntValue(), sides[1].getIntValue());
    }
    return out;
}

std::tuple<int, int, int> PianoRollStateHelpers::makeTupleFromColour(const juce::Colour& colour) {
    return std::make_tuple(
        colour.getRed(),
        colour.getGreen(),
        colour.getBlue()
        );
}

juce::Colour PianoRollStateHelpers::makeColourFromTuple(const std::tuple<int, int, int>& tuple) {
    return juce::Colour::fromRGB(
        static_cast<juce::uint8>(std::get<0>(tuple)),
        static_cast<juce::uint8>(std::get<1>(tuple)),
        static_cast<juce::uint8>(std::get<2>(tuple))
        );
}
