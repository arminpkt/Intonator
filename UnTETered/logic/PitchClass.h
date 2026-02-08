//
// Created by Vos on 10/11/2025.
//

#pragma once

#include <cmath>

struct PitchClass {
    double value;

    PitchClass(double v = 0.0f) {
        value = wrap(v);
    }

    static double wrap(double v) {
        v = std::fmod(v, 12.0f);
        if (v < 0) v += 12.0f;
        return v;
    }

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
    bool operator<(const PitchClass& other) const {
        return value < other.value;
    }

    PitchClass operator+(const double rhs) const { return {value + rhs}; }
    PitchClass operator-(const double rhs) const { return {value - rhs}; }
    PitchClass operator*(const double rhs) const { return {value * rhs}; }
    PitchClass operator/(const double rhs) const { return {value / rhs}; }

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
    PitchClass& operator+=(const double rhs) {
        value = wrap(value + rhs);
        return *this;
    }
    PitchClass& operator-=(const double rhs) {
        value = wrap(value - rhs);
        return *this;
    }
    PitchClass& operator*=(const double rhs) {
        value = wrap(value * rhs);
        return *this;
    }
    PitchClass& operator/=(const double rhs) {
        value = wrap(value / rhs);
        return *this;
    }
};

inline double dist(const PitchClass a, const PitchClass b) {
    const double one_way = (a - b).value;
    const double other_way = (b - a).value;
    return one_way < other_way ? one_way : other_way;
}