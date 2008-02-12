#include "struct.h"
#include "proto.h"
#include "savestruct.h"
extern FILE * SaveGameFile;


int save_enemy(char * tag, enemy * target)
{
fprintf(SaveGameFile, "<enemy %s>\n",tag?tag:"");
save_int16_t("type", &(target->type));
save_gps("pos", &(target->pos));
save_gps("virt_pos", &(target->virt_pos));
save_finepoint("speed", &(target->speed));
save_float("energy", &(target->energy));
save_float("phase", &(target->phase));
save_float("animation_phase", &(target->animation_phase));
save_int16_t("animation_type", &(target->animation_type));
save_int16_t("nextwaypoint", &(target->nextwaypoint));
save_int16_t("lastwaypoint", &(target->lastwaypoint));
save_int16_t("homewaypoint", &(target->homewaypoint));
save_int16_t("max_distance_to_home", &(target->max_distance_to_home));
save_int32_t("combat_state", &(target->combat_state));
save_float("state_timeout", &(target->state_timeout));
save_float("frozen", &(target->frozen));
save_float("poison_duration_left", &(target->poison_duration_left));
save_float("poison_damage_per_sec", &(target->poison_damage_per_sec));
save_float("paralysation_duration_left", &(target->paralysation_duration_left));
save_float("pure_wait", &(target->pure_wait));
save_float("firewait", &(target->firewait));
save_int16_t("ammo_left", &(target->ammo_left));
save_char("CompletelyFixed", &(target->CompletelyFixed));
save_char("follow_tux", &(target->follow_tux));
save_char("FollowingInflusTail", &(target->FollowingInflusTail));
save_char("SpecialForce", &(target->SpecialForce));
save_int16_t("on_death_drop_item_code", &(target->on_death_drop_item_code));
save_int32_t("marker", &(target->marker));
save_char("is_friendly", &(target->is_friendly));
save_char("has_been_taken_over", &(target->has_been_taken_over));
save_char("attack_target_type", &(target->attack_target_type));
save_enemy_ptr("bot_target", &(target->bot_target));
save_char("attack_run_only_when_direct_line", &(target->attack_run_only_when_direct_line));
save_string("dialog_section_name", &(target->dialog_section_name));
save_string("short_description_text", &(target->short_description_text));
save_char("will_rush_tux", &(target->will_rush_tux));
save_char("persuing_given_course", &(target->persuing_given_course));
save_int16_t("StayHowManyFramesBehind", &(target->StayHowManyFramesBehind));
save_int16_t("StayHowManySecondsBehind", &(target->StayHowManySecondsBehind));
save_char("has_greeted_influencer", &(target->has_greeted_influencer));
save_float("previous_angle", &(target->previous_angle));
save_float("current_angle", &(target->current_angle));
save_float("last_phase_change", &(target->last_phase_change));
save_float("previous_phase", &(target->previous_phase));
save_float("last_combat_step", &(target->last_combat_step));
save_float("TextVisibleTime", &(target->TextVisibleTime));
save_string("TextToBeDisplayed", &(target->TextToBeDisplayed));
save_moderately_finepoint_array("PrivatePathway", (target->PrivatePathway),  MAX_STEPS_IN_GIVEN_COURSE );
save_char("stick_to_waypoint_system_by_default", &(target->stick_to_waypoint_system_by_default));
save_char("bot_stuck_in_wall_at_previous_check", &(target->bot_stuck_in_wall_at_previous_check));
save_float("time_since_previous_stuck_in_wall_check", &(target->time_since_previous_stuck_in_wall_check));
save_enemy_ptr("NEXT", &(target->NEXT));
save_enemy_ptr("PREV", &(target->PREV));
fprintf(SaveGameFile, "</%s>\n", tag);
return 0;
}

int read_enemy(char* buffer, char * tag, enemy * target)
{

	        char * pos = strstr(buffer, tag);
		if ( ! pos ) return 1;
		char * epos = strstr ( pos + 1, tag);
		if ( * (epos - 1) != '/' ) return 2;
		while ( *epos != '>' ) epos++;
		*(epos) = 0;read_int16_t(pos, "type", &(target->type));
read_gps(pos, "pos", &(target->pos));
read_gps(pos, "virt_pos", &(target->virt_pos));
read_finepoint(pos, "speed", &(target->speed));
read_float(pos, "energy", &(target->energy));
read_float(pos, "phase", &(target->phase));
read_float(pos, "animation_phase", &(target->animation_phase));
read_int16_t(pos, "animation_type", &(target->animation_type));
read_int16_t(pos, "nextwaypoint", &(target->nextwaypoint));
read_int16_t(pos, "lastwaypoint", &(target->lastwaypoint));
read_int16_t(pos, "homewaypoint", &(target->homewaypoint));
read_int16_t(pos, "max_distance_to_home", &(target->max_distance_to_home));
read_int32_t(pos, "combat_state", &(target->combat_state));
read_float(pos, "state_timeout", &(target->state_timeout));
read_float(pos, "frozen", &(target->frozen));
read_float(pos, "poison_duration_left", &(target->poison_duration_left));
read_float(pos, "poison_damage_per_sec", &(target->poison_damage_per_sec));
read_float(pos, "paralysation_duration_left", &(target->paralysation_duration_left));
read_float(pos, "pure_wait", &(target->pure_wait));
read_float(pos, "firewait", &(target->firewait));
read_int16_t(pos, "ammo_left", &(target->ammo_left));
read_char(pos, "CompletelyFixed", &(target->CompletelyFixed));
read_char(pos, "follow_tux", &(target->follow_tux));
read_char(pos, "FollowingInflusTail", &(target->FollowingInflusTail));
read_char(pos, "SpecialForce", &(target->SpecialForce));
read_int16_t(pos, "on_death_drop_item_code", &(target->on_death_drop_item_code));
read_int32_t(pos, "marker", &(target->marker));
read_char(pos, "is_friendly", &(target->is_friendly));
read_char(pos, "has_been_taken_over", &(target->has_been_taken_over));
read_char(pos, "attack_target_type", &(target->attack_target_type));
read_enemy_ptr(pos, "bot_target", &(target->bot_target));
read_char(pos, "attack_run_only_when_direct_line", &(target->attack_run_only_when_direct_line));
read_string(pos, "dialog_section_name", &(target->dialog_section_name));
read_string(pos, "short_description_text", &(target->short_description_text));
read_char(pos, "will_rush_tux", &(target->will_rush_tux));
read_char(pos, "persuing_given_course", &(target->persuing_given_course));
read_int16_t(pos, "StayHowManyFramesBehind", &(target->StayHowManyFramesBehind));
read_int16_t(pos, "StayHowManySecondsBehind", &(target->StayHowManySecondsBehind));
read_char(pos, "has_greeted_influencer", &(target->has_greeted_influencer));
read_float(pos, "previous_angle", &(target->previous_angle));
read_float(pos, "current_angle", &(target->current_angle));
read_float(pos, "last_phase_change", &(target->last_phase_change));
read_float(pos, "previous_phase", &(target->previous_phase));
read_float(pos, "last_combat_step", &(target->last_combat_step));
read_float(pos, "TextVisibleTime", &(target->TextVisibleTime));
read_string(pos, "TextToBeDisplayed", &(target->TextToBeDisplayed));
read_moderately_finepoint_array(pos, "PrivatePathway", (target->PrivatePathway),  MAX_STEPS_IN_GIVEN_COURSE );
read_char(pos, "stick_to_waypoint_system_by_default", &(target->stick_to_waypoint_system_by_default));
read_char(pos, "bot_stuck_in_wall_at_previous_check", &(target->bot_stuck_in_wall_at_previous_check));
read_float(pos, "time_since_previous_stuck_in_wall_check", &(target->time_since_previous_stuck_in_wall_check));
read_enemy_ptr(pos, "NEXT", &(target->NEXT));
read_enemy_ptr(pos, "PREV", &(target->PREV));
*epos = '>'; 
return 0;
}

int save_bullet(char * tag, bullet * target)
{
fprintf(SaveGameFile, "<bullet %s>\n",tag?tag:"");
save_int16_t("type", &(target->type));
save_uchar("phase", &(target->phase));
save_char("mine", &(target->mine));
save_gps("pos", &(target->pos));
save_moderately_finepoint("speed", &(target->speed));
save_int16_t("time_in_frames", &(target->time_in_frames));
save_int16_t("damage", &(target->damage));
save_float("time_in_seconds", &(target->time_in_seconds));
save_float("bullet_lifetime", &(target->bullet_lifetime));
save_float("time_to_hide_still", &(target->time_to_hide_still));
save_char("reflect_other_bullets", &(target->reflect_other_bullets));
save_int16_t("owner", &(target->owner));
save_float("angle", &(target->angle));
save_char("was_reflected", &(target->was_reflected));
save_char("ignore_wall_collisions", &(target->ignore_wall_collisions));
save_int16_t("to_hit", &(target->to_hit));
save_char("pass_through_hit_bodies", &(target->pass_through_hit_bodies));
save_char("pass_through_explosions", &(target->pass_through_explosions));
save_int16_t("freezing_level", &(target->freezing_level));
save_float("poison_duration", &(target->poison_duration));
save_float("poison_damage_per_sec", &(target->poison_damage_per_sec));
save_float("paralysation_duration", &(target->paralysation_duration));
save_float("angle_change_rate", &(target->angle_change_rate));
save_float("fixed_offset", &(target->fixed_offset));
save_gps_ptr("owner_pos", &(target->owner_pos));
save_sdl_surface_ptr_array("SurfacePointer", (target->SurfacePointer),  MAX_PHASES_IN_A_BULLET );
save_char("Surfaces_were_generated", &(target->Surfaces_were_generated));
fprintf(SaveGameFile, "</%s>\n", tag);
return 0;
}

int read_bullet(char* buffer, char * tag, bullet * target)
{

	        char * pos = strstr(buffer, tag);
		if ( ! pos ) return 1;
		char * epos = strstr ( pos + 1, tag);
		if ( * (epos - 1) != '/' ) return 2;
		while ( *epos != '>' ) epos++;
		*(epos) = 0;read_int16_t(pos, "type", &(target->type));
read_uchar(pos, "phase", &(target->phase));
read_char(pos, "mine", &(target->mine));
read_gps(pos, "pos", &(target->pos));
read_moderately_finepoint(pos, "speed", &(target->speed));
read_int16_t(pos, "time_in_frames", &(target->time_in_frames));
read_int16_t(pos, "damage", &(target->damage));
read_float(pos, "time_in_seconds", &(target->time_in_seconds));
read_float(pos, "bullet_lifetime", &(target->bullet_lifetime));
read_float(pos, "time_to_hide_still", &(target->time_to_hide_still));
read_char(pos, "reflect_other_bullets", &(target->reflect_other_bullets));
read_int16_t(pos, "owner", &(target->owner));
read_float(pos, "angle", &(target->angle));
read_char(pos, "was_reflected", &(target->was_reflected));
read_char(pos, "ignore_wall_collisions", &(target->ignore_wall_collisions));
read_int16_t(pos, "to_hit", &(target->to_hit));
read_char(pos, "pass_through_hit_bodies", &(target->pass_through_hit_bodies));
read_char(pos, "pass_through_explosions", &(target->pass_through_explosions));
read_int16_t(pos, "freezing_level", &(target->freezing_level));
read_float(pos, "poison_duration", &(target->poison_duration));
read_float(pos, "poison_damage_per_sec", &(target->poison_damage_per_sec));
read_float(pos, "paralysation_duration", &(target->paralysation_duration));
read_float(pos, "angle_change_rate", &(target->angle_change_rate));
read_float(pos, "fixed_offset", &(target->fixed_offset));
read_gps_ptr(pos, "owner_pos", &(target->owner_pos));
read_sdl_surface_ptr_array(pos, "SurfacePointer", (target->SurfacePointer),  MAX_PHASES_IN_A_BULLET );
read_char(pos, "Surfaces_were_generated", &(target->Surfaces_were_generated));
*epos = '>'; 
return 0;
}

int save_point(char * tag, point * target)
{
fprintf(SaveGameFile, "<point %s>\n",tag?tag:"");
save_int32_t("x", &(target->x));
save_int32_t("y", &(target->y));
fprintf(SaveGameFile, "</%s>\n", tag);
return 0;
}

int read_point(char* buffer, char * tag, point * target)
{

	        char * pos = strstr(buffer, tag);
		if ( ! pos ) return 1;
		char * epos = strstr ( pos + 1, tag);
		if ( * (epos - 1) != '/' ) return 2;
		while ( *epos != '>' ) epos++;
		*(epos) = 0;read_int32_t(pos, "x", &(target->x));
read_int32_t(pos, "y", &(target->y));
*epos = '>'; 
return 0;
}

int save_moderately_finepoint(char * tag, moderately_finepoint * target)
{
fprintf(SaveGameFile, "<moderately_finepoint %s>\n",tag?tag:"");
save_float("x", &(target->x));
save_float("y", &(target->y));
fprintf(SaveGameFile, "</%s>\n", tag);
return 0;
}

int read_moderately_finepoint(char* buffer, char * tag, moderately_finepoint * target)
{

	        char * pos = strstr(buffer, tag);
		if ( ! pos ) return 1;
		char * epos = strstr ( pos + 1, tag);
		if ( * (epos - 1) != '/' ) return 2;
		while ( *epos != '>' ) epos++;
		*(epos) = 0;read_float(pos, "x", &(target->x));
read_float(pos, "y", &(target->y));
*epos = '>'; 
return 0;
}

int save_mission(char * tag, mission * target)
{
fprintf(SaveGameFile, "<mission %s>\n",tag?tag:"");
save_string("MissionName", &(target->MissionName));
save_int32_t("MissionWasAssigned", &(target->MissionWasAssigned));
save_int32_t("MissionIsComplete", &(target->MissionIsComplete));
save_int32_t("MissionWasFailed", &(target->MissionWasFailed));
save_int32_t("MissionExistsAtAll", &(target->MissionExistsAtAll));
save_int32_t("AutomaticallyAssignThisMissionAtGameStart", &(target->AutomaticallyAssignThisMissionAtGameStart));
save_int32_t("fetch_item", &(target->fetch_item));
save_int32_t("KillClass", &(target->KillClass));
save_int32_t("KillOne", &(target->KillOne));
save_int32_t("must_clear_first_level", &(target->must_clear_first_level));
save_int32_t("must_clear_second_level", &(target->must_clear_second_level));
save_int32_t("MustReachLevel", &(target->MustReachLevel));
save_point("MustReachPoint", &(target->MustReachPoint));
save_double("MustLiveTime", &(target->MustLiveTime));
save_int32_t("MustBeType", &(target->MustBeType));
save_int32_t("MustBeOne", &(target->MustBeOne));
save_int32_t_array("ListOfActionsToBeTriggeredAtAssignment", (target->ListOfActionsToBeTriggeredAtAssignment),  MAX_MISSION_TRIGGERED_ACTIONS );
save_int32_t_array("ListOfActionsToBeTriggeredAtCompletition", (target->ListOfActionsToBeTriggeredAtCompletition),  MAX_MISSION_TRIGGERED_ACTIONS );
save_int32_t("expanded_display_for_this_mission", &(target->expanded_display_for_this_mission));
fprintf(SaveGameFile, "</%s>\n", tag);
return 0;
}

int read_mission(char* buffer, char * tag, mission * target)
{

	        char * pos = strstr(buffer, tag);
		if ( ! pos ) return 1;
		char * epos = strstr ( pos + 1, tag);
		if ( * (epos - 1) != '/' ) return 2;
		while ( *epos != '>' ) epos++;
		*(epos) = 0;read_string(pos, "MissionName", &(target->MissionName));
read_int32_t(pos, "MissionWasAssigned", &(target->MissionWasAssigned));
read_int32_t(pos, "MissionIsComplete", &(target->MissionIsComplete));
read_int32_t(pos, "MissionWasFailed", &(target->MissionWasFailed));
read_int32_t(pos, "MissionExistsAtAll", &(target->MissionExistsAtAll));
read_int32_t(pos, "AutomaticallyAssignThisMissionAtGameStart", &(target->AutomaticallyAssignThisMissionAtGameStart));
read_int32_t(pos, "fetch_item", &(target->fetch_item));
read_int32_t(pos, "KillClass", &(target->KillClass));
read_int32_t(pos, "KillOne", &(target->KillOne));
read_int32_t(pos, "must_clear_first_level", &(target->must_clear_first_level));
read_int32_t(pos, "must_clear_second_level", &(target->must_clear_second_level));
read_int32_t(pos, "MustReachLevel", &(target->MustReachLevel));
read_point(pos, "MustReachPoint", &(target->MustReachPoint));
read_double(pos, "MustLiveTime", &(target->MustLiveTime));
read_int32_t(pos, "MustBeType", &(target->MustBeType));
read_int32_t(pos, "MustBeOne", &(target->MustBeOne));
read_int32_t_array(pos, "ListOfActionsToBeTriggeredAtAssignment", (target->ListOfActionsToBeTriggeredAtAssignment),  MAX_MISSION_TRIGGERED_ACTIONS );
read_int32_t_array(pos, "ListOfActionsToBeTriggeredAtCompletition", (target->ListOfActionsToBeTriggeredAtCompletition),  MAX_MISSION_TRIGGERED_ACTIONS );
read_int32_t(pos, "expanded_display_for_this_mission", &(target->expanded_display_for_this_mission));
*epos = '>'; 
return 0;
}

int save_tux_t(char * tag, tux_t * target)
{
fprintf(SaveGameFile, "<tux_t %s>\n",tag?tag:"");
save_char("type", &(target->type));
save_char("status", &(target->status));
save_float("current_game_date", &(target->current_game_date));
save_int32_t("current_power_bonus", &(target->current_power_bonus));
save_float("power_bonus_end_date", &(target->power_bonus_end_date));
save_int32_t("current_dexterity_bonus", &(target->current_dexterity_bonus));
save_float("dexterity_bonus_end_date", &(target->dexterity_bonus_end_date));
save_finepoint("speed", &(target->speed));
save_gps("pos", &(target->pos));
save_gps("teleport_anchor", &(target->teleport_anchor));
save_gps("mouse_move_target", &(target->mouse_move_target));
save_enemy_ptr("current_enemy_target", &(target->current_enemy_target));
save_int32_t("mouse_move_target_combo_action_type", &(target->mouse_move_target_combo_action_type));
save_int32_t("mouse_move_target_combo_action_parameter", &(target->mouse_move_target_combo_action_parameter));
save_int32_t("light_bonus_from_tux", &(target->light_bonus_from_tux));
save_int32_t("map_maker_is_present", &(target->map_maker_is_present));
save_float("maxenergy", &(target->maxenergy));
save_float("energy", &(target->energy));
save_float("max_temperature", &(target->max_temperature));
save_float("temperature", &(target->temperature));
save_float("old_temperature", &(target->old_temperature));
save_float("max_running_power", &(target->max_running_power));
save_float("running_power", &(target->running_power));
save_int32_t("running_must_rest", &(target->running_must_rest));
save_int32_t("running_power_bonus", &(target->running_power_bonus));
save_float("health_recovery_rate", &(target->health_recovery_rate));
save_float("cooling_rate", &(target->cooling_rate));
save_int16_t("LastMouse_X", &(target->LastMouse_X));
save_int16_t("LastMouse_Y", &(target->LastMouse_Y));
save_double("busy_time", &(target->busy_time));
save_int32_t("busy_type", &(target->busy_type));
save_double("phase", &(target->phase));
save_float("angle", &(target->angle));
save_float("walk_cycle_phase", &(target->walk_cycle_phase));
save_float("weapon_swing_time", &(target->weapon_swing_time));
save_float("MissionTimeElapsed", &(target->MissionTimeElapsed));
save_float("got_hit_time", &(target->got_hit_time));
save_string("freedroid_version_string", &(target->freedroid_version_string));
save_int32_t("Strength", &(target->Strength));
save_int32_t("Magic", &(target->Magic));
save_int32_t("Dexterity", &(target->Dexterity));
save_int32_t("base_vitality", &(target->base_vitality));
save_int32_t("base_strength", &(target->base_strength));
save_int32_t("base_magic", &(target->base_magic));
save_int32_t("base_dexterity", &(target->base_dexterity));
save_int32_t("Vitality", &(target->Vitality));
save_int32_t("points_to_distribute", &(target->points_to_distribute));
save_float("base_damage", &(target->base_damage));
save_float("damage_modifier", &(target->damage_modifier));
save_float("AC", &(target->AC));
save_float("to_hit", &(target->to_hit));
save_int32_t("lv_1_bot_will_hit_percentage", &(target->lv_1_bot_will_hit_percentage));
save_int32_t("resist_disruptor", &(target->resist_disruptor));
save_int32_t("resist_fire", &(target->resist_fire));
save_int32_t("resist_electricity", &(target->resist_electricity));
save_int32_t("freezing_melee_targets", &(target->freezing_melee_targets));
save_int32_t("double_ranged_damage", &(target->double_ranged_damage));
save_uint32_t("Experience", &(target->Experience));
save_int32_t("exp_level", &(target->exp_level));
save_uint32_t("ExpRequired", &(target->ExpRequired));
save_uint32_t("ExpRequired_previously", &(target->ExpRequired_previously));
save_uint32_t("Gold", &(target->Gold));
save_string("character_name", &(target->character_name));
save_mission_array("AllMissions", (target->AllMissions),  MAX_MISSIONS_IN_GAME );
save_int32_t("marker", &(target->marker));
save_float("LastCrysoundTime", &(target->LastCrysoundTime));
save_float("LastTransferSoundTime", &(target->LastTransferSoundTime));
save_float("TextVisibleTime", &(target->TextVisibleTime));
save_string("TextToBeDisplayed", &(target->TextToBeDisplayed));
save_float("Current_Victim_Resistance_Factor", &(target->Current_Victim_Resistance_Factor));
save_int32_t("FramesOnThisLevel", &(target->FramesOnThisLevel));
save_int32_t("readied_skill", &(target->readied_skill));
save_int32_t_array("SkillLevel", (target->SkillLevel), MAX_NUMBER_OF_PROGRAMS);
save_int32_t_array("base_skill_level", (target->base_skill_level), MAX_NUMBER_OF_PROGRAMS);
save_int32_t("melee_weapon_skill", &(target->melee_weapon_skill));
save_int32_t("ranged_weapon_skill", &(target->ranged_weapon_skill));
save_int32_t("spellcasting_skill", &(target->spellcasting_skill));
save_int32_t("hacking_skill", &(target->hacking_skill));
save_item_array("Inventory", (target->Inventory),  MAX_ITEMS_IN_INVENTORY );
save_item("weapon_item", &(target->weapon_item));
save_item("drive_item", &(target->drive_item));
save_item("armour_item", &(target->armour_item));
save_item("shield_item", &(target->shield_item));
save_item("special_item", &(target->special_item));
save_chatflags_t_array("Chat_Flags", (target->Chat_Flags),  MAX_ANSWERS_PER_PERSON );
save_cookielist_t_array("cookie_list", (target->cookie_list),  MAX_COOKIE_LENGTH );
save_int32_t("is_town_guard_member", &(target->is_town_guard_member));
save_uint16_t_array("KillRecord", (target->KillRecord),  200 );
save_automap_data_t_array("Automap", (target->Automap), 100);
save_int32_t("current_zero_ring_index", &(target->current_zero_ring_index));
save_gps_array("Position_History_Ring_Buffer", (target->Position_History_Ring_Buffer),  MAX_INFLU_POSITION_HISTORY );
save_int32_t("BigScreenMessageIndex", &(target->BigScreenMessageIndex));
save_float("slowdown_duration", &(target->slowdown_duration));
save_float("paralyze_duration", &(target->paralyze_duration));
fprintf(SaveGameFile, "</%s>\n", tag);
return 0;
}

int read_tux_t(char* buffer, char * tag, tux_t * target)
{

	        char * pos = strstr(buffer, tag);
		if ( ! pos ) return 1;
		char * epos = strstr ( pos + 1, tag);
		if ( * (epos - 1) != '/' ) return 2;
		while ( *epos != '>' ) epos++;
		*(epos) = 0;read_char(pos, "type", &(target->type));
read_char(pos, "status", &(target->status));
read_float(pos, "current_game_date", &(target->current_game_date));
read_int32_t(pos, "current_power_bonus", &(target->current_power_bonus));
read_float(pos, "power_bonus_end_date", &(target->power_bonus_end_date));
read_int32_t(pos, "current_dexterity_bonus", &(target->current_dexterity_bonus));
read_float(pos, "dexterity_bonus_end_date", &(target->dexterity_bonus_end_date));
read_finepoint(pos, "speed", &(target->speed));
read_gps(pos, "pos", &(target->pos));
read_gps(pos, "teleport_anchor", &(target->teleport_anchor));
read_gps(pos, "mouse_move_target", &(target->mouse_move_target));
read_enemy_ptr(pos, "current_enemy_target", &(target->current_enemy_target));
read_int32_t(pos, "mouse_move_target_combo_action_type", &(target->mouse_move_target_combo_action_type));
read_int32_t(pos, "mouse_move_target_combo_action_parameter", &(target->mouse_move_target_combo_action_parameter));
read_int32_t(pos, "light_bonus_from_tux", &(target->light_bonus_from_tux));
read_int32_t(pos, "map_maker_is_present", &(target->map_maker_is_present));
read_float(pos, "maxenergy", &(target->maxenergy));
read_float(pos, "energy", &(target->energy));
read_float(pos, "max_temperature", &(target->max_temperature));
read_float(pos, "temperature", &(target->temperature));
read_float(pos, "old_temperature", &(target->old_temperature));
read_float(pos, "max_running_power", &(target->max_running_power));
read_float(pos, "running_power", &(target->running_power));
read_int32_t(pos, "running_must_rest", &(target->running_must_rest));
read_int32_t(pos, "running_power_bonus", &(target->running_power_bonus));
read_float(pos, "health_recovery_rate", &(target->health_recovery_rate));
read_float(pos, "cooling_rate", &(target->cooling_rate));
read_int16_t(pos, "LastMouse_X", &(target->LastMouse_X));
read_int16_t(pos, "LastMouse_Y", &(target->LastMouse_Y));
read_double(pos, "busy_time", &(target->busy_time));
read_int32_t(pos, "busy_type", &(target->busy_type));
read_double(pos, "phase", &(target->phase));
read_float(pos, "angle", &(target->angle));
read_float(pos, "walk_cycle_phase", &(target->walk_cycle_phase));
read_float(pos, "weapon_swing_time", &(target->weapon_swing_time));
read_float(pos, "MissionTimeElapsed", &(target->MissionTimeElapsed));
read_float(pos, "got_hit_time", &(target->got_hit_time));
read_string(pos, "freedroid_version_string", &(target->freedroid_version_string));
read_int32_t(pos, "Strength", &(target->Strength));
read_int32_t(pos, "Magic", &(target->Magic));
read_int32_t(pos, "Dexterity", &(target->Dexterity));
read_int32_t(pos, "base_vitality", &(target->base_vitality));
read_int32_t(pos, "base_strength", &(target->base_strength));
read_int32_t(pos, "base_magic", &(target->base_magic));
read_int32_t(pos, "base_dexterity", &(target->base_dexterity));
read_int32_t(pos, "Vitality", &(target->Vitality));
read_int32_t(pos, "points_to_distribute", &(target->points_to_distribute));
read_float(pos, "base_damage", &(target->base_damage));
read_float(pos, "damage_modifier", &(target->damage_modifier));
read_float(pos, "AC", &(target->AC));
read_float(pos, "to_hit", &(target->to_hit));
read_int32_t(pos, "lv_1_bot_will_hit_percentage", &(target->lv_1_bot_will_hit_percentage));
read_int32_t(pos, "resist_disruptor", &(target->resist_disruptor));
read_int32_t(pos, "resist_fire", &(target->resist_fire));
read_int32_t(pos, "resist_electricity", &(target->resist_electricity));
read_int32_t(pos, "freezing_melee_targets", &(target->freezing_melee_targets));
read_int32_t(pos, "double_ranged_damage", &(target->double_ranged_damage));
read_uint32_t(pos, "Experience", &(target->Experience));
read_int32_t(pos, "exp_level", &(target->exp_level));
read_uint32_t(pos, "ExpRequired", &(target->ExpRequired));
read_uint32_t(pos, "ExpRequired_previously", &(target->ExpRequired_previously));
read_uint32_t(pos, "Gold", &(target->Gold));
read_string(pos, "character_name", &(target->character_name));
read_mission_array(pos, "AllMissions", (target->AllMissions),  MAX_MISSIONS_IN_GAME );
read_int32_t(pos, "marker", &(target->marker));
read_float(pos, "LastCrysoundTime", &(target->LastCrysoundTime));
read_float(pos, "LastTransferSoundTime", &(target->LastTransferSoundTime));
read_float(pos, "TextVisibleTime", &(target->TextVisibleTime));
read_string(pos, "TextToBeDisplayed", &(target->TextToBeDisplayed));
read_float(pos, "Current_Victim_Resistance_Factor", &(target->Current_Victim_Resistance_Factor));
read_int32_t(pos, "FramesOnThisLevel", &(target->FramesOnThisLevel));
read_int32_t(pos, "readied_skill", &(target->readied_skill));
read_int32_t_array(pos, "SkillLevel", (target->SkillLevel), MAX_NUMBER_OF_PROGRAMS);
read_int32_t_array(pos, "base_skill_level", (target->base_skill_level), MAX_NUMBER_OF_PROGRAMS);
read_int32_t(pos, "melee_weapon_skill", &(target->melee_weapon_skill));
read_int32_t(pos, "ranged_weapon_skill", &(target->ranged_weapon_skill));
read_int32_t(pos, "spellcasting_skill", &(target->spellcasting_skill));
read_int32_t(pos, "hacking_skill", &(target->hacking_skill));
read_item_array(pos, "Inventory", (target->Inventory),  MAX_ITEMS_IN_INVENTORY );
read_item(pos, "weapon_item", &(target->weapon_item));
read_item(pos, "drive_item", &(target->drive_item));
read_item(pos, "armour_item", &(target->armour_item));
read_item(pos, "shield_item", &(target->shield_item));
read_item(pos, "special_item", &(target->special_item));
read_chatflags_t_array(pos, "Chat_Flags", (target->Chat_Flags),  MAX_ANSWERS_PER_PERSON );
read_cookielist_t_array(pos, "cookie_list", (target->cookie_list),  MAX_COOKIE_LENGTH );
read_int32_t(pos, "is_town_guard_member", &(target->is_town_guard_member));
read_uint16_t_array(pos, "KillRecord", (target->KillRecord),  200 );
read_automap_data_t_array(pos, "Automap", (target->Automap), 100);
read_int32_t(pos, "current_zero_ring_index", &(target->current_zero_ring_index));
read_gps_array(pos, "Position_History_Ring_Buffer", (target->Position_History_Ring_Buffer),  MAX_INFLU_POSITION_HISTORY );
read_int32_t(pos, "BigScreenMessageIndex", &(target->BigScreenMessageIndex));
read_float(pos, "slowdown_duration", &(target->slowdown_duration));
read_float(pos, "paralyze_duration", &(target->paralyze_duration));
*epos = '>'; 
return 0;
}

int save_item(char * tag, item * target)
{
fprintf(SaveGameFile, "<item %s>\n",tag?tag:"");
save_finepoint("pos", &(target->pos));
save_sdl_rect("text_slot_rectangle", &(target->text_slot_rectangle));
save_int32_t("type", &(target->type));
save_int32_t("currently_held_in_hand", &(target->currently_held_in_hand));
save_int32_t("is_identified", &(target->is_identified));
save_int32_t("max_duration", &(target->max_duration));
save_float("current_duration", &(target->current_duration));
save_float("throw_time", &(target->throw_time));
save_int32_t("prefix_code", &(target->prefix_code));
save_int32_t("suffix_code", &(target->suffix_code));
save_int32_t("bonus_to_dex", &(target->bonus_to_dex));
save_int32_t("bonus_to_str", &(target->bonus_to_str));
save_int32_t("bonus_to_vit", &(target->bonus_to_vit));
save_int32_t("bonus_to_mag", &(target->bonus_to_mag));
save_int32_t("bonus_to_life", &(target->bonus_to_life));
save_int32_t("bonus_to_force", &(target->bonus_to_force));
save_float("bonus_to_health_recovery", &(target->bonus_to_health_recovery));
save_float("bonus_to_cooling_rate", &(target->bonus_to_cooling_rate));
save_int32_t("bonus_to_tohit", &(target->bonus_to_tohit));
save_int32_t("bonus_to_all_attributes", &(target->bonus_to_all_attributes));
save_int32_t("bonus_to_ac_or_damage", &(target->bonus_to_ac_or_damage));
save_int32_t("bonus_to_resist_fire", &(target->bonus_to_resist_fire));
save_int32_t("bonus_to_resist_electricity", &(target->bonus_to_resist_electricity));
save_int32_t("bonus_to_resist_disruptor", &(target->bonus_to_resist_disruptor));
save_int32_t("ac_bonus", &(target->ac_bonus));
save_int32_t("damage", &(target->damage));
save_int32_t("damage_modifier", &(target->damage_modifier));
save_int32_t("multiplicity", &(target->multiplicity));
save_int32_t("ammo_clip", &(target->ammo_clip));
save_point("inventory_position", &(target->inventory_position));
fprintf(SaveGameFile, "</%s>\n", tag);
return 0;
}

int read_item(char* buffer, char * tag, item * target)
{

	        char * pos = strstr(buffer, tag);
		if ( ! pos ) return 1;
		char * epos = strstr ( pos + 1, tag);
		if ( * (epos - 1) != '/' ) return 2;
		while ( *epos != '>' ) epos++;
		*(epos) = 0;read_finepoint(pos, "pos", &(target->pos));
read_sdl_rect(pos, "text_slot_rectangle", &(target->text_slot_rectangle));
read_int32_t(pos, "type", &(target->type));
read_int32_t(pos, "currently_held_in_hand", &(target->currently_held_in_hand));
read_int32_t(pos, "is_identified", &(target->is_identified));
read_int32_t(pos, "max_duration", &(target->max_duration));
read_float(pos, "current_duration", &(target->current_duration));
read_float(pos, "throw_time", &(target->throw_time));
read_int32_t(pos, "prefix_code", &(target->prefix_code));
read_int32_t(pos, "suffix_code", &(target->suffix_code));
read_int32_t(pos, "bonus_to_dex", &(target->bonus_to_dex));
read_int32_t(pos, "bonus_to_str", &(target->bonus_to_str));
read_int32_t(pos, "bonus_to_vit", &(target->bonus_to_vit));
read_int32_t(pos, "bonus_to_mag", &(target->bonus_to_mag));
read_int32_t(pos, "bonus_to_life", &(target->bonus_to_life));
read_int32_t(pos, "bonus_to_force", &(target->bonus_to_force));
read_float(pos, "bonus_to_health_recovery", &(target->bonus_to_health_recovery));
read_float(pos, "bonus_to_cooling_rate", &(target->bonus_to_cooling_rate));
read_int32_t(pos, "bonus_to_tohit", &(target->bonus_to_tohit));
read_int32_t(pos, "bonus_to_all_attributes", &(target->bonus_to_all_attributes));
read_int32_t(pos, "bonus_to_ac_or_damage", &(target->bonus_to_ac_or_damage));
read_int32_t(pos, "bonus_to_resist_fire", &(target->bonus_to_resist_fire));
read_int32_t(pos, "bonus_to_resist_electricity", &(target->bonus_to_resist_electricity));
read_int32_t(pos, "bonus_to_resist_disruptor", &(target->bonus_to_resist_disruptor));
read_int32_t(pos, "ac_bonus", &(target->ac_bonus));
read_int32_t(pos, "damage", &(target->damage));
read_int32_t(pos, "damage_modifier", &(target->damage_modifier));
read_int32_t(pos, "multiplicity", &(target->multiplicity));
read_int32_t(pos, "ammo_clip", &(target->ammo_clip));
read_point(pos, "inventory_position", &(target->inventory_position));
*epos = '>'; 
return 0;
}

int save_finepoint(char * tag, finepoint * target)
{
fprintf(SaveGameFile, "<finepoint %s>\n",tag?tag:"");
save_double("x", &(target->x));
save_double("y", &(target->y));
fprintf(SaveGameFile, "</%s>\n", tag);
return 0;
}

int read_finepoint(char* buffer, char * tag, finepoint * target)
{

	        char * pos = strstr(buffer, tag);
		if ( ! pos ) return 1;
		char * epos = strstr ( pos + 1, tag);
		if ( * (epos - 1) != '/' ) return 2;
		while ( *epos != '>' ) epos++;
		*(epos) = 0;read_double(pos, "x", &(target->x));
read_double(pos, "y", &(target->y));
*epos = '>'; 
return 0;
}

int save_configuration_for_freedroid(char * tag, configuration_for_freedroid * target)
{
fprintf(SaveGameFile, "<configuration_for_freedroid %s>\n",tag?tag:"");
save_float("WantedTextVisibleTime", &(target->WantedTextVisibleTime));
save_int32_t("Draw_Framerate", &(target->Draw_Framerate));
save_int32_t("Draw_Energy", &(target->Draw_Energy));
save_int32_t("Draw_Position", &(target->Draw_Position));
save_int32_t("Influencer_Refresh_Text", &(target->Influencer_Refresh_Text));
save_int32_t("Influencer_Blast_Text", &(target->Influencer_Blast_Text));
save_int32_t("Enemy_Hit_Text", &(target->Enemy_Hit_Text));
save_int32_t("Enemy_Bump_Text", &(target->Enemy_Bump_Text));
save_int32_t("Enemy_Aim_Text", &(target->Enemy_Aim_Text));
save_int32_t("All_Texts_Switch", &(target->All_Texts_Switch));
save_float("Current_BG_Music_Volume", &(target->Current_BG_Music_Volume));
save_float("Current_Sound_FX_Volume", &(target->Current_Sound_FX_Volume));
save_float("current_gamma_correction", &(target->current_gamma_correction));
save_int32_t("StandardEnemyMessages_On_Off", &(target->StandardEnemyMessages_On_Off));
save_int32_t("StandardInfluencerMessages_On_Off", &(target->StandardInfluencerMessages_On_Off));
save_int32_t("Mouse_Input_Permitted", &(target->Mouse_Input_Permitted));
save_int32_t("Mission_Log_Visible", &(target->Mission_Log_Visible));
save_float("Mission_Log_Visible_Time", &(target->Mission_Log_Visible_Time));
save_float("Mission_Log_Visible_Max_Time", &(target->Mission_Log_Visible_Max_Time));
save_int32_t("Inventory_Visible", &(target->Inventory_Visible));
save_int32_t("CharacterScreen_Visible", &(target->CharacterScreen_Visible));
save_int32_t("SkillScreen_Visible", &(target->SkillScreen_Visible));
save_int32_t("Automap_Visible", &(target->Automap_Visible));
save_int32_t("spell_level_visible", &(target->spell_level_visible));
save_int32_t("terminate_on_missing_speech_sample", &(target->terminate_on_missing_speech_sample));
save_int32_t("show_subtitles_in_dialogs", &(target->show_subtitles_in_dialogs));
save_string("freedroid_version_string", &(target->freedroid_version_string));
save_int32_t("skip_light_radius", &(target->skip_light_radius));
save_int32_t("skill_explanation_screen_visible", &(target->skill_explanation_screen_visible));
save_int32_t("enemy_energy_bars_visible", &(target->enemy_energy_bars_visible));
save_int32_t("hog_CPU", &(target->hog_CPU));
save_int32_t("highlighting_mode_full", &(target->highlighting_mode_full));
save_int32_t("omit_tux_in_level_editor", &(target->omit_tux_in_level_editor));
save_int32_t("omit_obstacles_in_level_editor", &(target->omit_obstacles_in_level_editor));
save_int32_t("omit_enemies_in_level_editor", &(target->omit_enemies_in_level_editor));
save_int32_t("level_editor_edit_mode", &(target->level_editor_edit_mode));
save_int32_t("zoom_is_on", &(target->zoom_is_on));
save_int32_t("show_quick_inventory", &(target->show_quick_inventory));
save_int32_t("show_blood", &(target->show_blood));
save_int32_t("show_tooltips", &(target->show_tooltips));
save_int32_t("tux_image_update_policy", &(target->tux_image_update_policy));
save_int32_t("number_of_big_screen_messages", &(target->number_of_big_screen_messages));
save_float("delay_for_big_screen_messages", &(target->delay_for_big_screen_messages));
save_int32_t("enable_cheatkeys", &(target->enable_cheatkeys));
save_int32_t("transparency", &(target->transparency));
save_int32_t("automap_manual_shift_x", &(target->automap_manual_shift_x));
save_int32_t("automap_manual_shift_y", &(target->automap_manual_shift_y));
save_int32_t("screen_width", &(target->screen_width));
save_int32_t("screen_height", &(target->screen_height));
save_int32_t("next_time_width_of_screen", &(target->next_time_width_of_screen));
save_int32_t("next_time_height_of_screen", &(target->next_time_height_of_screen));
save_float("automap_display_scale", &(target->automap_display_scale));
save_int32_t("skip_shadow_blitting", &(target->skip_shadow_blitting));
save_int32_t("language", &(target->language));
save_int32_t("do_fadings", &(target->do_fadings));
save_int32_t("auto_display_to_help", &(target->auto_display_to_help));
save_int32_t("fullscreen_on", &(target->fullscreen_on));
save_int32_t("talk_to_bots_after_takeover", &(target->talk_to_bots_after_takeover));
fprintf(SaveGameFile, "</%s>\n", tag);
return 0;
}

int read_configuration_for_freedroid(char* buffer, char * tag, configuration_for_freedroid * target)
{

	        char * pos = strstr(buffer, tag);
		if ( ! pos ) return 1;
		char * epos = strstr ( pos + 1, tag);
		if ( * (epos - 1) != '/' ) return 2;
		while ( *epos != '>' ) epos++;
		*(epos) = 0;read_float(pos, "WantedTextVisibleTime", &(target->WantedTextVisibleTime));
read_int32_t(pos, "Draw_Framerate", &(target->Draw_Framerate));
read_int32_t(pos, "Draw_Energy", &(target->Draw_Energy));
read_int32_t(pos, "Draw_Position", &(target->Draw_Position));
read_int32_t(pos, "Influencer_Refresh_Text", &(target->Influencer_Refresh_Text));
read_int32_t(pos, "Influencer_Blast_Text", &(target->Influencer_Blast_Text));
read_int32_t(pos, "Enemy_Hit_Text", &(target->Enemy_Hit_Text));
read_int32_t(pos, "Enemy_Bump_Text", &(target->Enemy_Bump_Text));
read_int32_t(pos, "Enemy_Aim_Text", &(target->Enemy_Aim_Text));
read_int32_t(pos, "All_Texts_Switch", &(target->All_Texts_Switch));
read_float(pos, "Current_BG_Music_Volume", &(target->Current_BG_Music_Volume));
read_float(pos, "Current_Sound_FX_Volume", &(target->Current_Sound_FX_Volume));
read_float(pos, "current_gamma_correction", &(target->current_gamma_correction));
read_int32_t(pos, "StandardEnemyMessages_On_Off", &(target->StandardEnemyMessages_On_Off));
read_int32_t(pos, "StandardInfluencerMessages_On_Off", &(target->StandardInfluencerMessages_On_Off));
read_int32_t(pos, "Mouse_Input_Permitted", &(target->Mouse_Input_Permitted));
read_int32_t(pos, "Mission_Log_Visible", &(target->Mission_Log_Visible));
read_float(pos, "Mission_Log_Visible_Time", &(target->Mission_Log_Visible_Time));
read_float(pos, "Mission_Log_Visible_Max_Time", &(target->Mission_Log_Visible_Max_Time));
read_int32_t(pos, "Inventory_Visible", &(target->Inventory_Visible));
read_int32_t(pos, "CharacterScreen_Visible", &(target->CharacterScreen_Visible));
read_int32_t(pos, "SkillScreen_Visible", &(target->SkillScreen_Visible));
read_int32_t(pos, "Automap_Visible", &(target->Automap_Visible));
read_int32_t(pos, "spell_level_visible", &(target->spell_level_visible));
read_int32_t(pos, "terminate_on_missing_speech_sample", &(target->terminate_on_missing_speech_sample));
read_int32_t(pos, "show_subtitles_in_dialogs", &(target->show_subtitles_in_dialogs));
read_string(pos, "freedroid_version_string", &(target->freedroid_version_string));
read_int32_t(pos, "skip_light_radius", &(target->skip_light_radius));
read_int32_t(pos, "skill_explanation_screen_visible", &(target->skill_explanation_screen_visible));
read_int32_t(pos, "enemy_energy_bars_visible", &(target->enemy_energy_bars_visible));
read_int32_t(pos, "hog_CPU", &(target->hog_CPU));
read_int32_t(pos, "highlighting_mode_full", &(target->highlighting_mode_full));
read_int32_t(pos, "omit_tux_in_level_editor", &(target->omit_tux_in_level_editor));
read_int32_t(pos, "omit_obstacles_in_level_editor", &(target->omit_obstacles_in_level_editor));
read_int32_t(pos, "omit_enemies_in_level_editor", &(target->omit_enemies_in_level_editor));
read_int32_t(pos, "level_editor_edit_mode", &(target->level_editor_edit_mode));
read_int32_t(pos, "zoom_is_on", &(target->zoom_is_on));
read_int32_t(pos, "show_quick_inventory", &(target->show_quick_inventory));
read_int32_t(pos, "show_blood", &(target->show_blood));
read_int32_t(pos, "show_tooltips", &(target->show_tooltips));
read_int32_t(pos, "tux_image_update_policy", &(target->tux_image_update_policy));
read_int32_t(pos, "number_of_big_screen_messages", &(target->number_of_big_screen_messages));
read_float(pos, "delay_for_big_screen_messages", &(target->delay_for_big_screen_messages));
read_int32_t(pos, "enable_cheatkeys", &(target->enable_cheatkeys));
read_int32_t(pos, "transparency", &(target->transparency));
read_int32_t(pos, "automap_manual_shift_x", &(target->automap_manual_shift_x));
read_int32_t(pos, "automap_manual_shift_y", &(target->automap_manual_shift_y));
read_int32_t(pos, "screen_width", &(target->screen_width));
read_int32_t(pos, "screen_height", &(target->screen_height));
read_int32_t(pos, "next_time_width_of_screen", &(target->next_time_width_of_screen));
read_int32_t(pos, "next_time_height_of_screen", &(target->next_time_height_of_screen));
read_float(pos, "automap_display_scale", &(target->automap_display_scale));
read_int32_t(pos, "skip_shadow_blitting", &(target->skip_shadow_blitting));
read_int32_t(pos, "language", &(target->language));
read_int32_t(pos, "do_fadings", &(target->do_fadings));
read_int32_t(pos, "auto_display_to_help", &(target->auto_display_to_help));
read_int32_t(pos, "fullscreen_on", &(target->fullscreen_on));
read_int32_t(pos, "talk_to_bots_after_takeover", &(target->talk_to_bots_after_takeover));
*epos = '>'; 
return 0;
}

int save_gps(char * tag, gps * target)
{
fprintf(SaveGameFile, "<gps %s>\n",tag?tag:"");
save_double("x", &(target->x));
save_double("y", &(target->y));
save_int32_t("z", &(target->z));
fprintf(SaveGameFile, "</%s>\n", tag);
return 0;
}

int read_gps(char* buffer, char * tag, gps * target)
{

	        char * pos = strstr(buffer, tag);
		if ( ! pos ) return 1;
		char * epos = strstr ( pos + 1, tag);
		if ( * (epos - 1) != '/' ) return 2;
		while ( *epos != '>' ) epos++;
		*(epos) = 0;read_double(pos, "x", &(target->x));
read_double(pos, "y", &(target->y));
read_int32_t(pos, "z", &(target->z));
*epos = '>'; 
return 0;
}

