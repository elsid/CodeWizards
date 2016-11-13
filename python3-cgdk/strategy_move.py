from collections import namedtuple
from heapq import heappop, heappush
from itertools import chain, product
from math import hypot

from model.CircularUnit import CircularUnit
from model.Game import Game
from model.World import World

from strategy_barriers import Circular, make_circular_barriers
from strategy_common import Point, normalize_angle

Movement = namedtuple('Movement', ('speed', 'strafe_speed', 'turn', 'step_size'))
State = namedtuple('State', ('position', 'angle', 'path_length', 'intersection'))

PARAMETERS_COUNT = 3


def optimize_movement(target: Point, look_target: Point, circular_unit: CircularUnit,
                      world: World, game: Game, step_sizes):
    bounds = Bounds(world=world, game=game)
    steps = sum(step_sizes)

    def is_in_range(unit):
        return (hypot(circular_unit.x - unit.x, circular_unit.y - unit.y) <=
                unit.radius + circular_unit.radius + steps * game.wizard_forward_speed)

    wizards = (v for v in world.wizards if v.id != circular_unit.id and is_in_range(v))
    minions = (v for v in world.minions if is_in_range(v))
    dynamic_units = {v.id: v for v in chain(wizards, minions)}
    initial_dynamic_units_positions = {v.id: Point(v.x, v.y) for v in dynamic_units.values()}
    static_barriers = list(chain(
        make_circular_barriers(v for v in world.buildings if is_in_range(v)),
        make_circular_barriers(v for v in world.trees if is_in_range(v)),
    ))
    initial_position = Point(circular_unit.x, circular_unit.y)
    initial_angle = normalize_angle(circular_unit.angle)
    initial_state = State(position=initial_position, angle=initial_angle, path_length=0, intersection=False)
    if initial_position.distance(target) < bounds.max_speed:
        speed_values = (0,)
        strafe_speed_values = (0,)
        turn_values = (bounds.min_turn, 0, bounds.max_turn)
    else:
        speed_values = (bounds.min_speed, 0, bounds.max_speed)
        strafe_speed_values = (bounds.min_strafe_speed, 0, bounds.max_strafe_speed)
        turn_values = (bounds.min_turn, 0, bounds.max_turn)
    branches = list()
    heappush(branches, (0, 0, 0, [initial_state], list(), initial_dynamic_units_positions))
    base_penalty = calculate_penalty(cur_state=initial_state, target=target, look_target=look_target, steps=1) * steps
    result = None
    result_penalty = None
    while branches:
        _, depth, sum_penalty, states, movements, dynamic_units_positions = heappop(branches)
        for speed, strafe_speed, turn in product(speed_values, strafe_speed_values, turn_values):
            cur_state = states[-1]
            new_movement = Movement(speed=speed, strafe_speed=strafe_speed, turn=turn, step_size=step_sizes[depth])

            def update_dynamic_units_positions():
                for k, v in dynamic_units_positions.items():
                    dynamic_unit = dynamic_units[k]
                    yield k, v + Point(dynamic_unit.speed_x, dynamic_unit.speed_y)

            new_dynamic_units = dict(update_dynamic_units_positions())

            def make_dynamic_barriers():
                for k, v in new_dynamic_units.items():
                    dynamic_unit = dynamic_units[k]
                    yield Circular(position=v, radius=dynamic_unit.radius)

            simulation = simulate_move(
                movements=[new_movement],
                state=cur_state,
                radius=circular_unit.radius,
                bounds=bounds,
                barriers=chain(static_barriers, make_dynamic_barriers()),
                map_size=game.map_size,
            )
            state = next(simulation)
            if state.intersection:
                continue
            new_depth = depth + 1
            penalty = calculate_penalty(cur_state=state, target=target, look_target=look_target,
                                        steps=sum(step_sizes[:new_depth]))
            new_sum_penalty = sum_penalty + penalty
            if new_sum_penalty > base_penalty:
                continue
            if result_penalty is not None and new_depth * penalty > result_penalty:
                continue
            new_states = states + [state]
            new_movements = movements + [new_movement]
            if new_depth < len(step_sizes) and state.position.distance(target) > bounds.max_speed:
                heappush(branches, (-new_depth, new_depth, new_sum_penalty,
                                    new_states, new_movements, new_dynamic_units))
                continue
            if result_penalty is None or result_penalty > penalty:
                result_penalty = penalty
                base_penalty = new_sum_penalty
                result = (new_states, new_movements)
    return result if result else (tuple([initial_state]), tuple())


def calculate_penalty(cur_state: State, target: Point, look_target: Point, steps: int):
    direction = Point(1, 0).rotate(cur_state.angle)
    target_direction = ((look_target - cur_state.position).normalized()
                        if look_target != cur_state.position else direction)
    return (
        cur_state.position.distance(target)
        * (1 + direction.distance(target_direction))
        - cur_state.path_length
        - cur_state.path_length / steps
    )


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


def simulate_move(movements, state: State, radius: float, bounds: Bounds, barriers, map_size: float):
    if state.intersection:
        return
    barrier = Circular(state.position, radius)
    position = state.position
    angle = state.angle
    path_length = state.path_length
    intersection = False
    for movement in movements:
        new_position = position
        new_angle = angle
        for _ in range(movement.step_size):
            shift, turn = get_shift_and_turn(
                angle=new_angle,
                bounds=bounds,
                speed=movement.speed,
                strafe_speed=movement.strafe_speed,
                turn=movement.turn,
            )
            new_position += shift
            new_angle = normalize_angle(new_angle + turn)
            barrier.position = new_position
            intersection = (
                has_intersection_with_borders(barrier, map_size) or
                has_intersection_with_barriers(barrier, barriers)
            )
            if intersection:
                break
        if not intersection:
            path_length += position.distance(new_position)
            position = new_position
            angle = new_angle
        yield State(position=position, angle=angle, path_length=path_length, intersection=intersection)


def has_intersection_with_borders(circular: Circular, map_size):
    delta = circular.radius * 0.1
    return (
        circular.position.x - circular.radius <= delta or
        circular.position.y - circular.radius <= delta or
        circular.position.x + circular.radius - map_size >= delta or
        circular.position.y + circular.radius - map_size >= delta
    )


def has_intersection_with_barriers(circular: Circular, barriers):
    return next((True for barrier in barriers if barrier.has_intersection_with_circular(circular, 1)), False)


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
