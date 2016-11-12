from strategy_move import optimize_movement, Point, Movement, simulate_move, Bounds

from .common import (
    Wizard,
    World,
    Game,
    WIZARD_BACKWARD_SPEED,
    WIZARD_FORWARD_SPEED,
    WIZARD_MAX_TURN_ANGLE,
    WIZARD_STRAFE_SPEED,
)


def test_optimize_movement():
    position = Point(0, 0)
    angle = 0
    player = Wizard(x=position.x, y=position.y, angle=angle)
    world = World()
    game = Game(
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
    )
    target = Point(100, 100)
    movements = list(optimize_movement(target=target, steps=30, player=player, world=world, game=game))
    assert movements == [
        Movement(speed=2.8301115915376882, strafe_speed=3.0, turn=-0.056319965688474513),
        Movement(speed=2.8427562397003432, strafe_speed=3.0, turn=-0.053455883713349112),
        Movement(speed=2.8551397408727173, strafe_speed=3.0, turn=-0.05075058331999277),
        Movement(speed=2.8672548644503419, strafe_speed=2.9965975075441325, turn=-0.040513580210451976),
        Movement(speed=2.8790955624348831, strafe_speed=2.989790296660908, turn=-0.031170711258157666),
        Movement(speed=2.8906537338547342, strafe_speed=2.9828230521276518, turn=-0.022755111678669487),
        Movement(speed=2.9019229958331421, strafe_speed=2.9757014200888525, turn=-0.015075844927845607),
        Movement(speed=2.9128949147762602, strafe_speed=2.9684277321698214, turn=-0.0079477191799826249),
        Movement(speed=2.9235627775979669, strafe_speed=2.9610087028161853, turn=-0.0011924725845452401),
        Movement(speed=2.9339202631093468, strafe_speed=2.9534466893572295, turn=0.0053635375865648286),
        Movement(speed=2.9439625582579985, strafe_speed=2.9457484310265496, turn=0.011889689803779675),
        Movement(speed=2.9536831489550162, strafe_speed=2.9379151303841451, turn=0.01855213870144487),
        Movement(speed=2.9630675804300854, strafe_speed=2.9299535794221723, turn=0.025515224781364355),
        Movement(speed=2.9721228472615939, strafe_speed=2.9218671845772075, turn=0.032941481511551365),
        Movement(speed=2.9808327974104762, strafe_speed=2.9136615944052302, turn=0.040992522254976391),
        Movement(speed=2.9891963543434876, strafe_speed=2.9053413643731152, turn=0.049773009747640901),
        Movement(speed=2.9972089449409185, strafe_speed=2.896912145303463, turn=0.053195256721607999),
        Movement(speed=3.0048635137929525, strafe_speed=2.8883784825357868, turn=0.055478628324589822),
        Movement(speed=3.0121530671138448, strafe_speed=2.879746031425928, turn=0.057668501226065941),
        Movement(speed=3.019071193246035, strafe_speed=2.8710171042604813, turn=0.058134061725254683),
        Movement(speed=3.0256020221978011, strafe_speed=2.862199582648346, turn=0.058593832197965656),
        Movement(speed=3.0317331573676376, strafe_speed=2.8532969145964193, turn=0.059047689075736043),
        Movement(speed=3.0374716768262151, strafe_speed=2.8443147430665645, turn=0.059495522536319366),
        Movement(speed=3.0428030301866404, strafe_speed=2.8352598261964879, turn=0.059937116659792768),
        Movement(speed=3.0477245735110698, strafe_speed=2.8258314993265174, turn=0.060372282537264919),
        Movement(speed=3.0522434302309889, strafe_speed=2.8139215795891483, turn=0.060800730030323567),
        Movement(speed=3.0563433461417953, strafe_speed=2.8019341432459894, turn=0.061222188006518997),
        Movement(speed=3.0600273583703954, strafe_speed=2.7898792891302349, turn=0.061636291339430577),
        Movement(speed=3.063292727390182, strafe_speed=2.7777614719889638, turn=0.062042637575819896),
        Movement(speed=3.0661368611221378, strafe_speed=2.7655894920150885, turn=0.062440847801653793),
    ]
    position, angle = simulate_move(
        position=Point(player.x, player.y),
        angle=player.angle,
        movements=movements,
        bounds=Bounds(world=world, game=game),
    )
    distance = position.distance(target)
    turn = Point(1, 0).rotate(angle).distance((target - position).normalized())
    assert (distance, turn) == (106.72076745809267, 0.00038266347196457874)
