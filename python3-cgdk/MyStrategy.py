from os import environ
from time import time

from model.Game import Game
from model.Move import Move
from model.Wizard import Wizard
from model.World import World

from strategy_release import Strategy as ReleaseStrategy
from strategy_release import Context


def profile(func):
    if 'PROFILE' in environ and environ['PROFILE'] == '1':
        from debug import log

        def wrap(self, me: Wizard, world: World, game: Game, move: Move):
            start = time()
            result = func(self, me, world, game, move)
            finish = time()
            log(tick=world.tick_index, id=me.id, time=finish - start)
            return result

        return wrap
    else:
        return func


class MyStrategy:
    def __init__(self):
        self.__fail_on_exception = 'FAIL_ON_EXCEPTION' in environ and environ['FAIL_ON_EXCEPTION'] == '1'
        if 'DEBUG' in environ and environ['DEBUG'] == '1':
            from strategy_debug import Strategy as DebugStrategy
            self.__strategy = DebugStrategy
        else:
            self.__strategy = ReleaseStrategy
        self.__impl = self.__strategy()

    def move(self, me: Wizard, world: World, game: Game, move: Move):
        if 'MAX_TICKS' in environ and world.tick_index >= int(environ['MAX_TICKS']):
            exit(0)
        context = Context(me=me, world=world, game=game, move=move)
        if self.__fail_on_exception:
            self.__impl.move(context)
        else:
            try:
                self.__impl.move(context)
            except Exception:
                self.__impl = self.__strategy()
            except BaseException:
                self.__impl = self.__strategy()
