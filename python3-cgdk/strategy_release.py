from model.Game import Game
from model.Move import Move
from model.Wizard import Wizard
from model.World import World

from strategy_common import LazyInit, lazy_init


OPTIMIZE_MOVEMENT_STEPS = 30


class Context:
    def __init__(self, me: Wizard, world: World, game: Game, move: Move):
        self.me = me
        self.world = world
        self.game = game
        self.move = move


class Strategy(LazyInit):
    @lazy_init
    def move(self, context: Context):
        pass

    def _init_impl(self, context: Context):
        pass
