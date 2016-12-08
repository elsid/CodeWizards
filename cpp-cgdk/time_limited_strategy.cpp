#include "time_limited_strategy.hpp"

#include <ratio>
#include <iostream>

namespace strategy {

struct AddSumTime {
    const Profiler& profiler;
    Duration& sum_time;

    ~AddSumTime() {
        sum_time += profiler.duration();
    }
};

void TimeLimitedStrategy::apply(Context& context) {
    AddSumTime add_sum_time {context.profiler(), sum_time_};
    context.time_limit(get_iteration_time_limit(context));
    base_->apply(context);
}

Duration TimeLimitedStrategy::get_iteration_time_limit(const Context& context) const {
    using Ms = std::chrono::duration<double, std::milli>;
    const auto tick = context.world().getTickIndex();
    const auto ticks_count = context.world().getTickCount();
    const auto is_master = context.self().isMaster();
    const auto max_time_for_current_iteration = Ms(Ms(get_full_time_limit(tick, is_master) - sum_time_).count() * 0.9);
    const auto max_mean_time_for_future_iterations = (get_full_time_limit(ticks_count, is_master) - sum_time_)
            / double(ticks_count - tick);
    const auto recommended = Ms((Ms(max_time_for_current_iteration).count() * tick
                                 + Ms(max_mean_time_for_future_iterations).count() * (ticks_count - tick)) / ticks_count);
    return Ms(std::min(Ms(max_time_for_current_iteration).count(), Ms(recommended).count()) * 0.9);
}

Duration TimeLimitedStrategy::get_full_time_limit(int tick, bool is_master) const {
    using Ms = std::chrono::duration<double, std::milli>;
    return Ms(get_base_time_limit_per_tick(is_master) * tick + 10000.0);
}

int TimeLimitedStrategy::get_base_time_limit_per_tick(bool is_master) {
    return is_master ? 20 : 10;
}

}
