from functools import reduce
from time import time

from model.ActionType import ActionType
from model.Game import Game
from model.Move import Move
from model.Wizard import Wizard
from model.World import World

from strategy_common import LazyInit, lazy_init, Point
from strategy_move import optimize_movement
from strategy_target import get_target


OPTIMIZE_MOVEMENT_STEP_SIZES = tuple([10] * 10 + [20, 40])
OPTIMIZE_MOVEMENT_TICKS = sum(OPTIMIZE_MOVEMENT_STEP_SIZES) // 2
UPDATE_TARGET_POSITION_TICKS = 30
UPDATE_TARGET_TICKS = 200
MAX_TIME = 0.1
CACHE_TTL = 100
LOST_TARGET_TICKS = 30


class Context:
    def __init__(self, me: Wizard, world: World, game: Game, move: Move):
        self.me = me
        self.world = world
        self.game = game
        self.move = move
        self.__start = None
        self.__finish = None

    def __enter__(self):
        self.__start = time()
        return self

    def __exit__(self, *_):
        self.__finish = time()

    @property
    def my_position(self):
        return Point(self.me.x, self.me.y)

    def time_left(self):
        return time() - self.__start


class Strategy(LazyInit):
    def __init__(self):
        super().__init__()
        self.__movements = list()
        self.__states = list()
        self.__cur_movement = None
        self.__last_update_movements_tick_index = None
        self.__last_next_movement_tick_index = None
        self.__target = None
        self.__target_position = None
        self.__actual_path = list()
        self.__expected_path = list()
        self.__get_attack_range = None
        self.__last_update_target = None
        self.__cached_buildings = dict()
        self.__cached_wizards = dict()
        self.__cached_minions = dict()

    @lazy_init
    def move(self, context: Context):
        self.__actual_path.append(Point(context.me.x, context.me.y))
        self.__update_cache(context)
        self.__update_target(context)
        self.__update_movements(context)
        if self.__movements:
            movement = self.__movements[self.__cur_movement]
            context.move.speed = movement.speed
            context.move.strafe_speed = movement.strafe_speed
            context.move.turn = movement.turn
        if self.__target:
            target_position = Point(self.__target.x, self.__target.y)
            distance = target_position.distance(context.my_position)
            if distance <= context.me.cast_range:
                context.move.action = ActionType.MAGIC_MISSILE
                context.move.turn = context.me.get_angle_to_unit(self.__target)

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

    def _init_impl(self, context: Context):
        self.__target_position = Point(context.world.width / 2, context.world.height / 2 + 300)

    def __update_cache(self, context: Context):
        for v in context.world.buildings:
            self.__cached_buildings[v.id] = v
            setattr(v, 'last_seen', context.world.tick_index)
        for v in context.world.minions:
            update_dynamic_unit(self.__cached_minions, v)
            setattr(v, 'last_seen', context.world.tick_index)
        for v in context.world.wizards:
            update_dynamic_unit(self.__cached_wizards, v)
            setattr(v, 'last_seen', context.world.tick_index)
        invalidate_cache(self.__cached_buildings, context.world.tick_index - CACHE_TTL)
        invalidate_cache(self.__cached_minions, context.world.tick_index - CACHE_TTL)
        invalidate_cache(self.__cached_wizards, context.world.tick_index - CACHE_TTL)

    def __update_target(self, context: Context):
        if (self.__target is not None and (
                context.world.tick_index - self.__target.last_seen > LOST_TARGET_TICKS
                or self.__target.last_seen < context.world.tick_index
                and self.__target.life < self.__target.max_life / 4)):
            self.__target = None
        if (self.__last_update_target is None or self.__target is None or
                context.world.tick_index - self.__last_update_target >= UPDATE_TARGET_TICKS):
            self.__target, position = get_target(
                me=context.me,
                buildings=tuple(self.__cached_buildings.values()),
                minions=tuple(v for v in self.__cached_minions.values() if v.last_seen == context.world.tick_index),
                wizards=tuple(v for v in self.__cached_wizards.values() if v.last_seen == context.world.tick_index),
                guardian_tower_attack_range=context.game.guardian_tower_attack_range,
                faction_base_attack_range=context.game.faction_base_attack_range,
                orc_woodcutter_attack_range=context.game.orc_woodcutter_attack_range,
                fetish_blowdart_attack_range=context.game.fetish_blowdart_attack_range,
                magic_missile_direct_damage=context.game.magic_missile_direct_damage,
            )
            if self.__target_position is None or position is not None and position != self.__target_position:
                self.__target_position = position
                self.__movements = None
                self.__last_update_target = context.world.tick_index

    def __update_movements(self, context: Context):
        if not self.__target_position:
            return
        if (not self.__movements or
                context.world.tick_index - self.__last_update_movements_tick_index >= OPTIMIZE_MOVEMENT_TICKS or
                self.__cur_movement >= len(self.__movements) - 1):
            self.__calculate_movements(context)
        elif (context.world.tick_index - self.__last_next_movement_tick_index >=
              self.__movements[self.__cur_movement].step_size):
            self.__next_movement(context)

    def __calculate_movements(self, context: Context):
        self.__states, self.__movements = optimize_movement(
            target=self.__target_position,
            look_target=Point(self.__target.x, self.__target.y) if self.__target else self.__target_position,
            circular_unit=context.me,
            world=context.world,
            game=context.game,
            step_sizes=OPTIMIZE_MOVEMENT_STEP_SIZES,
            random_seed=context.game.random_seed,
            max_time=context.time_left(),
        )
        if self.__movements:
            self.__cur_movement = 0
            self.__last_update_movements_tick_index = context.world.tick_index
            self.__last_next_movement_tick_index = context.world.tick_index
            self.__expected_path.append(self.__states[self.__cur_movement].position)

    def __next_movement(self, context: Context):
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
