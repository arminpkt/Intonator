#pragma once

#include <stdexcept>
#include <utility>

#include "Monzo.h"


class Fraction {
private:
    Monzo monzo;
    mutable std::optional<double> cachedValue;

public:
    Fraction(const int numerator, const int denominator) : monzo(numerator, denominator) {}
    explicit Fraction(const Monzo m) : monzo(m) {}

    static std::optional<Fraction> fromString(const juce::String& text) {
        auto trimmed = text.trim();
        auto parts = juce::StringArray::fromTokens(trimmed, "/", "");

        if (parts.size() != 2)
            return std::nullopt;

        if (!parts[0].containsOnly("0123456789") ||
            !parts[1].containsOnly("0123456789"))
            return std::nullopt;

        int a = parts[0].getIntValue();
        int b = parts[1].getIntValue();

        if (b == 0)
            return std::nullopt;

        return Fraction(a, b);
    }

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

    Fraction operator^(const int power) const {
        const Monzo m = power * monzo;
        return Fraction(m);
    }

    std::string toString() const {
        const auto [numerator, denominator] = getNumeratorAndDenominator();
        return std::to_string(numerator) + "/" + std::to_string(denominator);
    }

    friend std::ostream& operator<<(std::ostream& os, Fraction& f) {
        return os << f.toString();
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

    bool operator==(const Fraction& fraction) const {
        return monzo == fraction.monzo;
    }

    Monzo getMonzo() const {
        return monzo;
    }
};
