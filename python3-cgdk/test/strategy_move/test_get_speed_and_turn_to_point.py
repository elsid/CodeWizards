import pytest

from hamcrest import assert_that, close_to
from math import pi, cos, sin

from strategy_move import get_speed_and_turn_to_point, Bounds, Point

from test.common import (
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
    )
    result = get_speed_and_turn_to_point(position=position, angle=angle, target=target, bounds=bounds)
    assert_that(result[0], close_to(speed, 1e-8))
    assert_that(result[1], close_to(strafe_speed, 1e-8))
    assert_that(result[2], close_to(turn, 1e-8))
