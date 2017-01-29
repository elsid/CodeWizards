#pragma once

#include "circle.hpp"
#include "point.hpp"
#include "context.hpp"

#include <map>
#include <vector>

namespace strategy {

class StepState {
public:
    StepState(double priority = 0, double cost = 0, double tick = 0, Point position = Point(), Point target = Point())
        : priority_(priority), cost_(cost), tick_(tick), position_(position), target_(target) {}

    double priority() const {
        return priority_;
    }

    double cost() const {
        return cost_;
    }

    double tick() const {
        return tick_;
    }

    const Point& position() const {
        return position_;
    }

    const Point& target() const {
        return target_;
    }

private:
    double priority_;
    double cost_;
    double tick_;
    Point position_;
    Point target_;
};

using Path = std::vector<Point>;

struct DynamicBarrier {
    std::size_t id;
    Circle circle;
    Point target;
};

using DynamicBarriers = std::vector<DynamicBarrier>;

class TickState {
public:
    using Occupier = std::pair<bool, Circle>;

    TickState(DynamicBarriers dynamic_barriers,
              const Occupier& occupier, double max_distance_error)
            : dynamic_barriers_(std::move(dynamic_barriers)),
              occupier_(occupier),
              max_distance_error_(max_distance_error) {}

    double max_distance_error() const {
        return max_distance_error_;
    }

    const Occupier& occupier() const {
        return occupier_;
    }

    const DynamicBarriers& dynamic_barriers() const {
        return dynamic_barriers_;
    }

private:
    DynamicBarriers dynamic_barriers_;
    Occupier occupier_;
    double max_distance_error_;
};

class GetOptimalPath {
public:
    Path operator ()(const Context& context, const Point& target) const;

    GetOptimalPath& step_size(double value);
    GetOptimalPath& max_ticks(Tick value);
    GetOptimalPath& max_iterations(std::size_t value);
    GetOptimalPath& ticks_states(std::map<int, TickState>* value);
    GetOptimalPath& steps_states(std::vector<StepState>* value);

private:
    double step_size_ = 1;
    Tick max_ticks_ = std::numeric_limits<Tick>::max();
    std::size_t max_iterations_ = std::numeric_limits<std::size_t>::max();
    std::map<int, TickState>* ticks_states_ = nullptr;
    std::vector<StepState>* steps_states_ = nullptr;
};

bool has_intersection_with_borders(const Circle& circle, double map_size);
Circle make_circle(const model::CircularUnit* unit);
std::pair<Point, Point> get_tangent_points(const Circle& circle, const Point& source);

}
