/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2007 Arthur Huillet 
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
 * This file contains all the functions managing the things one gets to see.
 * That includes assembling of enemys, assembling the currently
 * relevant porting of the map (the bricks I mean), drawing all visible
 * elements like bullets, blasts, enemys or influencer in a nonvisible
 * place in memory at first, and finally drawing them to the visible
 * screen for the user.
 */

#define _mission_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "map.h"
#include "proto.h"
#include "SDL_rotozoom.h"

int currently_selected_mission = (-1);
SDL_Rect mission_list_rect = { 20, 280, 280, 180 };
SDL_Rect mission_description_rect = { 134, 86, 280, 320 };

char complete_mission_display_text[50000];
float mission_list_offset = 0;
int mission_list_scroll_override_from_user = 0;

#define QUEST_BROWSER_SHOW_OPEN_MISSIONS (-1011)
#define QUEST_BROWSER_SHOW_DONE_MISSIONS (-1012)
#define QUEST_BROWSER_SHOW_NOTES (-1013)

int current_quest_browser_mode = QUEST_BROWSER_SHOW_OPEN_MISSIONS;

int quest_browser_mission_lines_needed[MAX_MISSIONS_IN_GAME];

/**
 * This function is responsible for making a new quest diary entry 
 * visible inside the quest browser.
 */
void quest_browser_diary_add(const char *mis_name, const char *diarytext)
{
	int mis_num = GetMissionIndexByName(mis_name);
	int idx = 0;

	while (Me.AllMissions[mis_num].mission_description_visible[idx] && idx < MAX_MISSION_DESCRIPTION_TEXTS)
		idx++;

	if (idx >= MAX_MISSION_DESCRIPTION_TEXTS) {
		ErrorMessage(__FUNCTION__,
			     "There is no more room to append mission diary text \"%s\" to mission name \"%s\". Doing nothing.\n",
			     PLEASE_INFORM, IS_WARNING_ONLY, diarytext, mis_name);
		return;
	}

	Me.AllMissions[mis_num].mission_description_visible[idx] = TRUE;
	Me.AllMissions[mis_num].mission_description_time[idx] = Me.current_game_date;

	mission_diary_texts[mis_num][idx] = strdup(diarytext);
};

/**
 * If there is some mission selected inside the quest browser, then we
 * should also display all info on the current status and history of that
 * particular mission, which is exactly what this function is responsible
 * for.
 */
void quest_browser_append_mission_info(const char *mis_name, int full_description)
{
	char temp_text[10000];
	int mission_diary_index;
	int mis_num = GetMissionIndexByName(mis_name);

	SetTextCursor(mission_description_rect.x, mission_description_rect.y);

	strcat(complete_mission_display_text, _("Mission: "));
	strcat(complete_mission_display_text, _(Me.AllMissions[mis_num].MissionName));
	strcat(complete_mission_display_text, "\n");

	//--------------------
	// Depending on whether we want the full mission description or only
	// some closed file description, we can either return or must continue
	// to the detailed description.
	//
	if (!full_description)
		return;

	strcat(complete_mission_display_text, _("Status: "));
	if (Me.AllMissions[mis_num].MissionIsComplete)
		strcat(complete_mission_display_text, _("COMPLETE"));
	else if (Me.AllMissions[mis_num].MissionWasFailed)
		strcat(complete_mission_display_text, _("FAILED"));
	else
		strcat(complete_mission_display_text, _("STILL OPEN"));
	strcat(complete_mission_display_text, _("\nDetails:\n"));

	for (mission_diary_index = 0; mission_diary_index < MAX_MISSION_DESCRIPTION_TEXTS; mission_diary_index++) {
		if (Me.AllMissions[mis_num].mission_description_visible[mission_diary_index]) {
			sprintf(temp_text, _("Day %d  %02d:%02d"),
				get_days_of_game_duration(Me.AllMissions[mis_num].mission_description_time[mission_diary_index]),
				get_hours_of_game_duration(Me.AllMissions[mis_num].mission_description_time[mission_diary_index]),
				get_minutes_of_game_duration(Me.AllMissions[mis_num].mission_description_time[mission_diary_index]));
			strcat(complete_mission_display_text, "-------- ");
			strcat(complete_mission_display_text, temp_text);
			strcat(complete_mission_display_text, " --------\n ");
			strcat(complete_mission_display_text, D_(mission_diary_texts[mis_num][mission_diary_index]));
			strcat(complete_mission_display_text, "\n");
		}
	}

};				// void quest_browser_append_mission_info ( int mis_num )

extern int MyCursorY;
/**
 *
 *
 */
void quest_browser_display_mission_list(int list_type)
{
	int mis_num;
	int something_was_displayed = FALSE;

	strcpy(complete_mission_display_text, "");

	for (mis_num = 0; mis_num < MAX_MISSIONS_IN_GAME; mis_num++) {
		//--------------------
		// The mission short/long symbol positions must be initialized
		// with some default values.
		//
		quest_browser_mission_lines_needed[mis_num] = (-1);

		// In case the mission does not exist at all, we need not do anything more...
		if (Me.AllMissions[mis_num].MissionExistsAtAll != TRUE)
			continue;

		//  In case the mission was not yet assigned, we need not do anything more...
		if (Me.AllMissions[mis_num].MissionWasAssigned != TRUE)
			continue;

		//--------------------
		// We record the number of lines needed so far, so that we may later
		// draw suitable long/short buttons in front of the text, so that the
		// user can then select to see the full/short information on this quest.
		//
		if (list_type != QUEST_BROWSER_SHOW_NOTES) {
			quest_browser_mission_lines_needed[mis_num] =
			    GetNumberOfTextLinesNeeded(complete_mission_display_text, mission_description_rect, TEXT_STRETCH);
			DebugPrintf(2, "\n%s(): new mission start pos at lines needed: %d.",
				    __FUNCTION__, quest_browser_mission_lines_needed[mis_num]);
		}

		if ((list_type == QUEST_BROWSER_SHOW_OPEN_MISSIONS) && (Me.AllMissions[mis_num].MissionIsComplete == FALSE)) {
			quest_browser_append_mission_info(Me.AllMissions[mis_num].MissionName,
							  Me.AllMissions[mis_num].expanded_display_for_this_mission);
			something_was_displayed = TRUE;
		} else if ((list_type == QUEST_BROWSER_SHOW_DONE_MISSIONS) && (Me.AllMissions[mis_num].MissionIsComplete != FALSE)) {
			quest_browser_append_mission_info(Me.AllMissions[mis_num].MissionName,
							  Me.AllMissions[mis_num].expanded_display_for_this_mission);
			something_was_displayed = TRUE;
		} else {
			quest_browser_mission_lines_needed[mis_num] = (-1);
		}

	}

	SetTextCursor(mission_description_rect.x, mission_description_rect.y);

	if (something_was_displayed) {
		mission_list_offset = (FontHeight(GetCurrentFont()) * TEXT_STRETCH)
		    * (mission_list_scroll_override_from_user) * 1.00;
		SetTextCursor(mission_description_rect.x, mission_description_rect.y);
		DisplayText(complete_mission_display_text, mission_description_rect.x,
			    mission_description_rect.y - mission_list_offset, &mission_description_rect, TEXT_STRETCH);

		//--------------------
		// Now it's time to display some short/long symbols in front
		// of each of the missions.
		//
		for (mis_num = 0; mis_num < MAX_MISSIONS_IN_GAME; mis_num++) {
			SDL_Rect *rect_short = &(AllMousePressButtons[QUEST_BROWSER_ITEM_SHORT_BUTTON].button_rect);
			SDL_Rect *rect_long = &(AllMousePressButtons[QUEST_BROWSER_ITEM_LONG_BUTTON].button_rect);
			if (!Me.AllMissions[mis_num].MissionWasAssigned)
				continue;
			if ((list_type == QUEST_BROWSER_SHOW_OPEN_MISSIONS) && (Me.AllMissions[mis_num].MissionIsComplete == TRUE))
				continue;
			if ((list_type == QUEST_BROWSER_SHOW_DONE_MISSIONS) && (Me.AllMissions[mis_num].MissionIsComplete == FALSE))
				continue;
			//--------------------
			// At first we bring the short/long buttons into position.
			// This position might be well off the screen.  That's no
			// problem, because we won't blit the thing in this case
			// anyway.
			//
			if (quest_browser_mission_lines_needed[mis_num] != (-1)) {
				rect_short->y =
				    mission_description_rect.y - mission_list_offset +
				    (FontHeight(GetCurrentFont()) * TEXT_STRETCH) * (quest_browser_mission_lines_needed[mis_num] - 1) - 3;
				rect_short->x = mission_description_rect.x - rect_short->w;
				rect_long->y = rect_short->y;
				rect_long->x = rect_short->x;
			}
			//--------------------
			// Now we check if the y coordinate of the buttons are 
			// somewhat reasonable or not.  For those buttons that are
			// off the screen, things are simple, because then we can
			// skip the rest of this pass of the loop.
			//
			if (rect_short->y <= mission_description_rect.y - 4)
				continue;
			if (rect_short->y >= mission_description_rect.y + mission_description_rect.h - FontHeight(GetCurrentFont()))
				continue;

			if (Me.AllMissions[mis_num].expanded_display_for_this_mission)
				ShowGenericButtonFromList(QUEST_BROWSER_ITEM_LONG_BUTTON);
			else
				ShowGenericButtonFromList(QUEST_BROWSER_ITEM_SHORT_BUTTON);

			if (MouseLeftPressed()) {
				if (MouseCursorIsOnButton(QUEST_BROWSER_ITEM_SHORT_BUTTON, GetMousePos_x(), GetMousePos_y())) {
					Me.AllMissions[mis_num].expanded_display_for_this_mission =
					    !Me.AllMissions[mis_num].expanded_display_for_this_mission;
					while (MouseLeftPressed()) ;
				}
			}
		}
	} else {
		switch (list_type) {
		case QUEST_BROWSER_SHOW_OPEN_MISSIONS:
			DisplayText(_("No open quests yet."), -1, -1, &mission_description_rect, TEXT_STRETCH);
			break;
		case QUEST_BROWSER_SHOW_DONE_MISSIONS:
			DisplayText(_("No completed quests yet."), -1, -1, &mission_description_rect, TEXT_STRETCH);
			break;
		case QUEST_BROWSER_SHOW_NOTES:
			DisplayText(_("No notes yet."), -1, -1, &mission_description_rect, TEXT_STRETCH);
			break;
		default:
			ErrorMessage(__FUNCTION__, "\
Illegal quest browser status encountered.", PLEASE_INFORM, IS_FATAL);
			break;
		}
	}

	ShowGenericButtonFromList(QUEST_BROWSER_SCROLL_UP_BUTTON);
	ShowGenericButtonFromList(QUEST_BROWSER_SCROLL_DOWN_BUTTON);

};				// void quest_browser_display_mission_list ( void )

/**
 * This function manages the quest browser.
 */
void quest_browser_interface(void)
{
	int back_to_game = FALSE;
	int old_game_status = game_status;
	static int first_call = TRUE;

	game_status = INSIDE_MENU;

	//--------------------
	// On the very first 
	if (first_call) {
		first_call = FALSE;
		mission_description_rect.x *= (((float)GameConfig.screen_width) / 640.0);
		mission_description_rect.y *= (((float)GameConfig.screen_height) / 480.0);
		mission_description_rect.w *= (((float)GameConfig.screen_width) / 640.0);
		mission_description_rect.h *= (((float)GameConfig.screen_height) / 480.0);
	}
	//--------------------
	// This might take some time, so we need to be careful here,
	// so as not to generate a massive frame time, that would
	// throw every moving thing from the map.
	//
	Activate_Conservative_Frame_Computation();
	make_sure_system_mouse_cursor_is_turned_off();
	SetCurrentFont(FPS_Display_BFont);

	blit_special_background(QUEST_BROWSER_BACKGROUND_CODE);
	StoreMenuBackground(1);

	while (!back_to_game) {
		SDL_Delay(1);

		RestoreMenuBackground(1);
		if (current_quest_browser_mode == QUEST_BROWSER_SHOW_OPEN_MISSIONS)
			ShowGenericButtonFromList(QUEST_BROWSER_OPEN_QUESTS_BUTTON);
		else
			ShowGenericButtonFromList(QUEST_BROWSER_OPEN_QUESTS_OFF_BUTTON);
		if (current_quest_browser_mode == QUEST_BROWSER_SHOW_DONE_MISSIONS)
			ShowGenericButtonFromList(QUEST_BROWSER_DONE_QUESTS_BUTTON);
		else
			ShowGenericButtonFromList(QUEST_BROWSER_DONE_QUESTS_OFF_BUTTON);
		if (current_quest_browser_mode == QUEST_BROWSER_SHOW_NOTES)
			ShowGenericButtonFromList(QUEST_BROWSER_NOTES_BUTTON);
		else
			ShowGenericButtonFromList(QUEST_BROWSER_NOTES_OFF_BUTTON);

		quest_browser_display_mission_list(current_quest_browser_mode);

		blit_our_own_mouse_cursor();
		our_SDL_flip_wrapper();
		save_mouse_state();

		SDL_Event event;

		SDL_WaitEvent(&event);

		if (event.type == SDL_QUIT) {
			Terminate(0);
		}

		if (event.type == SDL_KEYDOWN) {

			if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q) {
				back_to_game = TRUE;
			}
		}

		if (MouseLeftClicked()) {
			if (MouseCursorIsOnButton(QUEST_BROWSER_OPEN_QUESTS_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				current_quest_browser_mode = QUEST_BROWSER_SHOW_OPEN_MISSIONS;
				mission_list_scroll_override_from_user = 0;
			}
			if (MouseCursorIsOnButton(QUEST_BROWSER_DONE_QUESTS_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				current_quest_browser_mode = QUEST_BROWSER_SHOW_DONE_MISSIONS;
				mission_list_scroll_override_from_user = 0;
			}
			if (MouseCursorIsOnButton(QUEST_BROWSER_NOTES_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				current_quest_browser_mode = QUEST_BROWSER_SHOW_NOTES;
				mission_list_scroll_override_from_user = 0;
			}
			if (MouseCursorIsOnButton(QUEST_BROWSER_SCROLL_UP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				mission_list_scroll_override_from_user--;
				if (mission_list_scroll_override_from_user < 0)
					mission_list_scroll_override_from_user = 0;
			}
			if (MouseCursorIsOnButton(QUEST_BROWSER_SCROLL_DOWN_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				mission_list_scroll_override_from_user++;
			}

			if (MouseCursorIsOnButton(QUEST_BROWSER_EXIT_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				back_to_game = TRUE;
			}
		}
	}
	game_status = old_game_status;
};				// void quest_browser_interface ( void )

/*----------------------------------------------------------------------
 * This function checks, if the influencer has succeeded in his given 
 * mission.  If not it returns, if yes the EndTitle/Debriefing is
 * started.
 ----------------------------------------------------------------------*/
void CheckIfMissionIsComplete(void)
{
	int ItemCounter;
	int mis_num;
	static int CheckMissionGrid;
	int this_mission_seems_completed = TRUE;

#define MIS_COMPLETE_DEBUG 1

	//--------------------
	// We do not need to check for mission completed EVERY frame
	// It will be enough to do it now and then..., e.g. every 50th frame
	//
	CheckMissionGrid++;
	if ((CheckMissionGrid % 50) != 0)
		return;

	for (mis_num = 0; mis_num < MAX_MISSIONS_IN_GAME; mis_num++) {
		//--------------------
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

		//--------------------
		// Continue if the Mission target KillOne is given but not fullfilled
		//
		if (Me.AllMissions[mis_num].KillOne != (-1)) {
			enemy *erot;
			BROWSE_ALIVE_BOTS(erot) {
				if ((erot->marker == Me.AllMissions[mis_num].KillOne)) {
					DebugPrintf(MIS_COMPLETE_DEBUG,
						    "\nOne of the marked droids is still alive... (0x%08x at %f:%f on %d)\n", erot,
						    erot->pos.x, erot->pos.y, erot->pos.z);
					this_mission_seems_completed = FALSE;
					break;
				}

			}
		}
		//--------------------
		// Continue if the Mission target fetch_item is given but not fullfilled
		//
		if (Me.AllMissions[mis_num].fetch_item != (-1)) {

			for (ItemCounter = 0; ItemCounter < MAX_ITEMS_IN_INVENTORY; ItemCounter++) {
				if (Me.Inventory[ItemCounter].type == Me.AllMissions[mis_num].fetch_item) {
					DebugPrintf(MIS_COMPLETE_DEBUG, "\nDesired item IS PRESENT!!");
					break;
				}
			}
			if (ItemCounter >= MAX_ITEMS_IN_INVENTORY) {
				// goto CheckNextMission;
				this_mission_seems_completed = FALSE;
			}
		}
		//--------------------
		// Continue if the Mission target must_clear_first_level is given but not fullfilled
		//
		if (Me.AllMissions[mis_num].must_clear_first_level != (-1)) {
			enemy *erot;
			BROWSE_LEVEL_BOTS(erot, Me.AllMissions[mis_num].must_clear_first_level) {
				if (!erot->is_friendly) {
					this_mission_seems_completed = FALSE;
					break;
				}
			}
		}
		//--------------------
		// Continue if the Mission target must_clear_second_level is given but not fullfilled
		//
		if (Me.AllMissions[mis_num].must_clear_second_level != (-1)) {
			enemy *erot;
			BROWSE_LEVEL_BOTS(erot, Me.AllMissions[mis_num].must_clear_second_level) {
				if (!erot->is_friendly) {
					this_mission_seems_completed = FALSE;
					break;
				}
			}
		}

		//--------------------
		// Continue if the Mission target MustReachLevel is given but not fullfilled
		//
		if (Me.AllMissions[mis_num].MustReachLevel != (-1)) {
			if (Me.pos.z != Me.AllMissions[mis_num].MustReachLevel) {
				DebugPrintf(MIS_COMPLETE_DEBUG, "\nLevel number does not match...");
				continue;
			}
		}
		//--------------------
		// Continue if the Mission target MustReachPoint.x is given but not fullfilled
		//
		if (Me.AllMissions[mis_num].MustReachPoint.x != (-1)) {
			if (Me.pos.x != Me.AllMissions[mis_num].MustReachPoint.x) {
				DebugPrintf(MIS_COMPLETE_DEBUG, "\nX coordinate does not match...");
				continue;
			}
		}
		//--------------------
		// Continue if the Mission target MustReachPoint.y is given but not fullfilled
		//
		if (Me.AllMissions[mis_num].MustReachPoint.y != (-1)) {
			if (Me.pos.y != Me.AllMissions[mis_num].MustReachPoint.y) {
				DebugPrintf(MIS_COMPLETE_DEBUG, "\nY coordinate does not match...");
				continue;
			}
		}
		//--------------------
		// Continue if the Mission target MustLiveTime is given but not fullfilled
		//
		if (Me.AllMissions[mis_num].MustLiveTime != (-1)) {
			if (Me.MissionTimeElapsed < Me.AllMissions[mis_num].MustLiveTime) {
				DebugPrintf(MIS_COMPLETE_DEBUG, "\nTime Limit not yet reached...");
				continue;
			}
		}
		//--------------------
		// Continue if the Mission target MustBeOne is given but not fullfilled
		//
		if (Me.AllMissions[mis_num].MustBeOne != (-1)) {
			if (Me.marker != Me.AllMissions[mis_num].MustBeOne) {
				DebugPrintf(MIS_COMPLETE_DEBUG, "\nYou're not yet one of the marked ones...");
				continue;
			}
		}

		if (this_mission_seems_completed)
			CompleteMission(Me.AllMissions[mis_num].MissionName);

	}			// for AllMissions

};				// void CheckIfMissionIsComplete

/**
 * This function assigns a new mission to the player, which means 
 * that the status of the mission in the mission array is changed and
 * perhaps the mission log activated.
 */
void AssignMission(const char *name)
{
	int MissNum = GetMissionIndexByName(name);

	Mission_Status_Change_Sound();
	Me.AllMissions[MissNum].MissionWasAssigned = TRUE;

	if (Me.AllMissions[MissNum].assignment_lua_code)
		run_lua(Me.AllMissions[MissNum].assignment_lua_code);

};

/**
 * This function marks a mission as complete
 */
void CompleteMission(const char *name)
{
	int MissNum = GetMissionIndexByName(name);

	Mission_Status_Change_Sound();
	Me.AllMissions[MissNum].MissionIsComplete = TRUE;

	if (Me.AllMissions[MissNum].completion_lua_code)
		run_lua(Me.AllMissions[MissNum].completion_lua_code);
}

/**
 * At the start of every new game, the mission info (i.e. which missions
 * are already assigned, completed, failed, available and such) should
 * be reset to default state, so that no zombie mission entries can appear. 
 */
void clear_tux_mission_info()
{
	int MissionTargetIndex;
	int diary_entry_nr;

	//--------------------
	// At first we clear out all existing mission entries, so that no 'zombies' remain
	// when the game is restarted and (perhaps less) new missions are loaded.
	//
	for (MissionTargetIndex = 0; MissionTargetIndex < MAX_MISSIONS_IN_GAME; MissionTargetIndex++) {
		Me.AllMissions[MissionTargetIndex].MissionExistsAtAll = FALSE;
		Me.AllMissions[MissionTargetIndex].MissionIsComplete = FALSE;
		Me.AllMissions[MissionTargetIndex].MissionWasFailed = FALSE;
		Me.AllMissions[MissionTargetIndex].MissionWasAssigned = FALSE;

		strcpy(Me.AllMissions[MissionTargetIndex].MissionName, "");

		for (diary_entry_nr = 0; diary_entry_nr < MAX_MISSION_DESCRIPTION_TEXTS; diary_entry_nr++) {
			if (mission_diary_texts[MissionTargetIndex][diary_entry_nr]) {
				free(mission_diary_texts[MissionTargetIndex][diary_entry_nr]);
				mission_diary_texts[MissionTargetIndex][diary_entry_nr] = NULL;
			}

			Me.AllMissions[MissionTargetIndex].mission_description_visible[diary_entry_nr] = FALSE;
			Me.AllMissions[MissionTargetIndex].mission_description_time[diary_entry_nr] = 0;
		}
	}

};				// void clear_tux_mission_info ( )

/**
 * This function reads the mission specifications from the mission file
 * which is assumed to be loaded into memory already.
 */
void GetQuestList(char *QuestListFilename)
{
	char *EndOfMissionTargetPointer;
	int MissionTargetIndex = 0;
	char *MissionTargetPointer;
	char fpath[2048];
	char *MissionFileContents;
	char InnerPreservedLetter = 0;
	int diary_entry_nr;

#define MISSION_TARGET_SUBSECTION_START_STRING "** Start of this mission target subsection **"
#define MISSION_TARGET_SUBSECTION_END_STRING "** End of this mission target subsection **"

#define MISSION_TARGET_NAME_INITIALIZER "Mission Name=_\""

#define MISSION_TARGET_FETCH_ITEM_STRING "Mission target is to fetch item : \""
#define MISSION_TARGET_KILL_ONE_STRING "Mission target is to kill droids with marker : "
#define MISSION_TARGET_MUST_CLEAR_FIRST_LEVEL "Mission target is to kill all hostile droids this first level : "
#define MISSION_TARGET_MUST_CLEAR_SECOND_LEVEL "Mission target is to also kill all hostile droids on second level : "
#define MISSION_TARGET_MUST_REACH_LEVEL_STRING "Mission target is to reach level : "
#define MISSION_TARGET_MUST_REACH_POINT_X_STRING "Mission target is to reach X-Pos : "
#define MISSION_TARGET_MUST_REACH_POINT_Y_STRING "Mission target is to reach Y-Pos : "
#define MISSION_TARGET_MUST_LIVE_TIME_STRING "Mission target is to live for how many seconds : "
#define MISSION_TARGET_MUST_BE_ONE_STRING "Mission target is to overtake a droid with marker : "

#define MISSION_ASSIGNMENT_LUACODE_STRING "Assignment LuaCode={"
#define MISSION_COMPLETION_LUACODE_STRING "Completion LuaCode={"

	//--------------------
	// At first we must load the quest list file given...
	//
	find_file(QuestListFilename, MAP_DIR, fpath, 0);
	MissionFileContents = ReadAndMallocAndTerminateFile(fpath, "*** END OF QUEST LIST *** LEAVE THIS TERMINATOR IN HERE ***");
	MissionTargetPointer = MissionFileContents;

	for (MissionTargetIndex = 0; MissionTargetIndex < MAX_MISSIONS_IN_GAME; MissionTargetIndex++) {
		Me.AllMissions[MissionTargetIndex].MissionExistsAtAll = FALSE;
	}

	MissionTargetIndex = 0;
	while ((MissionTargetPointer = strstr(MissionTargetPointer, MISSION_TARGET_SUBSECTION_START_STRING)) != NULL) {
		EndOfMissionTargetPointer = LocateStringInData(MissionTargetPointer, MISSION_TARGET_SUBSECTION_END_STRING);

		if (MissionTargetIndex >= MAX_MISSIONS_IN_GAME)
			ErrorMessage(__FUNCTION__, "The number of quests specified in %s exceeds MAX_MISSIONS_IN_GAME (%d).\n",
				     PLEASE_INFORM, IS_FATAL, QuestListFilename, MAX_MISSIONS_IN_GAME);

		//--------------------
		// We need to add an inner terminator here, so that the strstr operation
		// below will know where to stop within this subsection.
		//
		InnerPreservedLetter = *EndOfMissionTargetPointer;
		*EndOfMissionTargetPointer = 0;

		Me.AllMissions[MissionTargetIndex].MissionExistsAtAll = TRUE;

		char *tmname = ReadAndMallocStringFromData(MissionTargetPointer, MISSION_TARGET_NAME_INITIALIZER, "\"");
		strcpy(Me.AllMissions[MissionTargetIndex].MissionName, tmname);
		free(tmname);

		//--------------------
		// From here on we read the details of the mission target, i.e. what the
		// influencer has to do, so that the mission can be thought of as completed
		//
		if (strstr(MissionTargetPointer, MISSION_TARGET_FETCH_ITEM_STRING)) {
			char *iname = ReadAndMallocStringFromData(MissionTargetPointer, MISSION_TARGET_FETCH_ITEM_STRING, "\"");
			Me.AllMissions[MissionTargetIndex].fetch_item = GetItemIndexByName(iname);
			free(iname);
		} else
			Me.AllMissions[MissionTargetIndex].fetch_item = -1;

		ReadValueFromString(MissionTargetPointer, MISSION_TARGET_KILL_ONE_STRING, "%d",
				    &Me.AllMissions[MissionTargetIndex].KillOne, EndOfMissionTargetPointer);

		ReadValueFromString(MissionTargetPointer, MISSION_TARGET_MUST_CLEAR_FIRST_LEVEL, "%d",
				    &Me.AllMissions[MissionTargetIndex].must_clear_first_level, EndOfMissionTargetPointer);

		ReadValueFromString(MissionTargetPointer, MISSION_TARGET_MUST_CLEAR_SECOND_LEVEL, "%d",
				    &Me.AllMissions[MissionTargetIndex].must_clear_second_level, EndOfMissionTargetPointer);

		ReadValueFromString(MissionTargetPointer, MISSION_TARGET_MUST_BE_ONE_STRING, "%d",
				    &Me.AllMissions[MissionTargetIndex].MustBeOne, EndOfMissionTargetPointer);

		ReadValueFromString(MissionTargetPointer, MISSION_TARGET_MUST_REACH_POINT_X_STRING, "%d",
				    &Me.AllMissions[MissionTargetIndex].MustReachPoint.x, EndOfMissionTargetPointer);

		ReadValueFromString(MissionTargetPointer, MISSION_TARGET_MUST_REACH_POINT_Y_STRING, "%d",
				    &Me.AllMissions[MissionTargetIndex].MustReachPoint.y, EndOfMissionTargetPointer);

		ReadValueFromString(MissionTargetPointer, MISSION_TARGET_MUST_REACH_LEVEL_STRING, "%d",
				    &Me.AllMissions[MissionTargetIndex].MustReachLevel, EndOfMissionTargetPointer);

		ReadValueFromString(MissionTargetPointer, MISSION_TARGET_MUST_LIVE_TIME_STRING, "%lf",
				    &Me.AllMissions[MissionTargetIndex].MustLiveTime, EndOfMissionTargetPointer);

		if (strstr(MissionTargetPointer, MISSION_COMPLETION_LUACODE_STRING)) {
			Me.AllMissions[MissionTargetIndex].completion_lua_code =
			    ReadAndMallocStringFromData(MissionTargetPointer, MISSION_COMPLETION_LUACODE_STRING, "}");
		} else {
			Me.AllMissions[MissionTargetIndex].completion_lua_code = NULL;
		}

		if (strstr(MissionTargetPointer, MISSION_ASSIGNMENT_LUACODE_STRING)) {
			Me.AllMissions[MissionTargetIndex].assignment_lua_code =
			    ReadAndMallocStringFromData(MissionTargetPointer, MISSION_ASSIGNMENT_LUACODE_STRING, "}");
		} else {
			Me.AllMissions[MissionTargetIndex].assignment_lua_code = NULL;
		}
		//--------------------
		// Now it is time to read in the mission diary entries, that might
		// be displayed in the quest browser later.
		//
		for (diary_entry_nr = 0; diary_entry_nr < MAX_MISSION_DESCRIPTION_TEXTS; diary_entry_nr++) {
			mission_diary_texts[MissionTargetIndex][diary_entry_nr] = NULL;
		}

		//--------------------
		// Now we are done with reading in THIS one mission target
		// We need to advance the MissionTargetPointer, so that we avoid doubly
		// reading in this mission OR ONE OF THIS MISSIONS VALUES!!!!
		// 
		// And we need of course to advance the array index for mission targets too...
		//
		MissionTargetPointer = EndOfMissionTargetPointer;	// to avoid double entering the same target
		MissionTargetIndex++;	// to avoid overwriting the same entry again

		//--------------------
		// We restore the termination character we added before, even if that
		// is maybe not really nescessary...
		//
		*EndOfMissionTargetPointer = InnerPreservedLetter;

	}			// while mission target found...

	//--------------------
	// Finally we record the number of mission targets scanned and are done with this function
	DebugPrintf(1, "\nNUMBER OF MISSION TARGETS FOUND: %d.\n", MissionTargetIndex);
	fflush(stdout);
	free(MissionFileContents);
};				// void Get_Mission_Targets( ... )

/**
 * This function returns the mission that has the specified name.
 */
int GetMissionIndexByName(const char *name)
{
	int cidx;

	for (cidx = 0; cidx < MAX_MISSIONS_IN_GAME; cidx++) {
		if (!strcmp(Me.AllMissions[cidx].MissionName, name))
			return cidx;
	}

	ErrorMessage(__FUNCTION__, "Unable to find mission named %s\n", PLEASE_INFORM, IS_FATAL, name);
	return -1;
}

#undef _mission_c
