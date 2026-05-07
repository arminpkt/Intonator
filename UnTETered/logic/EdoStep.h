//
// Created by Vos de Mens on 06/05/2026.
//

#pragma once
#include <numeric>

struct EdoStep {
    int n;
    int base;

    EdoStep(int _n, int _base) {
        if (_base == 0)
            throw std::invalid_argument("Base must be non-zero");

        if (_base < 0) {
            _n = -_n;
            _base = -_base;
        }

        n = _n;
        base = _base;
    }

    EdoStep operator+(const EdoStep& other) const {
        int lcm = std::lcm(base, other.base);
        int nThisScaled = n * lcm / base;
        int nOtherScaled = other.n * lcm / other.base;

        return EdoStep(nThisScaled + nOtherScaled, lcm);
    }

    EdoStep operator-() const {
        return EdoStep(-n, base);
    }

    EdoStep operator-(const EdoStep& other) const {
        return *this + (-other);
    }
};
