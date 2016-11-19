from collections import namedtuple, defaultdict
from math import hypot
from itertools import chain
from heapq import heappop, heappush

from strategy_common import Point
from strategy_target import is_enemy

RESOLUTION = 400
ZONE_SIZE = 1.5 * RESOLUTION

Node = namedtuple('Node', ('position', 'arcs'))
Arc = namedtuple('Arc', ('dst', 'weight'))
Graph = namedtuple('Graph', ('nodes', 'center'))


def make_graph(map_size):
    nodes = defaultdict(dict)
    map_size = int(map_size)

    def add_node(x, y):
        nodes[x][y] = Node(Point(x, y), list())

    half = RESOLUTION // 2
    count = map_size // RESOLUTION

    for shift in range(half, map_size, RESOLUTION):
        add_node(half, map_size - shift)
        add_node(shift, map_size - half)
        add_node(shift, map_size - shift)
        add_node(map_size - shift, map_size - shift)
        add_node(map_size - half, shift)
        add_node(map_size - shift, half)

    def add_arc(src_x, src_y, dst_x, dst_y, weight):
        src = nodes[src_x][src_y]
        dst = nodes[dst_x][dst_y]
        src.arcs.append(Arc(dst, weight))

    def add_edge(src_x, src_y, dst_x, dst_y):
        weight = hypot(src_x - dst_x, src_y - dst_y)
        add_arc(src_x, src_y, dst_x, dst_y, weight)
        add_arc(dst_x, dst_y, src_x, src_y, weight)

    for shift in range(half + RESOLUTION, map_size - 2 * RESOLUTION, RESOLUTION):
        next_shift = shift + RESOLUTION
        add_edge(shift, map_size - shift, next_shift, map_size - next_shift)

    for shift in range(half, map_size - RESOLUTION, RESOLUTION):
        next_shift = shift + RESOLUTION
        add_edge(half, map_size - shift, half, map_size - next_shift)
        add_edge(shift, map_size - half, next_shift, map_size - half)
        add_edge(map_size - shift, map_size - shift, map_size - next_shift, map_size - next_shift)
        add_edge(map_size - half, shift, map_size - half, next_shift)
        add_edge(map_size - shift, half, map_size - next_shift, half)

    add_edge(half + RESOLUTION, half + RESOLUTION, half + RESOLUTION, half)
    add_edge(half + RESOLUTION, half + RESOLUTION, half, half + RESOLUTION)
    add_edge(half + RESOLUTION, half + RESOLUTION, half + 2 * RESOLUTION, half)
    add_edge(half + RESOLUTION, half + RESOLUTION, half, half + 2 * RESOLUTION)

    add_edge(map_size - half - RESOLUTION, half + RESOLUTION, map_size - half - RESOLUTION, half)
    add_edge(map_size - half - RESOLUTION, half + RESOLUTION, map_size - half, half + RESOLUTION)
    add_edge(map_size - half - RESOLUTION, half + RESOLUTION, map_size - half - 2 * RESOLUTION, half)
    add_edge(map_size - half - RESOLUTION, half + RESOLUTION, map_size - half, half + 2 * RESOLUTION)

    add_edge(map_size - half - RESOLUTION, map_size - half - RESOLUTION,
             map_size - half - RESOLUTION, map_size - half)
    add_edge(map_size - half - RESOLUTION, map_size - half - RESOLUTION,
             map_size - half, map_size - half - RESOLUTION)
    add_edge(map_size - half - RESOLUTION, map_size - half - RESOLUTION,
             map_size - half - 2 * RESOLUTION, map_size - half)
    add_edge(map_size - half - RESOLUTION, map_size - half - RESOLUTION,
             map_size - half, map_size - half - 2 * RESOLUTION)

    add_edge(half + RESOLUTION, map_size - half - RESOLUTION, half, map_size - half - RESOLUTION)
    add_edge(half + RESOLUTION, map_size - half - RESOLUTION, half + RESOLUTION, map_size - half)
    add_edge(half + RESOLUTION, map_size - half - RESOLUTION, half, map_size - half - 2 * RESOLUTION)
    add_edge(half + RESOLUTION, map_size - half - RESOLUTION, half + 2 * RESOLUTION, map_size - half)

    add_edge(count // 2 * RESOLUTION + half, count // 2 * RESOLUTION + half,
             count // 2 * RESOLUTION + half, count // 2 * RESOLUTION - half)
    add_edge(count // 2 * RESOLUTION + half, count // 2 * RESOLUTION - half,
             count // 2 * RESOLUTION - half, count // 2 * RESOLUTION - half)
    add_edge(count // 2 * RESOLUTION - half, count // 2 * RESOLUTION - half,
             count // 2 * RESOLUTION - half, count // 2 * RESOLUTION + half)
    add_edge(count // 2 * RESOLUTION - half, count // 2 * RESOLUTION + half,
             count // 2 * RESOLUTION + half, count // 2 * RESOLUTION + half)

    def generate():
        for v in nodes.values():
            for w in v.values():
                yield w

    return Graph(list(generate()), nodes[count // 2 * RESOLUTION + half][count // 2 * RESOLUTION + half])


def select_destination(graph: Graph, me, buildings, minions, wizards, bonuses):
    my_position = Point(me.x, me.y)
    units = tuple(chain(buildings, minions, wizards))
    nodes = graph.nodes
    nodes_with_bonus = tuple(node for node in nodes if has_near_units(node.position, bonuses))
    nearest_node = get_nearest_node(nodes, my_position)
    if nodes_with_bonus:
        return get_nearest_node_by_path(nodes_with_bonus, nearest_node)
    nodes_with_enemy = tuple(node for node in nodes if has_near_enemy(node.position, units, me.faction))
    if nodes_with_enemy:
        return get_nearest_node_by_path(nodes_with_enemy, nearest_node)
    return graph.center


def has_near_enemy(position, units, my_faction):
    return next((True for _ in filter_near_units(position, (v for v in units if is_enemy(v, my_faction)))), False)


def has_near_friend(position, units, my_faction):
    return next((True for _ in filter_near_units(position, (v for v in units if v.faction == my_faction))), False)


def filter_near_units(position, units):
    return (v for v in units if position.distance(Point(v.x, v.y)) < ZONE_SIZE)


def has_near_units(position, units):
    return next((True for _ in filter_near_units(position, units)), False)


def get_nearest_node(nodes, my_position):
    return min(nodes, key=lambda v: my_position.distance(v.position))


def get_nearest_node_by_path(nodes, my_node):
    return min(nodes, key=lambda v: get_shortest_path(my_node, v)[1])


def get_shortest_path(src: Node, dst: Node):
    queue = [(0, src, [src])]
    visited = set()
    result = None
    while queue:
        sum_weight, node, path = heappop(queue)
        visited.add(id(node))
        path = path + [node]
        if id(dst) == id(node):
            result = (path, sum_weight)
            break
        for arc in node.arcs:
            if id(arc.dst) not in visited:
                heappush(queue, (sum_weight + arc.weight, arc.dst, path))
    return result
