//
// Created by Armin Peukert on 17.03.26.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <unordered_set>
#include <vector>

using Point = juce::Point<int>;

struct PointHash
{
    std::size_t operator()(const Point& p) const
    {
        return std::hash<int>{}(p.x) ^ (std::hash<int>{}(p.y) << 1);
    }
};

using PointSet = std::unordered_set<Point, PointHash>;

struct SaveSlotState
{
    char mode = 0;
    std::vector<Point> screenCells;
};

struct GridState
{
    double originFreqHz = 220.0;
    int offsetX = 0;
    int offsetY = 0;
    PointSet activeCells;
    PointSet selectedCells;
    std::array<SaveSlotState, 91> saves;
};
