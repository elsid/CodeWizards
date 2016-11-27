#pragma once

#include "mode.hpp"

namespace strategy {

class BattleMode : public Mode {
public:
    Result apply(const Context& context) override final;

    const std::vector<std::pair<Point, double>>& points() const {
        return points_;
    }

private:
    Target target_;
    std::pair<bool, Point> destination_;
    std::vector<std::pair<Point, double>> points_;

    void update_target(const Context& context);
};

}
