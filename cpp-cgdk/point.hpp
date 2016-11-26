#pragma once

#define PI 3.14159265358979323846
#define _USE_MATH_DEFINES

#include <cmath>

namespace strategy {

class Point {
public:
    Point(double x = 0, double y = 0) : x_(x), y_(y) {}

    double x() const { return x_; }
    double y() const { return y_; }

    double distance(const Point& other) const {
        return std::hypot(other.x_ - x_, other.y_ - y_);
    }

    double square() const {
        return x_ * x_ + y_ * y_;
    }

    double dot(const Point& other) const {
        return x_ * other.x_ + y_ * other.y_;
    }

    Point rotated(double angle) const {
        const double cos = std::cos(angle);
        const double sin = std::sin(angle);
        return Point(x_ * cos - y_ * sin, y_ * cos + x_ * sin);
    }

    double norm() const {
        return hypot(x_, y_);
    }

    double cos(const Point& other) const {
        return dot(other) / (norm() * other.norm());
    }

private:
    double x_;
    double y_;
};

inline bool operator ==(const Point& lhs, const Point& rhs) {
    return lhs.x() == rhs.x() && lhs.y() == rhs.y();
}

inline bool operator !=(const Point& lhs, const Point& rhs) {
    return !(lhs == rhs);
}

inline Point operator *(const Point& lhs, const double rhs) {
    return Point(lhs.x() * rhs, lhs.y() * rhs);
}

inline Point operator /(const Point& lhs, const double rhs) {
    return Point(lhs.x() / rhs, lhs.y() / rhs);
}

inline Point operator +(const Point& lhs, const Point& rhs) {
    return Point(lhs.x() + rhs.x(), lhs.y() + rhs.y());
}

inline Point operator -(const Point& lhs, const Point& rhs) {
    return Point(lhs.x() - rhs.x(), lhs.y() - rhs.y());
}

inline bool operator <(const Point& lhs, const Point& rhs) {
    return lhs.x() < rhs.x() || (lhs.x() == rhs.x() && lhs.y() < rhs.y());
}

}
