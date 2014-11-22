/*
 *
 *  Copyright (c) 2014 Samuel Degrande
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
 * \file luaFD_npc.c
 * \brief This file contains the definition of the NPC Lua binding.
 */

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "luaFD.h"

/*
 * From a Lua script point of view, a NPC is, currently, composed by the
 * association of an 'enemy' C struct and a 'npc' C struct (if a dialog exists
 * for that enemy).
 * The Lua binding holds a reference to the 2 datastructures defining the
 * 'NPC'.
 */

struct luaFD_npc {
	struct enemy* enemy_ref;
	struct npc*   npc_ref;
};

/**
 * Called when Lua garbagecollects the NPC.
 */
static int _garbage_collect(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");
	// Nothing to do, currently
	return 0;
}

/**
 * Retrieve current chat context, and fail with error if there is no dialog
 * currently running.
 */
static struct chat_context *__get_current_chat_context(const char *funcname)
{
	struct chat_context *current_chat_context = chat_get_current_context();
	if (!current_chat_context)
		error_message(funcname, _("No chat context available on the context stack."), PLEASE_INFORM | IS_FATAL);
	return current_chat_context;
}

#define GET_CURRENT_CHAT_CONTEXT() __get_current_chat_context(__FUNCTION__)

///////////////////////////////////////////////////////////////////////////////
/// \defgroup FDnpc FDnpc: Lua NPC binding
/// \ingroup luaFD_bindings
///
/// Some doc on FDnpc - TO BE WRITTEN
///
///@{
// start FDnpc submodule
// Note: Functions comments are written to reflect their Lua usage.

/**
 * \brief Get NPC's name (enemy's short description)
 *
 * \param self [\p FDnpc] FDnpc instance
 *
 * \return [\p string] Name
 *
 * \bindtype cfun
 */
LUAFD_DOC(string get_name(self))

static int _get_name(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");

	if (self->enemy_ref->short_description_text)
		lua_pushstring(L, self->enemy_ref->short_description_text);
	else
		return luaL_error(L, "This NPC has no name ?");

	return 1;
}

/**
 * \brief Get translated npc's name (enemy's short description)
 *
 * \param self [\p FDnpc] FDnpc instance
 *
 * \return [\p string] Translated name
 *
 * \bindtype cfun
 */
LUAFD_DOC(string get_translated_name(self))

static int _get_translated_name(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");

	if (self->enemy_ref->short_description_text)
		lua_pushstring(L, D_(self->enemy_ref->short_description_text));
	else
		return luaL_error(L, "This NPC has no name ?");

	return 1;
}

/**
 * \brief Set npc's name (enemy's short description)
 *
 * \param self [\p FDnpc] FDnpc instance
 * \param name [\p string] New name
 *
 * \bindtype cfun
 */
LUAFD_DOC(void set_name(self, name))

static int _set_name(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");

	const char *npc_name = luaL_checkstring(L, 2);
	free(self->enemy_ref->short_description_text);
	self->enemy_ref->short_description_text = strdup(npc_name);
	return 0;
}

/**
 * \brief Get npc's type (enemy's type)
 *
 * \param self [\p FDnpc] FDnpc instance
 *
 * \return [\p string] Type
 *
 * \bindtype cfun
 */
LUAFD_DOC(string get_type(self))

static int _get_type(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");

	lua_pushstring(L, Droidmap[self->enemy_ref->type].droidname);
	return 1;
}

/**
 * \brief Get npc's class (enemy's class)
 *
 * \param self [\p FDnpc] FDnpc instance
 *
 * \return [\p string] Class
 *
 * \bindtype cfun
 */
LUAFD_DOC(string get_class(self))

static int _get_class(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");

	lua_pushinteger(L, Droidmap[self->enemy_ref->type].class);
	return 1;
}

/**
 * \brief Set npc's state (enemy's state)
 *
 * \param self [\p FDnpc] FDnpc instance
 * \param state [\p string] New state
 *
 * \bindtype cfun
 */
LUAFD_DOC(void set_state(self, state))

static int _set_state(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");

	const char *cmd = luaL_checkstring(L, 2);
	enemy_set_state(self->enemy_ref, cmd);
	return 0;
}

/**
 * \brief Set npc's faction
 *
 * \param self [\p FDnpc] FDnpc instance
 * \param faction [\p string] New faction
 *
 * \bindtype cfun
 */
LUAFD_DOC(void set_state(self, faction))

static int _set_faction(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");

	const char *faction = luaL_checkstring(L, 2);
	self->enemy_ref->faction = get_faction_id(faction);
	return 0;
}

/**
 * \brief Set npc's new destination (map label)
 *
 * \param self [\p FDnpc] FDnpc instance
 * \param label [\p string] Map label to set as the new destination
 *
 * \bindtype cfun
 */
LUAFD_DOC(void set_destination(self, label))

static int _set_destination(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");

	const char *label = luaL_checkstring(L, 2);
	enemy_set_destination(self->enemy_ref, label);
	return 0;
}

/**
 * \brief Set npc to rush Tux or not
 *
 * \param self [\p FDnpc]  FDnpc instance
 * \param flag [\p boolean] true/false: If 'true', the npc will rush Tux
 *
 * \bindtype cfun
 */
LUAFD_DOC(void set_rush_tux(self, flag))

static int _set_rush_tux(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");

	if (!lua_isboolean(L, 2)) {
		return luaL_error(L, "boolean expected, got %s", luaL_typename(L, 2));
	}
	const uint8_t flag = (uint8_t)lua_toboolean(L, 2);
	self->enemy_ref->will_rush_tux = flag;
	return 0;
}

/**
 * \brief Return npc's ability to rush Tux or not
 *
 * \param self [\p FDnpc] FDnpc instance
 *
 * \return [\p boolean] 'true' if the npc's is configured to rush Tux, 'false' otherwise
 *
 * \bindtype cfun
 */
LUAFD_DOC(boolean get_rush_tux(self))

static int _get_rush_tux(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");

	lua_pushboolean(L, self->enemy_ref->will_rush_tux);
	return 1;
}

/**
 * \brief Get npc's current HP value
 *
 * \param self [\p FDnpc] FDnpc instance
 *
 * \return [\p number] NPC's current HP
 *
 * \bindtype cfun
 */
LUAFD_DOC(number get_health(self))

static int _get_health(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");

	lua_pushnumber(L, self->enemy_ref->energy);
	return 1;
}

/**
 * \brief Get npc's maximum HP value
 *
 * \param self [\p FDnpc] FDnpc instance
 *
 * \return [\p number] NPC's maximum HP
 *
 * \bindtype cfun
 */
LUAFD_DOC(number get_max_health(self))

static int _get_max_health(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");

	lua_pushnumber(L, Droidmap[self->enemy_ref->type].maxenergy);
	return 1;
}

/**
 * \brief Set the item dropped when the npc dies.
 *
 * \param self [\p FDnpc] FDnpc instance
 * \param item [\p string] Item's name (or "NONE")
 *
 * \bindtype cfun
 */
LUAFD_DOC(void set_death_item(self, item))

static int _set_death_item(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");

	const char *item_id = luaL_checkstring(L, 2);
	if (!strcmp(item_id, "NONE"))
		self->enemy_ref->on_death_drop_item_code = -1;
	else
		self->enemy_ref->on_death_drop_item_code = get_item_type_by_id(item_id);
	return 0;
}

/**
 * \brief Check if the npc is dead or alive.
 *
 * \param self [\p FDnpc] FDnpc instance
 *
 * \return [\p boolean] 'true' if npc is dead
 *
 * \bindtype cfun
 */
LUAFD_DOC(boolean is_dead(self))

static int _is_dead(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");

	int dead = FALSE;
	enemy *erot;
	BROWSE_DEAD_BOTS(erot) {
		if (!strcmp(erot->dialog_section_name, self->enemy_ref->dialog_section_name)) {
			dead = TRUE;
			break;
		}
	}

	lua_pushboolean(L, dead);
	return 1;
}

/**
 * \brief Teleport the npc to a given map label
 *
 * \param self  [\p FDnpc] FDnpc instance
 * \param label [\p string] Map label
 *
 * \bindtype cfun
 */
LUAFD_DOC(void teleport(self, label))

static int _teleport(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");

	const char *label = luaL_checkstring(L, 2);
	gps teleport_pos = get_map_label_center(label);
	teleport_enemy(self->enemy_ref, teleport_pos.z, teleport_pos.x, teleport_pos.y);
	return 0;
}

/**
 * \brief Heal the npc, by restoring its maximum energy
 *
 * \param self [\p FDnpc] FDnpc instance
 *
 * \bindtype cfun
 */
LUAFD_DOC(void heal(self))

static int _heal(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");

	self->enemy_ref->energy = Droidmap[self->enemy_ref->type].maxenergy;
	return 0;
}

/**
 * \brief Freeze the npc, for a given duration
 *
 * \param self     [\p FDnpc]  FDnpc instance
 * \param duration [\p number] Duration
 *
 * \bindtype cfun
 */
LUAFD_DOC(void freeze(self, duration))

static int _freeze(lua_State *L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");

	float duration = luaL_checknumber(L, 2);
	self->enemy_ref->paralysation_duration_left = duration;
	return 0;
}

/**
 * \brief Hit npc to death
 *
 * \param self [\p FDnpc] FDnpc instance
 *
 * \bindtype cfun
 */
LUAFD_DOC(void drop_dead(self))

static int _drop_dead(lua_State * L)
{
	GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");

	hit_enemy(self->enemy_ref, self->enemy_ref->energy + 1, 0, Droidmap[self->enemy_ref->type].is_human - 2, 0);

	// If hit enemy is the one we are talking with, close the dialog
	struct chat_context *current_chat_context = GET_CURRENT_CHAT_CONTEXT();
	if (self->enemy_ref == current_chat_context->partner)
		current_chat_context->end_dialog = 1;

	return 0;
}

// end FDnpc submodule
///@}
///////////////////////////////////////////////////////////////////////////////

/**
 * FDnpc cfuns list
 */
static const luaL_Reg npc_cfuns[] = {
		{ "__gc", _garbage_collect },
		LUAFD_CFUN(get_name),
		LUAFD_CFUN(get_translated_name),
		LUAFD_CFUN(set_name),
		LUAFD_CFUN(get_type),
		LUAFD_CFUN(get_class),
		LUAFD_CFUN(set_state),
		LUAFD_CFUN(set_faction),
		LUAFD_CFUN(set_destination),
		LUAFD_CFUN(set_rush_tux),
		LUAFD_CFUN(get_rush_tux),
		LUAFD_CFUN(get_health),
		LUAFD_CFUN(get_max_health),
		LUAFD_CFUN(set_death_item),
		LUAFD_CFUN(is_dead),
		LUAFD_CFUN(teleport),
		LUAFD_CFUN(heal),
		LUAFD_CFUN(freeze),
		LUAFD_CFUN(drop_dead),
		{ NULL, NULL }
};

/**
 * \brief Create class-type metatable for NPCanic lua binding
 *
 * This function creates and stores in the C registry a FDnpc metatable containing
 * cfuns and lfuns to act on the C structs defining a NPC.\n
 * cfuns are defined in the npc_cfuns array.\n
 * lfuns are defined in the FDnpc_lfuns.lua file.
 *
 * \param L Pointer to the Lua state to use
 *
 * \return TRUE
 */
int luaFD_npc_init(lua_State *L)
{
	// Create a metatable that will contain the NPC cfuncs and lfuncs.
	// The metatable is stored in the C registry.

	luaL_newmetatable(L, "FDnpc");                                             // stack: (-1) metatable (1)
	lua_setfield(L, -1, "__index");                                             // empty stack

	// First load lfuns.
	// We use a simple trick: The metatable is, temporally, made available as a
	// global table called 'FDnpc'. We then load a script that will add entries
	// into that table, using standard table access such as 'FDnpc.key = value'.
	// Finally, we remove the table from the global name space.
	//
	// Note: Loading lfuns first will ensure that a lfun can not override a cfun
	// (because a cfun will overwrite a lfun with the same name).

	luaL_getmetatable(L, "FDnpc");                                             // stack: (-1) metatable (1)
	lua_setglobal(L, "FDnpc");                                                 // empty stack

	char fpath[PATH_MAX];
	if (find_file("FDnpc_lfuns.lua", LUA_MOD_DIR, fpath, PLEASE_INFORM | IS_FATAL)) {
		if (luaL_loadfile(L, fpath)) {
			error_message(__FUNCTION__, "Aborting loading FDnpc lfuns.\nError while loading ’%s’: %s",
					PLEASE_INFORM, "FDnpc_lfuns.lua", lua_tostring(L, -1));
			lua_pop(L, 1);
			return FALSE;
		}

		if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
			error_message(__FUNCTION__, "Aborting loading FDnpc lfuns.\nError while running ’%s’: %s",
					PLEASE_INFORM, "FDnpc_lfuns.lua", lua_tostring(L, -1));
			lua_pop(L, 1);
			return FALSE;
		}
	}

	lua_pushnil(L);                                                             // stack: (-1) nil  (1)
	lua_setglobal(L, "FDnpc");                                                 // empty stack

	// Now register the cfuns

	luaL_getmetatable(L, "FDnpc");                                             // stack: (-1) metatable (1)
	luaL_setfuncs(L, npc_cfuns, 0);                                            // stack: (-1) metatable (1)

	// When running the dialogs validator, the execution of some methods will
	// crash because the game engine is not actually running.
	// The __override_for_validator lfun redefines those methods (cfuns as
	// well as lfuns) so that they do not call the engine.
	if (do_benchmark && !strncmp(do_benchmark, "dialog", 6)) {
		// Run FDnpc.__override_for_validator, if it exists
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
 * \brief Get a FDnpc instance (singleton)
 *
 * On first call, this function creates a Lua userdata which acts as a FDnpc
 *
 * blah...
 *
 * \param L Pointer to the Lua state to use
 *
 * \return 1 (userdata on the Lua stack)
 */
int luaFD_npc_get_instance(lua_State *L)
{
	struct enemy *partner = NULL;
	const char *dialog = luaL_optstring(L, 1, NULL);

	if (!dialog) {
		struct chat_context *current_chat_context = GET_CURRENT_CHAT_CONTEXT();
		partner = current_chat_context->partner;
		if (!partner) {
			error_message(__FUNCTION__, ("Current chat context has no 'partner'. How is it possible?"), PLEASE_INFORM | IS_FATAL);
			return 0;
		}
	} else {
		partner = get_enemy_with_dialog(dialog);
		if (!partner)
			return luaL_error(L, ("Could not find a droid with name \"%s\""), dialog);
	}

	// Try to get the userdata from the C registry.
	// If it does not exist, create it.

	lua_rawgetp(L, LUA_REGISTRYINDEX, (void *)partner);                         // stack: (-1) userdata (1)

	if (lua_isnil(L, -1)) {

		lua_pop(L, 1);                                                          // empty stack

		/* Create a userdata and make it be a FDnpc instance */
		struct luaFD_npc* userdata = (struct luaFD_npc*)lua_newuserdata(L, sizeof(struct luaFD_npc));  // stack: (-1) userdata (1)
		userdata->enemy_ref = partner;
		userdata->npc_ref = npc_get(partner->dialog_section_name);
		luaL_setmetatable(L, "FDnpc");                                         // stack: (-1) userdata (1)

		/* Register the userdata, using the value of the pointer to create a lightuserdata key */
		lua_pushvalue(L, -1);                                                   // stack: (-1) userdata < userdata (1)
		lua_rawsetp(L, LUA_REGISTRYINDEX, (void *)partner);                     // stack: (-1) userdata (1)
    }

	// Return the userdata (already on the Lua stack)
	return 1;
}
