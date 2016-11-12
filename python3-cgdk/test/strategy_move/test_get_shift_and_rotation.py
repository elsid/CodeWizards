import pytest

from strategy_move import get_shift_and_rotation, Bounds, Point

from test.common import (
    World,
    Game,
    MAP_SIZE,
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
        (0, -WIZARD_BACKWARD_SPEED, 0, 0, Point(-WIZARD_BACKWARD_SPEED, 0), 0),
        (0, -WIZARD_BACKWARD_SPEED - 1, 0, 0, Point(-WIZARD_BACKWARD_SPEED, 0), 0),
        (0, 0, WIZARD_STRAFE_SPEED, 0, Point(0, WIZARD_STRAFE_SPEED), 0),
        (0, 0, WIZARD_STRAFE_SPEED + 1, 0, Point(0, WIZARD_STRAFE_SPEED), 0),
        (0, 0, -WIZARD_STRAFE_SPEED - 1, 0, Point(0, -WIZARD_STRAFE_SPEED), 0),
        (0, WIZARD_FORWARD_SPEED, WIZARD_STRAFE_SPEED, 0, Point(1, 1), 0),
    ]
)
def test_get_shift_and_rotation(angle, speed, strafe_speed, turn, expected_shift, expected_rotation):
    world = World(
        buildings=tuple(),
        minions=tuple(),
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
    bounds = Bounds(world=world, game=game)
    result = get_shift_and_rotation(angle=angle, bounds=bounds, speed=speed, strafe_speed=strafe_speed, turn=turn)
    assert result == (expected_shift, expected_rotation)
