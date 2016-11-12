import pytest

from strategy_barriers import Circular
from strategy_common import Point


@pytest.mark.parametrize(
    ('a', 'b', 'expected'), [
        (Circular(Point(0, 0), 1), Circular(Point(3, 0), 1), False),
        (Circular(Point(0, 0), 1), Circular(Point(2, 0), 1), True),
        (Circular(Point(0, 0), 1), Circular(Point(2, 0), 2), True),
    ]
)
def test_has_intersection_with_circular(a, b, expected):
    assert a.has_intersection_with_circular(b) == expected
