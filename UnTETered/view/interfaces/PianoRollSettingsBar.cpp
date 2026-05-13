//
// Created by Vos de Mens on 14/05/2026.
//

#include "PianoRollSettingsBar.h"

PianoRollSettingsBar::PianoRollSettingsBar(FreedomY freedom) {
    initialiseFreedom(freedom);
}

void PianoRollSettingsBar::initialiseFreedom(FreedomY freedom) {
    addAndMakeVisible(freedomSetting);
    freedomSetting.addItem("locked", locked+1);
    freedomSetting.addItem("snap", snap+1);
    freedomSetting.addItem("continuous", continuous+1);
    freedomSetting.setSelectedId(freedom+1);
}

PianoRollSettingsBar::FreedomY PianoRollSettingsBar::getFreedomY() const {
    int id = freedomSetting.getSelectedId();
    return FreedomY(id-1);
}

void PianoRollSettingsBar::resized() {
    auto bounds = getLocalBounds().reduced(MARGIN);
    freedomSetting.setBounds(bounds.removeFromLeft(120));
}