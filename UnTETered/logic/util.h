//
// Created by Vos on 28/10/2025.
//

#pragma once

#include <vector>

#include "Note.h"
#include "PitchClass.h"
#include "../view/Types.h"


double distsSumSq(const std::vector<PitchClass>& freq_as, const std::vector<PitchClass>& freq_bs);
double optimiseDestinationOrder(std::vector<Note*>& out_as_ordered, std::vector<Note*>& out_bs_ordered);
void optimiseOctaves(const std::vector<Note*>& as_ordered, std::vector<Note*>& bs_ordered);
void optimiseTransition(std::vector<Note*>& out_as_ordered, std::vector<Note*>& out_bs_ordered);
std::optional<std::vector<int>> getIntRatios(const std::vector<Note*>& notes, bool oddified = false);
std::vector<int> getIntRatios(std::vector<Fraction> ratios, bool oddified = false);
int getSmallestPowerAtIndex(const std::vector<Fraction>& fractions, size_t i);
void addToPowersAtIndex(std::vector<Fraction>& fractions, int increment, size_t index);
std::tuple<int, int, int, int, int, int> getTLBRWH(Rect r);
std::tuple<float, float, float, float, float, float> getTLBRWH(RectF r);

std::vector<std::vector<Note*>> divideIntoVoices(std::vector<Note*> notes);
std::vector<Note*> findFirstChordForSorted(std::vector<Note*> notes);
void updateVoices(std::vector<std::vector<Note*>>& voices, std::vector<Note*>& nextChord);