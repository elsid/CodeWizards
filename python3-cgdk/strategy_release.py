from model.ActionType import ActionType
from model.Game import Game
from model.Move import Move
from model.Wizard import Wizard
from model.World import World

from strategy_common import LazyInit, lazy_init, Point
from strategy_move import optimize_movement
from strategy_target import get_target


OPTIMIZE_MOVEMENT_STEP_SIZES = tuple([10] * 30)
OPTIMIZE_MOVEMENT_TICKS = int(sum(OPTIMIZE_MOVEMENT_STEP_SIZES) * 0.9)
UPDATE_TARGET_TICKS = 30


class Context:
    def __init__(self, me: Wizard, world: World, game: Game, move: Move):
        self.me = me
        self.world = world
        self.game = game
        self.move = move

    @property
    def my_position(self):
        return Point(self.me.x, self.me.y)


class Strategy(LazyInit):
    def __init__(self):
        super().__init__()
        self.__movements = None
        self.__states = None
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
            context.move.turn = context.me.get_angle_to_unit(self.__target) if self.__target else movement.turn
        if self.__target:
            target_position = Point(self.__target.x, self.__target.y)
            distance = target_position.distance(context.my_position)
            if distance <= context.me.cast_range:
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
    def target_position(self):
        return self.__target_position

    def _init_impl(self, context: Context):
        self.__target_position = Point(context.game.map_size / 2, context.game.map_size / 2)

    def __update_cache(self, context: Context):
        for v in context.world.buildings:
            self.__cached_buildings[v.id] = v
        for v in context.world.minions:
            self.__cached_minions[v.id] = v
        for v in context.world.wizards:
            self.__cached_wizards[v.id] = v

    def __update_target(self, context: Context):
        if (self.__last_update_target is None or
                context.world.tick_index - self.__last_update_target >= UPDATE_TARGET_TICKS):
            self.__target, position = get_target(
                me=context.me,
                buildings=tuple(self.__cached_buildings.values()),
                minions=tuple(self.__cached_minions.values()),
                wizards=tuple(self.__cached_wizards.values()),
                guardian_tower_attack_range=context.game.guardian_tower_attack_range,
                faction_base_attack_range=context.game.faction_base_attack_range,
                orc_woodcutter_attack_range=context.game.orc_woodcutter_attack_range,
                fetish_blowdart_attack_range=context.game.fetish_blowdart_attack_range,
                wizard_cast_range=context.game.wizard_cast_range,
            )
            if position is not None and position != self.__target_position:
                self.__target_position = position
                self.__movements = None
                self.__last_update_target = context.world.tick_index

    def __update_movements(self, context: Context):
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
