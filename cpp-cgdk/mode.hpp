#pragma once

#include "context.hpp"

namespace strategy {

class Mode {
public:
    class Result {
    public:
        Result() = default;

        Result(const Target& target, const Point& destination)
            : active_(true), target_(target), destination_(destination) {}

        bool active() const {
            return active_;
        }

        Target target() const {
            return target_;
        }

        Point destination() const {
            return destination_;
        }

    private:
        bool active_ = false;
        Target target_;
        Point destination_;
    };

    virtual ~Mode() = default;
    virtual Result apply(const Context& context) = 0;
    virtual void reset() = 0;
};

}
