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
fprintf(SaveGameFile, "</enemy>\n");
return 0;
}
#if 0
int read_enemy(char* buffer, char * tag, enemy * target)
{
read_int16_t(buffer, "type", &(target->type));
read_gps(buffer, "pos", &(target->pos));
read_gps(buffer, "virt_pos", &(target->virt_pos));
read_finepoint(buffer, "speed", &(target->speed));
read_float(buffer, "energy", &(target->energy));
read_float(buffer, "phase", &(target->phase));
read_float(buffer, "animation_phase", &(target->animation_phase));
read_int16_t(buffer, "animation_type", &(target->animation_type));
read_int16_t(buffer, "nextwaypoint", &(target->nextwaypoint));
read_int16_t(buffer, "lastwaypoint", &(target->lastwaypoint));
read_int16_t(buffer, "homewaypoint", &(target->homewaypoint));
read_int16_t(buffer, "max_distance_to_home", &(target->max_distance_to_home));
read_int32_t(buffer, "combat_state", &(target->combat_state));
read_float(buffer, "state_timeout", &(target->state_timeout));
read_float(buffer, "frozen", &(target->frozen));
read_float(buffer, "poison_duration_left", &(target->poison_duration_left));
read_float(buffer, "poison_damage_per_sec", &(target->poison_damage_per_sec));
read_float(buffer, "paralysation_duration_left", &(target->paralysation_duration_left));
read_float(buffer, "pure_wait", &(target->pure_wait));
read_float(buffer, "firewait", &(target->firewait));
read_int16_t(buffer, "ammo_left", &(target->ammo_left));
read_char(buffer, "CompletelyFixed", &(target->CompletelyFixed));
read_char(buffer, "follow_tux", &(target->follow_tux));
read_char(buffer, "FollowingInflusTail", &(target->FollowingInflusTail));
read_char(buffer, "SpecialForce", &(target->SpecialForce));
read_int16_t(buffer, "on_death_drop_item_code", &(target->on_death_drop_item_code));
read_int32_t(buffer, "marker", &(target->marker));
read_char(buffer, "is_friendly", &(target->is_friendly));
read_char(buffer, "has_been_taken_over", &(target->has_been_taken_over));
read_char(buffer, "attack_target_type", &(target->attack_target_type));
read_enemy_ptr(buffer, "bot_target", &(target->bot_target));
read_char(buffer, "attack_run_only_when_direct_line", &(target->attack_run_only_when_direct_line));
read_string(buffer, "dialog_section_name", &(target->dialog_section_name));
read_string(buffer, "short_description_text", &(target->short_description_text));
read_char(buffer, "will_rush_tux", &(target->will_rush_tux));
read_char(buffer, "persuing_given_course", &(target->persuing_given_course));
read_int16_t(buffer, "StayHowManyFramesBehind", &(target->StayHowManyFramesBehind));
read_int16_t(buffer, "StayHowManySecondsBehind", &(target->StayHowManySecondsBehind));
read_char(buffer, "has_greeted_influencer", &(target->has_greeted_influencer));
read_float(buffer, "previous_angle", &(target->previous_angle));
read_float(buffer, "current_angle", &(target->current_angle));
read_float(buffer, "last_phase_change", &(target->last_phase_change));
read_float(buffer, "previous_phase", &(target->previous_phase));
read_float(buffer, "last_combat_step", &(target->last_combat_step));
read_float(buffer, "TextVisibleTime", &(target->TextVisibleTime));
read_string(buffer, "TextToBeDisplayed", &(target->TextToBeDisplayed));
read_moderately_finepoint_array(buffer, "PrivatePathway", (target->PrivatePathway),  MAX_STEPS_IN_GIVEN_COURSE );
read_char(buffer, "stick_to_waypoint_system_by_default", &(target->stick_to_waypoint_system_by_default));
read_char(buffer, "bot_stuck_in_wall_at_previous_check", &(target->bot_stuck_in_wall_at_previous_check));
read_float(buffer, "time_since_previous_stuck_in_wall_check", &(target->time_since_previous_stuck_in_wall_check));
read_enemy_ptr(buffer, "NEXT", &(target->NEXT));
read_enemy_ptr(buffer, "PREV", &(target->PREV));
return 0;
}
#endif
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
fprintf(SaveGameFile, "</bullet>\n");
return 0;
}

#if 0
int read_bullet(char* buffer, char * tag, bullet * target)
{
read_int16_t(buffer, "type", &(target->type));
read_uchar(buffer, "phase", &(target->phase));
read_char(buffer, "mine", &(target->mine));
read_gps(buffer, "pos", &(target->pos));
read_moderately_finepoint(buffer, "speed", &(target->speed));
read_int16_t(buffer, "time_in_frames", &(target->time_in_frames));
read_int16_t(buffer, "damage", &(target->damage));
read_float(buffer, "time_in_seconds", &(target->time_in_seconds));
read_float(buffer, "bullet_lifetime", &(target->bullet_lifetime));
read_float(buffer, "time_to_hide_still", &(target->time_to_hide_still));
read_char(buffer, "reflect_other_bullets", &(target->reflect_other_bullets));
read_int16_t(buffer, "owner", &(target->owner));
read_float(buffer, "angle", &(target->angle));
read_char(buffer, "was_reflected", &(target->was_reflected));
read_char(buffer, "ignore_wall_collisions", &(target->ignore_wall_collisions));
read_int16_t(buffer, "to_hit", &(target->to_hit));
read_char(buffer, "pass_through_hit_bodies", &(target->pass_through_hit_bodies));
read_char(buffer, "pass_through_explosions", &(target->pass_through_explosions));
read_int16_t(buffer, "freezing_level", &(target->freezing_level));
read_float(buffer, "poison_duration", &(target->poison_duration));
read_float(buffer, "poison_damage_per_sec", &(target->poison_damage_per_sec));
read_float(buffer, "paralysation_duration", &(target->paralysation_duration));
read_float(buffer, "angle_change_rate", &(target->angle_change_rate));
read_float(buffer, "fixed_offset", &(target->fixed_offset));
read_gps_ptr(buffer, "owner_pos", &(target->owner_pos));
read_sdl_surface_ptr_array(buffer, "SurfacePointer", (target->SurfacePointer),  MAX_PHASES_IN_A_BULLET );
read_char(buffer, "Surfaces_were_generated", &(target->Surfaces_were_generated));
return 0;
}
#endif
int save_point(char * tag, point * target)
{
fprintf(SaveGameFile, "<point %s>\n",tag?tag:"");
save_int32_t("x", &(target->x));
save_int32_t("y", &(target->y));
fprintf(SaveGameFile, "</point>\n");
return 0;
}

int save_moderately_finepoint(char * tag, moderately_finepoint * target)
{
fprintf(SaveGameFile, "<moderately_finepoint %s>\n",tag?tag:"");
save_float("x", &(target->x));
save_float("y", &(target->y));
fprintf(SaveGameFile, "</moderately_finepoint>\n");
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
fprintf(SaveGameFile, "</mission>\n");
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
fprintf(SaveGameFile, "</tux_t>\n");
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
fprintf(SaveGameFile, "</item>\n");
return 0;
}

int save_finepoint(char * tag, finepoint * target)
{
fprintf(SaveGameFile, "<finepoint %s>\n",tag?tag:"");
save_double("x", &(target->x));
save_double("y", &(target->y));
fprintf(SaveGameFile, "</finepoint>\n");
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
fprintf(SaveGameFile, "</configuration_for_freedroid>\n");
return 0;
}

int save_gps(char * tag, gps * target)
{
fprintf(SaveGameFile, "<gps %s>\n",tag?tag:"");
save_double("x", &(target->x));
save_double("y", &(target->y));
save_int32_t("z", &(target->z));
fprintf(SaveGameFile, "</gps>\n");
return 0;
}


