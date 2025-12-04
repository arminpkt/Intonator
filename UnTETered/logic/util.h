//
// Created by Vos on 28/10/2025.
//

#pragma once

#include <vector>

#include "header-only/Note.h"
#include "header-only/PitchClass.h"

float dist(PitchClass& a, PitchClass& b);
float dists_sum(const std::vector<PitchClass>& freq_as, const std::vector<PitchClass>& freq_bs);
void optimiseDestinationOrder(const std::vector<Note*>& out_as_ordered, std::vector<Note*>& out_bs_ordered);
void optimiseOctaves(const std::vector<Note*>& as_ordered, std::vector<Note*>& bs_ordered);
void optimiseTransition(const std::vector<Note*>& out_as_ordered, std::vector<Note*>& out_bs_ordered);