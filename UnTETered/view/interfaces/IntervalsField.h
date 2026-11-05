//
// Created by Vos de Mens on 17/05/2026.
//

#pragma once

#include "../../logic/Interval.h"
#include <juce_audio_processors/juce_audio_processors.h>

struct IntervalsField : juce::TextEditor {
    std::vector<Interval> getIntervals() const {
        auto fractionsText = getText();
        auto splat = juce::StringArray::fromTokens(fractionsText, ",", "");

        std::vector<Interval> intervals;
        for (auto& s : splat)
            if (auto fracOptional = Interval::fromString(s))
                intervals.push_back(fracOptional.value());

        return intervals;
    }

    void setIntervals(const std::vector<Interval>& intervals) {
        if (intervals.empty())
            setText("");

        juce::String string = "";

        for (auto& interval : intervals)
            string += interval.toString() + ", ";

        string = string.dropLastCharacters(2);

        setText(string, false);
    }
};
