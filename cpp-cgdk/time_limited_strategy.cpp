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

    static constexpr double max_time_for_iteration =
#ifdef ELSID_INCREASED_TIMEOUT
        100000
#else
        100
#endif
    ;

    const auto tick = context.world().getTickIndex();
    const auto ticks_count = context.world().getTickCount();
    const auto is_master = context.self().isMaster();
    const auto max_time_for_current_iteration = std::min(Ms(get_full_time_limit(tick, is_master) - sum_time_),
                                                         Ms(max_time_for_iteration));
    const auto max_mean_time_for_future_iterations = (get_full_time_limit(ticks_count, is_master) - sum_time_)
            / double(ticks_count - tick);
    const auto recommended = Ms((Ms(max_time_for_current_iteration).count() * tick
                                 + Ms(max_mean_time_for_future_iterations).count() * (ticks_count - tick)) / ticks_count);
    return std::min(max_time_for_current_iteration, recommended) - Ms(1);
}

Duration TimeLimitedStrategy::get_full_time_limit(int tick, bool is_master) const {
    using Ms = std::chrono::duration<double, std::milli>;
    return Ms(get_base_time_limit_per_tick(is_master) * tick + 10000.0);
}

int TimeLimitedStrategy::get_base_time_limit_per_tick(bool is_master) {
#ifdef ELSID_INCREASED_TIMEOUT
    return is_master ? 20000 : 10000;
#else
    return is_master ? 20 : 10;
#endif
}

}
