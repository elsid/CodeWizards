#include "strategy.hpp"
#include "optimal_destination.hpp"

namespace strategy {

Strategy::Strategy(const Context& context)
    : graph_(context.game()) {
}

void Strategy::apply(Context &context) {
    update_cache(context);
}

void Strategy::update_cache(const Context& context) {
    update_specific_cache<model::Bonus>(context);
    update_specific_cache<model::Building>(context);
    update_specific_cache<model::Minion>(context);
    update_specific_cache<model::Projectile>(context);
    update_specific_cache<model::Tree>(context);
    update_specific_cache<model::Wizard>(context);
}

}
