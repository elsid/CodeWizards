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
