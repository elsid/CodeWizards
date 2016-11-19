import pytest

from model.Tree import Tree
from model.Minion import Minion

from strategy_move import optimize_movement, Point, State

from test.common import (
    CircularUnit,
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
    target = Point(200, 200)
    angle = (target - position).absolute_rotation()
    circular_unit = CircularUnit(
        x=position.x,
        y=position.y,
        angle=angle,
        radius=1,
    )
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
        position=Point(198.99494936611646, 198.99494936611646),
        angle=0.7853981633974483,
        path_length=139.99999999999966,
        intersection=False,
    )
    assert len(states) == 36
    assert len(movements) == 35


def test_optimize_movement_with_static_barriers():
    position = Point(100, 100)
    target = Point(200, 200)
    angle = (target - position).absolute_rotation()
    circular_unit = CircularUnit(
        x=position.x,
        y=position.y,
        angle=angle,
        radius=WIZARD_RADIUS,
    )
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
        position=Point(199.83572053947486, 198.99523892373728),
        angle=1.4087293105344507,
        path_length=75.99997882392724,
        intersection=False,
    )
    assert len(states) == 37
    assert len(movements) == 36


@pytest.mark.parametrize(
    ('minion_position', 'minion_speed', 'expected_final_state', 'expected_states', 'expected_movements'), [
        (
            Point(103 + WIZARD_RADIUS + MINION_RADIUS, 103 + WIZARD_RADIUS + MINION_RADIUS),
            Point(4, 4),
            State(
                position=Point(198.99494936611646, 198.99494936611646),
                angle=0.7853981633974483,
                path_length=91.99999999999979,
                intersection=False,
            ),
            28,
            27,
        ),
        (
            Point(300 + WIZARD_RADIUS + MINION_RADIUS, 300 + WIZARD_RADIUS + MINION_RADIUS),
            Point(-3, -3),
            State(
                position=Point(198.99494936611646, 198.99494936611646),
                angle=0.7853981633974483,
                path_length=139.99999999999966,
                intersection=False,
            ),
            36,
            35,
        )
    ]
)
def test_optimize_movement_with_dynamic_barriers(minion_position, minion_speed,
                                                 expected_final_state, expected_states, expected_movements):
    position = Point(100, 100)
    target = Point(200, 200)
    angle = (target - position).absolute_rotation()
    circular_unit = CircularUnit(
        x=position.x,
        y=position.y,
        angle=angle,
        radius=WIZARD_RADIUS,
    )
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
