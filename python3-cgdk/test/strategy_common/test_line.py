import pytest

from strategy_common import Point, Line


@pytest.mark.parametrize(
    ('line', 'point', 'distance'), [
        (Line(Point(0, 0), Point(1, 0)), Point(0, 0), 0),
        (Line(Point(0, 0), Point(1, 0)), Point(1, 0), 0),
        (Line(Point(0, 0), Point(1, 0)), Point(0.5, 0), 0),
        (Line(Point(0, 0), Point(1, 0)), Point(0, 1), 1),
        (Line(Point(0, 0), Point(1, 0)), Point(1, 1), 1),
        (Line(Point(0, 0), Point(1, 0)), Point(0.5, 1), 1),
        (Line(Point(0, 0), Point(1, 0)), Point(0, -1), -1),
        (Line(Point(0, 0), Point(1, 0)), Point(1, -1), -1),
        (Line(Point(0, 0), Point(1, 0)), Point(0.5, -1), -1),
        (Line(Point(0, 0), Point(1, 0)), Point(-1, 1), 1),
        (Line(Point(0, 0), Point(1, 0)), Point(2, 1), 1),
        (Line(Point(0, 0), Point(1, 0)), Point(-1, -1), -1),
        (Line(Point(0, 0), Point(1, 0)), Point(2, -1), -1),
        (Line(Point(0, 0), Point(0, 1)), Point(0, 0), 0),
        (Line(Point(0, 0), Point(0, 1)), Point(0, 1), 0),
        (Line(Point(0, 0), Point(0, 1)), Point(0, 0.5), 0),
        (Line(Point(0, 0), Point(0, 1)), Point(1, 0), -1),
        (Line(Point(0, 0), Point(0, 1)), Point(1, 1), -1),
        (Line(Point(0, 0), Point(0, 1)), Point(1, 0.5), -1),
        (Line(Point(0, 0), Point(0, 1)), Point(-1, 0), 1),
        (Line(Point(0, 0), Point(0, 1)), Point(-1, 1), 1),
        (Line(Point(0, 0), Point(0, 1)), Point(-1, 0.5), 1),
        (Line(Point(0, 0), Point(0, 1)), Point(1, -1), -1),
        (Line(Point(0, 0), Point(0, 1)), Point(1, 2), -1),
        (Line(Point(0, 0), Point(0, 1)), Point(-1, -1), 1),
        (Line(Point(0, 0), Point(0, 1)), Point(-1, 2), 1),
        (Line(Point(-1, -1), Point(1, 1)), Point(0, 0), 0),
        (Line(Point(-1, -1), Point(1, 1)), Point(1, -1), -1.414213562373095),
        (Line(Point(-1, -1), Point(1, 1)), Point(-1, 1), 1.414213562373095),
        (Line(Point(1, 1), Point(2, 4)), Point(-1, -2), 0.9486832980505138),
        (Line(Point(1275, 1000), Point(1170.3642964470298, 1033.863329353748)),
         Point(907.57447509095073, 1118.9101916505024), 0),
    ]
)
def test_signed_distance(line: Line, point: Point, distance: float):
    assert line.signed_distance(point) == distance


@pytest.mark.parametrize(
    ('line', 'point', 'distance'), [
        (Line(Point(0, 0), Point(1, 0)), Point(0, 1), 1),
        (Line(Point(0, 0), Point(1, 0)), Point(0, -1), 1),
    ]
)
def test_distance(line: Line, point: Point, distance: float):
    assert line.distance(point) == distance


@pytest.mark.parametrize(
    ('line', 'point', 'nearest'), [
        (Line(Point(0, 0), Point(1, 0)), Point(0, 0), Point(0, 0)),
        (Line(Point(0, 0), Point(1, 0)), Point(2, 0), Point(2, 0)),
        (Line(Point(0, 0), Point(1, 0)), Point(1, 1), Point(1, 0)),
        (Line(Point(0, 0), Point(1, 0)), Point(2, 1), Point(2, 0)),
        (Line(Point(0, 0), Point(0, 0)), Point(1, 0), Point(0, 0)),
    ]
)
def test_nearest(line, point, nearest):
    assert line.nearest(point) == nearest
