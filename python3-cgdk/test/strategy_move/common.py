from collections import namedtuple
from math import pi

CircularUnit = namedtuple('CircularUnit', (
    'angle',
    'radius',
    'x',
    'y',
))
World = namedtuple('World', (
    'buildings',
    'minions',
    'trees',
    'wizards',
))
Game = namedtuple('Game', (
    'wizard_backward_speed',
    'wizard_forward_speed',
    'wizard_max_turn_angle',
    'wizard_strafe_speed',
))

TREE_RADIUS = 20
WIZARD_BACKWARD_SPEED = -3.0
WIZARD_FORWARD_SPEED = 4.0
WIZARD_MAX_TURN_ANGLE = pi / 30
WIZARD_RADIUS = 35
WIZARD_STRAFE_SPEED = 3.0
