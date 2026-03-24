//
// Created by Vos de Mens on 19/02/2026.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../logic/Note.h"

using Point = juce::Point<int>;
using PointF = juce::Point<float>;
using Rect = juce::Rectangle<int>;
using RectF = juce::Rectangle<float>;
using Kernel = std::vector<std::vector<std::unique_ptr<Note>>>;