//
// Created by Vos de Mens on 06/05/2026.
//

#pragma once

#include "Fronzo.h"
#include "regex"

struct Interval {
    Fronzo fronzo;
    mutable std::optional<double> cachedValue;

    Interval(const int steps, const int divisions, const Fraction& toDivide) : fronzo(Fronzo(steps, divisions, toDivide)) {}
    Interval(const Fronzo& f) : fronzo(f) {}
    Interval(const int numerator, const int denominator) : fronzo(numerator, denominator) {}
    Interval(const int numerator) : fronzo(numerator, 1) {}
    Interval(const Fraction& f) : fronzo(f.getMonzo()) {}

    static std::optional<Interval> fromString(const juce::String& s) {
        try {
            const auto native = s.toStdString();
            std::smatch match;

            const std::regex re1(R"(\d+)");
            if (std::regex_match(native, match, re1))
                return Interval(std::stoi(match[0]));

            const std::regex re2(R"((\d+)/(\d+))");
            if (std::regex_match(native, match, re2)) {
                int a = std::stoi(match[1]);
                int b = std::stoi(match[2]);
                return Interval(a, b);
            }

            const std::regex re3(R"((\d+)\(\d+))");
            if (std::regex_match(native, match, re3)) {
                int steps = std::stoi(match[1]);
                int divisions = std::stoi(match[2]);
                return Interval(steps, divisions, {2, 1});
            }

            const std::regex re4(R"((\d+)\(\d+)ed(\d+)/(\d+))");
            if (std::regex_match(native, match, re3)) {
                int steps = std::stoi(match[1]);
                int divisions = std::stoi(match[2]);
                int a = std::stoi(match[3]);
                int b = std::stoi(match[4]);
                return Interval(steps, divisions, {a, b});
            }

            return std::nullopt;
        } catch (...) {
            return std::nullopt;
        }
    }

    Interval reciprocal() const {
        return *this^-1;
    }

    Interval operator*(const Interval& other) const {
        const Fronzo f = fronzo + other.fronzo;
        return Interval(f);
    }
    Interval operator*(const Fraction& fraction) const {
        return *this * Interval(fraction);
    }
    Interval operator*(const int i) const {
        return *this * Interval(i);
    }

    Interval operator/(const Interval& other) const {
        return *this * other.reciprocal();
    }
    Interval operator/(const Fraction& fraction) const {
        return *this / Interval(fraction);
    }
    Interval operator/(const int i) const {
        return *this / Interval(i);
    }

    Interval operator^(const int power) const {
        const Fronzo f = power * fronzo;
        return Interval(f);
    }

    bool operator==(const Interval& interval) const {
        return fronzo == interval.fronzo;
    }

    std::optional<Fraction> asFraction() const {
        Monzo m{1, 1};
        for (size_t i = 0; i < fronzo.primePowers.size(); ++i) {
            if (!fronzo.primePowers[i].isInteger())
                return std::nullopt;
            m.primePowers[i] = fronzo.primePowers[i].getNumeratorAndDenominator().first;
        }
        return Fraction(m);
    }

    bool isOctavial() const {
        if (auto f = asFraction())
            return f->isOctavial();
        return false;
    }

    bool isContractive() const {
        return toDouble() < 1;
    }

    bool isExpansive() const {
        return toDouble() > 1;
    }

    std::tuple<int, int, Fraction> getStepsDivisionsFractionToDivide() const {
        std::vector<Fraction> nonZeroFractions;
        for (auto& fu : fronzo.primePowers)
            if (fu.fraction)
                nonZeroFractions.push_back(fu.fraction.value());

        auto [intRatios, scalingFactor] = Fraction::getIntRatiosAndScalingFactor(nonZeroFractions);

        std::array<int, Primes::PrimeCount> fuIntRatiosAsIntegralFractions{};
        size_t j = 0;
        for (size_t i = 0; i < fronzo.primePowers.size(); ++i) {
            if (fronzo.primePowers[i].fraction) {
                fuIntRatiosAsIntegralFractions[i] = fronzo.primePowers[i].positive ? intRatios[j] : -intRatios[j];
                j++;
            } else
                fuIntRatiosAsIntegralFractions[i] = 0;
        }

        Fraction toDivide{Monzo(fuIntRatiosAsIntegralFractions)};
        auto [divisions, steps] = scalingFactor.getNumeratorAndDenominator();
        return std::make_tuple(steps, divisions, toDivide);
    }

    double getSizeInSemitones() const {
        double ratioLog = std::log2(toDouble());
        return ratioLog * 12;
    }

    int getSizeInCents() const {
        return static_cast<int>(std::round(getSizeInSemitones() * 100));
    }

    explicit operator double() const {
        if (!cachedValue)
            cachedValue = static_cast<double>(fronzo);
        return cachedValue.value();
    }

    double toDouble() const {
        return static_cast<double>(*this);
    }

    juce::String toString(int preferredDivisions) const {
        if (auto f = asFraction())
            return f->toString();
        auto [steps, divisions, toDivide] = getStepsDivisionsFractionToDivide();
        if (preferredDivisions % divisions == 0) {
            int factor = preferredDivisions / divisions;
            steps *= factor;
            divisions *= factor;
        }
        juce::String s = juce::String(steps) + juce::String("\\") + juce::String(divisions);
        if (!(toDivide == Fraction{2, 1}))
            s += juce::String("ed") + toDivide.toString();
        return s;
    }

    juce::String toString() const {
        return toString(1);
    }

    std::optional<juce::String> getName() const {
        if (auto f = asFraction())
            return f->getName();
        return std::nullopt;
    }
};
