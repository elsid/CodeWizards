#pragma once

#include "context.hpp"

namespace strategy {

class BaseStrategy;

class Stats {
public:
    struct UnitsStats {
        std::size_t hits_count = 0;
        std::size_t target_hits_count = 0;
        std::size_t target_casts_count = 0;
        std::size_t target_ticks_count = 0;

        double target_hits_per_target_casts_ = 0;
        double hits_per_casts_ = 0;
        double target_casts_per_target_ticks_ = 0;
    };

    Stats(const BaseStrategy& base_strategy);

#ifdef ELSID_STRATEGY_LOCAL
    ~Stats();
#endif

    void calculate(const Context& context);
    std::ostream& log(std::ostream& stream) const;

private:
    const BaseStrategy& base_strategy_;

    std::size_t casts_count_ = 0;
    std::size_t hits_count_ = 0;
    std::size_t units_hits_count_ = 0;
    std::size_t target_ticks_count_ = 0;

    double hits_per_casts_ = 0;
    double target_casts_per_target_ticks_ = 0;
    double target_casts_per_ticks_ = 0;

    UnitsStats buildings_;
    UnitsStats minions_;
    UnitsStats trees_;
    UnitsStats wizards_;

    int last_tick_ = 0;
    int last_life_ = 0;
    int prev_my_life_ = -1;
    int sum_damage_to_me_ = 0;
    std::size_t deaths_count_ = 0;
    int prev_tick_ = 0;
    int last_damage_ = 0;

    std::ostream& log_hits_per_casts(std::ostream& stream) const;
    std::ostream& log_target_casts_per_target_ticks(std::ostream& stream) const;
    std::ostream& log_target_casts_per_ticks(std::ostream& stream) const;
    std::ostream& log_damage_to_me(std::ostream& stream) const;
    std::ostream& log_deaths_count(std::ostream& stream) const;

    void fill(UnitsStats& stats) const;
};

}
