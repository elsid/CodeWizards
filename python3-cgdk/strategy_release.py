from model.ActionType import ActionType
from model.Game import Game
from model.Move import Move
from model.Wizard import Wizard
from model.World import World

from strategy_common import LazyInit, lazy_init, Point
from strategy_move import optimize_movement


OPTIMIZE_MOVEMENT_STEP_SIZES = tuple([10] * 30)
OPTIMIZE_MOVEMENT_TICKS = int(sum(OPTIMIZE_MOVEMENT_STEP_SIZES) * 0.9)


class Context:
    def __init__(self, me: Wizard, world: World, game: Game, move: Move):
        self.me = me
        self.world = world
        self.game = game
        self.move = move


class Strategy(LazyInit):
    def __init__(self):
        super().__init__()
        self.__movements = None
        self.__cur_movement = None
        self.__last_update_movements_tick_index = None
        self.__last_next_movement_tick_index = None
        self.__target = None

    @lazy_init
    def move(self, context: Context):
        self.__update_movements(context)
        if self.__movements:
            movement = self.__movements[self.__cur_movement]
            context.move.speed = movement.speed
            context.move.strafe_speed = movement.strafe_speed
            context.move.turn = movement.turn
            context.move.action = ActionType.MAGIC_MISSILE

    @property
    def movements(self):
        return self.__movements[self.__cur_movement:]

    @property
    def target(self):
        return self.__target

    def _init_impl(self, context: Context):
        self.__target = Point(context.game.map_size - 400, 400)

    def __update_movements(self, context):
        if (not self.__movements or
                context.world.tick_index - self.__last_update_movements_tick_index >= OPTIMIZE_MOVEMENT_TICKS or
                self.__cur_movement >= len(self.__movements) - 1):
            _, self.__movements = optimize_movement(
                target=self.__target,
                circular_unit=context.me,
                world=context.world,
                game=context.game,
                step_sizes=OPTIMIZE_MOVEMENT_STEP_SIZES,
            )
            if self.__movements:
                self.__cur_movement = 0
                self.__last_update_movements_tick_index = context.world.tick_index
                self.__last_next_movement_tick_index = context.world.tick_index
        elif (context.world.tick_index - self.__last_next_movement_tick_index >=
              self.__movements[self.__cur_movement].step_size):
            self.__cur_movement += 1
            self.__last_next_movement_tick_index = context.world.tick_index
