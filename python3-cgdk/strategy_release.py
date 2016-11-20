from collections import OrderedDict, deque
from functools import reduce
from itertools import chain
from time import time

from model.ActionType import ActionType
from model.Bonus import Bonus
from model.Game import Game
from model.Move import Move
from model.Wizard import Wizard
from model.World import World

from strategy_common import LazyInit, lazy_init, Point, Circle
from strategy_move import optimize_movement, has_intersection_with_barriers, make_circles
from strategy_path import make_graph, select_destination, get_shortest_path, get_nearest_node, ZONE_SIZE
from strategy_target import get_target


OPTIMIZE_MOVEMENT_STEP_SIZE = 20
OPTIMIZE_MOVEMENT_TICKS = 10
UPDATE_TARGET_TICKS = 10
MAX_TIME = 0.3
CACHE_TTL_BONUSES = 2500
CACHE_TTL_BUILDINGS = 2500
CACHE_TTL_TREES = 2500
CACHE_TTL_WIZARDS = 30
CACHE_TTL_MINIONS = 30
CACHE_TTL_PROJECTILES = 10
LOST_TARGET_TICKS = 5
GET_TARGET_MAX_ITERATIONS = 10
UPDATE_DESTINATION_TICKS = 50
CHANGE_MODE_TICKS = 100


class Context:
    def __init__(self, me: Wizard, world: World, game: Game, move: Move, log_events: bool):
        self.me = me
        self.world = world
        self.game = game
        self.move = move
        setattr(me, 'position', Point(me.x, me.y))
        self.__start = None
        self.__finish = None
        self.__events = list()
        if log_events:
            from debug import log_dict
            self.__write_log = log_dict
        else:
            self.__write_log = lambda _: None

    def __enter__(self):
        self.__start = time()
        self.post_event(name='start', x=self.me.x, y=self.me.y, angle=self.me.angle,
                        speed_x=self.me.speed_x, speed_y=self.me.speed_y, life=self.me.life,
                        owner_player_id=self.me.owner_player_id)
        return self

    def __exit__(self, *_):
        self.post_event(name='finish', duration=time() - self.__events[0]['time'], speed=self.move.speed,
                        strafe_speed=self.move.strafe_speed, turn=self.move.turn, action=self.move.action)
        for event in self.__events:
            self.__write_log(event)
        self.__finish = time()

    def time_left(self):
        return (self.__start + MAX_TIME) - time()

    def post_event(self, name, **kwargs):
        data = OrderedDict()
        data['tick'] = self.world.tick_index
        data['time'] = time()
        data['id'] = self.me.id
        data['name'] = name
        for k, v in kwargs.items():
            data[k] = v
        self.__events.append(data)

    @property
    def duration(self):
        return self.__finish - self.__start


class Strategy(LazyInit):
    def __init__(self):
        super().__init__()
        self.__movements = list()
        self.__states = list()
        self.__cur_movement = 0
        self.__last_update_movements_tick_index = None
        self.__last_next_movement_tick_index = None
        self.__target = None
        self.__target_position = None
        self.__actual_path = deque(maxlen=20 * OPTIMIZE_MOVEMENT_STEP_SIZE)
        self.__expected_path = deque(maxlen=20)
        self.__get_attack_range = None
        self.__last_update_target = None
        self.__cached_buildings = dict()
        self.__cached_wizards = dict()
        self.__cached_minions = dict()
        self.__cached_trees = dict()
        self.__cached_projectiles = dict()
        self.__cached_bonuses = dict()
        self.__target_positions_penalties = list()
        self.__last_update_destination = None
        self.__graph = None
        self.__destination = None
        self.__departure = None
        self.__path = list()
        self.__next_node = 0
        self.__apply_mode = self.__apply_move_mode
        self.__last_change_mode = 0

    @lazy_init
    def move(self, context: Context):
        context.post_event(name='strategy_release_move')
        self.__actual_path.append(Point(context.me.x, context.me.y))
        self.__update_cache(context)
        self.__apply_mode(context)
        self.__update_movements(context)
        self.__apply_move(context)
        self.__apply_action(context)

    @property
    def movements(self):
        return self.__movements[self.__cur_movement:]

    @property
    def states(self):
        return self.__states[self.__cur_movement:]

    @property
    def actual_path(self):
        return self.__actual_path

    @property
    def expected_path(self):
        return self.__expected_path

    @property
    def target(self):
        return self.__target

    @property
    def target_position(self):
        return self.__target_position

    @property
    def target_positions_penalties(self):
        return self.__target_positions_penalties

    @property
    def graph(self):
        return self.__graph

    @property
    def path(self):
        return self.__path

    def _init_impl(self, context: Context):
        self.__graph = make_graph(context.game.map_size)

    def __update_cache(self, context: Context):
        context.post_event(name='update_cache')
        if context.world.tick_index and context.world.tick_index % 2500 == 0:
            bonus1 = Bonus(
                id=-1,
                x=1200,
                y=1200,
                speed_x=None,
                speed_y=None,
                angle=None,
                faction=None,
                radius=context.game.bonus_radius,
                type=None,
            )
            setattr(bonus1, 'last_seen', context.world.tick_index)
            setattr(bonus1, 'position', Point(bonus1.x, bonus1.y))
            self.__cached_bonuses[-1] = bonus1
            bonus2 = Bonus(
                id=-2,
                x=context.game.map_size - 1200,
                y=context.game.map_size - 1200,
                speed_x=None,
                speed_y=None,
                angle=None,
                faction=None,
                radius=context.game.bonus_radius,
                type=None,
            )
            setattr(bonus2, 'last_seen', context.world.tick_index)
            setattr(bonus2, 'position', Point(bonus2.x, bonus2.y))
            self.__cached_bonuses[-2] = bonus2
        for v in context.world.buildings:
            self.__cached_buildings[v.id] = v
        for v in context.world.minions:
            update_dynamic_unit(self.__cached_minions, v)
        for v in context.world.wizards:
            update_dynamic_unit(self.__cached_wizards, v)
        for v in context.world.trees:
            self.__cached_trees[v.id] = v
        for v in context.world.projectiles:
            update_dynamic_unit(self.__cached_projectiles, v)
        for v in context.world.bonuses:
            self.__cached_bonuses[v.id] = v
        units = chain(
            context.world.buildings,
            context.world.minions,
            context.world.wizards,
            context.world.trees,
            context.world.projectiles,
            context.world.bonuses,
        )
        for unit in units:
            setattr(unit, 'last_seen', context.world.tick_index)
            setattr(unit, 'position', Point(unit.x, unit.y))
        invalidate_cache(self.__cached_buildings, context.world.tick_index, CACHE_TTL_BUILDINGS, context.me.position,
                         context.me.vision_range * 0.9)
        invalidate_cache(self.__cached_minions, context.world.tick_index, CACHE_TTL_MINIONS, context.me.position,
                         context.me.vision_range * 0.9)
        invalidate_cache(self.__cached_wizards, context.world.tick_index, CACHE_TTL_WIZARDS, context.me.position,
                         context.me.vision_range * 0.9)
        invalidate_cache(self.__cached_trees, context.world.tick_index, CACHE_TTL_BUILDINGS, context.me.position,
                         context.me.vision_range * 0.9)
        invalidate_cache(self.__cached_projectiles, context.world.tick_index, CACHE_TTL_TREES, context.me.position,
                         context.me.vision_range * 0.9)
        invalidate_cache(self.__cached_bonuses, context.world.tick_index, CACHE_TTL_BONUSES, context.me.position,
                         context.me.vision_range * 0.9)

    def __apply_battle_mode(self, context: Context):
        context.post_event(name='apply_battle_mode')
        self.__update_target(context)
        if (self.__apply_move_mode == self.__apply_battle_mode
                and context.world.tick_index - self.__last_change_mode > CHANGE_MODE_TICKS):
            context.post_event(name='change_mode', old='battle', new='move')
            self.__apply_mode = self.__apply_move_mode
            self.__last_change_mode = context.world.tick_index

    def __apply_move_mode(self, context: Context):
        context.post_event(name='apply_move_mode')
        self.__update_path(context)
        self.__next_path_node(context)

    def __update_path(self, context: Context):
        if (self.__last_update_destination is not None and
                context.world.tick_index - self.__last_update_destination < UPDATE_DESTINATION_TICKS):
            return
        destination = select_destination(
            graph=self.__graph,
            me=context.me,
            buildings=tuple(self.__cached_buildings.values()),
            minions=tuple(v for v in self.__cached_minions.values()),
            wizards=tuple(v for v in self.__cached_wizards.values()),
            bonuses=tuple(v for v in self.__cached_bonuses.values()),
        )
        nearest_node = get_nearest_node(self.__graph.nodes, context.me.position)
        if self.__destination is not None and id(nearest_node) == id(self.__departure):
            return
        path, _ = get_shortest_path(nearest_node, destination)
        context.post_event(name='update_destination', destination=str(destination.position),
                           path=[str(v.position) for v in path])
        context.post_event(name='update_target_position',
                           old=str(self.__target_position) if self.__target_position else self.__target_position,
                           new=str(path[0].position))
        self.__departure = nearest_node
        self.__destination = destination
        self.__path = path
        self.__next_node = 0
        self.__target_position = path[0].position
        self.__last_update_destination = context.world.tick_index

    def __next_path_node(self, context: Context):
        if self.__next_node >= len(self.__path):
            return
        if context.me.position.distance(self.__path[self.__next_node].position) > ZONE_SIZE:
            return
        self.__next_node += 1
        if self.__next_node < len(self.__path):
            context.post_event(name='next_path_node')
            context.post_event(name='update_target_position',
                               old=str(self.__target_position) if self.__target_position else self.__target_position,
                               new=str(self.__path[self.__next_node].position))
            self.__target_position = self.__path[self.__next_node].position
        else:
            context.post_event(name='change_mode', old='move', new='battle')
            self.__apply_mode = self.__apply_battle_mode
            self.__next_node = 0
            self.__path = list()
            self.__destination = None
            self.__departure = None

    def __update_target(self, context: Context):

        def is_recently_seen(unit):
            return unit.last_seen == context.world.tick_index

        context.post_event(name='update_target')
        if self.__target is not None and context.world.tick_index - self.__target.last_seen > LOST_TARGET_TICKS:
            context.post_event(name='reset_target', last_seen=self.__target.last_seen,
                               life=self.__target.life if hasattr(self.__target, 'life') else None)
            self.__target = None
        if (self.__last_update_target is not None and self.__target is not None and
                context.world.tick_index - self.__last_update_target < UPDATE_TARGET_TICKS):
            return
        context.post_event(name='get_target', last_update_target=self.__last_update_target,
                           target=str(self.__target) if self.__target else self.__target)
        self.__target_positions_penalties.clear()
        self.__target, position = get_target(
            me=context.me,
            buildings=tuple(self.__cached_buildings.values()),
            minions=tuple(v for v in self.__cached_minions.values() if is_recently_seen(v)),
            wizards=tuple(v for v in self.__cached_wizards.values() if is_recently_seen(v)),
            trees=tuple(self.__cached_trees.values()),
            projectiles=tuple(v for v in self.__cached_projectiles.values() if is_recently_seen(v)),
            bonuses=tuple(self.__cached_bonuses.values()),
            guardian_tower_attack_range=context.game.guardian_tower_attack_range,
            faction_base_attack_range=context.game.faction_base_attack_range,
            orc_woodcutter_attack_range=context.game.orc_woodcutter_attack_range,
            fetish_blowdart_attack_range=context.game.fetish_blowdart_attack_range,
            magic_missile_direct_damage=context.game.magic_missile_direct_damage,
            magic_missile_radius=context.game.magic_missile_radius,
            map_size=context.game.map_size,
            penalties=self.__target_positions_penalties,
            max_distance=ZONE_SIZE,
            max_iterations=GET_TARGET_MAX_ITERATIONS,
        )
        if self.__target:
            context.post_event(name='target_updated', target_type=str(type(self.__target)),
                               target_id=self.__target.id, position=str(self.__target.position))
            context.post_event(name='update_target_position',
                               old=str(self.__target_position) if self.__target_position else self.__target_position,
                               new=str(position))
            self.__target_position = position
            self.__last_update_target = context.world.tick_index
        else:
            context.post_event(name='change_mode', old='move', new='battle')
            self.__apply_mode = self.__apply_move_mode
            self.__target = None

    def __update_movements(self, context: Context):
        if not self.__target_position:
            return
        context.post_event(name='update_movements')
        if (self.__last_update_movements_tick_index is None or
                context.world.tick_index - self.__last_update_movements_tick_index >= OPTIMIZE_MOVEMENT_TICKS):
            self.__calculate_movements(context)
        else:
            self.__next_movement(context)

    def __calculate_movements(self, context: Context):

        def is_recently_seen(unit):
            return unit.last_seen == context.world.tick_index

        context.post_event(name='calculate_movements')
        self.__states, self.__movements = optimize_movement(
            target=self.__target_position,
            look_target=self.__target.position if self.__target else self.__target_position,
            circular_unit=context.me,
            buildings=tuple(self.__cached_buildings.values()),
            minions=tuple(v for v in self.__cached_minions.values() if is_recently_seen(v)),
            wizards=tuple(v for v in self.__cached_wizards.values() if is_recently_seen(v)),
            trees=tuple(self.__cached_trees.values()),
            wizard_forward_speed=context.game.wizard_forward_speed,
            wizard_backward_speed=context.game.wizard_backward_speed,
            wizard_strafe_speed=context.game.wizard_strafe_speed,
            wizard_max_turn_angle=context.game.wizard_max_turn_angle,
            map_size=context.game.map_size,
            step_size=OPTIMIZE_MOVEMENT_STEP_SIZE,
            max_barriers_range=context.me.vision_range,
            max_time=context.time_left(),
        )
        self.__last_update_movements_tick_index = context.world.tick_index
        if self.__movements:
            context.post_event(name='movements_updated')
            self.__cur_movement = 0
            self.__last_next_movement_tick_index = context.world.tick_index
            self.__expected_path.append(self.__states[self.__cur_movement].position)

    def __next_movement(self, context: Context):
        if (self.__cur_movement >= len(self.__movements) or
                context.world.tick_index - self.__last_next_movement_tick_index <
                self.__movements[self.__cur_movement].step_size):
            return
        context.post_event(name='next_movement')
        self.__cur_movement += 1
        self.__expected_path.append(self.__states[self.__cur_movement].position)
        error = abs(self.__expected_path[-1].distance(self.__actual_path[-1]))
        if error > context.game.wizard_radius * 2:
            self.__calculate_movements(context)
        else:
            self.__last_next_movement_tick_index = context.world.tick_index

    def __apply_move(self, context: Context):
        if self.__cur_movement >= len(self.__movements):
            return
        context.post_event(name='apply_movement')
        movement = self.__movements[self.__cur_movement]
        context.move.speed = movement.speed
        context.move.strafe_speed = movement.strafe_speed
        context.move.turn = movement.turn

    def __apply_action(self, context: Context):
        if not self.__target:
            return

        def need_apply_missle():
            direction = Point(1, 0).rotate(context.me.angle)
            if (target_position.distance(context.me.position + direction * distance) >
                    context.game.magic_missile_radius + self.__target.radius):
                return False
            missile = Circle(context.me.position, context.game.magic_missile_radius)
            barriers = tuple(chain(
                make_circles(v for v in context.world.wizards if v.id != context.me.id
                             and v.faction == context.me.faction),
                make_circles(v for v in context.world.minions if v.faction == context.me.faction),
                make_circles(v for v in context.world.buildings if v.faction == context.me.faction),
            ))
            return not has_intersection_with_barriers(missile, missile.position + direction * context.me.cast_range,
                                                      barriers)

        target_position = self.__target.position
        distance = target_position.distance(context.me.position)
        if (distance <= context.me.cast_range - context.me.radius
                + context.game.magic_missile_radius + self.__target.radius):
            context.post_event(name='apply_target_turn')
            turn = context.me.get_angle_to_unit(self.__target)
            context.move.turn = turn
            if isinstance(self.__target, Bonus):
                return
            if distance <= self.__target.radius + context.game.staff_range:
                context.post_event(name='apply_target_action', type='STAFF')
                context.move.action = ActionType.STAFF
            elif need_apply_missle():
                context.post_event(name='apply_target_action', type='MAGIC_MISSILE')
                context.move.cast_angle = turn
                context.move.action = ActionType.MAGIC_MISSILE


def update_dynamic_unit(cache, new):
    old = cache.get(new.id)
    new_position = Point(new.x, new.y)
    if old is None:
        speed = Point(new.speed_x, new.speed_y)
        setattr(new, 'positions_history', [new_position - speed, new_position])
        setattr(new, 'mean_speed', speed)
    else:
        setattr(new, 'positions_history', old.positions_history[-3:] + [new_position])
        setattr(new, 'mean_speed',
                reduce(lambda s, v: s + v, old.positions_history, Point(0, 0)) / len(old.positions_history))
    cache[new.id] = new


def invalidate_cache(cache, tick, ttl, my_position, min_range):
    to_rm = [v.id for v in cache.values() if v.last_seen < tick - ttl
             or (v.last_seen < tick and v.position.distance(my_position) < min_range)]
    for v in to_rm:
        del cache[v]
