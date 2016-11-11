from model.Game import Game
from model.Move import Move
from model.Wizard import Wizard
from model.World import World


OPTIMIZE_MOVEMENT_STEPS = 30


class Context:
    def __init__(self, me: Wizard, world: World, game: Game, move: Move):
        self.me = me
        self.world = world
        self.game = game
        self.move = move
