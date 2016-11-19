from collections import OrderedDict, deque
from functools import reduce
from time import time

from model.ActionType import ActionType
from model.Bonus import Bonus
from model.Game import Game
from model.Move import Move
from model.Wizard import Wizard
from model.World import World

from strategy_common import LazyInit, lazy_init, Point
from strategy_move import optimize_movement
from strategy_target import get_target


OPTIMIZE_MOVEMENT_STEP_SIZE = 10
OPTIMIZE_MOVEMENT_TICKS = 100
UPDATE_TARGET_TICKS = 200
MAX_TIME = 1
CACHE_TTL_BONUSES = 100
CACHE_TTL_BUILDINGS = 200
CACHE_TTL_TREES = 200
CACHE_TTL_WIZARDS = 30
CACHE_TTL_MINIONS = 30
CACHE_TTL_PROJECTILES = 10
LOST_TARGET_TICKS = 10


class Context:
    def __init__(self, me: Wizard, world: World, game: Game, move: Move, log_events: bool):
        self.me = me
        self.world = world
        self.game = game
        self.move = move
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
                        speed_x=self.me.speed_x, speed_y=self.me.speed_y, life=self.me.life)
        return self

    def __exit__(self, *_):
        self.post_event(name='finish', duration=time() - self.__events[0]['time'], speed=self.move.speed,
                        strafe_speed=self.move.strafe_speed, turn=self.move.turn, action=self.move.action)
        for event in self.__events:
            self.__write_log(event)
        self.__finish = time()

    @property
    def my_position(self):
        return Point(self.me.x, self.me.y)

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
        self.__actual_path = deque(maxlen=100 * OPTIMIZE_MOVEMENT_STEP_SIZE)
        self.__expected_path = deque(maxlen=100)
        self.__get_attack_range = None
        self.__last_update_target = None
        self.__cached_buildings = dict()
        self.__cached_wizards = dict()
        self.__cached_minions = dict()
        self.__cached_trees = dict()
        self.__cached_projectiles = dict()
        self.__cached_bonuses = dict()
        self.__target_positions_penalties = list()

    @lazy_init
    def move(self, context: Context):
        context.post_event(name='strategy_release_move')
        self.__actual_path.append(Point(context.me.x, context.me.y))
        self.__update_cache(context)
        self.__update_target(context)
        self.__update_movements(context)
        if self.__cur_movement < len(self.__movements):
            context.post_event(name='apply_movement')
            movement = self.__movements[self.__cur_movement]
            context.move.speed = movement.speed
            context.move.strafe_speed = movement.strafe_speed
            context.move.turn = movement.turn
        if self.__target:
            target_position = Point(self.__target.x, self.__target.y)
            distance = target_position.distance(context.my_position)
            direction = Point(1, 0).rotate(context.me.angle)
            if distance <= context.me.cast_range + context.me.radius + self.__target.radius:
                context.post_event(name='apply_target_turn')
                context.move.turn = context.me.get_angle_to_unit(self.__target)
                if (target_position.distance(context.my_position + direction * distance) <
                        context.game.magic_missile_radius + self.__target.radius
                        and not isinstance(self.__target, Bonus)):
                    context.post_event(name='apply_target_action')
                    context.move.action = ActionType.MAGIC_MISSILE

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

    def _init_impl(self, context: Context):
        self.__target_position = Point(context.world.width / 2, context.world.height / 2 + 300)

    def __update_cache(self, context: Context):
        context.post_event(name='update_cache')
        for v in context.world.buildings:
            self.__cached_buildings[v.id] = v
            setattr(v, 'last_seen', context.world.tick_index)
        invalidate_cache(self.__cached_buildings, context.world.tick_index - CACHE_TTL_BUILDINGS)
        for v in context.world.minions:
            update_dynamic_unit(self.__cached_minions, v)
            setattr(v, 'last_seen', context.world.tick_index)
        invalidate_cache(self.__cached_minions, context.world.tick_index - CACHE_TTL_MINIONS)
        for v in context.world.wizards:
            update_dynamic_unit(self.__cached_wizards, v)
            setattr(v, 'last_seen', context.world.tick_index)
        invalidate_cache(self.__cached_wizards, context.world.tick_index - CACHE_TTL_WIZARDS)
        for v in context.world.trees:
            self.__cached_trees[v.id] = v
            setattr(v, 'last_seen', context.world.tick_index)
        invalidate_cache(self.__cached_trees, context.world.tick_index - CACHE_TTL_BUILDINGS)
        for v in context.world.projectiles:
            update_dynamic_unit(self.__cached_projectiles, v)
            setattr(v, 'last_seen', context.world.tick_index)
        invalidate_cache(self.__cached_projectiles, context.world.tick_index - CACHE_TTL_TREES)
        for v in context.world.bonuses:
            self.__cached_bonuses[v.id] = v
            setattr(v, 'last_seen', context.world.tick_index)
        invalidate_cache(self.__cached_bonuses, context.world.tick_index - CACHE_TTL_BONUSES)

    def __update_target(self, context: Context):

        def is_recently_seen(unit):
            return unit.last_seen == context.world.tick_index

        context.post_event(name='update_target')
        if self.__target is not None and context.world.tick_index - self.__target.last_seen > LOST_TARGET_TICKS:
            context.post_event(name='reset_target', last_seen=self.__target.last_seen,
                               life=self.__target.life if hasattr(self.__target, 'life') else None)
            self.__target = None
        if (self.__last_update_target is None or self.__target is None or
                context.world.tick_index - self.__last_update_target >= UPDATE_TARGET_TICKS):
            context.post_event(name='get_target', last_update_target=self.__last_update_target,
                               target=str(self.__target))
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
            )
            if self.__target:
                context.post_event(name='target_updated', target_type=str(type(self.__target)),
                                   target_id=self.__target.id)
            else:
                context.post_event(name='target_updated')
            if self.__target_position is None or position is not None and position != self.__target_position:
                context.post_event(name='reset_target_position', old=str(position), new=str(position))
                self.__target_position = position
                self.__last_update_target = context.world.tick_index

    def __update_movements(self, context: Context):
        context.post_event(name='update_movements')
        if not self.__target_position:
            return
        if (self.__last_update_movements_tick_index is None or
                context.world.tick_index - self.__last_update_movements_tick_index >= OPTIMIZE_MOVEMENT_TICKS):
            self.__calculate_movements(context)
        elif (self.__cur_movement < len(self.__movements) and
              context.world.tick_index - self.__last_next_movement_tick_index >=
              self.__movements[self.__cur_movement].step_size):
            self.__next_movement(context)

    def __calculate_movements(self, context: Context):

        def is_recently_seen(unit):
            return unit.last_seen == context.world.tick_index

        context.post_event(name='calculate_movements')
        self.__states, self.__movements = optimize_movement(
            target=self.__target_position,
            look_target=Point(self.__target.x, self.__target.y) if self.__target else self.__target_position,
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
            max_barriers_range=OPTIMIZE_MOVEMENT_TICKS * context.game.wizard_forward_speed,
            max_time=context.time_left(),
        )
        self.__last_update_movements_tick_index = context.world.tick_index
        if self.__movements:
            context.post_event(name='movements_updated')
            self.__cur_movement = 0
            self.__last_next_movement_tick_index = context.world.tick_index
            self.__expected_path.append(self.__states[self.__cur_movement].position)

    def __next_movement(self, context: Context):
        context.post_event(name='next_movement')
        self.__cur_movement += 1
        self.__expected_path.append(self.__states[self.__cur_movement].position)
        error = abs(self.__expected_path[-1].distance(self.__actual_path[-1]))
        if error > context.game.wizard_radius * 2:
            self.__calculate_movements(context)
        else:
            self.__last_next_movement_tick_index = context.world.tick_index


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


def invalidate_cache(cache, earliest_last_seen):
    to_rm = [v.id for v in cache.values() if v.last_seen < earliest_last_seen]
    for v in to_rm:
        del cache[v]
