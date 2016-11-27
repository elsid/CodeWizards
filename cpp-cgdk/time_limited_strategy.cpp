#include "time_limited_strategy.hpp"

#include <iostream>

#include <ratio>

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
    Context time_limited_context(context.self(), context.world(), context.game(), context.move(),
                                 context.cache(), context.profiler(), get_iteration_time_limit(context));
    base_->apply(time_limited_context);
}

Duration TimeLimitedStrategy::get_iteration_time_limit(const Context& context) const {
    const auto tick = context.world().getTickIndex();
    const auto ticks_count = context.world().getTickCount();
    const auto is_master = context.self().isMaster();
    const auto max_time_for_current_iteration = get_full_time_limit(tick, is_master) - sum_time_;
    const auto max_mean_time_for_future_iterations = (get_full_time_limit(ticks_count, is_master) - sum_time_)
            / double(ticks_count - tick);
    return std::min(max_time_for_current_iteration, max_mean_time_for_future_iterations);
}

Duration TimeLimitedStrategy::get_full_time_limit(int tick, bool is_master) const {
    using Ms = std::chrono::duration<double, std::milli>;
    return Ms((get_base_time_limit_per_tick(is_master) * tick + 10000.0) / 1000.0);
}

int TimeLimitedStrategy::get_base_time_limit_per_tick(bool is_master) {
    return is_master ? 20 : 10;
}

}
