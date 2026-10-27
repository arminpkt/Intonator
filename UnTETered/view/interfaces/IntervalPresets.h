//
// Created by Vos de Mens on 25/05/2026.
//

#pragma once

#include <vector>
#include "../../logic/Fraction.h"

constexpr int STANDARD_ID = 1;
constexpr int CUSTOM_INTERVALS_ID = 2;

const std::vector<Fraction> STANDARD = {
    {1, 1},
    {2, 1},
    {3, 2},
    {4, 3},
    {5, 4},
    {5, 3},
    {8, 5},
    {6, 5},
    {7, 4},
    {9, 8},
    {1, 2},
    {2, 3},
    {3, 4},
    {4, 5},
    {3, 5},
    {5, 8},
    {5, 6},
    {4, 7},
    {8, 9},
    {7, 5},
    {5, 7},
    {16, 15},
    {15, 8},
    {15, 16},
    {8, 15}
};

inline std::vector<Fraction> getIntervalsByID(const int id) {
    if (id == STANDARD_ID) {
        return STANDARD;
    }
    throw std::invalid_argument("invalid interval ID");
}