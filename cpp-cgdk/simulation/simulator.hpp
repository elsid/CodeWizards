#pragma once

#include "circle.hpp"
#include "minion_move.hpp"

#include "model/Game.h"
#include "model/World.h"
#include "model/Move.h"

#include <unordered_map>

namespace strategy {
namespace simulation {

class Simulator {
public:
    Simulator(const model::Game& game, model::World& world);

    void next_tick();
    void handle_wizard_move(int id, const model::Move& move);
    void handle_minion_move(int id, const MinionMove& move);

private:
    const model::Game& game_;
    model::World& world_;
    int projectile_id_counter_ = 1;
    std::unordered_map<int, double> projectiles_damage_;

    const model::Projectile& get_projectile(int id) const;
    const model::Wizard& get_wizard(int id) const;
    const model::Minion& get_minion(int id) const;

    template <class Unit>
    const Unit& get_unit(int id) const;

    template <class Unit>
    const std::vector<Unit>& get_units() const;

    template <class Unit>
    static const Unit& get_unit(int id, const std::vector<Unit>& units);

    model::World get_world_at_next_tick(const model::World& world);

    template <class Unit>
    static std::vector<Unit> get_updated_units(const std::vector<Unit>& values);

    static model::Wizard get_updated_unit(const model::Wizard& value);
    static model::Minion get_updated_unit(const model::Minion& value);

    template <class Unit>
    static std::vector<Unit> get_moved_units(const std::vector<Unit>& values);

    static model::Projectile get_moved_unit(const model::Projectile& value);

    template <class Unit>
    static Unit get_moved_unit(const Unit& value);

    static model::Wizard make_moved_unit(const model::Wizard& value, const Point& next_position);
    static model::Minion make_moved_unit(const model::Minion& value, const Point& next_position);

    template <class Unit>
    std::vector<Unit> get_damaged_units(const std::vector<Unit>& values, std::vector<model::Projectile>& projectiles) const;

    template <class Unit>
    Unit get_damaged_unit(const Unit& value, std::vector<model::Projectile>& projectiles) const;

    static model::Wizard make_damaged_unit(const model::Wizard& value, int life);
    static model::Minion make_damaged_unit(const model::Minion& value, int life);

    template <class Unit>
    std::vector<Unit> get_live_units(std::vector<Unit> values) const;

    double get_projectile_damage(const model::Projectile& value) const;

    template <class Unit>
    static bool has_collision(const Circle& unit, const std::vector<Unit>& others);

    template <class Unit, class Other>
    static bool has_collision(const Unit& unit, const Other& other);

    template <class Other>
    static bool has_collision(const Circle& circle, const Other& other);

    static bool has_collision(const Circle& unit, const Circle& other);

    static std::vector<model::Status> get_statuses_at_next_tick(const std::vector<model::Status>& values);
    static std::pair<bool, model::Status> get_status_at_next_tick(const model::Status& value);

    static std::vector<int> get_next_ticks_values(const std::vector<int>& ticks);
    static int get_next_ticks_value(int value);

    model::Wizard get_updated_wizard(const model::Wizard& value, const model::Move& move) const;
    static model::Minion get_updated_minion(const model::Minion& value, const MinionMove& move);

    static Point get_speed(double angle, const model::Move& move);
    static Point get_speed(double angle, const MinionMove& move);

    std::vector<int> set_action_cooldown(std::vector<int> values, model::ActionType action, const model::Wizard& unit) const;
};

} // namespace simulation
} // namespace strategy
