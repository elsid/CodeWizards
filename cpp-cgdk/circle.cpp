#include "circle.hpp"

#include <cmath>
#include <tuple>

#include "golden_section.hpp"

#ifdef ELSID_STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

bool Circle::has_intersection(const Circle& circle, double max_error) const {
    const auto distance = position_.distance(circle.position_);
    const auto min_distance = radius_ + circle.radius_ + max_error;
    return distance < min_distance;
}

bool Circle::has_intersection(const Line& line, double max_error) const {
    const auto nearest = line.nearest(position_);
    return position_.distance(nearest) - radius_ <= max_error && line.has_point(nearest, max_error);
}

bool Circle::has_intersection(const Circle& other, const Point& final_position, double max_error) const {
    if (final_position == other.position()) {
        return has_intersection(other);
    }
    if (has_intersection(Circle(final_position, other.radius()))) {
        return true;
    }
    return Circle(position(), radius() + other.radius())
            .has_intersection(Line(other.position(), final_position), max_error);
}

bool Circle::has_intersection(const Point& this_final_position, const Circle& other,
                                              const Point& other_final_position, double max_error) const {
    if (position() == this_final_position && other.position() == other_final_position) {
        return has_intersection(other, max_error);
    } else if (position() == this_final_position) {
        return has_intersection(other, other_final_position, max_error);
    } else if (other.position() == other_final_position) {
        return other.has_intersection(*this, this_final_position, max_error);
    }
    const Line this_line(position(), this_final_position);
    const Line other_line(other.position(), other_final_position);
    bool is_intersection;
    Point intersection;
    std::tie(is_intersection, intersection) = this_line.intersection(other_line);
    if (!is_intersection) {
        return has_intersection(other, other_final_position, max_error);
    }
    const auto this_nearest = this_line.nearest(intersection);
    const auto other_nearest = other_line.nearest(intersection);
    const auto this_has_point = this_line.has_point(this_nearest);
    const auto other_has_point = other_line.has_point(other_nearest);
    if (this_has_point && other_has_point) {
        return true;
    } else if (this_has_point) {
        return has_intersection(other, other_final_position, max_error);
    } else if (other_has_point) {
        return other.has_intersection(*this, this_final_position, max_error);
    } else {
        return has_intersection(other)
                || has_intersection(Circle(other_final_position, other.radius()))
                || Circle(this_final_position, radius()).has_intersection(other)
                || Circle(this_final_position, radius()).has_intersection(Circle(other_final_position, other.radius()));
    }
}

std::pair<bool, Point> Circle::intersection(const Circle& other, const Point& final_position, double max_error) const {
    const auto direction = final_position - other.position();
    const auto required_distance = radius() + other.radius();
    const auto max_distance = other.position().distance(final_position);
    const auto f = [&] (const auto distance) {
        const auto other_position = other.position() + direction * distance;
        return std::abs(position().distance(other_position) - required_distance) / max_distance;
    };
    const std::size_t iterations = std::ceil(std::log(max_error) / std::log(INVERTED_PHI));
    const auto min_distance = golden_section(f, 0, 1, iterations);
    const auto min_distance_diff = f(min_distance);
    if (min_distance_diff <= max_error) {
        return {true, other.position() + direction * min_distance};
    } else {
        return {false, Point()};
    }
}

}
