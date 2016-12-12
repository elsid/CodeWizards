#pragma once

#include <cmath>

namespace strategy {

const auto INVERTED_PHI = 2.0 / (1.0 + std::sqrt(5.0));

template <class Function>
double golden_section(const Function& f, double a, double b, std::size_t iterations) {
    double x1;
    double y1;
    double x2;
    double y2;

    bool is1 = false;
    bool is2 = false;

    for (std::size_t i = 0; i < iterations; ++i) {
        if (!is1) {
            x1 = b - (b - a) * INVERTED_PHI;
            y1 = f(x1);
            is1 = true;
        }
        if (!is2) {
            x2 = a + (b - a) * INVERTED_PHI;
            y2 = f(x2);
            is2 = true;
        }
        if (y1 < y2) {
            b = x2;
            x2 = x1;
            y2 = y1;
            is1 = false;
        } else {
            a = x1;
            x1 = x2;
            y1 = y2;
            is2 = false;
        }
    }

    return (a + b) * 0.5;
}

}
