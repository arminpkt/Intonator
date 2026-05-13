//
// Created by Vos de Mens on 13/05/2026.
//

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class PianoRollSettingsBar : public juce::Component {
public:
    int MARGIN = 8;
    enum FreedomY{locked, snap, continuous};
    PianoRollSettingsBar(FreedomY freedom = locked);

    FreedomY getFreedomY() const;

private:
    juce::ComboBox freedomSetting;

    void initialiseFreedom(FreedomY freedom);
    void resized() override;
};
