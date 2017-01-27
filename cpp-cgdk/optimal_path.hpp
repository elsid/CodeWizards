#pragma once

#include "circle.hpp"
#include "point.hpp"
#include "context.hpp"

#include <map>
#include <vector>

namespace strategy {

class StepState {
public:
    StepState(double priority = 0, double cost = 0, double tick = 0, PointInt position = PointInt())
        : priority_(priority), cost_(cost), tick_(tick), position_(position) {}

    double priority() const {
        return priority_;
    }

    double cost() const {
        return cost_;
    }

    double tick() const {
        return tick_;
    }

    const PointInt& position() const {
        return position_;
    }

private:
    double priority_;
    double cost_;
    double tick_;
    PointInt position_;
};

using Path = std::vector<Point>;

using DynamicBarriers = std::vector<std::pair<Circle, Point>>;

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

    GetOptimalPath& step_size(int value);
    GetOptimalPath& max_ticks(Tick value);
    GetOptimalPath& max_iterations(std::size_t value);
    GetOptimalPath& ticks_states(std::map<double, TickState>* value);
    GetOptimalPath& steps_states(std::vector<StepState>* value);

private:
    int step_size_ = 1;
    Tick max_ticks_ = std::numeric_limits<Tick>::max();
    std::size_t max_iterations_ = std::numeric_limits<std::size_t>::max();
    std::map<double, TickState>* ticks_states_ = nullptr;
    std::vector<StepState>* steps_states_ = nullptr;
};

bool has_intersection_with_borders(const Circle& circle, double map_size);
bool has_intersection_with_barriers(const Circle& barrier, const std::vector<Circle>& barriers);
bool has_intersection_with_barriers(const Circle& barrier, const Point& final_position,
                                    const std::vector<Circle>& barriers);
bool has_intersection_with_barriers(const Circle& barrier, const Point& final_position,
                                    const std::vector<std::pair<Circle, Point>>& barriers);
Circle make_circle(const model::CircularUnit* unit);

}
