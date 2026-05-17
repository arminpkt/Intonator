//
// Created by Vos de Mens on 13/05/2026.
//

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "FractionsField.h"

enum LockY{locked, snap, continuous};
enum Reference{selectedNote, lockNote, customRef};

class PianoRollSettingsBar : public juce::Component {
public:
    const int MARGIN = 5;
    const juce::Colour COMBOBOX_BACKGROUND_COLOUR = {70, 70, 70};

    explicit PianoRollSettingsBar(
        std::function<void()> handleLock,
        std::function<void()> handleRef,
        std::function<void()> handleFrac
        );

    LockY getLockY() const;
    Reference getReference() const;
    std::vector<Fraction> getPotentialRatios() const;

    void setLockY(LockY lockY);
    void setReference(Reference reference);
    void setPotentialRatios(const std::vector<Fraction>& fractions);

private:
    juce::ComboBox lockYComboBox;
    juce::ComboBox referenceComboBox;
    FractionsField potentialRatiosField;

    std::function<void()> handleLockYChange;
    std::function<void()> handleReferenceChange;
    std::function<void()> handlePotentialRatiosChange;
    void initialiseLockY();
    void initialiseReference();
    void initialisePotentialRatios();

    void resized() override;
};
