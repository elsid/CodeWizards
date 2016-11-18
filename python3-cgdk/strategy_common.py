from math import cos, sin, atan2, pi, hypot
from numpy import arctan2


class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __repr__(self):
        return 'Point({x}, {y})'.format(x=repr(self.x), y=repr(self.y))

    def __eq__(self, other):
        return self.x == other.x and self.y == other.y

    def __add__(self, other):
        if isinstance(other, Point):
            return Point(self.x + other.x, self.y + other.y)
        else:
            return Point(self.x + other, self.y + other)

    def __sub__(self, other):
        if isinstance(other, Point):
            return Point(self.x - other.x, self.y - other.y)
        else:
            return Point(self.x - other, self.y - other)

    def __mul__(self, other):
        if isinstance(other, Point):
            return Point(self.x * other.x, self.y * other.y)
        else:
            return Point(self.x * other, self.y * other)

    def __truediv__(self, other):
        if isinstance(other, Point):
            return Point(self.x / other.x, self.y / other.x)
        else:
            return Point(self.x / other, self.y / other)

    def __neg__(self):
        return Point(-self.x, -self.y)

    def __lt__(self, other):
        if self.x != other.x:
            return self.x < other.x
        return self.y < other.y

    @property
    def radius(self):
        return self.x

    @radius.setter
    def radius(self, value):
        self.x = value

    @property
    def angle(self):
        return self.y

    @angle.setter
    def angle(self, value):
        self.y = value

    def dot(self, other):
        return self.x * other.x + self.y * other.y

    def norm(self):
        return hypot(self.x, self.y)

    def cos(self, other):
        return self.dot(other) / (self.norm() * other.norm())

    def distance(self, other):
        return hypot(other.x - self.x, other.y - self.y)

    def map(self, function):
        return Point(function(self.x), function(self.y))

    def polar(self, cartesian_origin=None):
        if cartesian_origin:
            return (self - cartesian_origin).polar()
        else:
            radius = self.norm()
            angle = arctan2(self.y, self.x)
            return Point(radius, angle)

    def cartesian(self, cartesian_origin=None):
        if cartesian_origin:
            return self.cartesian() + cartesian_origin
        else:
            return Point(x=self.radius * cos(self.angle),
                         y=self.radius * sin(self.angle))

    def left_orthogonal(self):
        return Point(-self.y, self.x)

    def absolute_rotation(self):
        return atan2(self.y, self.x)

    def rotation(self, other):
        return other.absolute_rotation() - self.absolute_rotation()

    def rotate(self, angle):
        return Point(self.x * cos(angle) - self.y * sin(angle),
                     self.y * cos(angle) + self.x * sin(angle))

    def normalized(self):
        return self / self.norm()

    def projection(self, other):
        return other * self.dot(other) / other.norm()

    def manhattan(self, other):
        return abs(self.x - other.x) + abs(self.y - other.y)


def normalize_angle(value):
    if value > pi:
        return value - round(value / (2.0 * pi)) * 2.0 * pi
    if value < -pi:
        return value + round(abs(value) / (2.0 * pi)) * 2.0 * pi
    return value


def lazy_init(method):
    def wrapper(self, *args):
        if not self.initialized:
            self.init(*args)
        return method(self, *args)
    return wrapper


class LazyInit:
    def __init__(self):
        self.__initialized = False

    @property
    def initialized(self):
        return self.__initialized

    def init(self, *args):
        self._init_impl(*args)
        self.__initialized = True

    def _init_impl(self, *args):
        raise NotImplementedError()


class Line:
    def __init__(self, begin: Point, end: Point):
        self.begin = begin
        self.end = end

    def __call__(self, parameter):
        return self.begin + (self.end - self.begin) * parameter

    def __repr__(self):
        return 'Line(begin={b}, end={e})'.format(
            b=repr(self.begin), e=repr(self.end))

    def signed_distance(self, point):
        return (
           (self.begin.y - self.end.y) * point.x
           + (self.end.x - self.begin.x) * point.y
           + (self.begin.x * self.end.y - self.end.x * self.begin.y)
        ) / self.length()

    def distance(self, point):
        return abs(self.signed_distance(point))

    def nearest(self, point):
        to_end = self.end - self.begin
        to_end_norm = to_end.norm()
        if to_end_norm == 0:
            return Point(self.begin.x, self.begin.y)
        to_point = point - self.begin
        return self.begin + to_end * to_point.dot(to_end) / (to_end_norm ** 2)

    def length(self):
        return (self.end - self.begin).norm()

    def has_point(self, point: Point, max_error=1e-8):
        to_end = self.end - point
        if to_end.norm() == 0:
            return True
        to_begin = self.begin - point
        if to_begin.norm() == 0:
            return True
        return abs(1 + to_begin.cos(to_end)) <= max_error

    def __eq__(self, other):
        return self.begin == other.begin and self.end == other.end

    def intersection(self, other):
        x_diff = Point(self.begin.x - self.end.x, other.begin.x - other.end.x)
        y_diff = Point(self.begin.y - self.end.y, other.begin.y - other.end.y)

        def det(a, b):
            return a.x * b.y - a.y * b.x

        div = det(x_diff, y_diff)
        if div == 0:
            return None
        d = Point(det(self.begin, self.end), det(other.begin, other.end))
        x = det(d, x_diff) / div
        y = det(d, y_diff) / div
        return Point(x, y)


class Circle:
    def __init__(self, position, radius):
        self.position = position
        self.radius = radius

    def __repr__(self):
        return 'Circle(position={p}, radius={r})'.format(
            p=repr(self.position), r=repr(self.radius))

    def __eq__(self, other):
        return (self.position == other.position and
                self.radius == other.radius)

    def has_intersection_with_circle(self, circle, max_error=1e-8):
        return self.position.distance(circle.position) <= self.radius + circle.radius + max_error

    def has_intersection_with_line(self, line: Line, max_error=1e-8):
        nearest = line.nearest(self.position)
        return self.position.distance(nearest) - self.radius <= max_error and line.has_point(nearest, max_error)

    def has_intersection_with_moving_circle(self, circle, next_position, max_error=1e-8):
        if self.has_intersection_with_circle(circle):
            return True
        if next_position == circle.position:
            return False
        if self.has_intersection_with_circle(Circle(next_position, circle.radius)):
            return True
        return (
            Circle(self.position, self.radius + circle.radius)
            .has_intersection_with_line(Line(circle.position, next_position), max_error)
        )
