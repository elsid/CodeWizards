import pytest

from strategy_move import MoveSimulator, Bounds, Point

from .common import (
    World,
    Game,
    WIZARD_BACKWARD_SPEED,
    WIZARD_FORWARD_SPEED,
    WIZARD_MAX_TURN_ANGLE,
    WIZARD_STRAFE_SPEED,
)


@pytest.mark.parametrize(
    ('angle', 'speed', 'strafe_speed', 'turn', 'expected_shift', 'expected_rotation'), [
        (0, 0, 0, 0, Point(0, 0), 0),
        (0, 0, 0, WIZARD_MAX_TURN_ANGLE, Point(0, 0), WIZARD_MAX_TURN_ANGLE),
        (0, 0, 0, 1, Point(0, 0), WIZARD_MAX_TURN_ANGLE),
        (0, 1, 0, 0, Point(1, 0), 0),
        (0, 0, 1, 0, Point(0, 1), 0),
        (0, 1, 1, 0, Point(1, 1), 0),
        (0, WIZARD_FORWARD_SPEED, 0, 0, Point(WIZARD_FORWARD_SPEED, 0), 0),
        (0, WIZARD_FORWARD_SPEED + 1, 0, 0, Point(WIZARD_FORWARD_SPEED, 0), 0),
        (0, WIZARD_BACKWARD_SPEED, 0, 0, Point(WIZARD_BACKWARD_SPEED, 0), 0),
        (0, WIZARD_BACKWARD_SPEED - 1, 0, 0, Point(WIZARD_BACKWARD_SPEED, 0), 0),
        (0, 0, WIZARD_STRAFE_SPEED, 0, Point(0, WIZARD_STRAFE_SPEED), 0),
        (0, 0, WIZARD_STRAFE_SPEED + 1, 0, Point(0, WIZARD_STRAFE_SPEED), 0),
        (0, 0, -WIZARD_STRAFE_SPEED - 1, 0, Point(0, -WIZARD_STRAFE_SPEED), 0),
        (0, WIZARD_FORWARD_SPEED, WIZARD_STRAFE_SPEED, 0, Point(1, 1), 0),
    ]
)
def test_move_simulator(angle, speed, strafe_speed, turn, expected_shift, expected_rotation):
    world = World(
        buildings=tuple(),
        minions=tuple(),
        players=tuple(),
        trees=tuple(),
    )
    game = Game(
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
    )
    bounds = Bounds(world=world, game=game)
    simulator = MoveSimulator(angle=angle, bounds=bounds)
    result = simulator.apply(speed=speed, strafe_speed=strafe_speed, turn=turn)
    assert result == (expected_shift, expected_rotation)
