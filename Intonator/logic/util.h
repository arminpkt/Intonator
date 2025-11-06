//
// Created by Vos on 28/10/2025.
//

#pragma once

#include "Note.h"
#include <vector>

float dist(const Note& a, const Note& b);
std::vector<float> dists(const std::vector<Note*>& freq_as, const std::vector<Note*>& freq_bs);
void optimiseDestinationOrder(const std::vector<Note>& as, std::vector<Note>& bs);
void optimiseDestinationOrder(const std::vector<Note*>& as, std::vector<Note*>& bs);