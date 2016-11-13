from strategy_common import Point, normalize_angle
from strategy_move import Bounds, simulate_move, State
from strategy_release import Strategy as ReleaseStrategy, Context
from debug.client import DebugClient


class Strategy:
    def __init__(self):
        self.__impl = ReleaseStrategy()
        self.__client = DebugClient()
        self.__path = list()

    def move(self, context: Context):
        self.__impl.move(context)
        self.__path.append(Point(context.me.x, context.me.y))
        self.__visualize_movements(context)

    def __visualize_movements(self, context: Context):
        with self.__client.post() as post:
            last_position = Point(context.me.x, context.me.y)
            target = self.__impl.target
            post.line(last_position.x, last_position.y, target.x, target.y, (0, 1, 0))
            movements = self.__impl.movements
            steps = len(movements)
            simulation = simulate_move(
                movements=movements,
                state=State(
                    position=last_position,
                    angle=normalize_angle(context.me.angle),
                    path_length=0,
                    intersection=False,
                ),
                radius=context.me.radius,
                bounds=Bounds(world=context.world, game=context.game),
                barriers=tuple(),
                map_size=context.game.map_size,
            )
            for step, values in enumerate(simulation):
                position = values[0]
                post.line(last_position.x, last_position.y, position.x, position.y, (step / steps, 0, 0))
                last_position = position
            last_position = self.__path[0]
            for position in self.__path:
                post.line(last_position.x, last_position.y, position.x, position.y, (0, 0, 1))
                last_position = position
