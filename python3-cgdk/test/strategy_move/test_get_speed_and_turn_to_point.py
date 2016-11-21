import pytest

from hamcrest import assert_that, close_to
from math import pi, cos, sin

from strategy_move import get_speed_and_turn_to_point, Bounds, Point

from test.common import (
    HASTENED_MOVEMENT_BONUS_FACTOR,
    HASTENED_ROTATION_BONUS_FACTOR,
    WIZARD_BACKWARD_SPEED,
    WIZARD_FORWARD_SPEED,
    WIZARD_MAX_TURN_ANGLE,
    WIZARD_STRAFE_SPEED,
)


@pytest.mark.parametrize(
    ('position', 'angle', 'target', 'speed', 'strafe_speed', 'turn'), [
        (Point(0, 0), 0, Point(0, 0), 0, 0, 0),
        (Point(0, 0), 0, Point(1, 0), 1, 0, 0),
        (Point(0, 0), 0, Point(0, 1), 0, 1, WIZARD_MAX_TURN_ANGLE),
        (Point(0, 0), pi / 2, Point(1, 0), 0, -1, -WIZARD_MAX_TURN_ANGLE),
        (Point(0, 0), 0, Point(-1, 0), -1, 0, WIZARD_MAX_TURN_ANGLE),
        (Point(0, 0), pi / 2, Point(0, -1), -1, 0, -WIZARD_MAX_TURN_ANGLE),
        (Point(0, 0), 0, Point(1, 1), 1.1313708498984762, 0.848528137423857, WIZARD_MAX_TURN_ANGLE),
        (Point(0, 0), 0, Point(10, 0), WIZARD_FORWARD_SPEED, 0, 0),
        (Point(0, 0), 0, Point(0, 10), 0, WIZARD_STRAFE_SPEED, WIZARD_MAX_TURN_ANGLE),
        (Point(0, 0), pi / 2, Point(10, 0), 0, -WIZARD_STRAFE_SPEED, -WIZARD_MAX_TURN_ANGLE),
        (Point(0, 0), 0, Point(-10, 0), -WIZARD_BACKWARD_SPEED, 0, WIZARD_MAX_TURN_ANGLE),
        (Point(0, 0), pi / 2, Point(0, -10), -WIZARD_BACKWARD_SPEED, 0, -WIZARD_MAX_TURN_ANGLE),
        (Point(0, 0), 0, Point(10, 10), WIZARD_FORWARD_SPEED * cos(pi / 4), WIZARD_STRAFE_SPEED * sin(pi / 4),
         WIZARD_MAX_TURN_ANGLE),
    ]
)
def test_get_speed_and_turn_to_point(position, angle, target, speed, strafe_speed, turn):
    bounds = Bounds(
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
        hastened_ticks=0,
        hastened_movement_bonus_factor=None,
        hastened_rotation_bonus_factor=None,
    )
    result = get_speed_and_turn_to_point(position=position, angle=angle, target=target, bounds=bounds, tick=0)
    assert_that(result[0], close_to(speed, 1e-8))
    assert_that(result[1], close_to(strafe_speed, 1e-8))
    assert_that(result[2], close_to(turn, 1e-8))


@pytest.mark.parametrize(
    ('hastened_ticks', 'tick', 'speed', 'strafe_speed', 'turn'), [
        (0, 0, 2.8284271247461903, 2.1213203435596424, 0.10471975511965977),
        (1, 0, 3.6769552621700474, 2.7577164466275352, 0.15707963267948966),
    ]
)
def test_get_speed_and_turn_to_point_with_hastened(hastened_ticks, tick, speed, strafe_speed, turn):
    bounds = Bounds(
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
        hastened_ticks=hastened_ticks,
        hastened_movement_bonus_factor=HASTENED_MOVEMENT_BONUS_FACTOR,
        hastened_rotation_bonus_factor=HASTENED_ROTATION_BONUS_FACTOR,
    )
    result = get_speed_and_turn_to_point(
        position=Point(0, 0),
        angle=pi / 4,
        target=Point(0, 100),
        bounds=bounds,
        tick=tick,
    )
    assert_that(result[0], close_to(speed, 1e-8))
    assert_that(result[1], close_to(strafe_speed, 1e-8))
    assert_that(result[2], close_to(turn, 1e-8))
