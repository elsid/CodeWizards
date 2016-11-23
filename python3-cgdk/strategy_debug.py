from operator import itemgetter

from model.LaneType import LaneType

from strategy_release import Strategy as ReleaseStrategy, Context
from debug.client import DebugClient

LANE_TYPES_COLORS = {
    LaneType.TOP: (0.8, 0.2, 0.2),
    LaneType.MIDDLE: (0.2, 0.8, 0.2),
    LaneType.BOTTOM: (0.2, 0.2, 0.8),
}


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
            self.__visualize_target_positions_penalties(post)
            self.__visualize_states(context.world.tick_index, post)
            self.__visualize_target(post)
            self.__visualize_target_position(context, post)

    def __visualize_target(self, post):
        target = self.__impl.target
        if target:
            post.circle(target.x, target.y, target.radius + 10, (1, 0, 0))

    def __visualize_target_position(self, context: Context, post):
        target = self.__impl.target_position
        if target:
            post.circle(context.me.x, context.me.y, 5, (0.2, 0.2, 0.2))

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
                post.fill_circle(point.x, point.y, 5, color)

    def __visualize_graph(self, post):
        graph = self.__impl.graph
        drawn_arcs = set()
        for node in graph.nodes:
            post.fill_circle(node.position.x, node.position.y, 10, (0.8, 0.8, 0.8))
            for arc in node.arcs:
                if ((node.position.x, node.position.y, arc.dst.position.x, arc.dst.position.y) not in drawn_arcs
                        and (arc.dst.position.x, arc.dst.position.y, node.position.x, node.position.y) not in drawn_arcs):
                    drawn_arcs.add((node.position.x, node.position.y, arc.dst.position.x, arc.dst.position.y))
                    post.line(node.position.x, node.position.y, arc.dst.position.x, arc.dst.position.y, (0.8, 0.8, 0.8))
        for lane, nodes in graph.lanes_nodes.items():
            for node in nodes:
                post.circle(node.position.x, node.position.y, 20, LANE_TYPES_COLORS[lane])

    def __visualize_graph_path(self, post):
        self.__visualize_path(post, (v.position for v in self.__impl.path), (0.2, 0.2, 0.2), 10)
        next_node = self.__impl.next_node
        if next_node:
            post.circle(next_node.position.x, next_node.position.y, 20, (0.2, 0.2, 0.2))

    def __visualize_states(self, tick, post):
        shift = tick % 10
        self.__visualize_path(post, (v.position for i, v in enumerate(self.__impl.states)
                                     if (i + shift) % 10 == 0), (0.2, 0.75, 0), 5)
