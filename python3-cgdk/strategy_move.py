from collections import namedtuple
from itertools import chain
from math import hypot
from numpy import zeros
from scipy.optimize import minimize

from model.CircularUnit import CircularUnit
from model.Game import Game
from model.World import World

from strategy_barriers import Circular, make_circular_barriers
from strategy_common import Point, normalize_angle

Movement = namedtuple('Movement', ('speed', 'strafe_speed', 'turn'))

PARAMETERS_COUNT = len(Movement(0, 0, 0))
DISTANCE_WEIGHT = 1.0
SPEED_WEIGHT = 0.01
TURN_WEIGHT = 0.1
OPTIMIZATION_ITERATIONS_COUNT = 5


def optimize_movement(target: Point, steps: int, circular_unit: CircularUnit, world: World, game: Game):
    bounds = Bounds(world=world, game=game)

    def is_in_range(unit):
        return (hypot(circular_unit.x - unit.x, circular_unit.y - unit.y) <=
                unit.radius + circular_unit.radius + steps * game.wizard_forward_speed)

    wizards = (v for v in world.wizards if v.id != circular_unit.id and is_in_range(v))
    buildings = (v for v in world.buildings if is_in_range(v))
    minions = (v for v in world.minions if is_in_range(v))
    trees = (v for v in world.trees if is_in_range(v))
    barriers = list(chain(
        make_circular_barriers(wizards),
        make_circular_barriers(buildings),
        make_circular_barriers(minions),
        make_circular_barriers(trees),
    ))

    def function(values):
        position, angle, speed = simulate_move(
            position=Point(circular_unit.x, circular_unit.y),
            angle=normalize_angle(circular_unit.angle),
            radius=circular_unit.radius,
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
        options=dict(maxiter=OPTIMIZATION_ITERATIONS_COUNT),
    )
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
        return -self.game.wizard_backward_speed

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
        shift, rotation = get_shift_and_rotation(
            angle=angle,
            bounds=bounds,
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
    return position, angle, path_distance / max(1, steps)


def has_intersection_with_barriers(circular: Circular, barriers):
    return next((True for barrier in barriers
                 if barrier.has_intersection_with_circular(circular, circular.radius / 10)), False)


def get_shift_and_rotation(angle: float, bounds: Bounds, speed: float, strafe_speed: float, turn: float):
    # TODO: use HASTENED
    speed, strafe_speed = limit_speed(speed, strafe_speed, bounds)
    turn = limit_turn(turn, bounds)
    speed_direction = Point(1, 0).rotate(angle)
    strafe_speed_direction = speed_direction.left_orthogonal()
    return speed_direction * speed + strafe_speed_direction * strafe_speed, turn


def limit_speed(speed: float, strafe_speed: float, bounds: Bounds):
    speed = min(bounds.max_speed, max(bounds.min_speed, speed))
    strafe_speed = min(bounds.max_strafe_speed, max(bounds.min_strafe_speed, strafe_speed))
    if hypot(speed / bounds.max_speed, strafe_speed / bounds.max_strafe_speed) > 1.0:
        return speed / bounds.max_speed, strafe_speed / bounds.max_strafe_speed
    return speed, strafe_speed


def limit_turn(value: float, bounds: Bounds):
    return min(bounds.max_turn, max(bounds.min_turn, value))
