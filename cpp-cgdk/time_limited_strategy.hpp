#pragma once

#include "abstract_strategy.hpp"

#ifdef ELSID_STRATEGY_LOCAL
#include <iostream>
#endif

namespace strategy {

class TimeLimitedStrategy : public AbstractStrategy {
public:
    TimeLimitedStrategy(std::unique_ptr<AbstractStrategy> base) : base_(std::move(base)), sum_time_(0) {}

#ifdef ELSID_STRATEGY_LOCAL
    ~TimeLimitedStrategy() {
        using Ms = std::chrono::duration<double, std::milli>;
        std::cout << "sum_time " << Ms(sum_time_).count() << '\n';
    }
#endif

    void apply(Context& context) override final;

private:
    std::unique_ptr<AbstractStrategy> base_;
    Duration sum_time_;

    Duration get_iteration_time_limit(const Context& context) const;
    Duration get_full_time_limit(int tick, bool is_master) const;
    static int get_base_time_limit_per_tick(bool is_master);
};

}
