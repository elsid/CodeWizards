import pytest

from strategy_move import get_shift_and_turn, Bounds, Point

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
    ('angle', 'speed', 'strafe_speed', 'turn', 'expected_shift', 'expected_turn'), [
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
        (0, WIZARD_FORWARD_SPEED, WIZARD_STRAFE_SPEED, 0, Point(2.82842712474619, 2.1213203435596424), 0),
    ]
)
def test_get_shift_and_turn(angle, speed, strafe_speed, turn, expected_shift, expected_turn):
    bounds = Bounds(
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
    )
    result = get_shift_and_turn(angle=angle, bounds=bounds, speed=speed, strafe_speed=strafe_speed, turn=turn)
    assert result == (expected_shift, expected_turn)
