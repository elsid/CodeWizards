#pragma once

#include "point.hpp"
#include "context.hpp"

#include <vector>

namespace strategy {

using Path = std::vector<Point>;

Path get_optimal_path(const Context& context, const Point& target, double step_size);

}
