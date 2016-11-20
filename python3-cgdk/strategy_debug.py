from operator import itemgetter

from strategy_path import ZONE_SIZE
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
        self.__visualize(context)

    def __visualize(self, context: Context):
        with self.__client.post() as post:
            self.__visualize_graph(post)
            self.__visualize_graph_path(post)
            self.__visualize_target(context, post)
            self.__visualize_target_position(context, post)
            self.__visualize_target_positions_penalties(post)
            post.circle(1200, 1200, 50, (0, 0, 0))
            post.circle(context.game.map_size - 1200, context.game.map_size - 1200, 50, (0, 0, 0))

    def __visualize_target(self, context: Context, post):
        target = self.__impl.target
        if target:
            post.line(context.me.x, context.me.y, target.x, target.y, (1, 0, 1))

    def __visualize_target_position(self, context: Context, post):
        target = self.__impl.target_position
        if target:
            post.line(context.me.x, context.me.y, target.x, target.y, (0, 0, 0))

    @staticmethod
    def __visualize_path(post, path, color, radius):
        if path:
            last_position = None
            for position in path:
                if radius:
                    post.fill_circle(position.x, position.y, radius, color)
                if last_position:
                    post.line(last_position.x, last_position.y, position.x, position.y, color)
                last_position = position

    def __visualize_target_positions_penalties(self, post):
        if self.__impl.target_positions_penalties:
            min_penalty = min(self.__impl.target_positions_penalties, key=itemgetter(1))[1]
            max_penalty = max(self.__impl.target_positions_penalties, key=itemgetter(1))[1]
            for point, penalty in self.__impl.target_positions_penalties:
                normalized = (penalty - min_penalty) / (abs(max_penalty - min_penalty) or 1)
                if normalized < 1 / 4:
                    color = (0, 4 * normalized, 1)
                elif normalized < 1 / 2:
                    color = (0, 1, 1 - 4 * (normalized - 1 / 2))
                elif normalized < 3 / 4:
                    color = (4 * (normalized - 1 / 2), 1, 0)
                else:
                    color = (1, 1 - 4 * (normalized - 3 / 4), 0)
                post.fill_circle(point.x, point.y, 10, color)

    def __visualize_graph(self, post):
        graph = self.__impl.graph
        for node in graph.nodes:
            post.fill_circle(node.position.x, node.position.y, 10, (0.8, 0.8, 0.8))
            for arc in node.arcs:
                post.line(node.position.x, node.position.y, arc.dst.position.x, arc.dst.position.y, (0.8, 0.8, 0.8))
                post.circle(node.position.x, node.position.y, ZONE_SIZE, (0.8, 0.8, 0.8))
        post.fill_circle(graph.center.position.x, graph.center.position.y, 20, (0.8, 0.8, 0.8))

    def __visualize_graph_path(self, post):
        self.__visualize_path(post, (v.position for v in self.__impl.path), (0.2, 0.2, 0.2), 10)
