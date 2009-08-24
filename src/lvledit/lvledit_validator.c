/* 
 *
 *   Copyright (c) 2008-2009 Samuel Degrande
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
 * This file contains all functions to validate level maps.
 * Used by the level editor.
 */

#define _leveleditor_validator_c

#include "lvledit/lvledit_validator.h"
#include "lang.h"

#define IS_CHEST(t)  ( (t) >= ISO_H_CHEST_CLOSED && (t) <= ISO_V_CHEST_OPEN )
#define IS_BARREL(t) ( (t) >= ISO_BARREL_1       && (t) <= ISO_BARREL_4     )

static char *bigline = "====================================================================";
static char *line = "--------------------------------------------------------------------";
static char *sepline = "+------------------------------";

static int lvlval_chest_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx);
static void *lvlval_chest_parse_excpt(char *string);
static int lvlval_chest_cmp_data(void *opaque_data1, void *opaque_data2);

static int lvlval_waypoint_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx);
static void *lvlval_waypoint_parse_excpt(char *string);
static int lvlval_waypoint_cmp_data(void *opaque_data1, void *opaque_data2);

static int lvlval_interface_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx);
static void *lvlval_interface_parse_excpt(char *string);
static int lvlval_interface_cmp_data(void *opaque_data1, void *opaque_data2);

static int lvlval_jumptarget_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx);
static void *lvlval_jumptarget_parse_excpt(char *string);
static int lvlval_jumptarget_cmp_data(void *opaque_data1, void *opaque_data2);

struct level_validator level_validators[] = {
	{'C',
	 LIST_HEAD_INIT(level_validators[0].excpt_list),
	 lvlval_chest_execute,
	 lvlval_chest_parse_excpt,
	 lvlval_chest_cmp_data},
	{'W',
	 LIST_HEAD_INIT(level_validators[1].excpt_list),
	 lvlval_waypoint_execute,
	 lvlval_waypoint_parse_excpt,
	 lvlval_waypoint_cmp_data},
	{'J',
	 LIST_HEAD_INIT(level_validators[2].excpt_list),
	 lvlval_jumptarget_execute,
	 lvlval_jumptarget_parse_excpt,
	 lvlval_jumptarget_cmp_data},
	{'I',
	 LIST_HEAD_INIT(level_validators[3].excpt_list),
	 lvlval_interface_execute,
	 lvlval_interface_parse_excpt,
	 lvlval_interface_cmp_data},
	{.initial = '\0'}
};

//===========================================================
// Helper Functions
//===========================================================

/**
 * Ouput to the console the validator's title and associated comment
 */

static void validator_print_header(struct lvlval_ctx *val_ctx, char *title, char *comment)
{
	int cpt = 0;
	char *ptr = comment;

	putchar('\n');
	puts(bigline);
	printf("| %s - Level %d\n", title, val_ctx->this_level->levelnum);
	puts(sepline);

	printf("| ");
	// Split the text at the first whitespace after the 60th character
	while (*ptr) {
		if (*ptr == '\n') {
			printf("\n| ");
			cpt = 0;
			++ptr;
			continue;
		}
		if (cpt < 60) {
			putchar(*ptr);
			++cpt;
			++ptr;
			continue;
		}
		if (*ptr == ' ') {
			printf("\n| ");
			cpt = 0;
			++ptr;
			continue;
		} else {
			putchar(*ptr);
			++ptr;
		}		// continue until a whitespace is found
	}
	putchar('\n');
	puts(line);
}

/**
 * Check if the connection between two waypoints is valid
 * This is an helper function for waypoint_validator()
 */

enum connect_validity {
	DIRECT_CONN = 0,
	NEED_PATH = 1,
	NO_PATH = 2,
	COMPLEX_PATH = 4
};

static enum connect_validity waypoints_connection_valid(gps * from_pos, gps * to_pos)
{
	if (DirectLineColldet(from_pos->x, from_pos->y, to_pos->x, to_pos->y, from_pos->z, &WalkablePassFilter))
		return DIRECT_CONN;

	moderately_finepoint mfp_to_pos = { to_pos->x, to_pos->y };
	moderately_finepoint mid_pos[40];

	pathfinder_context pf_ctx = { &WalkablePassFilter, NULL };

	int path_found = set_up_intermediate_course_between_positions(from_pos, &mfp_to_pos, mid_pos, 40, &pf_ctx);
	if (!path_found)
		return NO_PATH;

	int nb_mp = 0;
	while (mid_pos[nb_mp++].x != -1) ;
	if (nb_mp > 5)
		return (NO_PATH & COMPLEX_PATH);

	return NEED_PATH;
}

//===========================================================
// Exception Lists Handling
//===========================================================

/*
 * Parse the exceptions list
 * ( used by load_excpt_lists() )
 */

static void get_excpt_list(char *section_pointer)
{
	char *ptr = section_pointer;

	while ((ptr = strstr(ptr, "Rule=")) != NULL) {
		// null-terminate the line

		char *end_string = ptr;
		while ((*end_string != '\0') && (*end_string != '\n') && (*end_string != '\r'))
			++end_string;
		char tmp = *end_string;
		*end_string = '\0';

		// Find the sub-validator specified in the exception

		char *validator_type = ReadAndMallocStringFromData(ptr, "Type=\"", "\"");
		if (!validator_type) {
			ErrorMessage(__FUNCTION__, "The Validator's Type of an exception was not found!\n", PLEASE_INFORM, IS_FATAL);
			return;
		}

		int v = 0;
		struct level_validator *one_validator;
		while (one_validator = &(level_validators[v++]), one_validator->initial != '\0') {
			if (one_validator->initial != validator_type[0])
				continue;
			if (one_validator->parse_excpt == NULL)
				continue;

			// Call sub-validator's parse fonction

			struct lvlval_excpt_item *item = (struct lvlval_excpt_item *)malloc(sizeof(struct lvlval_excpt_item));
			item->caught = FALSE;
			ReadValueFromString(ptr, "Rule=", "%d", &(item->rule_id), NULL);

			item->opaque_data = one_validator->parse_excpt(ptr);

			// Add the retrieved data to the sub-validator's exceptions list

			if (item->opaque_data == NULL)
				free(item);
			else
				list_add(&(item->node), &(one_validator->excpt_list));

			break;
		}

		free(validator_type);

		if (one_validator->initial == '\0') {	// Sub-validator was not found
			ErrorMessage(__FUNCTION__, "The Validator's Type specified in an exception does not exist!\n", PLEASE_INFORM,
				     IS_FATAL);
			return;
		}
		// Restore input buffer

		if (tmp == '\0')
			break;	// We are at the end of the section

		*end_string = tmp;
		ptr = end_string + 1;
	}
}

/*
 * Load the exceptions list from the configuration file
 */

static void load_excpt_lists(char *filename)
{
	char fpath[2048];
	char *main_file_pointer;
	char *section_pointer;

#	define START_OF_DATA_STRING   "*** Beginning of LevelValidator Exceptions List ***"
#	define END_OF_DATA_STRING     "*** End of LevelValidator Exceptions List ***"

	// Read whole file in memory    
	find_file(filename, MAP_DIR, fpath, 0);
	main_file_pointer = ReadAndMallocAndTerminateFile(fpath, END_OF_DATA_STRING);

	// Search beginning of list 
	section_pointer = strstr(main_file_pointer, START_OF_DATA_STRING);
	if (section_pointer == NULL) {
		ErrorMessage(__FUNCTION__, "Start of exceptions list not found!\n", PLEASE_INFORM, IS_FATAL);
		return;
	}
	// Parse the list
	get_excpt_list(section_pointer);

	free(main_file_pointer);

#	undef START_OF_DATA_STRING
#	undef END_OF_DATA_STRING
}

/*
 * This function will compare one set of data to all the exceptions of a validator
 * Return FALSE if the data was not found in the list of exceptions
 */

static int lookup_exception(struct level_validator *this, void *opaque_data)
{
	if (this->cmp == NULL)
		return FALSE;

	struct lvlval_excpt_item *item;
	int rtn;

	// Loop on each item in the list, and call the validator's comparator

	list_for_each_entry(item, &(this->excpt_list), node) {
		rtn = this->cmp(item->opaque_data, opaque_data);
		if (rtn) {
			item->caught = TRUE;	// Mark the exception has caught
			return TRUE;
		}
	}

	return FALSE;
}

/*
 * This function prints all uncaught exceptions of a validator
 * Ouput : TRUE if there were uncaught exceptions
 */

static int print_uncaught_exceptions()
{
	int v = 0;
	struct level_validator *one_validator;
	int rtn = FALSE;
	int first = TRUE;

	// Loop on each validator

	while (one_validator = &(level_validators[v++]), one_validator->initial != '\0') {
		// Loop on each item in the list, and check 'caught' value
		struct lvlval_excpt_item *item;

		list_for_each_entry(item, &(one_validator->excpt_list), node) {
			if (!item->caught) {
				if (first) {
					putchar('\n');
					puts(bigline);
					printf("During execution of the validator, the following exception rules were not caught:\n");
					first = FALSE;
				}
				printf(" %d", item->rule_id);
				rtn = TRUE;
			}
		}
	}

	if (!first)
		putchar('\n');

	return rtn;
}

/*
 * Free all the allocated memory
 */

static void free_exception_lists()
{
	int v = 0;
	struct level_validator *one_validator;
	struct lvlval_excpt_item *item;
	struct lvlval_excpt_item *next;

	while (one_validator = &(level_validators[v++]), one_validator->initial != '\0') {
		list_for_each_entry_safe(item, next, &(one_validator->excpt_list), node) {
			free(item->opaque_data);
			list_del(&(item->node));
			free(item);
		}
	}
}

//===========================================================
// Chest Reachable Validator
//
// This validator checks if the activable objects (chests, barrels, 
// crates) are reachable
//===========================================================

struct chest_excpt_data {
	int obj_id;
	gps obj_pos;
};

/*
 * Parse "chest" exception
 */

static void *lvlval_chest_parse_excpt(char *string)
{
	struct chest_excpt_data *data = (struct chest_excpt_data *)malloc(sizeof(struct chest_excpt_data));

	ReadValueFromString(string, "Idx=", "%d", &(data->obj_id), NULL);
	ReadValueFromString(string, "X=", "%f", &(data->obj_pos.x), NULL);
	ReadValueFromString(string, "Y=", "%f", &(data->obj_pos.y), NULL);
	ReadValueFromString(string, "L=", "%d", &(data->obj_pos.z), NULL);

	return (data);
}

/*
 * Compare two "chest" exception data structures
 */

static int lvlval_chest_cmp_data(void *opaque_data1, void *opaque_data2)
{
#	define DIST_EPSILON 0.01f

	struct chest_excpt_data *data1 = opaque_data1;
	struct chest_excpt_data *data2 = opaque_data2;

	if (data1->obj_id != data2->obj_id)
		return FALSE;
	if (data1->obj_pos.z != data2->obj_pos.z)
		return FALSE;

	float dist = calc_euklid_distance(data1->obj_pos.x, data1->obj_pos.y, data2->obj_pos.x, data2->obj_pos.y);
	if (dist > DIST_EPSILON)
		return FALSE;

	return TRUE;

#	undef DIST_EPSILON
}

/*
 * "chest" validator
 */

static int lvlval_chest_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx)
{
	int x_tile, y_tile, glue_index;
	int is_invalid = FALSE;

	for (y_tile = 0; y_tile < validator_ctx->this_level->ylen; ++y_tile) {
		for (x_tile = 0; x_tile < validator_ctx->this_level->xlen; ++x_tile) {
			for (glue_index = 0; glue_index < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE; ++glue_index) {
				int obs_index = validator_ctx->this_level->map[y_tile][x_tile].obstacles_glued_to_here[glue_index];
				if (obs_index == (-1))
					break;

				obstacle *this_obs = &(validator_ctx->this_level->obstacle_list[obs_index]);

				struct chest_excpt_data to_check =
				    { obs_index, {this_obs->pos.x, this_obs->pos.y, validator_ctx->this_level->levelnum} };

				if (lookup_exception(this, &to_check))
					continue;

				if (!(IS_CHEST(this_obs->type) || IS_BARREL(this_obs->type)))
					continue;

				colldet_filter filter = WalkableExceptIdPassFilter;
				filter.data = &obs_index;
				if (!SinglePointColldet(this_obs->pos.x, this_obs->pos.y, validator_ctx->this_level->levelnum, &filter)) {
					if (!is_invalid) {	// First error : print header
						validator_print_header(validator_ctx, "Unreachable chests/barrels list",
								       "The center of the following objects was found to be inside an obstacle, preventing Tux from activating them.");
						is_invalid = TRUE;
					}
					printf("[Type=\"C\"] Obj Idx=%d (X=%f:Y=%f:L=%d) : %s\n", obs_index, this_obs->pos.x,
					       this_obs->pos.y, validator_ctx->this_level->levelnum,
					       obstacle_map[this_obs->type].obstacle_short_name);
				}
			}
		}
	}
	if (is_invalid)
		puts(line);

	return is_invalid;
}

//===========================================================
// Waypoint Validator
//
// This validator checks if waypoints are valid
//===========================================================

struct waypoint_excpt_data {
	char subtest;
	int wp_id[2];
	gps wp_pos[2];
};

/*
 * Parse a 'waypoint' exception
 */

static void *lvlval_waypoint_parse_excpt(char *string)
{
	char *validator_type = ReadAndMallocStringFromData(string, "Type=\"", "\"");
	if (!validator_type || strlen(validator_type) != 2) {
		ErrorMessage(__FUNCTION__, "The Subtest name of an exception is not valid!\n", PLEASE_INFORM, IS_FATAL);
		return NULL;
	}

	struct waypoint_excpt_data *data = (struct waypoint_excpt_data *)malloc(sizeof(struct waypoint_excpt_data));

	data->subtest = validator_type[1];
	free(validator_type);

	switch (data->subtest) {
	case 'P':
	case 'O':
	case 'S':
		ReadValueFromString(string, "Idx=", "%d", &(data->wp_id[0]), NULL);
		ReadValueFromString(string, "X=", "%f", &(data->wp_pos[0].x), NULL);
		ReadValueFromString(string, "Y=", "%f", &(data->wp_pos[0].y), NULL);
		ReadValueFromString(string, "L=", "%d", &(data->wp_pos[0].z), NULL);
		data->wp_id[1] = 0;
		data->wp_pos[1].x = data->wp_pos[1].y = 0.0;
		data->wp_pos[1].z = 0;
		break;

	case 'D':
	case 'W':
	case 'Q':
		ReadValueFromString(string, "Idx1=", "%d", &(data->wp_id[0]), NULL);
		ReadValueFromString(string, "X1=", "%f", &(data->wp_pos[0].x), NULL);
		ReadValueFromString(string, "Y1=", "%f", &(data->wp_pos[0].y), NULL);
		ReadValueFromString(string, "L1=", "%d", &(data->wp_pos[0].z), NULL);
		ReadValueFromString(string, "Idx2=", "%d", &(data->wp_id[1]), NULL);
		ReadValueFromString(string, "X2=", "%f", &(data->wp_pos[1].x), NULL);
		ReadValueFromString(string, "Y2=", "%f", &(data->wp_pos[1].y), NULL);
		ReadValueFromString(string, "L2=", "%d", &(data->wp_pos[1].z), NULL);
		break;

	default:
		ErrorMessage(__FUNCTION__, "The Subtest name of an exception is invalid!\n", PLEASE_INFORM, IS_FATAL);
		free(data);
		return NULL;
	}

	return (data);
}

/*
 * Compare two 'waypoint' exception data structures
 */

static int lvlval_waypoint_cmp_data(void *opaque_data1, void *opaque_data2)
{
#	define DIST_EPSILON 0.01f

	struct waypoint_excpt_data *data1 = opaque_data1;
	struct waypoint_excpt_data *data2 = opaque_data2;

	if (data1->subtest != data2->subtest)
		return FALSE;

	int i;
	for (i = 0; i < 2; ++i) {
		if (data1->wp_id[i] != data2->wp_id[i])
			return FALSE;
		if (data1->wp_pos[i].z != data2->wp_pos[i].z)
			return FALSE;

		float dist = calc_euklid_distance(data1->wp_pos[i].x, data1->wp_pos[i].y, data2->wp_pos[i].x, data2->wp_pos[i].y);
		if (dist > DIST_EPSILON)
			return FALSE;
	}

	return TRUE;

#	undef DIST_EPSILON
}

/*
 * 'waypoint' validator
 */

static int lvlval_waypoint_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx)
{
	int i, j;
	int pos_is_invalid = FALSE;
	int conn_is_invalid = FALSE;
	int dist_is_invalid = FALSE;
	int path_is_invalid = FALSE;
	int path_warning = FALSE;
#	define TRSL_FACT 0.02
#	define MIN_DIST 1.0

	// Check waypoints position
	for (i = 0; i < validator_ctx->this_level->num_waypoints; ++i) {
		struct waypoint_excpt_data to_check = { 'P',
			{i, 0},
			{
			 {validator_ctx->this_level->AllWaypoints[i].x + 0.5,
			  validator_ctx->this_level->AllWaypoints[i].y + 0.5,
			  validator_ctx->this_level->levelnum},
			 {0.0, 0.0, 0}
			 }
		};

		if (lookup_exception(this, &to_check))
			continue;

		if (!SinglePointColldet
		    (validator_ctx->this_level->AllWaypoints[i].x + 0.5, validator_ctx->this_level->AllWaypoints[i].y + 0.5,
		     validator_ctx->this_level->levelnum, &WalkablePassFilter)) {
			if (!pos_is_invalid) {	// First error : print header
				validator_print_header(validator_ctx, "Unreachable waypoints list",
						       "The following waypoints were found to be inside an obstacle.\n"
						       "This could lead to some bots being stuck.");
				pos_is_invalid = TRUE;
			}
			printf("[Type=\"WP\"] WP Idx=%d (X=%f:Y=%f:L=%d)\n", i, validator_ctx->this_level->AllWaypoints[i].x + 0.5,
			       validator_ctx->this_level->AllWaypoints[i].y + 0.5, validator_ctx->this_level->levelnum);
		}
	}
	if (pos_is_invalid)
		puts(line);

	// Check waypoints connectivity
	for (i = 0; i < validator_ctx->this_level->num_waypoints; ++i) {
		struct waypoint_excpt_data to_check = { 'O',
			{i, 0},
			{
			 {validator_ctx->this_level->AllWaypoints[i].x + 0.5,
			  validator_ctx->this_level->AllWaypoints[i].y + 0.5,
			  validator_ctx->this_level->levelnum},
			 {0.0, 0.0, 0}
			 }
		};

		if (!lookup_exception(this, &to_check) && validator_ctx->this_level->AllWaypoints[i].num_connections == 0) {
			if (!conn_is_invalid) {	// First error : print header
				validator_print_header(validator_ctx, "Unconnected waypoints list",
						       "The following waypoints were found to be without connection (Type=WO).\n"
						       "or self-connected (Type=WS).\n"
						       "This could lead to some bots being stuck on those waypoints.");
				conn_is_invalid = TRUE;
			}
			printf("[Type=\"WO\"] WP Idx=%d (X=%f:Y=%f:L=%d)\n", i, validator_ctx->this_level->AllWaypoints[i].x + 0.5,
			       validator_ctx->this_level->AllWaypoints[i].y + 0.5, validator_ctx->this_level->levelnum);
		}

		to_check.subtest = 'S';
		if (lookup_exception(this, &to_check))
			continue;

		for (j = 0; j < validator_ctx->this_level->AllWaypoints[i].num_connections; ++j) {
			int wp = validator_ctx->this_level->AllWaypoints[i].connections[j];
			if (wp == i) {
				if (!conn_is_invalid) {	// First error : print header
					validator_print_header(validator_ctx, "Unconnected waypoints list",
							       "The following waypoints were found to be without connection (Type=WO).\n"
							       "or self-connected (Type=WS).\n"
							       "This could lead to some bots being stuck on those waypoints.");
					conn_is_invalid = TRUE;
				}
				printf("[Type=\"WS\"] WP Idx=%d (X=%f:Y=%f:L=%d)\n", i, validator_ctx->this_level->AllWaypoints[i].x + 0.5,
				       validator_ctx->this_level->AllWaypoints[i].y + 0.5, validator_ctx->this_level->levelnum);
				continue;
			}
		}
	}
	if (conn_is_invalid)
		puts(line);

	// Check waypoints distance
	for (i = 0; i < validator_ctx->this_level->num_waypoints - 1; ++i) {
		gps wp_i =
		    { validator_ctx->this_level->AllWaypoints[i].x + 0.5, validator_ctx->this_level->AllWaypoints[i].y + 0.5,
		     validator_ctx->this_level->levelnum };

		for (j = i + 1; j < validator_ctx->this_level->num_waypoints; ++j) {
			struct waypoint_excpt_data to_check = { 'D',
				{i, j},
				{
				 {validator_ctx->this_level->AllWaypoints[i].x + 0.5,
				  validator_ctx->this_level->AllWaypoints[i].y + 0.5,
				  validator_ctx->this_level->levelnum},
				 {validator_ctx->this_level->AllWaypoints[j].x + 0.5,
				  validator_ctx->this_level->AllWaypoints[j].y + 0.5,
				  validator_ctx->this_level->levelnum}
				 }
			};

			if (lookup_exception(this, &to_check))
				continue;

			gps wp_j =
			    { validator_ctx->this_level->AllWaypoints[j].x + 0.5, validator_ctx->this_level->AllWaypoints[j].y + 0.5,
			     validator_ctx->this_level->levelnum };
			float dist = calc_euklid_distance(wp_i.x, wp_i.y, wp_j.x, wp_j.y);

			if (dist < MIN_DIST) {
				if (!dist_is_invalid) {	// First error : print header
					validator_print_header(validator_ctx, "Invalid waypoints distance",
							       "Two waypoints were found to be too close");
					dist_is_invalid = TRUE;
				}
				printf
				    ("[Type=\"WD\"] WP1 Idx1=%d (X1=%f:Y1=%f:L1=%d) <-> WP2 Idx2=%d (X2=%f:Y2=%f:L2=%d) : distance = %.3f\n",
				     i, wp_i.x, wp_i.y, validator_ctx->this_level->levelnum, j, wp_j.x, wp_j.y,
				     validator_ctx->this_level->levelnum, dist);
			}
		}
	}

	// Check waypoint paths walkability
	for (i = 0; i < validator_ctx->this_level->num_waypoints; ++i) {
		if (validator_ctx->this_level->AllWaypoints[i].num_connections == 0)
			continue;

		gps from_pos =
		    { validator_ctx->this_level->AllWaypoints[i].x + 0.5, validator_ctx->this_level->AllWaypoints[i].y + 0.5,
		 validator_ctx->this_level->levelnum };

		for (j = 0; j < validator_ctx->this_level->AllWaypoints[i].num_connections; ++j) {
			int wp = validator_ctx->this_level->AllWaypoints[i].connections[j];

			struct waypoint_excpt_data to_check = { 'W',
				{i, wp},
				{
				 {validator_ctx->this_level->AllWaypoints[i].x + 0.5,
				  validator_ctx->this_level->AllWaypoints[i].y + 0.5,
				  validator_ctx->this_level->levelnum},
				 {validator_ctx->this_level->AllWaypoints[wp].x + 0.5,
				  validator_ctx->this_level->AllWaypoints[wp].y + 0.5,
				  validator_ctx->this_level->levelnum}
				 }
			};

			if (lookup_exception(this, &to_check))
				continue;

			gps to_pos =
			    { validator_ctx->this_level->AllWaypoints[wp].x + 0.5, validator_ctx->this_level->AllWaypoints[wp].y + 0.5,
			   validator_ctx->this_level->levelnum };

			enum connect_validity rtn = waypoints_connection_valid(&from_pos, &to_pos);
			if (rtn & NO_PATH) {
				if (!path_is_invalid) {	// First error : print header
					validator_print_header(validator_ctx, "Invalid waypoint paths list",
							       "The pathfinder was not able to find a path between those waypoints.\n"
							       "This could lead those paths to not being usable.");
					path_is_invalid = TRUE;
				}
				printf("[Type=\"WW\"] WP1 Idx1=%d (X1=%f:Y1=%f:L1=%d) -> WP2 Idx2=%d (X2=%f:Y2=%f:L2=%d) : %s\n",
				       i, from_pos.x, from_pos.y, validator_ctx->this_level->levelnum,
				       wp, to_pos.x, to_pos.y, validator_ctx->this_level->levelnum,
				       (rtn & COMPLEX_PATH) ? "too complex" : "path not found");
			}
		}
	}
	if (path_is_invalid)
		puts(line);

	// Sometimes, a bot does not exactly follow the path between the waypoints
	// (perhaps due to floating point approximations).
	// So, even if the connection is in theory walkable, a bot could get stuck.
	// By translating a bit the waypoints positions, we simulate such a behavior.

	for (i = 0; i < validator_ctx->this_level->num_waypoints; ++i) {
		if (validator_ctx->this_level->AllWaypoints[i].num_connections == 0)
			continue;

		for (j = 0; j < validator_ctx->this_level->AllWaypoints[i].num_connections; ++j) {
			int wp = validator_ctx->this_level->AllWaypoints[i].connections[j];

			struct waypoint_excpt_data to_check = { 'Q',
				{i, wp},
				{
				 {validator_ctx->this_level->AllWaypoints[i].x + 0.5,
				  validator_ctx->this_level->AllWaypoints[i].y + 0.5,
				  validator_ctx->this_level->levelnum},
				 {validator_ctx->this_level->AllWaypoints[wp].x + 0.5,
				  validator_ctx->this_level->AllWaypoints[wp].y + 0.5,
				  validator_ctx->this_level->levelnum}
				 }
			};

			if (lookup_exception(this, &to_check))
				continue;

			gps from_pos =
			    { validator_ctx->this_level->AllWaypoints[i].x + 0.5, validator_ctx->this_level->AllWaypoints[i].y + 0.5,
			 validator_ctx->this_level->levelnum };
			gps to_pos =
			    { validator_ctx->this_level->AllWaypoints[wp].x + 0.5, validator_ctx->this_level->AllWaypoints[wp].y + 0.5,
			   validator_ctx->this_level->levelnum };

			// Translation vector
			moderately_finepoint line_vector;
			line_vector.x = to_pos.x - from_pos.x;
			line_vector.y = to_pos.y - from_pos.y;

			float length = sqrtf(line_vector.x * line_vector.x + line_vector.y * line_vector.y);
			if (length < MIN_DIST)
				continue;	// Too close waypoints. Already handled

			line_vector.x = (line_vector.x * TRSL_FACT) / length;
			line_vector.y = (line_vector.y * TRSL_FACT) / length;

			// Translation normal
			moderately_finepoint line_normal = { -line_vector.y, line_vector.x };

			// 1- Augment the length
			gps trsl_from_pos = { from_pos.x - line_vector.x, from_pos.y - line_vector.y, from_pos.z };
			gps trsl_to_pos = { to_pos.x + line_vector.x, to_pos.y + line_vector.y, to_pos.z };

			enum connect_validity rtn = waypoints_connection_valid(&trsl_from_pos, &trsl_to_pos);
			if (rtn & NO_PATH) {
				if (!path_warning) {	// First error : print header
					validator_print_header(validator_ctx, "Waypoint paths Warning list",
							       "The pathfinder was not able to find a path between two variations of those waypoints.\n"
							       "This could lead some bots to get stuck along those paths.");
					path_warning = TRUE;
				}
				printf("[Type=\"WQ\"] WP1 Idx1=%d (X1=%f:Y1=%f:L1=%d) -> WP2 Idx2=%d (X2=%f:Y2=%f:L2=%d) (warning)\n",
				       i, from_pos.x, from_pos.y, validator_ctx->this_level->levelnum,
				       wp, to_pos.x, to_pos.y, validator_ctx->this_level->levelnum);

				continue;	// Next connection
			}
			// 2- Translate up in the direction of the normal
			trsl_from_pos.x += line_normal.x;
			trsl_from_pos.y += line_normal.y;
			trsl_to_pos.x += line_normal.x;
			trsl_to_pos.y += line_normal.y;

			rtn = waypoints_connection_valid(&trsl_from_pos, &trsl_to_pos);
			if (rtn & NO_PATH) {
				if (!path_warning) {	// First error : print header
					validator_print_header(validator_ctx, "Waypoint paths Warning list",
							       "The pathfinder was not able to find a path between two variations of those waypoints.\n"
							       "This could lead some bots to get stuck along those paths.");
					path_warning = TRUE;
				}
				printf("[Type=\"WQ\"] WP1 Idx1=%d (X1=%f:Y1=%f:L1=%d) -> WP2 Idx2=%d (X2=%f:Y2=%f:L2=%d) (warning)\n",
				       i, from_pos.x, from_pos.y, validator_ctx->this_level->levelnum,
				       wp, to_pos.x, to_pos.y, validator_ctx->this_level->levelnum);

				continue;	// Next connection
			}
			// 3- Translate down in the direction of the normal
			trsl_from_pos.x -= 2 * line_normal.x;
			trsl_from_pos.y -= 2 * line_normal.y;
			trsl_to_pos.x -= 2 * line_normal.x;
			trsl_to_pos.y -= 2 * line_normal.y;

			rtn = waypoints_connection_valid(&trsl_from_pos, &trsl_to_pos);
			if (rtn & NO_PATH) {
				if (!path_warning) {	// First error : print header
					validator_print_header(validator_ctx, "Waypoint paths Warning list",
							       "The pathfinder was not able to find a path between two variations of those waypoints.\n"
							       "This could lead some bots to get stuck along those paths.");
					path_warning = TRUE;
				}
				printf("[Type=\"WQ\"] WP1 Idx1=%d (X1=%f:Y1=%f:L1=%d) -> WP2 Idx2=%d (X2=%f:Y2=%f:L2=%d) (warning)\n",
				       i, from_pos.x, from_pos.y, validator_ctx->this_level->levelnum,
				       wp, to_pos.x, to_pos.y, validator_ctx->this_level->levelnum);
			}
		}
	}
	if (path_warning)
		puts(line);

	return (pos_is_invalid || conn_is_invalid || dist_is_invalid || path_is_invalid || path_warning);
}

//===========================================================
// Jumptarget Validator
//
// This validator checks if jump targets are valid
//===========================================================

struct jumptarget_excpt_data {
	char jumptarget;
	int from_level;
	int to_level;
};

/*
 * Parse a 'jumptarger' exception
 */

static void *lvlval_jumptarget_parse_excpt(char *string)
{
	struct jumptarget_excpt_data *data = (struct jumptarget_excpt_data *)malloc(sizeof(struct jumptarget_excpt_data));

	char *direction_name = ReadAndMallocStringFromData(string, "Interface:", " ");
	if (!direction_name) {
		ErrorMessage(__FUNCTION__, "The Direction of an exception was not found!\n", PLEASE_INFORM, IS_FATAL);
		return NULL;
	}
	data->jumptarget = direction_name[0];
	free(direction_name);

	ReadValueFromString(string, "of Level:", "%d", &(data->from_level), NULL);
	ReadValueFromString(string, "to Level:", "%d", &(data->to_level), NULL);

	return (data);
}

/*
 * Compare two 'jumptarget' exception data structures 
 */

static int lvlval_jumptarget_cmp_data(void *opaque_data1, void *opaque_data2)
{
	struct jumptarget_excpt_data *data1 = opaque_data1;
	struct jumptarget_excpt_data *data2 = opaque_data2;

	if (data1->jumptarget != data2->jumptarget)
		return FALSE;
	if (data1->from_level != data2->from_level)
		return FALSE;
	if (data1->to_level != data2->to_level)
		return FALSE;

	return TRUE;
}

/*
 * 'jumptarget' validator
 */

static int lvlval_jumptarget_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx)
{
	int is_invalid = FALSE;

	if (validator_ctx->this_level->jump_target_north != -1 && curShip.AllLevels[validator_ctx->this_level->jump_target_north] == NULL) {
		struct jumptarget_excpt_data to_check =
		    { 'N', validator_ctx->this_level->levelnum, validator_ctx->this_level->jump_target_north };

		if (!lookup_exception(this, &to_check)) {
			validator_print_header(validator_ctx, "Non existant jump target",
					       "The north jump target on a level points to a non existing level.");
			is_invalid = TRUE;

			printf("[Type=\"J\"] Interface:North of Level:%d points to Level:%d which does not exist.\n",
			       validator_ctx->this_level->levelnum, validator_ctx->this_level->jump_target_north);
		}
	}

	if (validator_ctx->this_level->jump_target_west != -1 && curShip.AllLevels[validator_ctx->this_level->jump_target_west] == NULL) {
		struct jumptarget_excpt_data to_check =
		    { 'W', validator_ctx->this_level->levelnum, validator_ctx->this_level->jump_target_west };

		if (!lookup_exception(this, &to_check)) {
			validator_print_header(validator_ctx, "Non existant jump target",
					       "The west jump target on a level points to a non existing level.");
			is_invalid = TRUE;

			printf("[Type=\"J\"] Interface:West of Level:%d points to Level:%d which does not exist.\n",
			       validator_ctx->this_level->levelnum, validator_ctx->this_level->jump_target_west);
		}
	}

	if (validator_ctx->this_level->jump_target_east != -1 && curShip.AllLevels[validator_ctx->this_level->jump_target_east] == NULL) {
		struct jumptarget_excpt_data to_check =
		    { 'E', validator_ctx->this_level->levelnum, validator_ctx->this_level->jump_target_east };

		if (!lookup_exception(this, &to_check)) {
			validator_print_header(validator_ctx, "Non existant jump target",
					       "The east jump target on a level points to a non existing level.");
			is_invalid = TRUE;

			printf("[Type=\"J\"] Interface:East of Level:%d points to Level:%d which does not exist.\n",
			       validator_ctx->this_level->levelnum, validator_ctx->this_level->jump_target_east);
		}
	}

	if (validator_ctx->this_level->jump_target_south != -1 && curShip.AllLevels[validator_ctx->this_level->jump_target_south] == NULL) {
		struct jumptarget_excpt_data to_check =
		    { 'S', validator_ctx->this_level->levelnum, validator_ctx->this_level->jump_target_south };

		if (!lookup_exception(this, &to_check)) {
			validator_print_header(validator_ctx, "Non existant jump target",
					       "The south jump target on a level points to a non existing level.");
			is_invalid = TRUE;

			printf("[Type=\"J\"] Interface:South of Level:%d points to Level:%d which does not exist.\n",
			       validator_ctx->this_level->levelnum, validator_ctx->this_level->jump_target_south);
		}
	}

	return is_invalid;
}

//===========================================================
// Interface Validator
//
// This validator checks if level interfaces are valid
//===========================================================

struct interface_excpt_data {
	int obj_id;
	gps obj_pos;
};

/*
 * Parse "interface" validator exception
 */

static void *lvlval_interface_parse_excpt(char *string)
{
	struct interface_excpt_data *data = (struct interface_excpt_data *)malloc(sizeof(struct interface_excpt_data));

	ReadValueFromString(string, "Idx=", "%d", &(data->obj_id), NULL);
	ReadValueFromString(string, "X=", "%f", &(data->obj_pos.x), NULL);
	ReadValueFromString(string, "Y=", "%f", &(data->obj_pos.y), NULL);
	ReadValueFromString(string, "L=", "%d", &(data->obj_pos.z), NULL);

	return (data);
}

/*
 * Compare two "interface" exception data structures
 */

static int lvlval_interface_cmp_data(void *opaque_data1, void *opaque_data2)
{
#	define DIST_EPSILON 0.01f

	struct interface_excpt_data *data1 = opaque_data1;
	struct interface_excpt_data *data2 = opaque_data2;

	if (data1->obj_id != data2->obj_id)
		return FALSE;
	if (data1->obj_pos.z != data2->obj_pos.z)
		return FALSE;

	float dist = calc_euklid_distance(data1->obj_pos.x, data1->obj_pos.y, data2->obj_pos.x, data2->obj_pos.y);
	if (dist > DIST_EPSILON)
		return FALSE;

	return TRUE;

#	undef DIST_EPSILON
}

/*
 * "interface" validator
 */

static int lvlval_interface_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx)
{
	int x_tile, y_tile, glue_index;
	int is_invalid = FALSE;

	for (y_tile = 0; y_tile < validator_ctx->this_level->ylen; ++y_tile) {
		for (x_tile = 0; x_tile < validator_ctx->this_level->xlen; ++x_tile) {
			if ((y_tile >= validator_ctx->this_level->jump_threshold_north)
			    && (y_tile <= (validator_ctx->this_level->ylen - validator_ctx->this_level->jump_threshold_south))
			    && (x_tile >= validator_ctx->this_level->jump_threshold_west)
			    && (x_tile <= (validator_ctx->this_level->xlen - validator_ctx->this_level->jump_threshold_east)))
				continue;

			for (glue_index = 0; glue_index < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE; ++glue_index) {
				int obs_index = validator_ctx->this_level->map[y_tile][x_tile].obstacles_glued_to_here[glue_index];
				if (obs_index == (-1))
					break;

				obstacle *this_obs = &(validator_ctx->this_level->obstacle_list[obs_index]);

				struct interface_excpt_data to_check =
				    { obs_index, {this_obs->pos.x, this_obs->pos.y, validator_ctx->this_level->levelnum} };

				if (lookup_exception(this, &to_check))
					continue;

				if (!(IS_CHEST(this_obs->type) || IS_BARREL(this_obs->type)))
					continue;

				if (!(is_invalid)) {	// First error : print header
					validator_print_header(validator_ctx, "Invalid level-interfaces list",
							       "The following objects were found on the interface area.\n"
							       "The activation of those objects will not be reflected on the neighborhood.");
					is_invalid = TRUE;
				}
				printf("[Type=\"I\"] Obj Idx=%d (X=%f:Y=%f:L=%d) : %s\n", obs_index, this_obs->pos.x, this_obs->pos.y,
				       validator_ctx->this_level->levelnum, obstacle_map[this_obs->type].obstacle_short_name);
			}
		}
	}
	if (is_invalid)
		puts(line);

	return is_invalid;
}

//===========================================================
// ENTRY POINT
//
// Run several validations
//===========================================================

void LevelValidation()
{
	int is_invalid = FALSE;
	int uncaught_excpt = FALSE;

	SDL_Rect background_rect = { UNIVERSAL_COORD_W(20), UNIVERSAL_COORD_H(20), UNIVERSAL_COORD_W(600), UNIVERSAL_COORD_H(440) };
	SDL_Rect report_rect = { UNIVERSAL_COORD_W(30), UNIVERSAL_COORD_H(30), UNIVERSAL_COORD_W(580), UNIVERSAL_COORD_H(420) };

	BFont_Info *current_font = GetCurrentFont();
	int raw_height = FontHeight(current_font);
	int max_raws = (report_rect.h / raw_height) - 4;	// 4 lines are reserved for header and footer 
	int column_width = TextWidth("Level 000: empty");

	AssembleCombatPicture(ONLY_SHOW_MAP_AND_TEXT | SHOW_GRID | SKIP_LIGHT_RADIUS);
	ShadowingRectangle(Screen, background_rect);

	//--------------------
	// Title
	//
	CenteredPutString(Screen, report_rect.y, "Level Validation tests - Summary\n");

	//--------------------
	// Load exceptions rules
	//
	load_excpt_lists("freedroid.lvleditor_exceptions");

	//--------------------
	// Loop on each level
	//
	int l;
	int col_pos = 0;
	int raw_pos = 0;

	for (l = 0; l < curShip.num_levels; ++l) {
		struct lvlval_ctx validator_ctx = { &report_rect, curShip.AllLevels[l] };

		// Compute raw and column position, when a new column of text starts
		if ((l % max_raws) == 0) {
			col_pos = report_rect.x + (l / max_raws) * column_width;
			raw_pos = report_rect.y + 2 * raw_height;	// 2 lines are reserved for the header
			SetTextCursor(col_pos, raw_pos);
		}

		if (curShip.AllLevels[l] == NULL) {
			// Empty level
			char txt[40];
			sprintf(txt, "%s %3d: \2empty\n", "Level", l);
			DisplayText(txt, col_pos, -1, &report_rect, 1.0);
			SetCurrentFont(current_font);	// Reset font
		} else {
			// Loop on each validation function
			int v = 0;
			struct level_validator *one_validator;
			int level_is_invalid = FALSE;

			while (one_validator = &(level_validators[v++]), one_validator->execute != NULL)
				level_is_invalid |= one_validator->execute(one_validator, &validator_ctx);

			// Display report
			char txt[40];
			sprintf(txt, "%s %3d: %s\n", "Level", l, (level_is_invalid) ? "\1fail" : "pass");
			DisplayText(txt, col_pos, -1, &report_rect, 1.0);
			SetCurrentFont(current_font);	// Reset font in case of the red "fail" was displayed

			// Set global is_invalid flag
			is_invalid |= level_is_invalid;
		}
	}

	//--------------------
	// Outputs uncaught exception rules
	//

	uncaught_excpt = print_uncaught_exceptions();

	//--------------------

	free_exception_lists();

	//--------------------
	// This was it.  We can say so and return.
	//
	int posy = report_rect.y + report_rect.h - raw_height;

	CenteredPutString(Screen, posy, "--- End of List --- Press Space to return to leveleditor ---");

	if (is_invalid) {
		posy -= raw_height;
		CenteredPutString(Screen, posy, "\1Some tests were invalid. See the report in the console\3");
	}

	if (uncaught_excpt) {
		posy -= raw_height;
		CenteredPutString(Screen, posy, "\1Some exceptions were not caught. See the report in the console\3");
	}

	our_SDL_flip_wrapper();

}				// LevelValidation( )

#undef _leveleditor_validator_c
