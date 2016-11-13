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

Movement = namedtuple('Movement', ('speed', 'strafe_speed', 'turn', 'step_size'))
State = namedtuple('State', ('position', 'angle', 'intersection'))

PARAMETERS_COUNT = 3
DISTANCE_WEIGHT = 1.0
INTERSECTION_WEIGHT = 10000
SPEED_WEIGHT = 0.01
TURN_WEIGHT = 0.1


def optimize_movement(target: Point, circular_unit: CircularUnit, world: World, game: Game, step_sizes, iterations):
    bounds = Bounds(world=world, game=game)
    steps = sum(step_sizes)

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
    initial_position = Point(circular_unit.x, circular_unit.y)
    initial_angle = normalize_angle(circular_unit.angle)

    def function(values):
        simulation = simulate_move(
            movements=iter_movements(values, step_sizes),
            position=initial_position,
            angle=initial_angle,
            radius=circular_unit.radius,
            bounds=bounds,
            barriers=barriers,
            map_size=game.map_size,
        )
        last_state = State(position=initial_position, angle=initial_angle, intersection=False)
        intersections = 0
        for state in simulation:
            intersections += state.intersection
            last_state = state
        direction = Point(1, 0).rotate(last_state.angle)
        target_direction = (target - last_state.position).normalized()
        return (
            1
            * last_state.position.distance(target) * DISTANCE_WEIGHT
            * (1 + direction.distance(target_direction) * TURN_WEIGHT)
            + intersections * INTERSECTION_WEIGHT / max(1, initial_position.distance(last_state.position))
        )
    minimized = minimize(
        fun=function,
        x0=zeros(PARAMETERS_COUNT * steps),
        bounds=[(bounds.min_speed, bounds.max_speed),
                (bounds.min_strafe_speed, bounds.max_strafe_speed),
                (bounds.min_turn, bounds.max_turn)] * steps,
        options=dict(maxiter=iterations),
    )
    return iter_movements(minimized.x, step_sizes)


def iter_movements(values, step_sizes):
    for v, step_size in zip(chunks(values, PARAMETERS_COUNT), step_sizes):
        yield Movement(speed=v[0], strafe_speed=v[1], turn=v[2], step_size=step_size)


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


def simulate_move(movements, position: Point, angle: float, radius: float, bounds: Bounds, barriers, map_size: float):
    barrier = Circular(position, radius)
    for movement in movements:
        shift, turn = get_shift_and_turn(
            angle=angle,
            bounds=bounds,
            speed=movement.speed,
            strafe_speed=movement.strafe_speed,
            turn=movement.turn,
        )
        shift *= movement.step_size
        new_position = position + shift
        angle = normalize_angle(angle + turn * movement.step_size)
        barrier.position = new_position
        intersection = (
            has_intersection_with_borders(barrier, map_size) or
            has_intersection_with_barriers(barrier, barriers)
        )
        if not intersection:
            position = new_position
        yield State(position=position, angle=angle, intersection=intersection)


def has_intersection_with_borders(circular: Circular, map_size):
    delta = circular.radius * 0.1
    return (
        circular.position.x - circular.radius <= delta or
        circular.position.y - circular.radius <= delta or
        circular.position.x + circular.radius - map_size >= delta or
        circular.position.y + circular.radius - map_size >= delta
    )


def has_intersection_with_barriers(circular: Circular, barriers):
    return next((True for barrier in barriers
                 if barrier.has_intersection_with_circular(circular, circular.radius / 10)), False)


def get_shift_and_turn(angle: float, bounds: Bounds, speed: float, strafe_speed: float, turn: float):
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
