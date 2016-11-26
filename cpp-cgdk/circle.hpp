#pragma once

#include "point.hpp"
#include "line.hpp"

#include <cmath>

namespace strategy {

class Circle {
public:
    Circle(const Point& position = Point(), double radius = 0) : position_(position), radius_(radius) {}

    const Point& position() const { return position_; }
    double radius() const { return radius_; }

    bool has_intersection_with_circle(const Circle& circle, double max_error = 1e-3) const {
        return position_.distance(circle.position_) <= radius_ + circle.radius_ + max_error;
    }

    bool has_intersection_with_line(const Line& line, double max_error = 1e-3) const {
        const auto nearest = line.nearest(position_);
        return position_.distance(nearest) - radius_ <= max_error && line.has_point(nearest, max_error);
    }

    bool has_intersection_with_moving_circle(const Circle& circle, const Point& final_position, double max_error = 1e-3) const {
        if (has_intersection_with_circle(circle)) {
            return true;
        }
        if (final_position == circle.position_) {
            return false;
        }
        if (has_intersection_with_circle(Circle(final_position, circle.radius_))) {
            return true;
        }
        return Circle(position_, radius_ + circle.radius_)
                .has_intersection_with_line(Line(circle.position_, final_position), max_error);
    }

private:
    Point position_;
    double radius_;
};

}
