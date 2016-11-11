from math import sqrt
from numpy import sign
from collections import namedtuple
from scipy.optimize import minimize
from numpy import zeros

from model.Game import Game
from model.Wizard import Wizard
from model.World import World

from strategy_common import Point, normalize_angle
from strategy_release import Context, OPTIMIZE_MOVEMENT_STEPS

Movement = namedtuple('Movement', ('speed', 'strafe_speed', 'turn'))

PARAMETERS_COUNT = len(Movement(0, 0, 0))


def move_to(context: Context, target: Point):
    movement = next(optimize_movement(target=target, steps=OPTIMIZE_MOVEMENT_STEPS,
                                      player=context.me, world=context.world, game=context.game))
    context.move.speed = movement.speed
    context.move.strafe_speed = movement.strafe_speed
    context.move.turn = movement.turn


def optimize_movement(target: Point, steps: int, player: Wizard, world: World, game: Game):
    def function(values):
        position = Point(player.x, player.y)
        angle = normalize_angle(player.angle)
        for movement in iter_movements(values):
            simulator = MoveSimulator(angle, world, game)
            shift, rotation = simulator.apply(
                speed=movement.speed,
                strafe_speed=movement.strafe_speed,
                turn=movement.turn,
            )
            position += shift
            angle = normalize_angle(angle + rotation)
        direction = Point(1, 0).rotate(angle)
        target_direction = (target - position).normalized()
        return target.distance(position) * (1 + abs(target_direction.cos(direction)))
    minimized = minimize(function, zeros(PARAMETERS_COUNT * steps))
    function(minimized.x)
    return iter_movements(minimized.x)


def iter_movements(values):
    for v in chunks(values, PARAMETERS_COUNT):
        yield Movement(speed=v[0], strafe_speed=v[1], turn=v[2])


def chunks(values, size):
    for i in range(0, len(values), size):
        yield values[i:i + size]


class MoveSimulator:
    MAX_SPEED = 4.0
    MIN_SPEED = -3.0
    MAX_STRAFE_SPEED = 3.0
    MIN_STRAFE_SPEED = -3.0

    def __init__(self, angle, world: World, game: Game):
        self.angle = angle
        self.world = world
        self.game = game

    def apply(self, speed, strafe_speed, turn):
        # TODO: use HASTENED
        speed, strafe_speed = self.limit_speed(speed, strafe_speed)
        assert abs(speed) <= self.max_speed
        assert abs(strafe_speed) <= self.max_strafe_speed
        speed_direction = Point(1, 0).rotate(self.angle)
        strafe_speed_direction = Point(0, 1).rotate(self.angle)
        delta_turn = turn - self.angle
        return (
            speed_direction * speed + strafe_speed_direction * strafe_speed,
            sign(delta_turn) * min(abs(delta_turn), self.game.wizard_max_turn_angle),
        )

    def limit_speed(self, speed, strafe_speed):
        speed = min(self.max_speed, max(self.min_speed, speed))
        strafe_speed = min(self.max_strafe_speed, max(self.min_strafe_speed, strafe_speed))
        if sqrt((speed / self.max_speed) ** 2 + (strafe_speed / self.max_strafe_speed) ** 2) > 1.0:
            return speed / self.max_speed, strafe_speed / self.max_strafe_speed
        return speed, strafe_speed

    @property
    def max_speed(self):
        # TODO: use skills
        return self.MAX_SPEED

    @property
    def min_speed(self):
        # TODO: use skills
        return self.MIN_SPEED

    @property
    def max_strafe_speed(self):
        # TODO: use skills
        return self.MAX_STRAFE_SPEED

    @property
    def min_strafe_speed(self):
        # TODO: use skills
        return self.MIN_STRAFE_SPEED