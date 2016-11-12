from itertools import chain

from model.Tree import Tree

from strategy_barriers import make_circular_barriers
from strategy_move import optimize_movement, Point, Movement, simulate_move, Bounds

from .common import (
    Wizard,
    World,
    Game,
    TREE_RADIUS,
    WIZARD_BACKWARD_SPEED,
    WIZARD_FORWARD_SPEED,
    WIZARD_MAX_TURN_ANGLE,
    WIZARD_RADIUS,
    WIZARD_STRAFE_SPEED,
)


def test_optimize_movement():
    position = Point(0, 0)
    angle = 0
    player = Wizard(
        x=position.x,
        y=position.y,
        angle=angle,
        radius=1,
    )
    world = World(
        buildings=tuple(),
        minions=tuple(),
        players=tuple(),
        trees=tuple(),
    )
    game = Game(
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
    )
    target = Point(100, 100)
    movements = list(optimize_movement(target=target, steps=30, player=player, world=world, game=game))
    position, angle, speed = simulate_move(
        position=Point(player.x, player.y),
        angle=player.angle,
        radius=player.radius,
        movements=movements,
        bounds=Bounds(world=world, game=game),
        barriers=tuple(),
    )
    distance = position.distance(target)
    turn = Point(1, 0).rotate(angle).distance((target - position).normalized())
    assert (distance, turn, speed) == (106.26652817043892, 0.10038679287562888, 1.2048797897518173)


def test_optimize_movement_with_barriers():
    position = Point(0, 0)
    angle = 0
    player = Wizard(
        x=position.x,
        y=position.y,
        angle=angle,
        radius=WIZARD_RADIUS,
    )
    world = World(
        buildings=tuple(),
        minions=tuple(),
        players=tuple(),
        trees=[Tree(
            id=None,
            x=WIZARD_RADIUS + TREE_RADIUS + 1,
            y=WIZARD_RADIUS + TREE_RADIUS + 1,
            speed_x=None,
            speed_y=None,
            angle=None,
            faction=None,
            radius=TREE_RADIUS,
            life=None,
            max_life=None,
            statuses=None,
        )],
    )
    game = Game(
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
    )
    target = Point(100, 100)
    movements = list(optimize_movement(target=target, steps=30, player=player, world=world, game=game))
    position, angle, speed = simulate_move(
        position=Point(player.x, player.y),
        angle=player.angle,
        radius=player.radius,
        movements=movements,
        bounds=Bounds(world=world, game=game),
        barriers=list(chain(
            make_circular_barriers(v for v in world.players if v != player),
            make_circular_barriers(world.buildings),
            make_circular_barriers(world.minions),
            make_circular_barriers(world.trees),
        )),
    )
    distance = position.distance(target)
    turn = Point(1, 0).rotate(angle).distance((target - position).normalized())
    assert (distance, turn, speed) == (116.45658786302063, 0.08511991403938413, 1.0693995568413381)
