//
// Created by Vos on 10/11/2025.
//

#pragma once

#include <cmath>

struct PitchClass {
    float value;

    // Construct and normalize immediately
    PitchClass(float v = 0.0f) {
        value = wrap(v);
    }

    // Normalize helper (inline)
    static float wrap(float v) {
        v = std::fmod(v, 12.0f);
        if (v < 0) v += 12.0f;
        return v;
    }

    // Arithmetic with another ModFloat12
    PitchClass operator+(const PitchClass& other) const {
        return {value + other.value};
    }
    PitchClass operator-(const PitchClass& other) const {
        return {value - other.value};
    }
    PitchClass operator*(const PitchClass& other) const {
        return {value * other.value};
    }
    PitchClass operator/(const PitchClass& other) const {
        return {value / other.value};
    }

    // Arithmetic with plain float
    PitchClass operator+(float rhs) const { return {value + rhs}; }
    PitchClass operator-(float rhs) const { return {value - rhs}; }
    PitchClass operator*(float rhs) const { return {value * rhs}; }
    PitchClass operator/(float rhs) const { return {value / rhs}; }

    // Compound assignment
    PitchClass& operator+=(const PitchClass& other) {
        value = wrap(value + other.value);
        return *this;
    }
    PitchClass& operator-=(const PitchClass& other) {
        value = wrap(value - other.value);
        return *this;
    }
    PitchClass& operator*=(const PitchClass& other) {
        value = wrap(value * other.value);
        return *this;
    }
    PitchClass& operator/=(const PitchClass& other) {
        value = wrap(value / other.value);
        return *this;
    }
    PitchClass& operator+=(float rhs) {
        value = wrap(value + rhs);
        return *this;
    }
    PitchClass& operator-=(float rhs) {
        value = wrap(value - rhs);
        return *this;
    }
    PitchClass& operator*=(float rhs) {
        value = wrap(value * rhs);
        return *this;
    }
    PitchClass& operator/=(float rhs) {
        value = wrap(value / rhs);
        return *this;
    }
};

inline float dist(const PitchClass a, const PitchClass b) {
    float one_way = (a - b).value;
    float other_way = (b - a).value;
    return one_way < other_way ? one_way : other_way;
}