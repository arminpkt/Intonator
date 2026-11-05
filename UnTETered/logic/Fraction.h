#pragma once

#include <stdexcept>
#include <utility>

#include "Monzo.h"
#include "FractionNames.h"


class Fraction {
private:
    Monzo monzo;
    mutable std::optional<double> cachedValue;

public:
    Fraction(const int numerator, const int denominator) : monzo(numerator, denominator) {}
    Fraction(const int numerator) : Fraction(numerator, 1) {}
    explicit Fraction(const Monzo m) : monzo(m) {}

    static std::optional<Fraction> fromString(const juce::String& text) {
        auto trimmed = text.trim();
        auto parts = juce::StringArray::fromTokens(trimmed, "/", "");

        if (parts.size() == 1)
            parts.add("1");

        if (parts.size() != 2)
            return std::nullopt;

        if (!parts[0].containsOnly("0123456789") ||
            !parts[1].containsOnly("0123456789"))
            return std::nullopt;

        int a = parts[0].getIntValue();
        int b = parts[1].getIntValue();

        if (b == 0)
            return std::nullopt;

        try {
            return Fraction(a, b);
        } catch (...) {
            return std::nullopt;
        }
    }

    [[nodiscard]] std::pair<int, int> getNumeratorAndDenominator() const {
        return monzo.getNumeratorAndDenominator();
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

    explicit operator double() const {
        if (!cachedValue)
            cachedValue = static_cast<double>(monzo);
        return cachedValue.value();
    }

    double toDouble() const {
        return static_cast<double>(*this);
    }

    static std::pair<std::vector<int>, Fraction> getIntRatiosAndScalingFactor(std::vector<Fraction> fractions) {
        Monzo scalingFactor{1, 1};

        for (size_t i = 0; i < Primes::PrimeCount; ++i) {
            int smallestPower = getSmallestPowerAtIndex(fractions, i);
            addToPowersAtIndex(fractions, -smallestPower, i);
            scalingFactor.primePowers[i] = -smallestPower;
        }

        std::vector<int> intRatios;
        intRatios.reserve(fractions.size());
        for (const auto& fraction : fractions)
            intRatios.push_back(fraction.getNumeratorAndDenominator().first);

        return std::make_pair(intRatios, Fraction(scalingFactor));
    }

    static std::vector<int> getIntRatios(const std::vector<Fraction>& fractions) {
        return getIntRatiosAndScalingFactor(fractions).first;
    }

    static int getSmallestPowerAtIndex(const std::vector<Fraction>& fractions, const size_t i) {
        int smallestPower = std::numeric_limits<int>::max();
        for (const auto& fraction : fractions)
            if (const int primePower = fraction.getMonzo().primePowers[i]; primePower < smallestPower)
                smallestPower = primePower;
        return smallestPower;
    }

    static void addToPowersAtIndex(std::vector<Fraction>& fractions, int increment, size_t index) {
        Fraction toMultiply = Fraction{Primes::Primes[index], 1} ^ increment;
        for (auto& fraction : fractions) {
            fraction = fraction * toMultiply;
        }
    }

    juce::String toString() const {
        const auto [numerator, denominator] = getNumeratorAndDenominator();
        return fractionNames::toString(numerator, denominator);
    }

    std::optional<juce::String> getName() const {
        if (isContractive())
            return reciprocal().getName();

        if (fractionNames::NAMES.count(toString()))
            return fractionNames::NAMES.at(toString());

        Fraction copy = *this;
        int nrOfOctaves = 0;
        while ((copy / 2).isExpansive()) {
            copy = copy / 2;
            nrOfOctaves++;
        }
        if (fractionNames::NAMES.count(copy.toString())) {
            auto reducedName = fractionNames::NAMES.at(copy.toString());
            if (reducedName.equalsIgnoreCase("octave"))
                return juce::String(nrOfOctaves + 1) + " octaves";
            juce::String name = reducedName + " +" + juce::String(nrOfOctaves) + "oct";
            return name;
        }

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

    bool isExpansive() const {
        return toDouble() > 1;
    }

    bool isContractive() const {
        return toDouble() < 1;
    }

    bool isInteger() const {
        for (size_t i = 0; i < monzo.primePowers.size(); ++i)
            if (monzo.primePowers[i] < 0)
                return false;
        return true;
    }

    bool isOctavial() const {
        for (size_t i = 1; i < monzo.primePowers.size(); ++i)
            if (monzo.primePowers[i] != 0)
                return false;
        return true;
    }
};
