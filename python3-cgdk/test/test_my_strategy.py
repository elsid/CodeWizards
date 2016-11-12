from MyStrategy import MyStrategy

from model.Move import Move

from test.common import (
    Game,
    Wizard,
    World,
    MAP_SIZE,
    WIZARD_BACKWARD_SPEED,
    WIZARD_FORWARD_SPEED,
    WIZARD_MAX_TURN_ANGLE,
    WIZARD_STRAFE_SPEED,
)


def test_my_strategy_move():
    me = Wizard(
        x=0,
        y=0,
        angle=0,
        radius=1,
    )
    world = World(
        buildings=tuple(),
        minions=tuple(),
        trees=tuple(),
        wizards=tuple(),
    )
    game = Game(
        map_size=MAP_SIZE,
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
    )
    move = Move()
    my_strategy = MyStrategy()
    my_strategy.move(
        me=me,
        world=world,
        game=game,
        move=move,
    )
