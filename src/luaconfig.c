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
 * Set the value of a data from a field of a Lua table.
 *
 * \param L Lua state.
 * \param index Stack index where the table is.
 * \param field Name of the field to fetch.
 * \param type Type of the data to read
 * \param result Location of the data to set
 *
 * \return TRUE if the value was read, FALSE if it could not be read.
 */
static int set_value_from_table(lua_State *L, int index, const char *field, enum data_type type, void *result)
{
	int found_and_valid = FALSE;
	int ltype;

	lua_getfield(L, index, field);
	ltype = lua_type(L, -1);

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
			dynarray_init((struct dynarray *)result, lua_objlen(L, -1), sizeof(int));
			lua_pushnil(L);
			// Fill the dynarray with the content of the lua table
			while (lua_next(L, -2) != 0) {
				if (lua_type(L, -2) == LUA_TNUMBER && lua_type(L, -1) == LUA_TNUMBER) {
					int value = (int)lua_tonumber(L, -1);
					dynarray_add((struct dynarray *)result, &value, sizeof(int));
					found_and_valid = TRUE;
				}
				lua_pop(L, 1);
			}
		} else {
			// The data is not in the lua table: initialize the dynarray
			// with 1 empty slot (to possibly receive a single lua value
			// or a default value)
			dynarray_init((struct dynarray *)result, 1, sizeof(int));
			// If the data is a single value fill the dynarray slot
			if (ltype == LUA_TNUMBER) {
				int value = lua_tointeger(L, -1);
				dynarray_add((struct dynarray *)result, &value, sizeof(int));
				found_and_valid = TRUE;
			}
		}
		break;
	case FLOAT_ARRAY:
		if (ltype == LUA_TTABLE) {
			// Init a dynarray, using the lua table's length
			dynarray_init((struct dynarray *)result, lua_objlen(L, -1), sizeof(float));
			lua_pushnil(L);
			// Fill the dynarray with the content of the lua table
			while (lua_next(L, -2) != 0) {
				if (lua_type(L, -2) == LUA_TNUMBER && lua_type(L, -1) == LUA_TNUMBER) {
					float value = (float)lua_tonumber(L, -1);
					dynarray_add((struct dynarray *)result, &value, sizeof(float));
					found_and_valid = TRUE;
				}
				lua_pop(L, 1);
			}
		} else {
			// The data is not in the lua table: initialize the dynarray
			// with 1 empty slot (to possibly receive a single lua value
			// or a default value)
			dynarray_init((struct dynarray *)result, 1, sizeof(float));
			// If the data is a single value fill the dynarray slot
			if (ltype == LUA_TNUMBER) {
				float value = (float)lua_tonumber(L, -1);
				dynarray_add((struct dynarray *)result, &value, sizeof(float));
				found_and_valid = TRUE;
			}
		}
		break;
	case STRING_ARRAY:
		if (ltype == LUA_TTABLE) {
			// Init a dynarray, using the lua table's length
			dynarray_init((struct dynarray *)result, lua_objlen(L, -1), sizeof(char *));
			lua_pushnil(L);
			// Fill the dynarray with the content of the lua table
			while (lua_next(L, -2) != 0) {
				if (lua_type(L, -2) == LUA_TNUMBER && lua_type(L, -1) == LUA_TSTRING) {
					char *value = strdup(lua_tostring(L, -1));
					dynarray_add((struct dynarray *)result, &value, sizeof(char *));
					found_and_valid = TRUE;
				}
				lua_pop(L, 1);
			}
		} else {
			// The data is not in the lua table: initialize the dynarray
			// with 1 empty slot (to possibly receive a single lua value
			// or a default value)
			dynarray_init((struct dynarray *)result, 1, sizeof(char *));
			// If the data is a single value fill the dynarray slot
			if (ltype == LUA_TSTRING) {
				char *value = strdup(lua_tostring(L, -1));
				dynarray_add((struct dynarray *)result, &value, sizeof(char *));
				found_and_valid = TRUE;
			}
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
 *
 * \param default_value Default value, encoded in a string.
 * \param type Type of the data to read
 * \param data Location of the data to set
 */
static void set_value_from_default(const char *default_value, enum data_type type, void *data)
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
			dynarray_add((struct dynarray *)data, &int_value, sizeof(int));
		}
		break;
	case FLOAT_ARRAY:
		// Add the default value to the dynarray
		{
			float float_value = atof(default_value);
			dynarray_add((struct dynarray *)data, &float_value, sizeof(float));
		}
		break;
	case STRING_ARRAY:
		// Add the default value to the dynarray
		if (default_value != NULL) {
			dynarray_add((struct dynarray *)data, strdup(default_value), sizeof(char *));
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
 * Extract data structure content from a table on the Lua stack
 *
 * \param L Lua state.
 * \param data_specs An array of data_spec, defining the values to retrieve, and how to retrieve them
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
			}
			lua_pop(L, 1);
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

/*
 * Tux_rendering_config constructor. Called when a 'tux_rendering_config object' is read in a Lua config file.
 */
static int lua_tuxrendering_config_ctor(lua_State *L)
{
	// Specification of the data structure to retrieve from the lua table
	struct data_spec data_specs[] = {
		{ "motion_class_names",     NULL,            STRING_ARRAY, &tux_rendering.motion_class_names },
		{ "head_part_default",      "iso_head",      STRING_TYPE,  &tux_rendering.default_instances.head },
		{ "torso_part_default",     "iso_torso",     STRING_TYPE,  &tux_rendering.default_instances.torso },
		{ "weaponarm_part_default", "iso_weaponarm", STRING_TYPE,  &tux_rendering.default_instances.weaponarm },
		{ "weapon_part_default",    NULL,            STRING_TYPE,  &tux_rendering.default_instances.weapon },
		{ "shieldarm_part_default", "iso_shieldarm", STRING_TYPE,  &tux_rendering.default_instances.shieldarm },
		{ "feet_part_default",      "iso_feet",      STRING_TYPE,  &tux_rendering.default_instances.feet },
		{ "gun_muzzle_height",      "-60",           INT_TYPE,     &tux_rendering.gun_muzzle_height },
		{ NULL, NULL, 0, 0 }
	};

	set_structure_from_table(L, data_specs);

	// At least one motion class needs to be defined
	if (tux_rendering.motion_class_names.size < 1) {
		ErrorMessage(__FUNCTION__,
			"Tux rendering specification is invalid: at least one motion_class is needed",
			PLEASE_INFORM, IS_FATAL);
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

	set_structure_from_table(L, data_specs);

	// Check that the motion class is defined (and so that a data structure is
	// ready to store the retrieved rendering orders)
	int motion_class_id = get_motion_class_id_by_name(type);
	if (motion_class_id == -1) {
		ErrorMessage(__FUNCTION__,
				"Invalid tux_ordering spec:\n"
				"Unknown motion_class (%s)",
				PLEASE_INFORM, IS_FATAL, type);
	}

	// Fill the rendering order data structure according to what was retrieved
	// from the lua table
	int i;
	for (i = 0; i < rotations.size; i++) {

		// Check validity of the rotation index
		int rotation_idx = ((int *)rotations.arr)[i];
		if (rotation_idx < 0 || rotation_idx >= MAX_TUX_DIRECTIONS) {
			ErrorMessage(__FUNCTION__,
					"Invalid tux_ordering spec (motion_class: %s):\n"
					"rotation index (%d) must be between 0 and %d",
					PLEASE_INFORM, IS_FATAL, type, rotation_idx, MAX_TUX_DIRECTIONS - 1);
		}

		// Check validity of the phase values
		if (phase_start < 0 || phase_start >= TUX_TOTAL_PHASES) {
			ErrorMessage(__FUNCTION__,
					"Invalid tux_ordering spec (motion_class: %s - rotation %d):\n"
					"phase_start (%d) must be between 0 and %d",
					PLEASE_INFORM, IS_FATAL, type, rotation_idx, phase_start, TUX_TOTAL_PHASES - 1);
		}
		if (phase_end < -1 || phase_end >= TUX_TOTAL_PHASES) {
			ErrorMessage(__FUNCTION__,
					"Invalid tux_ordering spec (motion_class: %s - rotation %d):\n"
					"phase_end (%d) must be between -1 and %d",
					PLEASE_INFORM, IS_FATAL, type, rotation_idx, phase_end, TUX_TOTAL_PHASES - 1);
		}
		if ((phase_end != -1) && (phase_end < phase_start)) {
			ErrorMessage(__FUNCTION__,
					"Invalid tux_ordering spec (motion_class: %s - rotation %d):\n"
					"phase_start value (%d) must be lower or equal to phase_end value (%d)",
					PLEASE_INFORM, IS_FATAL, type, rotation_idx, phase_start, phase_end);
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
	char *action;
	struct dynarray groups;

	char default_transparency[20];
	sprintf(default_transparency, "%d", TRANSPARENCY_FOR_WALLS);

	struct data_spec data_specs[] = {
		{ "label", NULL, STRING_TYPE, &obstacle.label },
		{ "borders", "0", FLOAT_ARRAY, &borders },
		{ "flags", "0", INT_ARRAY, &flags },
		{ "after_smashing", "-1", INT_TYPE, &obstacle.result_type_after_smashing_once },
		{ "emitted_light_strength", "0", INT_ARRAY, &obstacle.emitted_light_strength },
		{ "transparency", default_transparency, INT_TYPE, &transparency },
		{ "action", NULL, STRING_TYPE, &action },
		{ "animation", NULL, STRING_TYPE, &animation },
		{ "animation_fps", "10", FLOAT_TYPE, &obstacle.animation_fps },
		{ "smashed_sound", NULL, STRING_TYPE, &obstacle.smashed_sound },
		{ "groups", NULL, STRING_ARRAY, &groups },
		{ NULL, NULL, 0, 0 }
	};

	if (!set_value_from_table(L, 1, "image_filenames", STRING_ARRAY, &obstacle.filenames)) {
			ErrorMessage(__FUNCTION__, "No image filename for obstacle. At least one image filename must be given.",
				PLEASE_INFORM, IS_FATAL);
	}

	set_structure_from_table(L, data_specs);

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
	obstacle.action_fn = get_action_by_name(action);
	free(action);

	// Parse animation
	obstacle.animation_fn = get_animation_by_name(animation);
	free(animation);

	if (obstacle.filenames.size > 1 || obstacle.emitted_light_strength.size > 1)
		obstacle.animation_fn = animate_obstacle;

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

	set_structure_from_table(L, data_specs);

	lvledit_set_obstacle_list_for_category(category_name, &obstacle_list);
	clean_structure(data_specs);
	return 0;
}

static int lua_blast_ctor(lua_State *L)
{
	static int blast_index = 0;
	blastspec* blast = &Blastmap[blast_index++];
	if (blast_index > ALLBLASTTYPES)
		ErrorMessage(__FUNCTION__, "Maximum number of blast types was exceeded.", PLEASE_INFORM, IS_FATAL);

	struct data_spec data_specs[] = {
		{ "name", NULL, STRING_TYPE, &blast->name },
		{ "animation_time", "1.0", FLOAT_TYPE, &blast->total_animation_time },
		{ "phases", 0, INT_TYPE, &blast->phases },
		{ "do_damage", "0", INT_TYPE, &blast->do_damage },
		{ "sound_file", NULL, STRING_TYPE, &blast->sound_file },
		{ NULL, NULL, 0, 0 }
	};

	set_structure_from_table(L, data_specs);
	blast->images = MyMalloc(sizeof(struct image) * blast->phases);
	return 0;
}

static void get_floor_tile_list(lua_State *L, struct dynarray *filenames, struct dynarray *images)
{
	struct image empty_image = EMPTY_IMAGE;

	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		if (lua_type(L, -1) == LUA_TSTRING) {
			const char *filename = strdup(lua_tostring(L, -1));
			dynarray_add(filenames, &filename, sizeof(char *));
			dynarray_add(images, &empty_image, sizeof(empty_image));
		}
		lua_pop(L, 1);
	}
	lua_pop(L, 1);
}

static int lua_underlay_floor_tile_list_ctor(lua_State *L)
{
	get_floor_tile_list(L, &underlay_floor_tile_filenames, &underlay_floor_images);
	if (underlay_floor_tile_filenames.size >= MAX_UNDERLAY_FLOOR_TILES)
		ErrorMessage(__FUNCTION__, "Maximum number of underlay floor tile types has been exceeded.", PLEASE_INFORM, IS_FATAL);
	return 0;
}

static int lua_overlay_floor_tile_list_ctor(lua_State *L)
{
	get_floor_tile_list(L, &overlay_floor_tile_filenames, &overlay_floor_images);
	if (overlay_floor_tile_filenames.size >= MAX_OVERLAY_FLOOR_TILES)
		ErrorMessage(__FUNCTION__, "Maximum number of overlay floor tile types has been exceeded.", PLEASE_INFORM, IS_FATAL);
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
	if (lua_type(L, 1) == LUA_TTABLE) {
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
	set_value_from_table(L, 1, "name", STRING_TYPE, &name);

	lua_getfield(L, 1, "items");

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
 * Add lua constructors of new data types
 */

void init_luaconfig()
{
	int i;

	luaL_reg lfuncs[] = {
		{"addon", lua_register_addon},
		{"tux_animation", lua_tuxanimation_ctor},
		{"tux_rendering_config", lua_tuxrendering_config_ctor},
		{"tux_ordering", lua_tuxordering_ctor},
		{"obstacle", lua_obstacle_ctor},
		{"leveleditor_obstacle_category", lua_leveleditor_obstacle_category_ctor},
		{"blast", lua_blast_ctor},
		{"underlay_floor_tile_list", lua_underlay_floor_tile_list_ctor},
		{"overlay_floor_tile_list", lua_overlay_floor_tile_list_ctor},
		{"npc_list", lua_npc_list_ctor},
		{"npc_shop", lua_npc_shop_ctor},
		{NULL, NULL}
	};

	lua_State *L = get_lua_state(LUA_CONFIG);

	for (i = 0; lfuncs[i].name != NULL; i++) {
		lua_pushcfunction(L, lfuncs[i].func);
		lua_setglobal(L, lfuncs[i].name);
	}

	init_obstacle_flags();
}
