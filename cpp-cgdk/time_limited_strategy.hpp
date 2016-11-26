#pragma once

#include "strategy.hpp"

#include <memory>

namespace strategy {

class TimeLimitedStrategy : public IStrategy {
public:
    TimeLimitedStrategy(std::unique_ptr<Strategy> base) : base_(std::move(base)), sum_time_(0) {}

    void apply(Context& context);

private:
    std::unique_ptr<Strategy> base_;
    Duration sum_time_;

    Duration get_iteration_time_limit(const Context& context) const;
    Duration get_full_time_limit(int tick, bool is_master) const;
    static int get_base_time_limit_per_tick(bool is_master);
};


}
