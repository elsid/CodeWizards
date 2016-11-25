from collections import namedtuple, defaultdict
from math import hypot
from itertools import chain
from heapq import heappop, heappush

from model.LaneType import LaneType

from strategy_common import Point
from strategy_target import is_enemy

Node = namedtuple('Node', ('position', 'arcs'))
Arc = namedtuple('Arc', ('dst', 'weight'))
Graph = namedtuple('Graph', ('nodes', 'center', 'zone_size', 'lanes_nodes', 'friend_base', 'enemy_base'))


def make_graph(map_size):
    nodes = defaultdict(dict)
    map_size = int(map_size)

    def add_node(x, y):
        node = Node(Point(x, y), list())
        nodes[x][y] = node
        return node

    resolution = map_size // 10
    half = resolution // 2
    count = map_size // resolution
    lanes_nodes = defaultdict(list)

    for shift in range(resolution + half, map_size - resolution, resolution):
        lanes_nodes[LaneType.TOP].append(add_node(half, map_size - shift))
        lanes_nodes[LaneType.TOP].append(add_node(map_size - shift, half))
        lanes_nodes[LaneType.BOTTOM].append(add_node(shift, map_size - half))
        lanes_nodes[LaneType.BOTTOM].append(add_node(map_size - half, shift))
        lanes_nodes[LaneType.MIDDLE].append(add_node(shift, map_size - shift))

    add_node(half, map_size - half)
    add_node(map_size - half, half)

    for shift in range(half, map_size, resolution):
        add_node(map_size - shift, map_size - shift)

    lanes_nodes[LaneType.TOP].append(nodes[half][half])
    lanes_nodes[LaneType.TOP].append(nodes[half + resolution][half + resolution])
    lanes_nodes[LaneType.BOTTOM].append(nodes[map_size - half][map_size - half])
    lanes_nodes[LaneType.BOTTOM].append(nodes[map_size - half - resolution][map_size - half - resolution])
    lanes_nodes[LaneType.MIDDLE].append(nodes[map_size // 2 - half][map_size // 2 - half])
    lanes_nodes[LaneType.MIDDLE].append(nodes[map_size // 2 + half][map_size // 2 + half])

    def add_arc(src_x, src_y, dst_x, dst_y, weight):
        src = nodes[src_x][src_y]
        dst = nodes[dst_x][dst_y]
        src.arcs.append(Arc(dst, weight))

    def add_edge(src_x, src_y, dst_x, dst_y):
        weight = hypot(src_x - dst_x, src_y - dst_y)
        add_arc(src_x, src_y, dst_x, dst_y, weight)
        add_arc(dst_x, dst_y, src_x, src_y, weight)

    for shift in range(half + resolution, map_size - 2 * resolution, resolution):
        next_shift = shift + resolution
        add_edge(shift, map_size - shift, next_shift, map_size - next_shift)

    for shift in range(half, map_size - resolution, resolution):
        next_shift = shift + resolution
        add_edge(half, map_size - shift, half, map_size - next_shift)
        add_edge(shift, map_size - half, next_shift, map_size - half)
        add_edge(map_size - shift, map_size - shift, map_size - next_shift, map_size - next_shift)
        add_edge(map_size - half, shift, map_size - half, next_shift)
        add_edge(map_size - shift, half, map_size - next_shift, half)

    add_edge(half + resolution, half + resolution, half + resolution, half)
    add_edge(half + resolution, half + resolution, half, half + resolution)
    add_edge(half + resolution, half + resolution, half + 2 * resolution, half)
    add_edge(half + resolution, half + resolution, half, half + 2 * resolution)

    add_edge(map_size - half - resolution, half + resolution, map_size - half - resolution, half)
    add_edge(map_size - half - resolution, half + resolution, map_size - half, half + resolution)
    add_edge(map_size - half - resolution, half + resolution, map_size - half - 2 * resolution, half)
    add_edge(map_size - half - resolution, half + resolution, map_size - half, half + 2 * resolution)

    add_edge(map_size - half - resolution, map_size - half - resolution,
             map_size - half - resolution, map_size - half)
    add_edge(map_size - half - resolution, map_size - half - resolution,
             map_size - half, map_size - half - resolution)
    add_edge(map_size - half - resolution, map_size - half - resolution,
             map_size - half - 2 * resolution, map_size - half)
    add_edge(map_size - half - resolution, map_size - half - resolution,
             map_size - half, map_size - half - 2 * resolution)

    add_edge(half + resolution, map_size - half - resolution, half, map_size - half - resolution)
    add_edge(half + resolution, map_size - half - resolution, half + resolution, map_size - half)
    add_edge(half + resolution, map_size - half - resolution, half, map_size - half - 2 * resolution)
    add_edge(half + resolution, map_size - half - resolution, half + 2 * resolution, map_size - half)

    add_edge(count // 2 * resolution + half, count // 2 * resolution + half,
             count // 2 * resolution + half, count // 2 * resolution - half)
    add_edge(count // 2 * resolution + half, count // 2 * resolution - half,
             count // 2 * resolution - half, count // 2 * resolution - half)
    add_edge(count // 2 * resolution - half, count // 2 * resolution - half,
             count // 2 * resolution - half, count // 2 * resolution + half)
    add_edge(count // 2 * resolution - half, count // 2 * resolution + half,
             count // 2 * resolution + half, count // 2 * resolution + half)

    def generate():
        for v in nodes.values():
            for w in v.values():
                yield w

    return Graph(
        nodes=list(generate()),
        center=nodes[count // 2 * resolution - half][count // 2 * resolution + half],
        zone_size=1.5 * resolution,
        lanes_nodes=lanes_nodes,
        friend_base=nodes[half][map_size - half],
        enemy_base=nodes[map_size - half][half],
    )


def select_destination(graph: Graph, me, buildings, minions, wizards, bonuses, target_lane):
    units = tuple(chain(buildings, minions, wizards))
    nodes = graph.nodes
    nodes_with_bonus = tuple(node for node in nodes
                             if has_near_units(node.position, bonuses, graph.zone_size))
    nodes_with_enemy = tuple(node for node in (graph.lanes_nodes[target_lane] if target_lane else nodes)
                             if has_near_enemy(node.position, units, me.faction, graph.zone_size))
    nearest_node = get_nearest_node(nodes, me.position)
    if nodes_with_bonus and nodes_with_enemy:
        enemy = get_nearest_node_by_path(nodes_with_enemy, nearest_node)
        if id(enemy) == id(nearest_node):
            return enemy
    if nodes_with_bonus:
        return get_nearest_node_by_path(nodes_with_bonus, nearest_node)
    if nodes_with_enemy:
        to_friend_base = get_path_to_nearest_node(nodes_with_enemy, graph.friend_base)
        to_enemy_base = get_shortest_path(nearest_node, graph.enemy_base)
        if to_friend_base[1] < 2 * graph.zone_size < to_enemy_base[1]:
            return to_friend_base[0][-1]
        return get_nearest_node_by_path(nodes_with_enemy, nearest_node)
    return graph.center


def has_near_enemy(position, units, my_faction, distance):
    return next((True for _ in filter_near_units(position, (v for v in units if is_enemy(v, my_faction)), distance)),
                False)


def has_near_friend(position, units, my_faction, distance):
    return next((True for _ in filter_near_units(position, (v for v in units if v.faction == my_faction), distance)),
                False)


def filter_near_units(position, units, distance):
    return (v for v in units if position.distance(v.position) <= distance)


def has_near_units(position, units, distance):
    return next((True for _ in filter_near_units(position, units, distance)), False)


def get_nearest_node(nodes, my_position):
    return min(nodes, key=lambda v: my_position.distance(v.position))


def get_nearest_node_by_path(nodes, my_node):
    return min(nodes, key=lambda v: get_shortest_path(my_node, v)[1])


def get_path_to_nearest_node(nodes, my_node):
    return min((get_shortest_path(my_node, v) for v in nodes), key=lambda v: v[1])


def get_shortest_path(src: Node, dst: Node):
    queue = [(0, src, list())]
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
