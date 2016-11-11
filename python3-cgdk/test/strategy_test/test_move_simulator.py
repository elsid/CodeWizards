import pytest

from collections import namedtuple
from math import pi

from strategy_move import MoveSimulator, Point


World = namedtuple('World', tuple())
Game = namedtuple('Game', ('wizard_max_turn_angle',))

MAX_TURN_ANGLE = pi / 30
MAX_SPEED = MoveSimulator.MAX_SPEED
MAX_STRAFE_SPEED = MoveSimulator.MAX_STRAFE_SPEED


@pytest.mark.parametrize(
    ('angle', 'speed', 'strafe_speed', 'turn', 'expected_shift', 'expected_rotation'), [
        (0, 0, 0, 0, Point(0, 0), 0),
        (0, 0, 0, MAX_TURN_ANGLE, Point(0, 0), MAX_TURN_ANGLE),
        (1, 0, 0, 1, Point(0, 0), 0),
        (0, 1, 0, 0, Point(1, 0), 0),
        (0, 0, 1, 0, Point(0, 1), 0),
        (0, 1, 1, 0, Point(1, 1), 0),
        (0, MAX_SPEED, 0, 0, Point(MAX_SPEED, 0), 0),
        (0, 0, MAX_STRAFE_SPEED, 0, Point(0, MAX_STRAFE_SPEED), 0),
        (0, MAX_SPEED, MAX_STRAFE_SPEED, 0, Point(1, 1), 0),
        (0, MAX_SPEED + 1, MAX_STRAFE_SPEED + 1, 0, Point(1, 1), 0),
    ]
)
def test_move_simulator(angle, speed, strafe_speed, turn, expected_shift, expected_rotation):
    world = World()
    game = Game(wizard_max_turn_angle=MAX_TURN_ANGLE)
    simulator = MoveSimulator(angle=angle, world=world, game=game)
    result = simulator.apply(speed=speed, strafe_speed=strafe_speed, turn=turn)
    assert result == (expected_shift, expected_rotation)
