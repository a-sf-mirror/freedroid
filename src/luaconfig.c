/*
 *
 *   Copyright (c) 2010 Samuel Degrande
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
 * This file contains table constructors used to load the config files writtent in Lua
 */

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "savestruct.h"

#include "../lua/lua.h"
#include "../lua/lauxlib.h"
#include "../lua/lualib.h"

/* Our Lua state for event execution (defined in lua.c) */
extern lua_State *global_lua_state;

/**
 * Data types that could be read from a Lua config file
 */
enum data_type {
	BOOL_TYPE = 0,
	INT_TYPE,
	FLOAT_TYPE,
	DOUBLE_TYPE,
	STRING_TYPE
};

/**
 * Structure used to retrieve and store a field of a Lua table
 */
struct data_spec {
	char *name;           // Data key (as defined in the Lua table)
	char *default_value;  // Default value defined in a string
	enum data_type type;  // Type of the data (see enum data_type)
	void *data;           // Generic pointer to the data container
};

/**
 * \brief Set the value of a data from a field of a Lua table.
 * \param L Lua state.
 * \param index Stack index where the table is.
 * \param field Name of the field to fetch.
 * \param type Type of the data to read
 * \param result Location of the data to set
 * \return TRUE if the value was read, FALSE if it could not be read.
 */
static int set_value_from_table(lua_State *L, int index, const char *field, enum data_type type, void *result)
{
	int found_and_valid = FALSE;
	int ltype;

	lua_getfield(L, index, field);
	ltype = lua_type(L, -1);

	switch (type) {
	case INT_TYPE:
		if (ltype == LUA_TNUMBER) {
			*((int *)result) = lua_tointeger(L, -1);
			found_and_valid = TRUE;
		}
		break;
	case FLOAT_TYPE:
		if (ltype == LUA_TNUMBER) {
			*((float *)result) = (float)lua_tonumber(L, -1);
			found_and_valid = TRUE;
		}
		break;
	case DOUBLE_TYPE:
		if (ltype == LUA_TNUMBER) {
			*((double *)result) = (double)lua_tonumber(L, -1);
			found_and_valid = TRUE;
		}
		break;
	case STRING_TYPE:
		if (ltype == LUA_TSTRING) {
			*((char **)result) = strdup(lua_tostring(L, -1));
			found_and_valid = TRUE;
		}
		break;
	default:
		break;
	}

	lua_pop(L, 1);
	return found_and_valid;
}

/**
 * Set a data field to its default value
 */
static void set_value_from_default(const char *default_value, enum data_type type, void *data)
{
	switch (type) {
	case INT_TYPE:
		*((int *)data) = atoi(default_value);
		break;
	case FLOAT_TYPE:
		*((float *)data) = (float)atof(default_value);
		break;
	case DOUBLE_TYPE:
		*((double *)data) = atof(default_value);
		break;
	case STRING_TYPE:
		*((char **)data) = strdup(default_value);
		break;
	default:
		break;
	}
}

/**
 * Extract data structure content from a table on the Lua stack
 */
static void set_structure_from_table(lua_State *L, struct data_spec *data_specs)
{
	int i;

	for (i = 0; data_specs[i].name != NULL; i++) {
		if (!set_value_from_table(L, 1, data_specs[i].name, data_specs[i].type, data_specs[i].data)) {
			set_value_from_default(data_specs[i].default_value, data_specs[i].type, data_specs[i].data);
		}
	}
}

/**
 * Addon constructor
 */
static int lua_register_addon(lua_State *L)
{
	char *name = NULL;
	struct addon_bonus bonus;
	struct addon_material material;
	struct addon_spec addonspec;

	// Read the item name and find the item index.
	memset(&addonspec, 0, sizeof(struct addon_spec));
	set_value_from_table(L, 1, "name", STRING_TYPE, &name);
	addonspec.type = GetItemIndexByName(name);
	free(name);

	// Read the simple add-on specific fields.
	set_value_from_table(L, 1, "require_socket", STRING_TYPE, &addonspec.requires_socket);
	set_value_from_table(L, 1, "require_item", STRING_TYPE, &addonspec.requires_item);
	set_value_from_table(L, 1, "upgrade_cost", INT_TYPE, &addonspec.upgrade_cost);

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

/*
 * Tux_anim constructor
 */
static int lua_tuxanimation_ctor(lua_State *L)
{
	// Specification of the data structure to retrieve from the lua table
	struct data_spec data_specs[] = {
		{ "standing_keyframe",     "15",  INT_TYPE,   &tux_anim.standing_keyframe     },
		{ "attack_duration",       "1.0", FLOAT_TYPE, &tux_anim.attack.duration       },
		{ "attack_first_keyframe", "0",   INT_TYPE,   &tux_anim.attack.first_keyframe },
		{ "attack_last_keyframe",  "-1",  INT_TYPE,   &tux_anim.attack.last_keyframe  },
		{ "walk_distance",         "0.0", FLOAT_TYPE, &tux_anim.walk.distance         },
		{ "walk_first_keyframe",   "0",   INT_TYPE,   &tux_anim.walk.first_keyframe   },
		{ "walk_last_keyframe",    "-1",  INT_TYPE,   &tux_anim.walk.last_keyframe    },
		{ "run_distance",          "0.0", FLOAT_TYPE, &tux_anim.run.distance          },
		{ "run_first_keyframe",    "0",   INT_TYPE,   &tux_anim.run.first_keyframe    },
		{ "run_last_keyframe",     "-1",  INT_TYPE,   &tux_anim.run.last_keyframe     },
		{ NULL, NULL, 0, 0 }
	};

	set_structure_from_table(L, data_specs);

	// Post-process
	tux_anim.attack.nb_keyframes = tux_anim.attack.last_keyframe - tux_anim.attack.first_keyframe + 1;
	tux_anim.walk.nb_keyframes = tux_anim.walk.last_keyframe - tux_anim.walk.first_keyframe + 1;
	tux_anim.run.nb_keyframes = tux_anim.run.last_keyframe - tux_anim.run.first_keyframe + 1;

	return 0;
}

void init_luaconfig()
{
	int i;

	luaL_reg lfuncs[] = {
		/*
		 * Constructors of new data types.
		 */
		{"addon", lua_register_addon}
		,
		{"tux_animation", lua_tuxanimation_ctor}
		,
		{NULL, NULL}
	};

	for (i = 0; lfuncs[i].name != NULL; i++) {
		lua_pushcfunction(global_lua_state, lfuncs[i].func);
		lua_setglobal(global_lua_state, lfuncs[i].name);
	}
}
