#pragma once

#include <chrono>

namespace strategy {

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;
using Duration = std::chrono::duration<double>;

class Profiler {
public:
    Profiler() : start_(Clock::now()) {}

    Duration duration() const {
        return Clock::now() - start_;
    }

private:
    TimePoint start_;
};

}
