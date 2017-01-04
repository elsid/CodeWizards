#ifndef STRATEGY_OPTIMAL_PATH_HPP
#define STRATEGY_OPTIMAL_PATH_HPP

#include "circle.hpp"
#include "point.hpp"
#include "context.hpp"

#include <vector>

namespace strategy {

using Path = std::vector<Point>;

class GetOptimalPath {
public:
    Path operator ()(const Context& context, const Point& target) const;

    GetOptimalPath& step_size(int value);
    GetOptimalPath& max_ticks(Tick value);
    GetOptimalPath& max_iterations(std::size_t value);

private:
    int step_size_ = 1;
    Tick max_ticks_ = std::numeric_limits<Tick>::max();
    std::size_t max_iterations_ = std::numeric_limits<std::size_t>::max();
};

bool has_intersection_with_barriers(const Circle& barrier, const std::vector<Circle>& barriers);
bool has_intersection_with_barriers(const Circle& barrier, const Point& final_position,
                                    const std::vector<Circle>& barriers);
bool has_intersection_with_barriers(const Circle& barrier, const Point& final_position,
                                    const std::vector<std::pair<Circle, Point>>& barriers);
Circle make_circle(const model::CircularUnit* unit);

}

#endif
