//
// Created by Vos de Mens on 17/05/2026.
//

#pragma once

#include "../../logic/Fraction.h"
#include <juce_audio_processors/juce_audio_processors.h>

struct FractionsField : juce::TextEditor {
    std::vector<Fraction> getFractions() const {
        auto fractionsText = getText();
        auto splat = juce::StringArray::fromTokens(fractionsText, ",", "");

        std::vector<Fraction> fractions;
        for (auto& s : splat)
            if (auto fracOptional = Fraction::fromString(s))
                fractions.push_back(fracOptional.value());

        return fractions;
    }

    void setFractions(std::vector<Fraction> fractions) {
        if (fractions.empty())
            setText("");

        std::string string = "";

        for (auto& fraction : fractions)
            string += fraction.toString() + ", ";

        string.pop_back();
        string.pop_back();

        setText(string);
    }
};
