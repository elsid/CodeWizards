from model.Bonus import Bonus
from model.Faction import Faction
from model.Wizard import Wizard

from strategy_common import Point
from strategy_path import select_destination, make_graph


def test_select_destination_to_enemy_wizard():
    graph = make_graph(4000)
    me = Wizard(
        id=None,
        x=2000,
        y=2000,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.ACADEMY,
        radius=None,
        life=None,
        max_life=None,
        statuses=None,
        owner_player_id=None,
        me=None,
        mana=None,
        max_mana=None,
        vision_range=None,
        cast_range=None,
        xp=None,
        level=None,
        skills=None,
        remaining_action_cooldown_ticks=None,
        remaining_cooldown_ticks_by_action=None,
        master=None,
        messages=None,
    )
    setattr(me, 'position', Point(me.x, me.y))
    wizard = Wizard(
        id=None,
        x=500,
        y=500,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.RENEGADES,
        radius=None,
        life=None,
        max_life=None,
        statuses=None,
        owner_player_id=None,
        me=None,
        mana=None,
        max_mana=None,
        vision_range=None,
        cast_range=None,
        xp=None,
        level=None,
        skills=None,
        remaining_action_cooldown_ticks=None,
        remaining_cooldown_ticks_by_action=None,
        master=None,
        messages=None,
    )
    setattr(wizard, 'position', Point(wizard.x, wizard.y))
    destination = select_destination(
        graph=graph,
        me=me,
        buildings=tuple(),
        minions=tuple(),
        wizards=[me, wizard],
        bonuses=tuple(),
        target_lane=tuple(),
    )
    destination = destination.position
    assert destination == Point(600, 600)
    assert destination.distance(wizard.position) <= graph.zone_size


def test_select_destination_to_nearest_enemy_wizard():
    graph = make_graph(4000)
    me = Wizard(
        id=None,
        x=500,
        y=500,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.ACADEMY,
        radius=None,
        life=None,
        max_life=None,
        statuses=None,
        owner_player_id=None,
        me=None,
        mana=None,
        max_mana=None,
        vision_range=None,
        cast_range=None,
        xp=None,
        level=None,
        skills=None,
        remaining_action_cooldown_ticks=None,
        remaining_cooldown_ticks_by_action=None,
        master=None,
        messages=None,
    )
    setattr(me, 'position', Point(me.x, me.y))
    far_wizard = Wizard(
        id=None,
        x=3500,
        y=3500,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.RENEGADES,
        radius=None,
        life=None,
        max_life=None,
        statuses=None,
        owner_player_id=None,
        me=None,
        mana=None,
        max_mana=None,
        vision_range=None,
        cast_range=None,
        xp=None,
        level=None,
        skills=None,
        remaining_action_cooldown_ticks=None,
        remaining_cooldown_ticks_by_action=None,
        master=None,
        messages=None,
    )
    setattr(far_wizard, 'position', Point(far_wizard.x, far_wizard.y))
    near_wizard = Wizard(
        id=None,
        x=3500,
        y=500,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.RENEGADES,
        radius=None,
        life=None,
        max_life=None,
        statuses=None,
        owner_player_id=None,
        me=None,
        mana=None,
        max_mana=None,
        vision_range=None,
        cast_range=None,
        xp=None,
        level=None,
        skills=None,
        remaining_action_cooldown_ticks=None,
        remaining_cooldown_ticks_by_action=None,
        master=None,
        messages=None,
    )
    setattr(near_wizard, 'position', Point(near_wizard.x, near_wizard.y))
    destination = select_destination(
        graph=graph,
        me=me,
        buildings=tuple(),
        minions=tuple(),
        wizards=[me, far_wizard, near_wizard],
        bonuses=tuple(),
        target_lane=tuple(),
    )
    destination = destination.position
    assert destination == Point(3000, 200)
    assert destination.distance(near_wizard.position) <= graph.zone_size


def test_select_destination_to_bonus():
    graph = make_graph(4000)
    me = Wizard(
        id=None,
        x=2000,
        y=2000,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.ACADEMY,
        radius=None,
        life=None,
        max_life=None,
        statuses=None,
        owner_player_id=None,
        me=None,
        mana=None,
        max_mana=None,
        vision_range=None,
        cast_range=None,
        xp=None,
        level=None,
        skills=None,
        remaining_action_cooldown_ticks=None,
        remaining_cooldown_ticks_by_action=None,
        master=None,
        messages=None,
    )
    setattr(me, 'position', Point(me.x, me.y))
    bonus = Bonus(
        id=None,
        x=1200,
        y=1200,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=None,
        radius=None,
        type=None,
    )
    setattr(bonus, 'position', Point(bonus.x, bonus.y))
    destination = select_destination(
        graph=graph,
        me=me,
        buildings=tuple(),
        minions=tuple(),
        wizards=[me],
        bonuses=[bonus],
        target_lane=tuple(),
    )
    destination = destination.position
    assert destination == Point(1400, 1400)
    assert destination.distance(bonus.position) <= graph.zone_size


def test_select_destination_to_bonus_instead_of_enemy():
    graph = make_graph(4000)
    me = Wizard(
        id=None,
        x=2000,
        y=2000,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.ACADEMY,
        radius=None,
        life=None,
        max_life=None,
        statuses=None,
        owner_player_id=None,
        me=None,
        mana=None,
        max_mana=None,
        vision_range=None,
        cast_range=None,
        xp=None,
        level=None,
        skills=None,
        remaining_action_cooldown_ticks=None,
        remaining_cooldown_ticks_by_action=None,
        master=None,
        messages=None,
    )
    setattr(me, 'position', Point(me.x, me.y))
    bonus = Bonus(
        id=None,
        x=1200,
        y=1200,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=None,
        radius=None,
        type=None,
    )
    setattr(bonus, 'position', Point(bonus.x, bonus.y))
    wizard = Wizard(
        id=None,
        x=2800,
        y=2800,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.RENEGADES,
        radius=None,
        life=None,
        max_life=None,
        statuses=None,
        owner_player_id=None,
        me=None,
        mana=None,
        max_mana=None,
        vision_range=None,
        cast_range=None,
        xp=None,
        level=None,
        skills=None,
        remaining_action_cooldown_ticks=None,
        remaining_cooldown_ticks_by_action=None,
        master=None,
        messages=None,
    )
    setattr(wizard, 'position', Point(wizard.x, wizard.y))
    destination = select_destination(
        graph=graph,
        me=me,
        buildings=tuple(),
        minions=tuple(),
        wizards=[me, wizard],
        bonuses=[bonus],
        target_lane=tuple(),
    )
    destination = destination.position
    assert destination == Point(1400, 1400)
    assert destination.distance(bonus.position) <= graph.zone_size


def test_select_destination_to_enemy_instead_of_bonus():
    graph = make_graph(4000)
    me = Wizard(
        id=None,
        x=2100,
        y=2100,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.ACADEMY,
        radius=None,
        life=None,
        max_life=None,
        statuses=None,
        owner_player_id=None,
        me=None,
        mana=None,
        max_mana=None,
        vision_range=None,
        cast_range=None,
        xp=None,
        level=None,
        skills=None,
        remaining_action_cooldown_ticks=None,
        remaining_cooldown_ticks_by_action=None,
        master=None,
        messages=None,
    )
    setattr(me, 'position', Point(me.x, me.y))
    bonus = Bonus(
        id=None,
        x=1200,
        y=1200,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=None,
        radius=None,
        type=None,
    )
    setattr(bonus, 'position', Point(bonus.x, bonus.y))
    wizard = Wizard(
        id=None,
        x=2200,
        y=2200,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.RENEGADES,
        radius=None,
        life=None,
        max_life=None,
        statuses=None,
        owner_player_id=None,
        me=None,
        mana=None,
        max_mana=None,
        vision_range=None,
        cast_range=None,
        xp=None,
        level=None,
        skills=None,
        remaining_action_cooldown_ticks=None,
        remaining_cooldown_ticks_by_action=None,
        master=None,
        messages=None,
    )
    setattr(wizard, 'position', Point(wizard.x, wizard.y))
    destination = select_destination(
        graph=graph,
        me=me,
        buildings=tuple(),
        minions=tuple(),
        wizards=[me, wizard],
        bonuses=[bonus],
        target_lane=tuple(),
    )
    destination = destination.position
    assert destination == Point(2200, 2200)
    assert destination.distance(wizard.position) <= graph.zone_size


def test_select_destination_to_enemy_wizard_near_friend_base():
    graph = make_graph(4000)
    me = Wizard(
        id=None,
        x=2500,
        y=2500,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.ACADEMY,
        radius=None,
        life=None,
        max_life=None,
        statuses=None,
        owner_player_id=None,
        me=None,
        mana=None,
        max_mana=None,
        vision_range=None,
        cast_range=None,
        xp=None,
        level=None,
        skills=None,
        remaining_action_cooldown_ticks=None,
        remaining_cooldown_ticks_by_action=None,
        master=None,
        messages=None,
    )
    setattr(me, 'position', Point(me.x, me.y))
    far_wizard = Wizard(
        id=None,
        x=3000,
        y=3000,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.RENEGADES,
        radius=None,
        life=None,
        max_life=None,
        statuses=None,
        owner_player_id=None,
        me=None,
        mana=None,
        max_mana=None,
        vision_range=None,
        cast_range=None,
        xp=None,
        level=None,
        skills=None,
        remaining_action_cooldown_ticks=None,
        remaining_cooldown_ticks_by_action=None,
        master=None,
        messages=None,
    )
    setattr(far_wizard, 'position', Point(far_wizard.x, far_wizard.y))
    near_wizard = Wizard(
        id=None,
        x=graph.friend_base.position.x,
        y=graph.friend_base.position.y,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.RENEGADES,
        radius=None,
        life=None,
        max_life=None,
        statuses=None,
        owner_player_id=None,
        me=None,
        mana=None,
        max_mana=None,
        vision_range=None,
        cast_range=None,
        xp=None,
        level=None,
        skills=None,
        remaining_action_cooldown_ticks=None,
        remaining_cooldown_ticks_by_action=None,
        master=None,
        messages=None,
    )
    setattr(near_wizard, 'position', Point(near_wizard.x, near_wizard.y))
    destination = select_destination(
        graph=graph,
        me=me,
        buildings=tuple(),
        minions=tuple(),
        wizards=[me, far_wizard, near_wizard],
        bonuses=tuple(),
        target_lane=tuple(),
    )
    destination = destination.position
    assert destination == Point(200, 3800)
    assert destination.distance(near_wizard.position) <= graph.zone_size


def test_select_destination_not_to_enemy_wizard_near_friend_base():
    graph = make_graph(4000)
    me = Wizard(
        id=None,
        x=graph.enemy_base.position.x,
        y=graph.enemy_base.position.y,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.ACADEMY,
        radius=None,
        life=None,
        max_life=None,
        statuses=None,
        owner_player_id=None,
        me=None,
        mana=None,
        max_mana=None,
        vision_range=None,
        cast_range=None,
        xp=None,
        level=None,
        skills=None,
        remaining_action_cooldown_ticks=None,
        remaining_cooldown_ticks_by_action=None,
        master=None,
        messages=None,
    )
    setattr(me, 'position', Point(me.x, me.y))
    near_wizard = Wizard(
        id=None,
        x=graph.friend_base.position.x,
        y=graph.friend_base.position.y,
        speed_x=None,
        speed_y=None,
        angle=None,
        faction=Faction.RENEGADES,
        radius=None,
        life=None,
        max_life=None,
        statuses=None,
        owner_player_id=None,
        me=None,
        mana=None,
        max_mana=None,
        vision_range=None,
        cast_range=None,
        xp=None,
        level=None,
        skills=None,
        remaining_action_cooldown_ticks=None,
        remaining_cooldown_ticks_by_action=None,
        master=None,
        messages=None,
    )
    setattr(near_wizard, 'position', Point(near_wizard.x, near_wizard.y))
    destination = select_destination(
        graph=graph,
        me=me,
        buildings=tuple(),
        minions=tuple(),
        wizards=[me, near_wizard],
        bonuses=tuple(),
        target_lane=tuple(),
    )
    destination = destination.position
    assert destination == Point(600, 3400)
    assert destination.distance(near_wizard.position) <= graph.zone_size
