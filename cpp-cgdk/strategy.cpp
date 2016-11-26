#include "strategy.hpp"
#include "optimal_destination.hpp"

namespace strategy {

Strategy::Strategy(const Context& context)
    : graph_(context.game()) {
}

void Strategy::apply(Context &context) {
}

}
