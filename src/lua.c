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
	gps teleport_pos = get_map_label_center(label);
	reset_visible_levels();
	Teleport(teleport_pos.z, teleport_pos.x, teleport_pos.y, TRUE, TRUE);
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
	gps teleport_pos = get_map_label_center(label);
	teleport_enemy(en, teleport_pos.z, teleport_pos.x, teleport_pos.y);
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
	play_sound_cached("effects/new_healing_sound.ogg");
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

	if (hp < 0)
		play_sound_cached("effects/new_healing_sound.ogg");

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

#ifndef WITH_NEW_DIALOG
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

static void fake_npc_dispose(struct chat_context *chat_context)
{
	free(chat_context->npc);
}

static int lua_chat_call_subdialog(lua_State * L)
{
	struct chat_context *current_chat_context, *new_chat_context;
	struct enemy *partner;
	struct npc *fake_npc;
	const char *dialog_name;
	int i;

	// A subdialog uses the same partner and npc than its caller dialog.
	// The caller dialog is the one on top of the chat_context stack.
	// However, the dialog flags (defining which dialog nodes are showed) of
	// the current npc cannot be used and more importantly they must not be
	// changed during the subdialog.
	// We thus create a fake npc which is a copy of the current npc, but with
	// reinitialized dialog flags. It also means that those dialog flags will
	// not be saved.

	current_chat_context = chat_get_current_context();
	partner = current_chat_context->partner;

	fake_npc = (struct npc *)MyMalloc(sizeof(struct npc));
	memcpy(fake_npc, current_chat_context->npc, sizeof(struct npc));
	for (i = 0; i<MAX_DIALOGUE_OPTIONS_IN_ROSTER; i++) {
		fake_npc->chat_flags[i] = 0;
	}
	fake_npc->chat_character_initialized = TRUE; // no init script in a subdialog

	dialog_name = luaL_checkstring(L, 1);

	new_chat_context = chat_create_context(partner, fake_npc, dialog_name);

	// Specific setting for subdialog :
	//  - do not reset the chat log window
	//  - delete the fake npc when the subdialog ends

	new_chat_context->display_log_markers = FALSE;
	new_chat_context->on_delete = fake_npc_dispose;

	// Push the chat context and yield the current dialog script, to let the
	// chat engine run the subdialog.

	chat_push_context(new_chat_context);
	return lua_yield(L, 0); // lua_yield must be called in a return statement
}
#endif

static int lua_start_chat(lua_State * L)
{
	int called_from_dialog;
	struct enemy *partner;
	struct npc *npc;
	struct chat_context *chat_context;
	char *dialog_name;

	// This function can be called from an event lua script or from a dialog
	// lua script.
	// In the first case, we have to open the chat screen and launch the chat
	// engine.
	// In the second case, a dialog is already running, so we have to interrupt
	// it (yield the lua coroutine) to let the chat engine run the new dialog.

	// To know if the function is called from a dialog script, we check if
	// there is already something on the chat context stack.
	called_from_dialog = (chat_get_current_context() != NULL);

	// Create a chat context and push it on the satck
	// Get the enemy to chat with from its name, get associated npc and
	// dialog, and create a chat context
	partner = get_enemy_arg(L, 1);
	npc = npc_get(partner->dialog_section_name);
	if (!npc)
		return 0;
	dialog_name = partner->dialog_section_name;

	chat_context = chat_create_context(partner, npc, dialog_name);
	if (!chat_push_context(chat_context)) {
		chat_delete_context(chat_context);
		return 0;
	}

	if (!called_from_dialog) {
		// Open the chat screen and run the chat engine.
		chat_run();
	} else {
		// Yield the current dialog script, to let the chat engine run the
		// new dialog.
		return lua_yield(L, 0); // lua_yield must be called in a return statement
	}

	return 0;
}

#ifndef WITH_NEW_DIALOG
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
#endif

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

#ifndef WITH_NEW_DIALOG
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
		current_chat_context->npc->chat_flags[flag] = value;
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
#endif

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
	gps dest_pos = get_map_label_center(label);
	struct level *lvl = curShip.AllLevels[dest_pos.z];
	int destinationwaypoint = get_waypoint(lvl, dest_pos.x, dest_pos.y);

	if (dest_pos.z !=  en->pos.z) {
		ErrorMessage(__FUNCTION__, "\
				Sending bot %s to map label %s (found on level %d) cannot be done because the bot\n\
				is not on the same level (z = %d). Doing nothing.",
				PLEASE_INFORM, IS_WARNING_ONLY, en->dialog_section_name, label, dest_pos.z, en->pos.z);
		return 0;
	}

	if (destinationwaypoint == -1) {
		ErrorMessage(__FUNCTION__, "\
				Map label %s (found on level %d) does not have a waypoint. Cannot send bot %s\n\
				to this location. Doing nothing.\n\
				GPS center coordinates x=%f, y=%f.",
				PLEASE_INFORM, IS_WARNING_ONLY, label, dest_pos.z, en->dialog_section_name, dest_pos.x, dest_pos.y);
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
	gps pos = get_map_label_center(map_label);
	int type;

	type = get_droid_type(type_name);

	enemy *en = enemy_new(type);
	enemy_reset(en);
	en->pos.x = pos.x;
	en->pos.y = pos.y;
	en->pos.z = pos.z;
	en->faction = get_faction_id(fact_name);
	en->dialog_section_name = strdup(dialog);
	enemy_insert_into_lists(en, TRUE);

	return 0;
}

static int lua_get_game_time(lua_State *L)
{
	lua_pushinteger(L, (int)(Me.current_game_date));

	return 1;
}

static int lua_play_sound(lua_State *L)
{
	const char *filename = luaL_checkstring(L, 1);

	play_sound_cached(filename);
	return 0;
}

static int lua_event_freeze_tux(lua_State * L)
{
	float duration = luaL_checknumber(L, 1);
	Me.paralyze_duration = duration;
	return 0;
}

static int lua_event_freeze_npc(lua_State * L)
{
	float duration = luaL_checknumber(L, 1);
	enemy *en = get_enemy_arg(L, 2);
	en->paralysation_duration_left = duration;
	return 0;
}

static int lua_add_obstacle(lua_State *L)
{
	int levelnum = luaL_checkinteger(L, 1);
	struct level *level = curShip.AllLevels[levelnum];
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	int type = luaL_checknumber(L, 4);

	add_obstacle(level, x, y, type);

	return 0;
}

static int lua_meters_traveled(lua_State *L)
{
	lua_pushinteger(L, (float)Me.meters_traveled);
	return 1;
}

static int lua_run_from_dialog(lua_State *L)
{
	lua_pushboolean(L, (chat_get_current_context() != NULL));
	return 1;
}

static int lua_running_benchmark(lua_State *L)
{
	lua_pushboolean(L, (do_benchmark) != NULL);
	return 1;
}

static int lua_reprogramm_bots_after_takeover(lua_State *L)
{
int rvat = luaL_checkinteger(L, 1);
GameConfig.talk_to_bots_after_takeover = rvat;

return 0;
}

static int lua_switch_background_music_to(lua_State *L)
{
	char *filename = (char *)luaL_checkstring(L, 1);
	SwitchBackgroundMusicTo(filename);
	return 0;
}

static int lua_exit_game(lua_State *L)
{
	int exit_status = luaL_checkinteger(L, 1);

	if (exit_status == 1) {
		Terminate(EXIT_FAILURE, TRUE);
		return 0;
	} else {
		Terminate(EXIT_SUCCESS, TRUE);
		return 0;
	}
}

static int lua_find_file(lua_State *L)
{
	char fpath[2048];
	const char *filename = (char *)luaL_checkstring(L, 1);
	const char *subdir   = (char *)luaL_checkstring(L, 2);
	if (find_file(filename, subdir, fpath, 0)) {
		lua_pushnil(L); /* return nil on error */
		return 1;
	}
	lua_pushstring(L, fpath);
	return 1;
}

static int lua_term_has_color_cap(lua_State *L)
{
	lua_pushboolean(L, (term_has_color_cap == TRUE));
	return 1;
}

static int lua_dir(lua_State *L)
{
	/* Note: Code taken (and adapted) from "Programming in Lua, 2nd edition" */

	char dirpath[2048];
	DIR *dir = NULL;
	struct dirent *entry = NULL;
	int i;
	const char *subdir = luaL_checkstring(L, 1);

	/* find full path */
	if (find_subdir(subdir, dirpath)) {
		lua_pushnil(L); /* return nil on error */
		return 1;
	}

	/* open directory */
	/* No need to check for errors since the same opendir() call was done in find_subdir */
	dir = opendir(dirpath);

	/* create the returned result table */
	lua_newtable(L);
	i = 1;
	while ((entry = readdir(dir)) != NULL) {
		lua_pushnumber(L, i++); /* push key */
		lua_pushstring(L, entry->d_name); /* push value */
		lua_settable(L, -3);
	}

	closedir(dir);
	return 1;
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
#ifndef WITH_NEW_DIALOG
	{"topic", lua_chat_push_topic}
	,
	{"pop_topic", lua_chat_pop_topic}
	,
	{"call_subdialog", lua_chat_call_subdialog}
	,
#endif
	{"start_chat", lua_start_chat}
	,
#ifndef WITH_NEW_DIALOG
	{"next", lua_chat_set_next_node}
	,
#endif
	{"end_dialog", lua_chat_end_dialog}
	,
	/* NOTE:  if (partner_started())  will always be true
	 * if rush_tux is 1
	 */
	{"partner_started", lua_chat_partner_started}
	,
#ifndef WITH_NEW_DIALOG
	{"show", lua_chat_enable_node}
	,
	{"hide", lua_chat_disable_node}
	,
#endif
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

	{"game_time", lua_get_game_time},
	/* play_sound("file")
	 * path has to originate from /sound , e.g.
	 * play_sound("effects/No_Ammo_Sound_0.ogg")
	 */
	{"play_sound", lua_play_sound},
	// freeze_tux() freezes tux for the given amount of seconds
	{"freeze_tux", lua_event_freeze_tux},
	// freeze_npc() freezes the npc for the given amount of seconds
	{"freeze_npc", lua_event_freeze_npc},
	/* add_obstacle(lvl, x, y, obst_ID) add obstacles to maps at given position
	 * add_obstacle(8, 41.4, 51.5, 100)
	 * where 8 is the level number, x and y are the coorinates and 100
	 * is the obstacle ID (see defs.h)
	 */
	{"add_obstacle", lua_add_obstacle},
	// meters_traveled() returns ingame meters tux has traveled
	{"meters_traveled", lua_meters_traveled},
	// if (run_from_dialog()) then
	// to check if certain code was run from inside a dialog or not
	{"run_from_dialog", lua_run_from_dialog},
	// returns if we are running a benchmark e.g. the dialog validator
	// or not
	// USE WITH CARE
	{"running_benchmark", lua_running_benchmark},
	/* switch_background_music("file")
	 * path has to originate from /sound/music , e.g.
	 * play_sound("menu.ogg")
	 */
	{"switch_background_music", lua_switch_background_music_to},
	// 1 = true,  0 = false
	{"reprogramm_bots_after_takeover", lua_reprogramm_bots_after_takeover},

	{"exit_game", lua_exit_game},

	{"find_file", lua_find_file},
	{"dir", lua_dir},

	{"term_has_color_cap", lua_term_has_color_cap },

	{NULL, NULL}
};

/*
 * Prepare a lua function call by pushing on the Lua stack the name of
 * the function to call and the arguments of the call (see comment of call_lua_func()).
 */
static int push_func_and_args(lua_State *L, const char *module, const char *func, const char *sig, va_list *vl)
{
	 /* push function */
	if (module) {
		lua_getglobal(L, module);
		lua_getfield(L, -1, func);
		lua_remove(L, -2);
	} else {
		lua_getglobal(L, func);
	}

	while (sig && *sig) { /* repeat for each argument */
		switch (*sig++) {
		case 'f': /* double argument */
			lua_pushnumber(L, va_arg(*vl, double));
			break;
		case 'd': /* int argument */
			lua_pushinteger(L, va_arg(*vl, int));
			break;
		case 's': /* string argument */
			lua_pushstring(L, va_arg(*vl, char *));
			break;
		case 'S': /* dynarray of strings : push a table containing the strings */
			{
				int i;
				struct dynarray *array = va_arg(*vl, struct dynarray *);
				lua_createtable(L, array->size, 0);
				for (i=0; i<array->size; i++)
				{
					char *text = ((char **)(array->arr))[i];
					lua_pushstring(L, text);
					lua_rawseti(L, -2, i+1); // table[i+1] = text
				}
			}
			break;
		default:
			DebugPrintf(-1, "call_lua_func: invalid input option (%c).", *(sig - 1));
			return 0;
		}
	}

	return 1;
}

/*
 * Retrieve the returned values of a lua function call.
 * The results are stored in the locations pointed to by the pointer arguments
 * that follow the signature string (see comment of call_lua_func()).
 */
static int pop_results(lua_State *L, const char *sig, va_list *vl)
{
	if (!sig)
		return 1;

	int nres = strlen(sig);
	int index;
	int rc = 0;

	for (index = -nres; *sig; index++) { /* repeat for each result */
		int ltype = lua_type(L, index);
		switch (*sig++) {
		case 'f': /* double result */
			if (ltype != LUA_TNUMBER) {
				DebugPrintf(-1, "call_lua_func: wrong result type for #%d returned value (number expected)", index);
				goto pop_and_return;
			}
			*va_arg(*vl, double *) = lua_tonumber(L, index);
			break;
		case 'd': /* int result */
			if (ltype != LUA_TNUMBER) {
				DebugPrintf(-1, "call_lua_func: wrong result type for #%d returned value (number expected)", index);
				goto pop_and_return;
			}
			*va_arg(*vl, int *) = lua_tointeger(L, index);
			break;
		case 's': /* string result */
			if (ltype != LUA_TSTRING) {
				DebugPrintf(-1, "call_lua_func: wrong result type for #%d returned value (string expected)", index);
				goto pop_and_return;
			}
			*va_arg(*vl, const char **) = strdup(lua_tostring(L, index));
			break;
		case 'S': /* dynarray of strings result */
			{
				int i;
				if (ltype != LUA_TTABLE) {
					DebugPrintf(-1, "call_lua_func: wrong result type for #%d returned value (table expected)", index);
					goto pop_and_return;
				}
				struct dynarray *array = va_arg(*vl, struct dynarray *);
				dynarray_init(array, lua_rawlen(L, index), sizeof(char *)); // the dynarray was reseted by the caller
				for (i=1; i<=lua_rawlen(L, index); i++) {
					lua_rawgeti(L, index, i);
					if (!lua_isstring(L, -1)) {
						DebugPrintf(-1, "call_lua_func: wrong result type for #%d:%d returned value (string expected).\nSkeeping that value", index, i);
					} else {
						char *nodename = strdup(lua_tostring(L, -1));
						dynarray_add(array, &nodename, sizeof(string));
					}
					lua_pop(L, 1);
				}
			}
			break;
		default:
			DebugPrintf(-1, "call_lua_func: invalid output option (%c)", *(sig - 1));
			goto pop_and_return;
		}
	}

	rc = 1;

pop_and_return:
	lua_pop(L, nres);
	return rc;
}

/**
 * \brief Helper function to call a Lua function.
 *
 * \details Call a Lua function passing parameters and retrieving results through the
 * Lua stack.
 * (inspired from a code found in "Programming in Lua, 2ed").
 *
 * Usage:
 *
 * To execute a Lua function call such as "var1, var2 = my_module.my_func(param1)",
 * use "call_lua_func(lua_target, "my_module", "my_func", insig, outsig, param1, &var1, var2)"
 *
 * C being a strong typed language, the type of the input parameters and of the
 * returned values has to be defined, through 'insig' (for input parameters)
 * and 'outsig' (for returned values). A type is defined by a single specific
 * character, and 'insig' (resp. 'outsig') is a string containing a list of
 * those characters, one character per parameter (resp. returned value).
 *
 * Known types are:
 *   'f' for double type
 *   'd' for int type
 *   's' for string type (i.e. char*)
 *   'S' for dynarray of strings
 *
 * Following 'insig' and 'outsig' is a list of data to be used as parameters for
 * the Lua function (one data per character in 'insig'), followed by a list
 * of pointers to store the returned values (one pointer per character in 'outsig').
 * Note 1: memory of string returned values is allocated by call_lua_func().
 * Note 2: memory of string dynarray slots is allocated by call_lua_func(),
 *         and the dynarray has to be freed before the call.
 *
 * If the Lua function is not inside a module, set 'module' to NULL.
 * If the Lua function has no parameters, set 'insig' to NULL.
 * If the Lua does not return values, set 'outsig' to NULL.
 *
 * Exemples (of doubtful utility...):
 * double sine;
 * call_lua_func(LUA_DIALOG, "math", "sin", "f", "f", 3.14, &sine);
 * char *tmpname;
 * call_lua_func(LUA_DIALOG, "os", "tmpname", NULL, "s", tmpname);
 *
 * \param target A lua_target enum value defining the Lua context to use
 * \param module Name of the module containing the Lua function, or NULL if none
 * \param func   Name of the Lua function to call
 * \param insig  Signature string for the input parameters, or NULL
 * \param outsig Signature string for the returned values
 * \param ...    Input parameters data and pointers to returned value storages
 *
 * \return TRUE if the function call succeeded, else return FALSE
 */
int call_lua_func(enum lua_target target, const char *module, const char *func, const char *insig, const char *outsig, ...)
{
	int narg = (insig) ? strlen(insig) : 0;   /* number of arguments */
	int nres = (outsig) ? strlen(outsig) : 0; /* number of results */

	lua_State *L = get_lua_state(target);

	va_list vl;
	va_start(vl, outsig);

	/* do the call */
	if (!push_func_and_args(L, module, func, insig, &vl)) {
		ErrorMessage(__FUNCTION__, "Aborting lua function call.\n", NO_NEED_TO_INFORM, IS_WARNING_ONLY);
		va_end(vl);
		return FALSE;
	}

	if (lua_pcall(L, narg, nres, 0) != LUA_OK) {
		DebugPrintf(-1, "call_lua_func: Error calling ’%s’: %s", func, lua_tostring(L, -1));
		lua_pop(L, 1);
		ErrorMessage(__FUNCTION__, "Aborting lua function call.\n", NO_NEED_TO_INFORM, IS_WARNING_ONLY);
		va_end(vl);
		return FALSE;
	}

	if (!pop_results(L, outsig, &vl)) {
		ErrorMessage(__FUNCTION__, "Aborting lua function call.\n", NO_NEED_TO_INFORM, IS_WARNING_ONLY);
		va_end(vl);
		return FALSE;
	}

	va_end(vl);
	return TRUE;
}

static void pretty_print_lua_error(lua_State* L, const char *code, int cur_line, const char *funcname)
{
	const char *error = lua_tostring(L, -1);
	char *display_code = strdup(code);
	const char *ptr = error;
	int err_line = 0;
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
			autostr_append(erronous_code, "%d  %s\n", cur_line, ptr);
		} else if (term_has_color_cap) { //color highlighting for Linux/Unix terminals
			autostr_append(erronous_code, "\033[41m>%d %s\033[0m\n", cur_line, ptr);
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

/**
 * \brief Prepare to call a lua function from a module in a coroutine
 *
 * \details Create a new lua thread, and fill the lua stack with the function to
 * call and its argument, in preparation to a call to resume_coroutine, which will
 * actually start the coroutine.
 * (See call_lua_func() for an explanation of the parameters, given that there are
 * no returned values for coroutine)
 *
 * \param target A lua_target enum value defining the Lua context to use
 * \param module Name of the module containing the Lua function, or NULL if none
 * \param func   Name of the Lua function to call
 * \param insig  Signature string for the input parameters, or NULL
 * \param ...    Input parameters data
 * 
 * \return Pointer to a lua_coroutine struct holding the data needed to start (resume) the coroutine
 */
struct lua_coroutine *prepare_lua_coroutine(enum lua_target target, const char *module, const char *func, const char *insig, ...)
{
	lua_State *L = get_lua_state(target);
	lua_State *co_L = lua_newthread(L);

	struct lua_coroutine *new_coroutine = (struct lua_coroutine *)MyMalloc(sizeof(struct lua_coroutine));
	new_coroutine->thread = co_L;
	new_coroutine->nargs = (insig) ? strlen(insig) : 0;

	va_list vl;
	va_start(vl, insig);

	push_func_and_args(co_L, module, func, insig, &vl);

	va_end(vl);

	return new_coroutine;
}

/**
 * \brief Prepare to call a lua function, given in a source code, in a coroutine
 *
 * \details Create a new lua thread, and fill the lua stack with the function to
 * call and its argument, in preparation to a call to resume_coroutine, which will
 * actually start the coroutine.
 *
 * \param target A lua_target enum value defining the Lua context to use
 * \param code   The code of the function to execute
 *
 * \return Pointer to a lua_coroutine struct holding the data needed to start (resume) the coroutine
 */
struct lua_coroutine *load_lua_coroutine(enum lua_target target, const char *code)
{
	lua_State *L = get_lua_state(target);
	lua_State *co_L = lua_newthread(L);

	if (luaL_loadstring(co_L, code)) {
		pretty_print_lua_error(co_L, code, 2, __FUNCTION__);
		return NULL;
	}
	struct lua_coroutine *new_coroutine = (struct lua_coroutine *)MyMalloc(sizeof(struct lua_coroutine));
	new_coroutine->thread = co_L;
	new_coroutine->nargs = 0;

	return new_coroutine;
}

int resume_lua_coroutine(struct lua_coroutine *coroutine)
{
	int rtn = lua_resume(coroutine->thread, NULL, coroutine->nargs);

	switch (rtn) {
		case 0:
			// The lua script has ended
			return TRUE;
		case LUA_YIELD:
			// The lua script is 'pausing'. Next resume will be without arguments
			coroutine->nargs = 0;
			return FALSE;
		default:
		{
			// Any other return code is an error.
			// Use the lua debug API to get informations about the code of the current script
			lua_Debug ar;
			lua_getstack(coroutine->thread, 0, &ar);
			lua_getinfo(coroutine->thread, "S", &ar);

			if (ar.source[0] != '@') {
				// ar.source contains the script code
				pretty_print_lua_error(coroutine->thread, ar.source, 2, __FUNCTION__);
			} else {
				// The script code is in an external file
				// Extract the erroneous function's code from the source file
				FILE *src = fopen(ar.short_src, "r");
				struct auto_string *code = alloc_autostr(256);
				char buffer[256] = "";
				char *ptr = buffer;
				int lc = 1;
				for (;;) {
					if (*ptr == '\0') {
						if (feof(src)) break;
						size_t nbc = fread(buffer, 1, 255, src);
						buffer[nbc] = '\0';
						ptr = buffer;
					}
					if (*ptr == '\n') lc++;
					if (lc > ar.lastlinedefined) break;
					if (lc >= ar.linedefined && lc <= ar.lastlinedefined) {
						// The use of autostr_append to add a single character is
						// not efficient, but this code is used only in case of a
						// script error, so we do not really care of efficiency
						autostr_append(code, "%c", *ptr);
					}
					ptr++;
				}

				pretty_print_lua_error(coroutine->thread, code->value, ar.linedefined, __FUNCTION__);
				free_autostr(code);
			}

			return TRUE; // Pretend the lua script has ended
		}
	}

	return TRUE;
}

void run_lua(enum lua_target target, const char *code)
{
	lua_State *L = get_lua_state(target);

	if (luaL_dostring(L, code))
		pretty_print_lua_error(L, code, 2, __FUNCTION__);
}

void run_lua_file(enum lua_target target, const char *path)
{
	lua_State *L = get_lua_state(target);

	if (luaL_dofile(L, path)) {
		ErrorMessage(__FUNCTION__, "Cannot run script file %s: %s.\n",
		         PLEASE_INFORM, IS_FATAL, path, lua_tostring(L, -1));
	}
}

/*
 * Load a lua module in a lua context.
 * The directory containing the module is added to the package.path lua global
 * variable.
 * The module is loaded by calling 'require(module)'.
 */
static void load_lua_module(enum lua_target target, const char *dir, const char *module)
{
	char *module_file;
	char fpath[2048];
	lua_State *L = get_lua_state(target);

	module_file = (char *)MyMalloc(strlen(module)+strlen(".lua")+1);
	strcpy(module_file, module);
	strcat(module_file, ".lua");

	/*
	 * Add the module's dir to the Lua package.path
	 */

	if (!find_file(module_file, dir, fpath, 1)) {

		// Keep the dirname of the file path and add the search pattern
		char *ptr = strstr(fpath, module_file);
		strcpy(ptr, "?.lua");

		// Get current Lua package.path
		lua_getglobal(L, "package");
		lua_getfield(L, 1, "path");
		const char *package_path = lua_tostring(L, -1);
		lua_pop(L, 2);

		// Add the search path, if needed
		if (!strstr(package_path, fpath)) {
			lua_getglobal(L, "package"); /* -> stack: package */
			lua_getfield(L, 1, "path");  /* -> stack: package.path < package */
			lua_pushliteral(L, ";");     /* -> stack: ";" < package.path < package */
			lua_pushstring(L, fpath);    /* -> stack: fpath < ";" < package.path < package */
			lua_concat(L, 3);            /* -> stack: package.path;fpath < package */
			lua_setfield(L, 1, "path");  /* package.path = package.path;fpath -> stack: package */
			lua_pop(L, 1);
		}
	}

	/*
	 * Call "require(module)" to load the module
	 */

	call_lua_func(target, NULL, "require", "s", NULL, module);

	free(module_file);
}

/**
 * Initialize the Lua state used to load the config files
 */
void init_lua()
{
	char fpath[2048];

	dialog_lua_state = NULL;

	config_lua_state = luaL_newstate();
	luaL_openlibs(config_lua_state);

	if (!find_file("script_helpers.lua", MAP_DIR, fpath, 1)) {
		run_lua_file(LUA_CONFIG, fpath);
	}
}

/**
 * Reset (or create) the Lua state used to load and execute the dialogs
 */
void reset_lua_state(void)
{
	int i;
	char fpath[2048];

	if (dialog_lua_state)
		lua_close(dialog_lua_state);
	dialog_lua_state = luaL_newstate();
	luaL_openlibs(dialog_lua_state);

	for (i = 0; lfuncs[i].name != NULL; i++) {
		lua_pushcfunction(dialog_lua_state, lfuncs[i].func);
		lua_setglobal(dialog_lua_state, lfuncs[i].name);
	}

#ifdef WITH_NEW_DIALOG
	load_lua_module(LUA_DIALOG, LUA_MOD_DIR, "FDutils");
#endif

	if (!find_file("script_helpers.lua", MAP_DIR, fpath, 1)) {
		run_lua_file(LUA_DIALOG, fpath);
	}

#ifdef WITH_NEW_DIALOG
	// Load and initialize lua part of the dialog engine
	load_lua_module(LUA_DIALOG, LUA_MOD_DIR, "FDdialog");
	call_lua_func(LUA_DIALOG, "FDdialog", "set_dialog_dir", "s", NULL, DIALOG_DIR);
#endif
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

	// Push global table on the stack
	lua_pushglobaltable(L);

	// Loop over the global table content
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
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

	// Pop global table from the stack
	lua_pop(L, 1);
}
