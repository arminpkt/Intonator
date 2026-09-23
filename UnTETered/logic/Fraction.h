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

    Fraction reciprocal() const {
        return *this^-1;
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

    juce::String toString() const {
        const auto [numerator, denominator] = getNumeratorAndDenominator();
        return std::to_string(numerator) + "/" + std::to_string(denominator);
    }

    std::optional<juce::String> getName() const {
        if (NAMES.count(toString()))
            return NAMES.at(toString());
        return std::nullopt;
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

    double getSizeInSemitones() const {
        double ratioLog = std::log2(static_cast<double>(*this));
        return ratioLog * 12;
    }

    int getSizeInCents() const {
        return static_cast<int>(std::round(getSizeInSemitones() * 100));
    }

    inline static const std::unordered_map<juce::String, juce::String> NAMES = {
        {"1/1", "unison"},
        {"2/1", "octave"},
        {"1/2", "octave"},
        {"3/2", "just perfect fifth"},
        {"2/3", "just perfect fifth"},
        {"4/3", "just perfect fourth"},
        {"3/4", "just perfect fourth"},
        {"5/4", "just major third"},
        {"4/5", "just major third"},
        {"6/5", "just minor third"},
        {"5/6", "just minor third"},
        {"9/8", "Pythagorean major second"},
        {"8/9", "Pythagorean major second"},
        {"7/4", "harmonic seventh"},
        {"4/7", "harmonic seventh"},
        {"7/5", "narrow tritone"},
        {"5/7", "narrow tritone"},
        {"16/15", "just diatonic semitone"},
        {"15/16", "just diatonic semitone"},
        {"5/3", "just major sixth"},
        {"3/5", "just major sixth"},
        {"8/5", "just minor sixth"},
        {"5/8", "just minor sixth"},
        {"15/8", "just major seventh"},
        {"8/15", "just major seventh"},
    };
};
