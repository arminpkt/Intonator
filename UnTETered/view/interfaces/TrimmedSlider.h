//
// Created by Vos de Mens on 20/10/2026.
//

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class TrimmedSlider : public juce::Slider {
public:
    TrimmedSlider() {
        // setSliderStyle(IncDecButtons);
        // int height = getHeight();
        // setTextBoxStyle(TextBoxLeft, false, 40, height);
        setRange(0.5, 48, 0.1);
    }

    juce::String getTextFromValue(double v) override {
        juce::String s = juce::String(v, 1);
        int dotPos = s.indexOf(".");
        if (dotPos >= 0) {
            s = s.trimEnd();
            while (s.endsWith("0"))
                s = s.dropLastCharacters (1);

            if (s.endsWith ("."))
                s = s.dropLastCharacters (1);
        }

        return s;
    }

    double getValueFromText(const juce::String& text) override {
        auto v =  text.getDoubleValue();
        v = std::round(10 * v) / 10;
        return v;
    }
};