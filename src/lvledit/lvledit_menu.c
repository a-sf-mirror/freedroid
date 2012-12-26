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

#define _leveleditor_menu_c

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
	char *str, *cstr;
	int tgt = 0;
	str = GetEditableStringInPopupWindow(10, displayed_text, suggested_val);
	if (str) {
		cstr = str;
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
void EditLevelDimensions(void)
{
	char *MenuTexts[20];
	char Options[20][1000];
	int MenuPosition = 1;
	int proceed_now = FALSE;
	int i;
	Level EditLevel;

	EditLevel = curShip.AllLevels[Me.pos.z];

	enum {
		INSERTREMOVE_LINE_VERY_NORTH = 1,
		INSERTREMOVE_COLUMN_VERY_EAST,
		INSERTREMOVE_LINE_VERY_SOUTH,
		INSERTREMOVE_COLUMN_VERY_WEST,
		DUMMY_NO_REACTION1,
		DUMMY_NO_REACTION2,
		BACK_TO_LE_MAIN_MENU
	};

	while (!proceed_now) {

		InitiateMenu("--EDITOR_BACKGROUND--");

		i = 0;
		sprintf(Options[i], _("North"));
		strcat(Options[i], " ");
		strcat(Options[i], _("Edge"));
		strcat(Options[i], ": -/+  (<-/->)");
		MenuTexts[i] = Options[i];
		i++;

		sprintf(Options[i], _("East"));
		strcat(Options[i], " ");
		strcat(Options[i], _("Edge"));
		strcat(Options[i], ": -/+  (<-/->)");
		MenuTexts[i] = Options[i];
		i++;

		sprintf(Options[i], _("South"));
		strcat(Options[i], " ");
		strcat(Options[i], _("Edge"));
		strcat(Options[i], ": -/+  (<-/->)");
		MenuTexts[i] = Options[i];
		i++;

		sprintf(Options[i], _("West"));
		strcat(Options[i], " ");
		strcat(Options[i], _("Edge"));
		strcat(Options[i], ": -/+  (<-/->)");
		MenuTexts[i] = Options[i];
		i++;

		sprintf(Options[i], _("Current level size in X: %d."), EditLevel->xlen);
		MenuTexts[i] = Options[i];
		i++;
		sprintf(Options[i], _("Current level size in Y: %d."), EditLevel->ylen);
		MenuTexts[i] = Options[i];
		i++;
		MenuTexts[i++] = _("Back");
		MenuTexts[i++] = "";

		MenuPosition = DoMenuSelection("", MenuTexts, -1, "--EDITOR_BACKGROUND--", FPS_Display_BFont);

		while (EnterPressed() || SpacePressed() || MouseLeftPressed())
			SDL_Delay(1);

		switch (MenuPosition) {
		case INSERTREMOVE_COLUMN_VERY_EAST:
			if (RightPressed()) {
				insert_column_east(EditLevel);
				while (RightPressed()) ;
			}
			if (LeftPressed()) {
				remove_column_east(EditLevel);
				while (LeftPressed()) ;
			}
			gps_transform_map_dirty_flag = TRUE;
			break;

		case INSERTREMOVE_COLUMN_VERY_WEST:
			if (RightPressed()) {
				insert_column_west(EditLevel);
				while (RightPressed()) ;
			}
			if (LeftPressed()) {
				remove_column_west(EditLevel);
				while (LeftPressed()) ;
			}
			gps_transform_map_dirty_flag = TRUE;
			break;

		case INSERTREMOVE_LINE_VERY_SOUTH:
			if (RightPressed()) {
				insert_line_south(EditLevel);
				while (RightPressed()) ;
			}
			if (LeftPressed()) {
				remove_line_south(EditLevel);
				while (LeftPressed()) ;
			}
			gps_transform_map_dirty_flag = TRUE;
			break;

		case INSERTREMOVE_LINE_VERY_NORTH:
			if (RightPressed()) {
				insert_line_north(EditLevel);
				while (RightPressed()) ;
			}
			if (LeftPressed()) {
				remove_line_north(EditLevel);
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
static void SetLevelInterfaces(void)
{
	char *MenuTexts[100];
	int proceed_now = FALSE;
	int MenuPosition = 1;
	int tgt = -1;
	char Options[20][500];
	int i;
	Level EditLevel;

	enum {
		JUMP_TARGET_NORTH = 1,
		JUMP_TARGET_EAST,
		JUMP_TARGET_SOUTH,
		JUMP_TARGET_WEST,
		QUIT_THRESHOLD_EDITOR_POSITION
	};

	while (!proceed_now) {
		EditLevel = curShip.AllLevels[Me.pos.z];
		InitiateMenu("--EDITOR_BACKGROUND--");

		i = 0;

		sprintf(Options[i], _("Jump target"));
		strcat(Options[i], " ");
		strcat(Options[i], _("North"));
		sprintf(Options[i + 1], ": %d.  (<-/->)", EditLevel->jump_target_north);
		strcat(Options[i], Options[i + 1]);
		MenuTexts[i] = Options[i];
		i++;
		sprintf(Options[i], _("Jump target"));
		strcat(Options[i], " ");
		strcat(Options[i], _("East"));
		sprintf(Options[i + 1], ": %d.  (<-/->)", EditLevel->jump_target_east);
		strcat(Options[i], Options[i + 1]);
		MenuTexts[i] = Options[i];
		i++;
		sprintf(Options[i], _("Jump target"));
		strcat(Options[i], " ");
		strcat(Options[i], _("South"));
		sprintf(Options[i + 1], ": %d.  (<-/->)", EditLevel->jump_target_south);
		strcat(Options[i], Options[i + 1]);
		MenuTexts[i] = Options[i];
		i++;
		sprintf(Options[i], _("Jump target"));
		strcat(Options[i], " ");
		strcat(Options[i], _("West"));
		sprintf(Options[i + 1], ": %d.  (<-/->)", EditLevel->jump_target_west);
		strcat(Options[i], Options[i + 1]);
		MenuTexts[i] = Options[i];
		i++;
		MenuTexts[i++] = _("Back");
		MenuTexts[i++] = "";

		MenuPosition = DoMenuSelection("", MenuTexts, -1, "--EDITOR_BACKGROUND--", FPS_Display_BFont);

		while (EnterPressed() || MouseLeftPressed())
			SDL_Delay(1);

		switch (MenuPosition) {
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
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);

			tgt = get_number_popup("\n Please enter new level number: \n\n", "");

			if (tgt >= -1 && tgt < curShip.num_levels) {
				EditLevel->jump_target_north = (tgt);
			}
			gps_transform_map_dirty_flag = TRUE;
			break;

		case JUMP_TARGET_EAST:
			if (LeftPressed() || RightPressed()) {	//left or right arrow ? handled below 
				break;
			}
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);

			tgt = get_number_popup("\n Please enter new level number: \n\n", "");

			if (tgt >= -1 && tgt < curShip.num_levels) {
				EditLevel->jump_target_east = (tgt);
			}
			gps_transform_map_dirty_flag = TRUE;
			break;

		case JUMP_TARGET_SOUTH:
			if (LeftPressed() || RightPressed()) {	//left or right arrow ? handled below 
				break;
			}
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);

			tgt = get_number_popup("\n Please enter new level number: \n\n", "");

			if (tgt >= -1 && tgt < curShip.num_levels) {
				EditLevel->jump_target_south = (tgt);
			}
			gps_transform_map_dirty_flag = TRUE;
			break;

		case JUMP_TARGET_WEST:
			if (LeftPressed() || RightPressed()) {	//left or right arrow ? handled below 
				break;
			}
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);

			tgt = get_number_popup("\n Please enter new level number: \n\n", "");

			if (tgt >= -1 && tgt < curShip.num_levels) {
				EditLevel->jump_target_west = (tgt);
			}
			gps_transform_map_dirty_flag = TRUE;
			break;

		}		// switch

		// If the user of the level editor pressed left or right, that should have
		// an effect IF he/she is a the change level menu point

		if (LeftPressed() || RightPressed()) {
			switch (MenuPosition) {
			case JUMP_TARGET_NORTH:
				if (LeftPressed()) {
					if (EditLevel->jump_target_north >= 0)
						EditLevel->jump_target_north--;
					while (LeftPressed()) ;
				}
				if (RightPressed()) {
					if ((EditLevel->jump_target_north + 1) < curShip.num_levels)
						EditLevel->jump_target_north++;
					while (RightPressed()) ;
				}
				gps_transform_map_dirty_flag = TRUE;
				break;

			case JUMP_TARGET_SOUTH:
				if (LeftPressed()) {
					if (EditLevel->jump_target_south >= 0)
						EditLevel->jump_target_south--;
					while (LeftPressed()) ;
				}
				if (RightPressed()) {
					if ((EditLevel->jump_target_south + 1) < curShip.num_levels)
						EditLevel->jump_target_south++;
					while (RightPressed()) ;
				}
				gps_transform_map_dirty_flag = TRUE;
				break;

			case JUMP_TARGET_EAST:
				if (LeftPressed()) {
					if (EditLevel->jump_target_east >= 0)
						EditLevel->jump_target_east--;
					while (LeftPressed()) ;
				}
				if (RightPressed()) {
					if ((EditLevel->jump_target_east + 1) < curShip.num_levels)
						EditLevel->jump_target_east++;
					while (RightPressed()) ;
				}
				gps_transform_map_dirty_flag = TRUE;
				break;

			case JUMP_TARGET_WEST:
				if (LeftPressed()) {
					if (EditLevel->jump_target_west >= 0)
						EditLevel->jump_target_west--;
					while (LeftPressed()) ;
				}
				if (RightPressed()) {
					if ((EditLevel->jump_target_west + 1) < curShip.num_levels)
						EditLevel->jump_target_west++;
					while (RightPressed()) ;
				}
				gps_transform_map_dirty_flag = TRUE;
				break;

			}
		}		// if LeftPressed || RightPressed

		gps_transform_map_init();
	}

};				// void SetLevelInterfaces ( void )

static void AddRemLevel(void)
{
	char *MenuTexts[100];
	int proceed_now = FALSE;
	int MenuPosition = 1;
	int i;

	enum {
		ADD_NEW_LEVEL = 1,
		REMOVE_CURRENT_LEVEL,
		LEAVE_OPTIONS_MENU,
	};

	game_status = INSIDE_MENU;

	while (!proceed_now) {
		InitiateMenu("--EDITOR_BACKGROUND--");
		i = 0;
		MenuTexts[i++] = _("Add New Level");
		MenuTexts[i++] = _("Remove Current Level");
		MenuTexts[i++] = _("Back");
		MenuTexts[i++] = "";

		while (EscapePressed())
			SDL_Delay(1);

		MenuPosition = DoMenuSelection("", MenuTexts, -1, "--EDITOR_BACKGROUND--", FPS_Display_BFont);

		while (EnterPressed() || SpacePressed() || MouseLeftPressed())
			SDL_Delay(1);

		switch (MenuPosition) {
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
				alert_window("%s", _("Reached the maximum number of levels."));
			break;
		case REMOVE_CURRENT_LEVEL:
			if (game_root_mode == ROOT_IS_GAME)
				break;
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);

			if (EditLevel()->levelnum == 0) {
				alert_window("%s", _("Cannot remove level number 0.\n"));
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

static void LevelOptions(void)
{
	char *MenuTexts[100];
	char Options[20][1000];
	int proceed_now = FALSE;
	int MenuPosition = 1;
	int tgt = -1;
	int i, j;
	int l = 0;
	int *droid_types;

	enum {
		CHANGE_LEVEL_POSITION = 1,
		SET_LEVEL_NAME,
		EDIT_LEVEL_DIMENSIONS,
		EDIT_LEVEL_FLOOR_LAYERS,
		SET_LEVEL_INTERFACE_POSITION,
		SET_RANDOM_LEVEL,
		SET_RANDOM_DROIDS,
		SET_TELEPORT_BLOCKED,
		SET_TELEPORT_PAIR,
		CHANGE_LIGHT,
		SET_BACKGROUND_SONG_NAME,
		CHANGE_INFINITE_RUNNING,
		ADD_REM_LEVEL,
		LEAVE_OPTIONS_MENU,
	};

	while (!proceed_now) {

		InitiateMenu("--EDITOR_BACKGROUND--");

		i = 0;
		sprintf(Options[i], _("Level"));
		sprintf(Options[i + 1], ": %d.  (<-/->)", EditLevel()->levelnum);
		strcat(Options[i], Options[i + 1]);
		MenuTexts[i] = Options[i];
		i++;
		sprintf(Options[i], _("Name"));
		sprintf(Options[i + 1], ": %s", EditLevel()->Levelname);
		strcat(Options[i], Options[i + 1]);
		MenuTexts[i] = Options[i];
		i++;
		sprintf(Options[i], _("Size"));
		sprintf(Options[i + 1], ":  X %d  Y %d ", EditLevel()->xlen, EditLevel()->ylen);
		strcat(Options[i], Options[i + 1]);
		MenuTexts[i] = Options[i];
		i++;
		sprintf(Options[i], _("Floor layers"));
		sprintf(Options[i + 1], ":  %d", EditLevel()->floor_layers);
		strcat(Options[i], Options[i + 1]);
		MenuTexts[i] = Options[i];
		i++;
		sprintf(Options[i], _("Edge"));
		strcat(Options[i], " ");
		sprintf(Options[i + 1], _("Interface"));
		strcat(Options[i], Options[i + 1]);
		strcat(Options[i], ":");
		if (EditLevel()->jump_target_north == -1 && EditLevel()->jump_target_east == -1
		    && EditLevel()->jump_target_south == -1 && EditLevel()->jump_target_west == -1) {
			strcat(Options[i], " none");
		} else {
			if (EditLevel()->jump_target_north != -1)
				sprintf(Options[i] + strlen(Options[i]), "  N: %d", EditLevel()->jump_target_north);
			if (EditLevel()->jump_target_east != -1)
				sprintf(Options[i] + strlen(Options[i]), "  E: %d", EditLevel()->jump_target_east);
			if (EditLevel()->jump_target_south != -1)
				sprintf(Options[i] + strlen(Options[i]), "  S: %d", EditLevel()->jump_target_south);
			if (EditLevel()->jump_target_west != -1)
				sprintf(Options[i] + strlen(Options[i]), "  W: %d", EditLevel()->jump_target_west);

		}
		MenuTexts[i] = Options[i];
		i++;

		sprintf(Options[i], "%s: %s", _("Random dungeon"),
			EditLevel()->random_dungeon == 2 ? _("2 connections") : EditLevel()->random_dungeon ==
			1 ? _("1 connection") : _("no"));
		MenuTexts[i] = Options[i];
		i++;

		droid_types = EditLevel()->random_droids.types;
		sprintf(Options[i], "Random droids: %d Types: ", EditLevel()->random_droids.nr);
		for (j = 0; j < EditLevel()->random_droids.types_size; j++) {
				if (j)
					strcat(Options[i], ", ");
				strcat(Options[i], Droidmap[droid_types[j]].droidname);
		}
		MenuTexts[i] = Options[i];
		i++;

		sprintf(Options[i], "%s: %s", _("Teleport blockade"),
			EditLevel()->flags & TELEPORT_BLOCKED ? _("yes") : _("no"));
		MenuTexts[i] = Options[i];
		i++;

		sprintf(Options[i], "%s: %s", _("Teleport pair"),
				mapgen_teleport_pair_str(EditLevel()->teleport_pair));
		MenuTexts[i] = Options[i];
		i++;

		sprintf(Options[i], _("Light"));
		strcat(Options[i], ":  ");
		strcat(Options[i], _("Ambient"));
		if (l == 0) {
			sprintf(Options[i + 1], " [%d]  ", EditLevel()->minimum_light_value);
			strcat(Options[i], Options[i + 1]);
			strcat(Options[i], _("Bonus"));
			sprintf(Options[i + 1], "  %d   (<-/->)", EditLevel()->light_bonus);
		} else if (l == 1) {
			sprintf(Options[i + 1], "  %d   ", EditLevel()->minimum_light_value);
			strcat(Options[i], Options[i + 1]);
			strcat(Options[i], _("Bonus"));
			sprintf(Options[i + 1], " [%d]  (<-/->)", EditLevel()->light_bonus);
		} else
			sprintf(Options[i + 1], "I'm a bug");
		strcat(Options[i], Options[i + 1]);
		MenuTexts[i] = Options[i];
		i++;

		sprintf(Options[i], _("Background Music"));
		sprintf(Options[i + 1], ": %s", EditLevel()->Background_Song_Name);
		strcat(Options[i], Options[i + 1]);
		MenuTexts[i] = Options[i];
		i++;
		sprintf(Options[i], _("Infinite Running Stamina"));
		strcat(Options[i], ": ");
		if (EditLevel()->infinite_running_on_this_level)
			strcat(Options[i], _("yes"));
		else
			(strcat(Options[i], _("no")));
		MenuTexts[i] = Options[i];
		i++;
		MenuTexts[i++] = _("Add/Rem Level");
		MenuTexts[i++] = _("Back");
		MenuTexts[i++] = "";

		while (EscapePressed())
			SDL_Delay(1);

		MenuPosition = DoMenuSelection("", MenuTexts, -1, "--EDITOR_BACKGROUND--", FPS_Display_BFont);

		while (EnterPressed() || SpacePressed() || MouseLeftPressed())
			SDL_Delay(1);

		switch (MenuPosition) {
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

			tgt = get_number_popup("\n Please enter new level number: \n\n", "");

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
			char *newLevelname = GetEditableStringInPopupWindow(1000, _("\n Please enter new level name: \n\n"), EditLevel()->Levelname);
			if (newLevelname)
				EditLevel()->Levelname = newLevelname;
			break;
		case SET_BACKGROUND_SONG_NAME:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			char* newBackgroundSongName =
				GetEditableStringInPopupWindow(1000, _("\n Please enter new music file name: \n\n"),
							   EditLevel()->Background_Song_Name);
			if (newBackgroundSongName)
				EditLevel()->Background_Song_Name = newBackgroundSongName;
			break;
		case SET_LEVEL_INTERFACE_POSITION:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			SetLevelInterfaces();
			break;
		case EDIT_LEVEL_DIMENSIONS:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			EditLevelDimensions();
			break;
		case EDIT_LEVEL_FLOOR_LAYERS:
			if (LeftPressed() || RightPressed())
				break;
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			char popup_message[1000];
			sprintf(popup_message, _("\n Please enter new number of floor layers (valid values are between 1 and %d): \n\n"), MAX_FLOOR_LAYERS);
			char *new_floor_layers = GetEditableStringInPopupWindow(1000, popup_message, "");
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
			AddRemLevel();
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
			switch (MenuPosition) {

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

static void AdvancedOptions(void)
{
	char *MenuTexts[100];
	int proceed_now = FALSE;
	int MenuPosition = 1;
	int i;

	enum {
		RUN_MAP_VALIDATION = 1,
//		RUN_LUA_VALIDATION,
		LEAVE_OPTIONS_MENU,
	};

	game_status = INSIDE_MENU;

	while (!proceed_now) {

		InitiateMenu("--EDITOR_BACKGROUND--");

		i = 0;
		MenuTexts[i++] = _("Run Map Level Validator");
//		MenuTexts[i++] = _("Run Dialog Lua Validator");
		MenuTexts[i++] = _("Back");
		MenuTexts[i++] = "";

		while (EscapePressed())
			SDL_Delay(1);

		MenuPosition = DoMenuSelection("", MenuTexts, -1, "--EDITOR_BACKGROUND--", FPS_Display_BFont);

		while (EnterPressed() || SpacePressed() || MouseLeftPressed())
			SDL_Delay(1);

		switch (MenuPosition) {
		case (-1):
			while (EscapePressed()) ;
			proceed_now = !proceed_now;
			break;
		case RUN_MAP_VALIDATION:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			level_validation();
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
};				// void AdvancedOptions ( void );

/**
 *
 *
 */
int DoLevelEditorMainMenu()
{
	char *MenuTexts[100];
	char Options[20][1000];
	int proceed_now = FALSE;
	int MenuPosition = 1;
	int tgt = -1;
	int Done = FALSE;
	int i;

	game_status = INSIDE_MENU;

	//hack : eat pending events
	input_handle();

	enum {
		ENTER_LEVEL_POSITION = 1,
		LEVEL_OPTIONS_POSITION,
		ADVANCED_OPTIONS_POSITION,
		TEST_MAP_POSITION,
		SAVE_LEVEL_POSITION,
//              MANAGE_LEVEL_POSITION,
		ESCAPE_FROM_MENU_POSITION,
                SHOW_HELP,
		QUIT_TO_MAIN_POSITION,
		QUIT_POSITION,
	};

	while (!proceed_now) {

		InitiateMenu("--EDITOR_BACKGROUND--");

		i = 0;
		sprintf(Options[i], _("Level"));
		sprintf(Options[i + 1], ": %d - %s", EditLevel()->levelnum, EditLevel()->Levelname);
		strcat(Options[i], Options[i + 1]);
		MenuTexts[i] = Options[i];
		i++;
		MenuTexts[i++] = _("Level Options");
		MenuTexts[i++] = _("Advanced Options");
		if (game_root_mode == ROOT_IS_LVLEDIT) {
			MenuTexts[i++] = _("Playtest Mapfile");
			MenuTexts[i++] = _("Save Mapfile");
			//              MenuTexts[i++] = _("Manage Mapfiles");
		} else {
			MenuTexts[i++] = _("Return to Game");
			MenuTexts[i++] = _("Saving disabled");
			//              MenuTexts[i++] = " ";
		}
		MenuTexts[i++] = _("Continue Editing");
                MenuTexts[i++] = _("Show Help");
		MenuTexts[i++] = _("Quit to Main Menu");
		MenuTexts[i++] = _("Exit FreedroidRPG");
		MenuTexts[i++] = "";

		while (EscapePressed())
			SDL_Delay(1);

		MenuPosition = DoMenuSelection("", MenuTexts, -1, "--EDITOR_BACKGROUND--", FPS_Display_BFont);

		while (EnterPressed() || SpacePressed() || MouseLeftPressed())
			SDL_Delay(1);

		switch (MenuPosition) {

		case (-1):
			while (EscapePressed()) ;
			proceed_now = !proceed_now;
			break;
		case ESCAPE_FROM_MENU_POSITION:
			proceed_now = !proceed_now;
			break;
		case ENTER_LEVEL_POSITION:
			if (LeftPressed() || RightPressed()) {	//left or right arrow ? handled below 
				break;
			}
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);

			tgt = get_number_popup("\n Please enter new level number: \n\n", "");

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
			LevelOptions();
			break;
		case ADVANCED_OPTIONS_POSITION:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			AdvancedOptions();
			break;
		case TEST_MAP_POSITION:
			TestMap();
			proceed_now = !proceed_now;
			if (game_root_mode == ROOT_IS_GAME)
				Done = TRUE;
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
			PlayATitleFile("level_editor_help.title");
			break;
		case QUIT_TO_MAIN_POSITION:
			while (EnterPressed() || SpacePressed() || MouseLeftPressed())
				SDL_Delay(1);
			if (game_root_mode == ROOT_IS_GAME) {
				proceed_now = !proceed_now;
				GameOver = TRUE;
				Done = TRUE;
			}
			if (game_root_mode == ROOT_IS_LVLEDIT) {
				proceed_now = !proceed_now;
				Done = TRUE;
			}
			break;
		case QUIT_POSITION:
			DebugPrintf(2, "\nvoid EscapeMenu( void ): Quit Requested by user.  Terminating...");
			Terminate(EXIT_SUCCESS, TRUE);
			break;
		default:
			break;

		}		// switch

		if (LeftPressed() || RightPressed()) {
			switch (MenuPosition) {
			
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
	return (Done);
};
