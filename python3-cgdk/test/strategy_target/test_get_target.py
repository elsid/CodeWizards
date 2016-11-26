from model.Bonus import Bonus
from model.Building import Building
from model.Faction import Faction
from model.Minion import Minion
from model.MinionType import MinionType
from model.Tree import Tree
from model.Wizard import Wizard

from strategy_common import Point
from strategy_target import get_target

from test.common import (
    BONUS_RADIUS,
    FETISH_BLOWDART_DAMAGE,
    FETISH_BLOWDART_MAX_LIFE,
    GUARDIAN_TOWER_ATTACK_RANGE,
    GUARDIAN_TOWER_DAMAGE,
    GUARDIAN_TOWER_LIFE,
    GUARDIAN_TOWER_RADIUS,
    MAGIC_MISSILE_DIRECT_DAMAGE,
    MINION_RADIUS,
    ORC_WOODCUTTER_DAMAGE,
    ORC_WOODCUTTER_MAX_LIFE,
    STAFF_RANGE,
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
setattr(WIZARD, 'mean_speed', Point(0, 0))
setattr(WIZARD, 'position', Point(WIZARD.x, WIZARD.y))


def test_get_target_with_only_me():
    assert get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=tuple(),
        wizards=[WIZARD],
        trees=tuple(),
        bonuses=tuple(),
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        empowered_damage_factor=None,
        staff_range=STAFF_RANGE,
    ) is None


def test_get_target_with_me_and_enemy_minion():
    minion = Minion(
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
    )
    setattr(minion, 'position', Point(minion.x, minion.y))
    assert minion == get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=[minion],
        wizards=[WIZARD],
        trees=tuple(),
        bonuses=tuple(),
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        empowered_damage_factor=None,
        staff_range=STAFF_RANGE,
    )


def test_get_target_with_me_and_neutral_minion():
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
    setattr(minion, 'position', Point(minion.x, minion.y))
    assert get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=[minion],
        wizards=[WIZARD],
        trees=tuple(),
        bonuses=tuple(),
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        empowered_damage_factor=None,
        staff_range=STAFF_RANGE,
    ) is None


def test_get_target_with_enemy_minions_and_wizards():
    minion = Minion(
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
    )
    setattr(minion, 'position', Point(minion.x, minion.y))
    wizard = Wizard(
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
    )
    setattr(wizard, 'position', Point(wizard.x, wizard.y))
    assert wizard == get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=[minion],
        wizards=[wizard, WIZARD],
        trees=tuple(),
        bonuses=tuple(),
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        empowered_damage_factor=None,
        staff_range=STAFF_RANGE,
    )


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
        statuses=tuple(),
    )
    setattr(tree, 'position', Point(tree.x, tree.y))
    assert get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=tuple(),
        wizards=[WIZARD],
        trees=[tree],
        bonuses=tuple(),
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        empowered_damage_factor=None,
        staff_range=STAFF_RANGE,
    ) is None


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
    assert tree == get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=tuple(),
        wizards=[WIZARD],
        trees=[tree],
        bonuses=tuple(),
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        empowered_damage_factor=None,
        staff_range=STAFF_RANGE,
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
    assert bonus == get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=tuple(),
        wizards=[WIZARD],
        trees=tuple(),
        bonuses=[bonus],
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        empowered_damage_factor=None,
        staff_range=STAFF_RANGE,
    )


def test_get_target_with_me_friend_and_enemy():
    friend = Wizard(
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
    )
    setattr(friend, 'position', Point(friend.x, friend.y))
    enemy = Wizard(
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
    )
    setattr(enemy, 'position', Point(enemy.x, enemy.y))
    assert enemy == get_target(
        me=WIZARD,
        buildings=tuple(),
        minions=tuple(),
        wizards=[WIZARD, friend, enemy],
        trees=tuple(),
        bonuses=tuple(),
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        empowered_damage_factor=None,
        staff_range=STAFF_RANGE,
    )


def test_get_target_with_me_and_tower():
    wizard = Wizard(
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
    assert tower == get_target(
        me=wizard,
        buildings=[tower],
        minions=tuple(),
        wizards=[wizard],
        trees=tuple(),
        bonuses=tuple(),
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
        empowered_damage_factor=None,
        staff_range=STAFF_RANGE,
    )
