//
// Created by Vos de Mens on 03/02/2026.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../PluginProcessor.h"
#include "../../logic/Note.h"


class TextInput : public juce::Component {
public:
    static constexpr int FIELD_WIDTH  = 50;
    static constexpr int FIELD_HEIGHT = 28;
    static constexpr int MARGIN = 10;

    static int getHeight();

    explicit TextInput(UnTETeredAudioProcessor& proc);
    ~TextInput() override = default;

private:
    UnTETeredAudioProcessor& processor;

    juce::TextEditor freqRootField;
    juce::TextEditor fractionsField;
    juce::TextButton button;

    std::vector<std::unique_ptr<Note>> activeNotes;

    void resized() override;
    void evaluateAndSendMidi();
};
