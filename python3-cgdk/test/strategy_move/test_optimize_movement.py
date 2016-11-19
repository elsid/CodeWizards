import pytest

from model.CircularUnit import CircularUnit
from model.Tree import Tree
from model.Minion import Minion

from strategy_move import optimize_movement, Point, State

from test.common import (
    MAP_SIZE,
    MINION_RADIUS,
    TREE_RADIUS,
    WIZARD_BACKWARD_SPEED,
    WIZARD_FORWARD_SPEED,
    WIZARD_MAX_TURN_ANGLE,
    WIZARD_RADIUS,
    WIZARD_STRAFE_SPEED,
)


def test_optimize_movement():
    position = Point(100, 100)
    target = Point(300, 300)
    angle = (target - position).absolute_rotation()
    circular_unit = CircularUnit(
        id=None,
        x=position.x,
        y=position.y,
        speed_x=None,
        speed_y=None,
        angle=angle,
        faction=None,
        radius=1,
    )
    set_position(circular_unit)
    states, movements = optimize_movement(
        target=target,
        look_target=target,
        circular_unit=circular_unit,
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
    )
    assert states[-1] == State(
        position=Point(297.9898987322333, 297.9898987322333),
        angle=0.7853981633974483,
        path_length=280.0,
        intersection=False,
    )
    assert len(states) == 71
    assert len(movements) == 70


def test_optimize_movement_with_static_barriers():
    position = Point(100, 100)
    target = Point(300, 300)
    angle = (target - position).absolute_rotation()
    circular_unit = CircularUnit(
        id=None,
        x=position.x,
        y=position.y,
        speed_x=None,
        speed_y=None,
        angle=angle,
        faction=None,
        radius=WIZARD_RADIUS,
    )
    set_position(circular_unit)
    tree_position = position + Point(WIZARD_RADIUS + TREE_RADIUS + 10, WIZARD_RADIUS + TREE_RADIUS + 10)
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
        target=target,
        look_target=target,
        circular_unit=circular_unit,
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
    position = Point(100, 100)
    target = Point(300, 300)
    angle = (target - position).absolute_rotation()
    circular_unit = CircularUnit(
        id=None,
        x=position.x,
        y=position.y,
        speed_x=None,
        speed_y=None,
        angle=angle,
        faction=None,
        radius=WIZARD_RADIUS,
    )
    set_position(circular_unit)
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
        target=target,
        look_target=target,
        circular_unit=circular_unit,
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
    ))
    assert states[-1] == expected_final_state
    assert len(states) == expected_states
    assert len(movements) == expected_movements


def set_position(unit):
    setattr(unit, 'position', Point(unit.x, unit.y))
