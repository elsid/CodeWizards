#include "two_wizards_fight_near_bonus.hpp"

#include <optimal_destination.hpp>
#include <action.hpp>

namespace strategy {
namespace simulation {
namespace scripts {

TwoWizardsFightNearBonus::TwoWizardsFightNearBonus(const Context& context)
        : strategy_(context) {
}

void TwoWizardsFightNearBonus::apply(Context &context) {
    if (context.world().getTickIndex() == 0) {
        strategy_.move_to_node(context, context.self().getFaction() == model::FACTION_ACADEMY ? 48u : 44u);
    }

    if (strategy_.is_at_node()) {
        const auto result = battle_mode_.apply(context);

        if (result.active()) {
            target_ = result.target();
            if (result.destination() != destination_) {
                destination_ = result.destination();
                strategy_.move_to_position(context, result.destination(), result.target());
            }
        }
    }

    if (context.world().getTickIndex() > 1000) {
        const auto actions = get_actions_by_priority_order(context, target_);

        for (const auto action_type : actions) {
            bool need_apply;
            Action action;
            std::tie(need_apply, action) = need_apply_action(context, target_, action_type);

            if (need_apply) {
                strategy_.apply_action(action);
            }
        }
    }

    strategy_.apply(context);
}

} // namespace scripts
} // namespace simulation
} // namespace strategy
