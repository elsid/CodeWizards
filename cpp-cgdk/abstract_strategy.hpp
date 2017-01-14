#pragma once

#include "context.hpp"

namespace strategy {

class AbstractStrategy {
public:
    virtual ~AbstractStrategy() = default;
    virtual void apply(Context& context) = 0;
};

} // namespace strategy
