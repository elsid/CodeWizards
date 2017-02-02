#pragma once

#include "point.hpp"
#include "line.hpp"

namespace strategy {

class Circle {
public:
    Circle(const Point& position = Point(), double radius = 0) : position_(position), radius_(radius) {}

    const Point& position() const { return position_; }
    double radius() const { return radius_; }

    bool has_intersection(const Circle& circle, double max_error = 1e-8) const;
    bool has_intersection(const Line& line, double max_error = 1e-8) const;
    bool has_intersection(const Circle& other, const Point& final_position, double max_error = 1e-8) const;
    bool has_intersection(const Point& this_final_position, const Circle& other,
                          const Point& other_final_position, double max_error = 1e-8) const;

private:
    Point position_;
    double radius_;
};

}
