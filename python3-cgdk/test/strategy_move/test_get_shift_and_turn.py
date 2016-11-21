import pytest

from math import pi

from strategy_move import get_shift_and_turn, Bounds, Point

from test.common import (
    HASTENED_MOVEMENT_BONUS_FACTOR,
    HASTENED_ROTATION_BONUS_FACTOR,
    WIZARD_BACKWARD_SPEED,
    WIZARD_FORWARD_SPEED,
    WIZARD_MAX_TURN_ANGLE,
    WIZARD_STRAFE_SPEED,
)


@pytest.mark.parametrize(
    ('angle', 'speed', 'strafe_speed', 'turn', 'expected_shift', 'expected_turn'), [
        (0, 0, 0, 0, Point(0, 0), 0),
        (0, 0, 0, WIZARD_MAX_TURN_ANGLE, Point(0, 0), WIZARD_MAX_TURN_ANGLE),
        (0, 0, 0, WIZARD_MAX_TURN_ANGLE + 1, Point(0, 0), WIZARD_MAX_TURN_ANGLE),
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
        hastened_ticks=0,
        hastened_movement_bonus_factor=None,
        hastened_rotation_bonus_factor=None,
    )
    result = get_shift_and_turn(angle=angle, bounds=bounds, speed=speed, strafe_speed=strafe_speed, turn=turn, tick=0)
    assert result == (expected_shift, expected_turn)


@pytest.mark.parametrize(
    ('hastened_ticks', 'tick', 'expected_shift', 'expected_turn'), [
        (0, 0, Point(0.5000000000000002, 3.4999999999999996), 0.10471975511965977),
        (1, 0, Point(0.6500000000000004, 4.55), 0.15707963267948966),
    ]
)
def test_get_shift_and_turn_with_hastened(hastened_ticks, tick, expected_shift, expected_turn):
    bounds = Bounds(
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
        hastened_ticks=hastened_ticks,
        hastened_movement_bonus_factor=HASTENED_MOVEMENT_BONUS_FACTOR,
        hastened_rotation_bonus_factor=HASTENED_ROTATION_BONUS_FACTOR,
    )
    result = get_shift_and_turn(
        angle=pi / 4,
        bounds=bounds,
        speed=10,
        strafe_speed=10,
        turn=1,
        tick=tick,
    )
    assert result == (expected_shift, expected_turn)
