import pytest

from collections import namedtuple

from model.Faction import Faction
from model.Minion import Minion
from model.MinionType import MinionType
from model.Tree import Tree

from strategy_common import Point
from strategy_target import get_target

from test.common import (
    FACTION_BASE_ATTACK_RANGE,
    FETISH_BLOWDART_ATTACK_RANGE,
    GUARDIAN_TOWER_ATTACK_RANGE,
    MINION_RADIUS,
    ORC_WOODCUTTER_ATTACK_RANGE,
    WIZARD_CAST_RANGE,
    WIZARD_RADIUS,
)

Wizard = namedtuple('Wizard', (
    'radius',
    'x',
    'y',
    'faction',
    'cast_range',
))


@pytest.mark.parametrize(
    ('me', 'buildings', 'minions', 'wizards', 'expected_target', 'expected_position'), [
        (
            Wizard(x=0, y=0, radius=WIZARD_RADIUS, faction=Faction.ACADEMY, cast_range=WIZARD_CAST_RANGE),
            tuple(),
            tuple(),
            tuple(),
            None,
            None,
        ),
        (
            Wizard(x=0, y=0, radius=WIZARD_RADIUS, faction=Faction.ACADEMY, cast_range=WIZARD_CAST_RANGE),
            tuple(),
            [
                Minion(
                    id=1,
                    x=100,
                    y=100,
                    speed_x=None,
                    speed_y=None,
                    angle=None,
                    faction=Faction.RENEGADES,
                    radius=MINION_RADIUS,
                    life=None,
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
            1,
            Point(-115.66756876334426, -115.66756876334426),
        ),
(
            Wizard(x=0, y=0, radius=WIZARD_RADIUS, faction=Faction.ACADEMY, cast_range=WIZARD_CAST_RANGE),
            tuple(),
            [
                Minion(
                    id=1,
                    x=100,
                    y=100,
                    speed_x=None,
                    speed_y=None,
                    angle=None,
                    faction=Faction.NEUTRAL,
                    radius=MINION_RADIUS,
                    life=None,
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
            Wizard(x=0, y=0, radius=WIZARD_RADIUS, faction=Faction.ACADEMY, cast_range=WIZARD_CAST_RANGE),
            tuple(),
            [
                Minion(
                    id=1,
                    x=100,
                    y=100,
                    speed_x=None,
                    speed_y=None,
                    angle=None,
                    faction=Faction.RENEGADES,
                    radius=MINION_RADIUS,
                    life=None,
                    max_life=None,
                    statuses=None,
                    type=MinionType.ORC_WOODCUTTER,
                    vision_range=None,
                    damage=None,
                    cooldown_ticks=None,
                    remaining_action_cooldown_ticks=None,
                ),
            ],
            tuple(),
            1,
            Point(-112.13203496074424, -112.13203496074424),
        ),
        (
            Wizard(x=0, y=0, radius=WIZARD_RADIUS, faction=Faction.ACADEMY, cast_range=WIZARD_CAST_RANGE),
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
        wizard_cast_range=WIZARD_CAST_RANGE,
    )
    assert (target.id if target else None, position) == (expected_target, expected_position)
