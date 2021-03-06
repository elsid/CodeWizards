from collections import OrderedDict, deque
from functools import reduce
from itertools import chain
from time import time

from model.ActionType import ActionType
from model.Bonus import Bonus
from model.Game import Game
from model.LaneType import LaneType
from model.Move import Move
from model.SkillType import SkillType
from model.StatusType import StatusType
from model.Wizard import Wizard
from model.World import World

from strategy_common import LazyInit, lazy_init, Point, Circle, normalize_angle
from strategy_move import optimize_movement, has_intersection_with_barriers, make_circles
from strategy_path import make_graph, select_destination, get_shortest_path, get_nearest_node, has_near_enemy
from strategy_target import get_target, get_optimal_position


OPTIMIZE_MOVEMENT_STEP_SIZE = 20
OPTIMIZE_MOVEMENT_TICKS = 7
UPDATE_TARGET_TICKS = 8
MAX_TIME = 0.05
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
MESSAGE_TICKS = 500
RECENTLY_SEEN_TICKS = 3
ACTION_TYPES_NAMES = {
    ActionType.NONE: 'NONE',
    ActionType.STAFF: 'STAFF',
    ActionType.MAGIC_MISSILE: 'MAGIC_MISSILE',
    ActionType.FROST_BOLT: 'FROST_BOLT',
    ActionType.FIREBALL: 'FIREBALL',
    ActionType.HASTE: 'HASTE',
    ActionType.SHIELD: 'SHIELD',
}
LANE_TYPES_NAMES = {
    LaneType.TOP: 'TOP',
    LaneType.BOTTOM: 'BOTTOM',
    LaneType.MIDDLE: 'MIDDLE',
}
SKILL_TYPES_NAMES = {
    SkillType.RANGE_BONUS_PASSIVE_1: 'RANGE_BONUS_PASSIVE_1',
    SkillType.RANGE_BONUS_AURA_1: 'RANGE_BONUS_AURA_1',
    SkillType.RANGE_BONUS_PASSIVE_2: 'RANGE_BONUS_PASSIVE_2',
    SkillType.RANGE_BONUS_AURA_2: 'RANGE_BONUS_AURA_2',
    SkillType.ADVANCED_MAGIC_MISSILE: 'ADVANCED_MAGIC_MISSILE',
    SkillType.MAGICAL_DAMAGE_BONUS_PASSIVE_1: 'MAGICAL_DAMAGE_BONUS_PASSIVE_1',
    SkillType.MAGICAL_DAMAGE_BONUS_AURA_1: 'MAGICAL_DAMAGE_BONUS_AURA_1',
    SkillType.MAGICAL_DAMAGE_BONUS_PASSIVE_2: 'MAGICAL_DAMAGE_BONUS_PASSIVE_2',
    SkillType.MAGICAL_DAMAGE_BONUS_AURA_2: 'MAGICAL_DAMAGE_BONUS_AURA_2',
    SkillType.FROST_BOLT: 'FROST_BOLT',
    SkillType.STAFF_DAMAGE_BONUS_PASSIVE_1: 'STAFF_DAMAGE_BONUS_PASSIVE_1',
    SkillType.STAFF_DAMAGE_BONUS_AURA_1: 'STAFF_DAMAGE_BONUS_AURA_1',
    SkillType.STAFF_DAMAGE_BONUS_PASSIVE_2: 'STAFF_DAMAGE_BONUS_PASSIVE_2',
    SkillType.STAFF_DAMAGE_BONUS_AURA_2: 'STAFF_DAMAGE_BONUS_AURA_2',
    SkillType.FIREBALL: 'FIREBALL',
    SkillType.MOVEMENT_BONUS_FACTOR_PASSIVE_1: 'MOVEMENT_BONUS_FACTOR_PASSIVE_1',
    SkillType.MOVEMENT_BONUS_FACTOR_AURA_1: 'MOVEMENT_BONUS_FACTOR_AURA_1',
    SkillType.MOVEMENT_BONUS_FACTOR_PASSIVE_2: 'MOVEMENT_BONUS_FACTOR_PASSIVE_2',
    SkillType.MOVEMENT_BONUS_FACTOR_AURA_2: 'MOVEMENT_BONUS_FACTOR_AURA_2',
    SkillType.HASTE: 'HASTE',
    SkillType.MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1: 'MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1',
    SkillType.MAGICAL_DAMAGE_ABSORPTION_AURA_1: 'MAGICAL_DAMAGE_ABSORPTION_AURA_1',
    SkillType.MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2: 'MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2',
    SkillType.MAGICAL_DAMAGE_ABSORPTION_AURA_2: 'MAGICAL_DAMAGE_ABSORPTION_AURA_2',
    SkillType.SHIELD: 'SHIELD',
}
STATUS_TYPES_NAMES = {
    StatusType.BURNING: 'BURNING',
    StatusType.EMPOWERED: 'EMPOWERED',
    StatusType.FROZEN: 'FROZEN',
    StatusType.HASTENED: 'HASTENED',
    StatusType.SHIELDED: 'SHIELDED',
}


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
            self.__write_log = None

    def __enter__(self):
        self.__start = time()
        if self.__write_log:
            player = next(v for v in self.world.players if v.id == self.me.owner_player_id)
            self.post_event(
                name='start',
                x=self.me.x,
                y=self.me.y,
                angle=self.me.angle,
                speed_x=self.me.speed_x,
                speed_y=self.me.speed_y,
                life=self.me.life,
                owner_player_id=self.me.owner_player_id,
                random_seed=self.game.random_seed,
                remaining_action_cooldown_ticks=self.me.remaining_action_cooldown_ticks,
                remaining_cooldown_ticks_by_action={ACTION_TYPES_NAMES.get(i, str(i)): v for i, v in
                                                    enumerate(self.me.remaining_cooldown_ticks_by_action)},
                mana=self.me.mana,
                max_mana=self.me.max_mana,
                vision_range=self.me.vision_range,
                cast_range=self.me.cast_range,
                xp=self.me.xp,
                level=self.me.level,
                skills=sorted(SKILL_TYPES_NAMES.get(v, str(v)) for v in self.me.skills),
                master=self.me.master,
                messages=[dict(lane=LANE_TYPES_NAMES.get(v.lane, str(v.lane)),
                               skill_to_learn=SKILL_TYPES_NAMES.get(v.skill_to_learn, str(v.skill_to_learn)),
                               raw_message=v.raw_message) for v in self.me.messages],
                max_life=self.me.max_life,
                statuses={STATUS_TYPES_NAMES.get(v.type, str(v.type)): v.remaining_duration_ticks
                          for v in self.me.statuses},
                player=dict(id=player.id, name=player.name, strategy_crashed=player.strategy_crashed, score=player.score),
            )
        return self

    def __exit__(self, *_):
        if self.__write_log:
            self.post_event(name='finish', duration=time() - self.__events[0]['time'], speed=self.move.speed,
                            strafe_speed=self.move.strafe_speed, turn=self.move.turn, action=self.move.action,
                            min_cast_distance=self.move.min_cast_distance)
            for event in self.__events:
                self.__write_log(event)
        self.__finish = time()

    def time_left(self):
        return (self.__start + MAX_TIME) - time()

    def post_event(self, name, **kwargs):
        if self.__write_log:
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
        self.__target_lane = None
        self.__last_apply_battle_mode = None
        self.__last_apply_move_mode = None
        self.__last_message = 0

    @property
    def movements(self):
        return self.__movements[self.__cur_movement:]

    @property
    def states(self):
        return self.__states[self.__cur_movement:]

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

    @property
    def next_node(self):
        return self.__path[self.__next_node] if self.__next_node < len(self.__path) else None

    @lazy_init
    def move(self, context: Context):
        self.__update_cache(context)
        self.__handle_messages(context)
        if context.world.tick_index - self.__last_change_mode > CHANGE_MODE_TICKS:
            if self.__apply_mode == self.__apply_battle_mode:
                self.__use_move_mode(context)
            elif self.__apply_mode == self.__apply_move_mode:
                self.__use_battle_mode(context)
        self.__apply_mode(context)
        self.__update_movements(context)
        self.__apply_move(context)
        self.__apply_action(context)

    def _init_impl(self, context: Context):
        self.__graph = make_graph(context.game.map_size)

    def __handle_messages(self, context: Context):
        if context.me.messages:
            self.__target_lane = context.me.messages[-1].lane
            self.__last_message = context.world.tick_index
            self.__use_move_mode(context)
        if context.world.tick_index - self.__last_message > MESSAGE_TICKS:
            self.__target_lane = None

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
            if v.me:
                setattr(context.me, 'mean_speed', v.mean_speed)
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
            if isinstance(unit, type(self.__target)) and unit.id == self.__target.id:
                self.__target = unit
        friends = [v for v in chain(context.world.wizards, context.world.minions, context.world.buildings)
                   if v.faction == context.me.faction]
        invalidate_cache(self.__cached_buildings, context.world.tick_index, CACHE_TTL_BUILDINGS, friends)
        invalidate_cache(self.__cached_minions, context.world.tick_index, CACHE_TTL_MINIONS, friends)
        invalidate_cache(self.__cached_wizards, context.world.tick_index, CACHE_TTL_WIZARDS, friends)
        invalidate_cache(self.__cached_trees, context.world.tick_index, CACHE_TTL_BUILDINGS, friends)
        invalidate_cache(self.__cached_projectiles, context.world.tick_index, CACHE_TTL_TREES, friends)
        invalidate_cache(self.__cached_bonuses, context.world.tick_index, CACHE_TTL_BONUSES, friends)

    def __apply_battle_mode(self, context: Context):
        if self.__last_apply_battle_mode == context.world.tick_index:
            return
        context.post_event(name='apply_battle_mode')
        self.__last_apply_battle_mode = context.world.tick_index
        self.__update_target(context)

    def __apply_move_mode(self, context: Context):
        if self.__last_apply_move_mode == context.world.tick_index:
            return
        context.post_event(name='apply_move_mode')
        self.__last_apply_move_mode = context.world.tick_index
        self.__update_path(context)
        self.__next_path_node(context)

    def __update_path(self, context: Context):
        if (self.__destination is not None and
                context.world.tick_index - self.__last_update_destination < UPDATE_DESTINATION_TICKS):
            return
        destination = select_destination(
            graph=self.__graph,
            me=context.me,
            buildings=tuple(self.__cached_buildings.values()),
            minions=tuple(v for v in self.__cached_minions.values()),
            wizards=tuple(v for v in self.__cached_wizards.values()),
            bonuses=tuple(v for v in self.__cached_bonuses.values()),
            target_lane=self.__target_lane,
        )
        nearest_node = get_nearest_node(self.__graph.nodes, context.me.position)
        if id(nearest_node) == id(self.__departure):
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
        required_distance = 0.9 * (self.__graph.zone_size if self.__next_node > len(self.__path) - 2
                                   else self.__path[self.__next_node + 1].position.distance(self.__path[self.__next_node].position))
        if context.me.position.distance(self.__path[self.__next_node].position) > required_distance:
            return
        context.post_event(name='next_path_node', old=self.__next_node, new=self.__next_node + 1)
        self.__next_node += 1
        units = chain(context.world.buildings, context.world.minions, context.world.wizards)
        if (self.__next_node < len(self.__path)
                and not has_near_enemy(context.me.position, tuple(units), context.me.faction, self.__graph.zone_size)):
            context.post_event(name='update_target_position',
                               old=str(self.__target_position) if self.__target_position else self.__target_position,
                               new=str(self.__path[self.__next_node].position))
            self.__target_position = self.__path[self.__next_node].position
        else:
            context.post_event(name='change_mode', old='move', new='battle')
            self.__use_battle_mode(context)
            self.__apply_mode(context)

    def __update_target(self, context: Context):

        def is_recently_seen(unit):
            return context.world.tick_index - unit.last_seen <= RECENTLY_SEEN_TICKS

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
        optimal_position = None
        max_distance = 1.3 * context.me.vision_range
        for _ in range(2):
            self.__target = get_target(
                me=context.me,
                buildings=tuple(self.__cached_buildings.values()),
                minions=tuple(v for v in self.__cached_minions.values() if is_recently_seen(v)),
                wizards=tuple(v for v in self.__cached_wizards.values() if is_recently_seen(v)),
                trees=tuple(self.__cached_trees.values()),
                bonuses=tuple(self.__cached_bonuses.values()),
                magic_missile_direct_damage=context.game.magic_missile_direct_damage,
                empowered_damage_factor=context.game.empowered_damage_factor,
                staff_range=context.game.staff_range,
                max_distance=max_distance,
            )
            if not self.__target:
                break
            optimal_position = get_optimal_position(
                target=self.__target,
                me=context.me,
                buildings=tuple(self.__cached_buildings.values()),
                minions=tuple(v for v in self.__cached_minions.values() if is_recently_seen(v)),
                wizards=tuple(v for v in self.__cached_wizards.values() if is_recently_seen(v)),
                trees=tuple(self.__cached_trees.values()),
                projectiles=tuple(v for v in self.__cached_projectiles.values() if is_recently_seen(v)),
                bonuses=tuple(self.__cached_bonuses.values()),
                orc_woodcutter_attack_range=context.game.orc_woodcutter_attack_range,
                fetish_blowdart_attack_range=context.game.fetish_blowdart_attack_range,
                magic_missile_direct_damage=context.game.magic_missile_direct_damage,
                magic_missile_radius=context.game.magic_missile_radius,
                dart_radius=context.game.dart_radius,
                map_size=context.game.map_size,
                shielded_direct_damage_absorption_factor=context.game.shielded_direct_damage_absorption_factor,
                empowered_damage_factor=context.game.empowered_damage_factor,
                penalties=self.__target_positions_penalties,
                max_distance=max_distance,
                max_iterations=GET_TARGET_MAX_ITERATIONS,
            )
            if (self.__target.position.distance(optimal_position)
                    <= context.me.cast_range + context.game.magic_missile_radius + self.__target.radius):
                break
            max_distance = context.me.cast_range + context.game.magic_missile_radius + self.__target.radius
        if self.__target:
            context.post_event(name='target_updated', target_type=str(type(self.__target)),
                               target_id=self.__target.id, position=str(self.__target.position))
            context.post_event(name='update_target_position',
                               old=str(self.__target_position) if self.__target_position else self.__target_position,
                               new=str(optimal_position))
            self.__target_position = optimal_position
            self.__last_update_target = context.world.tick_index
        else:
            self.__use_move_mode(context)
            self.__apply_mode(context)

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
            me=context.me,
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
            hastened_movement_bonus_factor=context.game.hastened_movement_bonus_factor,
            hastened_rotation_bonus_factor=context.game.hastened_rotation_bonus_factor,
            max_time=context.time_left(),
        )
        self.__last_update_movements_tick_index = context.world.tick_index
        if self.__movements:
            context.post_event(name='movements_updated')
            self.__cur_movement = 0
            self.__last_next_movement_tick_index = context.world.tick_index

    def __next_movement(self, context: Context):
        if (self.__cur_movement >= len(self.__movements) or
                context.world.tick_index - self.__last_next_movement_tick_index <
                self.__movements[self.__cur_movement].step_size):
            return
        context.post_event(name='next_movement')
        self.__cur_movement += 1
        error = abs(self.__states[self.__cur_movement].position.distance(context.me.position))
        if (error > context.game.wizard_radius
                and context.world.tick_index - self.__last_update_movements_tick_index >= OPTIMIZE_MOVEMENT_TICKS):
            context.post_event(name='movement_error_overflow', value=error)
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

        def need_apply_staff():
            return (context.me.remaining_cooldown_ticks_by_action[ActionType.STAFF] == 0
                    and distance < self.__target.radius + context.game.staff_range)

        def need_apply_missile():
            if context.me.remaining_cooldown_ticks_by_action[ActionType.MAGIC_MISSILE] != 0:
                return False
            direction = Point(1, 0).rotate(normalize_angle(context.me.angle + turn))
            missile_target = context.me.position + direction * distance
            if target_position.distance(missile_target) > context.game.magic_missile_radius + self.__target.radius:
                return False
            missile = Circle(context.me.position, context.game.magic_missile_radius)
            barriers = tuple(make_circles(v for v in context.world.wizards if v.id != context.me.id
                                          and v.faction == context.me.faction))
            return not has_intersection_with_barriers(missile, missile_target, barriers)

        target_position = self.__target.position
        distance = target_position.distance(context.me.position)
        if distance <= context.me.cast_range + self.__target.radius + context.game.magic_missile_radius:
            context.post_event(name='apply_target_turn')
            turn = context.me.get_angle_to_unit(self.__target)
            context.move.turn = turn
            if isinstance(self.__target, Bonus):
                return
            context.move.cast_angle = turn
            if context.me.remaining_action_cooldown_ticks != 0:
                return
            if need_apply_staff():
                context.post_event(name='apply_target_action', type='STAFF')
                context.move.action = ActionType.STAFF
            elif need_apply_missile():
                context.post_event(name='apply_target_action', type='MAGIC_MISSILE')
                context.move.action = ActionType.MAGIC_MISSILE
                context.move.min_cast_distance = distance - self.__target.radius - context.game.magic_missile_radius

    def __use_move_mode(self, context: Context):
        context.post_event(name='change_mode', old=self.__current_mode_name, new='move')
        self.__next_node = 0
        self.__path = list()
        self.__destination = None
        self.__departure = None
        self.__target = None
        self.__apply_mode = self.__apply_move_mode
        self.__last_change_mode = context.world.tick_index

    def __use_battle_mode(self, context: Context):
        context.post_event(name='change_mode', old=self.__current_mode_name, new='battle')
        self.__next_node = 0
        self.__path = list()
        self.__destination = None
        self.__departure = None
        self.__target = None
        self.__apply_mode = self.__apply_battle_mode
        self.__last_change_mode = context.world.tick_index

    @property
    def __current_mode_name(self):
        if self.__apply_mode == self.__apply_move_mode:
            return 'move'
        if self.__apply_mode == self.__apply_battle_mode:
            return 'battle'


def update_dynamic_unit(cache, new):
    old = cache.get(new.id)
    new_position = Point(new.x, new.y)
    if old is None:
        speed = Point(new.speed_x, new.speed_y)
        setattr(new, 'positions_history', deque([new_position - speed, new_position], maxlen=5))
        setattr(new, 'mean_speed', speed)
    else:
        old.positions_history.append(new_position)
        setattr(new, 'positions_history', old.positions_history)
        setattr(new, 'mean_speed',
                reduce(lambda s, v: s + v, old.positions_history, Point(0, 0)) / len(old.positions_history))
    cache[new.id] = new


def invalidate_cache(cache, tick, ttl, friends):
    def to_rm():
        for cached in cache.values():
            if cached.last_seen < tick - ttl:
                yield cached.id
            elif (cached.last_seen < tick
                    and next((True for v in friends if cached.position.distance(v.position)
                              < v.vision_range - 2 * cached.radius), False)):
                yield cached.id
    for cached_id in tuple(to_rm()):
        del cache[cached_id]
