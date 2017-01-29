/*
 *
 *   Copyright (c) 2017 Samuel Degrande
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
 * This file contains miscellaneous functions to manage game acts.
 */
#define _game_act_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

static struct game_act *current_game_act = NULL;

/*
 * Replace the keyword "$ACT", if found, in 'unresolved_path' by the content of 'act'
 * Return TRUE if "$ACT" was found and replaced
 */
static void game_act_resolve_path(char* unresolved_path, struct game_act *act)
{
	const char *key = "$ACT";
	char *tmp = strdup(unresolved_path);
	char *start = strstr(tmp, key);
	if (start) {
		*start = '\0';
		int nb = snprintf(unresolved_path, PATH_MAX, "%s%s%s", tmp, act->subdir, start+strlen(key));
		if (nb >= PATH_MAX) {
			error_message(__FUNCTION__, "data_dirs[].path is not big enough to store the following path: %s%s%s",
			             PLEASE_INFORM | IS_FATAL, tmp, act->subdir, start+strlen(key));
		}
	}
	free(tmp);
}

/*
 * Compute the path of the data dirs related to game act (replacing the $ACT keyword
 * using the current game act)
 */
static void game_act_set_data_dirs_path(struct game_act *act)
{
	// Resolve the $ACT key in the data dir paths
	int path_indices[] = { MAP_DIR, MAP_TITLES_DIR, MAP_DIALOG_DIR };
	for (int i = 0; i < sizeof(path_indices)/sizeof(path_indices[0]); i++) {
		// Restore default path (including the $ACT keyword)
		int nb = snprintf(data_dirs[path_indices[i]].path, PATH_MAX, "%s/%s", data_dirs[DATA_ROOT].path, data_dirs[path_indices[i]].name);
		if (nb >= PATH_MAX) {
			error_message(__FUNCTION__, "data_dirs[].path is not big enough to store the following path: %s/%s",
			             PLEASE_INFORM | IS_FATAL, data_dirs[DATA_ROOT].path, data_dirs[path_indices[i]].name);
		}
		// Resolve the $ACT keyword
		game_act_resolve_path(data_dirs[path_indices[i]].path, act);
	}
}

/**
 * Check if a game act subdir actually exists
 *
 * @param act Pointer to the game_act struct to validate
 *
 * @return TRUE is the subdir exists, FALSE otherwise
 */
int game_act_validate(struct game_act *act)
{
	char *act_dir = strdup(data_dirs[MAP_DIR].name);
	game_act_resolve_path(act_dir, act);
	int exists = check_directory(act_dir, DATA_ROOT, FALSE, SILENT);
	free(act_dir);
	if (exists == 0)
		return TRUE;
	return FALSE;
}

/**
 * Return the starting game act (as defined in game_config.lua)
 *
 * @return Pointer to a game_act struct
 */
struct game_act *game_act_get_starting()
{
	for (int i = 0; i < game_acts.size; i++) {
		struct game_act *act = (struct game_act *)dynarray_member(&game_acts, i, sizeof(struct game_act));
		if (act->starting_act)
			return act;
	}
	return NULL;
}

/**
 * Return a game act given its name
 *
 * @param The name of the requested game act
 *
 * @return Pointer to a game_act struct, or NULL if not found
 */
struct game_act *game_act_get_by_name(char *act_name)
{
	if (!act_name || !strlen(act_name))
		return NULL;

	for (int i = 0; i < game_acts.size; i++) {
		struct game_act *act = (struct game_act *)dynarray_member(&game_acts, i, sizeof(struct game_act));
		if (!strcmp(act->name, act_name))
			return act;
	}
	return NULL;
}

/**
 * Set the current game act.
 *
 * Store 'act' into 'curent_act', and set the data dirs accordingly to that game act.
 *
 * @param act Pointer to the game_act struct to set as the current one
 */
void game_act_set_current(struct game_act *act)
{
	current_game_act = act;
	game_act_set_data_dirs_path(act);
}

/**
 * Return the current game act (as set by game_act_set_current())
 *
 * @return Pointer to a game_act struct
 */
struct game_act *game_act_get_current()
{
	if (!current_game_act) {
		error_message(__FUNCTION__, "Current game act is not yet set. act_set_current() must be called before. We can not continue...",
		              PLEASE_INFORM | IS_FATAL);

	}
	return current_game_act;
}
