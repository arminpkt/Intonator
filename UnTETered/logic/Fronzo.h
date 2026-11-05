//
// Created by Vos de Mens on 28/10/2026.
//

#pragma once

#include "FractionU.h"

struct Fronzo {
    std::array<FractionU, Primes::PrimeCount> primePowers;

    Fronzo(int numerator, int denominator) : Fronzo(Monzo(numerator, denominator)) {}
    explicit Fronzo(int numerator) : Fronzo(Monzo(numerator, 1)) {}
    Fronzo(int steps, int divisions, Fraction toDivide)
        : Fronzo(toDivide.getMonzo()) {
        if (divisions <= 1)
            throw std::invalid_argument("Divisions must be at least 2");
        if (!toDivide.isExpansive())
            throw std::invalid_argument("Fraction to divide must be expansive");
        Fraction toMultiply = Fraction(steps, divisions);
        for (size_t i = 0; i < Primes::PrimeCount; i++)
            primePowers[i] = primePowers[i] * toMultiply;
    }
    Fronzo(const Monzo& monzo) {
        for (size_t i = 0; i < Primes::PrimeCount; i++)
            primePowers[i] = FractionU(monzo.primePowers[i]);
    }
    explicit Fronzo(const std::array<FractionU, Primes::PrimeCount>& pP) : primePowers(pP) {}

    Fronzo operator-() const {
        std::array<FractionU, Primes::PrimeCount> negatedPowers{};
        for (size_t i = 0; i < primePowers.size(); ++i)
            negatedPowers[i] = -primePowers[i];
        return Fronzo(negatedPowers);
    }

    Fronzo operator+(const Fronzo& other) const {
        std::array<FractionU, Primes::PrimeCount> added_powers{};
        for (size_t i = 0; i < Primes::PrimeCount; ++i)
            added_powers[i] = primePowers[i] + other.primePowers[i];
        return Fronzo(added_powers);
    }

    Fronzo operator-(const Fronzo& other) const {
        return *this + -other;
    }

    explicit operator double() const {
        double logv = 0.0;
        for (size_t i = 0; i < Primes::PrimeCount; ++i)
            if (primePowers[i].fraction)
                logv += primePowers[i].toDouble() * Primes::PrimeLogs[i];
        return std::exp(logv);
    }

    friend Fronzo operator*(const int factor, const Fronzo& fronzo) {
        std::array<FractionU, Primes::PrimeCount> multiplied_powers{};
        for (size_t i = 0; i < Primes::PrimeCount; ++i)
            multiplied_powers[i] = fronzo.primePowers[i] * factor;
        return Fronzo(multiplied_powers);
    }

    juce::String toString() const {
        size_t lastNonZero;
        for (size_t i = Primes::PrimeCount - 1; ; --i)
            if (primePowers[i].fraction) {
                lastNonZero = i;
                break;
            }

        juce::String s = "[";
        for (size_t i = 0; i <= lastNonZero; ++i) {
            s += primePowers[i].toString();
            if (i < lastNonZero)
                s += ", ";
        }
        s += "⟩";

        return s;
    }

    friend std::ostream& operator<<(std::ostream& out, const Fronzo& fronzo) {
        out << fronzo.toString();
        return out;
    }

    bool operator==(const Fronzo& fronzo) const {
        return primePowers == fronzo.primePowers;
    }
};
