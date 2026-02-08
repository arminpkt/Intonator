//
// Created by Vos de Mens on 07/01/2026.
//

#pragma once

#include <iostream>

namespace primes {
    constexpr auto Primes = std::array{
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
        31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
        73, 79, 83, 89, 97
    };

    constexpr std::size_t PrimeCount = Primes.size();

    const auto PrimeLogs = [] {
        std::array<double, PrimeCount> result{};

        for (size_t i = 0; i < PrimeCount; ++i)
            result[i] = std::log(Primes[i]);

        return result;
    }();
}

class Monzo {
private:
    explicit Monzo(const std::array<int, primes::PrimeCount>& pP) : primePowers(pP) {}

    static std::array<int, primes::PrimeCount> extractPrimePowers(int numerator, int denominator) {
        if (numerator <= 0 || denominator <= 0)
            throw std::invalid_argument("numerator and denominator need to be positive");

        std::array<int, primes::PrimeCount> result{};
        size_t numPrimeIndex = 0;
        while (numerator > 1) {
            if (numPrimeIndex == primes::PrimeCount)
                throw std::invalid_argument("Sorry, we're doing 97-limit JI for now.");
            if (numerator % primes::Primes[numPrimeIndex] == 0) {
                result[numPrimeIndex]++;
                numerator /= primes::Primes[numPrimeIndex];
            }
            else
                numPrimeIndex++;
        }

        size_t denomPrimeIndex = 0;
        while (denominator > 1) {
            if (denomPrimeIndex == primes::PrimeCount)
                throw std::invalid_argument("Sorry, we're doing 97-limit JI for now.");
            if (denominator % primes::Primes[denomPrimeIndex] == 0) {
                result[denomPrimeIndex]--;
                denominator /= primes::Primes[denomPrimeIndex];
            }
            else
                denomPrimeIndex++;
        }

        return result;
    }

public:
    std::array<int, primes::PrimeCount> primePowers;

    Monzo(const int numerator, const int denominator) : primePowers(extractPrimePowers(numerator, denominator)) {}

    [[nodiscard]] std::pair<int, int> getNumeratorAndDenominator() const {
        int num = 1;
        int denom = 1;

        for (size_t i = 0; i < primes::PrimeCount; ++i)
        {
            int power = primePowers[i];
            while (power > 0) {
                num *= primes::Primes[i];
                power--;
            }
            while (power < 0) {
                denom *= primes::Primes[i];
                power++;
            }
        }
        return std::make_pair(num, denom);
    }

    Monzo operator-() const {
        std::array<int, primes::PrimeCount> negatedPowers{};
        for (size_t i = 0; i < primePowers.size(); ++i)
            negatedPowers[i] = -primePowers[i];
        return Monzo(negatedPowers);
    }

    Monzo operator+(const Monzo& other) const {
        std::array<int, primes::PrimeCount> added_powers{};
        for (size_t i = 0; i < primes::PrimeCount; ++i)
            added_powers[i] = primePowers[i] + other.primePowers[i];
        return Monzo(added_powers);
    }

    Monzo operator-(const Monzo& other) const {
        return *this + -other;
    }

    explicit operator double() const {
        double logv = 0.0;
        for (size_t i = 0; i < primes::PrimeCount; ++i)
            if (primePowers[i] != 0)
                logv += primePowers[i] * primes::PrimeLogs[i];
        return std::exp(logv);
    }

    friend Monzo operator*(const int factor, const Monzo& m) {
        std::array<int, primes::PrimeCount> multiplied_powers{};
        for (size_t i = 0; i < primes::PrimeCount; ++i)
            multiplied_powers[i] = factor * m.primePowers[i];
        return Monzo(multiplied_powers);
    }

    friend std::ostream& operator<<(std::ostream& out, const Monzo& m) {
        size_t lastNonZero;
        for (size_t i = primes::PrimeCount - 1; ; --i)
            if (m.primePowers[i] != 0) {
                lastNonZero = i;
                break;
            }

        out << "[";
        for (size_t i = 0; i < lastNonZero; ++i) {
            out << m.primePowers[i];
            if (i < lastNonZero - 1)
                out << ", ";
        }
        out << "⟩";

        return out;
    }
};