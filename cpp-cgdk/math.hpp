#pragma once

#define _USE_MATH_DEFINES

#include <cmath>

namespace strategy {

inline constexpr double normalize_angle(double value) {
    if (value > M_PI) {
        return value - std::round(value * 0.5 * M_1_PI) * 2.0 * M_PI;
    }
    if (value < -M_PI) {
        return value + std::round(std::abs(value) * 0.5 * M_1_PI) * 2.0 * M_PI;
    }
    return value;
}

namespace math {

inline constexpr double square(double value) {
    return value * value;
}

inline double cos(double angle) {
    angle = normalize_angle(angle);
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
    angle = normalize_angle(angle);
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
