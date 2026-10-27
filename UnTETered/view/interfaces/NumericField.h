//
// Created by Vos de Mens on 23/10/2026.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class NumericField : public juce::TextEditor {
public:
    NumericField() {
        setJustification(juce::Justification::right);
    }

    float getFloatValue() const {
        return getText().getFloatValue();
    }

    void setFloatValue(float v) {
        setText(juce::String(v));
    }
};