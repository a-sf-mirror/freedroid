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
#define IS_SIGN(t)   ( (t) >= ISO_SIGN_1         && (t) <= ISO_SIGN_3       )

static char *bigline = "====================================================================";
static char *line = "--------------------------------------------------------------------";
static char *sepline = "+------------------------------";

static void lvlval_chest_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx);
static void *lvlval_chest_parse_excpt(char *string);
static int lvlval_chest_cmp_data(void *opaque_data1, void *opaque_data2);

static void lvlval_waypoint_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx);
static void *lvlval_waypoint_parse_excpt(char *string);
static int lvlval_waypoint_cmp_data(void *opaque_data1, void *opaque_data2);

static void lvlval_neighborhood_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx);
static void *lvlval_neighborhood_parse_excpt(char *string);
static int lvlval_neighborhood_cmp_data(void *opaque_data1, void *opaque_data2);

static void lvlval_obstacles_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx);
static void *lvlval_obstacles_parse_excpt(char *string);
static int lvlval_obstacles_cmp_data(void *opaque_data1, void *opaque_data2);

static void lvlval_extensions_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx);
static void *lvlval_extensions_parse_excpt(char *string);
static int lvlval_extensions_cmp_data(void *opaque_data1, void *opaque_data2);

static void lvlval_map_labels_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx);

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
	 lvlval_neighborhood_execute,
	 lvlval_neighborhood_parse_excpt,
	 lvlval_neighborhood_cmp_data},
	{'O',
	 LIST_HEAD_INIT(level_validators[3].excpt_list),
	 lvlval_obstacles_execute,
	 lvlval_obstacles_parse_excpt,
	 lvlval_obstacles_cmp_data},
	{'E',
	 LIST_HEAD_INIT(level_validators[4].excpt_list),
	 lvlval_extensions_execute,
	 lvlval_extensions_parse_excpt,
	 lvlval_extensions_cmp_data},
	{'L',
	 LIST_HEAD_INIT(level_validators[5].excpt_list),
	 lvlval_map_labels_execute,
	 NULL,
	 NULL},
	{.initial = '\0'}
};

//===========================================================
// Helper Functions
//===========================================================

/**
 * Compute a validator_return_code 'composition'
 */
static void compose_return_code(enum validator_return_code *old_code, enum validator_return_code with_code)
{
	// Values in enum validator_return_code are sorted in increasing order of
	// "failure". So the result of the composition is the highest of the 2 values.
	if (with_code > *old_code)
		*old_code = with_code;
}

/**
 * Output to the console the validator's title and associated comment
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
 * Output to the console a validator's error, with the associated header, on first output
 */

static void validator_print_error(struct lvlval_ctx *validator_ctx, struct lvlval_error *validator_error, ...)
{
	va_list args;
	va_start(args, validator_error);

	if (!validator_error->caught) {
		validator_print_header(validator_ctx, validator_error->title, validator_error->comment);
		validator_error->caught = TRUE;
		validator_ctx->in_report_section = TRUE;
	}
	
	compose_return_code(&validator_ctx->return_code, validator_error->code);
	
	vprintf(validator_error->format, args);
	printf("\n");
}

static void validator_print_separator(struct lvlval_ctx *validator_ctx)
{
	if (validator_ctx->in_report_section)
		puts(line);
	validator_ctx->in_report_section = FALSE;
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

			// Call sub-validator's parse function

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
 * Output : TRUE if there were uncaught exceptions
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

	float dist = calc_distance(data1->obj_pos.x, data1->obj_pos.y, data2->obj_pos.x, data2->obj_pos.y);
	if (dist > DIST_EPSILON)
		return FALSE;

	return TRUE;

#	undef DIST_EPSILON
}

/*
 * "chest" validator
 */

static void lvlval_chest_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx)
{
	int x_tile, y_tile, glue_index;

	struct lvlval_error chest_error = {
		.title = "Unreachable chests/barrels list",
		.comment = "The center of the following objects was found to be inside an obstacle, preventing Tux from activating them.",
		.format = "[Type=\"C\"] Obj Idx=%d (X=%f:Y=%f:L=%d)",
		.caught = FALSE,
		.code = VALIDATION_ERROR
	};
	
	for (y_tile = 0; y_tile < validator_ctx->this_level->ylen; ++y_tile) {
		for (x_tile = 0; x_tile < validator_ctx->this_level->xlen; ++x_tile) {
			for (glue_index = 0; glue_index < validator_ctx->this_level->map[y_tile][x_tile].glued_obstacles.size; ++glue_index) {
				int obs_index = ((int *)(validator_ctx->this_level->map[y_tile][x_tile].glued_obstacles.arr))[glue_index];

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
					validator_print_error(validator_ctx, &chest_error, 
					                      obs_index, this_obs->pos.x, this_obs->pos.y, validator_ctx->this_level->levelnum);
				}
			}
		}
	}
	
	validator_print_separator(validator_ctx);
}

//===========================================================
// Waypoint Validator
//
// This validator checks if waypoints are valid
//===========================================================

struct waypoint_excpt_data {
	char subtest;
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
		ReadValueFromString(string, "X=", "%f", &(data->wp_pos[0].x), NULL);
		ReadValueFromString(string, "Y=", "%f", &(data->wp_pos[0].y), NULL);
		ReadValueFromString(string, "L=", "%d", &(data->wp_pos[0].z), NULL);
		data->wp_pos[1].x = data->wp_pos[1].y = 0.0;
		data->wp_pos[1].z = 0;
		break;

	case 'D':
	case 'W':
	case 'Q':
		ReadValueFromString(string, "X1=", "%f", &(data->wp_pos[0].x), NULL);
		ReadValueFromString(string, "Y1=", "%f", &(data->wp_pos[0].y), NULL);
		ReadValueFromString(string, "L1=", "%d", &(data->wp_pos[0].z), NULL);
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
		if (data1->wp_pos[i].z != data2->wp_pos[i].z)
			return FALSE;

		float dist = calc_distance(data1->wp_pos[i].x, data1->wp_pos[i].y, data2->wp_pos[i].x, data2->wp_pos[i].y);
		if (dist > DIST_EPSILON)
			return FALSE;
	}

	return TRUE;

#	undef DIST_EPSILON
}

/*
 * 'waypoint' validator
 */

static void lvlval_waypoint_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx)
{
	int i, j;

	struct lvlval_error pos_error = {
		.title = "Unreachable waypoints list",
		.comment = "The following waypoints were found to be inside an obstacle.\n"
		           "This could lead to some bots being stuck.",
		.format = "[Type=\"WP\"] WP X=%f:Y=%f:L=%d",
		.caught = FALSE,
		.code = VALIDATION_ERROR
	};

	struct lvlval_error conn_error = {
		.title = "Unconnected waypoints list",
		.comment = "The following waypoints were found to be without connection (Type=WO).\n"
		           "or self-connected (Type=WS).\n"
		           "This could lead to some bots being stuck on those waypoints.",
		.format = "[Type=\"WO\"] WP X=%f:Y=%f:L=%d",
		.caught = FALSE,
		.code = VALIDATION_ERROR
	};

	struct lvlval_error dist_error = {
		.title = "Invalid waypoints distance",
		.comment = "Two waypoints were found to be too close.",
		.format = "[Type=\"WD\"] WP1 X1=%f:Y1=%f:L1=%d <-> WP2 X2=%f:Y2=%f:L2=%d : distance = %.3f",
		.caught = FALSE,
		.code = VALIDATION_ERROR
	};

	struct lvlval_error path_error = {
		.title = "Invalid waypoint paths list",
		.comment = "The pathfinder was not able to find a path between those waypoints.\n"
		           "This could lead those paths to not being usable.",
		.format = "[Type=\"WW\"] WP1 X1=%f:Y1=%f:L1=%d -> WP2 X2=%f:Y2=%f:L2=%d : %s",
		.caught = FALSE,
		.code = VALIDATION_ERROR
	};

	struct lvlval_error path_warning = {
		.title = "Waypoint paths Warning list",
		.comment = "The pathfinder was not able to find a path between two variations of those waypoints.\n"
		           "This could lead some bots to get stuck along those paths.",
		.format = "[Type=\"WQ\"] WP1 X1=%f:Y1=%f:L1=%d -> WP2 X2=%f:Y2=%f:L2=%d (warning)",
		.caught = FALSE,
		.code = VALIDATION_ERROR
	};

	struct lvlval_error no_waypoints = {
		.title = "Level has zero waypoints",
		.comment = "Zero waypoints on a level can be a problem when a bot gets on this level, such as "
		           "by following Tux. At least one waypoint should be placed on each level.",
		.format = "[Type=\"WZ\"] WP L=%d (warning)",
		.caught = FALSE,
		.code = VALIDATION_WARNING
	};

	waypoint *wpts;
#	define MIN_DIST 1.0

	// Get all waypoints for the level
	wpts = validator_ctx->this_level->waypoints.arr;

	//Check that we have waypoints
	if (!validator_ctx->this_level->waypoints.size)
		validator_print_error(validator_ctx, &no_waypoints, validator_ctx->this_level->levelnum);

	// Check waypoints position
	for (i = 0; i < validator_ctx->this_level->waypoints.size; ++i) {
		struct waypoint_excpt_data to_check = { 'P',
			{ 
				{wpts[i].x + 0.5, wpts[i].y + 0.5, validator_ctx->this_level->levelnum},
				{0.0, 0.0, 0}
			}
		};

		if (lookup_exception(this, &to_check))
			continue;

		if (!SinglePointColldet(wpts[i].x + 0.5, wpts[i].y + 0.5, validator_ctx->this_level->levelnum, &WalkablePassFilter)) {
			validator_print_error(validator_ctx, &pos_error, wpts[i].x + 0.5, wpts[i].y + 0.5, validator_ctx->this_level->levelnum);
		}
	}
	
	validator_print_separator(validator_ctx);

	// Check waypoints connectivity
	for (i = 0; i < validator_ctx->this_level->waypoints.size; ++i) {
		
		// Check for unconnection
		
		struct waypoint_excpt_data to_check = { 'O',
			{
				{wpts[i].x + 0.5, wpts[i].y + 0.5, validator_ctx->this_level->levelnum},
				{0.0, 0.0, 0}
			}
		};

		if (!lookup_exception(this, &to_check) && wpts[i].connections.size == 0) {
			validator_print_error(validator_ctx, &conn_error,
			                      wpts[i].x + 0.5, wpts[i].y + 0.5, validator_ctx->this_level->levelnum);
		}

		// Check for self-connection
		
		to_check.subtest = 'S';
		conn_error.format = "[Type=\"WS\"] WP X=%f:Y=%f:L=%d";
		
		if (lookup_exception(this, &to_check))
			continue;

		int *connections = wpts[i].connections.arr;
		for (j = 0; j < wpts[i].connections.size; ++j) {
			if (connections[j] == i) {
				validator_print_error(validator_ctx, &conn_error,
				                      wpts[i].x + 0.5, wpts[i].y + 0.5, validator_ctx->this_level->levelnum);
				continue;
			}
		}
	}
	
	validator_print_separator(validator_ctx);

	// Check waypoints distance
	
	for (i = 0; i < validator_ctx->this_level->waypoints.size - 1; ++i) {
		// Get the source waypoint
		waypoint *from_wp = &wpts[i];

		gps wp_i = { from_wp->x + 0.5, from_wp->y + 0.5, validator_ctx->this_level->levelnum };

		for (j = i + 1; j < validator_ctx->this_level->waypoints.size; ++j) {
			// Get the destination waypoint
			waypoint *to_wp = &wpts[j];

			struct waypoint_excpt_data to_check = { 'D',
				{
					{from_wp->x + 0.5, from_wp->y + 0.5, validator_ctx->this_level->levelnum},
					{to_wp->x + 0.5, to_wp->y + 0.5, validator_ctx->this_level->levelnum}
				}
			};

			if (lookup_exception(this, &to_check))
				continue;

			gps wp_j = { to_wp->x + 0.5, to_wp->y + 0.5, validator_ctx->this_level->levelnum };

			float dist = calc_distance(wp_i.x, wp_i.y, wp_j.x, wp_j.y);

			if (dist < MIN_DIST) {
				validator_print_error(validator_ctx, &dist_error,
				     wp_i.x, wp_i.y, validator_ctx->this_level->levelnum, wp_j.x, wp_j.y,
				     validator_ctx->this_level->levelnum, dist);
			}
		}
	}

	validator_print_separator(validator_ctx);

	// Check waypoint paths walkability
	
	for (i = 0; i < validator_ctx->this_level->waypoints.size; ++i) {
		// Get the source waypoint
		waypoint *from_wp = &wpts[i];

		if (from_wp->connections.size == 0)
			continue;

		gps from_pos = { from_wp->x + 0.5, from_wp->y + 0.5, validator_ctx->this_level->levelnum };

		// Get the connections of the waypoint
		int *connections = from_wp->connections.arr;

		for (j = 0; j < from_wp->connections.size; ++j) {
			// Get the destination waypoint
			waypoint *to_wp = &wpts[connections[j]];

			struct waypoint_excpt_data to_check = { 'W',
				{
					{from_wp->x + 0.5, from_wp->y + 0.5, validator_ctx->this_level->levelnum},
					{to_wp->x + 0.5, to_wp->y + 0.5, validator_ctx->this_level->levelnum}
				}
			};

			if (lookup_exception(this, &to_check))
				continue;

			gps to_pos = { to_wp->x + 0.5, to_wp->y + 0.5, validator_ctx->this_level->levelnum };

			enum connect_validity rtn = waypoints_connection_valid(&from_pos, &to_pos);
			if (rtn & NO_PATH) {
				validator_print_error(validator_ctx, &path_error,
				       from_pos.x, from_pos.y, validator_ctx->this_level->levelnum,
				       to_pos.x, to_pos.y, validator_ctx->this_level->levelnum,
				       (rtn & COMPLEX_PATH) ? "too complex" : "path not found");
			}
		}
	}

	validator_print_separator(validator_ctx);

	// Sometimes, a bot does not exactly follow the path between the waypoints
	// (perhaps due to floating point approximations).
	// So, even if the connection is in theory walkable, a bot could get stuck.
	// By translating a bit the waypoints positions, we simulate such a behavior.

	for (i = 0; i < validator_ctx->this_level->waypoints.size; ++i) {
		// Get the from waypoint
		waypoint *from_wp = &wpts[i];

		if (from_wp->connections.size == 0)
			continue;

		// Get the connections of the from waypoint
		int *connections = from_wp->connections.arr;

		for (j = 0; j < from_wp->connections.size; ++j) {
			// Get the destination waypoint
			waypoint *to_wp = &wpts[connections[j]];

			struct waypoint_excpt_data to_check = { 'Q',
				{
					{from_wp->x + 0.5, from_wp->y + 0.5, validator_ctx->this_level->levelnum},
					{to_wp->x + 0.5, to_wp->y + 0.5, validator_ctx->this_level->levelnum}
				}
			};

			if (lookup_exception(this, &to_check))
				continue;

			gps from_pos = { from_wp->x + 0.5, from_wp->y + 0.5, validator_ctx->this_level->levelnum };
			gps to_pos = { to_wp->x + 0.5, to_wp->y + 0.5, validator_ctx->this_level->levelnum };

			// Translation vector
			moderately_finepoint line_vector;
			line_vector.x = to_pos.x - from_pos.x;
			line_vector.y = to_pos.y - from_pos.y;

			float length = sqrtf(line_vector.x * line_vector.x + line_vector.y * line_vector.y);
			if (length < MIN_DIST)
				continue;	// Too close waypoints. Already handled

			line_vector.x = (line_vector.x * COLLDET_WALKABLE_MARGIN) / length;
			line_vector.y = (line_vector.y * COLLDET_WALKABLE_MARGIN) / length;

			// Translation normal
			moderately_finepoint line_normal = { -line_vector.y, line_vector.x };

			// 1- Augment the length
			gps trsl_from_pos = { from_pos.x - line_vector.x, from_pos.y - line_vector.y, from_pos.z };
			gps trsl_to_pos = { to_pos.x + line_vector.x, to_pos.y + line_vector.y, to_pos.z };

			enum connect_validity rtn = waypoints_connection_valid(&trsl_from_pos, &trsl_to_pos);
			if (rtn & NO_PATH) {
				validator_print_error(validator_ctx, &path_warning,
				       from_pos.x, from_pos.y, validator_ctx->this_level->levelnum,
				       to_pos.x, to_pos.y, validator_ctx->this_level->levelnum);

				continue;	// Next connection
			}
			
			// 2- Translate up in the direction of the normal
			trsl_from_pos.x += line_normal.x;
			trsl_from_pos.y += line_normal.y;
			trsl_to_pos.x += line_normal.x;
			trsl_to_pos.y += line_normal.y;

			rtn = waypoints_connection_valid(&trsl_from_pos, &trsl_to_pos);
			if (rtn & NO_PATH) {
				validator_print_error(validator_ctx, &path_warning,
				       from_pos.x, from_pos.y, validator_ctx->this_level->levelnum,
				       to_pos.x, to_pos.y, validator_ctx->this_level->levelnum);

				continue;	// Next connection
			}

			// 3- Translate down in the direction of the normal
			trsl_from_pos.x -= 2 * line_normal.x;
			trsl_from_pos.y -= 2 * line_normal.y;
			trsl_to_pos.x -= 2 * line_normal.x;
			trsl_to_pos.y -= 2 * line_normal.y;

			rtn = waypoints_connection_valid(&trsl_from_pos, &trsl_to_pos);
			if (rtn & NO_PATH) {
				validator_print_error(validator_ctx, &path_warning,
				                      from_pos.x, from_pos.y, validator_ctx->this_level->levelnum,
				                      to_pos.x, to_pos.y, validator_ctx->this_level->levelnum);
			}
		}
	}

	validator_print_separator(validator_ctx);
	
#	undef MIN_DIST
}

//===========================================================
// Neighborhood Validator
//
// This validator checks if neighborhood is valid
//===========================================================

struct neighborhood_excpt_data {
	char jumptarget;
	int from_level;
	int to_level;
};

/*
 * Parse a 'neighborhood' exception
 */

static void *lvlval_neighborhood_parse_excpt(char *string)
{
	struct neighborhood_excpt_data *data = (struct neighborhood_excpt_data *)malloc(sizeof(struct neighborhood_excpt_data));

	char *direction_name = ReadAndMallocStringFromData(string, "Interface:", " ");
	if (!direction_name) {
		ErrorMessage(__FUNCTION__, "The Direction of an exception was not found!\n", PLEASE_INFORM, IS_FATAL);
		free(data);
		return NULL;
	}
	data->jumptarget = direction_name[0];
	free(direction_name);

	ReadValueFromString(string, "of Level:", "%d", &(data->from_level), NULL);
	ReadValueFromString(string, "to Level:", "%d", &(data->to_level), NULL);

	return (data);
}

/*
 * Compare two 'neighborhood' exception data structures
 */

static int lvlval_neighborhood_cmp_data(void *opaque_data1, void *opaque_data2)
{
	struct neighborhood_excpt_data *data1 = opaque_data1;
	struct neighborhood_excpt_data *data2 = opaque_data2;

	if (data1->jumptarget != data2->jumptarget)
		return FALSE;
	if (data1->from_level != data2->from_level)
		return FALSE;
	if (data1->to_level != data2->to_level)
		return FALSE;

	return TRUE;
}

/*
 * 'neighborhood' validator
 */

static void lvlval_neighborhood_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx)
{
	struct lvlval_error ngb_error = {
		.title = "Non existent neighbor",
		.comment = "A jump target on a level points to a non existing level.",
		.format = "[Type=\"J\"] Interface:North of Level:%d points to Level:%d which does not exist.",
		.caught = FALSE,
		.code = VALIDATION_ERROR
	};

	struct lvlval_error incons_error = {
		.title = "Neighborhood inconsistency",
		.comment = "A neighbor of a level is not back-connected to that level,\n"
		           "or they have not the same width.",
		.caught = FALSE,
		.code = VALIDATION_ERROR
	};

	/*
	 * 1) check for existence of the defined neighbor
	 */

	if (validator_ctx->this_level->jump_target_north != -1) {
		if (!level_exists(validator_ctx->this_level->jump_target_north)) {
			struct neighborhood_excpt_data to_check =
		    		{ 'N', validator_ctx->this_level->levelnum, validator_ctx->this_level->jump_target_north };

			if (!lookup_exception(this, &to_check)) {
				validator_print_error(validator_ctx, &ngb_error,
				                      validator_ctx->this_level->levelnum, validator_ctx->this_level->jump_target_north);
			}
		}
	}

	ngb_error.format = "[Type=\"J\"] Interface:West of Level:%d points to Level:%d which does not exist.";
	if (validator_ctx->this_level->jump_target_west != -1) {
		if (!level_exists(validator_ctx->this_level->jump_target_west)) {
			struct neighborhood_excpt_data to_check =
		    		{ 'W', validator_ctx->this_level->levelnum, validator_ctx->this_level->jump_target_west };

			if (!lookup_exception(this, &to_check)) {
				validator_print_error(validator_ctx, &ngb_error,
				                      validator_ctx->this_level->levelnum, validator_ctx->this_level->jump_target_west);
			}
		}
	}

	ngb_error.format = "[Type=\"J\"] Interface:East of Level:%d points to Level:%d which does not exist.";
	if (validator_ctx->this_level->jump_target_east != -1) {
		if (!level_exists(validator_ctx->this_level->jump_target_east)) {
			struct neighborhood_excpt_data to_check =
		    		{ 'E', validator_ctx->this_level->levelnum, validator_ctx->this_level->jump_target_east };

			if (!lookup_exception(this, &to_check)) {
				validator_print_error(validator_ctx, &ngb_error,
				                      validator_ctx->this_level->levelnum, validator_ctx->this_level->jump_target_east);
			}
		}
	}

	ngb_error.format = "[Type=\"J\"] Interface:Sputh of Level:%d points to Level:%d which does not exist.";
	if (validator_ctx->this_level->jump_target_south != -1) {
		if (!level_exists(validator_ctx->this_level->jump_target_south)) {
			struct neighborhood_excpt_data to_check =
		    		{ 'S', validator_ctx->this_level->levelnum, validator_ctx->this_level->jump_target_south };

			if (!lookup_exception(this, &to_check)) {
				validator_print_error(validator_ctx, &ngb_error,
				                      validator_ctx->this_level->levelnum, validator_ctx->this_level->jump_target_south);
			}
		}
	}

	validator_print_separator(validator_ctx);
	
	/*
	 * 2) Check "reverse connections" and consistency of neighbors dimension
	 */

	if (validator_ctx->this_level->jump_target_north != -1) {
		if (curShip.AllLevels[validator_ctx->this_level->jump_target_north]->jump_target_south != validator_ctx->this_level->levelnum) {
			incons_error.format = "[ERROR] Interface:South of Level:%d points to Level:%d instead of Level:%d.",
			validator_print_error(validator_ctx, &incons_error,
			                      validator_ctx->this_level->jump_target_north,
			                      curShip.AllLevels[validator_ctx->this_level->jump_target_north]->jump_target_south,
			                      validator_ctx->this_level->levelnum);
		} else {
			if (curShip.AllLevels[validator_ctx->this_level->jump_target_north]->xlen != validator_ctx->this_level->xlen) {
				incons_error.format = "[ERROR] Level:%d and its northern neighbor:%d have not the same width.",
				validator_print_error(validator_ctx, &incons_error,
				                      validator_ctx->this_level->levelnum,
				                      validator_ctx->this_level->jump_target_north);
			}
		}
	}

	if (validator_ctx->this_level->jump_target_south != -1) {
		if (curShip.AllLevels[validator_ctx->this_level->jump_target_south]->jump_target_north != validator_ctx->this_level->levelnum) {
			incons_error.format = "[ERROR] Interface:North of Level:%d points to Level:%d instead of Level:%d.",
			validator_print_error(validator_ctx, &incons_error,
			                      validator_ctx->this_level->jump_target_south,
			                      curShip.AllLevels[validator_ctx->this_level->jump_target_south]->jump_target_north,
			                      validator_ctx->this_level->levelnum);
		} else {
			if (curShip.AllLevels[validator_ctx->this_level->jump_target_south]->xlen != validator_ctx->this_level->xlen) {
				incons_error.format = "[ERROR] Level:%d and its southern neighbor:%d have not the same width.",
				validator_print_error(validator_ctx, &incons_error,
				                      validator_ctx->this_level->levelnum,
				                      validator_ctx->this_level->jump_target_south);
			}
		}
	}

	if (validator_ctx->this_level->jump_target_east != -1) {
		if (curShip.AllLevels[validator_ctx->this_level->jump_target_east]->jump_target_west != validator_ctx->this_level->levelnum) {
			incons_error.format = "[ERROR] Interface:West of Level:%d points to Level:%d instead of Level:%d.",
			validator_print_error(validator_ctx, &incons_error,
			                      validator_ctx->this_level->jump_target_east,
			                      curShip.AllLevels[validator_ctx->this_level->jump_target_east]->jump_target_west,
			                      validator_ctx->this_level->levelnum);
		} else {
			if (curShip.AllLevels[validator_ctx->this_level->jump_target_east]->ylen != validator_ctx->this_level->ylen) {
				incons_error.format = "[ERROR] Level:%d and its eastern neighbor:%d have not the same width.",
				validator_print_error(validator_ctx, &incons_error,
				                      validator_ctx->this_level->levelnum,
				                      validator_ctx->this_level->jump_target_east);
			}
		}
	}

	if (validator_ctx->this_level->jump_target_west != -1) {
		if (curShip.AllLevels[validator_ctx->this_level->jump_target_west]->jump_target_east != validator_ctx->this_level->levelnum) {
			incons_error.format = "[ERROR] Interface:East of Level:%d points to Level:%d instead of Level:%d.",
			validator_print_error(validator_ctx, &incons_error,
			                      validator_ctx->this_level->jump_target_west,
			                      curShip.AllLevels[validator_ctx->this_level->jump_target_west]->jump_target_east,
			                      validator_ctx->this_level->levelnum);
		} else {
			if (curShip.AllLevels[validator_ctx->this_level->jump_target_west]->ylen != validator_ctx->this_level->ylen) {
				incons_error.format = "[ERROR] Level:%d and its western neighbor:%d have not the same width.",
				validator_print_error(validator_ctx, &incons_error,
				                      validator_ctx->this_level->levelnum,
				                      validator_ctx->this_level->jump_target_west);
			}
		}
	}

	validator_print_separator(validator_ctx);
}

//===========================================================
// Obstacles Validator
//
// This validator checks if obstacles positions are valid
//===========================================================

struct obstacle_excpt_data {
	char subtest;
	gps obs_pos;
	int type;
	float border;
};

static void *lvlval_obstacles_parse_excpt(char *string)
{
	char *validator_type = ReadAndMallocStringFromData(string, "Type=\"", "\"");
	if (!validator_type || strlen(validator_type) != 2) {
		ErrorMessage(__FUNCTION__, "The Subtest name of an exception is not valid!\n", PLEASE_INFORM, IS_FATAL);
		return NULL;
	}

	struct obstacle_excpt_data *data = (struct obstacle_excpt_data *)malloc(sizeof(struct obstacle_excpt_data));

	data->subtest = validator_type[1];
	free(validator_type);

	switch (data->subtest) {
	case 'N':
	case 'S':
	case 'W':
	case 'E':
		ReadValueFromString(string, "X=", "%f", &(data->obs_pos.x), NULL);
		ReadValueFromString(string, "Y=", "%f", &(data->obs_pos.y), NULL);
		ReadValueFromString(string, "L=", "%d", &(data->obs_pos.z), NULL);
		ReadValueFromString(string, "T=", "%d", &(data->type), NULL);
		ReadValueFromString(string, "border=", "%f", &(data->border), NULL);
		break;

	default:
		ErrorMessage(__FUNCTION__, "The Subtest name of an exception is invalid!\n", PLEASE_INFORM, IS_FATAL);
		free(data);
		return NULL;
	}

	return (data);
}

static int lvlval_obstacles_cmp_data(void *opaque_data1, void *opaque_data2)
{
#	define DIST_EPSILON 0.01f

	struct obstacle_excpt_data *data1 = opaque_data1;
	struct obstacle_excpt_data *data2 = opaque_data2;

	if (data1->subtest != data2->subtest)
		return FALSE;

	if (data1->obs_pos.z != data2->obs_pos.z)
		return FALSE;

	if (data1->type != data2->type)
		return FALSE;

	float dist = calc_distance(data1->obs_pos.x, data1->obs_pos.y, data2->obs_pos.x, data2->obs_pos.y);
	if (dist > DIST_EPSILON)
		return FALSE;

	if (fabs(data1->border - data2->border) > DIST_EPSILON)
		return FALSE;

	return TRUE;

#	undef DIST_EPSILON
}

/*
 * 'obstacles' validator
 */

static void lvlval_obstacles_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx)
{
	struct lvlval_error obs_error = {
		.title = "Invalid obstacle position",
		.comment = "The center of an obstacle is outside a level boundaries,\n"
		           "or the collision rectangle of the obstacle is spilling out on a neighbor level.",
		.caught = FALSE,
		.code = VALIDATION_ERROR
	};

	int i;
	float border;
	level *l = validator_ctx->this_level;

	float max_x = (float)l->xlen;
	float max_y = (float)l->ylen;

	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		obstacle *o = &l->obstacle_list[i];

		if (o->type == -1)
			continue;

		if (o->pos.x < 0 || o->pos.x > max_x || o->pos.y < 0 || o->pos.y > max_y) {
			obs_error.format = "[ERROR] Obstacle n.%d on level %d, type %d, has position %f %f. Allowed X position ranges from 0 to %f. Allowed Y position ranges from 0 to %f.";
			validator_print_error(validator_ctx, &obs_error, i, l->levelnum, o->type, o->pos.x, o->pos.y, max_x, max_y);
			continue;
		}

		obstacle_spec *obstacle_spec = get_obstacle_spec(o->type);
		border = o->pos.x + obstacle_spec->left_border;
		if (border < 0 && l->jump_target_west != -1) {
			struct obstacle_excpt_data to_check =
			    { 'W', {o->pos.x, o->pos.y, l->levelnum}, o->type, border };

			if (!lookup_exception(this, &to_check)) {
				obs_error.format = "[Type=\"OW\"] X=%f:Y=%f:L=%d T=%d -> west border=%f (warning)";
				obs_error.code = VALIDATION_WARNING;
				validator_print_error(validator_ctx, &obs_error, o->pos.x, o->pos.y, l->levelnum, o->type, border);
			}
		}

		border = o->pos.x + obstacle_spec->right_border;
		if (border > max_x && l->jump_target_east != -1) {
			struct obstacle_excpt_data to_check =
			    { 'E', {o->pos.x, o->pos.y, l->levelnum}, o->type, border };

			if (!lookup_exception(this, &to_check)) {
				obs_error.format = "[Type=\"OE\"] X=%f:Y=%f:L=%d T=%d -> east border=%f (warning)";
				obs_error.code = VALIDATION_WARNING;
				validator_print_error(validator_ctx, &obs_error, o->pos.x, o->pos.y, l->levelnum, o->type, border);
			}
		}

		border = o->pos.y + obstacle_spec->upper_border;
		if (border < 0 && l->jump_target_north != -1) {
			struct obstacle_excpt_data to_check =
			    { 'N', {o->pos.x, o->pos.y, l->levelnum}, o->type, border };

			if (!lookup_exception(this, &to_check)) {
				obs_error.format = "[Type=\"ON\"] X=%f:Y=%f:L=%d T=%d -> north border=%f (warning)";
				obs_error.code = VALIDATION_WARNING;
				validator_print_error(validator_ctx, &obs_error, o->pos.x, o->pos.y, l->levelnum, o->type, border);
			}
		}

		border = o->pos.y + obstacle_spec->lower_border;
		if (border > max_y && l->jump_target_south != -1) {

			struct obstacle_excpt_data to_check =
			    { 'S', {o->pos.x, o->pos.y, l->levelnum}, o->type, border };

			if (!lookup_exception(this, &to_check)) {
				obs_error.format = "[Type=\"OS\"] X=%f:Y=%f:L=%d T=%d -> south border=%f (warning)";
				obs_error.code = VALIDATION_WARNING;
				validator_print_error(validator_ctx, &obs_error, o->pos.x, o->pos.y, l->levelnum, o->type, border);
			}
		}

	}

	validator_print_separator(validator_ctx);
}

//===========================================================
// Obstacle Extensions Validator
//
// This validator checks if obstacle extensions are valid
//===========================================================

struct extension_excpt_data {
	char subtest;
	gps obs_pos;
	int obs_index;
	int ext_type;
};

static void *lvlval_extensions_parse_excpt(char *string)
{
	char *validator_type = ReadAndMallocStringFromData(string, "Type=\"", "\"");
	if (!validator_type || strlen(validator_type) != 2) {
		ErrorMessage(__FUNCTION__, "The Subtest name of an exception is not valid!\n", PLEASE_INFORM, IS_FATAL);
		return NULL;
	}

	struct extension_excpt_data *data = (struct extension_excpt_data *)malloc(sizeof(struct extension_excpt_data));

	data->subtest = validator_type[1];
	free(validator_type);

	switch (data->subtest) {
	case 'S':
		ReadValueFromString(string, "X=", "%f", &(data->obs_pos.x), NULL);
		ReadValueFromString(string, "Y=", "%f", &(data->obs_pos.y), NULL);
		ReadValueFromString(string, "L=", "%d", &(data->obs_pos.z), NULL);
		ReadValueFromString(string, "ObsIdx=", "%d", &(data->obs_index), NULL);
		ReadValueFromString(string, "ExtType=", "%d", &(data->ext_type), NULL);
		break;

	default:
		ErrorMessage(__FUNCTION__, "The Subtest name of an exception is invalid!\n", PLEASE_INFORM, IS_FATAL);
		free(data);
		return NULL;
	}

	return (data);
}

static int lvlval_extensions_cmp_data(void *opaque_data1, void *opaque_data2)
{
#	define DIST_EPSILON 0.01f

	struct extension_excpt_data *data1 = opaque_data1;
	struct extension_excpt_data *data2 = opaque_data2;

	if (data1->subtest != data2->subtest)
		return FALSE;

	if (data1->obs_pos.z != data2->obs_pos.z)
		return FALSE;

	if (data1->obs_index != data2->obs_index)
		return FALSE;

	if (data1->ext_type != data2->ext_type)
		return FALSE;

	float dist = calc_distance(data1->obs_pos.x, data1->obs_pos.y, data2->obs_pos.x, data2->obs_pos.y);
	if (dist > DIST_EPSILON)
		return FALSE;

	return TRUE;

#	undef DIST_EPSILON
}

/*
 * 'obstacle extensions' validator
 */

static void lvlval_extensions_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx)
{
	struct lvlval_error extension_error = {
		.title = "Invalid obstacle extension",
		.comment = "The extension data of an obstacle is missing, or the data is wrong",
		.caught = FALSE,
		.code = VALIDATION_WARNING
	};

	int i;
	level *l = validator_ctx->this_level;

	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		obstacle *o = &l->obstacle_list[i];

		if (o->type == -1)
			continue;

		if (IS_SIGN(o->type)) {
			struct extension_excpt_data to_check =
				{ 'S', {o->pos.x, o->pos.y, l->levelnum}, get_obstacle_index(l, o), o->type };

			if (!lookup_exception(this, &to_check)) {
				if (!get_obstacle_extension(l, o, OBSTACLE_EXTENSION_SIGNMESSAGE)) {
					extension_error.format = "[Type=\"ES\"] X=%f:Y=%f:L=%d ObsIdx=%d ExtType=%d -> SIGNMESSAGE missing (warning)";
					validator_print_error(validator_ctx, &extension_error, o->pos.x, o->pos.y, l->levelnum, get_obstacle_index(l, o), o->type);
				}
			}
		}
	}

	validator_print_separator(validator_ctx);
}

//===========================================================
// Map Labels Validator
//
// This validator checks for duplicated map labels
//===========================================================

struct lvlval_map_label {
	const char *name;
	int levelnum;
};

static struct dynarray map_labels;

static void lvlval_map_labels_execute(struct level_validator *this, struct lvlval_ctx *validator_ctx)
{
	int i, j;
	struct lvlval_error map_label_error = {
		.title = "Duplicated map labels list",
		.comment = "The following map labels are not world-unique.",
		.format = "Map label '%s' declared on levels %d, %d",
		.caught = FALSE,
		.code = VALIDATION_ERROR
	};

	struct level *lvl = validator_ctx->this_level;
	for (i = 0; i < lvl->map_labels.size; i++) {
		struct map_label *current_label = dynarray_member(&lvl->map_labels, i, sizeof(struct map_label));

		// Search for duplicated map labels
		for (j = 0; j < map_labels.size; j++) {
			struct lvlval_map_label *label = dynarray_member(&map_labels, j, sizeof(struct lvlval_map_label));
			if (!strcmp(label->name, current_label->label_name)) {
				validator_print_error(validator_ctx, &map_label_error, label->name, lvl->levelnum, label->levelnum);
				break;
			}
		}

		if (j == map_labels.size) {
			struct lvlval_map_label label = { current_label->label_name, lvl->levelnum };
			dynarray_add(&map_labels, &label, sizeof(struct lvlval_map_label));
		}
	}

	validator_print_separator(validator_ctx);
}

//===========================================================
// ENTRY POINT
//
// Run several validations
//===========================================================

// With on screen report

int level_validation()
{
	enum validator_return_code final_rc = VALIDATION_PASS;
	int uncaught_excpt = FALSE;

	SDL_Rect background_rect = { UNIVERSAL_COORD_W(20), UNIVERSAL_COORD_H(20), UNIVERSAL_COORD_W(600), UNIVERSAL_COORD_H(440) };
	SDL_Rect report_rect = { UNIVERSAL_COORD_W(30), UNIVERSAL_COORD_H(30), UNIVERSAL_COORD_W(600), UNIVERSAL_COORD_H(430) };

	BFont_Info *current_font = GetCurrentFont();
	int row_height = FontHeight(current_font);
	int max_rows = (report_rect.h / row_height) - 4;	// 4 lines are reserved for header and footer
	int column_width = TextWidthFont(GetCurrentFont(), "000: empty");
	AssembleCombatPicture(ONLY_SHOW_MAP_AND_TEXT | SHOW_ITEMS | OMIT_TUX | GameConfig.omit_obstacles_in_level_editor *
				OMIT_OBSTACLES | GameConfig.omit_enemies_in_level_editor * OMIT_ENEMIES | OMIT_BLASTS | SKIP_LIGHT_RADIUS |
				NO_CURSOR | OMIT_ITEMS_LABEL);
	SetCurrentFont(current_font);	// Reset font, in case it was modified by AssembleCombatPicture()

	ShadowingRectangle(Screen, background_rect);

	// Title

	CenteredPutStringFont(Screen, FPS_Display_BFont, report_rect.y, "Level Validation tests - Summary\n");

	// Load exceptions rules

	load_excpt_lists("lvleditor_exceptions.dat");

	// Init map labels validator data
	dynarray_init(&map_labels, 1024, sizeof(struct lvlval_map_label));

	// Loop on each level

	int l;
	int col_pos = 0;
	int row_pos = 0;

	for (l = 0; l < curShip.num_levels; ++l) {
		struct lvlval_ctx validator_ctx = { &report_rect, curShip.AllLevels[l], FALSE, VALIDATION_PASS };

		// Compute row and column position, when a new column of text starts
		if ((l % max_rows) == 0) {
			col_pos = report_rect.x + (l / max_rows) * column_width;
			row_pos = report_rect.y + 2 * row_height;	// 2 lines are reserved for the header
		}

		if (!level_exists(l)) {
			// Empty level
			char txt[40];
 			sprintf(txt, "%03d: \2empty", l);
 			int lines = display_text_using_line_height(txt, col_pos, row_pos, &report_rect, 1.0);
 			row_pos += lines * row_height;
			SetCurrentFont(current_font);	// Reset font
		} else {
			// Loop on each validation function
			int v = 0;
			struct level_validator *one_validator;

			while (one_validator = &(level_validators[v++]), one_validator->execute != NULL)
				one_validator->execute(one_validator, &validator_ctx);

			// Display report
			char txt[40];
			switch (validator_ctx.return_code) {
			case VALIDATION_ERROR:
				sprintf(txt, "\2%03d: \1fail", l);
				break;
			case VALIDATION_WARNING:
				sprintf(txt, "\2%03d: \3warn", l);
				break;
			case VALIDATION_PASS:
			default:
				sprintf(txt, "\2%03d: \2pass", l);
				break;
			}
			int lines = display_text_using_line_height(txt, col_pos, row_pos, &report_rect, 1.0);
			row_pos += lines * row_height;
			SetCurrentFont(current_font);	// Reset font in case of the red "fail" was displayed

			// Set final return code
			compose_return_code(&final_rc, validator_ctx.return_code);
		}
	}

	// Outputs uncaught exception rules

	uncaught_excpt = print_uncaught_exceptions();
	
	free_exception_lists();

	// Free map labels validator data
	dynarray_free(&map_labels);

	// That's it.  We can say goodbye and return.

	int posy = report_rect.y + report_rect.h - row_height;

	CenteredPutStringFont(Screen, FPS_Display_BFont, posy, "--- End of List --- Press Space to return to leveleditor ---");

	if (final_rc != VALIDATION_PASS) {
		posy -= row_height;
		CenteredPutStringFont(Screen, FPS_Display_BFont, posy, "\1Some tests were invalid. See the report in the console\3");
	}

	if (uncaught_excpt) {
		posy -= row_height;
		CenteredPutStringFont(Screen, FPS_Display_BFont, posy, "\1Some exceptions were not caught. See the report in the console\3");
	}

	our_SDL_flip_wrapper();

	while (!SpacePressed() && !EnterPressed() && !MouseLeftPressed() && !EscapePressed())
		SDL_Delay(1);
	// Hack: eat all pending events.
	input_handle();

	return (final_rc != VALIDATION_PASS);
}

// Without on screen report

int level_validation_on_console_only()
{
	enum validator_return_code final_rc = VALIDATION_PASS;
	int uncaught_excpt = FALSE;
	SDL_Rect report_rect = { 0, 0, 0, 0 };
	
	// Load exceptions rules

	load_excpt_lists("lvleditor_exceptions.dat");

	// Init map labels validator data
	dynarray_init(&map_labels, 1024, sizeof(struct lvlval_map_label));

	// Loop on each level

	int l;

	for (l = 0; l < curShip.num_levels; ++l) {
		struct lvlval_ctx validator_ctx = { &report_rect, curShip.AllLevels[l], FALSE, VALIDATION_PASS };

		// Nota: we do not currently validate random dungeons, due to a known
		// invalid waypoint generation.
		
		if (level_exists(l) && !curShip.AllLevels[l]->random_dungeon) {
			// Loop on each validation function
			int v = 0;
			struct level_validator *one_validator;

			while (one_validator = &(level_validators[v++]), one_validator->execute != NULL)
				one_validator->execute(one_validator, &validator_ctx);

			// Set final return code
			compose_return_code(&final_rc, validator_ctx.return_code);
		}
	}

	// Outputs uncaught exception rules

	uncaught_excpt = print_uncaught_exceptions();
	
	free_exception_lists();

	// Free map labels validator data
	dynarray_free(&map_labels);

	return (final_rc == VALIDATION_ERROR) || uncaught_excpt;
}

#undef IS_CHEST
#undef IS_BARREL
#undef IS_SIGN

#undef _leveleditor_validator_c
