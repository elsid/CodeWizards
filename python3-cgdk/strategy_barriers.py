from strategy_common import Point


def make_circular_barriers(values):
    for value in values:
        yield Circular(position=Point(value.x, value.y), radius=value.radius)


class Circular:
    def __init__(self, position, radius):
        self.position = position
        self.radius = radius

    def __repr__(self):
        return 'Circle(position={p}, radius={r})'.format(
            p=repr(self.position), r=repr(self.radius))

    def __eq__(self, other):
        return (self.position == other.position and
                self.radius == other.radius)

    def has_intersection_with_circular(self, circular):
        return self.position.distance(circular.position) <= self.radius + circular.radius
