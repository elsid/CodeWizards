from os import environ

from MyStrategy import MyStrategy

from model.Faction import Faction
from model.Game import Game
from model.Move import Move
from model.Wizard import Wizard
from model.World import World

from strategy_common import Point

from test.common import (
    MAP_SIZE,
    WIZARD_BACKWARD_SPEED,
    WIZARD_CAST_RANGE,
    WIZARD_FORWARD_SPEED,
    WIZARD_MAX_LIFE,
    WIZARD_MAX_TURN_ANGLE,
    WIZARD_RADIUS,
    WIZARD_STRAFE_SPEED,
    WIZARD_VISION_RANGE,
)


def test_my_strategy_move():
    environ['FAIL_ON_EXCEPTION'] = '1'
    me = Wizard(
        id=1,
        x=0,
        y=0,
        speed_x=0,
        speed_y=0,
        angle=0,
        faction=Faction.ACADEMY,
        radius=WIZARD_RADIUS,
        life=WIZARD_MAX_LIFE,
        max_life=WIZARD_MAX_LIFE,
        statuses=tuple(),
        owner_player_id=None,
        me=None,
        mana=None,
        max_mana=0,
        vision_range=WIZARD_VISION_RANGE,
        cast_range=WIZARD_CAST_RANGE,
        xp=None,
        level=None,
        skills=tuple(),
        remaining_action_cooldown_ticks=None,
        remaining_cooldown_ticks_by_action=tuple(),
        master=None,
        messages=tuple(),
    )
    setattr(me, 'mean_speed', Point(0, 0))
    world = World(
        tick_index=0,
        tick_count=None,
        width=MAP_SIZE,
        height=MAP_SIZE,
        players=None,
        wizards=[me],
        minions=tuple(),
        projectiles=tuple(),
        bonuses=tuple(),
        buildings=tuple(),
        trees=tuple(),
    )
    game = Game(
        random_seed=None,
        tick_count=None,
        map_size=MAP_SIZE,
        skills_enabled=None,
        raw_messages_enabled=None,
        friendly_fire_damage_factor=None,
        building_damage_score_factor=None,
        building_elimination_score_factor=None,
        minion_damage_score_factor=None,
        minion_elimination_score_factor=None,
        wizard_damage_score_factor=None,
        wizard_elimination_score_factor=None,
        team_working_score_factor=None,
        victory_score=None,
        score_gain_range=None,
        raw_message_max_length=None,
        raw_message_transmission_speed=None,
        wizard_radius=WIZARD_RADIUS,
        wizard_cast_range=WIZARD_CAST_RANGE,
        wizard_vision_range=None,
        wizard_forward_speed=WIZARD_FORWARD_SPEED,
        wizard_backward_speed=WIZARD_BACKWARD_SPEED,
        wizard_strafe_speed=WIZARD_STRAFE_SPEED,
        wizard_base_life=None,
        wizard_life_growth_per_level=None,
        wizard_base_mana=None,
        wizard_mana_growth_per_level=None,
        wizard_base_life_regeneration=None,
        wizard_life_regeneration_growth_per_level=None,
        wizard_base_mana_regeneration=None,
        wizard_mana_regeneration_growth_per_level=None,
        wizard_max_turn_angle=WIZARD_MAX_TURN_ANGLE,
        wizard_max_resurrection_delay_ticks=None,
        wizard_min_resurrection_delay_ticks=None,
        wizard_action_cooldown_ticks=None,
        staff_cooldown_ticks=None,
        magic_missile_cooldown_ticks=None,
        frost_bolt_cooldown_ticks=None,
        fireball_cooldown_ticks=None,
        haste_cooldown_ticks=None,
        shield_cooldown_ticks=None,
        magic_missile_manacost=None,
        frost_bolt_manacost=None,
        fireball_manacost=None,
        haste_manacost=None,
        shield_manacost=None,
        staff_damage=None,
        staff_sector=None,
        staff_range=None,
        level_up_xp_values=None,
        minion_radius=None,
        minion_vision_range=None,
        minion_speed=None,
        minion_max_turn_angle=None,
        minion_life=None,
        faction_minion_appearance_interval_ticks=None,
        orc_woodcutter_action_cooldown_ticks=None,
        orc_woodcutter_damage=None,
        orc_woodcutter_attack_sector=None,
        orc_woodcutter_attack_range=None,
        fetish_blowdart_action_cooldown_ticks=None,
        fetish_blowdart_attack_range=None,
        fetish_blowdart_attack_sector=None,
        bonus_radius=None,
        bonus_appearance_interval_ticks=None,
        bonus_score_amount=None,
        dart_radius=None,
        dart_speed=None,
        dart_direct_damage=None,
        magic_missile_radius=None,
        magic_missile_speed=None,
        magic_missile_direct_damage=None,
        frost_bolt_radius=None,
        frost_bolt_speed=None,
        frost_bolt_direct_damage=None,
        fireball_radius=None,
        fireball_speed=None,
        fireball_explosion_max_damage_range=None,
        fireball_explosion_min_damage_range=None,
        fireball_explosion_max_damage=None,
        fireball_explosion_min_damage=None,
        guardian_tower_radius=None,
        guardian_tower_vision_range=None,
        guardian_tower_life=None,
        guardian_tower_attack_range=None,
        guardian_tower_damage=None,
        guardian_tower_cooldown_ticks=None,
        faction_base_radius=None,
        faction_base_vision_range=None,
        faction_base_life=None,
        faction_base_attack_range=None,
        faction_base_damage=None,
        faction_base_cooldown_ticks=None,
        burning_duration_ticks=None,
        burning_summary_damage=None,
        empowered_duration_ticks=None,
        empowered_damage_factor=None,
        frozen_duration_ticks=None,
        hastened_duration_ticks=None,
        hastened_bonus_duration_factor=None,
        hastened_movement_bonus_factor=None,
        hastened_rotation_bonus_factor=None,
        shielded_duration_ticks=None,
        shielded_bonus_duration_factor=None,
        shielded_direct_damage_absorption_factor=None,
        aura_skill_range=None,
        range_bonus_per_skill_level=None,
        magical_damage_bonus_per_skill_level=None,
        staff_damage_bonus_per_skill_level=None,
        movement_bonus_factor_per_skill_level=None,
        magical_damage_absorption_per_skill_level=None,
    )
    move = Move()
    my_strategy = MyStrategy()
    my_strategy.move(
        me=me,
        world=world,
        game=game,
        move=move,
    )
