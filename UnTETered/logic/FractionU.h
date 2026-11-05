//
// Created by Vos de Mens on 31/10/2026.
//

#pragma once
#include "Fraction.h"

struct FractionU {
    std::optional<Fraction> fraction;
    bool positive;

    FractionU(const int numerator, const int denominator) : fraction({}), positive(false) {
        if (denominator == 0)
            throw std::invalid_argument("Division by zero");
        if (numerator == 0) {
            fraction = std::nullopt;
            positive = true;
            return;
        }
        if (numerator > 0 && denominator > 0) {
            fraction = Fraction(numerator, denominator);
            positive = true;
            return;
        }
        if (numerator < 0 && denominator < 0) {
            fraction = Fraction(-numerator, -denominator);
            positive = true;
            return;
        }
        if (numerator < 0 && denominator > 0) {
            fraction = Fraction(-numerator, denominator);
            positive = false;
            return;
        }
        if (numerator > 0 && denominator < 0) {
            fraction = Fraction(numerator, -denominator);
            positive = false;
            return;
        }
    }
    FractionU(const int numerator) : FractionU(numerator, 1) {}
    FractionU(Fraction f, bool pos) : fraction(f), positive(pos) {}
    FractionU() : fraction({}), positive(true) {}

    [[nodiscard]] std::pair<int, int> getNumeratorAndDenominator() const {
        if (!fraction)
            return std::make_pair(0, 1);
        auto [num, denom] = fraction.value().getNumeratorAndDenominator();
        if (!positive)
            num *= -1;
        return std::make_pair(num, denom);
    }

    FractionU operator+(const FractionU& fu) const {
        if (!fraction)
            return fu;
        if (!fu.fraction)
            return *this;

        auto [intRatios, scalingFactor] =
            Fraction::getIntRatiosAndScalingFactor({fraction.value(), fu.fraction.value()});

        auto num = (positive ? intRatios[0] : -intRatios[0]) + (fu.positive ? intRatios[1] : -intRatios[1]);

        if (num == 0)
            return FractionU{};

        bool pos = num > 0;
        if (num < 0)
            num *= -1;

        Fraction result = scalingFactor.reciprocal() * num;
        return {result, pos};
    }

    FractionU operator+(const int i) const {
        return *this + FractionU(i);
    }

    FractionU operator-() const {
        if (!fraction)
            return {};
        return {fraction.value(), !positive};
    }

    FractionU operator-(const FractionU& fu) const {
        return *this + -fu;
    }

    FractionU operator-(const int i) const {
        return *this - FractionU(i);
    }

    FractionU operator*(const FractionU& fu) const {
        if (!fraction || !fu.fraction)
            return {};
        bool pos = (positive && fu.positive) || (!positive && !fu.positive);
        return {fraction.value() * fu.fraction.value(), pos};
    }

    FractionU operator*(const Fraction& f) const {
        if (!fraction)
            return {};
        return {fraction.value() * f, positive};
    }

    FractionU operator*(const int i) const {
        const FractionU toMultiply = i;
        return *this * toMultiply;
    }

    explicit operator double() const {
        if (!fraction)
            return 0;
        return positive * fraction.value().toDouble();
    }

    double toDouble() const {
        return static_cast<double>(*this);
    }

    int roundTowardsZero() const {
        if (!fraction)
            return 0;
        int value = static_cast<int>(fraction->toDouble());
        if (!positive)
            value *= -1;
        return value;
    }

    juce::String toString() const {
        if (!fraction)
            return {"0"};
        const auto [numerator, denominator] = fraction.value().getNumeratorAndDenominator();
        const juce::String prefix = positive ? "" : "-";
        return prefix + fractionNames::toString(numerator, denominator);
    }

    friend std::ostream& operator<<(std::ostream& os, const FractionU& fu) {
        return os << fu.toString();
    }

    bool operator==(const FractionU& fu) const {
        return fraction == fu.fraction && positive == fu.positive;
    }

    bool isInteger() const {
        if (!fraction)
            return true;
        return fraction.value().isInteger();
    }
};
