import pytest

from strategy_common import Point, Circle


@pytest.mark.parametrize(
    ('a', 'b', 'expected'), [
        (Circle(Point(0, 0), 1), Circle(Point(3, 0), 1), False),
        (Circle(Point(0, 0), 1), Circle(Point(2, 0), 1), True),
        (Circle(Point(0, 0), 1), Circle(Point(2, 0), 2), True),
    ]
)
def test_has_intersection_with_circle(a, b, expected):
    assert a.has_intersection_with_circle(b) == expected
