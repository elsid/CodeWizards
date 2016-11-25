import pytest

from hamcrest import assert_that, close_to

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
from strategy_target import get_optimal_position

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
setattr(WIZARD, 'position', Point(WIZARD.x, WIZARD.y))
setattr(WIZARD, 'mean_speed', Point(0, 0))


def test_get_optimal_position_with_only_me():
    assert WIZARD.position == get_optimal_position(
        target=None,
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
        empowered_damage_factor=None,
    )


@pytest.mark.parametrize(
    ('minion', 'expected_position', 'expected_distance'), [
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
                statuses=tuple(),
                type=MinionType.FETISH_BLOWDART,
                vision_range=None,
                damage=FETISH_BLOWDART_DAMAGE,
                cooldown_ticks=None,
                remaining_action_cooldown_ticks=None,
            ),
            Point(1092.9945268848903, 590.04811663605028),
            WIZARD_CAST_RANGE + MAGIC_MISSILE_RADIUS,
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
                statuses=tuple(),
                type=MinionType.ORC_WOODCUTTER,
                vision_range=None,
                damage=ORC_WOODCUTTER_DAMAGE,
                cooldown_ticks=None,
                remaining_action_cooldown_ticks=None,
            ),
            Point(1092.9945268848903, 590.04811663605028),
            WIZARD_CAST_RANGE + MAGIC_MISSILE_RADIUS,
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
                statuses=tuple(),
                type=MinionType.ORC_WOODCUTTER,
                vision_range=None,
                damage=ORC_WOODCUTTER_DAMAGE,
                cooldown_ticks=None,
                remaining_action_cooldown_ticks=None,
            ),
            Point(1640.6753050738948, 1638.0804459327037),
            WIZARD_CAST_RANGE + MAGIC_MISSILE_RADIUS,
        ),
    ]
)
def test_get_target_with_me_and_enemy_minion(minion, expected_position, expected_distance):
    setattr(minion, 'position', Point(minion.x, minion.y))
    position = get_optimal_position(
        target=minion,
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
        empowered_damage_factor=None,
    )
    assert_that(minion.position.distance(position), close_to(expected_distance, 1e-8))
    assert position == expected_position


def test_get_target_with_me_and_neural_minion():
    minion = Minion(
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
        statuses=tuple(),
        type=MinionType.FETISH_BLOWDART,
        vision_range=None,
        damage=None,
        cooldown_ticks=None,
        remaining_action_cooldown_ticks=None,
    )
    assert WIZARD.position == get_optimal_position(
        target=None,
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
        empowered_damage_factor=None,
    )


def test_get_target_with_enemy_wizard():
    wizard = Wizard(
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
        statuses=tuple(),
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
    )
    setattr(wizard, 'position', Point(wizard.x, wizard.y))
    position = get_optimal_position(
        target=wizard,
        me=WIZARD,
        buildings=tuple(),
        minions=tuple(),
        wizards=[wizard, WIZARD],
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
        empowered_damage_factor=None,
    )
    assert_that(wizard.position.distance(position), close_to(WIZARD_CAST_RANGE + MAGIC_MISSILE_RADIUS, 1e-8))
    assert position == Point(1092.9945268848903, 590.04811663605028)


@pytest.mark.parametrize(
    ('minion', 'wizard', 'expected_position', 'expected_distance'), [
        (
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
                statuses=tuple(),
                type=MinionType.ORC_WOODCUTTER,
                vision_range=None,
                damage=ORC_WOODCUTTER_DAMAGE,
                cooldown_ticks=None,
                remaining_action_cooldown_ticks=None,
            ),
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
                statuses=tuple(),
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
            Point(1170.8522014525097, 494.94558159605424),
            WIZARD_CAST_RANGE + MAGIC_MISSILE_RADIUS,
        ),
        (
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
                statuses=tuple(),
                type=MinionType.ORC_WOODCUTTER,
                vision_range=None,
                damage=ORC_WOODCUTTER_DAMAGE,
                cooldown_ticks=None,
                remaining_action_cooldown_ticks=None,
            ),
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
                statuses=tuple(),
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
            Point(1170.8522014525097, 494.94558159605424),
            WIZARD_CAST_RANGE + MAGIC_MISSILE_RADIUS,
        ),
    ]
)
def test_get_target_with_enemy_minion_and_wizard(minion, wizard, expected_position, expected_distance):
    for unit in (minion, wizard):
        setattr(unit, 'position', Point(unit.x, unit.y))
    position = get_optimal_position(
        target=wizard,
        me=WIZARD,
        buildings=tuple(),
        minions=[minion],
        wizards=[wizard, WIZARD],
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
        empowered_damage_factor=None,
    )
    assert_that(wizard.position.distance(position), close_to(expected_distance, 1e-8))
    assert position == expected_position


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
        statuses=tuple(),
    )
    setattr(tree, 'position', Point(tree.x, tree.y))
    assert tree.position == get_optimal_position(
        target=tree,
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
        empowered_damage_factor=None,
    )


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
    position = get_optimal_position(
        target=bonus,
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
        empowered_damage_factor=None,
    )
    assert bonus.position.distance(position) <= bonus.radius + WIZARD_RADIUS
    assert position == Point(1099.9999654385429, 1100.0000372327122)


@pytest.mark.parametrize(
    ('friend', 'enemy',  'expected_position', 'expected_distance'), [
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
            Point(1060.0827586918113, 709.56838847326571),
            WIZARD_CAST_RANGE + MAGIC_MISSILE_RADIUS,
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
            Point(904.45684079912985, 1315.6979214651112),
            WIZARD_CAST_RANGE + MAGIC_MISSILE_RADIUS,
        ),
    ]
)
def test_get_target_with_me_friend_and_enemy(friend, enemy, expected_position, expected_distance):
    setattr(friend, 'position', Point(friend.x, friend.y))
    setattr(enemy, 'position', Point(enemy.x, enemy.y))
    position = get_optimal_position(
        target=enemy,
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
        empowered_damage_factor=None,
    )
    assert_that(enemy.position.distance(position), close_to(expected_distance, 1e-3))
    assert enemy.position.distance(position) < WIZARD_CAST_RANGE + MAGIC_MISSILE_RADIUS + enemy.radius
    assert position == expected_position
    assert not Circle(Point(friend.x, friend.y), friend.radius).has_intersection_with_moving_circle(
        Circle(position, MAGIC_MISSILE_RADIUS), Point(enemy.x, enemy.y))


@pytest.mark.parametrize(
    ('wizard_life', 'expected_distance'), [
        (WIZARD_MAX_LIFE, WIZARD_CAST_RANGE + MAGIC_MISSILE_RADIUS),
        (3 * GUARDIAN_TOWER_DAMAGE, WIZARD_CAST_RANGE + MAGIC_MISSILE_RADIUS),
        (2 * GUARDIAN_TOWER_DAMAGE, GUARDIAN_TOWER_ATTACK_RANGE + 2 * WIZARD_RADIUS + 5.781083953264329),
        (GUARDIAN_TOWER_DAMAGE, GUARDIAN_TOWER_ATTACK_RANGE + 2 * WIZARD_RADIUS + 5.781083953264329),
        (1, GUARDIAN_TOWER_ATTACK_RANGE + 2 * WIZARD_RADIUS + 5.781083953264329),
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
    position = get_optimal_position(
        target=tower,
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
        empowered_damage_factor=None,
    )
    assert_that(tower.position.distance(position), close_to(expected_distance, 1e-8))


@pytest.mark.parametrize(
    ('shielded', 'expected_distance'), [
        (False, GUARDIAN_TOWER_ATTACK_RANGE + 2 * WIZARD_RADIUS + 5.781083953264329),
        (True, WIZARD_CAST_RANGE + GUARDIAN_TOWER_RADIUS + MAGIC_MISSILE_RADIUS - 9.110597747611678),
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
        statuses=tuple(),
        type=None,
        vision_range=None,
        attack_range=GUARDIAN_TOWER_ATTACK_RANGE,
        damage=GUARDIAN_TOWER_DAMAGE,
        cooldown_ticks=None,
        remaining_action_cooldown_ticks=None,
    )
    setattr(tower, 'position', Point(tower.x, tower.y))
    position = get_optimal_position(
        target=tower,
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
        empowered_damage_factor=None,
    )
    assert_that(tower.position.distance(position), close_to(expected_distance, 1e-8))
