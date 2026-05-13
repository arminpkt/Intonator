//
// Created by Vos on 28/10/2025.
//

#pragma once

#include <vector>

#include "Note.h"
#include "PitchClass.h"
#include "../view/Types.h"


double dists_sum_sq(const std::vector<PitchClass>& freq_as, const std::vector<PitchClass>& freq_bs);
void optimiseDestinationOrder(const std::vector<Note*>& out_as_ordered, std::vector<Note*>& out_bs_ordered);
void optimiseOctaves(const std::vector<Note*>& as_ordered, std::vector<Note*>& bs_ordered);
void optimiseTransition(const std::vector<Note*>& out_as_ordered, std::vector<Note*>& out_bs_ordered);
std::optional<std::vector<int>> getIntRatios(const std::vector<Note*>& notes);
std::vector<int> getIntRatios(std::vector<Fraction> ratios);
int getSmallestPowerAtIndex(const std::vector<Fraction>& fractions, size_t i);
void addToPowersAtIndex(std::vector<Fraction>& fractions, int increment, size_t index);
std::tuple<int, int, int, int, int, int> getTLBRWH(Rect r);
std::tuple<float, float, float, float, float, float> getTLBRWH(RectF r);