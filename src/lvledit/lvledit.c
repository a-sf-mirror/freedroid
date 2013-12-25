/* 
 *
 *   Copyright (c) 2002, 2003 Johannes Prix
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

/**
 * This file contains all functions for the heart of the level editor.
 */

#define _leveleditor_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"


#include "lvledit/lvledit.h"
#include "lvledit/lvledit_validator.h"

#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_display.h"
#include "lvledit/lvledit_map.h"
#include "lvledit/lvledit_menu.h"
#include "lvledit/lvledit_tools.h"
#include "lvledit/lvledit_widgets.h"

int OriginWaypoint = (-1);

char VanishingMessage[10000] = "";
float VanishingMessageEndDate = 0;
int level_editor_done = FALSE;

/**
 * Return the X coordinate of the block we are on.
 */
int EditX(void)
{
	int BlockX = rintf(Me.pos.x - 0.5);
	if (BlockX < 0) {
		BlockX = 0;
		Me.pos.x = 0.51;
	}
	return BlockX;
}

/**
 * Return the Y coordinate of the block we are on.
 */
int EditY(void)
{
	int BlockY = rintf(Me.pos.y - 0.5);
	if (BlockY < 0) {
		BlockY = 0;
		Me.pos.y = 0.51;
	}
	return BlockY;
}

/**
 * Return a pointer to the level we are currently editing.
 */
level *EditLevel(void)
{
	return CURLEVEL();
}

/**
 * This function should associate the current mouse position with an
 * index in the level editor item drop screen.
 * (-1) is returned when cursor is not on any item in the item drop grid.
 */
int level_editor_item_drop_index(int row_len, int line_len)
{
	if ((GetMousePos_x() > UNIVERSAL_COORD_W(55)) && (GetMousePos_x() < UNIVERSAL_COORD_W(55 + 64 * line_len)) &&
	    (GetMousePos_y() > UNIVERSAL_COORD_H(32)) && (GetMousePos_y() < UNIVERSAL_COORD_H(32 + 66 * row_len))) {
		return ((GetMousePos_x() - UNIVERSAL_COORD_W(55)) / (64 * GameConfig.screen_width / 640) +
			((GetMousePos_y() - UNIVERSAL_COORD_H(32)) / (66 * GameConfig.screen_height / 480)) * line_len);
	}
	// If no level editor item grid index was found under the current
	// mouse cursor position, we just return (-1) to indicate that.
	//
	return (-1);

};				// int level_editor_item_drop_index ( void )

/**
 * This function drops an item onto the floor.  It works with a selection
 * of item images and clicking with the mouse on an item image or on one
 * of the buttons presented to the person editing the level.
 */
item *ItemDropFromLevelEditor(void)
{
#define SKIP_CTD_ITEM 1
	int SelectionDone = FALSE;
	int NewItemCode = (-1);
	int i;
	int j;
	item temp_item;
	int row_len = 5;
	int line_len = 8;
	int item_group = 0;
	item *dropped_item;
	static int previous_mouse_position_index = (-1);
	game_status = INSIDE_MENU;

	while (MouseLeftPressed())
		SDL_Delay(1);

	while (!SelectionDone) {
		SDL_Event event;
		clear_screen();

		for (j = 0; j < row_len; j++) {
			for (i = 0; i < line_len; i++) {
				init_item(&temp_item);
				temp_item.type = i + j * line_len + item_group * line_len * row_len + SKIP_CTD_ITEM;
				if (temp_item.type >= Number_Of_Item_Types)
					continue;	//temp_item.type = 1 ;
				ShowRescaledItem(i, 32 + (64 * GameConfig.screen_height / 480 + 2) * j, &(temp_item));
				DeleteItem(&temp_item);
			}
		}

		ShowGenericButtonFromList(LEVEL_EDITOR_NEXT_ITEM_GROUP_BUTTON);
		ShowGenericButtonFromList(LEVEL_EDITOR_PREV_ITEM_GROUP_BUTTON);
		ShowGenericButtonFromList(LEVEL_EDITOR_CANCEL_ITEM_DROP_BUTTON);

		if (MouseCursorIsOnButton(LEVEL_EDITOR_CANCEL_ITEM_DROP_BUTTON, GetMousePos_x(), GetMousePos_y()))
			put_string(FPS_Display_BFont, 20, 440 * GameConfig.screen_height / 480, _("Cancel item drop"));
		if (level_editor_item_drop_index(row_len, line_len) != (-1)) {
			previous_mouse_position_index = level_editor_item_drop_index(row_len, line_len) + item_group * line_len * row_len + SKIP_CTD_ITEM;
			if (previous_mouse_position_index >= Number_Of_Item_Types) {
				previous_mouse_position_index = Number_Of_Item_Types - 1;
			} else
				put_string(FPS_Display_BFont, 20, 440 * GameConfig.screen_height / 480,
					      item_specs_get_name(previous_mouse_position_index));
		}

		blit_mouse_cursor();
		our_SDL_flip_wrapper();

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					return NULL;
				}
			} else if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (MouseCursorIsOnButton(LEVEL_EDITOR_NEXT_ITEM_GROUP_BUTTON, event.button.x, event.button.y)) {
					if ((item_group + 1) * line_len * row_len < Number_Of_Item_Types)
						item_group++;
				} else if (MouseCursorIsOnButton(LEVEL_EDITOR_PREV_ITEM_GROUP_BUTTON, event.button.x, event.button.y)) {
					if (item_group > 0)
						item_group--;
				}

				if (MouseCursorIsOnButton(LEVEL_EDITOR_CANCEL_ITEM_DROP_BUTTON, event.button.x, event.button.y)) {
					return NULL;
				} else if (level_editor_item_drop_index(row_len, line_len) != (-1)) {
					NewItemCode = level_editor_item_drop_index(row_len, line_len) + item_group * line_len * row_len + SKIP_CTD_ITEM;
					if (NewItemCode < 0)
						NewItemCode = 0;	// just if the mouse has moved away in that little time...
					if (NewItemCode < Number_Of_Item_Types)
						SelectionDone = TRUE;
				}
			}
		}
	}

	if (NewItemCode >= Number_Of_Item_Types) {
		NewItemCode = 0;
	}

	// Create an item on the map
	dropped_item = action_create_item(EditLevel(), rintf(Me.pos.x), rintf(Me.pos.y), NewItemCode);

	game_status = INSIDE_LVLEDITOR;

	return dropped_item;
};				// void ItemDropFromLevelEditor( void )


static void leveleditor_init()
{
	level_editor_done = FALSE;

	// We set the Tux position to something 'round'.
	//
	Me.pos.x = rintf(Me.pos.x) + 0.5;
	Me.pos.y = rintf(Me.pos.y) + 0.5;

	// We disable all the 'screens' so that we have full view on the
	// map for the purpose of level editing.
	//
	GameConfig.Inventory_Visible = FALSE;
	GameConfig.CharacterScreen_Visible = FALSE;
	GameConfig.SkillScreen_Visible = FALSE;

	strcpy(VanishingMessage, "");
	VanishingMessageEndDate = 0;

	// Reset tooltips.
	widget_set_tooltip(NULL, NULL);

	// For drawing new waypoints, we init this.
	//
	OriginWaypoint = (-1);

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
}

static void leveleditor_cleanup()
{
	Activate_Conservative_Frame_Computation();
	action_freestack();
	clear_selection(-1);

	SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
	Me.mouse_move_target = Me.pos;
}

void TestMap(void)
{				/* Keeps World map in a clean state */
	leveleditor_cleanup();	//free current selection and undo stack as a workaround for existing problems
	if (game_root_mode == ROOT_IS_GAME)	/*don't allow map testing if root mode is GAME */
		return;
	game_status = INSIDE_GAME;
	SaveGame();
	Game();
	LoadGame();
	leveleditor_init();
	return;
}				// TestMap ( void )

/**
 * This function provides the leveleditor integrated into
 * FreedroidRPG.  Actually this function is a submenu of the big
 * Escape Menu.  In here you can edit the level and, upon pressing
 * escape, you can enter a new submenu where you can save the level,
 * change level name and quit from level editing.
 */
void LevelEditor()
{
	leveleditor_init();

	reset_visible_levels();
	get_visible_levels();
	animation_timeline_reset();

	while (!level_editor_done) {
		game_status = INSIDE_LVLEDITOR;

		StartTakingTimeForFPSCalculation();
		update_frames_displayed();

		if (GameConfig.limit_framerate)
			SDL_Delay(10);

		leveleditor_process_input();
		update_widgets();

		struct widget *w = get_active_widget(GetMousePos_x(), GetMousePos_y());
		if (w && w->type == WIDGET_MAP) {
			leveleditor_update_tool();
		}

		leveleditor_display();

		ComputeFPSForThisFrame();
	}

	leveleditor_cleanup();
};				// void LevelEditor ( void )

#undef _leveleditor_c
