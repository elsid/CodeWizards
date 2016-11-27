#pragma once

#include "circle.hpp"
#include "point.hpp"
#include "context.hpp"

#include <vector>

namespace strategy {

using Path = std::vector<Point>;

Path get_optimal_path(const Context& context, const Point& target, int step_size,
                      int max_ticks = std::numeric_limits<int>::max(),
                      Duration max_duration = Duration::max());
bool has_intersection_with_barriers(const Circle& barrier, const Point& final_position,
                                    const std::vector<Circle>& barriers);
Circle make_circle(const model::CircularUnit* unit);

}
