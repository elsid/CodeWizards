from strategy_path import make_graph


def test_make_graph():
    graph = make_graph(4000)
    nodes_len = len(graph.nodes)
    zone_size = graph.zone_size
    assert (nodes_len, zone_size) == (52, 600)
