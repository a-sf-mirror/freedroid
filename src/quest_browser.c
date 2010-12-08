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

/** This file implements the quest browser. */
#define _quest_browser_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

static SDL_Rect mission_description_rect = { 134, 86, 280, 320 };

static struct auto_string *quest_browser_text;

static int mission_list_scroll_override_from_user;
static char stats_display[NB_DROID_TYPES];

#define QUEST_BROWSER_SHOW_OPEN_MISSIONS (-1011)
#define QUEST_BROWSER_SHOW_DONE_MISSIONS (-1012)
#define QUEST_BROWSER_SHOW_NOTES (-1013)

static int current_quest_browser_mode = QUEST_BROWSER_SHOW_OPEN_MISSIONS;

/**
 * If there is some mission selected inside the quest browser, then we
 * should also display all info on the current status and history of that
 * particular mission, which is exactly what this function is responsible
 * for.
 */
static void quest_browser_append_mission_info(const char *mis_name, int full_description)
{
	int i;
	int mis_num = GetMissionIndexByName(mis_name);
	struct mission *mis = &Me.AllMissions[mis_num];

	autostr_append(quest_browser_text, "\3%s \1%s\n\3", _("Quest:"),  _(mis->mission_name));
	
	if (!full_description)
		return;

	autostr_append(quest_browser_text, "\3%s \2", _("Status:"));

	const char *status;
	if (mis->MissionIsComplete)
		status = _("COMPLETE");
	else if (mis->MissionWasFailed)
		status = _("FAILED");
	else
		status = _("STILL OPEN");

	autostr_append(quest_browser_text, "%s\n\3%s\n", status, _("Details:"));

	for (i = 0; i < MAX_MISSION_DESCRIPTION_TEXTS; i++) {
		if (mis->mission_diary_texts[i]) {
			autostr_append(quest_browser_text, "\3-------- \1%s %d  %02d:%02d\3 --------\n\2%s\n\3", _("Day"), 
					get_days_of_game_duration(mis->mission_description_time[i]),
					get_hours_of_game_duration(mis->mission_description_time[i]),
					get_minutes_of_game_duration(mis->mission_description_time[i]),
					D_(mis->mission_diary_texts[i]));
		}
	}
}

//comparison function for droid names
static int cmp_droid_names(const void *ptr1, const void *ptr2){
	const char *ch1 =  Druidmap[*(int *)ptr1].default_short_description;
	const char *ch2 =  Druidmap[*(int *)ptr2].default_short_description;
	return strcmp(ch1, ch2);
}

//function that assembles and prints statistics about the player
static void print_statistics(void)
{
	int statistics_browser_lines_needed[Number_Of_Droid_Types + 2];
	int display = 0;                //display location
	int srt[Number_Of_Droid_Types]; //used for alphabetizing droid names in display
	int model;                      //increment Druidmap
	int rate;                       //used for finding best/worst Takeover rate

	//totals, bests & worsts
	int total_destroyed = 0;
	int total_damage_dealt = 0;
	int total_takeover_success = 0;
	int fav_destroyed = -1;
	int fav_capture_target = -1;
	int fav_capture_success_rate = 0;
	int worst_capture_target = -1;
	int worst_capture_success_rate = 100;

	// Grab information out of the statistics arrays
	for (model = 0; model < Number_Of_Droid_Types; model++) {
		total_takeover_success += Me.TakeoverSuccesses[model];
	        total_destroyed += Me.destroyed_bots[model];
	        total_damage_dealt += Me.damage_dealt[model];
		if (Me.destroyed_bots[model] > Me.destroyed_bots[fav_destroyed])
			fav_destroyed = model;
		if (Me.TakeoverSuccesses[model] + Me.TakeoverFailures[model]) {
			rate = (100 * Me.TakeoverSuccesses[model])/ (Me.TakeoverSuccesses[model] + Me.TakeoverFailures[model]);
			if (rate > fav_capture_success_rate ) {
				fav_capture_target = model;
				fav_capture_success_rate = rate;
			}
			if (rate < worst_capture_success_rate) {
				worst_capture_target = model;
				worst_capture_success_rate = rate;
			}
		}
		srt[model] = model; //set to un-alphabetized order from freedroid.droid_archetypes
	}

	//Assemble Output
	autostr_printf(quest_browser_text, _("\1Parafunken Statistics\2\n"));

	// This is where we display grand total statistics
	autostr_append(quest_browser_text, _("\1Overview\2\n"));
	statistics_browser_lines_needed[display] = get_lines_needed(quest_browser_text->value, mission_description_rect, 1);
	if (stats_display[display++]) {
		autostr_append(quest_browser_text, _("Time Played: \3%dh%dm\2\n"),
					   (int) Me.current_game_date / (60 * 60),
					   ((int)Me.current_game_date / 60) % 60);
		autostr_append(quest_browser_text, _("Distance Traveled: \3%.1fm\2\n"), Me.meters_traveled);
		autostr_append(quest_browser_text, _("Destroyed Enemies: \3%i\2\n"), total_destroyed);
		autostr_append(quest_browser_text, _("Captured Enemies: \3%i\2\n"), total_takeover_success);
		autostr_append(quest_browser_text, _("Damage Dealt: \3%i\2\n"), total_damage_dealt);
	}

	// This is where we display statistics about the best and worst relationships with bots:
	// Attacking -> type most destroyed or killed (show #destroyed, #damage)
	// Takeover  -> type with highest takeover % (%rate, # attempts)
	// Defending -> type most damage done to Tux (# damage)
	// Failed Takeovers -> lowest takeover % (% rate, # attempts)
	autostr_append(quest_browser_text, _("\1Preferences\2\n"));
	statistics_browser_lines_needed[display] = get_lines_needed(quest_browser_text->value, mission_description_rect, 1);
	if (stats_display[display++]) {
		if (fav_destroyed != -1) {
			autostr_append(quest_browser_text, _("Attacking: \3%s\2 (\3%i\2 "), 
				Druidmap[fav_destroyed].default_short_description, Me.destroyed_bots[fav_destroyed]);
			if (Druidmap[fav_destroyed].is_human)
				autostr_append(quest_browser_text, _("killed,"));
			else
				autostr_append(quest_browser_text, _("destroyed,"));
			autostr_append(quest_browser_text, _(" \3%i\2 DP)\n"), Me.damage_dealt[fav_destroyed]);
		} else
			autostr_append(quest_browser_text, _("Attacking: no droids destroyed\n"));

		if (fav_capture_target != -1)
			autostr_append(quest_browser_text, _("Takeover: \3%s\2 (\3%i%%\2 rate, \3%i\2 attempts)\n"),
					Druidmap[fav_capture_target].default_short_description, fav_capture_success_rate,
					Me.TakeoverSuccesses[fav_capture_target] + Me.TakeoverFailures[fav_capture_target]);
		else
			autostr_append(quest_browser_text, _("Takeover: none successful\n"));


		if (worst_capture_target != -1)
			autostr_append(quest_browser_text, _("Failed Takeover: \3%s\2 (\3%i%%\2 rate, \3%i\2 attempts)\n"),
					Druidmap[worst_capture_target].default_short_description, worst_capture_success_rate,
					Me.TakeoverSuccesses[worst_capture_target] + Me.TakeoverFailures[worst_capture_target]);
		else
			autostr_append(quest_browser_text, _("Failed Takeover: no failures\n"));
	}

	//This sorts the droid types by default model name (from freedroid.droid_archetypes):
	//srt[] will hold the Druidmap[] indices in this order 
	qsort(srt, Number_Of_Droid_Types, sizeof(srt[0]), cmp_droid_names);

	// This is where we display statistics about each bot/human type we have interacted with:
	// Killed/Destroyed: humans are 'killed', bots are 'destroyed', show #
	// Takeover: # successes, # failures, % success/total (hidden for human models)
	// Damage: # damage dealt, # damage recieved, % delt/total
	int counter;
	for (counter = 0; counter < Number_Of_Droid_Types; counter++) {
		model = srt[counter];
		//only show bot types the player has interacted with:
		if (Me.damage_dealt[model] || Me.TakeoverSuccesses[model] || Me.TakeoverFailures[model] || Me.destroyed_bots[model]) {
			autostr_append(quest_browser_text, "\1%s\2\n", Druidmap[model].default_short_description);
			statistics_browser_lines_needed[display] = get_lines_needed(quest_browser_text->value, mission_description_rect, 1);
			if (stats_display[display++]) {
				if (Druidmap[model].is_human) {
					autostr_append(quest_browser_text, _("Killed: \3%d\2\n"), Me.destroyed_bots[model]);
				} else {
					autostr_append(quest_browser_text, _("Destroyed: \3%d\2\n"), Me.destroyed_bots[model]);
					autostr_append(quest_browser_text, _("Takeovers: \3%d\2/\3%d\2/\3"), Me.TakeoverSuccesses[model], Me.TakeoverFailures[model]);
					if (Me.TakeoverSuccesses[model] + Me.TakeoverFailures[model])
						autostr_append(quest_browser_text, _("%d%%\2"),
							(100 * Me.TakeoverSuccesses[model])/(Me.TakeoverSuccesses[model] + Me.TakeoverFailures[model]));
					else
						autostr_append(quest_browser_text, _(" 0%%\2"));
					autostr_append(quest_browser_text, _(" (success/fail/ratio)\n"));
				}
				autostr_append(quest_browser_text, _("Damage: \3%d\2 (dealt)\n"), Me.damage_dealt[model]);
			}
		} else {
			statistics_browser_lines_needed[display++] = -1;
		}
	}

	//Print all to screen
	float mission_list_offset = (FontHeight(GetCurrentFont()) * TEXT_STRETCH)
	    * mission_list_scroll_override_from_user;
	DisplayText(quest_browser_text->value, mission_description_rect.x,
			    mission_description_rect.y - mission_list_offset, &mission_description_rect, 1);

	// Now it's time to display some short/long symbols in front
	// of each of the sections.
	//
	for (display = 0; display < Number_Of_Droid_Types + 2; display++) {
		if (statistics_browser_lines_needed[display] == -1)
			continue;

		SDL_Rect *rect_short = &(AllMousePressButtons[QUEST_BROWSER_ITEM_SHORT_BUTTON].button_rect);
		SDL_Rect *rect_long = &(AllMousePressButtons[QUEST_BROWSER_ITEM_LONG_BUTTON].button_rect);

		// At first we bring the short/long buttons into position.
		// This position might be well off the screen.  That's no
		// problem, because we won't blit the thing in this case
		// anyway.
		rect_short->y =
		    mission_description_rect.y - mission_list_offset +
		    (FontHeight(GetCurrentFont()) * 1.0) * (statistics_browser_lines_needed[display] -1) - 2;
		rect_short->x = mission_description_rect.x - 26;
		rect_long->y = rect_short->y;
		rect_long->x = rect_short->x;

		// Now we check if the y coordinate of the buttons are 
		// somewhat reasonable or not.  For those buttons that are
		// off the screen, things are simple, because then we can
		// skip the rest of this pass of the loop.
		//
		if (rect_short->y <= mission_description_rect.y - 4)
			continue;
		if (rect_short->y >= mission_description_rect.y + mission_description_rect.h - FontHeight(GetCurrentFont()))
			continue;

		if (stats_display[display])
			ShowGenericButtonFromList(QUEST_BROWSER_ITEM_LONG_BUTTON);
		else
			ShowGenericButtonFromList(QUEST_BROWSER_ITEM_SHORT_BUTTON);

		// This toggles the button (and displayed info):
		if (MouseLeftPressed()) {
			if (MouseCursorIsOnButton(QUEST_BROWSER_ITEM_SHORT_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				stats_display[display] = !stats_display[display];
				while (MouseLeftPressed()) ;
			}
		}
	}
}

/**
 *
 *
 */
static void quest_browser_display_mission_list(int list_type)
{
	int mis_num;
	int something_was_displayed = FALSE;
	int quest_browser_mission_lines_needed[MAX_MISSIONS_IN_GAME];

	autostr_printf(quest_browser_text, "");

	for (mis_num = 0; mis_num < MAX_MISSIONS_IN_GAME; mis_num++) {
		quest_browser_mission_lines_needed[mis_num] = -1;

		if (Me.AllMissions[mis_num].MissionExistsAtAll != TRUE)
			continue;

		if (Me.AllMissions[mis_num].MissionWasAssigned != TRUE)
			continue;

		// We record the number of lines needed so far, so that we may later
		// draw suitable long/short buttons in front of the text, so that the
		// user can then select to see the full/short information on this quest.
		//
		quest_browser_mission_lines_needed[mis_num] =
			get_lines_needed(quest_browser_text->value, mission_description_rect, TEXT_STRETCH);

		if ((list_type == QUEST_BROWSER_SHOW_OPEN_MISSIONS) && (Me.AllMissions[mis_num].MissionIsComplete == FALSE)) {
			quest_browser_append_mission_info(Me.AllMissions[mis_num].mission_name,
							  Me.AllMissions[mis_num].expanded_display_for_this_mission);
			something_was_displayed = TRUE;
		} else if ((list_type == QUEST_BROWSER_SHOW_DONE_MISSIONS) && (Me.AllMissions[mis_num].MissionIsComplete != FALSE)) {
			quest_browser_append_mission_info(Me.AllMissions[mis_num].mission_name,
							  Me.AllMissions[mis_num].expanded_display_for_this_mission);
			something_was_displayed = TRUE;
		} else {
			quest_browser_mission_lines_needed[mis_num] = -1;
		}

	}

	if (something_was_displayed) {
		float mission_list_offset = FontHeight(GetCurrentFont()) * mission_list_scroll_override_from_user;
		DisplayText(quest_browser_text->value, mission_description_rect.x,
			    mission_description_rect.y - mission_list_offset, &mission_description_rect, 1.0);

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
			// At first we bring the short/long buttons into position.
			// This position might be well off the screen.  That's no
			// problem, because we won't blit the thing in this case
			// anyway.
			//
			if (quest_browser_mission_lines_needed[mis_num] != -1) {
				rect_short->y =
				    mission_description_rect.y - mission_list_offset +
				    (FontHeight(GetCurrentFont())) * (quest_browser_mission_lines_needed[mis_num]) - 2;
				rect_short->x = mission_description_rect.x - 26;
				rect_long->y = rect_short->y;
				rect_long->x = rect_short->x;
			}
			// Now we check if the y coordinate of the buttons are 
			// somewhat reasonable or not.  For those buttons that are
			// off the screen, things are simple, because then we can
			// skip the rest of this pass of the loop.
			//
			if (rect_short->y <= mission_description_rect.y - 4)
				continue;
			if (rect_short->y >= mission_description_rect.y + mission_description_rect.h - FontHeight(GetCurrentFont()) / 2)
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
		const char *txt;
		
		switch (list_type) {
			case QUEST_BROWSER_SHOW_OPEN_MISSIONS:
				txt = _("\3No open quests yet.\2");
				break;
			case QUEST_BROWSER_SHOW_DONE_MISSIONS:
				txt = _("\3No completed quests yet.\2");
				break;
		}

		DisplayText(txt, mission_description_rect.x, mission_description_rect.y, &mission_description_rect, TEXT_STRETCH);
	}
}

/**
 * This function manages the quest browser.
 */
void quest_browser_interface(void)
{
	int back_to_game = FALSE;
	int old_game_status = game_status;
	static int first_call = TRUE;

	game_status = INSIDE_MENU;

	// On the very first 
	if (first_call) {
		first_call = FALSE;
		mission_description_rect.x *= (((float)GameConfig.screen_width) / 640.0);
		mission_description_rect.y *= (((float)GameConfig.screen_height) / 480.0);
		mission_description_rect.w *= (((float)GameConfig.screen_width) / 640.0);
		mission_description_rect.h *= (((float)GameConfig.screen_height) / 480.0);
		quest_browser_text = alloc_autostr(1000);
	}
	// This might take some time, so we need to be careful here,
	// so as not to generate a massive frame time, that would
	// throw every moving thing from the map.
	//
	Activate_Conservative_Frame_Computation();
	SetCurrentFont(FPS_Display_BFont);

	blit_special_background(QUEST_BROWSER_BACKGROUND_CODE);
	StoreMenuBackground(1);

	Me.quest_browser_changed = 0;

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

		switch (current_quest_browser_mode) {
			case QUEST_BROWSER_SHOW_NOTES:
				print_statistics();
				break;
			case QUEST_BROWSER_SHOW_OPEN_MISSIONS:
			case QUEST_BROWSER_SHOW_DONE_MISSIONS:
				quest_browser_display_mission_list(current_quest_browser_mode);
				break;
		}
		
		ShowGenericButtonFromList(QUEST_BROWSER_SCROLL_UP_BUTTON);
		ShowGenericButtonFromList(QUEST_BROWSER_SCROLL_DOWN_BUTTON);

		blit_mouse_cursor();
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

				while (MouseLeftPressed());
			}
		}
	}
	game_status = old_game_status;
}

#undef _quest_browser_c
