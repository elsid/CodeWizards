from model.Tree import Tree

from strategy_move import optimize_movement, Point

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
    assert len(movements) == 3


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
    assert len(movements) == 3
