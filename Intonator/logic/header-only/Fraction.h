#pragma once

#include <iostream>
#include <stdexcept>
#include <numeric>

class Fraction {
public:
    int num;
    int den;

    Fraction(int numerator, int denominator) {
        if (denominator == 0)
            throw std::invalid_argument("Denominator cannot be zero.");

        num = numerator;
        den = denominator;

        simplify();
    }

    void simplify() {
        int g = std::gcd(num, den);
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
        return {num, den * i};
    }

    friend Fraction operator/(const int i, const Fraction& f) {
        if (f.num == 0)
            throw std::invalid_argument("Cannot divide by zero.");
        return {i * f.den, f.num};
    }

    Fraction operator*(const int i) const {
        return {num * i, den};
    }

    friend Fraction operator*(const int i, const Fraction& f) {
        return {f.num * i, f.den};
    }

    Fraction& operator*=(const Fraction& f) {
        num *= f.num;
        den *= f.den;
        simplify();
        return *this;
    }

    Fraction& operator*=(const int i) {
        num *= i;
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
        den *= i;
        simplify();
        return *this;
    }
};