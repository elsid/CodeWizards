import pytest

from model.Minion import Minion
from model.Status import Status
from model.StatusType import StatusType
from model.Tree import Tree
from model.Wizard import Wizard

from strategy_move import optimize_movement, Point, State

from test.common import (
    HASTENED_MOVEMENT_BONUS_FACTOR,
    HASTENED_ROTATION_BONUS_FACTOR,
    HASTENED_DURATION_TICKS,
    MAP_SIZE,
    MINION_RADIUS,
    TREE_RADIUS,
    WIZARD_BACKWARD_SPEED,
    WIZARD_FORWARD_SPEED,
    WIZARD_MAX_TURN_ANGLE,
    WIZARD_RADIUS,
    WIZARD_STRAFE_SPEED,
)

POSITION = Point(100, 100)
TARGET = Point(300, 300)
WIZARD = Wizard(
    id=1,
    x=POSITION.x,
    y=POSITION.y,
    speed_x=None,
    speed_y=None,
    angle=(TARGET - POSITION).absolute_rotation(),
    faction=None,
    radius=WIZARD_RADIUS,
    life=None,
    max_life=None,
    statuses=tuple(),
    owner_player_id=None,
    me=None,
    mana=None,
    max_mana=None,
    vision_range=None,
    cast_range=None,
    xp=None,
    level=None,
    skills=None,
    remaining_action_cooldown_ticks=None,
    remaining_cooldown_ticks_by_action=None,
    master=None,
    messages=None,
)
setattr(WIZARD, 'position', Point(WIZARD.x, WIZARD.y))


def test_optimize_movement():
    set_position(WIZARD)
    states, movements = optimize_movement(
        target=TARGET,
        look_target=TARGET,
        me=WIZARD,
        buildings=tuple(),
        minions=tuple(),
        wizards=tuple(),
        trees=tuple(),
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
        map_size=MAP_SIZE,
        step_size=3,
        max_barriers_range=1000,
        hastened_movement_bonus_factor=HASTENED_MOVEMENT_BONUS_FACTOR,
        hastened_rotation_bonus_factor=HASTENED_ROTATION_BONUS_FACTOR,
    )
    assert states[-1] == State(
        position=Point(297.9898987322333, 297.9898987322333),
        angle=0.7853981633974483,
        path_length=280.0,
        intersection=False,
    )
    assert len(states) == 71
    assert len(movements) == 70


@pytest.mark.parametrize(
    ('remaining_duration_ticks', 'final_state', 'states_count', 'movements_count'), [
        (
            0,
            State(
                position=Point(297.9467179507658, 297.95151131235906),
                angle=0.7842295560522802,
                path_length=279.94940459077685,
                intersection=False,
            ),
            71,
            70,
        ),
        (
            10,
            State(
                position=Point(297.94239244847984, 297.94754578276445),
                angle=0.7841443302742515,
                path_length=279.9442650377665,
                intersection=False,
            ),
            68,
            67,
        ),
        (
            HASTENED_DURATION_TICKS,
            State(
                position=Point(298.50878670370963, 298.51252148811966),
                angle=0.7841443302742576,
                path_length=280.7442650377662,
                intersection=False,
            ),
            55,
            54,
        ),
    ]
)
def test_optimize_movement_with_hastened(remaining_duration_ticks, final_state, states_count, movements_count):
    wizard = Wizard(
        id=1,
        x=POSITION.x,
        y=POSITION.y,
        speed_x=None,
        speed_y=None,
        angle=1,
        faction=None,
        radius=WIZARD_RADIUS,
        life=None,
        max_life=None,
        statuses=[
            Status(
                id=None,
                type=StatusType.HASTENED,
                wizard_id=None,
                player_id=None,
                remaining_duration_ticks=remaining_duration_ticks,
            ),
        ],
        owner_player_id=None,
        me=None,
        mana=None,
        max_mana=None,
        vision_range=None,
        cast_range=None,
        xp=None,
        level=None,
        skills=None,
        remaining_action_cooldown_ticks=None,
        remaining_cooldown_ticks_by_action=None,
        master=None,
        messages=None,
    )
    setattr(wizard, 'position', Point(wizard.x, wizard.y))
    states, movements = optimize_movement(
        target=TARGET,
        look_target=TARGET,
        me=wizard,
        buildings=tuple(),
        minions=tuple(),
        wizards=tuple(),
        trees=tuple(),
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
        map_size=MAP_SIZE,
        step_size=3,
        max_barriers_range=1000,
        hastened_movement_bonus_factor=HASTENED_MOVEMENT_BONUS_FACTOR,
        hastened_rotation_bonus_factor=HASTENED_ROTATION_BONUS_FACTOR,
    )
    assert states[-1] == final_state
    assert len(states) == states_count
    assert len(movements) == movements_count


def test_optimize_movement_with_static_barriers():
    tree_position = POSITION + Point(WIZARD_RADIUS + TREE_RADIUS + 10, WIZARD_RADIUS + TREE_RADIUS + 10)
    trees = [
        Tree(
            id=None,
            x=tree_position.x,
            y=tree_position.y,
            speed_x=None,
            speed_y=None,
            angle=None,
            faction=None,
            radius=TREE_RADIUS,
            life=None,
            max_life=None,
            statuses=None,
        ),
    ]
    for tree in trees:
        set_position(tree)
    states, movements = optimize_movement(
        target=TARGET,
        look_target=TARGET,
        me=WIZARD,
        buildings=tuple(),
        minions=tuple(),
        wizards=tuple(),
        trees=trees,
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
        map_size=MAP_SIZE,
        step_size=3,
        max_barriers_range=1000,
        hastened_movement_bonus_factor=HASTENED_MOVEMENT_BONUS_FACTOR,
        hastened_rotation_bonus_factor=HASTENED_ROTATION_BONUS_FACTOR,
    )
    assert states[-1] == State(
        position=Point(294.3365054368041, 300.90120747414716),
        angle=-0.09412908325704437,
        path_length=319.0143625923556,
        intersection=False,
    )
    assert len(states) == 86
    assert len(movements) == 85


@pytest.mark.parametrize(
    ('minion_position', 'minion_speed', 'expected_final_state', 'expected_states', 'expected_movements'), [
        (
            Point(103 + WIZARD_RADIUS + MINION_RADIUS, 103 + WIZARD_RADIUS + MINION_RADIUS),
            Point(4, 4),
            State(
                position=Point(295.0692506415855, 300.880607246771),
                angle=-0.10210268782695242,
                path_length=324.7493954393034,
                intersection=False,
            ),
            89,
            88,
        ),
        (
            Point(103 + WIZARD_RADIUS + MINION_RADIUS, 103 + WIZARD_RADIUS + MINION_RADIUS),
            Point(-3, -3),
            State(
                position=Point(296.62224237933714, 300.8759213840577),
                angle=-0.12191806204855929,
                path_length=352.68614105414207,
                intersection=False,
            ),
            97,
            96,
        )
    ]
)
def test_optimize_movement_with_dynamic_barriers(minion_position, minion_speed,
                                                 expected_final_state, expected_states, expected_movements):
    set_position(WIZARD)
    minion = Minion(
        id=1,
        x=minion_position.x,
        y=minion_position.y,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=None,
        radius=MINION_RADIUS,
        life=None,
        max_life=None,
        statuses=None,
        type=None,
        vision_range=None,
        damage=None,
        cooldown_ticks=None,
        remaining_action_cooldown_ticks=None,
    )
    set_position(minion)
    setattr(minion, 'mean_speed', minion_speed)
    states, movements = list(optimize_movement(
        target=TARGET,
        look_target=TARGET,
        me=WIZARD,
        buildings=tuple(),
        minions=[minion],
        wizards=tuple(),
        trees=tuple(),
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
        map_size=MAP_SIZE,
        step_size=3,
        max_barriers_range=1000,
        hastened_movement_bonus_factor=HASTENED_MOVEMENT_BONUS_FACTOR,
        hastened_rotation_bonus_factor=HASTENED_ROTATION_BONUS_FACTOR,
    ))
    assert states[-1] == expected_final_state
    assert len(states) == expected_states
    assert len(movements) == expected_movements


def set_position(unit):
    setattr(unit, 'position', Point(unit.x, unit.y))
