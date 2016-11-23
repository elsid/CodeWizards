import pytest

from hamcrest import assert_that, close_to
from itertools import chain

from model.Bonus import Bonus
from model.Building import Building
from model.Faction import Faction
from model.Minion import Minion
from model.MinionType import MinionType
from model.Status import Status
from model.StatusType import StatusType
from model.Tree import Tree
from model.Wizard import Wizard

from strategy_common import Point, Circle
from strategy_target import get_target

from test.common import (
    BONUS_RADIUS,
    DART_RADIUS,
    FETISH_BLOWDART_ATTACK_RANGE,
    FETISH_BLOWDART_DAMAGE,
    FETISH_BLOWDART_MAX_LIFE,
    GUARDIAN_TOWER_ATTACK_RANGE,
    GUARDIAN_TOWER_DAMAGE,
    GUARDIAN_TOWER_LIFE,
    GUARDIAN_TOWER_RADIUS,
    MAGIC_MISSILE_DIRECT_DAMAGE,
    MAGIC_MISSILE_RADIUS,
    MAP_SIZE,
    MINION_RADIUS,
    ORC_WOODCUTTER_ATTACK_RANGE,
    ORC_WOODCUTTER_DAMAGE,
    ORC_WOODCUTTER_MAX_LIFE,
    SHIELDED_DIRECT_DAMAGE_ABSORPTION_FACTOR,
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
    statuses=tuple(),
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
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        magic_missile_radius=MAGIC_MISSILE_RADIUS,
        dart_radius=DART_RADIUS,
        map_size=MAP_SIZE,
        shielded_direct_damage_absorption_factor=None,
    )


@pytest.mark.parametrize(
    ('minion', 'expected_target', 'expected_position', 'expected_distance'), [
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
            Point(1072.9637063145433, 565.80170028385146),
            WIZARD_CAST_RANGE + MINION_RADIUS + MAGIC_MISSILE_RADIUS - 0.11797133406935245,
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
            Point(1072.9637063145433, 565.80170028385146),
            WIZARD_CAST_RANGE + MINION_RADIUS + MAGIC_MISSILE_RADIUS - 0.11797133406935245,
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
            Point(1686.5111142408321, 1566.6139098964334),
            WIZARD_CAST_RANGE + MINION_RADIUS + MAGIC_MISSILE_RADIUS - 0.11797133406935245,
        ),
    ]
)
def test_get_target_with_me_and_enemy_minion(minion, expected_target, expected_position, expected_distance):
    setattr(minion, 'position', Point(minion.x, minion.y))
    target, position = get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=[minion],
        wizards=[WIZARD],
        trees=tuple(),
        projectiles=tuple(),
        bonuses=tuple(),
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        magic_missile_radius=MAGIC_MISSILE_RADIUS,
        dart_radius=DART_RADIUS,
        map_size=MAP_SIZE,
        shielded_direct_damage_absorption_factor=None,
    )
    assert_that(target.position.distance(position), close_to(expected_distance, 1e-8))
    assert target.position.distance(position) < WIZARD_CAST_RANGE + MAGIC_MISSILE_RADIUS + target.radius
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
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        magic_missile_radius=MAGIC_MISSILE_RADIUS,
        dart_radius=DART_RADIUS,
        map_size=MAP_SIZE,
        shielded_direct_damage_absorption_factor=None,
    )


@pytest.mark.parametrize(
    ('minions', 'wizards', 'expected_target', 'expected_position', 'expected_distance'), [
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
            Point(1086.7249125568969, 555.28164859088633),
            WIZARD_CAST_RANGE + WIZARD_RADIUS + MAGIC_MISSILE_RADIUS - 0.11991199119881912,
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
            Point(1142.5224700111228, 456.78167386157526),
            WIZARD_CAST_RANGE + WIZARD_RADIUS + MAGIC_MISSILE_RADIUS - 0.11991199119881912,
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
            Point(1142.5224700111228, 456.78167386157526),
            WIZARD_CAST_RANGE + WIZARD_RADIUS + MAGIC_MISSILE_RADIUS - 0.11991199119881912,
        ),
    ]
)
def test_get_target_with_enemy_minions_and_wizards(minions, wizards, expected_target, expected_position,
                                                   expected_distance):
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
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        magic_missile_radius=MAGIC_MISSILE_RADIUS,
        dart_radius=DART_RADIUS,
        map_size=MAP_SIZE,
        shielded_direct_damage_absorption_factor=None,
    )
    assert_that(target.position.distance(position), close_to(expected_distance, 1e-8))
    assert target.position.distance(position) < WIZARD_CAST_RANGE + MAGIC_MISSILE_RADIUS + target.radius
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
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        magic_missile_radius=MAGIC_MISSILE_RADIUS,
        dart_radius=DART_RADIUS,
        map_size=MAP_SIZE,
        shielded_direct_damage_absorption_factor=None,
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
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        magic_missile_radius=MAGIC_MISSILE_RADIUS,
        dart_radius=DART_RADIUS,
        map_size=MAP_SIZE,
        shielded_direct_damage_absorption_factor=None,
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
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        magic_missile_radius=MAGIC_MISSILE_RADIUS,
        dart_radius=DART_RADIUS,
        map_size=MAP_SIZE,
        shielded_direct_damage_absorption_factor=None,
    )
    assert target.position.distance(position) <= bonus.radius + WIZARD_RADIUS
    assert (target.id, position) == (2, Point(1099.9999654385429, 1100.0000372327122))


@pytest.mark.parametrize(
    ('friend', 'enemy', 'expected_target', 'expected_position', 'expected_distance'), [
        (
            Wizard(
                id=2,
                x=1200,
                y=1000,
                speed_x=None,
                speed_y=None,
                angle=None,
                faction=Faction.ACADEMY,
                radius=WIZARD_RADIUS,
                life=WIZARD_MAX_LIFE,
                max_life=WIZARD_MAX_LIFE,
                statuses=tuple(),
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
            ),
            Wizard(
                id=3,
                x=1200,
                y=1200,
                speed_x=None,
                speed_y=None,
                angle=None,
                faction=Faction.RENEGADES,
                radius=WIZARD_RADIUS,
                life=WIZARD_MAX_LIFE,
                max_life=WIZARD_MAX_LIFE,
                statuses=tuple(),
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
            ),
            3,
            Point(1055.7926969905302, 674.54918016315344),
            WIZARD_CAST_RANGE + WIZARD_RADIUS + MAGIC_MISSILE_RADIUS - 0.11991199112787854,
        ),
        (
            Wizard(
                id=2,
                x=1200,
                y=1000,
                speed_x=None,
                speed_y=None,
                angle=None,
                faction=Faction.ACADEMY,
                radius=WIZARD_RADIUS,
                life=WIZARD_MAX_LIFE,
                max_life=WIZARD_MAX_LIFE,
                statuses=tuple(),
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
            ),
            Wizard(
                id=3,
                x=1200 + 3 * WIZARD_RADIUS,
                y=1000,
                speed_x=None,
                speed_y=None,
                angle=None,
                faction=Faction.RENEGADES,
                radius=WIZARD_RADIUS,
                life=WIZARD_MAX_LIFE,
                max_life=WIZARD_MAX_LIFE,
                statuses=tuple(),
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
            ),
            3,
            Point(854.15126507744606, 1305.9897523239772),
            WIZARD_CAST_RANGE + WIZARD_RADIUS + MAGIC_MISSILE_RADIUS - 0.11991199112787854,
        ),
    ]
)
def test_get_target_with_me_friend_and_enemy(friend, enemy, expected_target, expected_position, expected_distance):
    setattr(friend, 'position', Point(friend.x, friend.y))
    setattr(enemy, 'position', Point(enemy.x, enemy.y))
    target, position = get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=tuple(),
        wizards=[WIZARD, friend, enemy],
        trees=tuple(),
        projectiles=tuple(),
        bonuses=tuple(),
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        magic_missile_radius=MAGIC_MISSILE_RADIUS,
        dart_radius=DART_RADIUS,
        map_size=MAP_SIZE,
        shielded_direct_damage_absorption_factor=None,
    )
    assert_that(target.position.distance(position), close_to(expected_distance, 1e-3))
    assert target.position.distance(position) < WIZARD_CAST_RANGE + MAGIC_MISSILE_RADIUS + target.radius
    assert (target.id, position) == (expected_target, expected_position)
    assert not Circle(Point(friend.x, friend.y), friend.radius).has_intersection_with_moving_circle(
        Circle(position, MAGIC_MISSILE_RADIUS), Point(enemy.x, enemy.y))


@pytest.mark.parametrize(
    ('wizard_life', 'expected_distance'), [
        (WIZARD_MAX_LIFE, WIZARD_CAST_RANGE + GUARDIAN_TOWER_RADIUS + MAGIC_MISSILE_RADIUS - 0.12280701754332313),
        (3 * GUARDIAN_TOWER_DAMAGE, WIZARD_CAST_RANGE + GUARDIAN_TOWER_RADIUS + MAGIC_MISSILE_RADIUS - 0.12280701754332313),
        (2 * GUARDIAN_TOWER_DAMAGE, GUARDIAN_TOWER_ATTACK_RANGE + 2 * WIZARD_RADIUS + 13.1232749147795),
        (GUARDIAN_TOWER_DAMAGE, GUARDIAN_TOWER_ATTACK_RANGE + 2 * WIZARD_RADIUS + 13.1232749147795),
        (1, GUARDIAN_TOWER_ATTACK_RANGE + 2 * WIZARD_RADIUS + 13.1232749147795),
    ]
)
def test_get_target_with_me_and_tower(wizard_life, expected_distance):
    wizard = Wizard(
        id=1,
        x=1000,
        y=1000,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.ACADEMY,
        radius=WIZARD_RADIUS,
        life=wizard_life,
        max_life=WIZARD_MAX_LIFE,
        statuses=tuple(),
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
    tower = Building(
        id=2,
        x=wizard.x + WIZARD_RADIUS + GUARDIAN_TOWER_RADIUS,
        y=wizard.y + WIZARD_RADIUS + GUARDIAN_TOWER_RADIUS,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.RENEGADES,
        radius=GUARDIAN_TOWER_RADIUS,
        life=GUARDIAN_TOWER_LIFE,
        max_life=GUARDIAN_TOWER_LIFE,
        statuses=tuple(),
        type=None,
        vision_range=None,
        attack_range=GUARDIAN_TOWER_ATTACK_RANGE,
        damage=GUARDIAN_TOWER_DAMAGE,
        cooldown_ticks=None,
        remaining_action_cooldown_ticks=None,
    )
    setattr(tower, 'position', Point(tower.x, tower.y))
    target, position = get_target(
        me=wizard,
        buildings=[tower],
        minions=tuple(),
        wizards=[wizard],
        trees=tuple(),
        projectiles=tuple(),
        bonuses=tuple(),
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        magic_missile_radius=MAGIC_MISSILE_RADIUS,
        dart_radius=DART_RADIUS,
        map_size=MAP_SIZE,
        shielded_direct_damage_absorption_factor=None,
    )
    assert_that(target.position.distance(position), close_to(expected_distance, 1e-8))
    assert target.id == 2


@pytest.mark.parametrize(
    ('shielded', 'expected_distance'), [
        (False, GUARDIAN_TOWER_ATTACK_RANGE + 2 * WIZARD_RADIUS + 13.123274914779472),
        (True, WIZARD_CAST_RANGE + GUARDIAN_TOWER_RADIUS + MAGIC_MISSILE_RADIUS),
    ]
)
def test_get_target_with_me_and_tower_use_shielded(shielded, expected_distance):
    wizard = Wizard(
        id=1,
        x=1000,
        y=1000,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.ACADEMY,
        radius=WIZARD_RADIUS,
        life=1.9 * GUARDIAN_TOWER_DAMAGE,
        max_life=WIZARD_MAX_LIFE,
        statuses=[Status(id=None, type=StatusType.SHIELDED, wizard_id=None, player_id=None,
                         remaining_duration_ticks=shielded)],
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
    tower = Building(
        id=2,
        x=wizard.x + WIZARD_RADIUS + GUARDIAN_TOWER_RADIUS,
        y=wizard.y + WIZARD_RADIUS + GUARDIAN_TOWER_RADIUS,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.RENEGADES,
        radius=GUARDIAN_TOWER_RADIUS,
        life=GUARDIAN_TOWER_LIFE,
        max_life=GUARDIAN_TOWER_LIFE,
        statuses=None,
        type=None,
        vision_range=None,
        attack_range=GUARDIAN_TOWER_ATTACK_RANGE,
        damage=GUARDIAN_TOWER_DAMAGE,
        cooldown_ticks=None,
        remaining_action_cooldown_ticks=None,
    )
    setattr(tower, 'position', Point(tower.x, tower.y))
    target, position = get_target(
        me=wizard,
        buildings=[tower],
        minions=tuple(),
        wizards=[wizard],
        trees=tuple(),
        projectiles=tuple(),
        bonuses=tuple(),
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        magic_missile_radius=MAGIC_MISSILE_RADIUS,
        dart_radius=DART_RADIUS,
        map_size=MAP_SIZE,
        shielded_direct_damage_absorption_factor=SHIELDED_DIRECT_DAMAGE_ABSORPTION_FACTOR,
    )
    assert_that(target.position.distance(position), close_to(expected_distance, 0.51))
    assert target.id == 2
