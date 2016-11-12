from model.ActionType import ActionType
from model.Game import Game
from model.Move import Move
from model.Wizard import Wizard
from model.World import World

from strategy_common import LazyInit, lazy_init, Point
from strategy_move import optimize_movement


OPTIMIZE_MOVEMENT_STEP_SIZES = tuple([10] * 10)
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
        self.__movements_iter = None
        self.__cur_movement = None
        self.__last_update_movements_tick_index = None
        self.__last_next_movement_tick_index = None

    @lazy_init
    def move(self, context: Context):
        self.__update_movements(context)
        context.move.speed = self.__cur_movement.speed
        context.move.strafe_speed = self.__cur_movement.strafe_speed
        context.move.turn = self.__cur_movement.turn
        context.move.action = ActionType.MAGIC_MISSILE

    def _init_impl(self, context: Context):
        pass

    def __update_movements(self, context):
        if (self.__movements is None or
                context.world.tick_index - self.__last_update_movements_tick_index >= OPTIMIZE_MOVEMENT_TICKS):
            self.__movements = list(optimize_movement(
                target=Point(context.game.map_size, context.game.map_size),
                circular_unit=context.me,
                world=context.world,
                game=context.game,
                step_sizes=OPTIMIZE_MOVEMENT_STEP_SIZES,
            ))
            self.__movements_iter = iter(self.__movements)
            self.__last_update_movements_tick_index = context.world.tick_index
            self.__cur_movement = next(self.__movements_iter)
            self.__last_next_movement_tick_index = context.world.tick_index
        elif context.world.tick_index - self.__last_next_movement_tick_index >= self.__cur_movement.step_size:
            self.__cur_movement = next(self.__movements_iter)
            self.__last_next_movement_tick_index = context.world.tick_index
