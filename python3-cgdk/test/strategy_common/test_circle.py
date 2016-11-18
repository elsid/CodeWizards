import pytest

from math import sqrt

from strategy_common import Point, Circle, Line


@pytest.mark.parametrize(
    ('this', 'other', 'result'), [
        (Circle(Point(0, 0), 1), Circle(Point(3, 0), 1), False),
        (Circle(Point(0, 0), 1), Circle(Point(2, 0), 1), True),
        (Circle(Point(0, 0), 1), Circle(Point(1, 0), 1), True),
        (Circle(Point(0, 0), 2), Circle(Point(1, 0), 1), True),
        (Circle(Point(0, 0), 2), Circle(Point(0, 0), 1), True),
    ]
)
def test_has_intersection_with_circle(this: Circle, other: Circle, result):
    assert this.has_intersection_with_circle(other) == result


@pytest.mark.parametrize(
    ('circle', 'line', 'result'), [
        (Circle(Point(0, 0), 1), Line(Point(-1, 0), Point(1, 0)), True),
        (Circle(Point(0, 0), 1), Line(Point(0, -1), Point(0, 1)), True),
        (Circle(Point(0, 0), 1), Line(Point(-1, -1), Point(1, 1)), True),
        (Circle(Point(0, 0), 1), Line(Point(1, 0), Point(-1, 0)), True),
        (Circle(Point(0, 0), 1), Line(Point(0, 1), Point(0, -1)), True),
        (Circle(Point(0, 0), 1), Line(Point(1, 1), Point(-1, -1)), True),
        (Circle(Point(0, 0), 1), Line(Point(sqrt(2), 0), Point(0, sqrt(2))), True),
        (Circle(Point(0, 0), 1), Line(Point(0, sqrt(2)), Point(-sqrt(2), 0)), True),
        (Circle(Point(0, 0), 1), Line(Point(-sqrt(2), 0), Point(0, -sqrt(2))), True),
        (Circle(Point(0, 0), 1), Line(Point(0, -sqrt(2)), Point(sqrt(2), 0)), True),
        (Circle(Point(0, 0), 1), Line(Point(2, 0), Point(0, 2)), False),
        (Circle(Point(0, 0), 1), Line(Point(0, 2), Point(-2, 0)), False),
        (Circle(Point(0, 0), 1), Line(Point(-2, 0), Point(0, -2)), False),
        (Circle(Point(0, 0), 1), Line(Point(0, -2), Point(2, 0)), False),
        (Circle(Point(0, 0), 1), Line(Point(2, 0), Point(3, 0)), False),
        (Circle(Point(0, 0), 1), Line(Point(3, 0), Point(2, 0)), False),
        (Circle(Point(0, 0), 1), Line(Point(-2, 0), Point(-3, 0)), False),
        (Circle(Point(0, 0), 1), Line(Point(-3, 0), Point(-2, 0)), False),
        (Circle(Point(0, 0), 1), Line(Point(0, 2), Point(0, 3)), False),
        (Circle(Point(0, 0), 1), Line(Point(0, 3), Point(0, 2)), False),
        (Circle(Point(0, 0), 1), Line(Point(0, -2), Point(0, -3)), False),
        (Circle(Point(0, 0), 1), Line(Point(0, -3), Point(0, -2)), False),
    ]
)
def test_has_intersection_with_line(circle: Circle, line: Line, result):
    assert circle.has_intersection_with_line(line) == result


@pytest.mark.parametrize(
    ('this', 'other', 'next_position', 'result'), [
        (Circle(Point(0, 0), 1), Circle(Point(-2, 0), 1), Point(-2, 0), True),
        (Circle(Point(0, 0), 1), Circle(Point(-3, 0), 1), Point(-3, 0), False),
        (Circle(Point(0, 0), 1), Circle(Point(-3, 0), 1), Point(-2, 0), True),
        (Circle(Point(0, 0), 1), Circle(Point(-3, 0), 1), Point(3, 0), True),
    ]
)
def test_has_intersection_with_moving_circle(this: Circle, other: Circle, next_position: Point, result):
    assert this.has_intersection_with_moving_circle(other, next_position) == result
