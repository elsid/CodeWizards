from collections import namedtuple
from heapq import heappop, heappush
from itertools import chain
from math import hypot, sin, cos, pi
from time import time

from model.StatusType import StatusType

from strategy_common import Point, Circle, normalize_angle

Movement = namedtuple('Movement', ('speed', 'strafe_speed', 'turn', 'step_size'))
State = namedtuple('State', ('position', 'angle', 'path_length'))

PARAMETERS_COUNT = 3


def optimize_movement(target, look_target, me, buildings, minions, wizards, trees,
                      wizard_forward_speed, wizard_backward_speed, wizard_strafe_speed, wizard_max_turn_angle,
                      map_size, step_size, max_barriers_range, hastened_movement_bonus_factor,
                      hastened_rotation_bonus_factor, max_time=None):

    def is_unit_in_range(unit):
        return me.position.distance(unit.position) <= max_barriers_range

    bounds = Bounds(
        wizard_forward_speed=wizard_forward_speed,
        wizard_backward_speed=wizard_backward_speed,
        wizard_strafe_speed=wizard_strafe_speed,
        wizard_max_turn_angle=wizard_max_turn_angle,
        hastened_ticks=next((v.remaining_duration_ticks for v in me.statuses if v.type == StatusType.HASTENED), 0),
        hastened_movement_bonus_factor=hastened_movement_bonus_factor,
        hastened_rotation_bonus_factor=hastened_rotation_bonus_factor,
    )
    wizards = (v for v in wizards if v.id != me.id)
    dynamic_units = tuple(chain(wizards, minions))
    static_barriers = list(chain(
        make_circles(v for v in buildings if is_unit_in_range(v)),
        make_circles(v for v in trees if is_unit_in_range(v)),
    ))
    cur_position = me.position
    cur_angle = normalize_angle(me.angle)
    states = [State(position=cur_position, angle=cur_angle, path_length=0)]
    path = get_shortest_path(
        target=target,
        position=cur_position,
        radius=me.radius,
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
    cur_path_length = 0
    path = tuple(path)
    cur_tick = 0

    def add_movement(position, angle, path_length, tick, path_position):
        speed, strafe_speed, turn = get_speed_and_turn_to_point(
            position=position,
            angle=angle,
            target=path_position,
            bounds=bounds,
            tick=tick,
        )
        if look_target:
            turn = limit_turn(normalize_angle((look_target - position).absolute_rotation() - angle), bounds, tick)
        shift, turn = get_shift_and_turn(
            angle=angle,
            bounds=bounds,
            speed=speed,
            strafe_speed=strafe_speed,
            turn=turn,
            tick=tick,
        )
        position += shift
        angle = normalize_angle(angle + turn)
        path_length += shift.norm()
        states.append(State(position=position,angle=angle, path_length=path_length))
        movements.append(Movement(speed=speed, strafe_speed=strafe_speed, turn=turn, step_size=1))
        tick += 1
        return position, angle, path_length, tick

    for cur_path_position in reversed(path):
        while cur_position.distance(cur_path_position) > bounds.max_speed(cur_tick):
            cur_position, cur_angle, cur_path_length, cur_tick = add_movement(cur_position, cur_angle, cur_path_length,
                                                                              cur_tick, cur_path_position)
    add_movement(cur_position, cur_angle, cur_path_length, cur_tick, path[0])
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
    max_distance_errors = {0: occupier.radius + barrier.radius + step_size if occupier else step_size}
    occupiers = {0: occupier}
    dynamic_barriers = {0: initial_dynamic_barriers}
    closed = set()
    opened = {position}
    queue = [(target.distance(position), 0, position)]
    came_from = dict()
    lengths = {position: 0}
    result = None
    while queue:
        distance, ticks, position = heappop(queue)
        if max_time is not None and time() - start > max_time:
            result = None
            break
        if distance <= max_distance_errors[ticks]:
            if occupiers[ticks] is None and target != position and target not in came_from and position in came_from:
                came_from[target] = came_from[position]
                result = target
            else:
                result = position
            break
        opened.remove(position)
        closed.add(position)
        barrier.position = position
        for i, shift in enumerate(chain((target - position,), shifts)):
            new_position = position + shift
            if i != 0 and new_position in closed:
                continue
            new_distance = target.distance(new_position)
            length = shift.norm()
            new_ticks = ticks + length / absolute_speed
            if ticks not in dynamic_barriers:
                barriers = dict()
                for unit in dynamic_units:
                    unit_barrier = dynamic_barriers[0][unit.id]
                    barriers[unit.id] = Circle(unit_barrier.position + unit.mean_speed.normalized() * ticks,
                                               unit_barrier.radius)
                dynamic_barriers[ticks] = barriers
            else:
                barriers = dynamic_barriers[ticks]
            if new_ticks not in max_distance_errors:
                if static_occupier:
                    max_distance_errors[new_ticks] = max_distance_errors[ticks]
                    occupiers[new_ticks] = static_occupier
                else:
                    occupier = next((v for v in barriers.values()
                                     if target.distance(v.position) < v.radius + barrier.radius), None)
                    max_distance_errors[new_ticks] = (occupier.radius + barrier.radius + step_size
                                                      if occupier else step_size)
                    occupiers[new_ticks] = occupier
            intersection = (
                (position.distance(initial_position) > max_range and
                 position.distance(target) > max_range) or
                has_intersection_with_borders(barrier, map_size) or
                has_intersection_with_barriers(barrier, new_position, chain(static_barriers, barriers.values()))
            )
            new_length = float('inf') if intersection else lengths[position] + length
            if new_position not in opened:
                if not intersection or position.distance(initial_position) > radius:
                    heappush(queue, (new_distance, new_ticks, new_position))
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


class Bounds:
    def __init__(self, wizard_forward_speed, wizard_backward_speed, wizard_strafe_speed, wizard_max_turn_angle,
                 hastened_ticks, hastened_movement_bonus_factor, hastened_rotation_bonus_factor):
        self.wizard_forward_speed = wizard_forward_speed
        self.wizard_backward_speed = wizard_backward_speed
        self.wizard_strafe_speed = wizard_strafe_speed
        self.wizard_max_turn_angle = wizard_max_turn_angle
        self.hastened_ticks = hastened_ticks
        self.hastened_movement_bonus_factor = hastened_movement_bonus_factor
        self.hastened_rotation_bonus_factor = hastened_rotation_bonus_factor

    def max_speed(self, tick):
        return self.wizard_forward_speed * self.movement_bonus_factor(tick)

    def min_speed(self, tick):
        return -self.wizard_backward_speed * self.movement_bonus_factor(tick)

    def max_strafe_speed(self, tick):
        return self.wizard_strafe_speed * self.movement_bonus_factor(tick)

    def min_strafe_speed(self, tick):
        return -self.wizard_strafe_speed * self.movement_bonus_factor(tick)

    def max_turn(self, tick):
        return self.wizard_max_turn_angle * self.rotation_bonus_factor(tick)

    def min_turn(self, tick):
        return -self.wizard_max_turn_angle * self.rotation_bonus_factor(tick)

    def movement_bonus_factor(self, tick):
        return 1 + (self.hastened_movement_bonus_factor if tick < self.hastened_ticks else 0)

    def rotation_bonus_factor(self, tick):
        return 1 + (self.hastened_rotation_bonus_factor if tick < self.hastened_ticks else 0)


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


def get_shift_and_turn(angle: float, bounds: Bounds, speed: float, strafe_speed: float, turn: float, tick: int):
    speed, strafe_speed = limit_speed(speed, strafe_speed, bounds, tick)
    turn = limit_turn(turn, bounds, tick)
    speed_direction = Point(1, 0).rotate(angle)
    strafe_speed_direction = speed_direction.left_orthogonal()
    return speed_direction * speed + strafe_speed_direction * strafe_speed, turn


def limit_speed(speed: float, strafe_speed: float, bounds: Bounds, tick: int):
    speed = min(bounds.max_speed(tick), max(bounds.min_speed(tick), speed))
    strafe_speed = min(bounds.max_strafe_speed(tick), max(bounds.min_strafe_speed(tick), strafe_speed))
    both = hypot(speed / bounds.max_speed(tick), strafe_speed / bounds.max_strafe_speed(tick))
    return (speed / both, strafe_speed / both) if both > 1.0 else (speed, strafe_speed)


def limit_turn(value: float, bounds: Bounds, tick: int):
    return min(bounds.max_turn(tick), max(bounds.min_turn(tick), value))


def make_circles(values):
    for value in values:
        yield Circle(position=value.position, radius=value.radius)


def get_speed_and_turn_to_point(position: Point, angle, target: Point, bounds: Bounds, tick: int):
    direction = target - position
    norm = direction.norm()
    if norm == 0:
        return 0, 0, 0
    turn = normalize_angle(direction.absolute_rotation() - angle)
    speed = bounds.max_speed(tick) * cos(turn) if abs(turn) <= pi / 2 else -bounds.min_speed(tick) * cos(turn)
    strafe_speed = bounds.max_strafe_speed(tick) * sin(turn)
    ratio = min(1, direction.norm() / hypot(speed, strafe_speed))
    return speed * ratio, strafe_speed * ratio, limit_turn(turn, bounds, tick)


def make_dynamic_units_barriers(positions, units):
    for k, v in positions.items():
        dynamic_unit = units[k]
        yield Circle(position=v, radius=dynamic_unit.radius)
