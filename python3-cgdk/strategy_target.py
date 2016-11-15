from itertools import chain
from numpy import array
from scipy.optimize import minimize

from model.Building import Building
from model.BuildingType import BuildingType
from model.Faction import Faction
from model.Minion import Minion
from model.MinionType import MinionType
from model.Wizard import Wizard

from strategy_common import Point


def get_target(me: Wizard, buildings, minions, wizards, guardian_tower_attack_range, faction_base_attack_range,
               orc_woodcutter_attack_range, fetish_blowdart_attack_range, magic_missile_direct_damage):
    enemy_buildings = tuple(filter_enemies(buildings, me.faction))
    enemy_minions = tuple(filter_enemies(minions, me.faction))
    enemy_wizards = tuple(filter_enemies(wizards, me.faction))
    if not enemy_buildings and not enemy_minions and not enemy_wizards:
        return None, None
    my_position = Point(me.x, me.y)
    get_damage = make_get_damage(magic_missile_direct_damage)

    def target_penalty(unit):
        distance = Point(unit.x, unit.y).distance(my_position)
        distance_penalty = (distance if distance <= me.vision_range
                            else (distance - me.vision_range) ** 2 + me.vision_range)
        kill_ability = (me.life / get_damage(unit)) / (unit.life / magic_missile_direct_damage)
        return distance_penalty / kill_ability

    if enemy_wizards or enemy_minions or enemy_buildings:
        target = min(chain(enemy_wizards, enemy_minions, enemy_buildings), key=target_penalty)
    else:
        target = None
    get_attack_range = make_get_attack_range(
        guardian_tower_attack_range=guardian_tower_attack_range,
        faction_base_attack_range=faction_base_attack_range,
        orc_woodcutter_attack_range=orc_woodcutter_attack_range,
        fetish_blowdart_attack_range=fetish_blowdart_attack_range,
    )
    units = tuple(chain(buildings, minions, (v for v in wizards if v.id != me.id)))

    def position_penalty(values):
        position = Point(values[0], values[1])

        def generate():
            for v in units:
                if is_enemy(v, me.faction):
                    safe_distance = max(get_attack_range(v), me.cast_range / 2)
                else:
                    safe_distance = 2 * (me.radius + v.radius)
                unit_position = Point(v.x, v.y)
                if unit_position == position:
                    distance_to_position = safe_distance
                else:
                    preferred_position = (unit_position +
                                          (position - unit_position).normalized() * safe_distance)
                    distance_to_position = my_position.distance(preferred_position)
                distance_to_unit = position.distance(unit_position)
                if distance_to_unit < safe_distance:
                    yield distance_to_position - distance_to_unit
                else:
                    yield distance_to_unit - 2 * safe_distance + distance_to_position

        return max(generate())

    result = minimize(position_penalty, array([my_position.x, my_position.y]), method='Nelder-Mead').x
    return target if target else None, Point(result[0], result[1])


def filter_enemies(units, my_faction):
    return (v for v in units if is_enemy(v, my_faction))


def is_enemy(unit, my_faction):
    return unit.faction != my_faction and unit.faction not in {Faction.NEUTRAL, Faction.OTHER}


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
