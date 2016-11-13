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
               orc_woodcutter_attack_range, fetish_blowdart_attack_range, wizard_cast_range):
    enemy_buildings = tuple(filter_enemies(buildings, me.faction))
    enemy_minions = tuple(filter_enemies(minions, me.faction))
    enemy_wizards = tuple(filter_enemies(wizards, me.faction))
    units = tuple(chain(enemy_buildings, enemy_minions, enemy_wizards))
    if not units:
        return None, None
    my_position = Point(me.x, me.y)
    if enemy_wizards or enemy_minions:
        target = min(chain(enemy_wizards, enemy_minions), key=lambda v: Point(v.x, v.y).distance(my_position))
    else:
        target = None
    get_attack_range = make_get_attack_range(
        guardian_tower_attack_range=guardian_tower_attack_range,
        faction_base_attack_range=faction_base_attack_range,
        orc_woodcutter_attack_range=orc_woodcutter_attack_range,
        fetish_blowdart_attack_range=fetish_blowdart_attack_range,
        wizard_cast_range=wizard_cast_range,
    )

    def function(values):
        position = Point(values[0], values[1])

        def generate():
            for v in units:
                distance = position.distance(Point(v.x, v.y))
                safe_distance = max(get_attack_range(v) + v.radius, me.cast_range / 2)
                yield -distance if distance < safe_distance else (distance - safe_distance) ** 2 - safe_distance

        return min(generate())

    result = minimize(function, array([my_position.x, my_position.y])).x
    return target if target else None, Point(result[0], result[1])


def filter_enemies(units, my_faction):
    return (v for v in units if v.faction != my_faction and v.faction not in {Faction.NEUTRAL, Faction.OTHER})


def filter_neural(units):
    return (v for v in units if v.faction == Faction.NEUTRAL)


def make_get_attack_range(guardian_tower_attack_range, faction_base_attack_range,
                          orc_woodcutter_attack_range, fetish_blowdart_attack_range, wizard_cast_range):
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
            return wizard_cast_range

    return impl
