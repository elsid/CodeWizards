from math import sqrt
from collections import namedtuple
from scipy.optimize import minimize
from numpy import zeros
from itertools import chain

from model.Game import Game
from model.Wizard import Wizard
from model.World import World

from strategy_barriers import Circular, make_circular_barriers
from strategy_common import Point, normalize_angle

Movement = namedtuple('Movement', ('speed', 'strafe_speed', 'turn'))

PARAMETERS_COUNT = len(Movement(0, 0, 0))
DISTANCE_WEIGHT = 1.0
SPEED_WEIGHT = 0.01
TURN_WEIGHT = 0.1


def optimize_movement(target: Point, steps: int, player: Wizard, world: World, game: Game):
    bounds = Bounds(world=world, game=game)
    barriers = list(chain(
        make_circular_barriers(v for v in world.players if v != player),
        make_circular_barriers(world.buildings),
        make_circular_barriers(world.minions),
        make_circular_barriers(world.trees),
    ))

    def function(values):
        position, angle, speed = simulate_move(
            position=Point(player.x, player.y),
            angle=normalize_angle(player.angle),
            radius=player.radius,
            movements=iter_movements(values),
            bounds=bounds,
            barriers=barriers,
        )
        direction = Point(1, 0).rotate(angle)
        target_direction = (target - position).normalized()
        return (
            1
            * position.distance(target) * DISTANCE_WEIGHT
            * (1 + direction.distance(target_direction) * TURN_WEIGHT)
            / (1 + speed * SPEED_WEIGHT)
        )
    minimized = minimize(
        fun=function,
        x0=zeros(PARAMETERS_COUNT * steps),
        bounds=[(bounds.min_speed, bounds.max_speed),
                (bounds.min_strafe_speed, bounds.max_strafe_speed),
                (bounds.min_turn, bounds.max_turn)] * steps,
        options=dict(maxiter=5),
    )
    function(minimized.x)
    return iter_movements(minimized.x)


def iter_movements(values):
    for v in chunks(values, PARAMETERS_COUNT):
        yield Movement(speed=v[0], strafe_speed=v[1], turn=v[2])


def chunks(values, size: int):
    for i in range(0, len(values), size):
        yield values[i:i + size]


class Bounds:
    def __init__(self, world: World, game: Game):
        self.world = world
        self.game = game

    @property
    def max_speed(self):
        # TODO: use skills
        return self.game.wizard_forward_speed

    @property
    def min_speed(self):
        # TODO: use skills
        return self.game.wizard_backward_speed

    @property
    def max_strafe_speed(self):
        # TODO: use skills
        return self.game.wizard_strafe_speed

    @property
    def min_strafe_speed(self):
        # TODO: use skills
        return -self.game.wizard_strafe_speed

    @property
    def max_turn(self):
        # TODO: use skills
        return self.game.wizard_max_turn_angle

    @property
    def min_turn(self):
        # TODO: use skills
        return -self.game.wizard_max_turn_angle


def simulate_move(position: Point, angle: float, radius: float, movements, bounds: Bounds, barriers):
    path_distance = 0
    steps = 0
    for movement in movements:
        simulator = MoveSimulator(angle=angle, bounds=bounds)
        shift, rotation = simulator.apply(
            speed=movement.speed,
            strafe_speed=movement.strafe_speed,
            turn=movement.turn,
        )
        new_position = position + shift
        barrier = Circular(new_position, radius)
        if has_intersection_with_barriers(barrier, barriers):
            break
        position = new_position
        angle = normalize_angle(angle + rotation)
        path_distance += shift.norm()
        steps += 1
    return position, angle, path_distance / steps


def has_intersection_with_barriers(circular: Circular, barriers):
    return next((True for barrier in barriers if barrier.has_intersection_with_circular(circular)), False)


class MoveSimulator:
    def __init__(self, angle: float, bounds: Bounds):
        self.angle = angle
        self.bounds = bounds

    def apply(self, speed: float, strafe_speed: float, turn: float):
        # TODO: use HASTENED
        speed, strafe_speed = self.limit_speed(speed, strafe_speed)
        assert abs(speed) <= self.bounds.max_speed
        assert abs(strafe_speed) <= self.bounds.max_strafe_speed
        turn = self.limit_turn(turn)
        speed_direction = Point(1, 0).rotate(self.angle)
        strafe_speed_direction = Point(0, 1).rotate(self.angle)
        return speed_direction * speed + strafe_speed_direction * strafe_speed, turn

    def limit_speed(self, speed: float, strafe_speed: float):
        speed = min(self.bounds.max_speed, max(self.bounds.min_speed, speed))
        strafe_speed = min(self.bounds.max_strafe_speed, max(self.bounds.min_strafe_speed, strafe_speed))
        if sqrt((speed / self.bounds.max_speed) ** 2 + (strafe_speed / self.bounds.max_strafe_speed) ** 2) > 1.0:
            return speed / self.bounds.max_speed, strafe_speed / self.bounds.max_strafe_speed
        return speed, strafe_speed

    def limit_turn(self, value: float):
        return min(self.bounds.max_turn, max(self.bounds.min_turn, value))
