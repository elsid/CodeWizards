#include "strategy.hpp"
#include "optimal_destination.hpp"

namespace strategy {

void Strategy::apply(Context &context) {
    if (!initialized_) {
        initialize(context);
    }
}

void Strategy::initialize(const Context& context) {
    graph_ = std::make_unique<WorldGraph>(context.game);
}

}
