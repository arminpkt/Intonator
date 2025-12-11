#pragma once

#include <iostream>
#include <stdexcept>
#include <numeric>

class Fraction {
public:
    u_long num;
    u_long den;

    Fraction(int numerator, int denominator) : Fraction(
        static_cast<u_long>(numerator),
        static_cast<u_long>(denominator)
    ) {
        if (numerator < 0 || denominator < 0)
            throw std::invalid_argument("Negative arguments indicate integer overflow.");
    }

    Fraction(u_long numerator, u_long denominator) {
        if (denominator == 0)
            throw std::invalid_argument("Denominator cannot be zero.");

        num = numerator;
        den = denominator;

        simplify();
    }

    void simplify() {
        const auto g = std::__gcd<u_long>(num, den);
        num /= g;
        den /= g;

        if (den < 0) {
            num = -num;
            den = -den;
        }
    }

    [[nodiscard]] float toFloat() const {
        return static_cast<float>(num) / static_cast<float>(den);
    }

    Fraction operator*(const Fraction& other) const {
        return {num * other.num, den * other.den};
    }

    Fraction operator/(const Fraction& other) const {
        if (other.num == 0)
            throw std::domain_error("Cannot divide by zero.");
        return {num * other.den, den * other.num};
    }

    friend std::ostream& operator<<(std::ostream& os, const Fraction& f) {
        return os << f.num << "/" << f.den;
    }

    Fraction operator/(const int i) const {
        if (i == 0)
            throw std::invalid_argument("Cannot divide by zero.");
        return {num, den * static_cast<u_long>(i)};
    }

    friend Fraction operator/(const int i, const Fraction& f) {
        if (f.num == 0)
            throw std::invalid_argument("Cannot divide by zero.");
        return {static_cast<u_long>(i) * f.den, f.num};
    }

    Fraction operator*(const int i) const {
        return {num * static_cast<u_long>(i), den};
    }

    friend Fraction operator*(const int i, const Fraction& f) {
        return {f.num * static_cast<u_long>(i), f.den};
    }

    Fraction& operator*=(const Fraction& f) {
        num *= f.num;
        den *= f.den;
        simplify();
        return *this;
    }

    Fraction& operator*=(const int i) {
        num *= static_cast<u_long>(i);
        simplify();
        return *this;
    }

    Fraction& operator/=(const Fraction& f) {
        num *= f.den;
        den *= f.num;
        simplify();
        return *this;
    }

    Fraction& operator/=(const int i) {
        den *= static_cast<u_long>(i);
        simplify();
        return *this;
    }
};