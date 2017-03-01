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
static struct game_act *next_game_act = NULL;

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
 * @param name The name of the requested game act
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
 * Return a game act given its identifier
 *
 * @param id The idientifer of the requested game act
 *
 * @return Pointer to a game_act struct, or NULL if not found
 */
struct game_act *game_act_get_by_id(char *act_id)
{
	if (!act_id || !strlen(act_id))
		return NULL;

	for (int i = 0; i < game_acts.size; i++) {
		struct game_act *act = (struct game_act *)dynarray_member(&game_acts, i, sizeof(struct game_act));
		if (!strcmp(act->id, act_id))
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

/**
 * Free the ememory used to store game acts data
 */
void game_act_free()
{
	for (int i = 0; i < game_acts.size; i++) {
		struct game_act *act = (struct game_act *)dynarray_member(&game_acts, i, sizeof(struct game_act));
		free(act->id);
		free(act->name);
		free(act->subdir);
	}
	dynarray_free(&game_acts);
}

/**
 * Set the next game act to jump to
 *
 * Called by an event or a dialog to store the next game act to run
 * once the event or the dialog is closed.
 *
 * @param act_id The identifier of the next game act
 */
void game_act_set_next(const char *act_id)
{
	struct game_act *next_act = game_act_get_by_id((char *)act_id);
	if (!next_act) {
		error_message(__FUNCTION__, "The requested game act (%s) is unknown. We can not proceed.",
		              PLEASE_INFORM, (strlen(act_id) != 0) ? act_id : "NULL");
		return;
	}
	next_game_act = next_act;
}

int game_act_finished()
{
	return (next_game_act != NULL);
}

void game_act_switch_to_next()
{
	if (!next_game_act) {
		error_message(__FUNCTION__, "The next game act to jump to is not defined. We can not proceed.",
		              PLEASE_INFORM);
		return;
	}

	play_title_file(MAP_TITLES_DIR, "EndOfAct.lua");

	game_act_set_current(next_game_act);
	next_game_act = NULL;

	free_game_data();
	lightly_free_tux();

	char fp[PATH_MAX];
	find_file(fp, MAP_DIR, "levels.dat", NULL, PLEASE_INFORM | IS_FATAL);
	LoadShip(fp, 0);

	skip_initial_menus = 1;
	prepare_start_of_new_game("NewTuxStartGameSquare", FALSE);
	skip_initial_menus = 0;

	play_title_file(MAP_TITLES_DIR, "StartOfAct.lua");
}
