from collections import namedtuple
from math import pi
from hamcrest import assert_that, close_to

from strategy_move import move_to, Point


Context = namedtuple('Context', ('me', 'world', 'game', 'move'))
Player = namedtuple('Player', ('x', 'y', 'angle'))
World = namedtuple('World', tuple())
Game = namedtuple('Game', ('wizard_max_turn_angle',))


class Move:
    def __init__(self, speed=None, strafe_speed=None, turn=None):
        self.speed = speed
        self.strafe_speed = strafe_speed
        self.turn = turn


def test_move_to():
    context = Context(
        me=Player(x=0, y=0, angle=0),
        world=World(),
        game=Game(wizard_max_turn_angle=pi / 30),
        move=Move(),
    )
    move_to(target=Point(100, 100), context=context)
    assert_that(context.move.speed, close_to(3.1694298639181482, delta=1e-8))
    assert_that(context.move.strafe_speed, close_to(2.1832921794339173, delta=1e-8))
    assert_that(context.move.turn, close_to(0.30908220448608181, delta=1e-8))
