/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2010 Arthur Huillet 
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

#define _mission_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "map.h"
#include "proto.h"

/**
 * This function is responsible for making a new quest diary entry 
 * visible inside the quest browser.
 */
void mission_diary_add(const char *mis_name, const char *diarytext)
{
	int mis_num = get_mission_index_by_name(mis_name);
	struct mission *quest = (struct mission *)dynarray_member(&Me.missions, mis_num, sizeof(struct mission));

	int idx = 0;

	while (idx < MAX_MISSION_DESCRIPTION_TEXTS && quest->mission_diary_texts[idx])
		idx++;

	if (idx >= MAX_MISSION_DESCRIPTION_TEXTS) {
		error_message(__FUNCTION__,
			     "There is no more room to append mission diary text \"%s\" to mission name \"%s\". Doing nothing.",
			     PLEASE_INFORM, diarytext, mis_name);
		return;
	}

	quest->mission_description_time[idx] = Me.current_game_date;
	quest->mission_diary_texts[idx] = strdup(diarytext);

	Me.quest_browser_changed = 1;
}

/*----------------------------------------------------------------------
 * This function checks, if the influencer has succeeded in his given 
 * mission.  If not it returns, if yes the EndTitle/Debriefing is
 * started.
 ----------------------------------------------------------------------*/
void check_if_mission_is_complete(void)
{
	static int check_mission_timestamp = 0;

	// We do not need to check for mission completed EVERY frame
	// It will be enough to do it now and then..., e.g. every 50th frame

	check_mission_timestamp++;
	if ((check_mission_timestamp % 50) != 0)
		return;

	for (int mis_num = 0; mis_num < Me.missions.size; mis_num++) {
		struct mission *quest = (struct mission *)dynarray_member(&Me.missions, mis_num, sizeof(struct mission));

		// We do not need to do anything if the mission has already failed or
		// if the mission is already completed or if the mission does not exist
		// at all or if the mission was not assigned yet

		if (quest->MissionIsComplete == TRUE)
			continue;
		if (quest->MissionWasFailed == TRUE)
			continue;
		if (quest->MissionWasAssigned != TRUE)
			continue;

		int this_mission_seems_completed = TRUE;
		int checked_one_criterion = FALSE;

		// Continue if the Mission target KillMarker is given but not fulfilled

		if (quest->KillMarker != -1) {
			struct enemy *erot;
			BROWSE_ALIVE_BOTS(erot) {
				if (erot->marker == quest->KillMarker) {
					this_mission_seems_completed = FALSE;
					break;
				}

			}
			checked_one_criterion = TRUE;
		}

		// Continue if the Mission target must_clear_level is given but not fulfilled

		if (quest->must_clear_level != -1) {
			struct enemy *erot;
			BROWSE_LEVEL_BOTS(erot, quest->must_clear_level) {
				if (!is_friendly(erot->faction, FACTION_SELF)) {
					this_mission_seems_completed = FALSE;
					break;
				}
			}
			checked_one_criterion = TRUE;
		}

		// If the mission actually had criteria we checked for, and those criteria
		// are OK, the mission is finished.

		if (checked_one_criterion && this_mission_seems_completed)
			complete_mission(quest->mission_name);

	}
}

/**
 * This function assigns a new mission to the player, which means 
 * that the status of the mission in the mission array is changed and
 * perhaps the mission log activated.
 */
void assign_mission(const char *name)
{
	int mis_num = get_mission_index_by_name(name);

	struct mission *quest = (struct mission *)dynarray_member(&Me.missions, mis_num, sizeof(struct mission));
	quest->MissionWasAssigned = TRUE;

	if (quest->assignment_lua_code)
		run_lua(LUA_DIALOG, quest->assignment_lua_code);

	Me.quest_browser_changed = 1;
};

/**
 * This function marks a mission as complete
 */
void complete_mission(const char *name)
{
	int mis_num = get_mission_index_by_name(name);

	struct mission *quest = (struct mission *)dynarray_member(&Me.missions, mis_num, sizeof(struct mission));
	quest->MissionIsComplete = TRUE;

	if (quest->completion_lua_code)
		run_lua(LUA_DIALOG, quest->completion_lua_code);

	Me.quest_browser_changed = 1;
}

/**
 * At the start of every new game, the mission info (i.e. which missions
 * are already assigned, completed, failed, available and such) should
 * be reset to default state, so that no zombie mission entries can appear. 
 */
void clear_tux_mission_info()
{
	for (int i = 0; i < Me.missions.size; i++) {
		struct mission *quest = (struct mission *)dynarray_member(&Me.missions, i, sizeof(struct mission));

		if (quest->mission_name) {
			free(quest->mission_name);
		}
		if (quest->completion_lua_code) {
			free(quest->completion_lua_code);
		}
		if (quest->assignment_lua_code) {
			free(quest->assignment_lua_code);
		}
	}

	dynarray_free(&Me.missions);
}

/**
 * This function reads the mission specifications from the mission file
 * which is assumed to be loaded into memory already.
 */
void get_quest_list(char *quest_list_filename)
{
	char fpath[PATH_MAX];

	find_file(fpath, MAP_DIR, quest_list_filename, NULL, PLEASE_INFORM | IS_FATAL);
	run_lua_file(LUA_CONFIG, fpath);
}

/**
 * This function returns the mission that has the specified name.
 */
int get_mission_index_by_name(const char *name)
{
	for (int cidx = 0; cidx < Me.missions.size; cidx++) {
		struct mission *quest = (struct mission *)dynarray_member(&Me.missions, cidx, sizeof(struct mission));
		const char *n = quest->mission_name;
		if (!n)
			continue;

		if (!strcmp(n, name))
			return cidx;
	}

	error_message(__FUNCTION__, "Unable to find mission named \"%s\"", PLEASE_INFORM | IS_FATAL, name);
	return -1;
}

#undef _mission_c
