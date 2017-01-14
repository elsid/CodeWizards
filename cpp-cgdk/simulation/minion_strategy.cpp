#include "minion_strategy.hpp"
#include "helpers.hpp"

namespace strategy {
namespace simulation {

void MinionStrategy::move(const model::Minion& self, const model::World& world, const model::Game& game, MinionMove& move) {
    const model::Wizard* nearest_wizard = nullptr;
    double min_distance = std::numeric_limits<double>::max();

    for (const auto& wizard : world.getWizards()) {
        const auto distance = get_position(self).distance(get_position(wizard));

        if (distance <= self.getVisionRange() && min_distance > distance) {
            nearest_wizard = &wizard;
            min_distance = distance;
        }
    }

    if (nearest_wizard) {
        move.speed(std::min(game.getMinionSpeed(), min_distance - get_attack_range(self, game)));
        move.turn(self.getAngleTo(*nearest_wizard));
    }
}

} // namespace simulation
} // namespace strategy
