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
	int mis_num = GetMissionIndexByName(mis_name);
	int idx = 0;

	while (idx < MAX_MISSION_DESCRIPTION_TEXTS && Me.AllMissions[mis_num].mission_diary_texts[idx])
		idx++;

	if (idx >= MAX_MISSION_DESCRIPTION_TEXTS) {
		error_message(__FUNCTION__,
			     "There is no more room to append mission diary text \"%s\" to mission name \"%s\". Doing nothing.",
			     PLEASE_INFORM, diarytext, mis_name);
		return;
	}


	Me.AllMissions[mis_num].mission_description_time[idx] = Me.current_game_date;

	Me.AllMissions[mis_num].mission_diary_texts[idx] = strdup(diarytext);

	Me.quest_browser_changed = 1;
};

/*----------------------------------------------------------------------
 * This function checks, if the influencer has succeeded in his given 
 * mission.  If not it returns, if yes the EndTitle/Debriefing is
 * started.
 ----------------------------------------------------------------------*/
void CheckIfMissionIsComplete(void)
{
	int mis_num;
	static int CheckMissionGrid;
	int this_mission_seems_completed;
	int checked_one_criterion;

#define MIS_COMPLETE_DEBUG 1

	// We do not need to check for mission completed EVERY frame
	// It will be enough to do it now and then..., e.g. every 50th frame
	//
	CheckMissionGrid++;
	if ((CheckMissionGrid % 50) != 0)
		return;

	for (mis_num = 0; mis_num < MAX_MISSIONS_IN_GAME; mis_num++) {
		// We need not do anything, if the mission has already failed or if
		// the mission is already completed or if the mission does not exist
		// at all or if the mission was not assigned yet
		//
		if (Me.AllMissions[mis_num].MissionIsComplete == TRUE)
			continue;
		if (Me.AllMissions[mis_num].MissionWasFailed == TRUE)
			continue;
		if (Me.AllMissions[mis_num].MissionExistsAtAll != TRUE)
			continue;
		if (Me.AllMissions[mis_num].MissionWasAssigned != TRUE)
			continue;

		DebugPrintf(MIS_COMPLETE_DEBUG, "\nSomething was assigned at all..... mis_num = %d ", mis_num);

		this_mission_seems_completed = TRUE;
		checked_one_criterion = FALSE;

		// Continue if the Mission target KillMarker is given but not fulfilled
		//
		if (Me.AllMissions[mis_num].KillMarker != (-1)) {
			enemy *erot;
			BROWSE_ALIVE_BOTS(erot) {
				if (erot->marker == Me.AllMissions[mis_num].KillMarker) {
					DebugPrintf(MIS_COMPLETE_DEBUG,
						    "\nOne of the marked droids is still alive... (%p at %f:%f on %d)\n", erot,
						    erot->pos.x, erot->pos.y, erot->pos.z);
					this_mission_seems_completed = FALSE;
					break;
				}

			}
			checked_one_criterion = TRUE;
		}
		// Continue if the Mission target must_clear_level is given but not fulfilled
		//
		if (Me.AllMissions[mis_num].must_clear_level != (-1)) {
			enemy *erot;
			BROWSE_LEVEL_BOTS(erot, Me.AllMissions[mis_num].must_clear_level) {
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
			CompleteMission(Me.AllMissions[mis_num].mission_name);

	}
};				// void CheckIfMissionIsComplete

/**
 * This function assigns a new mission to the player, which means 
 * that the status of the mission in the mission array is changed and
 * perhaps the mission log activated.
 */
void AssignMission(const char *name)
{
	int MissNum = GetMissionIndexByName(name);

	Me.AllMissions[MissNum].MissionWasAssigned = TRUE;

	if (Me.AllMissions[MissNum].assignment_lua_code)
		run_lua(LUA_DIALOG, Me.AllMissions[MissNum].assignment_lua_code);

	Me.quest_browser_changed = 1;
};

/**
 * This function marks a mission as complete
 */
void CompleteMission(const char *name)
{
	int MissNum = GetMissionIndexByName(name);

	Me.AllMissions[MissNum].MissionIsComplete = TRUE;

	if (Me.AllMissions[MissNum].completion_lua_code)
		run_lua(LUA_DIALOG, Me.AllMissions[MissNum].completion_lua_code);

	Me.quest_browser_changed = 1;
}

/**
 * At the start of every new game, the mission info (i.e. which missions
 * are already assigned, completed, failed, available and such) should
 * be reset to default state, so that no zombie mission entries can appear. 
 */
void clear_tux_mission_info()
{
	int i;
	int diary_entry_nr;

	for (i = 0; i < MAX_MISSIONS_IN_GAME; i++) {
		Me.AllMissions[i].MissionExistsAtAll = FALSE;
		Me.AllMissions[i].MissionIsComplete = FALSE;
		Me.AllMissions[i].MissionWasFailed = FALSE;
		Me.AllMissions[i].MissionWasAssigned = FALSE;

		if (Me.AllMissions[i].mission_name) {
			free(Me.AllMissions[i].mission_name);
			Me.AllMissions[i].mission_name = NULL;
		}

		for (diary_entry_nr = 0; diary_entry_nr < MAX_MISSION_DESCRIPTION_TEXTS; diary_entry_nr++) {
			if (Me.AllMissions[i].mission_diary_texts[diary_entry_nr]) {
				free(Me.AllMissions[i].mission_diary_texts[diary_entry_nr]);
				Me.AllMissions[i].mission_diary_texts[diary_entry_nr] = NULL;
			}

			Me.AllMissions[i].mission_description_time[diary_entry_nr] = 0;
		}

		if (Me.AllMissions[i].completion_lua_code) {
			free(Me.AllMissions[i].completion_lua_code);
			Me.AllMissions[i].completion_lua_code = NULL;
		}
		
		if (Me.AllMissions[i].assignment_lua_code) {
			free(Me.AllMissions[i].assignment_lua_code);
			Me.AllMissions[i].assignment_lua_code = NULL;
		}
	}
}

/**
 * This function reads the mission specifications from the mission file
 * which is assumed to be loaded into memory already.
 */
void get_quest_list(char *quest_list_filename)
{
#define MISSION_TARGET_SUBSECTION_START_STRING "** Start of this mission target subsection **"
#define MISSION_TARGET_SUBSECTION_END_STRING "** End of this mission target subsection **"

#define MISSION_TARGET_NAME_INITIALIZER "Mission Name=_\""

#define MISSION_TARGET_KILL_MARKER "Mission target is to kill droids with marker : "
#define MISSION_TARGET_MUST_CLEAR_LEVEL "Mission target is to kill all hostile droids on this level : "

#define MISSION_ASSIGNMENT_LUACODE_STRING "Assignment LuaCode={"
#define MISSION_COMPLETION_LUACODE_STRING "Completion LuaCode={"

	// At first we must load the quest list file given...

	char fpath[PATH_MAX];
	find_file(fpath, MAP_DIR, quest_list_filename, NULL, PLEASE_INFORM | IS_FATAL);
	char *mission_file_contents = read_and_malloc_and_terminate_file(fpath, "*** END OF QUEST LIST *** LEAVE THIS TERMINATOR IN HERE ***");
	char *mission_target_pointer = mission_file_contents;

	int mission_target_index = 0;
	for (mission_target_index = 0; mission_target_index < MAX_MISSIONS_IN_GAME; mission_target_index++) {
		Me.AllMissions[mission_target_index].MissionExistsAtAll = FALSE;
	}

	mission_target_index = 0;
	while ((mission_target_pointer = strstr(mission_target_pointer, MISSION_TARGET_SUBSECTION_START_STRING)) != NULL) {
		char *end_of_mission_target_pointer = LocateStringInData(mission_target_pointer, MISSION_TARGET_SUBSECTION_END_STRING);

		if (mission_target_index >= MAX_MISSIONS_IN_GAME)
			error_message(__FUNCTION__, "The number of quests specified in %s exceeds MAX_MISSIONS_IN_GAME (%d).",
				     PLEASE_INFORM | IS_FATAL, quest_list_filename, MAX_MISSIONS_IN_GAME);

		// We need to add an inner terminator here, so that the strstr operation
		// below will know where to stop within this subsection.
		//
		char inner_preserved_letter = *end_of_mission_target_pointer;
		*end_of_mission_target_pointer = 0;

		Me.AllMissions[mission_target_index].MissionExistsAtAll = TRUE;

		Me.AllMissions[mission_target_index].mission_name = ReadAndMallocStringFromData(mission_target_pointer, MISSION_TARGET_NAME_INITIALIZER, "\"");

		// From here on we read the details of the mission target, i.e. what the
		// influencer has to do, so that the mission can be thought of as completed

		ReadValueFromStringWithDefault(mission_target_pointer, MISSION_TARGET_KILL_MARKER, "%d", "-1",
				    &Me.AllMissions[mission_target_index].KillMarker, end_of_mission_target_pointer);

		ReadValueFromStringWithDefault(mission_target_pointer, MISSION_TARGET_MUST_CLEAR_LEVEL, "%d", "-1",
				    &Me.AllMissions[mission_target_index].must_clear_level, end_of_mission_target_pointer);

		if (strstr(mission_target_pointer, MISSION_COMPLETION_LUACODE_STRING)) {
			Me.AllMissions[mission_target_index].completion_lua_code =
			    ReadAndMallocStringFromData(mission_target_pointer, MISSION_COMPLETION_LUACODE_STRING, "}");
		} else {
			Me.AllMissions[mission_target_index].completion_lua_code = NULL;
		}

		if (strstr(mission_target_pointer, MISSION_ASSIGNMENT_LUACODE_STRING)) {
			Me.AllMissions[mission_target_index].assignment_lua_code =
			    ReadAndMallocStringFromData(mission_target_pointer, MISSION_ASSIGNMENT_LUACODE_STRING, "}");
		} else {
			Me.AllMissions[mission_target_index].assignment_lua_code = NULL;
		}
		// Now it is time to read in the mission diary entries, that might
		// be displayed in the quest browser later.

		int diary_entry_nr;
		for (diary_entry_nr = 0; diary_entry_nr < MAX_MISSION_DESCRIPTION_TEXTS; diary_entry_nr++) {
			Me.AllMissions[mission_target_index].mission_diary_texts[diary_entry_nr] = NULL;
		}

		// Now we are done with reading in THIS one mission target
		// We need to advance the MissionTargetPointer, so that we avoid doubly
		// reading in this mission OR ONE OF THIS MISSIONS VALUES!!!!
		// 
		// And we need of course to advance the array index for mission targets too...

		mission_target_pointer = end_of_mission_target_pointer;	// to avoid double entering the same target
		mission_target_index++;	// to avoid overwriting the same entry again

		// We restore the termination character we added before, even if that
		// is maybe not really necessary...

		*end_of_mission_target_pointer = inner_preserved_letter;

	}

	// Finally we record the number of mission targets scanned and are done with this function
	DebugPrintf(1, "\nNUMBER OF MISSION TARGETS FOUND: %d.\n", mission_target_index);
	fflush(stdout);
	free(mission_file_contents);
}

/**
 * This function returns the mission that has the specified name.
 */
int GetMissionIndexByName(const char *name)
{
	int cidx;

	for (cidx = 0; cidx < MAX_MISSIONS_IN_GAME; cidx++) {
		const char *n = Me.AllMissions[cidx].mission_name;
		if (!n)
			continue;

		if (!strcmp(n, name))
			return cidx;
	}

	error_message(__FUNCTION__, "Unable to find mission named \"%s\"", PLEASE_INFORM | IS_FATAL, name);
	return -1;
}

#undef _mission_c
