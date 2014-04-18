/*
 *
 *   Copyright (c) 2010-2013 Samuel Degrande
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
 * This file contains table constructors used to load the config files written in Lua
 */

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lua.h"
#include "lauxlib.h"

#include "lvledit/lvledit_object_lists.h"

/* Lua state for config execution (defined in lua.c) */
extern lua_State *config_lua_state;

/**
 * Data types that could be read from a Lua config file
 */
enum data_type {
	BOOL_TYPE = 0,
	SHORT_TYPE,
	INT_TYPE,
	FLOAT_TYPE,
	DOUBLE_TYPE,
	STRING_TYPE,
	INT_ARRAY,
	FLOAT_ARRAY,
	STRING_ARRAY,
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
 * Get a field in a hierarchy of arrays, using a path name, and push
 * the field on stack.
 *
 * The path separator is '.'.
 * Example: "foo.bar" => foo = {bar = "X"}
 * 
 * The function mimics the lua_getfield() behavior.
 *
 * \param L    Lua state
 * \param path Path of the field to fetch
 */
static void lua_getfield_by_path(lua_State *L, const char *path)
{
	int stack_counter = 0;

	if (!path) {
		lua_pushnil(L);
		return;
	}

	// For each subpart of the path, we try to find T[subpart], with T being
	// a table on the top of the lua stack.
	// If the field is found, then it is pushed on the lua stack, and we loop
	// to find the next subpart.
	// It the field is not found, then a LUA_TNIL is pushed to denote a search
	// failure (this mimics lua_getfield() behavior).

	char *tmp_path = strdup(path);
	char *token = strtok(tmp_path, ".");

	while (token) {

		// If the top of the lua stack is not a table, then there is an error
		// in the lua file.
		if (lua_type(L, -1) != LUA_TTABLE) {
			lua_pushnil(L);
			stack_counter++;
			break;
		}

		// Search the token name in the current top table
		lua_getfield(L, -1, token);
		stack_counter++;

		if (lua_type(L, -1) == LUA_TNIL) {
			// The field was not found
			break;
		}

		// Extract next token
		token = strtok(NULL, ".");
	}

	// Remove all the intermediate tables that were pushed on the stack, and
	// only keep to last pushed, so that only the field corresponding to the
	// whole search path is kept.
	
	if (stack_counter > 1) {
		// Replace the first pushed field with the last one.
		lua_replace(L, -stack_counter);
		stack_counter--;
		// Pop all remaining intermediate tables
		lua_pop(L, stack_counter - 1);
	}

	free(tmp_path);
}

/**
 * Get the value of a data from the top of the Lua stack.
 *
 * \param L      Lua state
 * \param type   Expected type of the data
 * \param result Pointer to the data location
 *
 * \return TRUE if the value was set, FALSE otherwise.
 */
static int get_value_from_stack(lua_State *L, enum data_type type, void *result)
{
	int found_and_valid = FALSE;
	int ltype;
	
	ltype = lua_type(L, -1);

	if (ltype == LUA_TNIL)
		return FALSE;

	switch (type) {
	case BOOL_TYPE:
		if (ltype == LUA_TBOOLEAN) {
			*((char *)result) = lua_toboolean(L, -1);
			found_and_valid = TRUE;
		}
		break;
	case SHORT_TYPE:
		if (ltype == LUA_TNUMBER) {
			*((short *)result) = (short)lua_tointeger(L, -1);
			found_and_valid = TRUE;
		}
		break;
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
	case INT_ARRAY:
		if (ltype == LUA_TTABLE) {
			// Init a dynarray, using the lua table's length
			dynarray_init((struct dynarray *)result, lua_rawlen(L, -1), sizeof(int));
			// Fill the dynarray with the content of the lua table
			lua_pushnil(L);
			while (lua_next(L, -2) != 0) {
				if (lua_type(L, -2) == LUA_TNUMBER && lua_type(L, -1) == LUA_TNUMBER) {
					int value = (int)lua_tonumber(L, -1);
					dynarray_add((struct dynarray *)result, &value, sizeof(int));
					found_and_valid = TRUE;
				}
				lua_pop(L, 1);
			}
		} else {
			// The data is not in a lua table: try with a single value
			if (ltype == LUA_TNUMBER) {
				int value = lua_tointeger(L, -1);
				dynarray_init((struct dynarray *)result, 1, sizeof(int));
				dynarray_add((struct dynarray *)result, &value, sizeof(int));
				found_and_valid = TRUE;
			}
		}
		break;
	case FLOAT_ARRAY:
		if (ltype == LUA_TTABLE) {
			// Init a dynarray, using the lua table's length
			dynarray_init((struct dynarray *)result, lua_rawlen(L, -1), sizeof(float));
			// Fill the dynarray with the content of the lua table
			lua_pushnil(L);
			while (lua_next(L, -2) != 0) {
				if (lua_type(L, -2) == LUA_TNUMBER && lua_type(L, -1) == LUA_TNUMBER) {
					float value = (float)lua_tonumber(L, -1);
					dynarray_add((struct dynarray *)result, &value, sizeof(float));
					found_and_valid = TRUE;
				}
				lua_pop(L, 1);
			}
		} else {
			// The data is not in a lua table: try with a single value
			if (ltype == LUA_TNUMBER) {
				float value = (float)lua_tonumber(L, -1);
				dynarray_init((struct dynarray *)result, 1, sizeof(float));
				dynarray_add((struct dynarray *)result, &value, sizeof(float));
				found_and_valid = TRUE;
			}
		}
		break;
	case STRING_ARRAY:
		if (ltype == LUA_TTABLE) {
			// Init a dynarray, using the lua table's length
			dynarray_init((struct dynarray *)result, lua_rawlen(L, -1), sizeof(char *));
			// Fill the dynarray with the content of the lua table
			lua_pushnil(L);
			while (lua_next(L, -2) != 0) {
				if (lua_type(L, -2) == LUA_TNUMBER && lua_type(L, -1) == LUA_TSTRING) {
					char *value = strdup(lua_tostring(L, -1));
					dynarray_add((struct dynarray *)result, &value, sizeof(char *));
					found_and_valid = TRUE;
				}
				lua_pop(L, 1);
			}
		} else {
			// The data is not in a lua table: try with a single value
			if (ltype == LUA_TSTRING) {
				char *value = strdup(lua_tostring(L, -1));
				dynarray_init((struct dynarray *)result, 1, sizeof(char *));
				dynarray_add((struct dynarray *)result, &value, sizeof(char *));
				found_and_valid = TRUE;
			}
		}
		break;
	default:
		break;
	}

	return found_and_valid;
}

/**
 * Get the value of a data from a field of a table on top of the Lua stack
 *
 * \param L      Lua state
 * \param field  Name of the field to fetch
 * \param type   Expected type of the data
 * \param result Pointer to the data location
 *
 * \return TRUE if the value was set, FALSE otherwise.
 */
static int get_value_from_table(lua_State *L, char *field, enum data_type type, void *result)
{
	int found_and_valid = FALSE;

	lua_getfield_by_path(L, field);
	found_and_valid = get_value_from_stack(L, type, result);
	lua_pop(L, 1);

	return found_and_valid;
}

/**
 * Set a data to its default value
 *
 * \param default_value Default value, encoded in a string.
 * \param type          Type of the data to set
 * \param data          Pointer to the data location
 */
static void set_value_to_default(const char *default_value, enum data_type type, void *data)
{
	switch (type) {
	case BOOL_TYPE:
		*((char *) data) = !strcmp("true", default_value);
		break;
	case SHORT_TYPE:
		*((short *)data) = (short)atoi(default_value);
		break;
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
		*((char **)data) = (default_value==NULL)?NULL:strdup(default_value);
		break;
	case INT_ARRAY:
		// Add the default value to the dynarray
		{
			int int_value = atoi(default_value);
			dynarray_init((struct dynarray *)data, 1, sizeof(int));
			dynarray_add((struct dynarray *)data, &int_value, sizeof(int));
		}
		break;
	case FLOAT_ARRAY:
		// Add the default value to the dynarray
		{
			float float_value = atof(default_value);
			dynarray_init((struct dynarray *)data, 1, sizeof(float));
			dynarray_add((struct dynarray *)data, &float_value, sizeof(float));
		}
		break;
	case STRING_ARRAY:
		// Add the default value to the dynarray
		if (default_value != NULL) {
			dynarray_init((struct dynarray *)data, 1, sizeof(char *));
			dynarray_add((struct dynarray *)data, strdup(default_value), sizeof(char *));
		} else {
			dynarray_free((struct dynarray *)data);
		}
		break;
	default:
		break;
	}
}

/**
 * Clean data structure content
 *
 * \param data_specs An array of data_spec to cleanup
 */
static void clean_structure(struct data_spec *data_specs)
{
	int i;

	for (i = 0; data_specs[i].name != NULL; i++) {
		switch (data_specs[i].type) {
		case BOOL_TYPE:
		case SHORT_TYPE:
		case INT_TYPE:
		case FLOAT_TYPE:
		case DOUBLE_TYPE:
			break;
		case STRING_TYPE:
			if (data_specs[i].data != NULL)
				free (*((char **)data_specs[i].data));
			break;
		case INT_ARRAY:
		case FLOAT_ARRAY:
			dynarray_free((struct dynarray *)(data_specs[i].data));
			break;
		case STRING_ARRAY:
			{
				// Free the content of the dynarray (pointers to char), before
				// to free the dynarray by itself
				int j;
				struct dynarray *dyn_arr = (struct dynarray *)data_specs[i].data;
				char **actual_arr = (char **)(dyn_arr->arr);
				for (j = 0; j < dyn_arr->size; j++) {
					if (actual_arr[j] != NULL)
						free (actual_arr[j]);
				}
				dynarray_free(dyn_arr);
			}
			break;
		default:
			break;
		}
	}
}

/**
 * Fill a data structure content from a table on the Lua stack
 *
 * \param L          Lua state.
 * \param data_specs An array of data_spec, defining the values to retrieve, and how to retrieve them
 */
static void fill_structure_from_table(lua_State *L, struct data_spec *data_specs)
{
	int i;

	for (i = 0; data_specs[i].name != NULL; i++) {
		if (!get_value_from_table(L, data_specs[i].name, data_specs[i].type, data_specs[i].data)) {
			set_value_to_default(data_specs[i].default_value, data_specs[i].type, data_specs[i].data);
		}
	}
}

/**
 * Fill a dynarray content from a table on the Lua stack, using a callback
 * function to extract each element.
 *
 * \param L          Lua state.
 * \param array      The dynarray to be filled
 * \param datasize   Size (in bytes) of an element of the dynarray
 * \param extract_cb Callback function, called to extract an element from the Lua stack
 */
void fill_dynarray_from_table(lua_State *L, struct dynarray *array, int datasize, void (*extract_cb)(lua_State *, void *))
{
	if (lua_type(L, -1) != LUA_TTABLE) {
		error_message(__FUNCTION__, "A Lua table is expected, but was not found", PLEASE_INFORM | IS_FATAL);
	}
	if (lua_rawlen(L, -1) < 1) {
		dynarray_free(array);
		return;
	}

	// Use the length of the Lua table (i.e the number of elements in the table)
	// to set the capacity of the dynarray.
	dynarray_init(array, lua_rawlen(L, -1), datasize);

	// This function has no idea of the type of the data stored in the dynarray.
	// However we only need to know its size in order to use an opaque data
	// that will be filled by extract_cb and 'memcopied' into the dynarray.
	void *data = MyMalloc(datasize);

	// For each element in the Lua table, call extract_cb to fill the data and
	// add it to the dynarray.
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		extract_cb(L, data);
		dynarray_add(array, data, datasize);
		lua_pop(L, 1);
	}
	lua_pop(L, 1);

	free(data);
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
	get_value_from_table(L, "name", STRING_TYPE, &name);
	addonspec.type = get_item_type_by_id(name);
	free(name);

	// Read the simple add-on specific fields.
	get_value_from_table(L, "require_socket", STRING_TYPE, &addonspec.requires_socket);
	get_value_from_table(L, "require_item", STRING_TYPE, &addonspec.requires_item);
	get_value_from_table(L, "upgrade_cost", INT_TYPE, &addonspec.upgrade_cost);

	// Process the table of bonuses. The keys of the table are the names
	// of the bonuses and the values the attribute increase amounts.
	lua_getfield(L, -1, "bonuses");
	if (lua_type(L, -1) == LUA_TTABLE) {
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TNUMBER) {
				bonus.name = strdup(lua_tostring(L, -2));
				bonus.value = lua_tonumber(L, -1);
				dynarray_add(&addonspec.bonuses, &bonus, sizeof(bonus));
			}
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);

	// Process the table of materials. The keys of the table are the names
	// of the materials and the values the required material counts.
	lua_getfield(L, -1, "materials");
	if (lua_type(L, -1) == LUA_TTABLE) {
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TNUMBER) {
				material.name = strdup(lua_tostring(L, -2));
				material.value = lua_tonumber(L, -1);
				dynarray_add(&addonspec.materials, &material, sizeof(material));
			}
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);

	// Register a new add-on specification.
	add_addon_spec(&addonspec);

	return 0;
}

/*
 * Tux_anim constructor. Called when a 'tux_animation object' is read in a Lua config file.
 */
static int lua_tuxanimation_ctor(lua_State *L)
{
	// Specification of the data structure to retrieve from the lua table
	struct data_spec data_specs[] = {
		{ "standing_keyframe",     "15",  INT_TYPE,   &tux_anim.standing_keyframe     },
		{ "attack.duration",       "1.0", FLOAT_TYPE, &tux_anim.attack.duration       },
		{ "attack.first_keyframe", "0",   INT_TYPE,   &tux_anim.attack.first_keyframe },
		{ "attack.last_keyframe",  "-1",  INT_TYPE,   &tux_anim.attack.last_keyframe  },
		{ "walk.distance",         "0.0", FLOAT_TYPE, &tux_anim.walk.distance         },
		{ "walk.first_keyframe",   "0",   INT_TYPE,   &tux_anim.walk.first_keyframe   },
		{ "walk.last_keyframe",    "-1",  INT_TYPE,   &tux_anim.walk.last_keyframe    },
		{ "run.distance",          "0.0", FLOAT_TYPE, &tux_anim.run.distance          },
		{ "run.first_keyframe",    "0",   INT_TYPE,   &tux_anim.run.first_keyframe    },
		{ "run.last_keyframe",     "-1",  INT_TYPE,   &tux_anim.run.last_keyframe     },
		{ NULL, NULL, 0, 0 }
	};

	fill_structure_from_table(L, data_specs);

	// Post-process
	tux_anim.attack.nb_keyframes = tux_anim.attack.last_keyframe - tux_anim.attack.first_keyframe + 1;
	tux_anim.walk.nb_keyframes = tux_anim.walk.last_keyframe - tux_anim.walk.first_keyframe + 1;
	tux_anim.run.nb_keyframes = tux_anim.run.last_keyframe - tux_anim.run.first_keyframe + 1;

	return 0;
}

/*
 * Tux_rendering_config constructor. Called when a 'tux_rendering_config object' is read in a Lua config file.
 */
static int lua_tuxrendering_config_ctor(lua_State *L)
{
	// Specification of the data structure to retrieve from the lua table
	struct data_spec data_specs[] = {
		{ "motion_class_names",      NULL,            STRING_ARRAY, &tux_rendering.motion_class_names },
		{ "default_parts.head",      "iso_head",      STRING_TYPE,  &tux_rendering.default_instances.head },
		{ "default_parts.torso",     "iso_torso",     STRING_TYPE,  &tux_rendering.default_instances.torso },
		{ "default_parts.weaponarm", "iso_weaponarm", STRING_TYPE,  &tux_rendering.default_instances.weaponarm },
		{ "default_parts.weapon",    NULL,            STRING_TYPE,  &tux_rendering.default_instances.weapon },
		{ "default_parts.shieldarm", "iso_shieldarm", STRING_TYPE,  &tux_rendering.default_instances.shieldarm },
		{ "default_parts.feet",      "iso_feet",      STRING_TYPE,  &tux_rendering.default_instances.feet },
		{ "gun_muzzle_height",       "-60",           INT_TYPE,     &tux_rendering.gun_muzzle_height },
		{ NULL, NULL, 0, 0 }
	};

	fill_structure_from_table(L, data_specs);

	// At least one motion class needs to be defined
	if (tux_rendering.motion_class_names.size < 1) {
		error_message(__FUNCTION__,
			"Tux rendering specification is invalid: at least one motion_class is needed",
			PLEASE_INFORM | IS_FATAL);
	}

	// Initialize the data structure used to store the Tux's parts rendering orders
	tux_rendering.render_order = (tux_part_render_motionclass *)MyMalloc(tux_rendering.motion_class_names.size * sizeof(tux_part_render_motionclass));

	int i, j;
	for (i = 0; i < tux_rendering.motion_class_names.size; i++) {
		for (j = 0; j < MAX_TUX_DIRECTIONS; j++) {
			tux_rendering.render_order[i][j] = NULL;
		}
	}

	return 0;
}

/**
 * Tux_ordering constructor. Called when a 'tux_ordering object' is read in a Lua config file.
 */
static int lua_tuxordering_ctor(lua_State *L)
{
	char *type;

	struct dynarray rotations;
	int phase_start;
	int phase_end;
	struct dynarray order;

	struct data_spec data_specs[] = {
		{ "type",        NULL, STRING_TYPE,  &type        },
		{ "rotations",   "0",  INT_ARRAY,    &rotations   },
		{ "phase_start", "0",  INT_TYPE,     &phase_start },
		{ "phase_end",   "-1", INT_TYPE,     &phase_end   },
		{ "order",       NULL, STRING_ARRAY, &order       },
		{ NULL, NULL, 0, 0 }
	};

	fill_structure_from_table(L, data_specs);

	// Check that the motion class is defined (and so that a data structure is
	// ready to store the retrieved rendering orders)
	int motion_class_id = get_motion_class_id_by_name(type);
	if (motion_class_id == -1) {
		error_message(__FUNCTION__,
				"Invalid tux_ordering spec:\n"
				"Unknown motion_class (%s)",
				PLEASE_INFORM | IS_FATAL, type);
	}

	// Fill the rendering order data structure according to what was retrieved
	// from the lua table
	int i;
	for (i = 0; i < rotations.size; i++) {

		// Check validity of the rotation index
		int rotation_idx = ((int *)rotations.arr)[i];
		if (rotation_idx < 0 || rotation_idx >= MAX_TUX_DIRECTIONS) {
			error_message(__FUNCTION__,
					"Invalid tux_ordering spec (motion_class: %s):\n"
					"rotation index (%d) must be between 0 and %d",
					PLEASE_INFORM | IS_FATAL, type, rotation_idx, MAX_TUX_DIRECTIONS - 1);
		}

		// Check validity of the phase values
		if (phase_start < 0 || phase_start >= TUX_TOTAL_PHASES) {
			error_message(__FUNCTION__,
					"Invalid tux_ordering spec (motion_class: %s - rotation %d):\n"
					"phase_start (%d) must be between 0 and %d",
					PLEASE_INFORM | IS_FATAL, type, rotation_idx, phase_start, TUX_TOTAL_PHASES - 1);
		}
		if (phase_end < -1 || phase_end >= TUX_TOTAL_PHASES) {
			error_message(__FUNCTION__,
					"Invalid tux_ordering spec (motion_class: %s - rotation %d):\n"
					"phase_end (%d) must be between -1 and %d",
					PLEASE_INFORM | IS_FATAL, type, rotation_idx, phase_end, TUX_TOTAL_PHASES - 1);
		}
		if ((phase_end != -1) && (phase_end < phase_start)) {
			error_message(__FUNCTION__,
					"Invalid tux_ordering spec (motion_class: %s - rotation %d):\n"
					"phase_start value (%d) must be lower or equal to phase_end value (%d)",
					PLEASE_INFORM | IS_FATAL, type, rotation_idx, phase_start, phase_end);
		}

		// Get the head of the tux_part_render_set linked list associated to the
		// current motion_class and rotation
		struct tux_part_render_set **render_order_ptr = &tux_rendering.render_order[motion_class_id][rotation_idx];

		// Find first free entry
		while ((*render_order_ptr) != NULL) {
			render_order_ptr = &((*render_order_ptr)->next);
		}

		// Create and fill a new entry
		*render_order_ptr = (struct tux_part_render_set *)MyMalloc(sizeof(struct tux_part_render_set));

		(*render_order_ptr)->phase_start = phase_start;
		(*render_order_ptr)->phase_end = (phase_end != -1) ? phase_end : (TUX_TOTAL_PHASES - 1);

		int j;
		for (j = 0; j < order.size; j++) {
			(*render_order_ptr)->part_render_data[j] = tux_get_part_render_data(((char **)order.arr)[j]);
		}

		(*render_order_ptr)->next = NULL;
	}

	// Cleanup
	clean_structure(data_specs);

	return 0;
}

static int lua_obstacle_ctor(lua_State *L)
{
	int i;
	struct obstacle_spec obstacle;

	struct dynarray borders;
	struct dynarray flags;
	int transparency;
	char *animation;
	struct dynarray groups = { 0 };

	char default_transparency[20];
	sprintf(default_transparency, "%d", TRANSPARENCY_FOR_WALLS);

	struct data_spec data_specs[] = {
		{ "name", NULL, STRING_TYPE, &obstacle.name },
		{ "label", NULL, STRING_TYPE, &obstacle.label },
		{ "borders", "0", FLOAT_ARRAY, &borders },
		{ "flags", "0", INT_ARRAY, &flags },
		{ "after_smashing", "-1", INT_TYPE, &obstacle.result_type_after_smashing_once },
		{ "after_looting", "-1", INT_TYPE, &obstacle.result_type_after_looting },
		{ "emitted_light_strength", "0", INT_ARRAY, &obstacle.emitted_light_strength },
		{ "transparency", default_transparency, INT_TYPE, &transparency },
		{ "action", NULL, STRING_TYPE, &obstacle.action },
		{ "animation", NULL, STRING_TYPE, &animation },
		{ "animation_fps", "10", FLOAT_TYPE, &obstacle.animation_fps },
		{ "blast_type", "1", INT_TYPE, &obstacle.blast_type },
		{ "smashed_sound", NULL, STRING_TYPE, &obstacle.smashed_sound },
		{ "groups", NULL, STRING_ARRAY, &groups },
		{ NULL, NULL, 0, 0 }
	};

	if (!get_value_from_table(L, "image_filenames", STRING_ARRAY, &obstacle.filenames)) {
			error_message(__FUNCTION__, "No image filename for obstacle. At least one image filename must be given.",
				PLEASE_INFORM | IS_FATAL);
	}

	fill_structure_from_table(L, data_specs);

#define DEFAULT_BORDER 0.6

	// Clear obstacle structure
	struct obstacle_graphics graphics;
	graphics.count = obstacle.filenames.size;
	graphics.images = MyMalloc(sizeof(struct image) * graphics.count);
	graphics.shadows = MyMalloc(sizeof(struct image) * graphics.count);
	dynarray_add(&obstacle_images, &graphics, sizeof(graphics));
	obstacle.left_border = -DEFAULT_BORDER;
	obstacle.right_border = DEFAULT_BORDER;
	obstacle.upper_border = -DEFAULT_BORDER;
	obstacle.lower_border = DEFAULT_BORDER;

	// Borders
	obstacle.block_area_type = COLLISION_TYPE_NONE;
	float *borders_array = borders.arr;
	if (borders.size == 4) {
		obstacle.block_area_type = COLLISION_TYPE_RECTANGLE;
		obstacle.left_border = borders_array[0];
		obstacle.right_border = borders_array[1];
		obstacle.upper_border = borders_array[2];
		obstacle.lower_border = borders_array[3];
	}
	dynarray_free(&borders);

	obstacle.block_area_parm_1 = obstacle.right_border - obstacle.left_border;
	obstacle.block_area_parm_2 = obstacle.lower_border - obstacle.upper_border;
	obstacle.diaglength = sqrt(obstacle.left_border * obstacle.left_border +
		obstacle.upper_border * obstacle.upper_border);

	// Combine flags
	obstacle.flags = 0;
	int *flags_array = flags.arr;
	for (i = 0; i < flags.size; i++)
		obstacle.flags |= flags_array[i];
	dynarray_free(&flags);

	obstacle.transparent = transparency;
	
	// Parse action
	obstacle.action_fn = get_action_by_name(obstacle.action);

	// Parse animation
	if (!animation &&
		(obstacle.filenames.size > 1 || obstacle.emitted_light_strength.size > 1)) {
		animation = strdup("obstacle");
	}
	obstacle.animation_fn = get_animation_by_name(animation);
	free(animation);

	if (groups.size > 0) {
		int obstacle_type = obstacle_map.size;
		for (i = 0; i < groups.size; i++) {
			char **group_name = dynarray_member(&groups, i, sizeof(char *));
			add_obstacle_to_group(*group_name, obstacle_type);
			free(*group_name);
		}
	}
	dynarray_free(&groups);

	dynarray_add(&obstacle_map, &obstacle, sizeof(obstacle_spec));
	return 0;
}

static int lua_leveleditor_obstacle_category_ctor(lua_State *L)
{
	char *category_name;
	struct dynarray obstacle_list;

	struct data_spec data_specs[] = {
		{ "name", NULL, STRING_TYPE, &category_name },
		{ "obstacles", NULL, STRING_ARRAY, &obstacle_list },
		{ NULL, NULL, 0, 0 }
	};

	fill_structure_from_table(L, data_specs);

	lvledit_set_obstacle_list_for_category(category_name, &obstacle_list);
	clean_structure(data_specs);
	return 0;
}

/*
 * Get one bullet from the top of the Lua stack
 */
static void get_one_bullet(lua_State *L, void *data)
{
	struct bulletspec *bullet = (struct bulletspec *)data;
	char *bullet_blast_type;

	struct data_spec data_specs[] = {
		{ "name",              NULL, STRING_TYPE, &(bullet->name)                     },
		{ "sound",             NULL, STRING_TYPE, &(bullet->sound)                    },
		{ "phases",            "1",  INT_TYPE,    &(bullet->phases)                   },
		{ "phases_per_second", "1",  DOUBLE_TYPE, &(bullet->phase_changes_per_second) },
		{ "blast_type",        NULL, STRING_TYPE, &bullet_blast_type                  },
		{ NULL, NULL, 0, 0 }
	};

	fill_structure_from_table(L, data_specs);

	bullet->blast_type = get_blast_type_by_name(bullet_blast_type);
	free(bullet_blast_type);
}

static int lua_bullet_list_ctor(lua_State *L)
{
	fill_dynarray_from_table(L, &bullet_specs, sizeof(struct bulletspec), get_one_bullet);

	return 0;
}

static int lua_blast_ctor(lua_State *L)
{
	static int blast_index = 0;
	blastspec* blast = &Blastmap[blast_index++];
	if (blast_index > ALLBLASTTYPES)
		error_message(__FUNCTION__, "Maximum number of blast types was exceeded.", PLEASE_INFORM | IS_FATAL);

	struct data_spec data_specs[] = {
		{ "name", NULL, STRING_TYPE, &blast->name },
		{ "animation_time", "1.0", FLOAT_TYPE, &blast->total_animation_time },
		{ "phases", 0, INT_TYPE, &blast->phases },
		{ "do_damage", "0", INT_TYPE, &blast->do_damage },
		{ "sound_file", NULL, STRING_TYPE, &blast->sound_file },
		{ NULL, NULL, 0, 0 }
	};

	fill_structure_from_table(L, data_specs);
	blast->images = MyMalloc(sizeof(struct image) * blast->phases);
	return 0;
}

static void get_floor_tile_list(lua_State *L, struct dynarray *floor_tiles)
{
	struct image empty_image = EMPTY_IMAGE;
	int i;

	// Init the dynarray, using the lua table's length
	dynarray_init(floor_tiles, lua_rawlen(L, -1), sizeof(struct floor_tile_spec));

	lua_pushnil(L);
	while (lua_next(L, -2)) {
		struct floor_tile_spec floor_tile = { .filenames = { NULL, 0, 0}, .animation_fn = NULL, .animation_fps = 0 };
		int ltype = lua_type(L, -1);
		if (ltype == LUA_TSTRING) {
			// Non-animated floor tile
			const char *filename = strdup(lua_tostring(L, -1));
			dynarray_init(&floor_tile.filenames, 1, sizeof(char *));
			dynarray_add(&floor_tile.filenames, &filename, sizeof(char *));

		} else if (ltype == LUA_TTABLE) {
			// Animated floor tile
			struct data_spec data_specs[] = {
				{ "filenames",     NULL,  STRING_ARRAY, &floor_tile.filenames     },
				{ "animation_fps", "0.0", FLOAT_TYPE,   &floor_tile.animation_fps },
				{ NULL, NULL, 0, 0 }
			};
			fill_structure_from_table(L, data_specs);
			floor_tile.animation_fn = get_animation_by_name("floor_tile");

		}
		lua_pop(L, 1);

		floor_tile.frames = floor_tile.filenames.size;
		floor_tile.images = MyMalloc(floor_tile.frames * sizeof(struct image));
		for (i=0; i<floor_tile.frames; i++)
			memcpy(&floor_tile.images[i], &empty_image, sizeof(struct image));
		floor_tile.current_image = &floor_tile.images[0];

		dynarray_add(floor_tiles, &floor_tile, sizeof(struct floor_tile_spec));
	}
}

static int lua_underlay_floor_tile_list_ctor(lua_State *L)
{
	get_floor_tile_list(L, &underlay_floor_tiles);
	if (underlay_floor_tiles.size >= MAX_UNDERLAY_FLOOR_TILES)
		error_message(__FUNCTION__, "Maximum number of underlay floor tile types has been exceeded.", PLEASE_INFORM | IS_FATAL);
	return 0;
}

static int lua_overlay_floor_tile_list_ctor(lua_State *L)
{
	get_floor_tile_list(L, &overlay_floor_tiles);
	if (overlay_floor_tiles.size >= MAX_OVERLAY_FLOOR_TILES)
		error_message(__FUNCTION__, "Maximum number of overlay floor tile types has been exceeded.", PLEASE_INFORM | IS_FATAL);
	return 0;
}

/**
 * Set obstacle flags as global lua values.
 */
static void init_obstacle_flags(void)
{
	const struct {
		const char *name;
		unsigned int value;
	} flags[] = {
		// Obstacle flags
		{ "IS_VERTICAL", IS_VERTICAL },
		{ "IS_HORIZONTAL", IS_HORIZONTAL },
		{ "IS_WALL", IS_WALL },
		{ "IS_SMASHABLE", IS_SMASHABLE },
		{ "BLOCKS_VISION", BLOCKS_VISION_TOO },
		{ "DROPS_RANDOM_TREASURE", DROPS_RANDOM_TREASURE },
		{ "NEEDS_PRE_PUT", NEEDS_PRE_PUT },
		{ "GROUND_LEVEL", GROUND_LEVEL },
		{ "IS_WALKABLE", IS_WALKABLE },
		{ "IS_CLICKABLE", IS_CLICKABLE },
		{ "IS_VOLATILE", IS_VOLATILE },
		{ "CORNER_NE", CORNER_NE },
		{ "CORNER_NW", CORNER_NW },
		{ "CORNER_SE", CORNER_SE },
		{ "CORNER_SW", CORNER_SW },
		// Obstacle transparency
		{ "NO_TRANSPARENCY", TRANSPARENCY_NONE },
		{ "WALLS_TRANSPARENCY", TRANSPARENCY_FOR_WALLS }
	};

	int i;
	for (i = 0; i < sizeof(flags) / sizeof(flags[0]); i++) {
		lua_pushinteger(config_lua_state, flags[i].value);
		lua_setglobal(config_lua_state, flags[i].name);
	}
}

/**
 * \brief Read a npc table. Called when a npc list is read in a Lua config file.
 * \param L Lua state.
 */
static int lua_npc_list_ctor(lua_State *L)
{
	if (lua_type(L, -1) == LUA_TTABLE) {
		lua_pushnil(L);
		// Process the lua table
		while (lua_next(L, -2) != 0) {
			const char *npc_name;
			if (lua_type(L, -2) == LUA_TNUMBER && lua_type(L, -1) == LUA_TSTRING) {
				npc_name = lua_tostring(L, -1);
				npc_add(npc_name);
			}
			lua_pop(L, 1);
		}
	} // Not default value because list have been intialized as empty

	return 0;
}

/**
 * \brief Npc_shop constructor. Called when a 'npc_shop' object is read in a Lua config file.
 * \param L Lua state.
 */
static int lua_npc_shop_ctor(lua_State *L)
{
	char *name;
	const char *item_name;
	int item_weight;

	// Get the name of npc
	get_value_from_table(L, "name", STRING_TYPE, &name);

	lua_getfield(L, -1, "items");

	// Process the items table
	if (lua_type(L, -1) == LUA_TTABLE) {
		lua_pushnil(L);

		while (lua_next(L, -2) != 0) {
			if (lua_type(L, -1) == LUA_TTABLE) {
				// get the name of item
				lua_rawgeti(L, -1, 1);
				if (lua_type(L, -1) == LUA_TSTRING)
					item_name = lua_tostring(L, -1);
				else
					item_name = NULL;
				lua_pop(L, 1);

				// get the weight of item
				lua_rawgeti(L, -1, 2);
				if (lua_type(L, -1) == LUA_TNUMBER)
					item_weight = lua_tointeger(L, -1);
				else
					item_weight = 1;
				lua_pop(L, 1);

				// add item to the shoplist of the npc
				npc_add_shoplist(name, item_name, item_weight);
			}
			lua_pop(L, 1);
		}
	}

	lua_pop(L, 1);
	free(name);
	return 0;
}

/**
 * \brief 
 * \param L Lua state.
 */
static void get_one_item(lua_State *L, void *data)
{
	struct itemspec *item = (struct itemspec *)data;
	char *item_slot;
	char *item_durability;
	char *item_drop_class;
	char *item_armor_class;
	char *item_right_use_busy_type;
	char *item_dropped;
	char *item_damage;
	char *item_motion_class;
	char *item_bullet_type;

	struct data_spec data_specs[] = {
		{"id", 						NULL,		STRING_TYPE, &item->id								},
		{"name", 					NULL,		STRING_TYPE, &item->name							},
		{"slot",					"none",		STRING_TYPE, &item_slot								},
		{"weapon.damage",			NULL,		STRING_TYPE, &item_damage							},
		{"weapon.attack_time",		"0",		FLOAT_TYPE,	 &item->item_gun_recharging_time		},
		{"weapon.reloading_time",	"0",		FLOAT_TYPE,	 &item->item_gun_reloading_time			},
		{"weapon.bullet.type", 		"NO BULLET IMAGE",	STRING_TYPE, 	&item_bullet_type			},
		{"weapon.bullet.speed",	 	"0",		FLOAT_TYPE,	 &item->item_gun_speed					},
		{"weapon.bullet.lifetime",	"0",		FLOAT_TYPE,	 &item->item_gun_bullet_lifetime		},
		{"weapon.bullet.angle",	 	"0",		FLOAT_TYPE,	 &item->item_gun_start_angle_modifier	},
		{"weapon.ammunition.id", 	NULL, 		STRING_TYPE, &item->ammo_id							},
		{"weapon.ammunition.clip",	"0",		INT_TYPE, 	 &item->item_gun_ammo_clip_size			},
		{"weapon.melee",			"false",	BOOL_TYPE,	 &item->item_weapon_is_melee			},
		{"weapon.two_hand",	 		"false",	BOOL_TYPE,	 &item->item_gun_requires_both_hands	},
		{"weapon.motion_class",		NULL,		STRING_TYPE, &item_motion_class						},
		{"armor_class",				NULL,		STRING_TYPE, &item_armor_class						},
		{"right_use.tooltip",		NULL,		STRING_TYPE, &item->right_use.tooltip				},
		{"right_use.skill",			NULL,		STRING_TYPE, &item->right_use.skill					},
		{"right_use.add_skill",		NULL,		STRING_TYPE, &item->right_use.add_skill				},
		{"right_use.busy.type",		NULL,		STRING_TYPE, &item_right_use_busy_type				},
		{"right_use.busy.duration",	"0",		INT_TYPE,	 &item->right_use.busy_time				},
		{"requirements.strength", 	"-1",		SHORT_TYPE,  &item->item_require_strength			},
		{"requirements.dexterity",	"-1",		SHORT_TYPE,	 &item->item_require_dexterity			},
		{"requirements.cooling",	"-1",		SHORT_TYPE,	 &item->item_require_cooling			},
		{"inventory.x",	 			"1",		INT_TYPE, 	 &item->inv_size.x						},
		{"inventory.y",				"1",		INT_TYPE,	 &item->inv_size.y						},
		{"inventory.stackable",		"false",	BOOL_TYPE,	 &item->item_group_together_in_inventory},
		{"inventory.image",			NULL,		STRING_TYPE, &item->item_inv_file_name				},
		{"base_price",				"-1",		INT_TYPE,	 &item->base_list_price					},
		{"drop.class", 				NULL,   	STRING_TYPE, &item_drop_class						},
		{"drop.number",				NULL,		STRING_TYPE, &item_dropped							},
		{"drop.sound",				NULL, 		STRING_TYPE, &item->item_drop_sound_file_name		},
		{"durability",				NULL, 		STRING_TYPE, &item_durability						},
		{"description",				"", 		STRING_TYPE, &item->item_description				},
		{"tux_part",				NULL, 		STRING_TYPE, &item->tux_part_instance				},
		{"rotation_series",			NULL, 		STRING_TYPE, &item->item_rotation_series_prefix		},
		{ NULL, NULL, 0, 0 }
	};

	fill_structure_from_table(L, data_specs);

	if (!item->id) {
		error_message(__FUNCTION__, "No id for item. The id must be given.", PLEASE_INFORM);
	}

	// Set the item slot
	item->slot = get_slot_type_by_name(item_slot);
	free(item_slot);

	// Set the durability
	get_range_from_string(item_durability, &item->base_item_durability, &item->item_durability_modifier, -1);
	item->item_durability_modifier -= item->base_item_durability;
	free(item_durability);
	
	// Set armor class
	get_range_from_string(item_armor_class, (int *)&item->base_armor_class, &item->armor_class_modifier, 0);
	item->armor_class_modifier -= item->base_armor_class;
	free(item_armor_class);

	// Set busy type
	if (item_right_use_busy_type) {
		item->right_use.busy_type = get_busy_type_by_name(item_right_use_busy_type);
	} else {
		item->right_use.busy_type = NONE;
	}
	free(item_right_use_busy_type);

	// Set the drop class
	if (item_drop_class) {
		get_range_from_string(item_drop_class, &item->min_drop_class, &item->max_drop_class, -1);
		free(item_drop_class);
	} else {
		item->min_drop_class = item->max_drop_class = -1;
	}
	
	if (item->min_drop_class != -1) {
		// Increment the item count per drop class
		int cc;
		for (cc = 0; cc < 10; cc++) {
			if (cc > item->max_drop_class)
				break;
			if (cc < item->min_drop_class)
				continue;
			else
				item_count_per_class[cc]++;
		}
		
		// Set the number of dropped item
		get_range_from_string(item_dropped, &item->drop_amount, &item->drop_amount_max, 1);
	}
	free(item_dropped);
	
	// Set damage
	get_range_from_string(item_damage, (int *)&item->base_item_gun_damage, (int *)&item->item_gun_damage_modifier, 0);
	item->item_gun_damage_modifier -= item->base_item_gun_damage;
	free(item_damage);
			
	// Set motion class
	item->motion_class = get_motion_class_id_by_name(item_motion_class);
	free(item_motion_class);

	item->item_gun_bullet_pass_through_hit_bodies = FALSE;
	
	if (item->slot == WEAPON_SLOT) {
		// Set bullet image type
		item->item_gun_bullet_image_type = GetBulletByName(item_bullet_type);
	}
	free(item_bullet_type);
}

static int lua_item_list_ctor(lua_State *L)
{
	struct dynarray item_specs = { 0 };

	fill_dynarray_from_table(L, &item_specs, sizeof(struct itemspec), get_one_item);

	// Copy the array of item_specs
	Number_Of_Item_Types = item_specs.size;
	ItemMap = (itemspec *) MyMalloc(sizeof(itemspec) * Number_Of_Item_Types + 1);
	memcpy(ItemMap, item_specs.arr, sizeof(itemspec) * Number_Of_Item_Types);

	dynarray_free(&item_specs);

	return 0;
}

static void get_one_lang(lua_State *L, void *data)
{
	struct langspec *lang = (struct langspec *)data;

	struct data_spec data_specs[] = {
		{ "name",   NULL, STRING_TYPE, &(lang->name)   },
		{ "locale", NULL, STRING_TYPE, &(lang->locale) },
		{ NULL, NULL, 0, 0 }
	};

	fill_structure_from_table(L, data_specs);
}

static int lua_languages_ctor(lua_State *L)
{
	fill_dynarray_from_table(L, &lang_specs, sizeof(struct langspec), get_one_lang);

	return 0;
}

static int lua_title_screen_ctor(lua_State *L)
{
	struct title_screen *title = (struct title_screen *)lua_touserdata(L, lua_upvalueindex(1));


	// Specification of the data structure to retrieve from the lua table
	struct data_spec data_specs[] = {
		{ "background", "title.jpg", STRING_TYPE, &title->background },
		{ "song",       NULL,        STRING_TYPE, &title->song       },
		{ "text",       NULL,        STRING_TYPE, &title->text       },
		{ NULL, NULL, 0, 0 }
	};

	fill_structure_from_table(L, data_specs);

	return 0;
}

/**
 * Add lua constructors of new data types
 */
void init_luaconfig()
{
	int i;
	lua_State *L = get_lua_state(LUA_CONFIG);

	// Register standard C functions

	luaL_Reg lfuncs[] = {
		{ "addon",                         lua_register_addon                     },
		{ "tux_animation",                 lua_tuxanimation_ctor                  },
		{ "tux_rendering_config",          lua_tuxrendering_config_ctor           },
		{ "tux_ordering",                  lua_tuxordering_ctor                   },
		{ "obstacle",                      lua_obstacle_ctor                      },
		{ "leveleditor_obstacle_category", lua_leveleditor_obstacle_category_ctor },
		{ "bullet_list",                   lua_bullet_list_ctor                   },
		{ "blast",                         lua_blast_ctor                         },
		{ "underlay_floor_tile_list",      lua_underlay_floor_tile_list_ctor      },
		{ "overlay_floor_tile_list",       lua_overlay_floor_tile_list_ctor       },
		{ "npc_list",                      lua_npc_list_ctor                      },
		{ "npc_shop",                      lua_npc_shop_ctor                      },
		{ "item_list",                     lua_item_list_ctor                     },
		{ "languages",                     lua_languages_ctor                     },
		{NULL, NULL}
	};

	for (i = 0; lfuncs[i].name != NULL; i++) {
		lua_pushcfunction(L, lfuncs[i].func);
		lua_setglobal(L, lfuncs[i].name);
	}

	// Register C closures

	luaL_Reg lclos[] = {
		{ "title_screen", lua_title_screen_ctor },
		{NULL, NULL}
	};

	for (i = 0; lclos[i].name != NULL; i++) {
		 // Initialize a pointer upvalue with a NULL content
		lua_pushlightuserdata(L, NULL);
		lua_pushcclosure(L, lclos[i].func, 1);
		lua_setglobal(L, lclos[i].name);
	}

	// Additional initializations

	init_obstacle_flags();
}
