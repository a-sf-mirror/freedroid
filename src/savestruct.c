#include "savestruct.h"

void write_enemy(struct auto_string *strout, enemy *data)
{
    autostr_append(strout, "{\n");
    autostr_append(strout, "id = ");
    write_int16_t(strout, &data->id);
    autostr_append(strout, ",\n");
    autostr_append(strout, "type = ");
    write_int16_t(strout, &data->type);
    autostr_append(strout, ",\n");
    autostr_append(strout, "SpecialForce = ");
    write_uint8_t(strout, &data->SpecialForce);
    autostr_append(strout, ",\n");
    autostr_append(strout, "marker = ");
    write_int32_t(strout, &data->marker);
    autostr_append(strout, ",\n");
    autostr_append(strout, "max_distance_to_home = ");
    write_int16_t(strout, &data->max_distance_to_home);
    autostr_append(strout, ",\n");
    autostr_append(strout, "dialog_section_name = ");
    write_string(strout, &data->dialog_section_name);
    autostr_append(strout, ",\n");
    autostr_append(strout, "short_description_text = ");
    write_string(strout, &data->short_description_text);
    autostr_append(strout, ",\n");
    autostr_append(strout, "on_death_drop_item_code = ");
    write_int16_t(strout, &data->on_death_drop_item_code);
    autostr_append(strout, ",\n");
    autostr_append(strout, "faction = ");
    write_int32_t(strout, &data->faction);
    autostr_append(strout, ",\n");
    autostr_append(strout, "will_rush_tux = ");
    write_uint8_t(strout, &data->will_rush_tux);
    autostr_append(strout, ",\n");
    autostr_append(strout, "combat_state = ");
    write_int32_t(strout, &data->combat_state);
    autostr_append(strout, ",\n");
    autostr_append(strout, "state_timeout = ");
    write_float(strout, &data->state_timeout);
    autostr_append(strout, ",\n");
    autostr_append(strout, "CompletelyFixed = ");
    write_int16_t(strout, &data->CompletelyFixed);
    autostr_append(strout, ",\n");
    autostr_append(strout, "has_been_taken_over = ");
    write_uint8_t(strout, &data->has_been_taken_over);
    autostr_append(strout, ",\n");
    autostr_append(strout, "follow_tux = ");
    write_uint8_t(strout, &data->follow_tux);
    autostr_append(strout, ",\n");
    autostr_append(strout, "has_greeted_influencer = ");
    write_uint8_t(strout, &data->has_greeted_influencer);
    autostr_append(strout, ",\n");
    autostr_append(strout, "nextwaypoint = ");
    write_int16_t(strout, &data->nextwaypoint);
    autostr_append(strout, ",\n");
    autostr_append(strout, "lastwaypoint = ");
    write_int16_t(strout, &data->lastwaypoint);
    autostr_append(strout, ",\n");
    autostr_append(strout, "homewaypoint = ");
    write_int16_t(strout, &data->homewaypoint);
    autostr_append(strout, ",\n");
    autostr_append(strout, "pos = ");
    write_gps(strout, &data->pos);
    autostr_append(strout, ",\n");
    autostr_append(strout, "speed = ");
    write_finepoint(strout, &data->speed);
    autostr_append(strout, ",\n");
    autostr_append(strout, "energy = ");
    write_float(strout, &data->energy);
    autostr_append(strout, ",\n");
    autostr_append(strout, "animation_phase = ");
    write_float(strout, &data->animation_phase);
    autostr_append(strout, ",\n");
    autostr_append(strout, "animation_type = ");
    write_int16_t(strout, &data->animation_type);
    autostr_append(strout, ",\n");
    autostr_append(strout, "frozen = ");
    write_float(strout, &data->frozen);
    autostr_append(strout, ",\n");
    autostr_append(strout, "poison_duration_left = ");
    write_float(strout, &data->poison_duration_left);
    autostr_append(strout, ",\n");
    autostr_append(strout, "poison_damage_per_sec = ");
    write_float(strout, &data->poison_damage_per_sec);
    autostr_append(strout, ",\n");
    autostr_append(strout, "paralysation_duration_left = ");
    write_float(strout, &data->paralysation_duration_left);
    autostr_append(strout, ",\n");
    autostr_append(strout, "pure_wait = ");
    write_float(strout, &data->pure_wait);
    autostr_append(strout, ",\n");
    autostr_append(strout, "firewait = ");
    write_float(strout, &data->firewait);
    autostr_append(strout, ",\n");
    autostr_append(strout, "ammo_left = ");
    write_int16_t(strout, &data->ammo_left);
    autostr_append(strout, ",\n");
    autostr_append(strout, "attack_target_type = ");
    write_uint8_t(strout, &data->attack_target_type);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bot_target_n = ");
    write_int16_t(strout, &data->bot_target_n);
    autostr_append(strout, ",\n");
    autostr_append(strout, "previous_angle = ");
    write_float(strout, &data->previous_angle);
    autostr_append(strout, ",\n");
    autostr_append(strout, "current_angle = ");
    write_float(strout, &data->current_angle);
    autostr_append(strout, ",\n");
    autostr_append(strout, "previous_phase = ");
    write_float(strout, &data->previous_phase);
    autostr_append(strout, ",\n");
    autostr_append(strout, "last_phase_change = ");
    write_float(strout, &data->last_phase_change);
    autostr_append(strout, ",\n");
    autostr_append(strout, "last_combat_step = ");
    write_float(strout, &data->last_combat_step);
    autostr_append(strout, ",\n");
    autostr_append(strout, "TextVisibleTime = ");
    write_float(strout, &data->TextVisibleTime);
    autostr_append(strout, ",\n");
    autostr_append(strout, "PrivatePathway = ");
    write_moderately_finepoint_array(strout, data->PrivatePathway, 5);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bot_stuck_in_wall_at_previous_check = ");
    write_uint8_t(strout, &data->bot_stuck_in_wall_at_previous_check);
    autostr_append(strout, ",\n");
    autostr_append(strout, "time_since_previous_stuck_in_wall_check = ");
    write_float(strout, &data->time_since_previous_stuck_in_wall_check);
    autostr_append(strout, ",\n");
    autostr_append(strout, "virt_pos = ");
    write_gps(strout, &data->virt_pos);
    autostr_append(strout, ",\n");
    autostr_append(strout, "global_list = ");
    write_list_head_t(strout, &data->global_list);
    autostr_append(strout, ",\n");
    autostr_append(strout, "level_list = ");
    write_list_head_t(strout, &data->level_list);
    autostr_append(strout, ",\n");
    autostr_append(strout, "}");
}

void read_enemy(lua_State* L, int index, enemy *data)
{
    if (lua_getfield_or_warn(L, index, "id")) {
        read_int16_t(L, -1, &data->id);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "type")) {
        read_int16_t(L, -1, &data->type);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "SpecialForce")) {
        read_uint8_t(L, -1, &data->SpecialForce);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "marker")) {
        read_int32_t(L, -1, &data->marker);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "max_distance_to_home")) {
        read_int16_t(L, -1, &data->max_distance_to_home);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "dialog_section_name")) {
        read_string(L, -1, &data->dialog_section_name);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "short_description_text")) {
        read_string(L, -1, &data->short_description_text);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "on_death_drop_item_code")) {
        read_int16_t(L, -1, &data->on_death_drop_item_code);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "faction")) {
        read_int32_t(L, -1, &data->faction);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "will_rush_tux")) {
        read_uint8_t(L, -1, &data->will_rush_tux);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "combat_state")) {
        read_int32_t(L, -1, &data->combat_state);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "state_timeout")) {
        read_float(L, -1, &data->state_timeout);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "CompletelyFixed")) {
        read_int16_t(L, -1, &data->CompletelyFixed);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "has_been_taken_over")) {
        read_uint8_t(L, -1, &data->has_been_taken_over);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "follow_tux")) {
        read_uint8_t(L, -1, &data->follow_tux);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "has_greeted_influencer")) {
        read_uint8_t(L, -1, &data->has_greeted_influencer);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "nextwaypoint")) {
        read_int16_t(L, -1, &data->nextwaypoint);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "lastwaypoint")) {
        read_int16_t(L, -1, &data->lastwaypoint);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "homewaypoint")) {
        read_int16_t(L, -1, &data->homewaypoint);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "pos")) {
        read_gps(L, -1, &data->pos);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "speed")) {
        read_finepoint(L, -1, &data->speed);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "energy")) {
        read_float(L, -1, &data->energy);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "animation_phase")) {
        read_float(L, -1, &data->animation_phase);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "animation_type")) {
        read_int16_t(L, -1, &data->animation_type);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "frozen")) {
        read_float(L, -1, &data->frozen);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "poison_duration_left")) {
        read_float(L, -1, &data->poison_duration_left);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "poison_damage_per_sec")) {
        read_float(L, -1, &data->poison_damage_per_sec);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "paralysation_duration_left")) {
        read_float(L, -1, &data->paralysation_duration_left);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "pure_wait")) {
        read_float(L, -1, &data->pure_wait);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "firewait")) {
        read_float(L, -1, &data->firewait);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "ammo_left")) {
        read_int16_t(L, -1, &data->ammo_left);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "attack_target_type")) {
        read_uint8_t(L, -1, &data->attack_target_type);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bot_target_n")) {
        read_int16_t(L, -1, &data->bot_target_n);
        lua_pop(L, 1);
    }
    data->bot_target_addr = NULL;
    if (lua_getfield_or_warn(L, index, "previous_angle")) {
        read_float(L, -1, &data->previous_angle);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "current_angle")) {
        read_float(L, -1, &data->current_angle);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "previous_phase")) {
        read_float(L, -1, &data->previous_phase);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "last_phase_change")) {
        read_float(L, -1, &data->last_phase_change);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "last_combat_step")) {
        read_float(L, -1, &data->last_combat_step);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "TextVisibleTime")) {
        read_float(L, -1, &data->TextVisibleTime);
        lua_pop(L, 1);
    }
    data->TextToBeDisplayed = NULL;
    if (lua_getfield_or_warn(L, index, "PrivatePathway")) {
        read_moderately_finepoint_array(L, -1, data->PrivatePathway, 5);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bot_stuck_in_wall_at_previous_check")) {
        read_uint8_t(L, -1, &data->bot_stuck_in_wall_at_previous_check);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "time_since_previous_stuck_in_wall_check")) {
        read_float(L, -1, &data->time_since_previous_stuck_in_wall_check);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "virt_pos")) {
        read_gps(L, -1, &data->virt_pos);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "global_list")) {
        read_list_head_t(L, -1, &data->global_list);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "level_list")) {
        read_list_head_t(L, -1, &data->level_list);
        lua_pop(L, 1);
    }
}

void write_bullet(struct auto_string *strout, bullet *data)
{
    autostr_append(strout, "{\n");
    autostr_append(strout, "type = ");
    write_int16_t(strout, &data->type);
    autostr_append(strout, ",\n");
    autostr_append(strout, "phase = ");
    write_int32_t(strout, &data->phase);
    autostr_append(strout, ",\n");
    autostr_append(strout, "mine = ");
    write_uint8_t(strout, &data->mine);
    autostr_append(strout, ",\n");
    autostr_append(strout, "pos = ");
    write_gps(strout, &data->pos);
    autostr_append(strout, ",\n");
    autostr_append(strout, "speed = ");
    write_moderately_finepoint(strout, &data->speed);
    autostr_append(strout, ",\n");
    autostr_append(strout, "damage = ");
    write_int16_t(strout, &data->damage);
    autostr_append(strout, ",\n");
    autostr_append(strout, "time_in_seconds = ");
    write_float(strout, &data->time_in_seconds);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bullet_lifetime = ");
    write_float(strout, &data->bullet_lifetime);
    autostr_append(strout, ",\n");
    autostr_append(strout, "time_to_hide_still = ");
    write_float(strout, &data->time_to_hide_still);
    autostr_append(strout, ",\n");
    autostr_append(strout, "owner = ");
    write_int16_t(strout, &data->owner);
    autostr_append(strout, ",\n");
    autostr_append(strout, "angle = ");
    write_float(strout, &data->angle);
    autostr_append(strout, ",\n");
    autostr_append(strout, "pass_through_hit_bodies = ");
    write_uint8_t(strout, &data->pass_through_hit_bodies);
    autostr_append(strout, ",\n");
    autostr_append(strout, "freezing_level = ");
    write_int16_t(strout, &data->freezing_level);
    autostr_append(strout, ",\n");
    autostr_append(strout, "poison_duration = ");
    write_float(strout, &data->poison_duration);
    autostr_append(strout, ",\n");
    autostr_append(strout, "poison_damage_per_sec = ");
    write_float(strout, &data->poison_damage_per_sec);
    autostr_append(strout, ",\n");
    autostr_append(strout, "paralysation_duration = ");
    write_float(strout, &data->paralysation_duration);
    autostr_append(strout, ",\n");
    autostr_append(strout, "faction = ");
    write_int32_t(strout, &data->faction);
    autostr_append(strout, ",\n");
    autostr_append(strout, "hit_type = ");
    write_uint8_t(strout, &data->hit_type);
    autostr_append(strout, ",\n");
    autostr_append(strout, "}");
}

void read_bullet(lua_State* L, int index, bullet *data)
{
    if (lua_getfield_or_warn(L, index, "type")) {
        read_int16_t(L, -1, &data->type);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "phase")) {
        read_int32_t(L, -1, &data->phase);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "mine")) {
        read_uint8_t(L, -1, &data->mine);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "pos")) {
        read_gps(L, -1, &data->pos);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "speed")) {
        read_moderately_finepoint(L, -1, &data->speed);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "damage")) {
        read_int16_t(L, -1, &data->damage);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "time_in_seconds")) {
        read_float(L, -1, &data->time_in_seconds);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bullet_lifetime")) {
        read_float(L, -1, &data->bullet_lifetime);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "time_to_hide_still")) {
        read_float(L, -1, &data->time_to_hide_still);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "owner")) {
        read_int16_t(L, -1, &data->owner);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "angle")) {
        read_float(L, -1, &data->angle);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "pass_through_hit_bodies")) {
        read_uint8_t(L, -1, &data->pass_through_hit_bodies);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "freezing_level")) {
        read_int16_t(L, -1, &data->freezing_level);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "poison_duration")) {
        read_float(L, -1, &data->poison_duration);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "poison_damage_per_sec")) {
        read_float(L, -1, &data->poison_damage_per_sec);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "paralysation_duration")) {
        read_float(L, -1, &data->paralysation_duration);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "faction")) {
        read_int32_t(L, -1, &data->faction);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "hit_type")) {
        read_uint8_t(L, -1, &data->hit_type);
        lua_pop(L, 1);
    }
}

void write_upgrade_socket(struct auto_string *strout, upgrade_socket *data)
{
    autostr_append(strout, "{\n");
    autostr_append(strout, "type = ");
    write_int32_t(strout, &data->type);
    autostr_append(strout, ",\n");
    autostr_append(strout, "addon = ");
    write_string(strout, &data->addon);
    autostr_append(strout, ",\n");
    autostr_append(strout, "}");
}

void read_upgrade_socket(lua_State* L, int index, upgrade_socket *data)
{
    if (lua_getfield_or_warn(L, index, "type")) {
        read_int32_t(L, -1, &data->type);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "addon")) {
        read_string(L, -1, &data->addon);
        lua_pop(L, 1);
    }
}

void write_point(struct auto_string *strout, point *data)
{
    autostr_append(strout, "{\n");
    autostr_append(strout, "x = ");
    write_int32_t(strout, &data->x);
    autostr_append(strout, ",\n");
    autostr_append(strout, "y = ");
    write_int32_t(strout, &data->y);
    autostr_append(strout, ",\n");
    autostr_append(strout, "}");
}

void read_point(lua_State* L, int index, point *data)
{
    if (lua_getfield_or_warn(L, index, "x")) {
        read_int32_t(L, -1, &data->x);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "y")) {
        read_int32_t(L, -1, &data->y);
        lua_pop(L, 1);
    }
}

void write_moderately_finepoint(struct auto_string *strout, moderately_finepoint *data)
{
    autostr_append(strout, "{\n");
    autostr_append(strout, "x = ");
    write_float(strout, &data->x);
    autostr_append(strout, ",\n");
    autostr_append(strout, "y = ");
    write_float(strout, &data->y);
    autostr_append(strout, ",\n");
    autostr_append(strout, "}");
}

void read_moderately_finepoint(lua_State* L, int index, moderately_finepoint *data)
{
    if (lua_getfield_or_warn(L, index, "x")) {
        read_float(L, -1, &data->x);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "y")) {
        read_float(L, -1, &data->y);
        lua_pop(L, 1);
    }
}

void write_spell_active(struct auto_string *strout, spell_active *data)
{
    autostr_append(strout, "{\n");
    autostr_append(strout, "img_type = ");
    write_int32_t(strout, &data->img_type);
    autostr_append(strout, ",\n");
    autostr_append(strout, "damage = ");
    write_int32_t(strout, &data->damage);
    autostr_append(strout, ",\n");
    autostr_append(strout, "poison_duration = ");
    write_int32_t(strout, &data->poison_duration);
    autostr_append(strout, ",\n");
    autostr_append(strout, "poison_dmg = ");
    write_int32_t(strout, &data->poison_dmg);
    autostr_append(strout, ",\n");
    autostr_append(strout, "freeze_duration = ");
    write_int32_t(strout, &data->freeze_duration);
    autostr_append(strout, ",\n");
    autostr_append(strout, "paralyze_duration = ");
    write_int32_t(strout, &data->paralyze_duration);
    autostr_append(strout, ",\n");
    autostr_append(strout, "spell_center = ");
    write_moderately_finepoint(strout, &data->spell_center);
    autostr_append(strout, ",\n");
    autostr_append(strout, "spell_radius = ");
    write_float(strout, &data->spell_radius);
    autostr_append(strout, ",\n");
    autostr_append(strout, "spell_age = ");
    write_float(strout, &data->spell_age);
    autostr_append(strout, ",\n");
    autostr_append(strout, "active_directions = ");
    write_uint8_t_array(strout, data->active_directions, RADIAL_SPELL_DIRECTIONS);
    autostr_append(strout, ",\n");
    autostr_append(strout, "mine = ");
    write_int32_t(strout, &data->mine);
    autostr_append(strout, ",\n");
    autostr_append(strout, "hit_type = ");
    write_uint8_t(strout, &data->hit_type);
    autostr_append(strout, ",\n");
    autostr_append(strout, "}");
}

void read_spell_active(lua_State* L, int index, spell_active *data)
{
    if (lua_getfield_or_warn(L, index, "img_type")) {
        read_int32_t(L, -1, &data->img_type);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "damage")) {
        read_int32_t(L, -1, &data->damage);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "poison_duration")) {
        read_int32_t(L, -1, &data->poison_duration);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "poison_dmg")) {
        read_int32_t(L, -1, &data->poison_dmg);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "freeze_duration")) {
        read_int32_t(L, -1, &data->freeze_duration);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "paralyze_duration")) {
        read_int32_t(L, -1, &data->paralyze_duration);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "spell_center")) {
        read_moderately_finepoint(L, -1, &data->spell_center);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "spell_radius")) {
        read_float(L, -1, &data->spell_radius);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "spell_age")) {
        read_float(L, -1, &data->spell_age);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "active_directions")) {
        read_uint8_t_array(L, -1, data->active_directions, RADIAL_SPELL_DIRECTIONS);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "mine")) {
        read_int32_t(L, -1, &data->mine);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "hit_type")) {
        read_uint8_t(L, -1, &data->hit_type);
        lua_pop(L, 1);
    }
}

void write_mission(struct auto_string *strout, mission *data)
{
    autostr_append(strout, "{\n");
    autostr_append(strout, "mission_name = ");
    write_string(strout, &data->mission_name);
    autostr_append(strout, ",\n");
    autostr_append(strout, "MissionWasAssigned = ");
    write_int32_t(strout, &data->MissionWasAssigned);
    autostr_append(strout, ",\n");
    autostr_append(strout, "MissionIsComplete = ");
    write_int32_t(strout, &data->MissionIsComplete);
    autostr_append(strout, ",\n");
    autostr_append(strout, "MissionWasFailed = ");
    write_int32_t(strout, &data->MissionWasFailed);
    autostr_append(strout, ",\n");
    autostr_append(strout, "MissionExistsAtAll = ");
    write_int32_t(strout, &data->MissionExistsAtAll);
    autostr_append(strout, ",\n");
    autostr_append(strout, "fetch_item = ");
    write_int32_t(strout, &data->fetch_item);
    autostr_append(strout, ",\n");
    autostr_append(strout, "KillOne = ");
    write_int32_t(strout, &data->KillOne);
    autostr_append(strout, ",\n");
    autostr_append(strout, "must_clear_first_level = ");
    write_int32_t(strout, &data->must_clear_first_level);
    autostr_append(strout, ",\n");
    autostr_append(strout, "must_clear_second_level = ");
    write_int32_t(strout, &data->must_clear_second_level);
    autostr_append(strout, ",\n");
    autostr_append(strout, "MustReachLevel = ");
    write_int32_t(strout, &data->MustReachLevel);
    autostr_append(strout, ",\n");
    autostr_append(strout, "MustReachPoint = ");
    write_point(strout, &data->MustReachPoint);
    autostr_append(strout, ",\n");
    autostr_append(strout, "MustLiveTime = ");
    write_double(strout, &data->MustLiveTime);
    autostr_append(strout, ",\n");
    autostr_append(strout, "MustBeOne = ");
    write_int32_t(strout, &data->MustBeOne);
    autostr_append(strout, ",\n");
    autostr_append(strout, "completion_lua_code = ");
    write_luacode(strout, &data->completion_lua_code);
    autostr_append(strout, ",\n");
    autostr_append(strout, "assignment_lua_code = ");
    write_luacode(strout, &data->assignment_lua_code);
    autostr_append(strout, ",\n");
    autostr_append(strout, "mission_diary_texts = ");
    write_string_array(strout, data->mission_diary_texts, MAX_MISSION_DESCRIPTION_TEXTS);
    autostr_append(strout, ",\n");
    autostr_append(strout, "mission_description_time = ");
    write_float_array(strout, data->mission_description_time, MAX_MISSION_DESCRIPTION_TEXTS);
    autostr_append(strout, ",\n");
    autostr_append(strout, "expanded_display_for_this_mission = ");
    write_int32_t(strout, &data->expanded_display_for_this_mission);
    autostr_append(strout, ",\n");
    autostr_append(strout, "}");
}

void read_mission(lua_State* L, int index, mission *data)
{
    if (lua_getfield_or_warn(L, index, "mission_name")) {
        read_string(L, -1, &data->mission_name);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "MissionWasAssigned")) {
        read_int32_t(L, -1, &data->MissionWasAssigned);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "MissionIsComplete")) {
        read_int32_t(L, -1, &data->MissionIsComplete);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "MissionWasFailed")) {
        read_int32_t(L, -1, &data->MissionWasFailed);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "MissionExistsAtAll")) {
        read_int32_t(L, -1, &data->MissionExistsAtAll);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "fetch_item")) {
        read_int32_t(L, -1, &data->fetch_item);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "KillOne")) {
        read_int32_t(L, -1, &data->KillOne);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "must_clear_first_level")) {
        read_int32_t(L, -1, &data->must_clear_first_level);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "must_clear_second_level")) {
        read_int32_t(L, -1, &data->must_clear_second_level);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "MustReachLevel")) {
        read_int32_t(L, -1, &data->MustReachLevel);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "MustReachPoint")) {
        read_point(L, -1, &data->MustReachPoint);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "MustLiveTime")) {
        read_double(L, -1, &data->MustLiveTime);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "MustBeOne")) {
        read_int32_t(L, -1, &data->MustBeOne);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "completion_lua_code")) {
        read_luacode(L, -1, &data->completion_lua_code);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "assignment_lua_code")) {
        read_luacode(L, -1, &data->assignment_lua_code);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "mission_diary_texts")) {
        read_string_array(L, -1, data->mission_diary_texts, MAX_MISSION_DESCRIPTION_TEXTS);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "mission_description_time")) {
        read_float_array(L, -1, data->mission_description_time, MAX_MISSION_DESCRIPTION_TEXTS);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "expanded_display_for_this_mission")) {
        read_int32_t(L, -1, &data->expanded_display_for_this_mission);
        lua_pop(L, 1);
    }
}

void write_npc(struct auto_string *strout, npc *data)
{
    autostr_append(strout, "{\n");
    autostr_append(strout, "dialog_basename = ");
    write_string(strout, &data->dialog_basename);
    autostr_append(strout, ",\n");
    autostr_append(strout, "chat_character_initialized = ");
    write_uint8_t(strout, &data->chat_character_initialized);
    autostr_append(strout, ",\n");
    autostr_append(strout, "chat_flags = ");
    write_uint8_t_array(strout, data->chat_flags, MAX_ANSWERS_PER_PERSON);
    autostr_append(strout, ",\n");
    autostr_append(strout, "shoplist = ");
    write_string_array(strout, data->shoplist, MAX_ITEMS_IN_INVENTORY);
    autostr_append(strout, ",\n");
    autostr_append(strout, "shoplistweight = ");
    write_int32_t_array(strout, data->shoplistweight, MAX_ITEMS_IN_INVENTORY);
    autostr_append(strout, ",\n");
    autostr_append(strout, "npc_inventory = ");
    write_item_dynarray(strout, &data->npc_inventory);
    autostr_append(strout, ",\n");
    autostr_append(strout, "last_trading_date = ");
    write_float(strout, &data->last_trading_date);
    autostr_append(strout, ",\n");
    autostr_append(strout, "node = ");
    write_list_head_t(strout, &data->node);
    autostr_append(strout, ",\n");
    autostr_append(strout, "}");
}

void read_npc(lua_State* L, int index, npc *data)
{
    if (lua_getfield_or_warn(L, index, "dialog_basename")) {
        read_string(L, -1, &data->dialog_basename);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "chat_character_initialized")) {
        read_uint8_t(L, -1, &data->chat_character_initialized);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "chat_flags")) {
        read_uint8_t_array(L, -1, data->chat_flags, MAX_ANSWERS_PER_PERSON);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "shoplist")) {
        read_string_array(L, -1, data->shoplist, MAX_ITEMS_IN_INVENTORY);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "shoplistweight")) {
        read_int32_t_array(L, -1, data->shoplistweight, MAX_ITEMS_IN_INVENTORY);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "npc_inventory")) {
        read_item_dynarray(L, -1, &data->npc_inventory);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "last_trading_date")) {
        read_float(L, -1, &data->last_trading_date);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "node")) {
        read_list_head_t(L, -1, &data->node);
        lua_pop(L, 1);
    }
}

void write_tux_t(struct auto_string *strout, tux_t *data)
{
    autostr_append(strout, "{\n");
    autostr_append(strout, "current_game_date = ");
    write_float(strout, &data->current_game_date);
    autostr_append(strout, ",\n");
    autostr_append(strout, "current_power_bonus = ");
    write_int32_t(strout, &data->current_power_bonus);
    autostr_append(strout, ",\n");
    autostr_append(strout, "power_bonus_end_date = ");
    write_float(strout, &data->power_bonus_end_date);
    autostr_append(strout, ",\n");
    autostr_append(strout, "current_dexterity_bonus = ");
    write_int32_t(strout, &data->current_dexterity_bonus);
    autostr_append(strout, ",\n");
    autostr_append(strout, "dexterity_bonus_end_date = ");
    write_float(strout, &data->dexterity_bonus_end_date);
    autostr_append(strout, ",\n");
    autostr_append(strout, "light_bonus_end_date = ");
    write_float(strout, &data->light_bonus_end_date);
    autostr_append(strout, ",\n");
    autostr_append(strout, "speed = ");
    write_finepoint(strout, &data->speed);
    autostr_append(strout, ",\n");
    autostr_append(strout, "pos = ");
    write_gps(strout, &data->pos);
    autostr_append(strout, ",\n");
    autostr_append(strout, "teleport_anchor = ");
    write_gps(strout, &data->teleport_anchor);
    autostr_append(strout, ",\n");
    autostr_append(strout, "mouse_move_target = ");
    write_gps(strout, &data->mouse_move_target);
    autostr_append(strout, ",\n");
    autostr_append(strout, "current_enemy_target_n = ");
    write_int16_t(strout, &data->current_enemy_target_n);
    autostr_append(strout, ",\n");
    autostr_append(strout, "god_mode = ");
    write_uint8_t(strout, &data->god_mode);
    autostr_append(strout, ",\n");
    autostr_append(strout, "mouse_move_target_combo_action_type = ");
    write_int32_t(strout, &data->mouse_move_target_combo_action_type);
    autostr_append(strout, ",\n");
    autostr_append(strout, "mouse_move_target_combo_action_parameter = ");
    write_int32_t(strout, &data->mouse_move_target_combo_action_parameter);
    autostr_append(strout, ",\n");
    autostr_append(strout, "light_bonus_from_tux = ");
    write_int32_t(strout, &data->light_bonus_from_tux);
    autostr_append(strout, ",\n");
    autostr_append(strout, "map_maker_is_present = ");
    write_int32_t(strout, &data->map_maker_is_present);
    autostr_append(strout, ",\n");
    autostr_append(strout, "maxenergy = ");
    write_float(strout, &data->maxenergy);
    autostr_append(strout, ",\n");
    autostr_append(strout, "energy = ");
    write_float(strout, &data->energy);
    autostr_append(strout, ",\n");
    autostr_append(strout, "max_temperature = ");
    write_float(strout, &data->max_temperature);
    autostr_append(strout, ",\n");
    autostr_append(strout, "temperature = ");
    write_float(strout, &data->temperature);
    autostr_append(strout, ",\n");
    autostr_append(strout, "old_temperature = ");
    write_float(strout, &data->old_temperature);
    autostr_append(strout, ",\n");
    autostr_append(strout, "max_running_power = ");
    write_float(strout, &data->max_running_power);
    autostr_append(strout, ",\n");
    autostr_append(strout, "running_power = ");
    write_float(strout, &data->running_power);
    autostr_append(strout, ",\n");
    autostr_append(strout, "running_must_rest = ");
    write_int32_t(strout, &data->running_must_rest);
    autostr_append(strout, ",\n");
    autostr_append(strout, "running_power_bonus = ");
    write_int32_t(strout, &data->running_power_bonus);
    autostr_append(strout, ",\n");
    autostr_append(strout, "health_recovery_rate = ");
    write_float(strout, &data->health_recovery_rate);
    autostr_append(strout, ",\n");
    autostr_append(strout, "cooling_rate = ");
    write_float(strout, &data->cooling_rate);
    autostr_append(strout, ",\n");
    autostr_append(strout, "busy_time = ");
    write_double(strout, &data->busy_time);
    autostr_append(strout, ",\n");
    autostr_append(strout, "busy_type = ");
    write_int32_t(strout, &data->busy_type);
    autostr_append(strout, ",\n");
    autostr_append(strout, "phase = ");
    write_double(strout, &data->phase);
    autostr_append(strout, ",\n");
    autostr_append(strout, "angle = ");
    write_float(strout, &data->angle);
    autostr_append(strout, ",\n");
    autostr_append(strout, "walk_cycle_phase = ");
    write_float(strout, &data->walk_cycle_phase);
    autostr_append(strout, ",\n");
    autostr_append(strout, "weapon_swing_time = ");
    write_float(strout, &data->weapon_swing_time);
    autostr_append(strout, ",\n");
    autostr_append(strout, "MissionTimeElapsed = ");
    write_float(strout, &data->MissionTimeElapsed);
    autostr_append(strout, ",\n");
    autostr_append(strout, "got_hit_time = ");
    write_float(strout, &data->got_hit_time);
    autostr_append(strout, ",\n");
    autostr_append(strout, "savegame_version_string = ");
    write_string(strout, &data->savegame_version_string);
    autostr_append(strout, ",\n");
    autostr_append(strout, "base_cooling = ");
    write_int32_t(strout, &data->base_cooling);
    autostr_append(strout, ",\n");
    autostr_append(strout, "base_dexterity = ");
    write_int32_t(strout, &data->base_dexterity);
    autostr_append(strout, ",\n");
    autostr_append(strout, "base_physique = ");
    write_int32_t(strout, &data->base_physique);
    autostr_append(strout, ",\n");
    autostr_append(strout, "base_strength = ");
    write_int32_t(strout, &data->base_strength);
    autostr_append(strout, ",\n");
    autostr_append(strout, "cooling = ");
    write_int32_t(strout, &data->cooling);
    autostr_append(strout, ",\n");
    autostr_append(strout, "dexterity = ");
    write_int32_t(strout, &data->dexterity);
    autostr_append(strout, ",\n");
    autostr_append(strout, "physique = ");
    write_int32_t(strout, &data->physique);
    autostr_append(strout, ",\n");
    autostr_append(strout, "strength = ");
    write_int32_t(strout, &data->strength);
    autostr_append(strout, ",\n");
    autostr_append(strout, "points_to_distribute = ");
    write_int32_t(strout, &data->points_to_distribute);
    autostr_append(strout, ",\n");
    autostr_append(strout, "base_damage = ");
    write_float(strout, &data->base_damage);
    autostr_append(strout, ",\n");
    autostr_append(strout, "damage_modifier = ");
    write_float(strout, &data->damage_modifier);
    autostr_append(strout, ",\n");
    autostr_append(strout, "armor_class = ");
    write_float(strout, &data->armor_class);
    autostr_append(strout, ",\n");
    autostr_append(strout, "to_hit = ");
    write_float(strout, &data->to_hit);
    autostr_append(strout, ",\n");
    autostr_append(strout, "slowing_melee_targets = ");
    write_int32_t(strout, &data->slowing_melee_targets);
    autostr_append(strout, ",\n");
    autostr_append(strout, "paralyzing_melee_targets = ");
    write_int32_t(strout, &data->paralyzing_melee_targets);
    autostr_append(strout, ",\n");
    autostr_append(strout, "experience_factor = ");
    write_float(strout, &data->experience_factor);
    autostr_append(strout, ",\n");
    autostr_append(strout, "Experience = ");
    write_uint32_t(strout, &data->Experience);
    autostr_append(strout, ",\n");
    autostr_append(strout, "exp_level = ");
    write_int32_t(strout, &data->exp_level);
    autostr_append(strout, ",\n");
    autostr_append(strout, "Gold = ");
    write_uint32_t(strout, &data->Gold);
    autostr_append(strout, ",\n");
    autostr_append(strout, "character_name = ");
    write_string(strout, &data->character_name);
    autostr_append(strout, ",\n");
    autostr_append(strout, "AllMissions = ");
    write_mission_array(strout, data->AllMissions, MAX_MISSIONS_IN_GAME);
    autostr_append(strout, ",\n");
    autostr_append(strout, "marker = ");
    write_int32_t(strout, &data->marker);
    autostr_append(strout, ",\n");
    autostr_append(strout, "LastCrysoundTime = ");
    write_float(strout, &data->LastCrysoundTime);
    autostr_append(strout, ",\n");
    autostr_append(strout, "TextVisibleTime = ");
    write_float(strout, &data->TextVisibleTime);
    autostr_append(strout, ",\n");
    autostr_append(strout, "readied_skill = ");
    write_int32_t(strout, &data->readied_skill);
    autostr_append(strout, ",\n");
    autostr_append(strout, "skill_level = ");
    write_int32_t_array(strout, data->skill_level, MAX_NUMBER_OF_PROGRAMS);
    autostr_append(strout, ",\n");
    autostr_append(strout, "melee_weapon_skill = ");
    write_int32_t(strout, &data->melee_weapon_skill);
    autostr_append(strout, ",\n");
    autostr_append(strout, "ranged_weapon_skill = ");
    write_int32_t(strout, &data->ranged_weapon_skill);
    autostr_append(strout, ",\n");
    autostr_append(strout, "spellcasting_skill = ");
    write_int32_t(strout, &data->spellcasting_skill);
    autostr_append(strout, ",\n");
    autostr_append(strout, "Inventory = ");
    write_item_array(strout, data->Inventory, MAX_ITEMS_IN_INVENTORY);
    autostr_append(strout, ",\n");
    autostr_append(strout, "weapon_item = ");
    write_item(strout, &data->weapon_item);
    autostr_append(strout, ",\n");
    autostr_append(strout, "drive_item = ");
    write_item(strout, &data->drive_item);
    autostr_append(strout, ",\n");
    autostr_append(strout, "armour_item = ");
    write_item(strout, &data->armour_item);
    autostr_append(strout, ",\n");
    autostr_append(strout, "shield_item = ");
    write_item(strout, &data->shield_item);
    autostr_append(strout, ",\n");
    autostr_append(strout, "special_item = ");
    write_item(strout, &data->special_item);
    autostr_append(strout, ",\n");
    autostr_append(strout, "HaveBeenToLevel = ");
    write_uint8_t_array(strout, data->HaveBeenToLevel, MAX_LEVELS);
    autostr_append(strout, ",\n");
    autostr_append(strout, "time_since_last_visit_or_respawn = ");
    write_float_array(strout, data->time_since_last_visit_or_respawn, MAX_LEVELS);
    autostr_append(strout, ",\n");
    autostr_append(strout, "next_intermediate_point = ");
    write_moderately_finepoint_array(strout, data->next_intermediate_point, MAX_INTERMEDIATE_WAYPOINTS_FOR_TUX);
    autostr_append(strout, ",\n");
    autostr_append(strout, "Automap = ");
    write_automap_data_t_array(strout, data->Automap, MAX_LEVELS);
    autostr_append(strout, ",\n");
    autostr_append(strout, "current_zero_ring_index = ");
    write_int32_t(strout, &data->current_zero_ring_index);
    autostr_append(strout, ",\n");
    autostr_append(strout, "Position_History_Ring_Buffer = ");
    write_gps_array(strout, data->Position_History_Ring_Buffer, MAX_INFLU_POSITION_HISTORY);
    autostr_append(strout, ",\n");
    autostr_append(strout, "BigScreenMessageIndex = ");
    write_int32_t(strout, &data->BigScreenMessageIndex);
    autostr_append(strout, ",\n");
    autostr_append(strout, "BigScreenMessage = ");
    write_string_array(strout, data->BigScreenMessage, MAX_BIG_SCREEN_MESSAGES);
    autostr_append(strout, ",\n");
    autostr_append(strout, "BigScreenMessageDuration = ");
    write_float_array(strout, data->BigScreenMessageDuration, MAX_BIG_SCREEN_MESSAGES);
    autostr_append(strout, ",\n");
    autostr_append(strout, "slowdown_duration = ");
    write_float(strout, &data->slowdown_duration);
    autostr_append(strout, ",\n");
    autostr_append(strout, "paralyze_duration = ");
    write_float(strout, &data->paralyze_duration);
    autostr_append(strout, ",\n");
    autostr_append(strout, "invisible_duration = ");
    write_float(strout, &data->invisible_duration);
    autostr_append(strout, ",\n");
    autostr_append(strout, "nmap_duration = ");
    write_float(strout, &data->nmap_duration);
    autostr_append(strout, ",\n");
    autostr_append(strout, "quest_browser_changed = ");
    write_int32_t(strout, &data->quest_browser_changed);
    autostr_append(strout, ",\n");
    autostr_append(strout, "program_shortcuts = ");
    write_int32_t_array(strout, data->program_shortcuts, 10);
    autostr_append(strout, ",\n");
    autostr_append(strout, "TakeoverSuccesses = ");
    write_int32_t_array(strout, data->TakeoverSuccesses, NB_DROID_TYPES);
    autostr_append(strout, ",\n");
    autostr_append(strout, "TakeoverFailures = ");
    write_int32_t_array(strout, data->TakeoverFailures, NB_DROID_TYPES);
    autostr_append(strout, ",\n");
    autostr_append(strout, "destroyed_bots = ");
    write_int32_t_array(strout, data->destroyed_bots, NB_DROID_TYPES);
    autostr_append(strout, ",\n");
    autostr_append(strout, "damage_dealt = ");
    write_int32_t_array(strout, data->damage_dealt, NB_DROID_TYPES);
    autostr_append(strout, ",\n");
    autostr_append(strout, "meters_traveled = ");
    write_float(strout, &data->meters_traveled);
    autostr_append(strout, ",\n");
    autostr_append(strout, "}");
}

void read_tux_t(lua_State* L, int index, tux_t *data)
{
    if (lua_getfield_or_warn(L, index, "current_game_date")) {
        read_float(L, -1, &data->current_game_date);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "current_power_bonus")) {
        read_int32_t(L, -1, &data->current_power_bonus);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "power_bonus_end_date")) {
        read_float(L, -1, &data->power_bonus_end_date);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "current_dexterity_bonus")) {
        read_int32_t(L, -1, &data->current_dexterity_bonus);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "dexterity_bonus_end_date")) {
        read_float(L, -1, &data->dexterity_bonus_end_date);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "light_bonus_end_date")) {
        read_float(L, -1, &data->light_bonus_end_date);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "speed")) {
        read_finepoint(L, -1, &data->speed);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "pos")) {
        read_gps(L, -1, &data->pos);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "teleport_anchor")) {
        read_gps(L, -1, &data->teleport_anchor);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "mouse_move_target")) {
        read_gps(L, -1, &data->mouse_move_target);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "current_enemy_target_n")) {
        read_int16_t(L, -1, &data->current_enemy_target_n);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "god_mode")) {
        read_uint8_t(L, -1, &data->god_mode);
        lua_pop(L, 1);
    }
    data->current_enemy_target_addr = NULL;
    if (lua_getfield_or_warn(L, index, "mouse_move_target_combo_action_type")) {
        read_int32_t(L, -1, &data->mouse_move_target_combo_action_type);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "mouse_move_target_combo_action_parameter")) {
        read_int32_t(L, -1, &data->mouse_move_target_combo_action_parameter);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "light_bonus_from_tux")) {
        read_int32_t(L, -1, &data->light_bonus_from_tux);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "map_maker_is_present")) {
        read_int32_t(L, -1, &data->map_maker_is_present);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "maxenergy")) {
        read_float(L, -1, &data->maxenergy);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "energy")) {
        read_float(L, -1, &data->energy);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "max_temperature")) {
        read_float(L, -1, &data->max_temperature);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "temperature")) {
        read_float(L, -1, &data->temperature);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "old_temperature")) {
        read_float(L, -1, &data->old_temperature);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "max_running_power")) {
        read_float(L, -1, &data->max_running_power);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "running_power")) {
        read_float(L, -1, &data->running_power);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "running_must_rest")) {
        read_int32_t(L, -1, &data->running_must_rest);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "running_power_bonus")) {
        read_int32_t(L, -1, &data->running_power_bonus);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "health_recovery_rate")) {
        read_float(L, -1, &data->health_recovery_rate);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "cooling_rate")) {
        read_float(L, -1, &data->cooling_rate);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "busy_time")) {
        read_double(L, -1, &data->busy_time);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "busy_type")) {
        read_int32_t(L, -1, &data->busy_type);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "phase")) {
        read_double(L, -1, &data->phase);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "angle")) {
        read_float(L, -1, &data->angle);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "walk_cycle_phase")) {
        read_float(L, -1, &data->walk_cycle_phase);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "weapon_swing_time")) {
        read_float(L, -1, &data->weapon_swing_time);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "MissionTimeElapsed")) {
        read_float(L, -1, &data->MissionTimeElapsed);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "got_hit_time")) {
        read_float(L, -1, &data->got_hit_time);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "savegame_version_string")) {
        read_string(L, -1, &data->savegame_version_string);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "base_cooling")) {
        read_int32_t(L, -1, &data->base_cooling);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "base_dexterity")) {
        read_int32_t(L, -1, &data->base_dexterity);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "base_physique")) {
        read_int32_t(L, -1, &data->base_physique);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "base_strength")) {
        read_int32_t(L, -1, &data->base_strength);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "cooling")) {
        read_int32_t(L, -1, &data->cooling);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "dexterity")) {
        read_int32_t(L, -1, &data->dexterity);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "physique")) {
        read_int32_t(L, -1, &data->physique);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "strength")) {
        read_int32_t(L, -1, &data->strength);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "points_to_distribute")) {
        read_int32_t(L, -1, &data->points_to_distribute);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "base_damage")) {
        read_float(L, -1, &data->base_damage);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "damage_modifier")) {
        read_float(L, -1, &data->damage_modifier);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "armor_class")) {
        read_float(L, -1, &data->armor_class);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "to_hit")) {
        read_float(L, -1, &data->to_hit);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "slowing_melee_targets")) {
        read_int32_t(L, -1, &data->slowing_melee_targets);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "paralyzing_melee_targets")) {
        read_int32_t(L, -1, &data->paralyzing_melee_targets);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "experience_factor")) {
        read_float(L, -1, &data->experience_factor);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "Experience")) {
        read_uint32_t(L, -1, &data->Experience);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "exp_level")) {
        read_int32_t(L, -1, &data->exp_level);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "Gold")) {
        read_uint32_t(L, -1, &data->Gold);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "character_name")) {
        read_string(L, -1, &data->character_name);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "AllMissions")) {
        read_mission_array(L, -1, data->AllMissions, MAX_MISSIONS_IN_GAME);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "marker")) {
        read_int32_t(L, -1, &data->marker);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "LastCrysoundTime")) {
        read_float(L, -1, &data->LastCrysoundTime);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "TextVisibleTime")) {
        read_float(L, -1, &data->TextVisibleTime);
        lua_pop(L, 1);
    }
    data->TextToBeDisplayed = NULL;
    if (lua_getfield_or_warn(L, index, "readied_skill")) {
        read_int32_t(L, -1, &data->readied_skill);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "skill_level")) {
        read_int32_t_array(L, -1, data->skill_level, MAX_NUMBER_OF_PROGRAMS);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "melee_weapon_skill")) {
        read_int32_t(L, -1, &data->melee_weapon_skill);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "ranged_weapon_skill")) {
        read_int32_t(L, -1, &data->ranged_weapon_skill);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "spellcasting_skill")) {
        read_int32_t(L, -1, &data->spellcasting_skill);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "Inventory")) {
        read_item_array(L, -1, data->Inventory, MAX_ITEMS_IN_INVENTORY);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "weapon_item")) {
        read_item(L, -1, &data->weapon_item);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "drive_item")) {
        read_item(L, -1, &data->drive_item);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "armour_item")) {
        read_item(L, -1, &data->armour_item);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "shield_item")) {
        read_item(L, -1, &data->shield_item);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "special_item")) {
        read_item(L, -1, &data->special_item);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "HaveBeenToLevel")) {
        read_uint8_t_array(L, -1, data->HaveBeenToLevel, MAX_LEVELS);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "time_since_last_visit_or_respawn")) {
        read_float_array(L, -1, data->time_since_last_visit_or_respawn, MAX_LEVELS);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "next_intermediate_point")) {
        read_moderately_finepoint_array(L, -1, data->next_intermediate_point, MAX_INTERMEDIATE_WAYPOINTS_FOR_TUX);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "Automap")) {
        read_automap_data_t_array(L, -1, data->Automap, MAX_LEVELS);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "current_zero_ring_index")) {
        read_int32_t(L, -1, &data->current_zero_ring_index);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "Position_History_Ring_Buffer")) {
        read_gps_array(L, -1, data->Position_History_Ring_Buffer, MAX_INFLU_POSITION_HISTORY);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "BigScreenMessageIndex")) {
        read_int32_t(L, -1, &data->BigScreenMessageIndex);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "BigScreenMessage")) {
        read_string_array(L, -1, data->BigScreenMessage, MAX_BIG_SCREEN_MESSAGES);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "BigScreenMessageDuration")) {
        read_float_array(L, -1, data->BigScreenMessageDuration, MAX_BIG_SCREEN_MESSAGES);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "slowdown_duration")) {
        read_float(L, -1, &data->slowdown_duration);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "paralyze_duration")) {
        read_float(L, -1, &data->paralyze_duration);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "invisible_duration")) {
        read_float(L, -1, &data->invisible_duration);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "nmap_duration")) {
        read_float(L, -1, &data->nmap_duration);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "quest_browser_changed")) {
        read_int32_t(L, -1, &data->quest_browser_changed);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "program_shortcuts")) {
        read_int32_t_array(L, -1, data->program_shortcuts, 10);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "TakeoverSuccesses")) {
        read_int32_t_array(L, -1, data->TakeoverSuccesses, NB_DROID_TYPES);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "TakeoverFailures")) {
        read_int32_t_array(L, -1, data->TakeoverFailures, NB_DROID_TYPES);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "destroyed_bots")) {
        read_int32_t_array(L, -1, data->destroyed_bots, NB_DROID_TYPES);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "damage_dealt")) {
        read_int32_t_array(L, -1, data->damage_dealt, NB_DROID_TYPES);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "meters_traveled")) {
        read_float(L, -1, &data->meters_traveled);
        lua_pop(L, 1);
    }
}

void write_configuration_for_freedroid(struct auto_string *strout, configuration_for_freedroid *data)
{
    autostr_append(strout, "{\n");
    autostr_append(strout, "WantedTextVisibleTime = ");
    write_float(strout, &data->WantedTextVisibleTime);
    autostr_append(strout, ",\n");
    autostr_append(strout, "Draw_Framerate = ");
    write_int32_t(strout, &data->Draw_Framerate);
    autostr_append(strout, ",\n");
    autostr_append(strout, "Draw_Position = ");
    write_int32_t(strout, &data->Draw_Position);
    autostr_append(strout, ",\n");
    autostr_append(strout, "All_Texts_Switch = ");
    write_int32_t(strout, &data->All_Texts_Switch);
    autostr_append(strout, ",\n");
    autostr_append(strout, "Current_BG_Music_Volume = ");
    write_float(strout, &data->Current_BG_Music_Volume);
    autostr_append(strout, ",\n");
    autostr_append(strout, "Current_Sound_FX_Volume = ");
    write_float(strout, &data->Current_Sound_FX_Volume);
    autostr_append(strout, ",\n");
    autostr_append(strout, "current_gamma_correction = ");
    write_float(strout, &data->current_gamma_correction);
    autostr_append(strout, ",\n");
    autostr_append(strout, "Inventory_Visible = ");
    write_int32_t(strout, &data->Inventory_Visible);
    autostr_append(strout, ",\n");
    autostr_append(strout, "CharacterScreen_Visible = ");
    write_int32_t(strout, &data->CharacterScreen_Visible);
    autostr_append(strout, ",\n");
    autostr_append(strout, "SkillScreen_Visible = ");
    write_int32_t(strout, &data->SkillScreen_Visible);
    autostr_append(strout, ",\n");
    autostr_append(strout, "Automap_Visible = ");
    write_int32_t(strout, &data->Automap_Visible);
    autostr_append(strout, ",\n");
    autostr_append(strout, "spell_level_visible = ");
    write_int32_t(strout, &data->spell_level_visible);
    autostr_append(strout, ",\n");
    autostr_append(strout, "autorun_activated = ");
    write_int32_t(strout, &data->autorun_activated);
    autostr_append(strout, ",\n");
    autostr_append(strout, "freedroid_version_string = ");
    write_string(strout, &data->freedroid_version_string);
    autostr_append(strout, ",\n");
    autostr_append(strout, "skip_light_radius = ");
    write_int32_t(strout, &data->skip_light_radius);
    autostr_append(strout, ",\n");
    autostr_append(strout, "skill_explanation_screen_visible = ");
    write_int32_t(strout, &data->skill_explanation_screen_visible);
    autostr_append(strout, ",\n");
    autostr_append(strout, "enemy_energy_bars_visible = ");
    write_int32_t(strout, &data->enemy_energy_bars_visible);
    autostr_append(strout, ",\n");
    autostr_append(strout, "limit_framerate = ");
    write_int32_t(strout, &data->limit_framerate);
    autostr_append(strout, ",\n");
    autostr_append(strout, "omit_obstacles_in_level_editor = ");
    write_int32_t(strout, &data->omit_obstacles_in_level_editor);
    autostr_append(strout, ",\n");
    autostr_append(strout, "omit_enemies_in_level_editor = ");
    write_int32_t(strout, &data->omit_enemies_in_level_editor);
    autostr_append(strout, ",\n");
    autostr_append(strout, "zoom_is_on = ");
    write_int32_t(strout, &data->zoom_is_on);
    autostr_append(strout, ",\n");
    autostr_append(strout, "show_blood = ");
    write_int32_t(strout, &data->show_blood);
    autostr_append(strout, ",\n");
    autostr_append(strout, "show_lvledit_tooltips = ");
    write_int32_t(strout, &data->show_lvledit_tooltips);
    autostr_append(strout, ",\n");
    autostr_append(strout, "show_grid = ");
    write_int32_t(strout, &data->show_grid);
    autostr_append(strout, ",\n");
    autostr_append(strout, "show_wp_connections = ");
    write_int32_t(strout, &data->show_wp_connections);
    autostr_append(strout, ",\n");
    autostr_append(strout, "grid_mode = ");
    write_int32_t(strout, &data->grid_mode);
    autostr_append(strout, ",\n");
    autostr_append(strout, "number_of_big_screen_messages = ");
    write_int32_t(strout, &data->number_of_big_screen_messages);
    autostr_append(strout, ",\n");
    autostr_append(strout, "delay_for_big_screen_messages = ");
    write_float(strout, &data->delay_for_big_screen_messages);
    autostr_append(strout, ",\n");
    autostr_append(strout, "enable_cheatkeys = ");
    write_int32_t(strout, &data->enable_cheatkeys);
    autostr_append(strout, ",\n");
    autostr_append(strout, "transparency = ");
    write_int32_t(strout, &data->transparency);
    autostr_append(strout, ",\n");
    autostr_append(strout, "screen_width = ");
    write_int32_t(strout, &data->screen_width);
    autostr_append(strout, ",\n");
    autostr_append(strout, "screen_height = ");
    write_int32_t(strout, &data->screen_height);
    autostr_append(strout, ",\n");
    autostr_append(strout, "next_time_width_of_screen = ");
    write_int32_t(strout, &data->next_time_width_of_screen);
    autostr_append(strout, ",\n");
    autostr_append(strout, "next_time_height_of_screen = ");
    write_int32_t(strout, &data->next_time_height_of_screen);
    autostr_append(strout, ",\n");
    autostr_append(strout, "skip_shadow_blitting = ");
    write_int32_t(strout, &data->skip_shadow_blitting);
    autostr_append(strout, ",\n");
    autostr_append(strout, "do_fadings = ");
    write_int32_t(strout, &data->do_fadings);
    autostr_append(strout, ",\n");
    autostr_append(strout, "fullscreen_on = ");
    write_int32_t(strout, &data->fullscreen_on);
    autostr_append(strout, ",\n");
    autostr_append(strout, "talk_to_bots_after_takeover = ");
    write_int32_t(strout, &data->talk_to_bots_after_takeover);
    autostr_append(strout, ",\n");
    autostr_append(strout, "xray_vision_for_tux = ");
    write_int32_t(strout, &data->xray_vision_for_tux);
    autostr_append(strout, ",\n");
    autostr_append(strout, "cheat_running_stamina = ");
    write_int32_t(strout, &data->cheat_running_stamina);
    autostr_append(strout, ",\n");
    autostr_append(strout, "cheat_double_speed = ");
    write_int32_t(strout, &data->cheat_double_speed);
    autostr_append(strout, ",\n");
    autostr_append(strout, "lazyload = ");
    write_int32_t(strout, &data->lazyload);
    autostr_append(strout, ",\n");
    autostr_append(strout, "show_item_labels = ");
    write_int32_t(strout, &data->show_item_labels);
    autostr_append(strout, ",\n");
    autostr_append(strout, "last_edited_level = ");
    write_int32_t(strout, &data->last_edited_level);
    autostr_append(strout, ",\n");
    autostr_append(strout, "show_all_floor_layers = ");
    write_int32_t(strout, &data->show_all_floor_layers);
    autostr_append(strout, ",\n");
    autostr_append(strout, "difficulty_level = ");
    write_int32_t(strout, &data->difficulty_level);
    autostr_append(strout, ",\n");
    autostr_append(strout, "input_keybinds = ");
    write_keybind_t_array(strout, data->input_keybinds, 100);
    autostr_append(strout, ",\n");
    autostr_append(strout, "}");
}

void read_configuration_for_freedroid(lua_State* L, int index, configuration_for_freedroid *data)
{
    if (lua_getfield_or_warn(L, index, "WantedTextVisibleTime")) {
        read_float(L, -1, &data->WantedTextVisibleTime);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "Draw_Framerate")) {
        read_int32_t(L, -1, &data->Draw_Framerate);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "Draw_Position")) {
        read_int32_t(L, -1, &data->Draw_Position);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "All_Texts_Switch")) {
        read_int32_t(L, -1, &data->All_Texts_Switch);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "Current_BG_Music_Volume")) {
        read_float(L, -1, &data->Current_BG_Music_Volume);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "Current_Sound_FX_Volume")) {
        read_float(L, -1, &data->Current_Sound_FX_Volume);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "current_gamma_correction")) {
        read_float(L, -1, &data->current_gamma_correction);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "Inventory_Visible")) {
        read_int32_t(L, -1, &data->Inventory_Visible);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "CharacterScreen_Visible")) {
        read_int32_t(L, -1, &data->CharacterScreen_Visible);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "SkillScreen_Visible")) {
        read_int32_t(L, -1, &data->SkillScreen_Visible);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "Automap_Visible")) {
        read_int32_t(L, -1, &data->Automap_Visible);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "spell_level_visible")) {
        read_int32_t(L, -1, &data->spell_level_visible);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "autorun_activated")) {
        read_int32_t(L, -1, &data->autorun_activated);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "freedroid_version_string")) {
        read_string(L, -1, &data->freedroid_version_string);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "skip_light_radius")) {
        read_int32_t(L, -1, &data->skip_light_radius);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "skill_explanation_screen_visible")) {
        read_int32_t(L, -1, &data->skill_explanation_screen_visible);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "enemy_energy_bars_visible")) {
        read_int32_t(L, -1, &data->enemy_energy_bars_visible);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "limit_framerate")) {
        read_int32_t(L, -1, &data->limit_framerate);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "omit_obstacles_in_level_editor")) {
        read_int32_t(L, -1, &data->omit_obstacles_in_level_editor);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "omit_enemies_in_level_editor")) {
        read_int32_t(L, -1, &data->omit_enemies_in_level_editor);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "zoom_is_on")) {
        read_int32_t(L, -1, &data->zoom_is_on);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "show_blood")) {
        read_int32_t(L, -1, &data->show_blood);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "show_lvledit_tooltips")) {
        read_int32_t(L, -1, &data->show_lvledit_tooltips);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "show_grid")) {
        read_int32_t(L, -1, &data->show_grid);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "show_wp_connections")) {
        read_int32_t(L, -1, &data->show_wp_connections);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "grid_mode")) {
        read_int32_t(L, -1, &data->grid_mode);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "number_of_big_screen_messages")) {
        read_int32_t(L, -1, &data->number_of_big_screen_messages);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "delay_for_big_screen_messages")) {
        read_float(L, -1, &data->delay_for_big_screen_messages);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "enable_cheatkeys")) {
        read_int32_t(L, -1, &data->enable_cheatkeys);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "transparency")) {
        read_int32_t(L, -1, &data->transparency);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "screen_width")) {
        read_int32_t(L, -1, &data->screen_width);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "screen_height")) {
        read_int32_t(L, -1, &data->screen_height);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "next_time_width_of_screen")) {
        read_int32_t(L, -1, &data->next_time_width_of_screen);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "next_time_height_of_screen")) {
        read_int32_t(L, -1, &data->next_time_height_of_screen);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "skip_shadow_blitting")) {
        read_int32_t(L, -1, &data->skip_shadow_blitting);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "do_fadings")) {
        read_int32_t(L, -1, &data->do_fadings);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "fullscreen_on")) {
        read_int32_t(L, -1, &data->fullscreen_on);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "talk_to_bots_after_takeover")) {
        read_int32_t(L, -1, &data->talk_to_bots_after_takeover);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "xray_vision_for_tux")) {
        read_int32_t(L, -1, &data->xray_vision_for_tux);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "cheat_running_stamina")) {
        read_int32_t(L, -1, &data->cheat_running_stamina);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "cheat_double_speed")) {
        read_int32_t(L, -1, &data->cheat_double_speed);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "lazyload")) {
        read_int32_t(L, -1, &data->lazyload);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "show_item_labels")) {
        read_int32_t(L, -1, &data->show_item_labels);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "last_edited_level")) {
        read_int32_t(L, -1, &data->last_edited_level);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "show_all_floor_layers")) {
        read_int32_t(L, -1, &data->show_all_floor_layers);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "difficulty_level")) {
        read_int32_t(L, -1, &data->difficulty_level);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "input_keybinds")) {
        read_keybind_t_array(L, -1, data->input_keybinds, 100);
        lua_pop(L, 1);
    }
}

void write_finepoint(struct auto_string *strout, finepoint *data)
{
    autostr_append(strout, "{\n");
    autostr_append(strout, "x = ");
    write_double(strout, &data->x);
    autostr_append(strout, ",\n");
    autostr_append(strout, "y = ");
    write_double(strout, &data->y);
    autostr_append(strout, ",\n");
    autostr_append(strout, "}");
}

void read_finepoint(lua_State* L, int index, finepoint *data)
{
    if (lua_getfield_or_warn(L, index, "x")) {
        read_double(L, -1, &data->x);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "y")) {
        read_double(L, -1, &data->y);
        lua_pop(L, 1);
    }
}

void write_item(struct auto_string *strout, item *data)
{
    autostr_append(strout, "{\n");
    autostr_append(strout, "pos = ");
    write_gps(strout, &data->pos);
    autostr_append(strout, ",\n");
    autostr_append(strout, "virt_pos = ");
    write_gps(strout, &data->virt_pos);
    autostr_append(strout, ",\n");
    autostr_append(strout, "text_slot_rectangle = ");
    write_SDL_Rect(strout, &data->text_slot_rectangle);
    autostr_append(strout, ",\n");
    autostr_append(strout, "type = ");
    write_int32_t(strout, &data->type);
    autostr_append(strout, ",\n");
    autostr_append(strout, "max_durability = ");
    write_int32_t(strout, &data->max_durability);
    autostr_append(strout, ",\n");
    autostr_append(strout, "current_durability = ");
    write_float(strout, &data->current_durability);
    autostr_append(strout, ",\n");
    autostr_append(strout, "throw_time = ");
    write_float(strout, &data->throw_time);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bonus_to_dex = ");
    write_int32_t(strout, &data->bonus_to_dex);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bonus_to_str = ");
    write_int32_t(strout, &data->bonus_to_str);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bonus_to_physique = ");
    write_int32_t(strout, &data->bonus_to_physique);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bonus_to_cooling = ");
    write_int32_t(strout, &data->bonus_to_cooling);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bonus_to_health_points = ");
    write_int32_t(strout, &data->bonus_to_health_points);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bonus_to_health_recovery = ");
    write_float(strout, &data->bonus_to_health_recovery);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bonus_to_cooling_rate = ");
    write_float(strout, &data->bonus_to_cooling_rate);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bonus_to_attack = ");
    write_int32_t(strout, &data->bonus_to_attack);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bonus_to_all_attributes = ");
    write_int32_t(strout, &data->bonus_to_all_attributes);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bonus_to_armor_class = ");
    write_int32_t(strout, &data->bonus_to_armor_class);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bonus_to_damage = ");
    write_int32_t(strout, &data->bonus_to_damage);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bonus_to_paralyze_enemy = ");
    write_int32_t(strout, &data->bonus_to_paralyze_enemy);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bonus_to_slow_enemy = ");
    write_int32_t(strout, &data->bonus_to_slow_enemy);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bonus_to_light_radius = ");
    write_int32_t(strout, &data->bonus_to_light_radius);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bonus_to_experience_gain = ");
    write_int32_t(strout, &data->bonus_to_experience_gain);
    autostr_append(strout, ",\n");
    autostr_append(strout, "armor_class = ");
    write_int32_t(strout, &data->armor_class);
    autostr_append(strout, ",\n");
    autostr_append(strout, "damage = ");
    write_int32_t(strout, &data->damage);
    autostr_append(strout, ",\n");
    autostr_append(strout, "damage_modifier = ");
    write_int32_t(strout, &data->damage_modifier);
    autostr_append(strout, ",\n");
    autostr_append(strout, "multiplicity = ");
    write_int32_t(strout, &data->multiplicity);
    autostr_append(strout, ",\n");
    autostr_append(strout, "ammo_clip = ");
    write_int32_t(strout, &data->ammo_clip);
    autostr_append(strout, ",\n");
    autostr_append(strout, "inventory_position = ");
    write_point(strout, &data->inventory_position);
    autostr_append(strout, ",\n");
    autostr_append(strout, "upgrade_sockets = ");
    write_upgrade_socket_dynarray(strout, &data->upgrade_sockets);
    autostr_append(strout, ",\n");
    autostr_append(strout, "quality = ");
    write_int32_t(strout, &data->quality);
    autostr_append(strout, ",\n");
    autostr_append(strout, "}");
}

void read_item(lua_State* L, int index, item *data)
{
    if (lua_getfield_or_warn(L, index, "pos")) {
        read_gps(L, -1, &data->pos);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "virt_pos")) {
        read_gps(L, -1, &data->virt_pos);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "text_slot_rectangle")) {
        read_SDL_Rect(L, -1, &data->text_slot_rectangle);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "type")) {
        read_int32_t(L, -1, &data->type);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "max_durability")) {
        read_int32_t(L, -1, &data->max_durability);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "current_durability")) {
        read_float(L, -1, &data->current_durability);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "throw_time")) {
        read_float(L, -1, &data->throw_time);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bonus_to_dex")) {
        read_int32_t(L, -1, &data->bonus_to_dex);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bonus_to_str")) {
        read_int32_t(L, -1, &data->bonus_to_str);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bonus_to_physique")) {
        read_int32_t(L, -1, &data->bonus_to_physique);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bonus_to_cooling")) {
        read_int32_t(L, -1, &data->bonus_to_cooling);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bonus_to_health_points")) {
        read_int32_t(L, -1, &data->bonus_to_health_points);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bonus_to_health_recovery")) {
        read_float(L, -1, &data->bonus_to_health_recovery);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bonus_to_cooling_rate")) {
        read_float(L, -1, &data->bonus_to_cooling_rate);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bonus_to_attack")) {
        read_int32_t(L, -1, &data->bonus_to_attack);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bonus_to_all_attributes")) {
        read_int32_t(L, -1, &data->bonus_to_all_attributes);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bonus_to_armor_class")) {
        read_int32_t(L, -1, &data->bonus_to_armor_class);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bonus_to_damage")) {
        read_int32_t(L, -1, &data->bonus_to_damage);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bonus_to_paralyze_enemy")) {
        read_int32_t(L, -1, &data->bonus_to_paralyze_enemy);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bonus_to_slow_enemy")) {
        read_int32_t(L, -1, &data->bonus_to_slow_enemy);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bonus_to_light_radius")) {
        read_int32_t(L, -1, &data->bonus_to_light_radius);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bonus_to_experience_gain")) {
        read_int32_t(L, -1, &data->bonus_to_experience_gain);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "armor_class")) {
        read_int32_t(L, -1, &data->armor_class);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "damage")) {
        read_int32_t(L, -1, &data->damage);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "damage_modifier")) {
        read_int32_t(L, -1, &data->damage_modifier);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "multiplicity")) {
        read_int32_t(L, -1, &data->multiplicity);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "ammo_clip")) {
        read_int32_t(L, -1, &data->ammo_clip);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "inventory_position")) {
        read_point(L, -1, &data->inventory_position);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "upgrade_sockets")) {
        read_upgrade_socket_dynarray(L, -1, &data->upgrade_sockets);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "quality")) {
        read_int32_t(L, -1, &data->quality);
        lua_pop(L, 1);
    }
}

void write_melee_shot(struct auto_string *strout, melee_shot *data)
{
    autostr_append(strout, "{\n");
    autostr_append(strout, "attack_target_type = ");
    write_uint8_t(strout, &data->attack_target_type);
    autostr_append(strout, ",\n");
    autostr_append(strout, "mine = ");
    write_uint8_t(strout, &data->mine);
    autostr_append(strout, ",\n");
    autostr_append(strout, "bot_target_n = ");
    write_int16_t(strout, &data->bot_target_n);
    autostr_append(strout, ",\n");
    autostr_append(strout, "to_hit = ");
    write_int16_t(strout, &data->to_hit);
    autostr_append(strout, ",\n");
    autostr_append(strout, "damage = ");
    write_int16_t(strout, &data->damage);
    autostr_append(strout, ",\n");
    autostr_append(strout, "owner = ");
    write_int16_t(strout, &data->owner);
    autostr_append(strout, ",\n");
    autostr_append(strout, "time_to_hit = ");
    write_float(strout, &data->time_to_hit);
    autostr_append(strout, ",\n");
    autostr_append(strout, "}");
}

void read_melee_shot(lua_State* L, int index, melee_shot *data)
{
    if (lua_getfield_or_warn(L, index, "attack_target_type")) {
        read_uint8_t(L, -1, &data->attack_target_type);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "mine")) {
        read_uint8_t(L, -1, &data->mine);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "bot_target_n")) {
        read_int16_t(L, -1, &data->bot_target_n);
        lua_pop(L, 1);
    }
    data->bot_target_addr = NULL;
    if (lua_getfield_or_warn(L, index, "to_hit")) {
        read_int16_t(L, -1, &data->to_hit);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "damage")) {
        read_int16_t(L, -1, &data->damage);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "owner")) {
        read_int16_t(L, -1, &data->owner);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "time_to_hit")) {
        read_float(L, -1, &data->time_to_hit);
        lua_pop(L, 1);
    }
}

void write_keybind_t(struct auto_string *strout, keybind_t *data)
{
    autostr_append(strout, "{\n");
    autostr_append(strout, "name = ");
    write_string(strout, &data->name);
    autostr_append(strout, ",\n");
    autostr_append(strout, "key = ");
    write_int32_t(strout, &data->key);
    autostr_append(strout, ",\n");
    autostr_append(strout, "mod = ");
    write_int32_t(strout, &data->mod);
    autostr_append(strout, ",\n");
    autostr_append(strout, "}");
}

void read_keybind_t(lua_State* L, int index, keybind_t *data)
{
    if (lua_getfield_or_warn(L, index, "name")) {
        read_string(L, -1, &data->name);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "key")) {
        read_int32_t(L, -1, &data->key);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "mod")) {
        read_int32_t(L, -1, &data->mod);
        lua_pop(L, 1);
    }
}

void write_blast(struct auto_string *strout, blast *data)
{
    autostr_append(strout, "{\n");
    autostr_append(strout, "pos = ");
    write_gps(strout, &data->pos);
    autostr_append(strout, ",\n");
    autostr_append(strout, "type = ");
    write_int32_t(strout, &data->type);
    autostr_append(strout, ",\n");
    autostr_append(strout, "phase = ");
    write_float(strout, &data->phase);
    autostr_append(strout, ",\n");
    autostr_append(strout, "damage_per_second = ");
    write_float(strout, &data->damage_per_second);
    autostr_append(strout, ",\n");
    autostr_append(strout, "faction = ");
    write_int32_t(strout, &data->faction);
    autostr_append(strout, ",\n");
    autostr_append(strout, "}");
}

void read_blast(lua_State* L, int index, blast *data)
{
    if (lua_getfield_or_warn(L, index, "pos")) {
        read_gps(L, -1, &data->pos);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "type")) {
        read_int32_t(L, -1, &data->type);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "phase")) {
        read_float(L, -1, &data->phase);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "damage_per_second")) {
        read_float(L, -1, &data->damage_per_second);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "faction")) {
        read_int32_t(L, -1, &data->faction);
        lua_pop(L, 1);
    }
}

void write_gps(struct auto_string *strout, gps *data)
{
    autostr_append(strout, "{\n");
    autostr_append(strout, "x = ");
    write_float(strout, &data->x);
    autostr_append(strout, ",\n");
    autostr_append(strout, "y = ");
    write_float(strout, &data->y);
    autostr_append(strout, ",\n");
    autostr_append(strout, "z = ");
    write_int32_t(strout, &data->z);
    autostr_append(strout, ",\n");
    autostr_append(strout, "}");
}

void read_gps(lua_State* L, int index, gps *data)
{
    if (lua_getfield_or_warn(L, index, "x")) {
        read_float(L, -1, &data->x);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "y")) {
        read_float(L, -1, &data->y);
        lua_pop(L, 1);
    }
    if (lua_getfield_or_warn(L, index, "z")) {
        read_int32_t(L, -1, &data->z);
        lua_pop(L, 1);
    }
}

define_write_xxx_array(item);
define_read_xxx_array(item);
define_write_xxx_dynarray(upgrade_socket);
define_read_xxx_dynarray(upgrade_socket);
define_write_xxx_array(automap_data_t);
define_read_xxx_array(automap_data_t);
define_write_xxx_dynarray(item);
define_read_xxx_dynarray(item);
define_write_xxx_array(uint8_t);
define_read_xxx_array(uint8_t);
define_write_xxx_array(float);
define_read_xxx_array(float);
define_write_xxx_array(spell_active);
define_read_xxx_array(spell_active);
define_write_xxx_array(melee_shot);
define_read_xxx_array(melee_shot);
define_write_xxx_array(mission);
define_read_xxx_array(mission);
define_write_xxx_array(bullet);
define_read_xxx_array(bullet);
define_write_xxx_array(int32_t);
define_read_xxx_array(int32_t);
define_write_xxx_array(moderately_finepoint);
define_read_xxx_array(moderately_finepoint);
define_write_xxx_array(string);
define_read_xxx_array(string);
define_write_xxx_array(gps);
define_read_xxx_array(gps);
define_write_xxx_array(blast);
define_read_xxx_array(blast);
