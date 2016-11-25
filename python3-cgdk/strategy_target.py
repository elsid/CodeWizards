from itertools import chain
from math import acos
from numpy import array
from scipy.optimize import minimize

from model.Bonus import Bonus
from model.Building import Building
from model.Faction import Faction
from model.Minion import Minion
from model.MinionType import MinionType
from model.StatusType import StatusType
from model.Tree import Tree
from model.Wizard import Wizard

from strategy_common import Point, Line, Circle

BONUS_WEIGHT = 1
DIRECTION_WEIGHT = 1
PROJECTILE_WEIGHT = 1
TARGET_DISTANCE_WEIGHT = 1
UNIT_WEIGHT = 1


def get_optimal_position(target, me: Wizard, buildings, minions, wizards, trees, projectiles, bonuses,
                         orc_woodcutter_attack_range, fetish_blowdart_attack_range, magic_missile_direct_damage,
                         magic_missile_radius, dart_radius, map_size, shielded_direct_damage_absorption_factor,
                         empowered_damage_factor, max_distance=None, max_iterations=None, penalties=None):
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
        factor = 2 if me.mean_speed.norm() < 1 else 1.3
        trees = tuple(v for v in trees if my_position.distance(v.position) < factor * me.radius + v.radius)
        if not trees:
            return my_position
        target = min(trees, key=lambda v: v.life)
        return target.position
    get_damage = make_get_damage(magic_missile_direct_damage, empowered_damage_factor)
    get_attack_range = make_get_attack_range(
        orc_woodcutter_attack_range=orc_woodcutter_attack_range,
        fetish_blowdart_attack_range=fetish_blowdart_attack_range,
        magic_missile_radius=magic_missile_radius,
        dart_radius=dart_radius,
        my_radius=me.radius,
    )
    other_wizards = tuple(v for v in wizards if v.id != me.id)
    units = tuple(v for v in chain(buildings, trees, minions, other_wizards) if id(v) != id(target))
    friends_wizards = tuple(filter_friends(chain(other_wizards)))
    friends_units = tuple(chain(friends_wizards, filter_friends(chain(minions, buildings))))
    enemies_units = tuple(filter_enemies(chain(buildings, minions, other_wizards)))
    target_position = target.position if target else None
    damage_factor = 1 - next((shielded_direct_damage_absorption_factor * bool(v.remaining_duration_ticks)
                              for v in me.statuses if v.type == StatusType.SHIELDED), 0)

    def position_penalty(values):
        position = Point(values[0], values[1])

        def get_unit_damage(unit):
            attack_range = get_attack_range(unit)
            distance = position.distance(unit.position)
            return get_damage(unit) * (1 if attack_range >= distance
                                       else distance_penalty(distance - attack_range, 0.5 * attack_range))

        sum_damage = sum(damage_factor * get_unit_damage(v) for v in enemies_units)

        def unit_intersection_penalty(unit):
            if isinstance(unit, Tree):
                return distance_penalty(position.distance(unit.position), 2 * me.radius + 2 * unit.radius)
            else:
                return distance_penalty(position.distance(unit.position), 1.1 * me.radius + unit.radius)

        def unit_danger_penalty(unit):
            if not is_enemy(unit, me.faction):
                return 0
            distance_to_unit = position.distance(unit.position)
            if isinstance(unit, Minion) and friends_units:
                nearest_friend = min(friends_units, key=lambda v: v.position.distance(unit.position))
                if nearest_friend.position.distance(unit.position) < distance_to_unit:
                    return 0
            add_damage = damage_factor * (get_damage(unit) - get_unit_damage(unit))
            safe_distance = max(me.cast_range + magic_missile_radius,
                                (get_attack_range(unit) + 2 * me.radius) *
                                min(1, 2 * (sum_damage + add_damage) / me.life))
            return distance_penalty(distance_to_unit, safe_distance)

        def target_distance_penalty():
            if not target:
                return 0
            if isinstance(target, Bonus):
                return bonus_penalty(target)
            else:
                max_cast_range = me.cast_range + magic_missile_radius
                distance = position.distance(target_position)
                if distance <= max_cast_range:
                    d_penalty = 0
                else:
                    safe_distance = max(max_cast_range, map_size)
                    d_penalty = 1 - distance_penalty(distance - max_cast_range, safe_distance)
                return max(unit_danger_penalty(target), d_penalty)

        def friend_unit_intersection_penalty(friend):
            circle = Circle(friend.position, friend.radius)
            intersection = circle.has_intersection_with_moving_circle(
                Circle(position, magic_missile_radius), target_position)
            if not intersection:
                return 0
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
            return distance_to_tangent / max_distance

        def friend_units_intersections_penalties():
            return (friend_unit_intersection_penalty(v) for v in friends_wizards)

        def direction_penalty():
            return max(friend_units_intersections_penalties()) if target_position and friends_wizards else 0

        def bonus_penalty(bonus):
            return 1 - distance_penalty(position.distance(bonus.position),
                                        my_position.distance(bonus.position) + me.radius)

        def units_penalties():
            for unit in units:
                yield max(unit_intersection_penalty(unit), unit_danger_penalty(unit))

        def bonuses_penalties():
            return (bonus_penalty(v) for v in bonuses)

        def projectile_penalty(projectile):
            projectile_speed = projectile.mean_speed
            safe_distance = 2 * me.radius + projectile.radius
            distance_to = Line(projectile.position, projectile.position + projectile_speed).distance(position)
            return distance_penalty(distance_to, safe_distance)

        def projectiles_penalties():
            return (projectile_penalty(v) for v in projectiles)

        def borders_penalty():
            return (0 if me.radius < position.x < map_size - me.radius
                    and me.radius < position.y < map_size - me.radius else 1)

        penalties_count = (
            + len(units) * UNIT_WEIGHT
            + len(bonuses) * BONUS_WEIGHT
            + len(projectiles) * PROJECTILE_WEIGHT
            + (DIRECTION_WEIGHT if target_position and friends_wizards else 0)
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
    return Point(from_my_position.x[0], from_my_position.x[1])


def get_target(me: Wizard, buildings, minions, wizards, trees, bonuses, magic_missile_direct_damage,
               empowered_damage_factor, staff_range, max_distance=None):
    my_position = Point(me.x, me.y)

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
        factor = 2 if me.mean_speed.norm() < 1 else 1.3
        trees = tuple(v for v in trees if my_position.distance(v.position) < factor * me.radius + v.radius)
        if not trees:
            return None
        target = min(trees, key=lambda v: v.life)
        return target
    get_damage = make_get_damage(magic_missile_direct_damage, empowered_damage_factor)

    def target_penalty(unit):
        distance = unit.position.distance(my_position)
        return distance * unit.life / get_damage(me) if distance <= 2 * unit.radius + me.vision_range else distance

    def is_in_staff_range(unit):
        return my_position.distance(unit.position) <= staff_range + unit.radius

    def get_optimal_in_range(units, is_in_range):
        in_range = tuple(v for v in units if is_in_range(v))
        return min(in_range, key=target_penalty) if in_range else None

    if bonuses:
        return min(bonuses, key=lambda v: my_position.distance(v.position))
    elif enemy_wizards or enemy_minions or enemy_buildings:
        target = get_optimal_in_range(enemy_wizards, is_in_staff_range)
        if target is None:
            target = get_optimal_in_range(enemy_minions, is_in_staff_range)
        if target is None:
            target = get_optimal_in_range(enemy_buildings, is_in_staff_range)
        if target is None:
            if enemy_wizards:
                target = min(enemy_wizards, key=target_penalty)
            elif enemy_minions:
                target = min(enemy_minions, key=target_penalty)
            elif enemy_buildings:
                target = min(enemy_buildings, key=target_penalty)
        return target

NOT_ENEMIES = frozenset((Faction.NEUTRAL, Faction.OTHER))


def is_enemy(unit, my_faction):
    return unit.faction != my_faction and unit.faction not in NOT_ENEMIES


def make_get_attack_range(orc_woodcutter_attack_range, fetish_blowdart_attack_range, magic_missile_radius,
                          dart_radius, my_radius):
    minions = {
        MinionType.ORC_WOODCUTTER: orc_woodcutter_attack_range,
        MinionType.FETISH_BLOWDART: fetish_blowdart_attack_range,
    }

    def impl(unit):
        if isinstance(unit, Building):
            return unit.attack_range + my_radius
        if isinstance(unit, Minion):
            if unit.type == MinionType.ORC_WOODCUTTER:
                return minions[unit.type] + my_radius
            elif unit.type == MinionType.FETISH_BLOWDART:
                return minions[unit.type] + my_radius + dart_radius
        if isinstance(unit, Wizard):
            return unit.cast_range + my_radius + magic_missile_radius

    return impl


def make_get_damage(magic_missile_direct_damage, empowered_damage_factor):

    def impl(unit):
        factor = 1 + next((empowered_damage_factor for v in unit.statuses if v.type == StatusType.EMPOWERED), 0)
        if isinstance(unit, (Building, Minion)):
            return factor * unit.damage
        if isinstance(unit, Wizard):
            return factor * magic_missile_direct_damage

    return impl


def distance_penalty(value, safe_distance):
    return max(0, (safe_distance - value) / safe_distance)
