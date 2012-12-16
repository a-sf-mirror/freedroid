/*
 *
 *   Copyright (c) 2008-2010 Arthur Huillet 
 *
 *
 *  This file is part of Freedroid
 *
 *  Freedroid is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Freedroid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Freedroid; see the file COPYING. If not, write to the 
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 *
 */

/**
 * This file contains functions related to the Lua scripting interface of FreedroidRPG
 */

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_map.h"

#include "widgets/widgets.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

/* Lua state for dialog execution */
static lua_State *dialog_lua_state;

/* Lua state for config execution */
lua_State *config_lua_state;

lua_State *get_lua_state(enum lua_target target)
{
	if (target == LUA_CONFIG)
		return config_lua_state;

	return dialog_lua_state;
}

/**
 * Retrieve current chat context, and fail with error if there is no dialog
 * currently running.
 */
static struct chat_context *__get_current_chat_context(const char *funcname)
{
	struct chat_context *current_chat_context = chat_get_current_context();
	if (!current_chat_context)
		ErrorMessage(funcname, _("No chat context available on the context stack."), PLEASE_INFORM, IS_FATAL);
	return current_chat_context;
}

#define GET_CURRENT_CHAT_CONTEXT() __get_current_chat_context(__FUNCTION__)

/** Helper to retrieve the enemy a Lua function must act upon.
  * An optional dialog name can be provided, by default the function will act
  * upon the current chat droid.
  */
static enemy *get_enemy_opt(lua_State *L, int param_number, int optional)
{
	const char *dialog = luaL_optstring(L, param_number, NULL);

	if (!dialog) {
		struct chat_context *current_chat_context = GET_CURRENT_CHAT_CONTEXT();
		return current_chat_context->partner;
	}

	enemy *en = get_enemy_with_dialog(dialog);
	if (!optional && !en)
		ErrorMessage(__FUNCTION__, _("Could not find a droid corresponding to the dialog \"%s\"."), PLEASE_INFORM, IS_FATAL, dialog);
	return en;
}

static enemy *get_enemy_arg(lua_State *L, int param_number)
{
	return get_enemy_opt(L, param_number, FALSE);
}

/** Helper to modify the enemy state
  * with a constant set of names.
  */
static void set_bot_state(enemy *en, const char *cmd) 
{
	if (!strcmp(cmd, "follow_tux")) {
		en->follow_tux = TRUE;
		en->CompletelyFixed = FALSE;
	} else if (!strcmp(cmd, "fixed")) {
		en->follow_tux = FALSE;
		en->CompletelyFixed = TRUE;
	} else if (!strcmp(cmd, "free")) {
		en->follow_tux = FALSE;
		en->CompletelyFixed = FALSE;
	} else if (!strcmp(cmd, "home")) {
		en->follow_tux = FALSE;
		en->CompletelyFixed = FALSE;
		en->combat_state = RETURNING_HOME;
	} else if (!strcmp(cmd, "patrol")) {
		en->follow_tux = FALSE;
		en->CompletelyFixed = FALSE;
		en->combat_state = SELECT_NEW_WAYPOINT;
	} else {
		ErrorMessage(__FUNCTION__,
		     "I was called with an invalid state named %s. Accepted values are \"follow_tux\", \"fixed\", \"free\", \"home\", and \"patrol\".\n",
		     PLEASE_INFORM, IS_WARNING_ONLY, cmd);
	}
}

static int lua_event_teleport(lua_State * L)
{
	gps stop_pos = { -1, -1, -1 };
	const char *label = luaL_checkstring(L, 1);
	location TempLocation;
	ResolveMapLabelOnShip(label, &TempLocation);
	reset_visible_levels();
	Teleport(TempLocation.level, TempLocation.x + 0.5, TempLocation.y + 0.5, TRUE, TRUE);
	Me.speed.x = 0.0;
	Me.speed.y = 0.0;
	clear_out_intermediate_points(&stop_pos, Me.next_intermediate_point, MAX_INTERMEDIATE_WAYPOINTS_FOR_TUX);
	clear_active_bullets();
	return 0;
}

static int lua_event_teleport_npc(lua_State * L)
{
	const char *label = luaL_checkstring(L, 1);
	enemy *en = get_enemy_arg(L, 2); 
	location TempLocation;
	ResolveMapLabelOnShip(label, &TempLocation);
	teleport_enemy(en,TempLocation.level, TempLocation.x + 0.5, TempLocation.y + 0.5);
	return 0;
}

static int lua_event_teleport_home(lua_State * L)
{
	TeleportHome();

	return 0;
}

 
static int lua_event_has_teleport_anchor(lua_State * L)
{
	if (Me.teleport_anchor.z != -1)
		lua_pushboolean(L, TRUE);
	else
		lua_pushboolean(L, FALSE);

	return 1;
}

static int lua_event_display_big_message(lua_State * L)
{
	const char *msg = luaL_checkstring(L, 1);
	SetNewBigScreenMessage(msg);
	return 0;
}

static int lua_event_display_console_message(lua_State * L)
{
	const char *msg = luaL_checkstring(L, 1);
	append_new_game_message("%s", msg);
	return 0;
}

static int lua_event_enable_trigger(lua_State * L)
{
	const char *name = luaL_checkstring(L, 1);
	event_modify_trigger_state(name, 1);
	return 0;
}

static int lua_event_disable_trigger(lua_State * L)
{
	const char *name = luaL_checkstring(L, 1);
	event_modify_trigger_state(name, 0);
	return 0;
}

static int lua_event_change_obstacle(lua_State * L)
{
	const char *obslabel = luaL_checkstring(L, 1);
	int type = luaL_checkinteger(L, 2);
	change_obstacle_type(obslabel, type);
	return 0;
}

static int lua_event_get_obstacle_type(lua_State * L)
{
	const char *obslabel = luaL_checkstring(L, 1);
	
	obstacle *our_obstacle = give_pointer_to_obstacle_with_label(obslabel, NULL);

	lua_pushinteger(L, our_obstacle->type);	
	return 1;
}

static int lua_event_delete_obstacle(lua_State * L)
{
	const char *obslabel = luaL_checkstring(L, 1);
	change_obstacle_type(obslabel, -1);
	return 0;
}

static int lua_change_obstacle_message(lua_State *L)
{
	const char *obslabel = luaL_checkstring(L, 1);
	const char *message = luaL_checkstring(L, 2);
	int obstacle_level_num;
	obstacle *o = give_pointer_to_obstacle_with_label(obslabel, &obstacle_level_num);
	level *l = curShip.AllLevels[obstacle_level_num];

	message = strdup(message);

	del_obstacle_extension(l, o, OBSTACLE_EXTENSION_SIGNMESSAGE);
	add_obstacle_extension(l, o, OBSTACLE_EXTENSION_SIGNMESSAGE, (void *)message);

	return 0;
}

static int lua_event_heal_tux(lua_State * L)
{
	Me.energy = Me.maxenergy;
	return 0;
}

static int lua_event_kill_tux(lua_State * L)
{
	Me.energy = -100;
	return 0;
}

static int lua_event_hurt_tux(lua_State * L)
{
	int hp = luaL_checkinteger(L, 1);
	hit_tux(hp);
	return 0;
}

static int lua_event_get_tux_hp(lua_State * L)
{
	lua_pushinteger(L, (int)Me.energy);
	return 1;
}

static int lua_event_get_tux_max_hp(lua_State * L)
{
	lua_pushinteger(L, (int)Me.maxenergy);
	return 1;
}

static int lua_event_heat_tux(lua_State * L)
{
	int temp = luaL_checkinteger(L, 1);
	Me.temperature += temp;
	return 0;
}

static int lua_event_get_tux_cool(lua_State * L)
{
	lua_pushinteger(L, (int)Me.max_temperature - (int)Me.temperature);
	return 1;
}

static int lua_event_improve_skill(lua_State * L)
{
	const char *skilltype = luaL_checkstring(L, 1);
	int *skillptr = NULL;
	if (!strcmp(skilltype, "melee")) {
		skillptr = &Me.melee_weapon_skill;
		SetNewBigScreenMessage(_("Melee fighting ability improved!"));
	} else if (!strcmp(skilltype, "ranged")) {
		skillptr = &Me.ranged_weapon_skill;
		SetNewBigScreenMessage(_("Ranged combat ability improved!"));
	} else if (!strcmp(skilltype, "programming")) {
		skillptr = &Me.spellcasting_skill;
		SetNewBigScreenMessage(_("Programming ability improved!"));
	} else {
		ErrorMessage(__FUNCTION__,
			     "Lua script called me with an incorrect parameter. Accepted values are \"melee\", \"ranged\", and \"programming\".\n",
			     PLEASE_INFORM, IS_WARNING_ONLY);
	}

	if (skillptr) {
		ImproveSkill(skillptr);
	}
	return 0;
}

static int lua_event_get_skill(lua_State * L)
{
	const char *skilltype = luaL_checkstring(L, 1);
	int *skillptr = NULL;
	if (!strcmp(skilltype, "melee")) {
		skillptr = &Me.melee_weapon_skill;
	} else if (!strcmp(skilltype, "ranged")) {
		skillptr = &Me.ranged_weapon_skill;
	} else if (!strcmp(skilltype, "programming")) {
		skillptr = &Me.spellcasting_skill;
	} else {
		ErrorMessage(__FUNCTION__,
			     "Lua script called me with an incorrect parameter. Accepted values are \"melee\", \"ranged\", and \"programming\".\n",
			     PLEASE_INFORM, IS_WARNING_ONLY);
	}

	if (skillptr) {
		lua_pushinteger(L, *skillptr);
	} else
		lua_pushinteger(L, 0);

	return 1;
}

static int lua_event_improve_program(lua_State * L)
{
	const char *pname = luaL_checkstring(L, 1);
	improve_program(get_program_index_with_name(pname));
	return 0;
}

static int lua_event_downgrade_program(lua_State * L)
{
	const char *pname = luaL_checkstring(L, 1);
	downgrade_program(get_program_index_with_name(pname));
	return 0;
}

static int lua_event_get_program_revision(lua_State * L)
{
	const char *pname = luaL_checkstring(L, 1);
	lua_pushinteger(L, Me.skill_level[get_program_index_with_name(pname)]);
	return 1;
}

static int lua_event_delete_item(lua_State * L)
{
	const char *itemname = luaL_checkstring(L, 1);
	int mult = luaL_optinteger(L, 2, 1);
	DeleteInventoryItemsOfType(GetItemIndexByName(itemname), mult);
	return 0;
}

static int lua_event_give_item(lua_State * L)
{
	const char *itemname = luaL_checkstring(L, 1);
	int mult = luaL_optinteger(L, 2, 1);

	item NewItem;
	NewItem = create_item_with_name(itemname, TRUE, mult);

	// Either we put the new item directly into inventory or we issue a warning
	// that there is no room and then drop the item to the floor directly under 
	// the current Tux position.  That can't fail, right?
	char msg[1000];
	if (!give_item(&NewItem)) {
		sprintf(msg, _("Received Item: %s (on floor)"), itemname);
	} else {
		sprintf(msg, _("Received Item: %s"), itemname);
	}
	SetNewBigScreenMessage(msg);
	return 0;
}

static int lua_event_sell_item(lua_State *L)
{
	struct chat_context *current_chat_context = GET_CURRENT_CHAT_CONTEXT();

	const char *itemname = luaL_checkstring(L, 1);
	int weight = luaL_optint(L, 2, 1);
	const char *charname = luaL_optstring(L, 3, current_chat_context->partner->dialog_section_name);

	npc_add_shoplist(charname, itemname, weight);

	return 0;
}

static int lua_event_count_item_backpack(lua_State * L)
{
	const char *itemname = luaL_checkstring(L, 1);

	lua_pushinteger(L, CountItemtypeInInventory(GetItemIndexByName(itemname)));

	return 1;
}

static int lua_event_has_item_equipped(lua_State * L)
{
	const char *itemname = luaL_checkstring(L, 1);
	int item = GetItemIndexByName(itemname);
	if ((Me.weapon_item.type == item) || (Me.drive_item.type == item) 
		|| (Me.armour_item.type == item) || (Me.shield_item.type == item) 
		|| (Me.special_item.type == item)) {
		lua_pushboolean(L, TRUE);
	} else {
		lua_pushboolean(L, FALSE);
	}
	return 1;
}

static int lua_event_equip_item(lua_State * L)
{
	const char *item_name = luaL_checkstring(L, 1);

	if (!item_name) {
		ErrorMessage(__FUNCTION__, "Tried to add item without a name\n", PLEASE_INFORM,
			     IS_WARNING_ONLY);
		return 0;
	}
	item new_item = create_item_with_name(item_name, TRUE, 1);
	equip_item(&new_item);
	SetNewBigScreenMessage(_("1 Item received!"));
	return 0;
}

static int lua_set_death_item(lua_State * L)
{
	const char *item_name = luaL_checkstring(L, 1);
	enemy *en = get_enemy_arg(L, 2);
	if (!strcmp(item_name, "NONE"))
		en->on_death_drop_item_code = -1;
	else
		en->on_death_drop_item_code = GetItemIndexByName(item_name);
	return 0;
}

static int lua_event_add_diary_entry(lua_State * L)
{
	const char *mis_name = luaL_checkstring(L, 1);
	const char *text = luaL_checkstring(L, 2);

	mission_diary_add(mis_name, text);
	return 0;
}

static int lua_event_has_met(lua_State *L)
{
	const char *npc_name = luaL_checkstring(L, 1);
	struct npc *npc = npc_get(npc_name);
	lua_pushboolean(L, npc->chat_character_initialized);
	return 1;
}

static int lua_event_assign_mission(lua_State * L)
{
	const char *misname = luaL_checkstring(L, 1);
	const char *diarytext = luaL_optstring(L, 2, NULL);

	AssignMission(misname);
	if (diarytext != NULL)
		mission_diary_add(misname, diarytext);

	return 0;
}

static int lua_event_complete_mission(lua_State * L)
{
	const char *misname = luaL_checkstring(L, 1);
	const char *diarytext = luaL_optstring(L, 2, NULL);

	CompleteMission(misname);
	if (diarytext != NULL)
		mission_diary_add(misname, diarytext);

	return 0;
}

static int lua_event_is_mission_assigned(lua_State * L)
{
	const char *misname = luaL_checkstring(L, 1);

	lua_pushboolean(L, Me.AllMissions[GetMissionIndexByName(misname)].MissionWasAssigned);

	return 1;
}

static int lua_event_is_mission_complete(lua_State * L)
{
	const char *misname = luaL_checkstring(L, 1);

	lua_pushboolean(L, Me.AllMissions[GetMissionIndexByName(misname)].MissionIsComplete);

	return 1;
}

static int lua_event_give_xp(lua_State * L)
{
	int xp = luaL_checkinteger(L, 1) * Me.experience_factor;
	char tmpstr[150];
	Me.Experience += xp;
	sprintf(tmpstr, _("+%d Experience Points"), xp);
	SetNewBigScreenMessage(tmpstr);
	return 0;
}

static int lua_event_eat_training_points(lua_State * L)
{
	int nb = luaL_checkinteger(L, 1);
	Me.points_to_distribute -= nb;
	return 0;
}

static int lua_event_get_training_points(lua_State * L)
{
	lua_pushinteger(L, Me.points_to_distribute);
	return 1;
}

static int lua_event_add_gold(lua_State * L)
{
	int nb = luaL_checkinteger(L, 1);
	char tmpstr[150];

	if (nb < 0 && -nb > Me.Gold) {
		ErrorMessage(__FUNCTION__, "Tried to remove %d gold from the player that only has %d!\n", PLEASE_INFORM,
			     IS_WARNING_ONLY, -nb, Me.Gold);
		nb = -Me.Gold;
	}

	Me.Gold += nb;

	if (nb > 0)
		sprintf(tmpstr, _("Gained %d valuable circuits!"), nb);
	else
		sprintf(tmpstr, _("Lost %d valuable circuits!"), nb);

	SetNewBigScreenMessage(tmpstr);
	return 0;
}

static int lua_event_get_gold(lua_State * L)
{
	lua_pushinteger(L, Me.Gold);
	return 1;
}

static int lua_event_change_stat(lua_State * L)
{
	const char *characteristic = luaL_checkstring(L, 1);
	int nb = luaL_checkinteger(L, 2);
	int *statptr = NULL;

	if (!strcmp(characteristic, "strength")) {
		statptr = &Me.base_strength;
	} else if (!strcmp(characteristic, "dexterity")) {
		statptr = &Me.base_dexterity;
	} else if (!strcmp(characteristic, "CPU")) {
		statptr = &Me.base_cooling;
	} else if (!strcmp(characteristic, "vitality")) {
		statptr = &Me.base_physique;
	} else {
		ErrorMessage(__FUNCTION__,
			     "I was called with characteristic name %s - accepted values are \"strength\", \"dexterity\", \"CPU\", and \"vitality\".",
			     PLEASE_INFORM, IS_WARNING_ONLY, characteristic);
		return 0;
	}

	*statptr += nb;
	return 0;
}

static int lua_event_respawn_level(lua_State * L)
{
	int lnb = luaL_checkinteger(L, 1);

	respawn_level(lnb);

	return 0;
}

static int lua_event_trade_with(lua_State * L)
{
	const char *cname = luaL_checkstring(L, 1);

	struct npc *n = npc_get(cname);
	InitTradeWithCharacter(n);

	return 0;
}

static int lua_event_upgrade_items(lua_State * L)
{
	item_upgrade_ui();

	return 0;
}

static int lua_event_craft_addons(lua_State * L)
{
	addon_crafting_ui();

	return 0;
}

static int lua_event_heal_npc(lua_State * L)
{
	enemy *en = get_enemy_arg(L, 1); 
	en->energy = Droidmap[en->type].maxenergy;
	return 0;
}

static int lua_get_npc_damage_amount(lua_State * L)
{
	enemy *en = get_enemy_arg(L, 1);
	lua_pushinteger(L, (int)(Droidmap[en->type].maxenergy - en->energy));
	return 1;
}

static int lua_get_npc_max_health(lua_State * L)
{
	enemy *en = get_enemy_arg(L, 1);
	lua_pushinteger(L, (int)(Droidmap[en->type].maxenergy));
	return 1;
}

static int lua_event_npc_dead(lua_State *L)
{
	const char *cname = luaL_checkstring(L, 1);
	enemy *erot;
	int dead = 0;

	BROWSE_DEAD_BOTS(erot) {
		if (!strcmp(erot->dialog_section_name, cname)) {
			dead = 1;
			break;
		}
	}
	
	lua_pushboolean(L, dead);
	return 1;
}

static int lua_event_freeze_tux_npc(lua_State * L)
{
	float duration = luaL_checknumber(L, 1);
	enemy *en = get_enemy_arg(L, 2); 
	en->paralysation_duration_left = duration;
	Me.paralyze_duration = duration;
	return 0;
}

static int lua_chat_player_name(lua_State * L)
{
	if (Me.character_name)
		lua_pushstring(L, Me.character_name);
	else
		lua_pushstring(L, "");
	return 1;
}

static int lua_chat_says(lua_State * L)
{
	const char *answer = luaL_checkstring(L, 1);
	int no_wait = !strcmp(luaL_optstring(L, 2, "WAIT"), "NO_WAIT");

	chat_add_response(L_(answer));

	if (no_wait)
		return 0;

	struct chat_context *current_chat_context = GET_CURRENT_CHAT_CONTEXT();
	current_chat_context->wait_user_click = TRUE;
	// The Lua manual says that:
	// "lua_yield() should only be called as the return expression of a C function"
	// But the "should" is actually a "must"...
	return lua_yield(L, 0);
}

static int lua_chat_push_topic(lua_State * L)
{
	const char *topic = luaL_checkstring(L, 1);
	
	chat_push_topic(topic);

	return 0;
}

static int lua_chat_pop_topic(lua_State * L)
{
	chat_pop_topic();

	return 0;
}

static int lua_chat_run_subdialog(lua_State * L)
{
	const char *tmp_filename = luaL_checkstring(L, 1);

	if (!stack_subdialog(tmp_filename))
		return 0;

	// Yield the current dialog script, to let the chat engine run the
	// subdialog.

	// Note: The Lua manual says that: "lua_yield() should only be called as the
	// return expression of a C function". But the "should" is actually a "must"...
	return lua_yield(L, 0);
}

static int lua_start_chat(lua_State * L)
{
	enemy *en = get_enemy_arg(L, 1); 

	if (!stack_dialog(en))
		return 0;

	// Yield the current dialog script, to let the chat engine run the
	// dialog.

	// Note: The Lua manual says that: "lua_yield() should only be called as the
	// return expression of a C function". But the "should" is actually a "must"...
	return lua_yield(L, 0);
}

static int lua_chat_set_next_node(lua_State * L)
{
	int nodenb = luaL_checkint(L, 1);

	struct chat_context *current_chat_context = GET_CURRENT_CHAT_CONTEXT();

	if (!current_chat_context->dialog_options[nodenb].exists) {
		ErrorMessage(__FUNCTION__, "A dialog tried to run node %d that does not exist.\n", PLEASE_INFORM, IS_WARNING_ONLY, nodenb);
		return 0;
	}
	current_chat_context->current_option = nodenb;

	return 0;
}

static int lua_chat_end_dialog(lua_State * L)
{
	struct chat_context *current_chat_context = GET_CURRENT_CHAT_CONTEXT();
	current_chat_context->end_dialog = 1;
	return 0;
}

static int lua_chat_partner_started(lua_State * L)
{
	struct chat_context *current_chat_context = GET_CURRENT_CHAT_CONTEXT();
	lua_pushboolean(L, current_chat_context->partner_started);
	return 1;
}

static int __lua_chat_toggle_node(lua_State * L, int value)
{
	int i = 1, flag;

	struct chat_context *current_chat_context = GET_CURRENT_CHAT_CONTEXT();

	while ((flag = luaL_optinteger(L, i, -1)) != -1) {
		i++;
		// for each optional node
		if (!current_chat_context->dialog_options[flag].exists) {
			ErrorMessage(__FUNCTION__, "A dialog tried to %s chat node %d that does not exist.\n", PLEASE_INFORM,
				     IS_WARNING_ONLY, value == 1 ? "enable" : "disable", flag);
			continue;
		}
		current_chat_context->dialog_flags[flag] = value;
	}
	return 0;
}

static int lua_chat_enable_node(lua_State * L)
{
	return __lua_chat_toggle_node(L, 1);
}

static int lua_chat_disable_node(lua_State * L)
{
	return __lua_chat_toggle_node(L, 0);
}

static int lua_chat_drop_dead(lua_State * L)
{
	enemy *en = get_enemy_arg(L, 1); 
	hit_enemy(en, en->energy + 1, 0, Droidmap[en->type].is_human - 2, 0);

	struct chat_context *current_chat_context = GET_CURRENT_CHAT_CONTEXT();

	if (en == current_chat_context->partner)
		current_chat_context->end_dialog = 1;
	return 0;
}

static int lua_chat_set_bot_state(lua_State * L)
{
	const char *cmd = luaL_checkstring(L, 1);
	enemy *en = get_enemy_arg(L, 2); 
	set_bot_state(en, cmd);
	return 0;
}

static int lua_chat_broadcast_bot_state(lua_State * L)
{
	const char *cmd = luaL_checkstring(L, 1);

	struct chat_context *current_chat_context = GET_CURRENT_CHAT_CONTEXT();

	const char *dialogname = current_chat_context->partner->dialog_section_name;
	enemy *en;
	BROWSE_LEVEL_BOTS(en, current_chat_context->partner->pos.z) {
		if (!strcmp(en->dialog_section_name, dialogname) && (is_friendly(en->faction, FACTION_SELF))) {
			set_bot_state(en, cmd);
		}
	}
	return 0;
}

static int lua_set_bot_destination(lua_State * L)
{
	const char *label = luaL_checkstring(L, 1);
	enemy *en = get_enemy_arg(L, 2); 
	location TempLocation;
	ResolveMapLabelOnShip(label, &TempLocation);
	struct level *lvl = curShip.AllLevels[TempLocation.level];
	int destinationwaypoint = get_waypoint(lvl, TempLocation.x + 0.5, TempLocation.y + 0.5);

	if (TempLocation.level !=  en->pos.z) {
		ErrorMessage(__FUNCTION__, "\
				Sending bot %s to map label %s (found on level %d) cannot be done because the bot\n\
				is not on the same level (z = %d). Doing nothing.",
				PLEASE_INFORM, IS_WARNING_ONLY, en->dialog_section_name, label, TempLocation.level, en->pos.z);
		return 0;
	}

	if (destinationwaypoint == -1) {
		ErrorMessage(__FUNCTION__, "\
				Map label %s (found on level %d) does not have a waypoint. Cannot send bot %s\n\
				to this location. Doing nothing.\n\
				Location coordinates x=%d, y=%d.",
				PLEASE_INFORM, IS_WARNING_ONLY, label, TempLocation.level, en->dialog_section_name, TempLocation.x, TempLocation.y);
		return 0;
	}

	clear_out_intermediate_points(&en->pos, &en->PrivatePathway[0], 5);
	en->lastwaypoint = destinationwaypoint;
	en->nextwaypoint = destinationwaypoint;
	en->combat_state = TURN_TOWARDS_NEXT_WAYPOINT;
	return 0;
}

static int lua_set_rush_tux(lua_State * L)
{
	const uint8_t cmd = (uint8_t)luaL_checkinteger(L, 1);
	enemy *en = get_enemy_arg(L, 2);
	en->will_rush_tux = cmd;
	return 0;
}

static int lua_will_rush_tux(lua_State * L)
{
	enemy *en = get_enemy_arg(L, 1);
	lua_pushboolean(L, en->will_rush_tux);
	return 1;
}

static int lua_chat_takeover(lua_State * L)
{
	int opponent_capsules = luaL_checkinteger(L, 1);
	int player_capsules = 2 + Me.skill_level[get_program_index_with_name("Hacking")];
	int game_length = luaL_optint(L, 2, 100);

	int won = do_takeover(player_capsules, opponent_capsules, game_length, NULL);

	lua_pushboolean(L, won);

	return 1;
}

static int lua_chat_bot_exists(lua_State *L)
{
	int exists = get_enemy_opt(L, 1, TRUE) != NULL;
	lua_pushboolean(L, exists);
	return 1;
}

static int lua_chat_get_bot_type(lua_State * L)
{
	enemy *en = get_enemy_arg(L, 1); 
	lua_pushstring(L, Droidmap[en->type].droidname);
	return 1;
}

static int lua_event_bot_class(lua_State * L)
{
       enemy *en = get_enemy_arg(L, 1);
       lua_pushinteger(L, Droidmap[en->type].class);
       return 1;
}

static int lua_chat_get_bot_name(lua_State * L)
{
	enemy *en = get_enemy_arg(L, 1); 
	lua_pushstring(L, en->short_description_text);
	return 1;
}

static int lua_chat_set_bot_name(lua_State * L)
{
	const char *bot_name = luaL_checkstring(L, 1);
	enemy *en = get_enemy_arg(L, 2); 
	free(en->short_description_text);
	en->short_description_text = strdup(bot_name);
	return 0;
}

static int lua_difficulty_level(lua_State * L)
{
	lua_pushnumber(L, GameConfig.difficulty_level);
	return 1;
}

static int lua_set_npc_faction(lua_State *L)
{
	const char *fact = luaL_checkstring(L, 1);
	enemy *en = get_enemy_arg(L, 2); 
	en->faction = get_faction_id(fact);
	return 0;
}

static int lua_user_input_string(lua_State *L)
{
	const char *title = luaL_checkstring(L, 1);
	const char *default_str = luaL_optstring(L, 2, "");

	const char *str = GetEditableStringInPopupWindow(100, title, default_str);

	if (!str)
		str = strdup("");

	lua_pushstring(L, str);

	free((void *)str);
	return 1;
}

static int lua_set_faction_state(lua_State *L)
{
	const char *fact_name = luaL_checkstring(L, 1);
	const char *state_str = luaL_checkstring(L, 2);
	const char *fact2_name = luaL_optstring(L, 3, "self");

	enum faction_state state;
	enum faction_id fact_id = get_faction_id(fact_name);
	enum faction_id fact2_id = get_faction_id(fact2_name);
	
	if (!strcmp(state_str, "hostile"))
		state = HOSTILE;
	else if (!strcmp(state_str, "friendly"))
		state = FRIENDLY;
	else {
		ErrorMessage(__FUNCTION__, "Unknown faction state %s.", PLEASE_INFORM, IS_WARNING_ONLY, state_str);
		return 0;
	}

	set_faction_state(fact_id, fact2_id, state);

	return 0;
}

static int lua_create_droid(lua_State *L)
{
	const char *map_label = luaL_checkstring(L, 1);
	const char *type_name = luaL_checkstring(L, 2);
	const char *fact_name = luaL_optstring(L, 3, "ms");
	const char *dialog    = luaL_optstring(L, 4, "AfterTakeover");
	location loc;
	int type;

	ResolveMapLabelOnShip(map_label, &loc);

	type = get_droid_type(type_name);

	enemy *en = enemy_new(type);
	enemy_reset(en);
	en->pos.x = loc.x + 0.5;
	en->pos.y = loc.y + 0.5;
	en->pos.z = loc.level;
	en->faction = get_faction_id(fact_name);
	en->dialog_section_name = strdup(dialog);
	enemy_insert_into_lists(en, TRUE);

	return 0;
}

luaL_Reg lfuncs[] = {
	/* teleport(string map_label) 
	 * Teleports the player to the given map label.
	 */
	{"teleport", lua_event_teleport}
	,
	/* teleport_npc(string map_label, [dialog name])
	 * Teleports the current npc, or named npc to the given map label
	 */
	{"teleport_npc", lua_event_teleport_npc}
	,
	/* teleport_home(string map_label) 
	 * Teleports the player to the home.
	 */
	{"teleport_home", lua_event_teleport_home}
	,
	/* has_teleport_anchor()
	 * Return true if a teleport anchor is active.
	 */
	{"has_teleport_anchor", lua_event_has_teleport_anchor}
	,
	/* display_big_message(string msg)
	 * Displays a big vanishing message on screen (seen in game, not in the dialog).
	 */
	{"display_big_message", lua_event_display_big_message}
	,
	/* use display_console_message(string msg), supports [b] and [/b]
	 * Displays a message on the game console.
	 */
	{"event_display_console_message", lua_event_display_console_message}
	,

	/* enable_trigger(string event_name)
	 * disable_trigger(string event_name)
	 * Enables/Disables the event trigger with the given name
	 */
	{"enable_trigger", lua_event_enable_trigger}
	,
	{"disable_trigger", lua_event_disable_trigger}
	,

	/* change_obstacle_type(string obstacle_label, int obstacle_type)
	 * Changes the obstacle to the given state.
	 */
	{"change_obstacle_type", lua_event_change_obstacle},
	{"get_obstacle_type", lua_event_get_obstacle_type}
	,
	/* del_obstacle(string obstacle_label)
	 * Delete the given obstacle
	 */
	{"del_obstacle", lua_event_delete_obstacle}
	,

	/* change_obstacle_message(string obstacle_label, string message)
	 * Change the SIGNMESSAGE of the given obstacle.
	 */
	{"change_obstacle_message", lua_change_obstacle_message}
	,

	/* kill_tux() - kills Tux
	 * heal_tux() - heal_tux completely heals Tux
	 * hurt_tux(int how_many_hp_to_remove) - removes the given number of health points.
	 * 	This number can obviously be negative.
	 * heat_tux(int amount)	- Increases temperature (=removes cooling), number can be negative.
	 */
	{"kill_tux", lua_event_kill_tux}
	,
	{"heal_tux", lua_event_heal_tux}
	,
	{"hurt_tux", lua_event_hurt_tux}
	,
	{"heat_tux", lua_event_heat_tux}
	,
	/* get_tux_hp()             - Returns Tux's current health
	 * get_tux_max_hp()         - Returns Tux's current maximum health
	 * see also: tux_hp_ratio() - Returns the ratio of the two
	 */
	{"get_tux_hp", lua_event_get_tux_hp}
	,
	{"get_tux_max_hp", lua_event_get_tux_max_hp}
	,
	/* get_tux_cool()		- Returns Tux's current remaining heat absorbing capabilities
	 */
	{"get_tux_cool", lua_event_get_tux_cool}
	,
	/* improve_skill(string skill_name)
	 * get_skill()
	 * improve_skill improves one of the three "melee", "ranged" and "programming" skills
	 * by one level.
	 * get_skill returns the current level (as an integer) of one of the three skills.
	 */
	{"improve_skill", lua_event_improve_skill}
	,
	{"get_skill", lua_event_get_skill}
	,

	/* improve_program(string program_name)
	 * Improve the program given by one level.
	 * get_program_revision(string program_name) returns current program revision level
	 */
	{"improve_program", lua_event_improve_program}
	,
	{"downgrade_program", lua_event_downgrade_program}
	,
	{"get_program_revision", lua_event_get_program_revision}
        ,

	/* del_item_backpack(string item_name[, int multiplicity = 1])
	 * add_item(string item_name, int multiplicity)
	 * - Deletes or adds the given number of items from/to the inventory.
	 *
	 * count_item_backpack(string item_name)
	 * - returns the number of items of the given name currently in the inventory.
	 *
	 * has_item_equipped(string item_name)
	 * - returns true when the item is equipped
	 */
	{"del_item_backpack", lua_event_delete_item}
	,
	{"add_item", lua_event_give_item}
	,
	{"count_item_backpack", lua_event_count_item_backpack}
	,
	{"has_item_equipped", lua_event_has_item_equipped}
	,
	{"equip_item", lua_event_equip_item}
	,
	{"sell_item", lua_event_sell_item}
	,
	/* set_death_item(string item_name [, string  npc]) 
	 * changes the item dropped when the droid dies
	*/
	{"set_death_item", lua_set_death_item}
	,
	{"add_diary_entry", lua_event_add_diary_entry}
	,
	{"has_met", lua_event_has_met}
	,
	{"assign_quest", lua_event_assign_mission}
	,
	{"has_quest", lua_event_is_mission_assigned}
	,
	{"complete_quest", lua_event_complete_mission}
	,
	{"done_quest", lua_event_is_mission_complete}
	,
	{"add_xp", lua_event_give_xp}
	,
	{"del_training_points", lua_event_eat_training_points}
	,
	{"get_training_points", lua_event_get_training_points}
	,
	{"add_gold", lua_event_add_gold}
	,
	{"get_gold", lua_event_get_gold}
	,
	{"change_stat", lua_event_change_stat}
	,
	{"respawn_level", lua_event_respawn_level}
	,
	{"trade_with", lua_event_trade_with}
	,
	{"upgrade_items", lua_event_upgrade_items}
	,
	{"craft_addons", lua_event_craft_addons}
	,
	{"get_player_name", lua_chat_player_name} 
	,
	{"chat_says", lua_chat_says}
	,
	{"topic", lua_chat_push_topic}
	,
	{"pop_topic", lua_chat_pop_topic}
	,
	{"run_subdialog", lua_chat_run_subdialog}
	,
	{"start_chat", lua_start_chat}
	,
	{"set_next_node", lua_chat_set_next_node}
	,
	{"end_dialog", lua_chat_end_dialog}
	,
	/* NOTE:  if (partner_started())  will always be true
	 * if rush_tux is 1
	 */
	{"partner_started", lua_chat_partner_started}
	,
	{"enable_node", lua_chat_enable_node}
	,
	{"disable_node", lua_chat_disable_node}
	,

	{"drop_dead", lua_chat_drop_dead}
	,
	{"bot_exists", lua_chat_bot_exists}
	,
	{"set_bot_state", lua_chat_set_bot_state}
	,
	{"set_bot_destination", lua_set_bot_destination}
	,
	{"broadcast_bot_state", lua_chat_broadcast_bot_state}
	,
	/* set_rush_tux()   - Sets or unsets if the NPC should rush and talk to Tux
	 * will_rush_tux() - Checks if the NPC is planning on rushing Tux
	 */
	{"set_rush_tux", lua_set_rush_tux}
	,
	{"will_rush_tux", lua_will_rush_tux}
	,
	{"takeover", lua_chat_takeover}
	,
	/* heal_npc([dialog])			- Returns the NPC's current health
	 * npc_damage_amount([dialog])		- Returns the current damage for the NPC
	 * npc_max_health([dialog])		- Returns the max possible health for the NPC
	 * see also: npc_damage_ratio([dialog]) - Returns the ratio of the two
	 */
	{"heal_npc", lua_event_heal_npc}
	,
	{"npc_damage_amount", lua_get_npc_damage_amount}
	,
	{"npc_max_health", lua_get_npc_max_health}
	,
	{"freeze_tux_npc", lua_event_freeze_tux_npc}
	,
	{"npc_dead", lua_event_npc_dead},
	/* bot_type() tells you what model
	   bot_class() tells you the class of a bot
	   bot_name() tells you what name it displays
	   set_bot_name() puts a new name in
	 */
	{"bot_type", lua_chat_get_bot_type},

	{"bot_class", lua_event_bot_class},

	{"bot_name", lua_chat_get_bot_name},

	{"set_bot_name", lua_chat_set_bot_name},

	{"difficulty_level", lua_difficulty_level},

	{"set_npc_faction", lua_set_npc_faction},
	{"set_faction_state", lua_set_faction_state},

	{"user_input_string", lua_user_input_string},

	{"create_droid", lua_create_droid},

	{NULL, NULL}
};

static void pretty_print_lua_error(lua_State* L, const char *code, const char *funcname)
{
	const char *error = lua_tostring(L, -1);
	char *display_code = strdup(code);
	const char *ptr = error;
	int err_line = 0;
	int cur_line = 2;
	struct auto_string *erronous_code;

	erronous_code = alloc_autostr(16);

	//Find which line the error is on (if there is a line number in the error message)
	while (*ptr != 0 && *ptr != ':') {
		ptr++;
	}
	if (*ptr != 0) {
		// Line number found
		ptr++;
		err_line = strtol(ptr, NULL, 10);
	}

	//Break up lua code by newlines then insert line numbers & error notification
	ptr = strtok(display_code,"\n");

	while (ptr != NULL) {
		if (err_line != cur_line) {
			autostr_append(erronous_code, "%d %s\n", cur_line, ptr);
#ifndef __WIN32__
		} else if (!strcmp(getenv("TERM"), "xterm")) { //color highlighting for Linux/Unix terminals
			autostr_append(erronous_code, "\033[41m>%d %s\033[0m\n", cur_line, ptr);
#endif
		} else {
			autostr_append(erronous_code, ">%d %s\n", cur_line, ptr);
		}

		ptr = strtok(NULL, "\n");
		cur_line++;
	}

	fflush(stdout);
	ErrorMessage(funcname, "Error running Lua code: %s.\nErroneous LuaCode={\n%s}",
			 PLEASE_INFORM, IS_WARNING_ONLY, error, erronous_code->value);

	free(display_code);
	free_autostr(erronous_code);
}

lua_State *load_lua_coroutine(enum lua_target target, const char *code)
{
	lua_State *L = get_lua_state(target);
	lua_State *co_L = lua_newthread(L);

	if (luaL_loadstring(co_L, code)) {
		pretty_print_lua_error(co_L, code, __FUNCTION__);
		return NULL;
	}

	return co_L;
}

int resume_lua_coroutine(lua_State* L)
{
	int rtn = lua_resume(L, 0);

	switch (rtn) {
		case 0:
			// The lua script has ended
			return TRUE;
		case LUA_YIELD:
			// The lua script is 'pausing'
			return FALSE;
		default:
		{
			// Any other return code is an error.
			// Use the lua debug API to get the code of the current script
			lua_Debug ar;
			lua_getstack(L, 0, &ar);
			lua_getinfo(L, "S", &ar);
			pretty_print_lua_error(L, ar.source, __FUNCTION__);
			return TRUE; // let's pretend the lua script has ended
		}
	}

	return TRUE;
}

void run_lua(enum lua_target target, const char *code)
{
	lua_State *L = get_lua_state(target);

	if (luaL_dostring(L, code))
		pretty_print_lua_error(L, code, __FUNCTION__);
}

void run_lua_file(enum lua_target target, const char *path)
{
	lua_State *L = get_lua_state(target);

	if (luaL_dofile(L, path)) {
		ErrorMessage(__FUNCTION__, "Cannot run script file %s: %s.\n",
		         PLEASE_INFORM, IS_FATAL, path, lua_tostring(L, -1));
	}
}

void init_lua()
{
	char fpath[2048];
	int i;

	dialog_lua_state = lua_open();
	luaL_openlibs(dialog_lua_state);
	config_lua_state = lua_open();
	luaL_openlibs(config_lua_state);

	for (i = 0; lfuncs[i].name != NULL; i++) {
		lua_pushcfunction(dialog_lua_state, lfuncs[i].func);
		lua_setglobal(dialog_lua_state, lfuncs[i].name);
	}

	if (!find_file("script_helpers.lua", MAP_DIR, fpath, 1)) {
		run_lua_file(LUA_DIALOG, fpath);
		run_lua_file(LUA_CONFIG, fpath);
	}
}

/**
 * Reset Lua state
 */
void reset_lua_state(void)
{
	int i;
	char fpath[2048];

	lua_close(dialog_lua_state);
	dialog_lua_state = lua_open();
	luaL_openlibs(dialog_lua_state);

	for (i = 0; lfuncs[i].name != NULL; i++) {
		lua_pushcfunction(dialog_lua_state, lfuncs[i].func);
		lua_setglobal(dialog_lua_state, lfuncs[i].name);
	}

	if (!find_file("script_helpers.lua", MAP_DIR, fpath, 1)) {
		run_lua_file(LUA_DIALOG, fpath);
	}
}

/**
 * Save Lua variables as lua code.
 * Variables prefixed with '_' are omitted because these are Lua predefined variables.
 */
void write_lua_variables(struct auto_string *savestruct_autostr)
{
	int boolean;
	const char *value;
	lua_State *L = get_lua_state(LUA_DIALOG);

	lua_pushnil(L);
	while (lua_next(L, LUA_GLOBALSINDEX) != 0) {
		int value_type = lua_type(L, -1);
		int key_type = lua_type(L, -2);

		if (key_type != LUA_TSTRING) {
			lua_pop(L, 1);
			continue;
		}

		const char *name = lua_tostring(L, -2);
		if (name[0] == '_') {
			lua_pop(L, 1);
			continue;
		}

		switch (value_type)
		{
			case LUA_TBOOLEAN:
				boolean = lua_toboolean(L, -1);
				autostr_append(savestruct_autostr, "_G[\"%s\"] = %s\n", name, boolean ? "true" : "false");
				break;
			case LUA_TSTRING:
				value = lua_tostring(L, -1);
				autostr_append(savestruct_autostr, "_G[\"%s\"] = \"%s\"\n", name, value);
				break;
			case LUA_TNUMBER:
				value = lua_tostring(L, -1);
				autostr_append(savestruct_autostr, "_G[\"%s\"] = %s\n", name, value);
				break;
			default:
				break;
		}

		lua_pop(L, 1);
	}

	autostr_append(savestruct_autostr, "\n");
}
