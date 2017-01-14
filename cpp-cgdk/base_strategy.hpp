#pragma once

#include "retreat_mode.hpp"
#include "world_graph.hpp"
#include "optimal_movement.hpp"
#include "stats.hpp"
#include "move_to_position.hpp"
#include "abstract_strategy.hpp"

namespace strategy {

class BaseStrategy : public AbstractStrategy {
public:
    BaseStrategy(const Context& context);

    const WorldGraph& graph() const {
        return graph_;
    }

    const Path& path() const {
        return move_to_position_.path();
    }

    const MovementsStates& states() const {
        return move_to_position_.states();
    }

    const Point& destination() const {
        return destination_;
    }

    const Target& target() const {
        return target_;
    }

    const BattleMode& battle_mode() const {
        return *battle_mode_;
    }

    const MoveMode& move_mode() const {
        return *move_mode_;
    }

    const Mode& mode() const {
        return *mode_;
    }

    const std::map<double, TickState>& ticks_states() const {
        return move_to_position_.ticks_states();
    }

    const std::vector<StepState>& steps_states() const {
        return move_to_position_.steps_states();
    }

    void apply(Context& context) override final;

private:
    const WorldGraph graph_;
    const std::shared_ptr<BattleMode> battle_mode_;
    const std::shared_ptr<MoveMode> move_mode_;
    const std::shared_ptr<RetreatMode> retreat_mode_;
    std::shared_ptr<Mode> mode_;
    Target target_;
    Point destination_;
    MoveToPosition move_to_position_;
    model::SkillType skill_from_message_ = model::_SKILL_UNKNOWN_;
    Tick mode_ticks_ = 0;
    std::map<double, TickState> ticks_states_;
    std::vector<StepState> steps_states_;
    Stats stats_;

    void handle_messages(const Context& context);
    void select_mode(const Context& context);
    void apply_mode(const Context& context);
    void update_movements(const Context& context);
    void apply_move(Context& context) const;
    void apply_action(Context& context) const;
    bool apply_action(Context& context, const Target& target) const;
    void learn_skills(Context& Context) const;

    void use_move_mode();
    void use_battle_mode();
    void use_retreat_mode();
    void use_mode(const std::shared_ptr<Mode>& mode);
};

}
