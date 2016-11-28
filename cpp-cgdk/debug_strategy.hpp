#pragma once

#include "optimal_position.hpp"
#include "base_strategy.hpp"

#include "russian-ai-cup-visual/Debug.h"

#ifdef STRATEGY_DEBUG

#include "debug/output.hpp"

#include <iostream>

#endif

namespace strategy {

std::int32_t get_color(double red, double green, double blue);
std::int32_t get_color(double heat);

class DebugStrategy : public Strategy {
public:
    DebugStrategy(std::unique_ptr<BaseStrategy> base) : base_(std::move(base)) {}

    void apply(Context& context) override final;

private:
    std::unique_ptr<BaseStrategy> base_;
    Debug debug_;

    void visualize(const Context& context);
    void visualize_graph(const Context& context);
    void visualize_graph_path(const Context& context);
    void visualize_positions_penalties(const Context& context);
    void visualize_path(const Context& context);
    void visualize_destination(const Context& context);
    void visualize_target(const Context& context);
    void visualize_self(const Context& context);

    template <class T>
    void visualize_positions_penalties(const Context& context, const T* target) {
        const double max_distance = 1.3 * context.self().getVisionRange();
        const GetPositionPenalty<T> get_position_penalty(context, target, max_distance);

        const auto self_position = get_position(context.self());
        std::vector<std::pair<Point, double>> penalties;
        penalties.reserve(max_distance * max_distance * 4);

        const int step = 2 * context.self().getRadius();
        const int count = std::round(max_distance / step);
        const auto origin = self_position.to_int() + self_position.to_int() % step;

        for (int x = -count; x < count; ++x) {
            for (int y = -count; y < count; ++y) {
                const auto position = (origin + PointInt(x, y) * step).to_double();
                if (position.distance(self_position) <= max_distance) {
                    penalties.emplace_back(position, get_position_penalty(position));
                }
            }
        }

        const auto min_max = std::minmax_element(penalties.begin(), penalties.end(),
            [] (const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; });
        const double min = min_max.first->second;
        const double max = min_max.second->second;
        const double norm = max_distance != min ? std::abs(max - min) : 1.0;

        for (const auto& v : penalties) {
            Point position;
            double penalty;
            std::tie(position, penalty) = v;
            const double normalized = (penalty - min) / norm;
            const auto color = get_color(normalized);
            debug_.fillCircle(position.x(), position.y(), 5, color);
        }
    }
};

}
