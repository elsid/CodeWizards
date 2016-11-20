from collections import namedtuple
from heapq import heappop, heappush
from itertools import chain
from math import hypot, sin, cos, pi
from time import time

from strategy_common import Point, Circle, normalize_angle

Movement = namedtuple('Movement', ('speed', 'strafe_speed', 'turn', 'step_size'))
State = namedtuple('State', ('position', 'angle', 'path_length', 'intersection'))

PARAMETERS_COUNT = 3


def optimize_movement(target, look_target, circular_unit, buildings, minions, wizards, trees,
                      wizard_forward_speed, wizard_backward_speed, wizard_strafe_speed, wizard_max_turn_angle,
                      map_size, step_size, max_barriers_range, max_time=None):

    def is_unit_in_range(unit):
        return circular_unit.position.distance(unit.position) <= max_barriers_range

    bounds = Bounds(
        wizard_forward_speed=wizard_forward_speed,
        wizard_backward_speed=wizard_backward_speed,
        wizard_strafe_speed=wizard_strafe_speed,
        wizard_max_turn_angle=wizard_max_turn_angle,
    )
    wizards = (v for v in wizards if v.id != circular_unit.id)
    dynamic_units = tuple(chain(wizards, minions))
    static_barriers = list(chain(
        make_circles(v for v in buildings if is_unit_in_range(v)),
        make_circles(v for v in trees if is_unit_in_range(v)),
    ))
    position = circular_unit.position
    angle = normalize_angle(circular_unit.angle)
    states = [State(position=position, angle=angle, path_length=0, intersection=False)]
    path = get_shortest_path(
        target=target,
        position=position,
        radius=circular_unit.radius,
        step_size=step_size,
        map_size=map_size,
        static_barriers=static_barriers,
        dynamic_units=dynamic_units,
        absolute_speed=wizard_strafe_speed,
        max_range=max_barriers_range,
        max_time=max_time,
    )
    if path is None:
        return tuple(states), tuple()
    movements = list()
    path_length = 0
    path = tuple(path)
    for path_position in reversed(path):
        while position.distance(path_position) > bounds.max_speed:
            speed, strafe_speed, turn = get_speed_and_turn_to_point(
                position=position,
                angle=angle,
                target=path_position,
                bounds=bounds,
            )
            if look_target:
                turn = limit_turn(normalize_angle((look_target - position).absolute_rotation() - angle), bounds)
            shift, turn = get_shift_and_turn(
                angle=angle,
                bounds=bounds,
                speed=speed,
                strafe_speed=strafe_speed,
                turn=turn,
            )
            position += shift
            angle = normalize_angle(angle + turn)
            path_length += shift.norm()
            states.append(State(
                position=position,
                angle=angle,
                path_length=path_length,
                intersection=False,
            ))
            movements.append(Movement(speed=speed, strafe_speed=strafe_speed, turn=turn, step_size=1))
    return states, movements


def get_shortest_path(target, position, radius, step_size, map_size, static_barriers, dynamic_units, absolute_speed,
                      max_range, max_time=None):
    start = time()
    shifts = (
        Point(step_size, 0),
        Point(step_size, step_size),
        Point(0, step_size),
        Point(-step_size, 0),
        Point(-step_size, -step_size),
        Point(0, -step_size),
        Point(step_size, -step_size),
        Point(-step_size, step_size),
    )
    initial_position = position
    max_range = max(max_range, target.distance(position) / 2 + step_size)
    barrier = Circle(position, radius + 1)
    initial_dynamic_barriers = {v.id: Circle(v.position, v.radius) for v in dynamic_units}
    static_occupier = next((v for v in chain(static_barriers)
                           if target.distance(v.position) < v.radius + barrier.radius), None)
    if static_occupier:
        occupier = static_occupier
    else:
        occupier = next((v for v in chain(initial_dynamic_barriers.values())
                         if target.distance(v.position) < v.radius + barrier.radius), None)
    max_distance_errors = [occupier.radius + barrier.radius + step_size if occupier else step_size]
    dynamic_barriers = [initial_dynamic_barriers]
    closed = set()
    opened = {position}
    queue = [(target.distance(position), 0, position)]
    came_from = dict()
    lengths = {position: 0}
    result = None
    while queue:
        distance, depth, position = heappop(queue)
        if max_time is not None and time() - start > max_time:
            result = None
            break
        if distance <= max_distance_errors[depth]:
            result = position
            break
        opened.remove(position)
        closed.add(position)
        barrier.position = position
        for shift in chain((target - position,), shifts):
            new_position = position + shift
            if new_position in closed:
                continue
            new_distance = target.distance(new_position)
            length = shift.norm()
            new_length = lengths[position] + length
            if new_position not in opened:
                if depth >= len(dynamic_barriers):
                    ticks = length / absolute_speed
                    barriers = dict()
                    for unit in dynamic_units:
                        unit_barrier = dynamic_barriers[depth - 1][unit.id]
                        barriers[unit.id] = Circle(unit_barrier.position + unit.mean_speed.normalized() * ticks,
                                                   unit_barrier.radius)
                    dynamic_barriers.append(barriers)
                else:
                    barriers = dynamic_barriers[depth]
                if depth + 1 >= len(max_distance_errors):
                    if static_occupier:
                        max_distance_errors.append(max_distance_errors[-1])
                    else:
                        occupier = next((v for v in barriers.values()
                                         if target.distance(v.position) < v.radius + barrier.radius), None)
                        max_distance_errors.append(occupier.radius + barrier.radius + step_size
                                                   if occupier else step_size)
                intersection = (
                    (position.distance(initial_position) > max_range and
                     position.distance(target) > max_range) or
                    has_intersection_with_borders(barrier, map_size) or
                    has_intersection_with_barriers(barrier, new_position, chain(static_barriers, barriers.values()))
                )
                heappush(queue, (float('inf') if intersection else new_distance, depth + 1, new_position))
                opened.add(new_position)
            elif new_length >= lengths[new_position]:
                continue
            came_from[new_position] = position
            lengths[new_position] = new_length
    if result is None:
        return None
    return reconstruct_path(came_from, result)


def reconstruct_path(came_from, position):
    yield position
    while position in came_from:
        position = came_from[position]
        yield position


def calculate_priority(state: State, target: Point, look_target: Point):
    direction = Point(1, 0).rotate(state.angle)
    target_direction = ((look_target - state.position).normalized()
                        if look_target != state.position else direction)
    return (
        target.distance(state.position)
        * (1 + direction.distance(target_direction))
    )


def iter_movements(values, step_sizes):
    for v, step_size in zip(chunks(values, PARAMETERS_COUNT), step_sizes):
        yield Movement(speed=v[0], strafe_speed=v[1], turn=v[2], step_size=step_size)


def chunks(values, size: int):
    for i in range(0, len(values), size):
        yield values[i:i + size]


class Bounds:
    def __init__(self, wizard_forward_speed, wizard_backward_speed, wizard_strafe_speed, wizard_max_turn_angle):
        self.wizard_forward_speed = wizard_forward_speed
        self.wizard_backward_speed = wizard_backward_speed
        self.wizard_strafe_speed = wizard_strafe_speed
        self.wizard_max_turn_angle = wizard_max_turn_angle

    @property
    def max_speed(self):
        # TODO: use skills
        return self.wizard_forward_speed

    @property
    def min_speed(self):
        # TODO: use skills
        return -self.wizard_backward_speed

    @property
    def max_strafe_speed(self):
        # TODO: use skills
        return self.wizard_strafe_speed

    @property
    def min_strafe_speed(self):
        # TODO: use skills
        return -self.wizard_strafe_speed

    @property
    def max_turn(self):
        # TODO: use skills
        return self.wizard_max_turn_angle

    @property
    def min_turn(self):
        # TODO: use skills
        return -self.wizard_max_turn_angle


def simulate_move(movements, state: State, radius: float, bounds: Bounds, barriers, map_size: float):
    if state.intersection:
        return
    barrier = Circle(state.position, radius)
    position = state.position
    angle = state.angle
    path_length = state.path_length
    for movement in movements:
        new_position = position
        new_angle = angle
        barrier.position = new_position
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
        intersection = (
            has_intersection_with_borders(barrier, map_size) or
            has_intersection_with_barriers(barrier, new_position, barriers)
        )
        if not intersection:
            path_length += position.distance(new_position)
            position = new_position
            angle = new_angle
        yield State(position=position, angle=angle, path_length=path_length, intersection=intersection)


def has_intersection_with_borders(circle: Circle, map_size):
    delta = circle.radius * 0.1
    return (
        circle.position.x - circle.radius <= delta or
        circle.position.y - circle.radius <= delta or
        circle.position.x + circle.radius - map_size >= delta or
        circle.position.y + circle.radius - map_size >= delta
    )


def has_intersection_with_barriers(circle: Circle, next_position: Point, barriers):
    return next((True for barrier in barriers
                 if barrier.has_intersection_with_moving_circle(circle, next_position)), False)


def get_shift_and_turn(angle: float, bounds: Bounds, speed: float, strafe_speed: float, turn: float):
    speed, strafe_speed = limit_speed(speed, strafe_speed, bounds)
    turn = limit_turn(turn, bounds)
    speed_direction = Point(1, 0).rotate(angle)
    strafe_speed_direction = speed_direction.left_orthogonal()
    return speed_direction * speed + strafe_speed_direction * strafe_speed, turn


def limit_speed(speed: float, strafe_speed: float, bounds: Bounds):
    speed = min(bounds.max_speed, max(bounds.min_speed, speed))
    strafe_speed = min(bounds.max_strafe_speed, max(bounds.min_strafe_speed, strafe_speed))
    both = hypot(speed / bounds.max_speed, strafe_speed / bounds.max_strafe_speed)
    return (speed / both, strafe_speed / both) if both > 1.0 else (speed, strafe_speed)


def limit_turn(value: float, bounds: Bounds):
    return min(bounds.max_turn, max(bounds.min_turn, value))


def make_circles(values):
    for value in values:
        yield Circle(position=value.position, radius=value.radius)


def get_speed_and_turn_to_point(position: Point, angle, target: Point, bounds: Bounds):
    direction = target - position
    norm = direction.norm()
    if norm == 0:
        return 0, 0, 0
    turn = normalize_angle(direction.absolute_rotation() - angle)
    speed = bounds.max_speed * cos(turn) if abs(turn) <= pi / 2 else -bounds.min_speed * cos(turn)
    strafe_speed = bounds.max_strafe_speed * sin(turn)
    ratio = min(1, direction.norm() / hypot(speed, strafe_speed))
    return speed * ratio, strafe_speed * ratio, limit_turn(turn, bounds)


def make_dynamic_units_barriers(positions, units):
    for k, v in positions.items():
        dynamic_unit = units[k]
        yield Circle(position=v, radius=dynamic_unit.radius)
