#include "simulator.hpp"
#include "helpers.hpp"
#include "optimal_position.hpp"

#include <algorithm>

namespace strategy {
namespace simulation {

template <class Unit>
struct GetUnits {
    static const std::vector<Unit>& get(const model::World& world);
};

template <>
struct GetUnits<model::Wizard> {
    static const std::vector<model::Wizard>& get(const model::World& world) {
        return world.getWizards();
    }
};

template <>
struct GetUnits<model::Minion> {
    static const std::vector<model::Minion>& get(const model::World& world) {
        return world.getMinions();
    }
};

template <>
struct GetUnits<model::Projectile> {
    static const std::vector<model::Projectile>& get(const model::World& world) {
        return world.getProjectiles();
    }
};

Circle make_circle(const model::CircularUnit& unit) {
    return Circle(get_position(unit), unit.getRadius());
}

Simulator::Simulator(const model::Game& game, model::World& world)
        : game_(game), world_(world) {
}

void Simulator::next_tick() {
    world_ = get_world_at_next_tick(world_);
}

void Simulator::handle_wizard_move(int id, const model::Move& move) {
    auto& unit = const_cast<model::Wizard&>(get_wizard(id));

    unit = get_updated_wizard(unit, move);

    const auto projectile_type = get_projectile_type_by_action(move.getAction());

    if (projectile_type == model::_PROJECTILE_UNKNOWN_) {
        return;
    }

    auto projectiles = world_.getProjectiles();
    const auto projectile_angle = unit.getAngle() + move.getCastAngle();
    const auto projectile_speed = Point(get_projectile_speed(projectile_type, game_), 0).rotated(projectile_angle);
    const auto projectile_id = projectile_id_counter_++;
    const auto projectile_damage = get_action_damage(move.getAction(), unit, game_).sum();

    const model::Projectile projectile(
        projectile_id,
        unit.getX(),
        unit.getY(),
        projectile_speed.x(),
        projectile_speed.y(),
        projectile_angle,
        unit.getFaction(),
        game_.getMagicMissileRadius(),
        projectile_type,
        unit.getId(),
        unit.getOwnerPlayerId()
    );

    projectiles_damage_.emplace(projectile_id, projectile_damage);

    projectiles.push_back(projectile);

    world_ = model::World(
        world_.getTickIndex(),
        world_.getTickCount(),
        world_.getWidth(),
        world_.getHeight(),
        world_.getPlayers(),
        world_.getWizards(),
        world_.getMinions(),
        projectiles,
        world_.getBonuses(),
        world_.getBuildings(),
        world_.getTrees()
    );
}

void Simulator::handle_minion_move(int id, const MinionMove& move) {
    auto& unit = const_cast<model::Minion&>(get_minion(id));

    unit = get_updated_minion(unit, move);
}

const model::Projectile& Simulator::get_projectile(int id) const {
    return get_unit<model::Projectile>(id);
}

const model::Wizard& Simulator::get_wizard(int id) const {
    return get_unit<model::Wizard>(id);
}

const model::Minion& Simulator::get_minion(int id) const {
    return get_unit<model::Minion>(id);
}

template <class Unit>
const Unit& Simulator::get_unit(int id) const {
    return get_unit(id, get_units<Unit>());
}

template <class Unit>
const std::vector<Unit>& Simulator::get_units() const {
    return GetUnits<Unit>::get(world_);
}

template <class Unit>
const Unit& Simulator::get_unit(int id, const std::vector<Unit>& units) {
    return *std::find_if(units.begin(), units.end(), [&] (const auto& v) { return v.getId() == id; });
}

model::World Simulator::get_world_at_next_tick(const model::World& world) {
    const auto updated_wizards = get_updated_units(world.getWizards());
    const auto updated_minions = get_updated_units(world.getMinions());

    auto projectiles = get_moved_units(world.getProjectiles());

    const auto moved_wizards = get_moved_units(updated_wizards);
    const auto moved_minions = get_moved_units(updated_minions);

    auto damaged_wizards = get_damaged_units(moved_wizards, projectiles);
    auto damaged_minions = get_damaged_units(moved_minions, projectiles);

    const auto live_wizards = get_live_units(std::move(damaged_wizards));
    const auto live_minions = get_live_units(std::move(damaged_minions));

    return model::World(
        world.getTickIndex() + 1,
        world.getTickCount(),
        world.getWidth(),
        world.getHeight(),
        world.getPlayers(),
        live_wizards,
        live_minions,
        projectiles,
        world.getBonuses(),
        world.getBuildings(),
        world.getTrees()
    );
}

template <class Unit>
std::vector<Unit> Simulator::get_updated_units(const std::vector<Unit>& values) {
    std::vector<Unit> result;
    result.reserve(values.size());
    std::transform(values.begin(), values.end(), std::back_inserter(result),
        [&] (const auto& unit) { return Simulator::get_updated_unit(unit); });
    return result;
}

model::Wizard Simulator::get_updated_unit(const model::Wizard& value) {
    return model::Wizard(
        value.getId(),
        value.getX(),
        value.getY(),
        value.getSpeedX(),
        value.getSpeedY(),
        value.getAngle(),
        value.getFaction(),
        value.getRadius(),
        value.getLife(),
        value.getMaxLife(),
        get_statuses_at_next_tick(value.getStatuses()),
        value.getOwnerPlayerId(),
        value.isMe(),
        value.getMana(),
        value.getMaxMana(),
        value.getVisionRange(),
        value.getCastRange(),
        value.getXp(),
        value.getLevel(),
        value.getSkills(),
        get_next_ticks_value(value.getRemainingActionCooldownTicks()),
        get_next_ticks_values(value.getRemainingCooldownTicksByAction()),
        value.isMaster(),
        value.getMessages()
    );
}

model::Minion Simulator::get_updated_unit(const model::Minion& value) {
    return model::Minion(
        value.getId(),
        value.getX(),
        value.getY(),
        value.getSpeedX(),
        value.getSpeedY(),
        value.getAngle(),
        value.getFaction(),
        value.getRadius(),
        value.getLife(),
        value.getMaxLife(),
        get_statuses_at_next_tick(value.getStatuses()),
        value.getType(),
        value.getVisionRange(),
        value.getDamage(),
        value.getCooldownTicks(),
        get_next_ticks_value(value.getRemainingActionCooldownTicks())
    );
}

template <class Unit>
std::vector<Unit> Simulator::get_moved_units(const std::vector<Unit>& values) {
    std::vector<Unit> result;
    result.reserve(values.size());
    std::transform(values.begin(), values.end(), std::back_inserter(result),
        [&] (const auto& unit) { return Simulator::get_moved_unit(unit); });
    return result;
}

model::Projectile Simulator::get_moved_unit(const model::Projectile& value) {
    return model::Projectile(
        value.getId(),
        value.getX() + value.getSpeedX(),
        value.getY() + value.getSpeedY(),
        value.getSpeedX(),
        value.getSpeedY(),
        value.getAngle(),
        value.getFaction(),
        value.getRadius(),
        value.getType(),
        value.getOwnerUnitId(),
        value.getOwnerPlayerId()
    );
}

template <class Unit>
Unit Simulator::get_moved_unit(const Unit& value) {
    const auto next_position = get_position(value) + strategy::get_speed(value);
    return make_moved_unit(value, next_position);
}

model::Wizard Simulator::make_moved_unit(const model::Wizard& value, const Point& next_position) {
    return model::Wizard(
        value.getId(),
        next_position.x(),
        next_position.y(),
        value.getSpeedX(),
        value.getSpeedY(),
        value.getAngle(),
        value.getFaction(),
        value.getRadius(),
        value.getLife(),
        value.getMaxLife(),
        value.getStatuses(),
        value.getOwnerPlayerId(),
        value.isMe(),
        value.getMana(),
        value.getMaxMana(),
        value.getVisionRange(),
        value.getCastRange(),
        value.getXp(),
        value.getLevel(),
        value.getSkills(),
        value.getRemainingActionCooldownTicks(),
        value.getRemainingCooldownTicksByAction(),
        value.isMaster(),
        value.getMessages()
    );
}

model::Minion Simulator::make_moved_unit(const model::Minion& value, const Point& next_position) {
    return model::Minion(
        value.getId(),
        next_position.x(),
        next_position.y(),
        value.getSpeedX(),
        value.getSpeedY(),
        value.getAngle(),
        value.getFaction(),
        value.getRadius(),
        value.getLife(),
        value.getMaxLife(),
        value.getStatuses(),
        value.getType(),
        value.getVisionRange(),
        value.getDamage(),
        value.getCooldownTicks(),
        value.getRemainingActionCooldownTicks()
    );
}

template <class Unit>
std::vector<Unit> Simulator::get_damaged_units(const std::vector<Unit>& values, std::vector<model::Projectile>& projectiles) const {
    std::vector<Unit> result;
    result.reserve(values.size());
    std::transform(values.begin(), values.end(), std::back_inserter(result),
        [&] (const auto& unit) { return this->get_damaged_unit(unit, projectiles); });
    return result;
}

template <class Unit>
Unit Simulator::get_damaged_unit(const Unit& value, std::vector<model::Projectile>& projectiles) const {
    const auto circle = make_circle(value);
    const auto initial_position = get_position(get_unit<Unit>(value.getId()));
    const auto end = std::remove_if(projectiles.begin(), projectiles.end(),
        [&] (const auto& projectile) {
            return !is_owner(value, projectile)
                    && circle.has_intersection(initial_position, make_circle(projectile), get_position(this->get_projectile(projectile.getId())));
        });
    const auto damage = std::accumulate(end, projectiles.end(), 0.0,
        [&] (auto sum, const auto& projectile) { return sum + this->get_projectile_damage(projectile); });

    projectiles.erase(end, projectiles.end());

    return make_damaged_unit(value, value.getLife() - damage);
}

model::Wizard Simulator::make_damaged_unit(const model::Wizard& value, int life) {
    return model::Wizard(
        value.getId(),
        value.getX(),
        value.getY(),
        value.getSpeedY(),
        value.getSpeedX(),
        value.getAngle(),
        value.getFaction(),
        value.getRadius(),
        life,
        value.getMaxLife(),
        value.getStatuses(),
        value.getOwnerPlayerId(),
        value.isMe(),
        value.getMana(),
        value.getMaxMana(),
        value.getVisionRange(),
        value.getCastRange(),
        value.getXp(),
        value.getLevel(),
        value.getSkills(),
        value.getRemainingActionCooldownTicks(),
        value.getRemainingCooldownTicksByAction(),
        value.isMaster(),
        value.getMessages()
    );
}

model::Minion Simulator::make_damaged_unit(const model::Minion& value, int life) {
    return model::Minion(
        value.getId(),
        value.getX(),
        value.getY(),
        value.getSpeedX(),
        value.getSpeedY(),
        value.getAngle(),
        value.getFaction(),
        value.getRadius(),
        life,
        value.getMaxLife(),
        value.getStatuses(),
        value.getType(),
        value.getVisionRange(),
        value.getDamage(),
        value.getCooldownTicks(),
        value.getRemainingActionCooldownTicks()
    );
}

template <class Unit>
std::vector<Unit> Simulator::get_live_units(std::vector<Unit> values) const {
    const auto end = std::remove_if(values.begin(), values.end(), [] (const auto& unit) { return unit.getLife() <= 0; });
    values.erase(end, values.end());
    return values;
}

double Simulator::get_projectile_damage(const model::Projectile& value) const {
    const auto it = projectiles_damage_.find(value.getId());
    return it == projectiles_damage_.end() ? 1 : it->second;
}

template <class Unit>
bool Simulator::has_collision(const Circle& unit, const std::vector<Unit>& wizards) {
    return wizards.end() != std::find_if(wizards.begin(), wizards.end(),
        [&] (const model::Wizard& other) { return unit.has_intersection(make_circle(other)); });
}

template <class Unit, class Other>
bool Simulator::has_collision(const Unit& unit, const Other& other) {
    return has_collision(make_circle(unit), make_circle(other));
}

template <class Other>
bool Simulator::has_collision(const Circle& circle, const Other& other) {
    return has_collision(circle, make_circle(other));
}

bool Simulator::has_collision(const Circle& unit, const Circle& other) {
    return unit.has_intersection(other);
}

std::vector<model::Status> Simulator::get_statuses_at_next_tick(const std::vector<model::Status>& values) {
    std::vector<model::Status> result;
    result.reserve(values.size());

    for (const auto& value : values) {
        const auto& next_value = get_status_at_next_tick(value);
        if (next_value.first) {
            result.push_back(next_value.second);
        }
    }

    return result;
}

std::pair<bool, model::Status> Simulator::get_status_at_next_tick(const model::Status& value) {
    const auto remaining_duration_ticks = get_next_ticks_value(value.getRemainingDurationTicks());

    if (remaining_duration_ticks == 0) {
        return {false, value};
    }

    return {
        true,
        model::Status(
            value.getId(),
            value.getType(),
            value.getWizardId(),
            value.getPlayerId(),
            remaining_duration_ticks
        )
    };
}

std::vector<int> Simulator::get_next_ticks_values(const std::vector<int>& ticks) {
    std::vector<int> result;
    result.reserve(ticks.size());
    std::transform(ticks.begin(), ticks.end(), std::back_inserter(result),
        [&] (int tick) { return get_next_ticks_value(tick); });
    return result;
}

int Simulator::get_next_ticks_value(int value) {
    return std::max(0, value - 1);
}

model::Wizard Simulator::get_updated_wizard(const model::Wizard& value, const model::Move& move) const {
    const auto speed = get_speed(value.getAngle(), move);

    return model::Wizard(
        value.getId(),
        value.getX(),
        value.getY(),
        speed.x(),
        speed.y(),
        value.getAngle() + move.getTurn(),
        value.getFaction(),
        value.getRadius(),
        value.getLife(),
        value.getMaxLife(),
        value.getStatuses(),
        value.getOwnerPlayerId(),
        value.isMe(),
        value.getMana(),
        value.getMaxMana(),
        value.getVisionRange(),
        value.getCastRange(),
        value.getXp(),
        value.getLevel(),
        value.getSkills(),
        move.getAction() == model::_ACTION_UNKNOWN_ ? value.getRemainingActionCooldownTicks()
                                                    : game_.getWizardActionCooldownTicks(),
        move.getAction() == model::_ACTION_UNKNOWN_ ? value.getRemainingCooldownTicksByAction()
                                                    : set_action_cooldown(value.getRemainingCooldownTicksByAction(), move.getAction(), value),
        value.isMaster(),
        value.getMessages()
    );
}

model::Minion Simulator::get_updated_minion(const model::Minion& value, const MinionMove& move) {
    const auto speed = get_speed(value.getAngle(), move);

    return model::Minion(
        value.getId(),
        value.getX(),
        value.getY(),
        speed.x(),
        speed.y(),
        value.getAngle() + move.turn(),
        value.getFaction(),
        value.getRadius(),
        value.getLife(),
        value.getMaxLife(),
        value.getStatuses(),
        value.getType(),
        value.getVisionRange(),
        value.getDamage(),
        value.getCooldownTicks(),
        value.getRemainingActionCooldownTicks()
    );
}

Point Simulator::get_speed(double angle, const model::Move& move) {
    return Point(move.getSpeed(), move.getStrafeSpeed()).rotated(angle);
}

Point Simulator::get_speed(double angle, const MinionMove& move) {
    return Point(move.speed(), 0).rotated(angle);
}

std::vector<int> Simulator::set_action_cooldown(std::vector<int> values, model::ActionType action, const model::Wizard& unit) const {
    values[action] = get_action_cooldown(action, unit, game_);
    return values;
}

} // namespace simulation
} // namespace strategy
