/*
 *
 *  Copyright (c) 2013 Samuel Degrande
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
 * \file luaFD_tux.c
 * \brief This file contains the definition of the Tux Lua binding.
 *
 */

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "luaFD.h"

/*
 * From a Lua script point of view, the Tux entity is entirely defined by
 * the 'tux' C struct.
 * The Lua binding holds a reference to that datastructure.
 */

struct luaFD_tux {
	struct tux* me;
};

/**
 * Called when Lua garbagecollects the Tux.
 */
static int _garbage_collect(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");
	// Nothing to do, currently
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
/// \defgroup FDtux FDtux: Lua Tux binding
/// \ingroup luaFD_bindings
///
/// Some doc on FDtux - TO BE WRITTEN
///
///@{
// start FDtux submodule
// Note: Functions comments are written to reflect their Lua usage.

/**
 * \brief Get the player's name
 *
 * \param self [\p FDtux] FDtux instance
 *
 * \return Name
 *
 * \bindtype cfun
 */
LUAFD_DOC(string get_player_name(self))

static int _get_player_name(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	if (self->me->character_name)
		lua_pushstring(L, self->me->character_name);
	else
		lua_pushstring(L, "");

	return 1;
}

/**
 * \brief Get Tux's current health
 *
 * \param self [\p FDtux] FDtux instance
 *
 * \return Health value
 *
 * \bindtype cfun
 */
LUAFD_DOC(integer get_hp(self))

static int _get_hp(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	lua_pushinteger(L, (int)self->me->energy);
	return 1;
}

/**
 * \brief Get Tux's current maximum health
 *
 * \param self [\p FDtux] FDtux instance
 *
 * \return Maximum Health value
 *
 * \bindtype cfun
 */
LUAFD_DOC(integer get_max_hp(self))

static int _get_max_hp(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	lua_pushinteger(L, (int)self->me->maxenergy);

	return 1;
}

/**
 * \brief Get Tux's current remaining heat absorbing capabilities
 *
 * The remaining heat absorbing capabilities is computed as 'max_temperature - current_temperature'
 *
 * \param self [\p FDtux] FDtux instance
 *
 * \return Cool value
 *
 * \bindtype cfun
 */
LUAFD_DOC(integer get_cool(self))

static int _get_cool(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	lua_pushinteger(L, (int)self->me->max_temperature - (int)self->me->temperature);

	return 1;
}

/**
 * \brief Get ingame meters that Tux has traveled
 *
 * \param self [\p FDtux] FDtux instance
 *
 * \return Meters traveled value
 *
 * \bindtype cfun
 */
LUAFD_DOC(integer get_meters_traveled(self))

static int _get_meters_traveled(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	lua_pushinteger(L, (float)self->me->meters_traveled);

	return 1;
}

/**
 * \brief Kills Tux
 *
 * Implemented by setting Tux's energy to a huge negative value
 *
 * \param self [\p FDtux] FDtux instance
 *
 * \bindtype cfun
 */
LUAFD_DOC(void kill(self))

static int _kill(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	self->me->energy = -100;

	return 0;
}

/**
 * \brief Completely heals Tux
 *
 * Set Tux's energy to the maxenergy value, and play the healing sound
 *
 * \param self [\p FDtux] FDtux instance
 *
 * \bindtype cfun
 */
LUAFD_DOC(void heal(self))

static int _heal(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	self->me->energy = self->me->maxenergy;
	play_sound("effects/new_healing_sound.ogg");

	return 0;
}

/**
 * \brief Change Tux's health points
 *
 * If the \e amount parameter is positive, the function removes those given number
 * of health points.\n
 * If the \e amount parameter is negative, the function adds those given number of
 * health points, and plays the healing sound.
 *
 * \param self   [\p FDtux]   FDtux instance
 * \param amount [\p integer] Number of health points to remove or to add.
 *
 * \bindtype cfun
 */
LUAFD_DOC(void hurt(self, amount))

static int _hurt(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	int amount = luaL_checkinteger(L, 2);

	if (amount < 0)
		play_sound("effects/new_healing_sound.ogg");

	hit_tux(amount);

	return 0;
}

/**
 * \brief Change Tux's heat
 *
 * Add \e amount to Tux's current temperature. \e amount can be negative, then
 * decreasing the temperature.
 *
 * \param self   [\p FDtux]   FDtux instance
 * \param amount [\p integer] Value to add to Tux's temperature
 *
 * \bindtype cfun
 */
LUAFD_DOC(void heat(self, amount))

static int _heat(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	int amount = luaL_checkinteger(L, 2);
	self->me->temperature += amount;

	return 0;
}

/**
 * \brief Freezes Tux for the given amount of seconds
 *
 * \param self     [\p FDtux]  FDtux instance
 * \param duration [\p number] Number of seconds to freeze Tux
 *
 * \bindtype cfun
 */
LUAFD_DOC(void freeze(self, duration))

static int _freeze(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	float duration = luaL_checknumber(L, 2);
	self->me->paralyze_duration = duration;
	return 0;
}

/**
 * \brief Add experience points to Tux
 *
 * \param self   [\p FDtux]   FDtux instance
 * \param points [\p integer] Number of experience points to add
 *
 * \bindtype cfun
 */
LUAFD_DOC(void add_xp(self, points))

static int _add_xp(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	int points = luaL_checkinteger(L, 2) * self->me->experience_factor;
	self->me->Experience += points;

	char tmpstr[150];
	sprintf(tmpstr, _("+%d Experience Points"), points);
	SetNewBigScreenMessage(tmpstr);

	return 0;
}

/**
 * \brief Add or remove gold to Tux
 *
 * If \e amount is positive, add that amount of gold.\n
 * If \e amount is negative, remove that amount of gold, unless the value
 * is bigger than what Tux has. In that case an error is generated.
 *
 * \param self   [\p FDtux]   FDtux instance
 * \param amount [\p integer] Amount of gold to add or remove
 *
 * \bindtype cfun
 */
LUAFD_DOC(void add_gold(self, amount))

static int _add_gold(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	int amount = luaL_checkinteger(L, 2);
	int new_gold = (int)(self->me->Gold) + amount;

	if (new_gold < 0) {
		return luaL_error(L, "%s() tried to remove %d gold from the player that only has %d!", __FUNCTION__, -amount, self->me->Gold);
	}

	self->me->Gold = (unsigned int)new_gold;

	char tmpstr[150];
	if (amount > 0)
		sprintf(tmpstr, _("Gained %d valuable circuits!"), amount);
	else
		sprintf(tmpstr, _("Lost %d valuable circuits!"), amount);
	SetNewBigScreenMessage(tmpstr);

	return 0;
}

/**
 * \brief Return the amount of Tux's gold
 *
 * \param self [\p FDtux] FDtux instance
 *
 * \return Amount of gold
 *
 * \bindtype cfun
 */
LUAFD_DOC(integer get_gold(self))

static int _get_gold(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	lua_pushinteger(L, self->me->Gold);

	return 1;
}

/**
 * \brief Remove the given amount of training points
 *
 * \param self   [\p FDtux]   FDtux instance
 * \param points [\p integer] Amount of training points to remove
 *
 * \bindtype cfun
 */
LUAFD_DOC(void del_training_points(self, points))

static int _del_training_points(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	int points = luaL_checkinteger(L, 2);
	self->me->points_to_distribute -= points;

	return 0;
}

/**
 * \brief Return the amount of current training points
 *
 * \param self [\p FDtux] FDtux instance
 *
 * \return Amount of training points
 *
 * \bindtype cfun
 */
LUAFD_DOC(integer get_training_points(self))

static int _get_training_points(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	lua_pushinteger(L, self->me->points_to_distribute);

	return 1;
}

/**
 * \brief Improve the given skill by one level
 *
 * \param self  [\p FDtux]  FDtux instance
 * \param skill [\p string] Name of the skill to improve. Allowed value: "melee", "ranged", "programming"
 *
 * \bindtype cfun
 */
LUAFD_DOC(void improve_skill(self, skill))

static int _improve_skill(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	const char *skill = luaL_checkstring(L, 2);
	int *skillptr = NULL;
	if (!strcmp(skill, "melee")) {
		skillptr = &(self->me->melee_weapon_skill);
		SetNewBigScreenMessage(_("Melee fighting ability improved!"));
	} else if (!strcmp(skill, "ranged")) {
		skillptr = &(self->me->ranged_weapon_skill);
		SetNewBigScreenMessage(_("Ranged combat ability improved!"));
	} else if (!strcmp(skill, "programming")) {
		skillptr = &(self->me->spellcasting_skill);
		SetNewBigScreenMessage(_("Programming ability improved!"));
	} else {
		return luaL_error(L, "%s() %s", __FUNCTION__, "with an incorrect parameter. Accepted values are \"melee\", \"ranged\", and \"programming\".");
	}

	if (skillptr) {
		ImproveSkill(skillptr);
	}

	return 0;
}

/**
 * \brief Return the given skill's level
 *
 * \param self  [\p FDtux]  FDtux instance
 * \param skill [\p string] Name of the skill. Allowed value: "melee", "ranged", "programming"
 *
 * \return Skill's level
 *
 * \bindtype cfun
 */
LUAFD_DOC(integer get_skill(self, skill))

static int _get_skill(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	const char *skill = luaL_checkstring(L, 2);
	int *skillptr = NULL;
	if (!strcmp(skill, "melee")) {
		skillptr = &(self->me->melee_weapon_skill);
	} else if (!strcmp(skill, "ranged")) {
		skillptr = &(self->me->ranged_weapon_skill);
	} else if (!strcmp(skill, "programming")) {
		skillptr = &(self->me->spellcasting_skill);
	} else {
		return luaL_error(L, "%s() %s", __FUNCTION__, "with an incorrect parameter. Accepted values are \"melee\", \"ranged\", and \"programming\".");
	}

	if (skillptr) {
		lua_pushinteger(L, *skillptr);
	} else
		lua_pushinteger(L, 0);

	return 1;
}

/**
 * \brief Upgrade the given program's by one level
 *
 * \param self    [\p FDtux]  FDtux instance
 * \param program [\p string] Name of the program.
 *
 * \bindtype cfun
 */
LUAFD_DOC(void improve_program(self, program))

static int _improve_program(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	const char *program = luaL_checkstring(L, 2);
	improve_program(get_program_index_with_name(program));

	return 0;
}

/**
 * \brief Downgrade the given program's by one level
 *
 * \param self    [\p FDtux]  FDtux instance
 * \param program [\p string] Name of the program.
 *
 * \bindtype cfun
 */
LUAFD_DOC(void downgrade_program(self, program))

static int _downgrade_program(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	const char *program = luaL_checkstring(L, 2);
	downgrade_program(get_program_index_with_name(program));

	return 0;
}

/**
 * \brief Return program's current revision level
 *
 * \param self    [\p FDtux]  FDtux instance
 * \param program [\p string] Name of the program.
 *
 * \return Program's level
 *
 * \bindtype cfun
 */
LUAFD_DOC(integer get_program_revision(self, program))

static int _get_program_revision(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	const char *program = luaL_checkstring(L, 2);
	lua_pushinteger(L, self->me->skill_level[get_program_index_with_name(program)]);

	return 1;
}

/**
 * \brief Add points to the given characteristic value
 *
 * \param self           [\p FDtux]  FDtux instance
 * \param characteristic [\p string] Name of the characteristic to change. Allowed value: "strength", "dexterity", "CPU", "vitality"
 *
 * \bindtype cfun
 */
LUAFD_DOC(void change_stat(self, characteristic))

static int _change_stat(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	const char *characteristic = luaL_checkstring(L, 2);
	int nb = luaL_checkinteger(L, 3);
	int *statptr = NULL;

	if (!strcmp(characteristic, "strength")) {
		statptr = &(self->me->base_strength);
	} else if (!strcmp(characteristic, "dexterity")) {
		statptr = &(self->me->base_dexterity);
	} else if (!strcmp(characteristic, "CPU")) {
		statptr = &(self->me->base_cooling);
	} else if (!strcmp(characteristic, "vitality")) {
		statptr = &(self->me->base_physique);
	} else {
		return luaL_error(L, "%s() was called with characteristic name %s - accepted values are \"strength\", \"dexterity\", \"CPU\", and \"vitality\".",
		                     __FUNCTION__, characteristic);
	}

	*statptr += nb;
	return 0;
}

/**
 * \brief Assign a quest to the player, and fill the diary
 *
 * \param self  [\p FDtux]  FDtux instance
 * \param quest [\p string] Name of the quest to assign
 * \param text  [\p string] Text to add to the diary [optional]
 *
 * \bindtype cfun
 */
LUAFD_DOC(void assign_quest(self, quest, text))

static int _assign_quest(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	const char *quest = luaL_checkstring(L, 2);
	const char *text = luaL_optstring(L, 3, NULL);

	AssignMission(quest);
	if (text != NULL)
		mission_diary_add(quest, text);

	return 0;
}

/**
 * \brief Check if the given quest was assigned to the player
 *
 * \param self  [\p FDtux]  FDtux instance
 * \param quest [\p string] Name of the quest to check
 *
 * \return TRUE if quest was assigned, FALSE otherwise
 *
 * \bindtype cfun
 */
LUAFD_DOC(bool has_quest(self, quest))

static int _has_quest(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	const char *quest = luaL_checkstring(L, 2);

	lua_pushboolean(L, self->me->AllMissions[GetMissionIndexByName(quest)].MissionWasAssigned);

	return 1;
}

/**
 * \brief Set the given quest as being completed, and fill the diary
 *
 * \param self  [\p FDtux]  FDtux instance
 * \param quest [\p string] Name of the quest to complete
 * \param text  [\p string] Text to add to the diary [optional]
 *
 * \bindtype cfun
 */
LUAFD_DOC(void complete_quest(self, quest, text))

static int _complete_quest(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	const char *quest = luaL_checkstring(L, 2);
	const char *text = luaL_optstring(L, 3, NULL);

	CompleteMission(quest);
	if (text != NULL)
		mission_diary_add(quest, text);

	return 0;
}

/**
 * \brief Return TRUE if the given quest was completed
 *
 * \param self  [\p FDtux]  FDtux instance
 * \param quest [\p string] Name of the quest to check
 *
 * \return TRUE if the quest was completed, FALSE otherwise
 *
 * \bindtype cfun
 */
LUAFD_DOC(bool done_quest(self, quest))

static int _done_quest(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	const char *quest = luaL_checkstring(L, 2);

	lua_pushboolean(L, self->me->AllMissions[GetMissionIndexByName(quest)].MissionIsComplete);

	return 1;
}

/**
 * \brief Add the given number of items to the inventory
 *
 * \param self   [\p FDtux]   FDtux instance
 * \param item   [\p string]  Name of the item to add
 * \param number [\p integer] Number of items to add [optional - default: 1]
 *
 * \bindtype cfun
 */
LUAFD_DOC(void add_item(self, item, number))

static int _add_item(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	const char *item_name = luaL_checkstring(L, 2);
	int number = luaL_optinteger(L, 3, 1);

	if (number == 0) {
		return luaL_error(L, "%s() %s", __FUNCTION__, "Tried to give item with 0 multiplicity");
	}

	struct item new_item = create_item_with_id(item_name, TRUE, number);

	// Either we put the new item directly into inventory or we issue a warning
	// that there is no room and then drop the item to the floor directly under
	// the current Tux position.  That can't fail, right?
	char msg[1000];
	if (!give_item(&new_item)) {
		sprintf(msg, _("Received Item: %s (on floor)"), item_name);
	} else {
		sprintf(msg, _("Received Item: %s"), item_name);
	}
	SetNewBigScreenMessage(msg);

	return 0;
}

/**
 * \brief Given the given number of items from the inventory
 *
 * \param self   [\p FDtux]   FDtux instance
 * \param item   [\p string]  Name of the item to remove
 * \param number [\p integer] Number of items to remove [optional - default: 1]
 *
 * \bindtype cfun
 */
LUAFD_DOC(void del_item_backpack(self, item, number))

static int _del_item_backpack(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	const char *item_name = luaL_checkstring(L, 2);
	int number = luaL_optinteger(L, 3, 1);
	DeleteInventoryItemsOfType(get_item_type_by_id(item_name), number);

	return 0;
}

/**
 * \brief Return the number of given items in the inventory
 *
 * \param self [\p FDtux]  FDtux instance
 * \param item [\p string] Name of the item to count
 *
 * \return Number of items in the inventory
 *
 * \bindtype cfun
 */
LUAFD_DOC(integer count_item_backpack(self, item))

static int _count_item_backpack(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	const char *item_name = luaL_checkstring(L, 2);

	lua_pushinteger(L, CountItemtypeInInventory(get_item_type_by_id(item_name)));

	return 1;
}

/**
 * \brief Create a new item and equip Tux with it
 *
 * \param self [\p FDtux]  FDtux instance
 * \param item [\p string] Name of the item to equip
 *
 * \bindtype cfun
 */
LUAFD_DOC(void equip_item(self, item))

static int _equip_item(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	const char *item_name = luaL_checkstring(L, 2);
	if (!item_name) {
		return luaL_error(L, "%s() %s", __FUNCTION__, "Tried to equip item without a name");
	}

	item new_item = create_item_with_id(item_name, TRUE, 1);
	equip_item(&new_item);
	SetNewBigScreenMessage(_("1 item received!"));

	return 0;
}

/**
 * \brief Check if the given item is equipped
 *
 * \param self [\p FDtux]  FDtux instance
 * \param item [\p string] Name of the item to check
 *
 * \return TRUE if the item is equipped, FALSE otherwise
 *
 * \bindtype cfun
 */
LUAFD_DOC(bool has_item_equipped(self, item))

static int _has_item_equipped(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	const char *item_name = luaL_checkstring(L, 2);
	int item_type = get_item_type_by_id(item_name);
	if ((self->me->weapon_item.type == item_type) || (self->me->drive_item.type == item_type) ||
	    (self->me->armour_item.type == item_type) || (self->me->shield_item.type == item_type) ||
	    (self->me->special_item.type == item_type)) {
		lua_pushboolean(L, TRUE);
	} else {
		lua_pushboolean(L, FALSE);
	}

	return 1;
}

/**
 * \brief Check if Tux has met the given NPC
 *
 * \param self [\p FDtux]  FDtux instance
 * \param npc  [\p string] Name of the NPC to check
 *
 * \return TRUE if the NPC was met, FALSE otherwise
 *
 * \bindtype cfun
 */
LUAFD_DOC(bool has_met(self, npc))

static int _has_met(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	const char *npc_name = luaL_checkstring(L, 2);
	struct npc *npc = npc_get(npc_name);
	lua_pushboolean(L, npc->chat_character_initialized);

	return 1;
}

/**
 * \brief Teleport Tux to the given map label
 *
 * \param self  [\p FDtux]  FDtux instance
 * \param label [\p string] Name of the label to teleport to. Use "~" to teleport home.
 *
 * \bindtype cfun
 */
LUAFD_DOC(void teleport(self, label))

static int _teleport(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	gps stop_pos = { -1, -1, -1 };
	const char *label = luaL_checkstring(L, 2);

	if (!strncmp(label, "~", 1)) {
		TeleportHome();
	} else {
		gps teleport_pos = get_map_label_center(label);

		reset_visible_levels();

		Teleport(teleport_pos.z, teleport_pos.x, teleport_pos.y, TRUE, TRUE);
		self->me->speed.x = 0.0;
		self->me->speed.y = 0.0;
		clear_out_intermediate_points(&stop_pos, self->me->next_intermediate_point, MAX_INTERMEDIATE_WAYPOINTS_FOR_TUX);

		clear_active_bullets();
	}

	return 0;
}

/**
 * \brief Check if a teleport anchor is active
 *
 * \param self [\p FDtux] FDtux instance
 *
 * \return TRUE if a teleport anchor is active, FALSE otherwise
 *
 * \bindtype cfun
 */
LUAFD_DOC(bool has_teleport_anchor(self))

static int _has_teleport_anchor(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_tux, L, "FDtux");

	/* Check that 'self' is of the right type */
	//struct tux** self = (struct tux**)luaL_testudata(L, 1, "FDtux");
	//if (!self) {
	//	return luaL_error(L, "%s() called with a userdata of wrong type (FDtux expected)", __FUNCTION__);
	//}

	if (self->me->teleport_anchor.z != -1)
		lua_pushboolean(L, TRUE);
	else
		lua_pushboolean(L, FALSE);

	return 1;
}

// end FDtux submodule
///@}
///////////////////////////////////////////////////////////////////////////////

/**
 * FDtux cfuns list
 */
static const luaL_Reg tux_cfuns[] = {
		{ "__gc", _garbage_collect },
		LUAFD_CFUN(get_player_name),
		LUAFD_CFUN(get_hp),
		LUAFD_CFUN(get_max_hp),
		LUAFD_CFUN(get_cool),
		LUAFD_CFUN(get_meters_traveled),
		LUAFD_CFUN(kill),
		LUAFD_CFUN(heal),
		LUAFD_CFUN(hurt),
		LUAFD_CFUN(heat),
		LUAFD_CFUN(freeze),
		LUAFD_CFUN(add_xp),
		LUAFD_CFUN(add_gold),
		LUAFD_CFUN(get_gold),
		LUAFD_CFUN(del_training_points),
		LUAFD_CFUN(get_training_points),
		LUAFD_CFUN(improve_skill),
		LUAFD_CFUN(get_skill),
		LUAFD_CFUN(improve_program),
		LUAFD_CFUN(downgrade_program),
		LUAFD_CFUN(get_program_revision),
		LUAFD_CFUN(change_stat),
		LUAFD_CFUN(assign_quest),
		LUAFD_CFUN(has_quest),
		LUAFD_CFUN(complete_quest),
		LUAFD_CFUN(done_quest),
		LUAFD_CFUN(add_item),
		LUAFD_CFUN(del_item_backpack),
		LUAFD_CFUN(count_item_backpack),
		LUAFD_CFUN(equip_item),
		LUAFD_CFUN(has_item_equipped),
		LUAFD_CFUN(has_met),
		LUAFD_CFUN(teleport),
		LUAFD_CFUN(has_teleport_anchor),
		{ NULL, NULL }
};

/**
 * \brief Create class-type metatable for Tux lua binding
 *
 * This function creates and stores in the C registry a FDtux metatable containing
 * cfuns and lfuns to act on the Tux C struct.\n
 * cfuns are defined in the tux_cfuns array.\n
 * lfuns are defined in the FDtux_lfuns.lua file.
 *
 * \param L Pointer to the Lua state to use
 *
 * \return TRUE
 */
int luaFD_tux_init(lua_State *L)
{
	// Create a metatable that will contain the Tux cfuncs and lfuncs.
	// The metatable is stored in the C registry.

	luaL_newmetatable(L, "FDtux");                                              // stack: (-1) metatable (1)
	lua_setfield(L, -1, "__index");                                             // empty stack

	// First load lfuns.
	// We use a simple trick: The metatable is, temporally, made available as a
	// global table called 'FDtux'. We then load a script that will add entries
	// into that table, using standard table access such as 'FDtux.key = value'.
	// Finally, we remove the table from the global name space.
	//
	// Note: Loading lfuns first will ensure that a lfun can not override a cfun
	// (because a cfun will overwrite a lfun with the same name).

	luaL_getmetatable(L, "FDtux");                                              // stack: (-1) metatable (1)
	lua_setglobal(L, "FDtux");                                                  // empty stack

	char fpath[PATH_MAX];
	if (find_file("FDtux_lfuns.lua", LUA_MOD_DIR, fpath, PLEASE_INFORM | IS_FATAL)) {
		if (luaL_loadfile(L, fpath)) {
			error_message(__FUNCTION__, "Aborting loading FDtux lfuns.\nError while loading ’%s’: %s",
					PLEASE_INFORM, "FDtux_lfuns.lua", lua_tostring(L, -1));
			lua_pop(L, 1);
			return FALSE;
		}

		if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
			error_message(__FUNCTION__, "Aborting loading FDtux lfuns.\nError while running ’%s’: %s",
					PLEASE_INFORM, "FDtux_lfuns.lua", lua_tostring(L, -1));
			lua_pop(L, 1);
			return FALSE;
		}
	}

	lua_pushnil(L);                                                             // stack: (-1) nil  (1)
	lua_setglobal(L, "FDtux");                                                  // empty stack

	// Now register the cfuns

	luaL_getmetatable(L, "FDtux");                                              // stack: (-1) metatable (1)
	luaL_setfuncs(L, tux_cfuns, 0);                                             // stack: (-1) metatable (1)

	// When running the dialogs validator, the execution of some methods will
	// crash because the game engine is not actually running.
	// The __override_for_validator lfun redefines those methods (cfuns as
	// well as lfuns) so that they do not call the engine.
	if (do_benchmark && !strncmp(do_benchmark, "dialog", 6)) {
		// Run FDtux.__override_for_validator, if it exists
		lua_getfield(L, -1, "__override_for_validator");                        // stack: (-1) fun < metatable (1)
		if (!lua_isnil(L, -1)) {
			lua_pushvalue(L, -2);                                               // stack: (-1) metatable < fun < metatable (1)
			lua_call(L, 1, 0);                                                  // stack: (-1) metatable (1)
		} else {
			lua_pop(L, 1);                                                      // stack: (-1) metatable (1)
		}
	}

	lua_pop(L, 1);                                                              // empty stack

	return TRUE;
}

/**
 * \brief Get a FDtux instance (singleton)
 *
 * On first call, this function creates a Lua userdata which acts as a FDtux
 * instance: the userdata is actually a pointer to the 'Me' C struct, associated
 * to a FDtux_class metatable.\n
 * The userdata is stored in the C registry, to be returned on next calls of this
 * function.\n
 * The created FDtux instance is thus a singleton.
 *
 * \param L Pointer to the Lua state to use
 *
 * \return 1 (userdata on the Lua stack)
 */
int luaFD_tux_get_instance(lua_State *L)
{
	// Try to get the userdata from the C registry.
	// If it does not exist, create it.

	lua_rawgetp(L, LUA_REGISTRYINDEX, (void *)&Me);                                        // stack: (-1) userdata (1)

	if (lua_isnil(L, -1)) {

		lua_pop(L, 1);                                                                     // empty stack

		/* Create a userdata which is a pointer to the 'Me' C struct, and make it be a FDtux instance */
		struct luaFD_tux* userdata = (struct luaFD_tux *)lua_newuserdata(L, sizeof(struct luaFD_tux));    // stack: (-1) userdata (1)
		userdata->me = &Me;
		luaL_setmetatable(L, "FDtux");                                                     // stack: (-1) userdata (1)

		/* Register the userdata, using the value of the pointer to create a lightuserdata key */
		lua_pushvalue(L, -1);                                                              // stack: (-1) userdata < userdata (1)
		lua_rawsetp(L, LUA_REGISTRYINDEX, (void *)&Me);                                    // stack: (-1) userdata (1)
    }

	// Return the userdata on the Lua stack

	return 1;
}
