from strategy_release import Strategy as ReleaseStrategy, Context
from debug.client import DebugClient


class Strategy:
    def __init__(self):
        self.__impl = ReleaseStrategy()
        self.__client = DebugClient()

    def move(self, context: Context):
        self.__impl.move(context)
