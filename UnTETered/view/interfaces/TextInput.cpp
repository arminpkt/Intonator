//
// Created by Vos de Mens on 03/02/2026.
//

#include "TextInput.h"
#include "../../logic/Fraction.h"

int TextInput::getHeight() {
    return 3 * FIELD_HEIGHT + 4 * MARGIN;
}

TextInput::TextInput(UnTETeredAudioProcessor& proc) :
    processor(proc) {
    addAndMakeVisible(freqRootField);
    freqRootField.setTextToShowWhenEmpty("freq", juce::Colours::grey);
    freqRootField.setJustification(juce::Justification::centredLeft);

    addAndMakeVisible(fractionsField);
    fractionsField.setTextToShowWhenEmpty("a/b, c/d, ...", juce::Colours::grey);
    fractionsField.setJustification(juce::Justification::centredLeft);

    addAndMakeVisible(button);
    button.setButtonText("button");
    button.onClick = [this]{evaluateAndSendMidi();};
}

void TextInput::resized() {
    auto area = getLocalBounds().reduced(MARGIN);
    freqRootField.setBounds(area.removeFromTop(FIELD_HEIGHT));
    area.removeFromTop(MARGIN);

    fractionsField.setBounds(area.getX(), area.getY(), area.getWidth(), FIELD_HEIGHT);
    area.removeFromTop(FIELD_HEIGHT + MARGIN);

    button.setBounds(area.getX(), area.getY(), 2 * FIELD_WIDTH, FIELD_HEIGHT);
}

void TextInput::evaluateAndSendMidi() {
    for (size_t i = 0; i < activeNotes.size(); ++i) {
        const Note* note = activeNotes[i].get();

        auto noteOff = juce::MidiMessage::noteOff(
            static_cast<int>(i + 2), note->getRoundedMidiValue());
        processor.midiBuffer.addEvent(noteOff, 10);
    }
    std::vector<std::unique_ptr<Note>> nextActiveNotes;

    auto freqText = freqRootField.getText();
    bool freqIsNumerical = freqText.containsOnly("0123456789.");
    if (freqText.isEmpty() || !freqIsNumerical)
        return;

    double freq = freqText.getDoubleValue();

    auto fractionsText = fractionsField.getText();
    auto splat = juce::StringArray::fromTokens(fractionsText, ",", "");
    for (auto& s : splat) {
        if (auto fracOptional = Fraction::fromString(s)) {
            auto note = std::make_unique<Note>(freq, fracOptional.value(), 1, 0, 0);
            nextActiveNotes.push_back(std::move(note));
        }
    }

    for (size_t i = 0; i < nextActiveNotes.size(); ++i)
    {
        const Note* note = nextActiveNotes[i].get();

        auto noteOn = juce::MidiMessage::noteOn(
            static_cast<int>(i + 2), note->getRoundedMidiValue(),
            static_cast<juce::uint8>(100));
        processor.midiBuffer.addEvent(noteOn, 20);

        auto pitchBendValue = note->getPitchBendValue();
        auto pitchBend = juce::MidiMessage::pitchWheel(
            static_cast<int>(i + 2), pitchBendValue);
        processor.midiBuffer.addEvent(pitchBend, 30);
    }

    activeNotes = std::move(nextActiveNotes);
}
