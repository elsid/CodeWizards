from itertools import chain

from model.Tree import Tree

from strategy_barriers import make_circular_barriers
from strategy_move import optimize_movement, Point, simulate_move, Bounds, State

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
        circular_unit=circular_unit,
        world=world,
        game=game,
        step_sizes=tuple([3] * 3),
        iterations=10,
    ))
    simulation = list(simulate_move(
        position=Point(circular_unit.x, circular_unit.y),
        angle=circular_unit.angle,
        radius=circular_unit.radius,
        movements=movements,
        bounds=Bounds(world=world, game=game),
        barriers=tuple(),
        map_size=game.map_size,
    ))
    distance = simulation[-1].position.distance(target)
    turn = Point(1, 0).rotate(simulation[-1].angle).distance((target - simulation[-1].position).normalized())

    assert (simulation, distance, turn) == ([
        State(position=Point(106.58142618192187, 107.52561361050149), angle=0.3141592653589793, intersection=False),
        State(position=Point(110.51519194000983, 116.71667001132653), angle=0.6283185307179586, intersection=False),
        State(position=Point(111.41665076299647, 126.67290938884074), angle=0.9424777960769379, intersection=False),
    ], 114.9950954586416, 0.25037017627287766)


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
        circular_unit=circular_unit,
        world=world,
        game=game,
        step_sizes=[3] * 3,
        iterations=10,
    ))
    simulation = list(simulate_move(
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
    ))
    distance = simulation[-1].position.distance(target)
    turn = Point(1, 0).rotate(simulation[-1].angle).distance((target - simulation[-1].position).normalized())
    assert (simulation, distance, turn) == ([
        State(position=Point(106.58142618192187, 107.52561361050149), angle=0.3141592653589793, intersection=False),
        State(position=Point(110.51519194000983, 116.71667001132653), angle=0.6283185307179586, intersection=False),
        State(position=Point(111.41665076299647, 126.67290938884074), angle=0.9424777960769379, intersection=False),
    ], 114.9950954586416, 0.25037017627287766)
