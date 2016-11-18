import pytest

from model.Tree import Tree
from model.Minion import Minion

from strategy_move import optimize_movement, Point, State

from test.common import (
    CircularUnit,
    Game,
    World,
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
    world = World(
        buildings=tuple(),
        minions=tuple(),
        wizards=tuple(),
        trees=tuple(),
    )
    game = Game(
        map_size=MAP_SIZE,
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
    )
    states, movements = optimize_movement(
        target=target,
        look_target=target,
        circular_unit=circular_unit,
        world=world,
        game=game,
        step_size=3,
        max_barriers_range=1000,
    )
    assert states[-1] == State(
        position=Point(196.16652224137027, 196.16652224137027),
        angle=0.7853981633974483,
        path_length=135.99999999999972,
        intersection=False,
    )
    assert len(states) == 15
    assert len(movements) == 14


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
    world = World(
        buildings=tuple(),
        minions=tuple(),
        trees=[Tree(
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
        )],
        wizards=tuple(),
    )
    game = Game(
        map_size=MAP_SIZE,
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
    )
    states, movements = optimize_movement(
        target=target,
        look_target=target,
        circular_unit=circular_unit,
        world=world,
        game=game,
        step_size=3,
        max_barriers_range=1000,
    )
    assert states[-1] == State(
        position=Point(195.558304287408, 199.47628200043968),
        angle=0.15707963267948966,
        path_length=298.54142544811884,
        intersection=False,
    )
    assert len(states) == 31
    assert len(movements) == 30


@pytest.mark.parametrize(
    ('minion_position', 'minion_speed', 'expected_final_state', 'expected_states', 'expected_movements'), [
        (
            Point(103 + WIZARD_RADIUS + MINION_RADIUS, 103 + WIZARD_RADIUS + MINION_RADIUS),
            Point(4, 4),
            State(
                position=Point(196.16652224137027, 196.16652224137027),
                angle=0.7853981633974483,
                path_length=135.99999999999972,
                intersection=False,
            ),
            15,
            14,
        ),
        (
            Point(300 + WIZARD_RADIUS + MINION_RADIUS, 300 + WIZARD_RADIUS + MINION_RADIUS),
            Point(-3, -3),
            State(
                position=Point(194.31693442016044, 199.73516556514113),
                angle=0.15707963267948966,
                path_length=175.67204387668457,
                intersection=False,
            ),
            18,
            17,
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
    world = World(
        buildings=tuple(),
        minions=[minion],
        trees=tuple(),
        wizards=tuple(),
    )
    game = Game(
        map_size=MAP_SIZE,
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
    )
    states, movements = list(optimize_movement(
        target=target,
        look_target=target,
        circular_unit=circular_unit,
        world=world,
        game=game,
        step_size=3,
        max_barriers_range=1000,
    ))
    assert states[-1] == expected_final_state
    assert len(states) == expected_states
    assert len(movements) == expected_movements
