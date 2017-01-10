#pragma once

#include "mode.hpp"

namespace strategy {

class BattleMode : public Mode {
public:
    Result apply(const Context& context) override final;
    void reset() override final;

    const std::vector<std::pair<Point, double>>& points() const {
        return points_;
    }

    const char* name() const override final {
        return "battle";
    }

private:
    Target target_;
    std::pair<bool, Point> destination_;
    std::vector<std::pair<Point, double>> points_;

    void update_target(const Context& context);
    bool is_under_fire(const Context& context) const;
    bool will_cast_later(const Context& context) const;

    template <class TargetT>
    Point get_optimal_position(const Context& context, const TargetT* target);

    Point get_optimal_position(const Context& context, const model::Bonus* target);
    Point get_optimal_position(const Context& context);
};

}
