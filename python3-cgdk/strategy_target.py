from itertools import chain
from math import acos
from numpy import array
from scipy.optimize import minimize

from model.Bonus import Bonus
from model.Building import Building
from model.BuildingType import BuildingType
from model.Faction import Faction
from model.Minion import Minion
from model.MinionType import MinionType
from model.Wizard import Wizard

from strategy_common import Point, Line, Circle

BONUS_WEIGHT = 1
DIRECTION_WEIGHT = 1
PROJECTILE_WEIGHT = 0.5
TARGET_DISTANCE_WEIGHT = 1
UNIT_WEIGHT = 1


def get_target(me: Wizard, buildings, minions, wizards, trees, projectiles, bonuses, guardian_tower_attack_range,
               faction_base_attack_range, orc_woodcutter_attack_range, fetish_blowdart_attack_range,
               magic_missile_direct_damage, magic_missile_radius, map_size, max_distance=None, max_iterations=None,
               penalties=None):
    my_position = Point(me.x, me.y)

    def filter_friends(units):
        return (v for v in units if v.faction == me.faction)

    def filter_enemies(units):
        return (v for v in units if is_enemy(v, me.faction))

    def filter_max_distance(units):
        if max_distance is None:
            return units
        else:
            return (v for v in units if my_position.distance(v.position) <= max_distance)

    enemy_buildings = tuple(filter_max_distance(filter_enemies(buildings)))
    enemy_minions = tuple(filter_max_distance(filter_enemies(minions)))
    enemy_wizards = tuple(filter_max_distance(filter_enemies(wizards)))
    bonuses = tuple(filter_max_distance(bonuses))
    if not enemy_buildings and not enemy_minions and not enemy_wizards and not bonuses:
        return None, None
    get_damage = make_get_damage(magic_missile_direct_damage)

    def target_penalty(unit):
        distance = unit.position.distance(my_position)
        return distance * unit.life / unit.max_life if distance <= me.vision_range else distance

    if bonuses:
        target = min(bonuses, key=lambda v: my_position.distance(Point(v.x, v.y)))
    elif enemy_wizards or enemy_minions or enemy_buildings:
        target = min(chain(enemy_wizards, enemy_minions, enemy_buildings, bonuses), key=target_penalty)
    else:
        target = None
    get_attack_range = make_get_attack_range(
        guardian_tower_attack_range=guardian_tower_attack_range,
        faction_base_attack_range=faction_base_attack_range,
        orc_woodcutter_attack_range=orc_woodcutter_attack_range,
        fetish_blowdart_attack_range=fetish_blowdart_attack_range,
    )
    other_wizards = tuple(v for v in wizards if v.id != me.id)
    units = tuple(chain(buildings, trees, minions, other_wizards))
    friends_units = tuple(filter_friends(chain(buildings, minions, other_wizards)))
    target_position = target.position if target else None

    def position_penalty(values):
        position = Point(values[0], values[1])

        def unit_intersection_penalty(unit):
            return distance_penalty(position.distance(unit.position), 2 * me.radius + unit.radius)

        def unit_danger_penalty(unit):
            if not is_enemy(unit, me.faction):
                return 0
            safe_distance = max(me.cast_range - magic_missile_radius,
                                (me.radius + get_attack_range(unit)) * min(1, get_damage(unit) / me.life))
            distance_to_unit = position.distance(unit.position)
            return distance_penalty(distance_to_unit, safe_distance)

        def target_distance_penalty():
            if not target:
                return 0
            if isinstance(target, Bonus):
                return bonus_penalty(target)
            else:
                return max(unit_danger_penalty(target),
                           1 - distance_penalty(position.distance(target_position),
                                                my_position.distance(target_position)))

        def friend_units_intersections_penalties():
            for friend in friends_units:
                circle = Circle(friend.position, friend.radius)
                intersection = circle.has_intersection_with_moving_circle(
                    Circle(position, magic_missile_radius), target_position)
                if not intersection:
                    yield 0
                else:
                    target_to_friend = friend.position - target_position
                    tangent_cos = (friend.radius + magic_missile_radius) / target_position.distance(friend.position)
                    tangent_angle = acos(min(1, max(-1, tangent_cos)))
                    tangent1_direction = target_to_friend.rotate(tangent_angle)
                    tangent2_direction = target_to_friend.rotate(-tangent_angle)
                    tangent1 = target_position + tangent1_direction
                    tangent2 = target_position + tangent2_direction
                    tangent1_distance = Line(target_position, tangent1).distance(position)
                    tangent2_distance = Line(target_position, tangent2).distance(position)
                    max_distance = (tangent1_distance + tangent2_distance) * 0.5
                    distance_to_tangent = min(tangent1_distance, tangent2_distance)
                    yield distance_to_tangent / max_distance

        def direction_penalty():
            return max(friend_units_intersections_penalties()) if target_position and friends_units else 0

        def bonus_penalty(bonus):
            return 1 - distance_penalty(position.distance(bonus.position), my_position.distance(bonus.position))

        def units_penalties():
            for unit in units:
                yield max(unit_intersection_penalty(unit), unit_danger_penalty(unit))

        def bonuses_penalties():
            for bonus in bonuses:
                yield bonus_penalty(bonus)

        def projectile_penalty(projectile):
            projectile_speed = projectile.mean_speed
            safe_distance = me.radius + projectile.radius
            distance_to = Line(projectile.position, projectile.position + projectile_speed).distance(position)
            return distance_penalty(distance_to, safe_distance)

        def projectiles_penalties():
            for projectile in projectiles:
                yield projectile_penalty(projectile)

        def borders_penalty():
            return (0 if me.radius < position.x < map_size - me.radius
                    and me.radius < position.y < map_size - me.radius else 1)

        penalties_count = (
            + len(units) * UNIT_WEIGHT
            + len(bonuses) * BONUS_WEIGHT
            + len(projectiles) * PROJECTILE_WEIGHT
            + (DIRECTION_WEIGHT if target_position and friends_units else 0)
            + (TARGET_DISTANCE_WEIGHT if target_position else 0)
        )
        penalty = max(borders_penalty() * penalties_count, (
            + sum(units_penalties()) * UNIT_WEIGHT
            + sum(bonuses_penalties()) * BONUS_WEIGHT
            + sum(projectiles_penalties()) * PROJECTILE_WEIGHT
            + direction_penalty() * DIRECTION_WEIGHT
            + target_distance_penalty() * TARGET_DISTANCE_WEIGHT
        ))
        if penalties is not None:
            penalties.append((position, penalty))
        return penalty

    from_my_position = minimize(position_penalty, array([my_position.x, my_position.y]),
                                method='Nelder-Mead', options=dict(maxiter=max_iterations))
    result = Point(from_my_position.x[0], from_my_position.x[1])
    return target if target else None, result


NOT_ENEMIES = frozenset((Faction.NEUTRAL, Faction.OTHER))


def is_enemy(unit, my_faction):
    return unit.faction != my_faction and unit.faction not in NOT_ENEMIES


def make_get_attack_range(guardian_tower_attack_range, faction_base_attack_range,
                          orc_woodcutter_attack_range, fetish_blowdart_attack_range):
    buildings = {
        BuildingType.GUARDIAN_TOWER: guardian_tower_attack_range,
        BuildingType.FACTION_BASE: faction_base_attack_range,
    }
    minions = {
        MinionType.ORC_WOODCUTTER: orc_woodcutter_attack_range,
        MinionType.FETISH_BLOWDART: fetish_blowdart_attack_range,
    }

    def impl(unit):
        if isinstance(unit, Building):
            return buildings[unit.type]
        if isinstance(unit, Minion):
            return minions[unit.type]
        if isinstance(unit, Wizard):
            return unit.cast_range

    return impl


def make_get_damage(magic_missile_direct_damage):

    def impl(unit):
        if isinstance(unit, (Building, Minion)):
            return unit.damage
        if isinstance(unit, Wizard):
            return magic_missile_direct_damage

    return impl


def distance_penalty(value, safe_distance):
    return max(0, (safe_distance - value) / safe_distance)
