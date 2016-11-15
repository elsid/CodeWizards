import pytest

from model.Faction import Faction
from model.Minion import Minion
from model.MinionType import MinionType
from model.Tree import Tree
from model.Wizard import Wizard

from strategy_common import Point
from strategy_target import get_target

from test.common import (
    FACTION_BASE_ATTACK_RANGE,
    FETISH_BLOWDART_ATTACK_RANGE,
    FETISH_BLOWDART_MAX_LIFE,
    GUARDIAN_TOWER_ATTACK_RANGE,
    MAGIC_MISSILE_DIRECT_DAMAGE,
    MINION_RADIUS,
    ORC_WOODCUTTER_ATTACK_RANGE,
    ORC_WOODCUTTER_DAMAGE,
    ORC_WOODCUTTER_LIFE,
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
        max_life=None,
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


@pytest.mark.parametrize(
    ('me', 'buildings', 'minions', 'wizards', 'expected_target', 'expected_position'), [
        (
            WIZARD,
            tuple(),
            tuple(),
            tuple(),
            None,
            None,
        ),
        (
            WIZARD,
            tuple(),
            tuple(),
            [WIZARD],
            None,
            None,
        ),
        (
            WIZARD,
            tuple(),
            [
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
                    max_life=None,
                    statuses=None,
                    type=MinionType.FETISH_BLOWDART,
                    vision_range=None,
                    damage=12,
                    cooldown_ticks=None,
                    remaining_action_cooldown_ticks=None,
                ),
            ],
            tuple(),
            2,
            Point(-112.13203496074424, -112.13203496074424),
        ),
        (
            WIZARD,
            tuple(),
            [
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
            tuple(),
            None,
            None,
            #Point(-112.13203496074424, -112.13203496074424),
        ),
        (
            WIZARD,
            tuple(),
            [
                Minion(
                    id=2,
                    x=100,
                    y=100,
                    speed_x=None,
                    speed_y=None,
                    angle=None,
                    faction=Faction.RENEGADES,
                    radius=MINION_RADIUS,
                    life=ORC_WOODCUTTER_LIFE,
                    max_life=None,
                    statuses=None,
                    type=MinionType.ORC_WOODCUTTER,
                    vision_range=None,
                    damage=ORC_WOODCUTTER_DAMAGE,
                    cooldown_ticks=None,
                    remaining_action_cooldown_ticks=None,
                ),
            ],
            tuple(),
            2,
            Point(-112.13203496074424, -112.13203496074424),
        ),
        (
            WIZARD,
            [
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
                )
            ],
            tuple(),
            tuple(),
            None,
            None,
        ),
        (
            WIZARD,
            tuple(),
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
                    max_life=None,
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
            Point(-324.26406987776716, -324.26406987776716),
        ),
        (
            WIZARD,
            tuple(),
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
                    max_life=None,
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
                    max_life=None,
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
            Point(0, 0),
        ),
        (
            WIZARD,
            tuple(),
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
                    max_life=None,
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
                    max_life=None,
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
            Point(0, 0),
        ),
    ]
)
def test_get_target(me, buildings, minions, wizards, expected_target, expected_position):
    target, position = get_target(
        me=me,
        buildings=buildings,
        minions=minions,
        wizards=wizards,
        guardian_tower_attack_range=GUARDIAN_TOWER_ATTACK_RANGE,
        faction_base_attack_range=FACTION_BASE_ATTACK_RANGE,
        orc_woodcutter_attack_range=ORC_WOODCUTTER_ATTACK_RANGE,
        fetish_blowdart_attack_range=FETISH_BLOWDART_ATTACK_RANGE,
        magic_missile_direct_damage=MAGIC_MISSILE_DIRECT_DAMAGE,
    )
    assert (target.id if target else None, position) == (expected_target, expected_position)