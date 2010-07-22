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
#include "savestruct.h"

#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_map.h"

#include "../lua/lua.h"
#include "../lua/lauxlib.h"
#include "../lua/lualib.h"

/* Our Lua state for event execution */
lua_State *global_lua_state;

static int lua_event_teleport(lua_State * L)
{
	const char *label = luaL_checkstring(L, 1);
	location TempLocation;
	ResolveMapLabelOnShip(label, &TempLocation);
	reset_visible_levels();
	Teleport(TempLocation.level, TempLocation.x + 0.5, TempLocation.y + 0.5, TRUE);
	clear_active_bullets();
	return 0;
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
	append_new_game_message(msg);
	return 0;
}

static void event_modify_trigger(const char *name, int state)
{
	event_trigger *target_event = NULL;
	int i;
	for (i = 0; i < MAX_EVENT_TRIGGERS; i++) {
		if (!strcmp(AllEventTriggers[i].name, name))
			target_event = &AllEventTriggers[i];
	}

	target_event->enabled = state;
}

static int lua_event_enable_trigger(lua_State * L)
{
	const char *name = luaL_checkstring(L, 1);
	event_modify_trigger(name, 1);
	return 0;
}

static int lua_event_disable_trigger(lua_State * L)
{
	const char *name = luaL_checkstring(L, 1);
	event_modify_trigger(name, 0);
	return 0;
}

static int event_change_obstacle_type(const char *obslabel, int type)
{
	int obstacle_level_num;
	obstacle *our_obstacle = give_pointer_to_obstacle_with_label(obslabel, &obstacle_level_num);
	level *obstacle_level = curShip.AllLevels[obstacle_level_num];

	if (type != -1) {
		our_obstacle->type = type;
	} else {
		action_remove_obstacle(obstacle_level, our_obstacle);
	}

	// Now we make sure the door lists and that are all updated...
	dirty_animated_obstacle_lists(obstacle_level->levelnum);

	return 0;
}

static int lua_event_change_obstacle(lua_State * L)
{
	const char *obslabel = luaL_checkstring(L, 1);
	int type = luaL_checkinteger(L, 2);
	event_change_obstacle_type(obslabel, type);
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
	event_change_obstacle_type(obslabel, -1);
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
	Me.energy -= hp;
	return 0;
}

static int lua_event_get_tux_hp(lua_State * L)
{
	lua_pushinteger(L, (int)Me.energy);
	return 1;
}

static int lua_event_heat_tux(lua_State * L)
{
	int temp = luaL_checkinteger(L, 1);
	Me.temperature += temp;
	return 0;
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
	Me.base_skill_level[get_program_index_with_name(pname)]--;
	if (Me.base_skill_level[get_program_index_with_name(pname)] < 0)
		Me.base_skill_level[get_program_index_with_name(pname)] = 0;
	return 0;
}

static int lua_event_get_program_revision(lua_State * L)
{
	const char *pname = luaL_checkstring(L, 1);
	lua_pushinteger(L, Me.base_skill_level[get_program_index_with_name(pname)]);
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
	int mult = luaL_checkinteger(L, 2);

	item NewItem;
	NewItem = create_item_with_name(itemname, TRUE, mult);

	// Either we put the new item directly into inventory or we issue a warning
	// that there is no room and then drop the item to the floor directly under 
	// the current Tux position.  That can't fail, right?
	//
	if (!give_item(&NewItem)) {
		SetNewBigScreenMessage(_("1 Item received (on floor)"));
	} else {
		SetNewBigScreenMessage(_("1 Item received!"));
	}
	return 0;
}

static int lua_event_sell_item(lua_State *L)
{
	const char *itemname = luaL_checkstring(L, 1);
	int weight = luaL_optint(L, 2, 1);
	const char *charname = luaL_optstring(L, 3, chat_control_chat_droid->dialog_section_name);

	npc_add_shoplist(charname, itemname, weight);

	return 0;
}

static int lua_event_has_item_backpack(lua_State * L)
{
	const char *itemname = luaL_checkstring(L, 1);

	lua_pushinteger(L, CountItemtypeInInventory(GetItemIndexByName(itemname)));

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


static int lua_event_add_diary_entry(lua_State * L)
{
	const char *mis_name = luaL_checkstring(L, 1);
	const char *text = luaL_checkstring(L, 2);

	quest_browser_diary_add(mis_name, text);
	return 0;
}

static int lua_event_plant_cookie(lua_State * L)
{
	const char *cookie = luaL_checkstring(L, 1);
	PlantCookie(cookie);
	return 0;
}

static int lua_event_cookie_planted(lua_State * L)
{
	const char *cookie = luaL_checkstring(L, 1);
	int i;
	int cond = 0;
	for (i = 0; i < MAX_COOKIES; i++) {
		if (!Me.cookie_list[i])
			break;
		
		if (!strcmp(Me.cookie_list[i], cookie)) {
			cond = 1;
			break;
		}
	}

	lua_pushboolean(L, cond);
	return 1;
}

static int lua_event_remove_cookie(lua_State * L)
{
	const char *cookie = luaL_checkstring(L, 1);
	DeleteCookie(cookie);
	return 0;
}

static int lua_event_assign_mission(lua_State * L)
{
	const char *misname = luaL_checkstring(L, 1);
	const char *diarytext = luaL_optstring(L, 2, NULL);

	AssignMission(misname);
	if (diarytext != NULL)
		quest_browser_diary_add(misname, diarytext);

	return 0;
}

static int lua_event_complete_mission(lua_State * L)
{
	const char *misname = luaL_checkstring(L, 1);
	const char *diarytext = luaL_optstring(L, 2, NULL);

	CompleteMission(misname);
	if (diarytext != NULL)
		quest_browser_diary_add(misname, diarytext);

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
	int xp = luaL_checkinteger(L, 1);
	char tmpstr[150];
	Me.Experience += xp;
	sprintf(tmpstr, _("+%d Experience Points"), xp);
	SetNewBigScreenMessage(tmpstr);
	return 0;
}

static int lua_event_eat_training_points(lua_State * L)
{
	int nb = luaL_checkinteger(L, 1);
	char tmpstr[150];
	Me.points_to_distribute -= nb;
	sprintf(tmpstr, _("%d training point(s) spent!"), nb);
	SetNewBigScreenMessage(tmpstr);
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
		statptr = &Me.base_magic;
	} else if (!strcmp(characteristic, "vitality")) {
		statptr = &Me.base_vitality;
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
	chat_control_chat_droid->energy = Druidmap[chat_control_chat_droid->type].maxenergy;
	return 0;
}

static int lua_display_npc_damage_amount(lua_State * L)
{
	lua_pushinteger(L, (int)(Druidmap[chat_control_chat_droid->type].maxenergy - chat_control_chat_droid->energy)); 
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
	int duration = luaL_checkinteger(L, 1);
	chat_control_chat_droid->paralysation_duration_left = duration;
	Me.paralyze_duration = duration;
	return 0;
}

static int lua_chat_player_name(lua_State * L)
{
	lua_pushstring(L, Me.character_name);
	return 1;
}

static int lua_chat_tux_says(lua_State * L)
{
	const char *answer = luaL_checkstring(L, 1);
	const char *sample = luaL_optstring(L, 2, "Sorry_No_Voice_Sample_Yet_0.wav");

	autostr_append(chat_log.text, "\1- ");
	GiveSubtitleNSample(L_(answer), sample, chat_control_chat_droid, TRUE);
	autostr_append(chat_log.text, "\n");
	return 0;
}

static int lua_chat_npc_says(lua_State * L)
{
	const char *answer = luaL_checkstring(L, 1);
	const char *sample = luaL_optstring(L, 2, "Sorry_No_Voice_Sample_Yet_0.wav");

	autostr_append(chat_log.text, "\2");
	GiveSubtitleNSample(L_(answer), sample, chat_control_chat_droid, TRUE);
	autostr_append(chat_log.text, "\n");
	return 0;
}

static int lua_chat_cli_says(lua_State * L)
{
	const char *answer = luaL_checkstring(L, 1);
	const char *sample = luaL_optstring(L, 2, "Sorry_No_Voice_Sample_Yet_0.wav");

	autostr_append(chat_log.text, "\3");
	GiveSubtitleNSample(L_(answer), sample, chat_control_chat_droid, TRUE);

	return 0;
}


static int lua_chat_run_subdialog(lua_State * L)
{
	const char *tmp_filename = luaL_checkstring(L, 1);

	run_subdialog(tmp_filename);

	return 0;
}

static int lua_chat_set_next_node(lua_State * L)
{
	int nodenb = luaL_checkint(L, 1);

	if (!ChatRoster[nodenb].exists) {
		ErrorMessage(__FUNCTION__, "A dialog tried to run node %d that does not exist.\n", PLEASE_INFORM, IS_WARNING_ONLY, nodenb);
		return 0;
	}
	chat_control_next_node = nodenb;

	return 0;
}

static int lua_chat_end_dialog(lua_State * L)
{
	chat_control_end_dialog = 1;
	return 0;
}

static int lua_chat_partner_started(lua_State * L)
{
	lua_pushboolean(L, chat_control_partner_started);

	return 1;
}

static int __lua_chat_toggle_node(lua_State * L, int value)
{
	int i = 1, flag;
	while ((flag = luaL_optinteger(L, i, -1)) != -1) {
		i++;
		// for each optional node
		if (!ChatRoster[flag].exists) {
			ErrorMessage(__FUNCTION__, "A dialog tried to %s chat node %d that does not exist.\n", PLEASE_INFORM,
				     IS_WARNING_ONLY, value == 1 ? "enable" : "disable", flag);
			continue;
		}
		chat_control_chat_flags[flag] = value;
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
	hit_enemy(chat_control_chat_droid, chat_control_chat_droid->energy + 1, 0, Druidmap[chat_control_chat_droid->type].is_human - 2, 0);
	chat_control_end_dialog = 1;
	return 0;
}

static int lua_chat_set_bot_state(lua_State * L)
{
	const char *cmd = luaL_checkstring(L, 1);
	if (!strcmp(cmd, "follow_tux")) {
		chat_control_chat_droid->follow_tux = TRUE;
		chat_control_chat_droid->CompletelyFixed = FALSE;
	} else if (!strcmp(cmd, "fixed")) {
		chat_control_chat_droid->follow_tux = FALSE;
		chat_control_chat_droid->CompletelyFixed = TRUE;
	} else if (!strcmp(cmd, "free")) {
		chat_control_chat_droid->follow_tux = FALSE;
		chat_control_chat_droid->CompletelyFixed = FALSE;
	} else {
		ErrorMessage(__FUNCTION__,
			     "I was called with an invalid state namei %s. Accepted values are \"follow_tux\", \"fixed\" and \"free\".\n",
			     PLEASE_INFORM, IS_FATAL, cmd);
	}

	return 0;
}

static int lua_chat_takeover(lua_State * L)
{
	int opponent_capsules = luaL_checkinteger(L, 1);
	int player_capsules = 2 + Me.base_skill_level[get_program_index_with_name("Hacking")];
	int game_length = luaL_optint(L, 2, 100);

	int won = do_takeover(player_capsules, opponent_capsules, game_length);

	lua_pushboolean(L, won);

	return 1;
}

static int lua_chat_get_bot_type(lua_State * L)
{
	lua_pushstring(L, Druidmap[chat_control_chat_droid->type].druidname);
	return 1;
}

static int lua_chat_get_bot_name(lua_State * L)
{
	lua_pushstring(L, chat_control_chat_droid->short_description_text);
	return 1;
}

static int lua_chat_set_bot_name(lua_State * L)
{
	const char *bot_name = luaL_checkstring(L, 1);
	free(chat_control_chat_droid->short_description_text);
	chat_control_chat_droid->short_description_text = strdup(bot_name);
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

	chat_control_chat_droid->faction = get_faction_id(fact);
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
		ErrorMessage(__FUNCTION__, "Unknown faction state %s.", PLEASE_INFORM, IS_WARNING_ONLY);
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

/**
 * \brief Gets the value of a field of a Lua table.
 * \param L Lua state.
 * \param index Stack index where the table is.
 * \param field Name of the field to fetch.
 * \param type Either LUA_TNUMBER or LUA_TSTRING.
 * \param result Return location for an int or a newly allocated string.
 * \return TRUE if the value was read, FALSE if it could not be read.
 */
static int get_value_from_table(lua_State *L, int index, const char *field, int type, void *result)
{
	lua_getfield(L, index, field);
	if (lua_type(L, -1) == type) {
		switch (type) {
		case LUA_TNUMBER:
			*((int *) result) = lua_tointeger(L, -1);
			break;
		case LUA_TSTRING:
			*((char **) result) = strdup(lua_tostring(L, -1));
			break;
		}
		lua_pop(L, 1);
		return TRUE;
	} else {
		lua_pop(L, 1);
		return FALSE;
	}
}

static int lua_register_addon(lua_State *L)
{
	char *name = NULL;
	struct addon_bonus bonus;
	struct addon_material material;
	struct addon_spec addonspec;

	// Read the item name and find the item index.
	memset(&addonspec, 0, sizeof(struct addon_spec));
	get_value_from_table(L, 1, "name", LUA_TSTRING, &name);
	addonspec.type = GetItemIndexByName(name);
	free(name);

	// Read the simple add-on specific fields.
	get_value_from_table(L, 1, "require_socket", LUA_TSTRING, &addonspec.requires_socket);
	get_value_from_table(L, 1, "require_item", LUA_TSTRING, &addonspec.requires_item);
	get_value_from_table(L, 1, "upgrade_cost", LUA_TNUMBER, &addonspec.upgrade_cost);

	// Process the table of bonuses. The keys of the table are the names
	// of the bonuses and the values the attribute increase amounts.
	lua_getfield(L, 1, "bonuses");
	if (lua_type(L, -1) == LUA_TTABLE) {
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TNUMBER) {
				bonus.name = strdup(lua_tostring(L, -2));
				bonus.value = lua_tonumber(L, -1);
				dynarray_add(&addonspec.bonuses, &bonus, sizeof(bonus));
				lua_pop(L, 1);
			}
		}
	}
	lua_pop(L, 1);

	// Process the table of materials. The keys of the table are the names
	// of the materials and the values the required material counts.
	lua_getfield(L, 1, "materials");
	if (lua_type(L, -1) == LUA_TTABLE) {
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TNUMBER) {
				material.name = strdup(lua_tostring(L, -2));
				material.value = lua_tonumber(L, -1);
				dynarray_add(&addonspec.materials, &material, sizeof(material));
				lua_pop(L, 1);
			}
		}
	}
	lua_pop(L, 1);

	// Register a new add-on specification.
	add_addon_spec(&addonspec);

	return 0;
}

luaL_reg lfuncs[] = {
	/* teleport(string map_label) 
	 * Teleports the player to the given map label.
	 */
	{"teleport", lua_event_teleport}
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

	/* kill_tux()
	 * heal_tux()
	 * hurt_tux(int how_many_hp_to_remove)
	 *
	 * kill_tux kills Tux, heal_tux completely heals Tux,
	 * hurt_tux removes the given number of health points. This number
	 * can obviously be negative.
	 * heat_tux increases temperature. Number can be negative as well.
	 */
	{"kill_tux", lua_event_kill_tux}
	,
	{"heal_tux", lua_event_heal_tux}
	,
	{"hurt_tux", lua_event_hurt_tux}
	,
	{"heat_tux", lua_event_heat_tux}
	,
	{"get_tux_hp", lua_event_get_tux_hp}
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
	 * has_item_backpack(string item_name)
	 *
	 * Deletes or gives the given number of items.
	 * has_time returns the number of items of the given name currently in the inventory.
	 */
	{"del_item_backpack", lua_event_delete_item}
	,
	{"add_item", lua_event_give_item}
	,
	{"has_item_backpack", lua_event_has_item_backpack}
	,
	{"equip_item", lua_event_equip_item}
	,
	{"sell_item", lua_event_sell_item}
	,
	{"add_diary_entry", lua_event_add_diary_entry}
	,
	{"add_cookie", lua_event_plant_cookie}
	,
	{"has_cookie", lua_event_cookie_planted}
	,
	{"del_cookie", lua_event_remove_cookie}
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
	{"tux_says", lua_chat_tux_says}
	,
	/*use npc_says() rather than chat_npc_says() */
	{"chat_npc_says", lua_chat_npc_says}
	,
	{"cli_says", lua_chat_cli_says}
	,
	{"run_subdialog", lua_chat_run_subdialog}
	,
	{"set_next_node", lua_chat_set_next_node}
	,
	{"end_dialog", lua_chat_end_dialog}
	,
	{"partner_started", lua_chat_partner_started}
	,
	{"enable_node", lua_chat_enable_node}
	,
	{"disable_node", lua_chat_disable_node}
	,

	{"drop_dead", lua_chat_drop_dead}
	,
	{"set_bot_state", lua_chat_set_bot_state}
	,

	{"takeover", lua_chat_takeover}
	,
	{"heal_npc", lua_event_heal_npc}
	,
	{"npc_damage_amount", lua_display_npc_damage_amount}
	,
	{"freeze_tux_npc", lua_event_freeze_tux_npc}
	,
	{"npc_dead", lua_event_npc_dead},
	/* bot_type() tells you what model
	   bot_name() tells you what name it displays
	   set_bot_name() puts a new name in
	*/	
	{"bot_type", lua_chat_get_bot_type}, 

	{"bot_name", lua_chat_get_bot_name},

	{"set_bot_name", lua_chat_set_bot_name}, 

	{"difficulty_level", lua_difficulty_level},

	{"set_npc_faction", lua_set_npc_faction},
	{"set_faction_state", lua_set_faction_state},

	{"user_input_string", lua_user_input_string},

	{"create_droid", lua_create_droid},

	/* addon()
	 * Registers a new add-on specification.
	 */
	{"addon", lua_register_addon},
	{NULL, NULL}
	,
};

void run_lua(const char *code)
{
	if (luaL_dostring(global_lua_state, code)) {
		ErrorMessage(__FUNCTION__, "Error running Lua code {%s}: %s.\n", PLEASE_INFORM, IS_FATAL, code,
			     lua_tostring(global_lua_state, -1));
	}
}

void run_lua_file(const char *path)
{
	if (luaL_dofile(global_lua_state, path)) {
		ErrorMessage(__FUNCTION__, "Cannot run script file %s: %s.\n",
		         PLEASE_INFORM, IS_FATAL, path, lua_tostring(global_lua_state, -1));
	}
}

void init_lua()
{
	char fpath[2048];
	int i;

	global_lua_state = lua_open();
	luaL_openlibs(global_lua_state);

	for (i = 0; lfuncs[i].name != NULL; i++) {
		lua_pushcfunction(global_lua_state, lfuncs[i].func);
		lua_setglobal(global_lua_state, lfuncs[i].name);
	}

	if (!find_file("script_helpers.lua", MAP_DIR, fpath, 1)) {
		run_lua_file(fpath);
	}
}
