from strategy_common import Point, Line


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

    def has_intersection_with_circular(self, circular, delta=1e-8):
        return self.position.distance(circular.position) - self.radius - circular.radius <= delta

    def has_intersection_with_moving_circular(self, circular, next_position, delta=1e-8):
        if self.has_intersection_with_circular(circular):
            return True
        if next_position == circular.position:
            return False
        if self.has_intersection_with_circular(Circular(next_position, circular.radius)):
            return True
        radius_vec = ((next_position - circular.position).normalized().left_orthogonal() *
                      (self.radius + circular.radius))
        line = Line(self.position - radius_vec, self.position + radius_vec)
        circular_line = Line(circular.position, next_position)
        intersection = line.intersection(circular_line)
        return line.has_point(intersection, delta) and circular_line.has_point(intersection, delta)
