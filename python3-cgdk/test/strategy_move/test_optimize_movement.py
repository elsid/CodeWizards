from itertools import chain

from model.Tree import Tree

from strategy_barriers import make_circular_barriers
from strategy_move import optimize_movement, Point, simulate_move, Bounds

from test.common import (
    CircularUnit,
    Game,
    World,
    MAP_SIZE,
    TREE_RADIUS,
    WIZARD_BACKWARD_SPEED,
    WIZARD_FORWARD_SPEED,
    WIZARD_MAX_TURN_ANGLE,
    WIZARD_RADIUS,
    WIZARD_STRAFE_SPEED,
)


def test_optimize_movement():
    position = Point(100, 100)
    angle = 0
    circular_unit = CircularUnit(
        x=position.x,
        y=position.y,
        angle=angle,
        radius=1,
    )
    world = World(
        buildings=tuple(),
        minions=tuple(),
        wizards=tuple(),
        trees=tuple(),
    )
    game = Game(
        map_size=MAP_SIZE,
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
    )
    target = Point(200, 200)
    movements = list(optimize_movement(
        target=target,
        steps=30,
        circular_unit=circular_unit,
        world=world,
        game=game,
    ))
    position, angle, speed = simulate_move(
        position=Point(circular_unit.x, circular_unit.y),
        angle=circular_unit.angle,
        radius=circular_unit.radius,
        movements=movements,
        bounds=Bounds(world=world, game=game),
        barriers=tuple(),
        map_size=game.map_size
    )
    distance = position.distance(target)
    turn = Point(1, 0).rotate(angle).distance((target - position).normalized())
    assert (distance, turn, speed) == (106.26638145082254, 0.10041658565837429, 1.2048886596892028)


def test_optimize_movement_with_barriers():
    position = Point(100, 100)
    angle = 0
    circular_unit = CircularUnit(
        x=position.x,
        y=position.y,
        angle=angle,
        radius=WIZARD_RADIUS,
    )
    world = World(
        buildings=tuple(),
        minions=tuple(),
        trees=[Tree(
            id=None,
            x=position.x + 1.1 * WIZARD_RADIUS + TREE_RADIUS,
            y=position.y + 1.1 * WIZARD_RADIUS + TREE_RADIUS,
            speed_x=None,
            speed_y=None,
            angle=None,
            faction=None,
            radius=TREE_RADIUS,
            life=None,
            max_life=None,
            statuses=None,
        )],
        wizards=tuple(),
    )
    game = Game(
        map_size=MAP_SIZE,
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
    )
    target = Point(200, 200)
    movements = list(optimize_movement(
        target=target,
        steps=30,
        circular_unit=circular_unit,
        world=world,
        game=game,
    ))
    position, angle, speed = simulate_move(
        position=Point(circular_unit.x, circular_unit.y),
        angle=circular_unit.angle,
        radius=circular_unit.radius,
        movements=movements,
        bounds=Bounds(world=world, game=game),
        barriers=list(chain(
            make_circular_barriers(v for v in world.wizards if v != circular_unit),
            make_circular_barriers(world.buildings),
            make_circular_barriers(world.minions),
            make_circular_barriers(world.trees),
        )),
        map_size=game.map_size,
    )
    distance = position.distance(target)
    turn = Point(1, 0).rotate(angle).distance((target - position).normalized())
    assert (distance, turn, speed) == (116.34647039376166, 0.11479053153397464, 1.0605333552847822)
