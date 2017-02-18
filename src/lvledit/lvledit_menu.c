/* 
 *
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

#define _leveleditor_menu_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_map.h"
#include "lvledit/lvledit_validator.h"
#include "lvledit/lvledit_menu.h"
#include "lvledit/lvledit_widgets.h"

#include "mapgen/mapgen.h"

/**
 * Get a natural value from the user
 * @param displayed_text Text to be displayed
 * @param suggested_val Suggested value for the number
 * @return Input number, -1 if the input is not made of digits only and -2 if the input is NULL
 */
int get_number_popup(const char *displayed_text, const char *suggested_val)
{
	int tgt = 0;
	char *str = get_editable_string_in_popup_window(10, displayed_text, suggested_val);
	if (str) {
		char *cstr = str;
		while (*cstr) {
			if (!isdigit(*cstr))
				tgt = -1;
			cstr++;
		}

		if (!tgt)
			tgt = atoi(str);
	} else
		tgt = -2;
	
	free(str);

	return (tgt);

}

/**
 * This a a menu interface to allow to edit the level dimensions in a
 * convenient way, i.e. so that little stupid copying work or things like
 * that have to be done and more time can be spent creating game material.
 */
void edit_level_dimensions(void)
{
	char *menu_texts[20];
	char options[20][1000];
	struct level *edit_level = curShip.AllLevels[Me.pos.z];

	enum {
		INSERTREMOVE_LINE_VERY_NORTH = 1,
		INSERTREMOVE_COLUMN_VERY_EAST,
		INSERTREMOVE_LINE_VERY_SOUTH,
		INSERTREMOVE_COLUMN_VERY_WEST,
		DUMMY_NO_REACTION1,
		DUMMY_NO_REACTION2,
		BACK_TO_LE_MAIN_MENU
	};

	int proceed_now = FALSE;
	while (!proceed_now) {

		InitiateMenu("--EDITOR_BACKGROUND--");

		int i = 0;
		sprintf(options[i], _("North Edge: -/+  (<-/->)"));
		menu_texts[i] = options[i];
		i++;

		sprintf(options[i], _("East Edge: -/+  (<-/->)"));
		menu_texts[i] = options[i];
		i++;

		sprintf(options[i], _("South Edge: -/+  (<-/->)"));
		menu_texts[i] = options[i];
		i++;

		sprintf(options[i], _("West Edge: -/+  (<-/->)"));
		menu_texts[i] = options[i];
		i++;

		sprintf(options[i], _("Current level size in X: %d."), edit_level->xlen);
		menu_texts[i] = options[i];
		i++;
		sprintf(options[i], _("Current level size in Y: %d."), edit_level->ylen);
		menu_texts[i] = options[i];
		i++;
		menu_texts[i++] = _("Back");
		menu_texts[i++] = "";

		int menu_position = do_menu_selection("", (char **)menu_texts, -1, "--EDITOR_BACKGROUND--", FPS_Display_Font);

		while (EnterPressed() || SpacePressed() || MouseLeftPressed())
			SDL_Delay(1);

		switch (menu_position) {
		case INSERTREMOVE_COLUMN_VERY_EAST:
			if (RightPressed()) {
				insert_column_east(edit_level);
				while (RightPressed()) ;
			}
			if (LeftPressed()) {
				remove_column_east(edit_level);
				while (LeftPressed()) ;
			}
			gps_transform_map_dirty_flag = TRUE;
			break;

		case INSERTREMOVE_COLUMN_VERY_WEST:
			if (RightPressed()) {
				insert_column_west(edit_level);
				while (RightPressed()) ;
			}
			if (LeftPressed()) {
				remove_column_west(edit_level);
				while (LeftPressed()) ;
			}
			gps_transform_map_dirty_flag = TRUE;
			break;

		case INSERTREMOVE_LINE_VERY_SOUTH:
			if (RightPressed()) {
				insert_line_south(edit_level);
				while (RightPressed()) ;
			}
			if (LeftPressed()) {
				remove_line_south(edit_level);
				while (LeftPressed()) ;
			}
			gps_transform_map_dirty_flag = TRUE;
			break;

		case INSERTREMOVE_LINE_VERY_NORTH:
			if (RightPressed()) {
				insert_line_north(edit_level);
				while (RightPressed()) ;
			}
			if (LeftPressed()) {
				remove_line_north(edit_level);
				while (LeftPressed()) ;
			}
			gps_transform_map_dirty_flag = TRUE;
			break;

		case (-1):
		case BACK_TO_LE_MAIN_MENU:
			while (EnterPressed() || SpacePressed() || EscapePressed() || MouseLeftPressed())
				SDL_Delay(1);
			proceed_now = !proceed_now;
			break;

		default:
			break;

		}

		gps_transform_map_init();

	}			// while (!proceed_now)

};				// void EditLevelDimensions ( void )

/**
 *
 *
 */
static void input_target_value(int *jump_target)
{
	while (EnterPressed() || SpacePressed() || MouseLeftPressed())
		SDL_Delay(1);

	int tgt = get_number_popup(_("\n Please enter new level number: \n\n"), "");

	if (tgt >= -1 && tgt < curShip.num_levels && curShip.AllLevels[tgt]) {
		*jump_target = tgt;
	}
	gps_transform_map_dirty_flag = TRUE;
}

static void change_target_value(int *jump_target)
{
	if (LeftPressed()) {
		int target = *jump_target;
		do {
			target--;
		} while((target > 0) && !curShip.AllLevels[target]);
		if (target >= 0)
			*jump_target = target;
		while (LeftPressed()) ;
	}
	if (RightPressed()) {
		int target = *jump_target;
		do {
			target++;
		} while((target < curShip.num_levels) && !curShip.AllLevels[target]);
		if (target < curShip.num_levels)
			*jump_target = target;
		while (RightPressed()) ;
	}
	gps_transform_map_dirty_flag = TRUE;
}

static void set_level_interfaces(void)
{
	char *menu_texts[100];
	char options[20][500];

	enum {
		JUMP_TARGET_NORTH = 1,
		JUMP_TARGET_EAST,
		JUMP_TARGET_SOUTH,
		JUMP_TARGET_WEST,
		QUIT_THRESHOLD_EDITOR_POSITION
	};

	int proceed_now = FALSE;
	while (!proceed_now) {
		struct level *edit_level = curShip.AllLevels[Me.pos.z];
		InitiateMenu("--EDITOR_BACKGROUND--");

		int i = 0;
		sprintf(options[i], _("Neighbor level North: %d.  (<-/->)"), edit_level->jump_target_north);
		menu_texts[i] = options[i];
		i++;
		sprintf(options[i], _("Neighbor level East: %d.  (<-/->)"), edit_level->jump_target_east);
		menu_texts[i] = options[i];
		i++;
		sprintf(options[i], _("Neighbor level South: %d.  (<-/->)"), edit_level->jump_target_south);
		menu_texts[i] = options[i];
		i++;
		sprintf(options[i], _("Neighbor level West: %d.  (<-/->)"), edit_level->jump_target_west);
		menu_texts[i] = options[i];
		i++;
		menu_texts[i++] = _("Back");
		menu_texts[i++] = "";

		int menu_position = do_menu_selection("", (char **)menu_texts, -1, "--EDITOR_BACKGROUND--", FPS_Display_Font);

		while (EnterPressed() || MouseLeftPressed())
			SDL_Delay(1);

		switch (menu_position) {
		case (-1):
			while (EscapePressed())
				SDL_Delay(1);
			proceed_now = !proceed_now;
			break;

		case QUIT_THRESHOLD_EDITOR_POSITION:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			proceed_now = !proceed_now;
			break;
		default:
			break;

		case JUMP_TARGET_NORTH:
			if (LeftPressed() || RightPressed()) {	//left or right arrow ? handled below 
				break;
			}
			input_target_value(&edit_level->jump_target_north);
			break;

		case JUMP_TARGET_EAST:
			if (LeftPressed() || RightPressed()) {	//left or right arrow ? handled below 
				break;
			}
			input_target_value(&edit_level->jump_target_east);
			break;

		case JUMP_TARGET_SOUTH:
			if (LeftPressed() || RightPressed()) {	//left or right arrow ? handled below 
				break;
			}
			input_target_value(&edit_level->jump_target_south);
			break;

		case JUMP_TARGET_WEST:
			if (LeftPressed() || RightPressed()) {	//left or right arrow ? handled below 
				break;
			}
			input_target_value(&edit_level->jump_target_west);
			break;

		}		// switch

		// If the user of the level editor pressed left or right, that should have
		// an effect IF he/she is a the change level menu point

		if (LeftPressed() || RightPressed()) {
			switch (menu_position) {
			case JUMP_TARGET_NORTH:
				change_target_value(&edit_level->jump_target_north);
				break;

			case JUMP_TARGET_SOUTH:
				change_target_value(&edit_level->jump_target_south);
				break;

			case JUMP_TARGET_EAST:
				change_target_value(&edit_level->jump_target_east);
				break;

			case JUMP_TARGET_WEST:
				change_target_value(&edit_level->jump_target_west);
				break;

			}
		}		// if LeftPressed || RightPressed

		gps_transform_map_init();
	}

};				// void SetLevelInterfaces ( void )

static void add_rem_level(void)
{
	char *menu_texts[100];

	enum {
		ADD_NEW_LEVEL = 1,
		REMOVE_CURRENT_LEVEL,
		LEAVE_OPTIONS_MENU,
	};

	game_status = INSIDE_MENU;

	int proceed_now = FALSE;
	while (!proceed_now) {

		InitiateMenu("--EDITOR_BACKGROUND--");

		int i = 0;
		menu_texts[i++] = _("Add New Level");
		menu_texts[i++] = _("Remove Current Level");
		menu_texts[i++] = _("Back");
		menu_texts[i++] = "";

		while (EscapePressed())
			SDL_Delay(1);

		int menu_position = do_menu_selection("", (char **)menu_texts, -1, "--EDITOR_BACKGROUND--", FPS_Display_Font);

		while (EnterPressed() || SpacePressed() || MouseLeftPressed())
			SDL_Delay(1);

		switch (menu_position) {
		case (-1):
			while (EscapePressed()) ;
			proceed_now = !proceed_now;
			break;
		case ADD_NEW_LEVEL:
			if (game_root_mode == ROOT_IS_GAME)
				break;
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			if (curShip.num_levels < MAX_LEVELS) {
				int new_level_num = curShip.num_levels;
				int i;
				// search empty level, if any
				for (i = 0; i < curShip.num_levels; ++i) {
					if (!level_exists(i)) {
						new_level_num = i;
						break;
					}
				}
				if (new_level_num == curShip.num_levels)
					curShip.num_levels += 1;
				CreateNewMapLevel(new_level_num);

				gps_transform_map_dirty_flag = TRUE;
				gps_transform_map_init();
				// Teleporting Tux will re-render the menu background
				reset_visible_levels();
				action_jump_to_level_center(new_level_num);
			}
			else      //curShip.num_levels >= MAX_LEVELS
				alert_window(_("Reached the maximum number of levels."));
			break;
		case REMOVE_CURRENT_LEVEL:
			if (game_root_mode == ROOT_IS_GAME)
				break;
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);

			if (EditLevel()->levelnum == 0) {
				alert_window(_("Cannot remove level number 0.\n"));
				break;
			}

			{
				int tmp = EditLevel()->levelnum; // needed due to Teleport changing EditLevel() result

				delete_map_level(tmp);

				lvledit_reset_tools();

				gps_transform_map_dirty_flag = TRUE;
				gps_transform_map_init();
				// Teleporting Tux will re-render the menu background
				reset_visible_levels();
				Teleport(0, 3, 3, FALSE, TRUE);
			}
			break;
		case LEAVE_OPTIONS_MENU:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			proceed_now = !proceed_now;
			break;
		default:
			break;

		}		// switch

	}

	game_status = INSIDE_LVLEDITOR;
	return;
};				// void AddRemLevel ( void );

static void level_options(void)
{
	char *menu_texts[100];
	char options[20][1000];
	char class[16];
	int l = 0;

	class[15] = 0; // null-character to null terminated strncpy

	enum {
		CHANGE_LEVEL_POSITION = 1,
		SET_LEVEL_NAME,
		EDIT_LEVEL_DIMENSIONS,
		EDIT_LEVEL_FLOOR_LAYERS,
		SET_LEVEL_INTERFACE_POSITION,
		SET_RANDOM_LEVEL,
		SET_RANDOM_DROIDS,
		SET_NO_RESPAWN,
		SET_DROP_CLASS,
		SET_TELEPORT_BLOCKED,
		SET_TELEPORT_PAIR,
		CHANGE_LIGHT,
		SET_BACKGROUND_SONG_NAME,
		CHANGE_INFINITE_RUNNING,
		ADD_REM_LEVEL,
		LEAVE_OPTIONS_MENU,
	};

	int proceed_now = FALSE;
	while (!proceed_now) {

		InitiateMenu("--EDITOR_BACKGROUND--");

		int i = 0;
		sprintf(options[i], _("Level: %d.  (<-/->)"), EditLevel()->levelnum);
		menu_texts[i] = options[i];
		i++;
		sprintf(options[i], _("Level name (untranslated): %s"), EditLevel()->Levelname);
		menu_texts[i] = options[i];
		i++;
		sprintf(options[i], _("Size:  X %d  Y %d"), EditLevel()->xlen, EditLevel()->ylen);
		menu_texts[i] = options[i];
		i++;
		sprintf(options[i], _("Floor layers:  %d"), EditLevel()->floor_layers);
		menu_texts[i] = options[i];
		i++;
		sprintf(options[i], _("Edge Interface: "));
		if (EditLevel()->jump_target_north == -1 && EditLevel()->jump_target_east == -1
		    && EditLevel()->jump_target_south == -1 && EditLevel()->jump_target_west == -1) {
			strcat(options[i], _("none"));
		} else {
			if (EditLevel()->jump_target_north != -1)
				sprintf(options[i] + strlen(options[i]), _(" N: %d"), EditLevel()->jump_target_north);
			if (EditLevel()->jump_target_east != -1)
				sprintf(options[i] + strlen(options[i]), _(" E: %d"), EditLevel()->jump_target_east);
			if (EditLevel()->jump_target_south != -1)
				sprintf(options[i] + strlen(options[i]), _(" S: %d"), EditLevel()->jump_target_south);
			if (EditLevel()->jump_target_west != -1)
				sprintf(options[i] + strlen(options[i]), _(" W: %d"), EditLevel()->jump_target_west);

		}
		menu_texts[i] = options[i];
		i++;

		if (EditLevel()->random_dungeon == 2) {
			sprintf(options[i], _("Random dungeon: 2 connections"));
		} else if (EditLevel()->random_dungeon == 1) {
			sprintf(options[i], _("Random dungeon: 1 connection"));
		} else {
			sprintf(options[i], _("Random dungeon: no"));
		}
		menu_texts[i] = options[i];
		i++;

		int *droid_types = EditLevel()->random_droids.types;
		sprintf(options[i], _("Random droids: %d Types: "), EditLevel()->random_droids.nr);
		int j;
		for (j = 0; j < EditLevel()->random_droids.types_size; j++) {
			if (j)
				strcat(options[i], ", ");
			strcat(options[i], Droidmap[droid_types[j]].droidname);
		}
		menu_texts[i] = options[i];
		i++;

		sprintf(options[i], "%s: %s", _("Respawn"),
			(EditLevel()->flags & NO_RESPAWN) ? _("no") : _("yes"));
		menu_texts[i] = options[i];
		i++;

		if (EditLevel()->drop_class >= 0 && EditLevel()->drop_class <= MAX_DROP_CLASS)
			snprintf(class, 16, "%d", EditLevel()->drop_class);
		else
			strncpy(class, _("None"), 15);

		sprintf(options[i], _("Item drop class for obstacles: %s.  (<-/->)"), class);
		menu_texts[i] = options[i];
		i++;

		sprintf(options[i], "%s: %s", _("Teleport blockade"),
			(EditLevel()->flags & TELEPORT_BLOCKED) ? _("yes") : _("no"));
		menu_texts[i] = options[i];
		i++;

		sprintf(options[i], "%s: %s", _("Teleport pair"),
				mapgen_teleport_pair_str(EditLevel()->teleport_pair));
		menu_texts[i] = options[i];
		i++;

		sprintf(options[i], _("Light:  Ambient"));
		if (l == 0) {
			sprintf(options[i + 1], " [%d]  ", EditLevel()->minimum_light_value);
			strcat(options[i], options[i + 1]);
			strcat(options[i], _("Bonus"));
			sprintf(options[i + 1], "  %d   (<-/->)", EditLevel()->light_bonus);
		} else if (l == 1) {
			sprintf(options[i + 1], "  %d   ", EditLevel()->minimum_light_value);
			strcat(options[i], options[i + 1]);
			strcat(options[i], _("Bonus"));
			sprintf(options[i + 1], " [%d]  (<-/->)", EditLevel()->light_bonus);
		} else
			sprintf(options[i + 1], "I'm a bug");
		strcat(options[i], options[i + 1]);
		menu_texts[i] = options[i];
		i++;

		sprintf(options[i], _("Background Music"));
		sprintf(options[i + 1], ": %s", EditLevel()->Background_Song_Name);
		strcat(options[i], options[i + 1]);
		menu_texts[i] = options[i];
		i++;
		sprintf(options[i], _("Infinite Running Stamina: "));
		if (EditLevel()->infinite_running_on_this_level)
			strcat(options[i], _("yes"));
		else
			(strcat(options[i], _("no")));
		menu_texts[i] = options[i];
		i++;
		menu_texts[i++] = _("Add/Rem Level");
		menu_texts[i++] = _("Back");
		menu_texts[i++] = "";

		while (EscapePressed())
			SDL_Delay(1);

		int menu_position = do_menu_selection("", (char **)menu_texts, -1, "--EDITOR_BACKGROUND--", FPS_Display_Font);

		while (EnterPressed() || SpacePressed() || MouseLeftPressed())
			SDL_Delay(1);

		switch (menu_position) {
		case (-1):
			while (EscapePressed()) ;
			proceed_now = !proceed_now;
			break;
		case CHANGE_LEVEL_POSITION:
			if (LeftPressed() || RightPressed()) {	//left or right arrow ? handled below 
				break;
			}
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);

			int tgt = get_number_popup(_("\n Please enter new level number: \n\n"), "");

			if (tgt >= 0 && tgt < curShip.num_levels) {
				if (level_exists(tgt)) {
					reset_visible_levels();
					action_jump_to_level_center(tgt);
				}
				proceed_now = !proceed_now;
			}
			break;
		case SET_RANDOM_LEVEL:
			EditLevel()->random_dungeon++;
			EditLevel()->random_dungeon %= 3;
			break;
		case SET_RANDOM_DROIDS:
			get_random_droids_from_user();
			break;
		case SET_NO_RESPAWN:
			EditLevel()->flags ^= NO_RESPAWN;
			break;
		case SET_DROP_CLASS:
			if (LeftPressed() || RightPressed()) {	//left or right arrow ? handled below 
				break;
			}
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);

			if (EditLevel()->drop_class >= 0 && EditLevel()->drop_class <= MAX_DROP_CLASS)
				snprintf(class, 16, "%d", EditLevel()->drop_class);
			else
				strcpy(class, "");

			tgt = get_number_popup(_("\n Please enter new drop class: \n\n"), class);
			if (tgt < -1)
				EditLevel()->drop_class = -1;
			else if (tgt > MAX_DROP_CLASS)
				EditLevel()->drop_class = MAX_DROP_CLASS;
			else
				EditLevel()->drop_class = tgt;
				
			break;
		case SET_TELEPORT_BLOCKED:
			EditLevel()->flags ^= TELEPORT_BLOCKED;
			break;
		case SET_TELEPORT_PAIR:
			EditLevel()->teleport_pair = mapgen_cycle_teleport_pair(EditLevel()->teleport_pair);
			break;
		case CHANGE_LIGHT:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			// Switch between Radius and Minimum modification mode.
			if (l == 0) {
				if (LeftPressed())
					l = 0;
				else if (RightPressed())
					l = 0;
				else
					l = 1;
			} else if (l == 1) {
				if (LeftPressed())
					l = 1;
				else if (RightPressed())
					l = 1;
				else
					l = 0;
			} else
				l = 2;
			break;
		case SET_LEVEL_NAME:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			char *newLevelname = get_editable_string_in_popup_window(1000, _("\n Please enter new level name (in English): \n\n"), EditLevel()->Levelname);
			if (newLevelname)
				EditLevel()->Levelname = newLevelname;
			break;
		case SET_BACKGROUND_SONG_NAME:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			char* newBackgroundSongName =
				get_editable_string_in_popup_window(1000, _("\n Please enter new music file name: \n\n"),
							   EditLevel()->Background_Song_Name);
			if (newBackgroundSongName)
				EditLevel()->Background_Song_Name = newBackgroundSongName;
			break;
		case SET_LEVEL_INTERFACE_POSITION:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			set_level_interfaces();
			break;
		case EDIT_LEVEL_DIMENSIONS:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			edit_level_dimensions();
			break;
		case EDIT_LEVEL_FLOOR_LAYERS:
			if (LeftPressed() || RightPressed())
				break;
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			char popup_message[1000];
			sprintf(popup_message, _("\n Please enter new number of floor layers (valid values are between 1 and %d): \n\n"), MAX_FLOOR_LAYERS);
			char *new_floor_layers = get_editable_string_in_popup_window(1000, popup_message, "");
			if (new_floor_layers) {
				char *endptr;
				long layers = strtol(new_floor_layers, &endptr, 10);
				if (layers > MAX_FLOOR_LAYERS) layers = MAX_FLOOR_LAYERS;
				else if (layers < 1) layers = 1;
				if (*endptr != *new_floor_layers)
					EditLevel()->floor_layers = layers;
				free(new_floor_layers);
			}
			break;
		case CHANGE_INFINITE_RUNNING:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			EditLevel()->infinite_running_on_this_level = !EditLevel()->infinite_running_on_this_level;
			break;
		case ADD_REM_LEVEL:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			add_rem_level();
			break;
		case LEAVE_OPTIONS_MENU:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			proceed_now = !proceed_now;
			break;
		default:
			break;

		}		// switch

		// If the user of the level editor pressed left or right, that should have
		// an effect IF he/she is a the change level menu point

		if (LeftPressed() || RightPressed()) {
			switch (menu_position) {

			case CHANGE_LEVEL_POSITION:
				if (LeftPressed()) {
					// find first available level lower than the current one
					int newlevel = EditLevel()->levelnum - 1;
					while (!level_exists(newlevel) && newlevel >= 0)
						--newlevel;
					// teleport if new level exists
					if (newlevel >= 0) {
						reset_visible_levels();
						action_jump_to_level_center(newlevel);
					}
					while (LeftPressed()) ;
				}
				if (RightPressed()) {
					// find first available level higher than the current one
					int newlevel = EditLevel()->levelnum + 1;
					while (!level_exists(newlevel) && newlevel < curShip.num_levels)
						++newlevel;
					// teleport if new level exists
					if (newlevel < curShip.num_levels) {
						reset_visible_levels();
						action_jump_to_level_center(newlevel);
					}
					while (RightPressed()) ;
				}
				break;

			case EDIT_LEVEL_FLOOR_LAYERS:
				if (LeftPressed()) {
					int new_layers = EditLevel()->floor_layers - 1;
					if (new_layers > 0) {
						int x, y;
						EditLevel()->floor_layers = new_layers;
						// Clear the removed floor layer
						for (y = 0; y < EditLevel()->ylen; y++) {
							for (x = 0; x < EditLevel()->xlen; x++) {
								EditLevel()->map[y][x].floor_values[new_layers] = ISO_FLOOR_EMPTY;
							}
						}
					}
					if (current_floor_layer >= EditLevel()->floor_layers)
						current_floor_layer = EditLevel()->floor_layers - 1;
					while (LeftPressed()) ;
				}
				if (RightPressed()) {
					int new_layers = EditLevel()->floor_layers + 1;
					if (new_layers <= MAX_FLOOR_LAYERS)
						EditLevel()->floor_layers = new_layers;
					while (RightPressed()) ;
				}
				break;
			case SET_DROP_CLASS:
				if (LeftPressed()) {
					EditLevel()->drop_class--;
					if (EditLevel()->drop_class < -1)
						EditLevel()->drop_class = -1;
					while (LeftPressed());
				}
				if (RightPressed()) {
					EditLevel()->drop_class++;
					if (EditLevel()->drop_class > MAX_DROP_CLASS)
						EditLevel()->drop_class = MAX_DROP_CLASS;
					while (RightPressed());
				}
				break;

			case CHANGE_LIGHT:
				if (l == 0) {
					if (RightPressed()) {
						EditLevel()->minimum_light_value++;
						if (EditLevel()->minimum_light_value > (NUMBER_OF_SHADOW_IMAGES-1))
							EditLevel()->minimum_light_value = (NUMBER_OF_SHADOW_IMAGES-1);
						while (RightPressed()) ;
					}
					if (LeftPressed()) {
						EditLevel()->minimum_light_value--;
						if (EditLevel()->minimum_light_value < 0)
							EditLevel()->minimum_light_value = 0;
						while (LeftPressed()) ;
					}
					reset_visible_levels();
					Teleport(EditLevel()->levelnum, Me.pos.x, Me.pos.y, FALSE, TRUE);
					break;
				} else {
					if (RightPressed()) {
						EditLevel()->light_bonus++;
						while (RightPressed()) ;
					}
					if (LeftPressed()) {
						EditLevel()->light_bonus--;
						if (EditLevel()->light_bonus < 0)
							EditLevel()->light_bonus = 0;
						while (LeftPressed()) ;
					}
					reset_visible_levels();
					Teleport(EditLevel()->levelnum, Me.pos.x, Me.pos.y, FALSE, TRUE);
					break;
				}

			}	// switch
		}		// if LeftPressed || RightPressed
	}
	return;
};				// void LevelOptions ( void );

void choose_game_act()
{
	char **menu_texts = NULL;

	game_status = INSIDE_MENU;

	char *menu_title = _("Choose a game act\n[r]Warning: Unsaved changes will be lost[n]");
	struct game_act *new_act = NULL;

	for (;;) {

		InitiateMenu("--EDITOR_BACKGROUND--");

		// Fill the menu entries text with the name of the game acts
		// + a 'back' entry + the empty guard
		menu_texts = (char **)MyMalloc(sizeof(char *)*(game_acts.size + 2));
		int cpt;
		for (cpt = 0; cpt < game_acts.size; cpt++) {
			struct game_act *act = (struct game_act *)dynarray_member(&game_acts, cpt, sizeof(struct game_act));
			menu_texts[cpt] = act->name;
		}
		int back_position = cpt;
		menu_texts[cpt++] = _("Back");
		menu_texts[cpt] = "";

		// Note: DoMenuSelection() returned value is the menu index + 1,
		// or -1 if the menu was escaped
		// We default to the 'Back' entry
		int selected_position = do_menu_selection(menu_title, menu_texts, back_position, "--EDITOR_BACKGROUND--", FPS_Display_Font) - 1;
		if (selected_position == -2 || selected_position == back_position) {
			break;
		}

		new_act = game_act_get_by_name(menu_texts[selected_position]);
		break;
	}

	if (menu_texts)
		free(menu_texts);

	game_status = INSIDE_LVLEDITOR;
	if (new_act) {
		leveleditor_cleanup();
		prepare_level_editor(new_act);
	}
}

/**
 *
 *
 */
int do_level_editor_main_menu()
{
	char *menu_texts[100];
	char options[20][1000];
	int done = FALSE;

	game_status = INSIDE_MENU;

	//hack : eat pending events
	input_handle();

	enum {
		ENTER_GAME_ACT_POSITION = 1,
		ENTER_LEVEL_POSITION,
		LEVEL_OPTIONS_POSITION,
		RUN_MAP_VALIDATION,
		TEST_MAP_POSITION,
		SAVE_LEVEL_POSITION,
//              MANAGE_LEVEL_POSITION,
		ESCAPE_FROM_MENU_POSITION,
                SHOW_HELP,
		QUIT_TO_MAIN_POSITION,
		QUIT_POSITION,
	};

	MenuItemSelectedSound();

	int proceed_now = FALSE;
	while (!proceed_now) {

		InitiateMenu("--EDITOR_BACKGROUND--");

		int i = 0;
		sprintf(options[i], _("Game act (untranslated): %s"), game_act_get_current()->name);
		menu_texts[i] = options[i];
		i++;
		sprintf(options[i], _("Level name (untranslated): %d - %s"), EditLevel()->levelnum, EditLevel()->Levelname);
		menu_texts[i] = options[i];
		i++;
		menu_texts[i++] = _("Level Options");
		menu_texts[i++] = _("Run Map Level Validator");
		if (game_root_mode == ROOT_IS_LVLEDIT) {
			menu_texts[i++] = _("Playtest Mapfile");
			menu_texts[i++] = _("Save Mapfile");
			//              MenuTexts[i++] = _("Manage Mapfiles");
		} else {
			menu_texts[i++] = _("Return to Game");
			menu_texts[i++] = _("Saving disabled");
			//              MenuTexts[i++] = " ";
		}
		menu_texts[i++] = _("Continue Editing");
		menu_texts[i++] = _("Show Help");
		menu_texts[i++] = _("Quit to Main Menu");
		menu_texts[i++] = _("Exit FreedroidRPG");
		menu_texts[i++] = "";

		while (EscapePressed())
			SDL_Delay(1);

		int menu_position = do_menu_selection("", (char **)menu_texts, -1, "--EDITOR_BACKGROUND--", FPS_Display_Font);

		while (EnterPressed() || SpacePressed() || MouseLeftPressed())
			SDL_Delay(1);

		switch (menu_position) {

		case (-1):
			while (EscapePressed()) ;
			proceed_now = !proceed_now;
			break;
		case ESCAPE_FROM_MENU_POSITION:
			proceed_now = !proceed_now;
			break;
		case ENTER_GAME_ACT_POSITION:
			choose_game_act();
			break;
		case ENTER_LEVEL_POSITION:
			if (LeftPressed() || RightPressed()) {	//left or right arrow ? handled below 
				break;
			}
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);

			int tgt = get_number_popup(_("\n Please enter new level number: \n\n"), "");

			if (tgt >= 0 && tgt < curShip.num_levels) {
				if (level_exists(tgt)) {
					reset_visible_levels();
					action_jump_to_level_center(tgt);
				}
				proceed_now = !proceed_now;
			}
			break;
		case LEVEL_OPTIONS_POSITION:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			level_options();
			break;
		case RUN_MAP_VALIDATION:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			level_validation();
			break;
		case TEST_MAP_POSITION:
			TestMap();
			proceed_now = !proceed_now;
			if (game_root_mode == ROOT_IS_GAME)
				done = TRUE;
			break;
		case SAVE_LEVEL_POSITION:
			if (game_root_mode == ROOT_IS_GAME)	/*don't allow saving if root mode is GAME */
				break;
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			save_map();
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			break;
                case SHOW_HELP:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			play_title_file(BASE_TITLES_DIR, "level_editor_help.lua");
			break;
		case QUIT_TO_MAIN_POSITION:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			if (game_root_mode == ROOT_IS_GAME) {
				proceed_now = !proceed_now;
				GameOver = TRUE;
				done = TRUE;
			}
			if (game_root_mode == ROOT_IS_LVLEDIT) {
				proceed_now = !proceed_now;
				done = TRUE;
			}
			break;
		case QUIT_POSITION:
			DebugPrintf(2, "\nvoid EscapeMenu( void ): Quit Requested by user.  Terminating...");
			Terminate(EXIT_SUCCESS);
			break;
		default:
			break;

		}		// switch

		if (LeftPressed() || RightPressed()) {
			switch (menu_position) {
			
			case ENTER_LEVEL_POSITION:
				if (LeftPressed()) {
					// find first available level lower than the current one
					int newlevel = EditLevel()->levelnum - 1;
					while (!level_exists(newlevel) && newlevel >= 0)
						--newlevel;
					// teleport if new level exists
					if (newlevel >= 0) {
						reset_visible_levels();
						action_jump_to_level_center(newlevel);
					}
					while (LeftPressed()) ;
				}
				if (RightPressed()) {
					// find first available level higher than the current one
					int newlevel = EditLevel()->levelnum + 1;
					while (!level_exists(newlevel) && newlevel < curShip.num_levels)
						++newlevel;
					// teleport if new level exists
					if (newlevel < curShip.num_levels) {
						reset_visible_levels();
						action_jump_to_level_center(newlevel);
					}
					while (RightPressed()) ;
				}
				break;

			}
		}		// if LeftPressed || RightPressed

	}
	game_status = INSIDE_LVLEDITOR;
	return (done);
};
