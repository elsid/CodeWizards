from strategy_release import Strategy as ReleaseStrategy, Context
from debug.client import DebugClient


class Strategy:
    def __init__(self):
        self.__impl = ReleaseStrategy()
        self.__client = DebugClient()
        self.__actual_path = list()
        self.__expected_path = list()

    def move(self, context: Context):
        self.__impl.move(context)
        self.__visualize()

    def __visualize(self):
        with self.__client.post() as post:
            self.__visualize_path(post, self.__impl.actual_path, (0, 0, 1))
            self.__visualize_path(post, self.__impl.expected_path, (0, 1, 0))
            self.__visualize_states(post)

    def __visualize_states(self, post):
        self.__visualize_path(post, [v.position for v in self.__impl.states], (1, 0, 0))

    @staticmethod
    def __visualize_path(post, path, color):
        if path:
            last_position = path[0]
            for position in path:
                post.line(last_position.x, last_position.y, position.x, position.y, color)
                last_position = position
