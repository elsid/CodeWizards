#ifndef STRATEGY_OPTIMAL_PATH
#define STRATEGY_OPTIMAL_PATH

#include "circle.hpp"
#include "point.hpp"
#include "context.hpp"

#include <vector>

namespace strategy {

using Path = std::vector<Point>;

Path get_optimal_path(const Context& context, const Point& target, int step_size,
                      int max_ticks = std::numeric_limits<int>::max());
bool has_intersection_with_barriers(const Circle& barrier, const Point& final_position,
                                    const std::vector<Circle>& barriers);
bool has_intersection_with_barriers(const Circle& barrier, const Point& final_position,
                                    const std::vector<std::pair<Circle, Point>>& barriers);
Circle make_circle(const model::CircularUnit* unit);

}

#endif
