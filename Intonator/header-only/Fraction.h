//
// Created by Vos on 30/09/2025.
//

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

        int g = std::gcd(numerator, denominator);
        num = numerator / g;
        den = denominator / g;

        if (den < 0) {
            num = -num;
            den = -den;
        }
    }

    Fraction operator*(const Fraction& other) const {
        return Fraction(num * other.num, den * other.den);
    }

    Fraction operator/(const Fraction& other) const {
        if (other.num == 0)
            throw std::domain_error("Cannot divide by zero.");
        return Fraction(num * other.den, den * other.num);
    }

    friend std::ostream& operator<<(std::ostream& os, const Fraction& f) {
        return os << f.num << "/" << f.den;
    }

    friend Fraction operator/(const Fraction& f, const int i) {
        if (i == 0)
            throw std::invalid_argument("Cannot divide by zero.");
        return Fraction(f.num, f.den * i);
    }

    friend Fraction operator/(const int i, const Fraction& f) {
        if (f.num == 0)
            throw std::invalid_argument("Cannot divide by zero.");
        return Fraction(i * f.den, f.num);
    }

    friend Fraction operator*(const Fraction& f, const int i) {
        return Fraction(f.num * i, f.den);
    }

    friend Fraction operator*(const int i, const Fraction& f) {
        return Fraction(f.num * i, f.den);
    }
};