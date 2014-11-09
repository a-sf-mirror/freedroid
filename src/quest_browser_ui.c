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
#include "widgets/widgets.h"

static struct widget_group *quest_browser = NULL;

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

	autostr_append(quest_browser_text, _("[n]Quest [r]%s\n[n]"), D_(mis->mission_name));
	
	if (!full_description)
		return;

	autostr_append(quest_browser_text, _("[n]Status [w]"));

	const char *status;
	if (mis->MissionIsComplete)
		status = _("COMPLETE");
	else if (mis->MissionWasFailed)
		status = _("FAILED");
	else
		status = _("STILL OPEN");

	autostr_append(quest_browser_text, "%s\n[n]%s\n", status, _("Details:"));

	for (i = 0; i < MAX_MISSION_DESCRIPTION_TEXTS; i++) {
		if (mis->mission_diary_texts[i]) {
			autostr_append(quest_browser_text, "[n]-------- [r]%s %d  %02d:%02d[n] --------\n[w]%s\n[n]", _("Day"),
					get_days_of_game_duration(mis->mission_description_time[i]),
					get_hours_of_game_duration(mis->mission_description_time[i]),
					get_minutes_of_game_duration(mis->mission_description_time[i]),
					D_(mis->mission_diary_texts[i]));
		}
	}
}

/**
 * Calculates the amount of a level that the player has seen
*/
static void calculate_level_explored(int levelnum, int *num_squares_seen, int *num_squares_exist)
{
	int x = 0;
	int y = 0;
	level *automap_level = curShip.AllLevels[levelnum];

	for (y = 0; y < automap_level->ylen; y++) {
		for (x = 0; x < automap_level->xlen; x++) {
			if (Me.Automap[levelnum][y][x] & SQUARE_SEEN_AT_ALL_BIT) {
				*num_squares_seen = *num_squares_seen + 1;
			}
		}
	}

	*num_squares_exist = *num_squares_exist + (x * y);
	return; 
}

/**
 * Calculates the total percentage of all the levels that a player has seen
*/
static float calculate_total_explored_percentage(void)
{
	int num_squares_seen = 0;
	int num_squares_exist = 0;
	int lvl;

	for (lvl = 0; lvl < curShip.num_levels; lvl++) {
		calculate_level_explored(lvl, &num_squares_seen, &num_squares_exist);
	}
	return ((float) num_squares_seen)/((float) num_squares_exist);
}

//comparison function for droid names
static int cmp_droid_names(const void *ptr1, const void *ptr2){
	const char *ch1 =  Droidmap[*(int *)ptr1].default_short_description;
	const char *ch2 =  Droidmap[*(int *)ptr2].default_short_description;
	return strcmp(ch1, ch2);
}

//function that assembles and prints statistics about the player
static void print_statistics(void)
{
	int statistics_browser_lines_needed[Number_Of_Droid_Types + 2];
	int display = 0;                //display location
	int srt[Number_Of_Droid_Types]; //used for alphabetizing droid names in display
	int model;                      //increment Droidmap
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

	int i;
	for (i = 0; i < sizeof(statistics_browser_lines_needed)/sizeof(statistics_browser_lines_needed[0]); i++)
		statistics_browser_lines_needed[i] = -1;

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
		srt[model] = model; //set to un-alphabetized order from droid_archetypes.dat
	}

	//Assemble Output
	autostr_printf(quest_browser_text, _("[r]Parafunken Statistics[w]\n"));

	// This is where we display grand total statistics
	autostr_append(quest_browser_text, _("[r]Overview[w]\n"));
	statistics_browser_lines_needed[display] = get_lines_needed(quest_browser_text->value, mission_description_rect, 1);
	if (stats_display[display++]) {
		// ; TRANSLATORS: h in %dh is for hours, m in %dm is for minutes, please also translate this :)
		autostr_append(quest_browser_text, _("Time Played: [n]%dh%dm[w]\n"),
					   (int) Me.current_game_date / (60 * 60),
					   ((int)Me.current_game_date / 60) % 60);
		// ; TRANSLATORS: m in %.1fm is for meters, please also translate this :)
		autostr_append(quest_browser_text, _("Distance Traveled: [n]%.1fm[w]\n"), Me.meters_traveled);
		if (Me.map_maker_is_present)
			autostr_append(quest_browser_text, _("Percentage explored: [n]%.1f%%[w]\n"), calculate_total_explored_percentage() * 100);
		autostr_append(quest_browser_text, _("Destroyed Enemies: [n]%i[w]\n"), total_destroyed);
		autostr_append(quest_browser_text, _("Captured Enemies: [n]%i[w]\n"), total_takeover_success);
		autostr_append(quest_browser_text, _("Damage Dealt: [n]%i[w]\n"), total_damage_dealt);
	}

	// This is where we display statistics about the best and worst relationships with bots:
	// Attacking -> type most destroyed or killed (show #destroyed, #damage)
	// Takeover  -> type with highest takeover % (%rate, # attempts)
	// Defending -> type most damage done to Tux (# damage)
	// Failed Takeovers -> lowest takeover % (% rate, # attempts)
	autostr_append(quest_browser_text, _("[r]Preferences[w]\n"));
	statistics_browser_lines_needed[display] = get_lines_needed(quest_browser_text->value, mission_description_rect, 1);
	if (stats_display[display++]) {
		if (fav_destroyed != -1) {
			if (Droidmap[fav_destroyed].is_human)
				autostr_append(quest_browser_text, _("Attacking: [n]%s[w] ([n]%i[w] killed, [n]%i[w] DP)\n"),
				               D_(Droidmap[fav_destroyed].default_short_description),
				               Me.destroyed_bots[fav_destroyed],
				               Me.damage_dealt[fav_destroyed]);
			else
				autostr_append(quest_browser_text, _("Attacking: [n]%s[w] ([n]%i[w] destroyed, [n]%i[w] DP)\n"),
				               D_(Droidmap[fav_destroyed].default_short_description),
				               Me.destroyed_bots[fav_destroyed],
				               Me.damage_dealt[fav_destroyed]);
		} else
			autostr_append(quest_browser_text, _("Attacking: no droids destroyed\n"));

		if (fav_capture_target != -1)
			autostr_append(quest_browser_text, _("Takeover: [n]%s[w] ([n]%i%%[w] rate, [n]%i[w] attempts)\n"),
					D_(Droidmap[fav_capture_target].default_short_description), fav_capture_success_rate,
					Me.TakeoverSuccesses[fav_capture_target] + Me.TakeoverFailures[fav_capture_target]);
		else
			autostr_append(quest_browser_text, _("Takeover: none successful\n"));


		if (worst_capture_target != -1)
			autostr_append(quest_browser_text, _("Failed Takeover: [n]%s[w] ([n]%i%%[w] rate, [n]%i[w] attempts)\n"),
					D_(Droidmap[worst_capture_target].default_short_description), worst_capture_success_rate,
					Me.TakeoverSuccesses[worst_capture_target] + Me.TakeoverFailures[worst_capture_target]);
		else
			autostr_append(quest_browser_text, _("Failed Takeover: no failures\n"));
	}

	//This sorts the droid types by default model name (from droid_archetypes.dat):
	//srt[] will hold the Droidmap[] indices in this order 
	qsort(srt, Number_Of_Droid_Types, sizeof(srt[0]), cmp_droid_names);

	// This is where we display statistics about each bot/human type we have interacted with:
	// Killed/Destroyed: humans are 'killed', bots are 'destroyed', show #
	// Takeover: # successes, # failures, % success/total (hidden for human models)
	// Damage: # damage dealt, # damage received, % delt/total
	int counter;
	for (counter = 0; counter < Number_Of_Droid_Types; counter++) {
		model = srt[counter];
		//only show bot types the player has interacted with:
		if (Me.damage_dealt[model] || Me.TakeoverSuccesses[model] || Me.TakeoverFailures[model] || Me.destroyed_bots[model]) {
			autostr_append(quest_browser_text, "[r]%s[w]\n", D_(Droidmap[model].default_short_description));
			statistics_browser_lines_needed[display] = get_lines_needed(quest_browser_text->value, mission_description_rect, 1);
			if (stats_display[display++]) {
				if (Droidmap[model].is_human) {
					autostr_append(quest_browser_text, _("Killed: [n]%d[w]\n"), Me.destroyed_bots[model]);
				} else {
					autostr_append(quest_browser_text, _("Destroyed: [n]%d[w]\n"), Me.destroyed_bots[model]);
					autostr_append(quest_browser_text, _("Takeovers: [n]%d[w]/[n]%d[w]/[n]"), Me.TakeoverSuccesses[model], Me.TakeoverFailures[model]);
					if (Me.TakeoverSuccesses[model] + Me.TakeoverFailures[model])
						autostr_append(quest_browser_text, "%d%%[w]",
							(100 * Me.TakeoverSuccesses[model])/(Me.TakeoverSuccesses[model] + Me.TakeoverFailures[model]));
					else
						autostr_append(quest_browser_text, " 0%%[w]");
					autostr_append(quest_browser_text, _(" (success/fail/ratio)\n"));
				}
				autostr_append(quest_browser_text, _("Damage: [n]%d[w] (dealt)\n"), Me.damage_dealt[model]);
			}
		} else {
			statistics_browser_lines_needed[display++] = -1;
		}
	}

	//Print all to screen
	float mission_list_offset = (FontHeight(GetCurrentFont()) * LINE_HEIGHT_FACTOR)
	    * mission_list_scroll_override_from_user;
	display_text_using_line_height(quest_browser_text->value, mission_description_rect.x,
                                       mission_description_rect.y - mission_list_offset, &mission_description_rect, 1.0);

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
			get_lines_needed(quest_browser_text->value, mission_description_rect, LINE_HEIGHT_FACTOR);

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
		display_text_using_line_height(quest_browser_text->value, mission_description_rect.x,
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
		const char *txt = NULL;
		
		switch (list_type) {
			case QUEST_BROWSER_SHOW_OPEN_MISSIONS:
				txt = _("[n]No open quests yet.[w]");
				break;
			case QUEST_BROWSER_SHOW_DONE_MISSIONS:
				txt = _("[n]No completed quests yet.[w]");
				break;
		}

		display_text(txt, mission_description_rect.x, mission_description_rect.y, &mission_description_rect);
	}
}

/**
 * This function enables the quest browser panel.
 */
void toggle_quest_browser(void)
{
	Me.quest_browser_changed = 0;
	quest_browser_activated = !quest_browser_activated;

	if (quest_browser_activated) {
		input_hold_keyboard();
		freeze_world();
	} else {
		unfreeze_world();
		input_release_keyboard();
	}
}

/**
 * Event handler for the quest browser top level group widget.
 */
static int quest_browser_handle_event(struct widget *w, SDL_Event *event)
{
	// Handle keyboard events
	if (event->type == SDL_KEYDOWN)
		if (event->key.keysym.sym == SDLK_q || event->key.keysym.sym == SDLK_ESCAPE) {
			toggle_quest_browser();
			return 1;
		}

	// Call default group widget event handler.
	return widget_group_handle_event(w, event);
}

/**
 * This function displays the selected category on the quest browser monitor.
 */
static void text_display(struct widget *w)
{
   switch (current_quest_browser_mode) {
	case QUEST_BROWSER_SHOW_NOTES:
		print_statistics();
		break;

	case QUEST_BROWSER_SHOW_OPEN_MISSIONS:
	case QUEST_BROWSER_SHOW_DONE_MISSIONS:
		quest_browser_display_mission_list(current_quest_browser_mode);
		break;
	}
}

/** Toggle open quests. */
static void toggle_open_quests(struct widget_button *wb)
{
	mission_list_scroll_override_from_user = 0;
	current_quest_browser_mode = QUEST_BROWSER_SHOW_OPEN_MISSIONS;
}

/** Toggle finished quests. */
static void toggle_done_quests(struct widget_button *wb)
{
	mission_list_scroll_override_from_user = 0;
	current_quest_browser_mode = QUEST_BROWSER_SHOW_DONE_MISSIONS;
}

/** Toggle notes. */
static void toggle_notes(struct widget_button *wb)
{
	mission_list_scroll_override_from_user = 0;
	current_quest_browser_mode = QUEST_BROWSER_SHOW_NOTES;
}

/** Check if the quest browser can scroll up. */
static int can_scroll_up()
{
	return (mission_list_scroll_override_from_user > 0);
}

/** Quest browser scroll up. */
static void scroll_up(struct widget_button *wb)
{
	if (wb->active)
		mission_list_scroll_override_from_user--;
}

/** Check if the quest browser can scroll down. */
static int can_scroll_down()
{
	int lines_needed = get_lines_needed(quest_browser_text->value, mission_description_rect, 1.0);
	int visible_lines = mission_description_rect.h / (float) FontHeight(GetCurrentFont());

	return (lines_needed > visible_lines) && (mission_list_scroll_override_from_user < lines_needed - visible_lines);
}

/** Quest browser scroll down. */
static void scroll_down(struct widget_button *wb)
{
	if (wb->active)
		mission_list_scroll_override_from_user++;
}

/** This function is used for darkening the screen area outside the quest browser. */
static void display_dark_background(struct widget *w)
{
	draw_rectangle(&w->rect, 0, 0, 0, 150);
}

/**
 * This function returns the quest log top level widget and creates it if necessary.
 */
struct widget_group *create_quest_browser()
{
	if (quest_browser)
		return quest_browser;

	quest_browser = widget_group_create();
	widget_set_rect(WIDGET(quest_browser), 0, 0, GameConfig.screen_width, GameConfig.screen_height);
	WIDGET(quest_browser)->handle_event = quest_browser_handle_event;

	// Dark background
	struct widget *dark_background = widget_create();
	widget_set_rect(dark_background, 0, 0, GameConfig.screen_width, GameConfig.screen_height);
	dark_background->display = display_dark_background;
	widget_group_add(quest_browser, dark_background);

	// Expand the quest browser as much as possible, keeping the main monitor's aspect ratio and
	// leaving some space for the buttons on the right side.

	// available screen space
	SDL_Rect available_space = {0, 30, GameConfig.screen_width - 127, GameConfig.screen_height - 60};

	// Size on the lowest resolution
	float quest_browser_w = 327;
	float quest_browser_h = 387;

	// Compute the maximum scale that can be applied.
	float scale = min(available_space.w / quest_browser_w, available_space.h / quest_browser_h);

	quest_browser_w *= scale;
	quest_browser_h *= scale;

	// Position the quest browser in the center of the screen.
	float quest_browser_x = available_space.x + (available_space.w - quest_browser_w) / 2;
	float quest_browser_y = available_space.y + (available_space.h - quest_browser_h) / 2;

	// Main body background
	struct widget_background *panel = widget_background_create();
	widget_set_rect(WIDGET(panel), quest_browser_x, quest_browser_y, quest_browser_w, quest_browser_h);
	widget_background_load_3x3_tiles(panel, "widgets/quest");
	widget_group_add(quest_browser, WIDGET(panel));

	// Text widget
	struct widget *text = widget_create();
	Set_Rect(mission_description_rect, quest_browser_x + 55, quest_browser_y + 35, quest_browser_w - 85, quest_browser_h - 70);
	text->display = text_display;
	widget_group_add(quest_browser, text);
	quest_browser_text = alloc_autostr(32);

	// Exit button arm background.
	int right_side_buttons_x = quest_browser_x + quest_browser_w;
	int exit_button_arm_y = quest_browser_y + quest_browser_h - 183;

	struct widget_background *exit_button = widget_background_create();
	struct image *img = widget_load_image_resource("widgets/exit_button_background.png", 0);
	widget_set_rect(WIDGET(exit_button), right_side_buttons_x - 5, exit_button_arm_y, img->w, img->h);
	widget_background_add(exit_button, img, WIDGET(exit_button)->rect.x, WIDGET(exit_button)->rect.y, img->w, img->h, 0);
	widget_group_add(quest_browser, WIDGET(exit_button));

	struct {
		char *image[3];
		SDL_Rect rect;
		void (*activate_button)(struct widget_button *);
		void (WIDGET_ANONYMOUS_MARKER update) (struct widget *);
	} b[] = {
		// Open quests
		{
			{"widgets/quest_open_off.png", NULL, "widgets/quest_open.png"},
			{right_side_buttons_x, quest_browser_y + 37, 126, 29},
			toggle_open_quests,
			WIDGET_ANONYMOUS(struct widget *w, {
				WIDGET_BUTTON(w)->active = current_quest_browser_mode == QUEST_BROWSER_SHOW_OPEN_MISSIONS;
			})
		},
		// Done quests
		{
			{"widgets/quest_done_off.png", NULL, "widgets/quest_done.png"},
			{right_side_buttons_x, quest_browser_y + 76, 126, 29},
			toggle_done_quests,
			WIDGET_ANONYMOUS(struct widget *w, {
				WIDGET_BUTTON(w)->active = current_quest_browser_mode == QUEST_BROWSER_SHOW_DONE_MISSIONS;
			})
		},
		// Notes
		{
			{"widgets/quest_notes_off.png", NULL, "widgets/quest_notes.png"},
			{right_side_buttons_x, quest_browser_y + 115, 126, 29},
			toggle_notes,
			WIDGET_ANONYMOUS(struct widget *w, {
				WIDGET_BUTTON(w)->active = current_quest_browser_mode == QUEST_BROWSER_SHOW_NOTES;
			})
		},
		// Exit button
		{
			{"widgets/exit_button_default.png", "widgets/exit_button_pressed.png", NULL},
			{WIDGET(exit_button)->rect.x + 16, WIDGET(exit_button)->rect.y + 54, 54, 55},
			(void(*)(struct widget_button *))toggle_quest_browser,
			NULL
		},
		// Scroll up
		{
			{"widgets/scroll_up_off.png", NULL, "widgets/scroll_up.png"},
			{quest_browser_x + quest_browser_w / 2 - 59, quest_browser_y - 14, 118, 17},
			scroll_up,
			WIDGET_ANONYMOUS(struct widget *w, {
				WIDGET_BUTTON(w)->active = can_scroll_up();
			})
		},
		// Scroll down
		{
			{"widgets/scroll_down_off.png", NULL, "widgets/scroll_down.png"},
			{quest_browser_x + quest_browser_w / 2 - 59, quest_browser_y + quest_browser_h, 118, 17}, 
			scroll_down,
			WIDGET_ANONYMOUS(struct widget *w, {
				WIDGET_BUTTON(w)->active = can_scroll_down();
			})
		}
	};

	int i;
	for (i = 0; i < sizeof(b) / sizeof(b[0]); i++) {
		struct widget_button *button = widget_button_create();

		button->image[0][DEFAULT] = widget_load_image_resource(b[i].image[0], 0);
		button->image[0][PRESSED] = widget_load_image_resource(b[i].image[1], 0);
		button->image[1][DEFAULT] = widget_load_image_resource(b[i].image[2], 0);

		WIDGET(button)->rect = b[i].rect;
		button->activate_button = b[i].activate_button;
		WIDGET(button)->update = b[i].update;
		widget_group_add(quest_browser, WIDGET(button));
	}

	return quest_browser;
}

#undef _quest_browser_c
