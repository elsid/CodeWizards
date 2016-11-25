#pragma once

#include "point.hpp"

#include <cmath>

namespace strategy {

class Line {
public:
    Line(const Point& begin, const Point& end) : begin_(begin), end_(end) {}

    const Point& begin() const { return begin_; }
    const Point& end() const { return end_; }

    Point nearest(const Point& point) const {
        const auto to_end = end_ - begin_;
        const auto to_end_squared_norm = to_end.square();
        if (to_end_squared_norm == 0) {
            return begin_;
        }
        const auto to_point = point - begin_;
        return begin_ + to_end * to_point.dot(to_end) / to_end_squared_norm;
    }

    double length() const {
        return begin_.distance(end_);
    }

    double signed_distance(const Point& point) const {
        return (
            (begin_.y() - end_.y()) * point.x()
            + (end_.x() - begin_.x()) * point.y()
            + (begin_.x() * end_.y() - end_.x() * begin_.y())
        ) / length();
    }

    double distance(const Point& point) const {
        return std::abs(signed_distance(point));
    }

    bool has_point(const Point& point, double max_error = 1e-3) const {
        const auto to_end = end_ - point;
        if (to_end.square() == 0) {
            return true;
        }
        const auto to_begin = begin_ - point;
        if (to_begin.square() == 0) {
            return true;
        }
        return std::abs(1 + to_begin.cos(to_end)) <= max_error;
    }

private:
    const Point begin_;
    const Point end_;
};

}
