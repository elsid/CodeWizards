#include "MyStrategy.h"

void MyStrategy::move(const model::Wizard& self, const model::World& world, const model::Game& game, model::Move& move) {
    strategy::Context context {self, world, game, move};
    strategy_.apply(context);
}
