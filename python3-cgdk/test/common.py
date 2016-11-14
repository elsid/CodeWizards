from collections import namedtuple
from math import pi

CircularUnit = namedtuple('CircularUnit', (
    'angle',
    'radius',
    'x',
    'y',
))
Wizard = namedtuple('Wizard', (
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
    'map_size',
    'wizard_backward_speed',
    'wizard_forward_speed',
    'wizard_max_turn_angle',
    'wizard_strafe_speed',
))

FACTION_BASE_ATTACK_RANGE = 800
FETISH_BLOWDART_ATTACK_RANGE = 300
GUARDIAN_TOWER_ATTACK_RANGE = 600
MAP_SIZE = 4000
MINION_RADIUS = 5
ORC_WOODCUTTER_ATTACK_RANGE = 50
TREE_RADIUS = 5
WIZARD_BACKWARD_SPEED = 3.0
WIZARD_CAST_RANGE = 600
WIZARD_FORWARD_SPEED = 4.0
WIZARD_MAX_LIFE = 100
WIZARD_MAX_TURN_ANGLE = pi / 30
WIZARD_RADIUS = 35
WIZARD_STRAFE_SPEED = 3.0
