#pragma once

#define _USE_MATH_DEFINES

#include <cmath>

namespace strategy {
namespace math {

inline constexpr double square(double value) {
    return value * value;
}

inline double cos(double angle) {
    if (angle <= -M_PI + M_PI_4) {
        return std::cos(angle);
    } else if (angle <= -M_PI_2) {
        return -std::sqrt(1.0 - square(std::sin(angle)));
    } else if (angle <= -M_PI_4) {
        return std::sqrt(1.0 - square(std::sin(angle)));
    } else if (angle <= M_PI_4) {
        return std::cos(angle);
    } else if (angle <= M_PI_2) {
        return std::sqrt(1.0 - square(std::sin(angle)));
    } else if (angle <= M_PI - M_PI_4) {
        return -std::sqrt(1.0 - square(std::sin(angle)));
    } else {
        return std::cos(angle);
    }
}

inline double sin(double angle) {
    if (angle <= -M_PI_2) {
        return -std::sqrt(1.0 - square(std::cos(angle)));
    } else if (angle <= M_PI_2) {
        return std::sin(angle);
    } else {
        return std::sqrt(1.0 - square(std::cos(angle)));
    }
}

inline double hypot(double x, double y) {
    return std::sqrt(square(x) + square(y));
}

}
}
