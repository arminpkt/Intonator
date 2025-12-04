//
// Created by Vos on 28/10/2025.
//

#pragma once

#include <vector>

#include "header-only/Note.h"
#include "header-only/PitchClass.h"


float dist(PitchClass& a, PitchClass& b);
float dists_sum(const std::vector<PitchClass>& freq_as, const std::vector<PitchClass>& freq_bs);
void optimiseDestinationOrder(const std::vector<std::unique_ptr<Note>>& as, std::vector<std::unique_ptr<Note>>& bs);
void optimiseOctaves(const std::vector<std::unique_ptr<Note>>& as, std::vector<std::unique_ptr<Note>>& bs);
void optimiseTransition(const std::vector<std::unique_ptr<Note>>& as, std::vector<std::unique_ptr<Note>>& bs);