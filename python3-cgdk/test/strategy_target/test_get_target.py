import pytest

from itertools import chain

from model.Bonus import Bonus
from model.Faction import Faction
from model.Minion import Minion
from model.MinionType import MinionType
from model.Tree import Tree
from model.Wizard import Wizard

from strategy_common import Point, Circle
from strategy_target import get_target

from test.common import (
    BONUS_RADIUS,
    FACTION_BASE_ATTACK_RANGE,
    FETISH_BLOWDART_ATTACK_RANGE,
    FETISH_BLOWDART_DAMAGE,
    FETISH_BLOWDART_MAX_LIFE,
    GUARDIAN_TOWER_ATTACK_RANGE,
    MAGIC_MISSILE_DIRECT_DAMAGE,
    MAGIC_MISSILE_RADIUS,
    MAP_SIZE,
    MINION_RADIUS,
    ORC_WOODCUTTER_ATTACK_RANGE,
    ORC_WOODCUTTER_DAMAGE,
    ORC_WOODCUTTER_MAX_LIFE,
    TREE_RADIUS,
    WIZARD_CAST_RANGE,
    WIZARD_MAX_LIFE,
    WIZARD_RADIUS,
    WIZARD_VISION_RANGE,
)

WIZARD = Wizard(
    id=1,
    x=1000,
    y=1000,
    speed_x=None,
    speed_y=None,
    angle=None,
    faction=Faction.ACADEMY,
    radius=WIZARD_RADIUS,
    life=WIZARD_MAX_LIFE,
    max_life=WIZARD_MAX_LIFE,
    statuses=None,
    owner_player_id=None,
    me=None,
    mana=None,
    max_mana=None,
    vision_range=WIZARD_VISION_RANGE,
    cast_range=WIZARD_CAST_RANGE,
    xp=None,
    level=None,
    skills=None,
    remaining_action_cooldown_ticks=None,
    remaining_cooldown_ticks_by_action=None,
    master=None,
    messages=None,
)


def test_get_target_with_only_me():
    assert (None, None) == get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=tuple(),
        wizards=[WIZARD],
        trees=tuple(),
        projectiles=tuple(),
        bonuses=tuple(),
        guardian_tower_attack_range=GUARDIAN_TOWER_ATTACK_RANGE,
        faction_base_attack_range=FACTION_BASE_ATTACK_RANGE,
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        magic_missile_radius=MAGIC_MISSILE_RADIUS,
        map_size=MAP_SIZE,
    )


@pytest.mark.parametrize(
    ('minion', 'expected_target', 'expected_position'), [
        (
            Minion(
                id=2,
                x=1100,
                y=1100,
                speed_x=None,
                speed_y=None,
                angle=None,
                faction=Faction.RENEGADES,
                radius=MINION_RADIUS,
                life=FETISH_BLOWDART_MAX_LIFE,
                max_life=FETISH_BLOWDART_MAX_LIFE,
                statuses=None,
                type=MinionType.FETISH_BLOWDART,
                vision_range=None,
                damage=FETISH_BLOWDART_DAMAGE,
                cooldown_ticks=None,
                remaining_action_cooldown_ticks=None,
            ),
            2,
            Point(1008.7003105766321, 998.75084650994177),
        ),
        (
            Minion(
                id=2,
                x=1100,
                y=1100,
                speed_x=None,
                speed_y=None,
                angle=None,
                faction=Faction.RENEGADES,
                radius=MINION_RADIUS,
                life=ORC_WOODCUTTER_MAX_LIFE,
                max_life=ORC_WOODCUTTER_MAX_LIFE,
                statuses=None,
                type=MinionType.ORC_WOODCUTTER,
                vision_range=None,
                damage=ORC_WOODCUTTER_DAMAGE,
                cooldown_ticks=None,
                remaining_action_cooldown_ticks=None,
            ),
            2,
            Point(1008.7003105766321, 998.75084650994177),
        ),
        (
            Minion(
                id=2,
                x=2000,
                y=2000,
                speed_x=None,
                speed_y=None,
                angle=None,
                faction=Faction.RENEGADES,
                radius=MINION_RADIUS,
                life=ORC_WOODCUTTER_MAX_LIFE,
                max_life=ORC_WOODCUTTER_MAX_LIFE,
                statuses=None,
                type=MinionType.ORC_WOODCUTTER,
                vision_range=None,
                damage=ORC_WOODCUTTER_DAMAGE,
                cooldown_ticks=None,
                remaining_action_cooldown_ticks=None,
            ),
            2,
            Point(1690.1666804835595, 1710.0807104067385),
        ),
    ]
)
def test_get_target_with_me_and_enemy_minion(minion, expected_target, expected_position):
    setattr(minion, 'position', Point(minion.x, minion.y))
    target, position = get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=[minion],
        wizards=[WIZARD],
        trees=tuple(),
        projectiles=tuple(),
        bonuses=tuple(),
        guardian_tower_attack_range=GUARDIAN_TOWER_ATTACK_RANGE,
        faction_base_attack_range=FACTION_BASE_ATTACK_RANGE,
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        magic_missile_radius=MAGIC_MISSILE_RADIUS,
        map_size=MAP_SIZE,
    )
    assert target.position.distance(position) <= WIZARD_CAST_RANGE
    assert (target.id, position) == (expected_target, expected_position)


def test_get_target_with_me_and_neural_minion():
    assert (None, None) == get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=[
            Minion(
                id=2,
                x=1100,
                y=1100,
                speed_x=None,
                speed_y=None,
                angle=None,
                faction=Faction.NEUTRAL,
                radius=MINION_RADIUS,
                life=FETISH_BLOWDART_MAX_LIFE,
                max_life=None,
                statuses=None,
                type=MinionType.FETISH_BLOWDART,
                vision_range=None,
                damage=None,
                cooldown_ticks=None,
                remaining_action_cooldown_ticks=None,
            ),
        ],
        wizards=[WIZARD],
        trees=tuple(),
        projectiles=tuple(),
        bonuses=tuple(),
        guardian_tower_attack_range=GUARDIAN_TOWER_ATTACK_RANGE,
        faction_base_attack_range=FACTION_BASE_ATTACK_RANGE,
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        magic_missile_radius=MAGIC_MISSILE_RADIUS,
        map_size=MAP_SIZE,
    )


@pytest.mark.parametrize(
    ('minions', 'wizards', 'expected_target', 'expected_position'), [
        (
            tuple(),
            [
                Wizard(
                    id=2,
                    x=1100,
                    y=1100,
                    speed_x=None,
                    speed_y=None,
                    angle=None,
                    faction=Faction.RENEGADES,
                    radius=WIZARD_RADIUS,
                    life=WIZARD_MAX_LIFE,
                    max_life=WIZARD_MAX_LIFE,
                    statuses=None,
                    owner_player_id=None,
                    me=None,
                    mana=None,
                    max_mana=None,
                    vision_range=None,
                    cast_range=WIZARD_CAST_RANGE,
                    xp=None,
                    level=None,
                    skills=None,
                    remaining_action_cooldown_ticks=None,
                    remaining_cooldown_ticks_by_action=None,
                    master=None,
                    messages=None,
                ),
            ],
            2,
            Point(1008.7003105766321, 998.75084650994177),
        ),
        (
            [
                Minion(
                    id=2,
                    x=1000,
                    y=1100,
                    speed_x=None,
                    speed_y=None,
                    angle=None,
                    faction=Faction.RENEGADES,
                    radius=MINION_RADIUS,
                    life=50,
                    max_life=ORC_WOODCUTTER_MAX_LIFE,
                    statuses=None,
                    type=MinionType.ORC_WOODCUTTER,
                    vision_range=None,
                    damage=ORC_WOODCUTTER_DAMAGE,
                    cooldown_ticks=None,
                    remaining_action_cooldown_ticks=None,
                ),
            ],
            [
                Wizard(
                    id=3,
                    x=1100,
                    y=1000,
                    speed_x=None,
                    speed_y=None,
                    angle=None,
                    faction=Faction.RENEGADES,
                    radius=WIZARD_RADIUS,
                    life=100,
                    max_life=WIZARD_MAX_LIFE,
                    statuses=None,
                    owner_player_id=None,
                    me=None,
                    mana=None,
                    max_mana=None,
                    vision_range=None,
                    cast_range=WIZARD_CAST_RANGE,
                    xp=None,
                    level=None,
                    skills=None,
                    remaining_action_cooldown_ticks=None,
                    remaining_cooldown_ticks_by_action=None,
                    master=None,
                    messages=None,
                ),
            ],
            3,
            Point(1404.6875, 531.25),
        ),
        (
            [
                Minion(
                    id=2,
                    x=1000,
                    y=1100,
                    speed_x=None,
                    speed_y=None,
                    angle=None,
                    faction=Faction.RENEGADES,
                    radius=MINION_RADIUS,
                    life=100,
                    max_life=ORC_WOODCUTTER_MAX_LIFE,
                    statuses=None,
                    type=MinionType.ORC_WOODCUTTER,
                    vision_range=None,
                    damage=ORC_WOODCUTTER_DAMAGE,
                    cooldown_ticks=None,
                    remaining_action_cooldown_ticks=None,
                ),
            ],
            [
                Wizard(
                    id=3,
                    x=1100,
                    y=1000,
                    speed_x=None,
                    speed_y=None,
                    angle=None,
                    faction=Faction.RENEGADES,
                    radius=WIZARD_RADIUS,
                    life=50,
                    max_life=WIZARD_MAX_LIFE,
                    statuses=None,
                    owner_player_id=None,
                    me=None,
                    mana=None,
                    max_mana=None,
                    vision_range=None,
                    cast_range=WIZARD_CAST_RANGE,
                    xp=None,
                    level=None,
                    skills=None,
                    remaining_action_cooldown_ticks=None,
                    remaining_cooldown_ticks_by_action=None,
                    master=None,
                    messages=None,
                ),
            ],
            3,
            Point(1404.6875, 531.25),
        ),
    ]
)
def test_get_target_with_enemy_minions_and_wizards(minions, wizards, expected_target, expected_position):
    for unit in chain(minions, wizards):
        setattr(unit, 'position', Point(unit.x, unit.y))
    target, position = get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=minions,
        wizards=wizards + [WIZARD],
        trees=tuple(),
        projectiles=tuple(),
        bonuses=tuple(),
        guardian_tower_attack_range=GUARDIAN_TOWER_ATTACK_RANGE,
        faction_base_attack_range=FACTION_BASE_ATTACK_RANGE,
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        magic_missile_radius=MAGIC_MISSILE_RADIUS,
        map_size=MAP_SIZE,
    )
    assert target.position.distance(position) <= WIZARD_CAST_RANGE
    assert (target.id, position) == (expected_target, expected_position)


def test_get_target_with_me_and_tree():
    tree = Tree(
        id=None,
        x=1100,
        y=1100,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.OTHER,
        radius=TREE_RADIUS,
        life=None,
        max_life=None,
        statuses=None,
    )
    setattr(tree, 'position', Point(tree.x, tree.y))
    assert (None, None) == get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=tuple(),
        wizards=[WIZARD],
        trees=[tree],
        projectiles=tuple(),
        bonuses=tuple(),
        guardian_tower_attack_range=GUARDIAN_TOWER_ATTACK_RANGE,
        faction_base_attack_range=FACTION_BASE_ATTACK_RANGE,
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        magic_missile_radius=MAGIC_MISSILE_RADIUS,
        map_size=MAP_SIZE,
    )


def test_get_target_with_me_and_nearby_tree():
    tree = Tree(
        id=2,
        x=WIZARD.x + WIZARD_RADIUS + TREE_RADIUS + 1,
        y=WIZARD.y,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.OTHER,
        radius=TREE_RADIUS,
        life=100,
        max_life=None,
        statuses=None,
    )
    setattr(tree, 'position', Point(tree.x, tree.y))
    target, position = get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=tuple(),
        wizards=[WIZARD],
        trees=[tree],
        projectiles=tuple(),
        bonuses=tuple(),
        guardian_tower_attack_range=GUARDIAN_TOWER_ATTACK_RANGE,
        faction_base_attack_range=FACTION_BASE_ATTACK_RANGE,
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        magic_missile_radius=MAGIC_MISSILE_RADIUS,
        map_size=MAP_SIZE,
    )
    assert (target.id, position) == (2, tree.position)


def test_get_target_with_me_and_bonus():
    bonus = Bonus(
        id=2,
        x=1100,
        y=1100,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=None,
        radius=BONUS_RADIUS,
        type=None,
    )
    setattr(bonus, 'position', Point(bonus.x, bonus.y))
    target, position = get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=tuple(),
        wizards=[WIZARD],
        trees=tuple(),
        projectiles=tuple(),
        bonuses=[bonus],
        guardian_tower_attack_range=GUARDIAN_TOWER_ATTACK_RANGE,
        faction_base_attack_range=FACTION_BASE_ATTACK_RANGE,
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        magic_missile_radius=MAGIC_MISSILE_RADIUS,
        map_size=MAP_SIZE,
    )
    assert target.position.distance(position) <= bonus.radius + WIZARD_RADIUS
    assert (target.id, position) == (2, Point(1099.9999654385429, 1100.0000372327122))


@pytest.mark.parametrize(
    ('minions', 'expected_target', 'expected_position'), [
        (
            [
                Minion(
                    id=2,
                    x=1200,
                    y=1000,
                    speed_x=None,
                    speed_y=None,
                    angle=None,
                    faction=Faction.ACADEMY,
                    radius=MINION_RADIUS,
                    life=FETISH_BLOWDART_MAX_LIFE,
                    max_life=FETISH_BLOWDART_MAX_LIFE,
                    statuses=None,
                    type=MinionType.FETISH_BLOWDART,
                    vision_range=None,
                    damage=FETISH_BLOWDART_DAMAGE,
                    cooldown_ticks=None,
                    remaining_action_cooldown_ticks=None,
                ),
                Minion(
                    id=3,
                    x=1200,
                    y=1200,
                    speed_x=None,
                    speed_y=None,
                    angle=None,
                    faction=Faction.RENEGADES,
                    radius=MINION_RADIUS,
                    life=FETISH_BLOWDART_MAX_LIFE,
                    max_life=FETISH_BLOWDART_MAX_LIFE,
                    statuses=None,
                    type=MinionType.FETISH_BLOWDART,
                    vision_range=None,
                    damage=FETISH_BLOWDART_DAMAGE,
                    cooldown_ticks=None,
                    remaining_action_cooldown_ticks=None,
                ),
            ],
            3,
            Point(1050.1523807348231, 1056.0676199122199),
        ),
        (
            [
                Minion(
                    id=2,
                    x=1200,
                    y=1000,
                    speed_x=None,
                    speed_y=None,
                    angle=None,
                    faction=Faction.ACADEMY,
                    radius=MINION_RADIUS,
                    life=FETISH_BLOWDART_MAX_LIFE,
                    max_life=FETISH_BLOWDART_MAX_LIFE,
                    statuses=None,
                    type=MinionType.FETISH_BLOWDART,
                    vision_range=None,
                    damage=FETISH_BLOWDART_DAMAGE,
                    cooldown_ticks=None,
                    remaining_action_cooldown_ticks=None,
                ),
                Minion(
                    id=3,
                    x=1200 + 3 * MINION_RADIUS,
                    y=1000,
                    speed_x=None,
                    speed_y=None,
                    angle=None,
                    faction=Faction.RENEGADES,
                    radius=MINION_RADIUS,
                    life=FETISH_BLOWDART_MAX_LIFE,
                    max_life=FETISH_BLOWDART_MAX_LIFE,
                    statuses=None,
                    type=MinionType.FETISH_BLOWDART,
                    vision_range=None,
                    damage=FETISH_BLOWDART_DAMAGE,
                    cooldown_ticks=None,
                    remaining_action_cooldown_ticks=None,
                ),
            ],
            3,
            Point(1253.1420997988896, 1203.2235106844803),
        ),
    ]
)
def test_get_target_with_me_friend_and_enemy_minion(minions, expected_target, expected_position):
    for unit in minions:
        setattr(unit, 'position', Point(unit.x, unit.y))
    target, position = get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=minions,
        wizards=[WIZARD],
        trees=tuple(),
        projectiles=tuple(),
        bonuses=tuple(),
        guardian_tower_attack_range=GUARDIAN_TOWER_ATTACK_RANGE,
        faction_base_attack_range=FACTION_BASE_ATTACK_RANGE,
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        magic_missile_radius=MAGIC_MISSILE_RADIUS,
        map_size=MAP_SIZE,
    )
    assert target.position.distance(position) <= WIZARD_CAST_RANGE
    assert (target.id, position) == (expected_target, expected_position)
    assert not Circle(Point(minions[0].x, minions[0].y), minions[0].radius).has_intersection_with_moving_circle(
        Circle(position, MAGIC_MISSILE_RADIUS), Point(minions[1].x, minions[1].y))
