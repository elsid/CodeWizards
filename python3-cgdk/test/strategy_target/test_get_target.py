import pytest

from model.Bonus import Bonus
from model.Faction import Faction
from model.Minion import Minion
from model.MinionType import MinionType
from model.Tree import Tree
from model.Wizard import Wizard

from strategy_common import Point
from strategy_target import get_target

from test.common import (
    BONUS_RADIUS,
    FACTION_BASE_ATTACK_RANGE,
    FETISH_BLOWDART_ATTACK_RANGE,
    FETISH_BLOWDART_DAMAGE,
    FETISH_BLOWDART_MAX_LIFE,
    GUARDIAN_TOWER_ATTACK_RANGE,
    MAGIC_MISSILE_DIRECT_DAMAGE,
    MINION_RADIUS,
    ORC_WOODCUTTER_ATTACK_RANGE,
    ORC_WOODCUTTER_DAMAGE,
    ORC_WOODCUTTER_MAX_LIFE,
    WIZARD_CAST_RANGE,
    WIZARD_MAX_LIFE,
    WIZARD_RADIUS,
    WIZARD_VISION_RANGE,
)

WIZARD = Wizard(
    id=1,
    x=0,
    y=0,
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
    )


@pytest.mark.parametrize(
    ('minion', 'expected_target', 'expected_position'), [
        (
            Minion(
                id=2,
                x=100,
                y=100,
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
            Point(-112.12787499999999, -112.12762500000004),
        ),
        (
            Minion(
                id=2,
                x=100,
                y=100,
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
            Point(-110.07974999999998, -110.07975000000005),
        ),
        (
            Minion(
                id=2,
                x=1000,
                y=1000,
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
            Point(786.43199999999968, 786.43174999999997),
        ),
    ]
)
def test_get_target_with_me_and_enemy_minion(minion, expected_target, expected_position):
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
    )
    assert (target.id, position) == (expected_target, expected_position)


def test_get_target_with_me_and_neural_minion():
    assert (None, None) == get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=[
            Minion(
                id=2,
                x=100,
                y=100,
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
    )


@pytest.mark.parametrize(
    ('minions', 'wizards', 'expected_target', 'expected_position'), [
        (
            tuple(),
            [
                Wizard(
                    id=2,
                    x=100,
                    y=100,
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
            Point(-325.48194438171373, -325.48555561828624),
        ),
        (
            [
                Minion(
                    id=2,
                    x=0,
                    y=100,
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
                    x=100,
                    y=0,
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
            2,
            Point(-36.276570207209446, -194.80094953623865),
        ),
        (
            [
                Minion(
                    id=2,
                    x=0,
                    y=100,
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
                    x=100,
                    y=0,
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
            Point(-36.265036467690905, -194.81294730381165),
        ),
    ]
)
def test_get_target_with_enemy_minions_and_wizards(minions, wizards, expected_target, expected_position):
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
    )
    assert (target.id, position) == (expected_target, expected_position)


def test_get_target_with_me_and_tree():
    assert (None, None) == get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=tuple(),
        wizards=[WIZARD],
        trees=[
            Tree(
                id=None,
                x=100,
                y=100,
                speed_x=None,
                speed_y=None,
                angle=None,
                faction=Faction.OTHER,
                radius=None,
                life=None,
                max_life=None,
                statuses=None,
            ),
        ],
        projectiles=tuple(),
        bonuses=tuple(),
        guardian_tower_attack_range=GUARDIAN_TOWER_ATTACK_RANGE,
        faction_base_attack_range=FACTION_BASE_ATTACK_RANGE,
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
    )


def test_get_target_with_me_and_bonus():
    target, position = get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=tuple(),
        wizards=[WIZARD],
        trees=tuple(),
        projectiles=tuple(),
        bonuses=[
            Bonus(
                id=2,
                x=100,
                y=100,
                speed_x=None,
                speed_y=None,
                angle=None,
                faction=None,
                radius=BONUS_RADIUS,
                type=None,
            )
        ],
        guardian_tower_attack_range=GUARDIAN_TOWER_ATTACK_RANGE,
        faction_base_attack_range=FACTION_BASE_ATTACK_RANGE,
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
    )
    assert (target.id, position) == (2, Point(100.09593749999995, 100.09581250000006))
