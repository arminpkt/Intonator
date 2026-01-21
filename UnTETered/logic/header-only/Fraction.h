#pragma once

#include <stdexcept>
#include <utility>

#include "Monzo.h"

class Fraction {
private:
    Monzo monzo;
    mutable std::optional<double> cachedValue;

    explicit Fraction(const Monzo m) : monzo(m) {}

public:
    Fraction(const int numerator, const int denominator) : monzo(numerator, denominator) {}

    [[nodiscard]] std::pair<int, int> getNumeratorAndDenominator() const {
        return monzo.getNumeratorAndDenominator();
    }

    explicit operator double() const {
        if (!cachedValue)
            cachedValue = static_cast<double>(monzo);
        return cachedValue.value();
    }

    Fraction operator*(const Fraction& other) const {
        const Monzo m = monzo + other.monzo;
        return Fraction(m);
    }

    Fraction operator/(const Fraction& other) const {
        const Monzo m = monzo - other.monzo;
        return Fraction(m);
    }

    friend std::ostream& operator<<(std::ostream& os, Fraction& f) {
        const auto [numerator, denominator] = f.getNumeratorAndDenominator();
        return os << std::to_string(numerator) << "/" << std::to_string(denominator);
    }

    Fraction operator/(const int i) const {
        if (i == 0)
            throw std::invalid_argument("Cannot divide by zero.");
        const Fraction toDivideBy = {i, 1};
        return *this / toDivideBy;
    }

    friend Fraction operator/(const int i, const Fraction& f) {
        const Fraction toBeDivided = {i, 1};
        return toBeDivided / f;
    }

    Fraction operator*(const int i) const {
        const Fraction toMultiply = {i, 1};
        return *this * toMultiply;
    }

    friend Fraction operator*(const int i, const Fraction& f) {
        const Fraction toMultiply = {i, 1};
        return f * toMultiply;
    }
};