import pytest

from strategy_common import Point
from strategy_path import get_shortest_path, Graph, Node, Arc


@pytest.mark.parametrize(
    ('nodes', 'arcs', 'src', 'dst', 'path', 'sum_weights'), [
        (
            [Point(0, 0), Point(1, 0)],
            [(0, 1)],
            0,
            1,
            [0, 1],
            1,
        ),
        (
            [Point(0, 0), Point(1, 0),
             Point(0, 1), Point(1, 1)],
            [(0, 1), (0, 2), (0, 3), (1, 2), (1, 3), (2, 3)],
            0,
            3,
            [0, 3],
            1.4142135623730951,
        ),
        (
            [Point(0, 0), Point(1, 0), Point(2, 0),
             Point(0, 1), Point(1, 1), Point(2, 1),
             Point(0, 2), Point(1, 2), Point(2, 2)],
            [(0, 1), (1, 2), (2, 5), (5, 8),
             (0, 3), (3, 6), (6, 7)],
            0,
            8,
            [0, 1, 2, 5, 8],
            4,
        ),
    ]
)
def test_get_shortest_path(nodes, arcs, src, dst, path, sum_weights):
    graph = make_graph(nodes, arcs)
    result = get_shortest_path(graph.nodes[src], graph.nodes[dst])
    assert ([v.position for v in result[0]], result[1]) == ([graph.nodes[v].position for v in path], sum_weights)


def make_graph(nodes, arcs):
    graph = Graph(list(), list(), 0, None, None, None)
    for node in nodes:
        graph.nodes.append(Node(node, list()))
    for arc in arcs:
        src = graph.nodes[arc[0]]
        dst = graph.nodes[arc[1]]
        src.arcs.append(Arc(dst, src.position.distance(dst.position)))
    return graph
