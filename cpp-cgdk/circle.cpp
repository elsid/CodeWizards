#include "circle.hpp"

namespace strategy {

bool Circle::has_intersection(const Circle& circle, double max_error) const {
    const auto distance = position_.distance(circle.position_);
    const auto min_distance = radius_ + circle.radius_ + max_error;
    return distance < min_distance;
}

bool Circle::has_intersection(const Line& line, double max_error) const {
    const auto nearest = line.nearest(position_);
    double distance;
    if (line.has_point(nearest, max_error)) {
        distance = position_.distance(nearest);
    } else {
        const auto to_begin = position_.distance(line.begin());
        const auto to_end = position_.distance(line.end());
        distance = std::min(to_begin, to_end);
    }
    const auto min_distance = radius_ + max_error;
    return distance < min_distance;
}

bool Circle::has_intersection(const Circle& other, const Point& final_position, double max_error) const {
    if (final_position == other.position()) {
        return has_intersection(other, max_error);
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
        return Circle(this_nearest, radius()).has_intersection(other, other_final_position, max_error);
    } else if (other_has_point) {
        return Circle(other_nearest, other.radius()).has_intersection(*this, this_final_position, max_error);
    } else {
        return has_intersection(other)
                || has_intersection(Circle(other_final_position, other.radius()))
                || Circle(this_final_position, radius()).has_intersection(other)
                || Circle(this_final_position, radius()).has_intersection(Circle(other_final_position, other.radius()));
    }
}

Point Circle::get_intersection(const Line& line) const {
    const auto nearest = line.nearest(position_);
    const auto far_cathetus = position_.distance(nearest);
    if (far_cathetus >= radius_) {
        return nearest;
    }
    const auto near_cathetus = std::sqrt(math::square(radius_) - math::square(far_cathetus));
    const auto path = nearest - line.begin();
    const auto length = path.norm() - near_cathetus;
    const auto end = path.normalized() * length;
    return line.begin() + end;
}

}
