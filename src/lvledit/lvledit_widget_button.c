/* 
 *
 *   Copyright (c) 2009-2010 Arthur Huillet
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

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "SDL_rotozoom.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_display.h"
#include "lvledit/lvledit_grass_actions.h"
#include "lvledit/lvledit_map.h"
#include "lvledit/lvledit_menu.h"
#include "lvledit/lvledit_tools.h"
#include "lvledit/lvledit_widgets.h"

static void activate_button(struct leveleditor_button *b)
{
	int idx = b->btn_index;
	int new_x, new_y;
	switch (idx) {
	case LEVEL_EDITOR_UNDO_BUTTON:
		level_editor_action_undo();
		break;
	case LEVEL_EDITOR_REDO_BUTTON:
		level_editor_action_redo();
		break;
	case LEVEL_EDITOR_SAVE_SHIP_BUTTON:
		if (game_root_mode == ROOT_IS_LVLEDIT) {	/*don't allow saving if root mode is GAME */
			char fp[2048];
			find_file("freedroid.levels", MAP_DIR, fp, 0);
			SaveShip(fp, TRUE, 0);

			alert_window(_("M E S S A G E\n\nYour ship was saved to file 'freedroid.levels' in the map directory.\n\nIf you have set up something cool and you wish to contribute it to FreedroidRPG, please contact the FreedroidRPG dev team."));
		} else
			alert_window(_("M E S S A G E\n\nE R R O R ! Your ship was not saved.\n\nPlaying on a map leaves the world in an unclean state not suitable for saving. Enter the editor from the main menu to be able to save."));
		break;
	case LEVEL_EDITOR_BEAUTIFY_GRASS_BUTTON:
		level_editor_beautify_grass_tiles(EditLevel());
		break;
	case LEVEL_EDITOR_DELETE_OBSTACLE_BUTTON:
		level_editor_cut_selection();
		break;
	case LEVEL_EDITOR_NEXT_OBSTACLE_BUTTON:
		level_editor_cycle_marked_obstacle();
		break;
	case LEVEL_EDITOR_NEW_OBSTACLE_LABEL_BUTTON:
		if (single_tile_selection(OBJECT_OBSTACLE)) {
			action_change_obstacle_label_user(EditLevel(), single_tile_selection(OBJECT_OBSTACLE), NULL);
		}
		break;
	case LEVEL_EDITOR_NEW_MAP_LABEL_BUTTON:
		level_editor_action_change_map_label_user(EditLevel());
		break;
	case LEVEL_EDITOR_EDIT_CHEST_BUTTON:
		level_editor_edit_chest(single_tile_selection(OBJECT_OBSTACLE));
		break;
	case LEVEL_EDITOR_ESC_BUTTON:
		level_editor_done = DoLevelEditorMainMenu();
		break;
	case LEVEL_EDITOR_LEVEL_RESIZE_BUTTON:
		EditLevelDimensions();
		break;
	case LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON:
		GameConfig.omit_enemies_in_level_editor = !GameConfig.omit_enemies_in_level_editor;
		break;
	case LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON:
		GameConfig.show_tooltips = !GameConfig.show_tooltips;
		break;
	case LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON:
		draw_collision_rectangles = !draw_collision_rectangles;
		break;
	case LEVEL_EDITOR_TOGGLE_GRID_BUTTON_OFF:
		draw_grid = (draw_grid + 1) % 3;
		break;
	case LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON:
		GameConfig.omit_obstacles_in_level_editor = !GameConfig.omit_obstacles_in_level_editor;
		break;
	case LEVEL_EDITOR_ZOOM_IN_BUTTON:
		GameConfig.zoom_is_on = !GameConfig.zoom_is_on;
		break;
	case LEVEL_EDITOR_QUIT_BUTTON:
		TestMap();
		if (game_root_mode == ROOT_IS_GAME)
			level_editor_done = TRUE;

		break;
	case GO_LEVEL_NORTH_BUTTON:
		if (EditLevel()->jump_target_north < 0)
			break;
		if (Me.pos.x < curShip.AllLevels[EditLevel()->jump_target_north]->xlen - 1)
			new_x = Me.pos.x;
		else
			new_x = 3;
		new_y = curShip.AllLevels[EditLevel()->jump_target_north]->ylen - 4;
		action_jump_to_level(EditLevel()->jump_target_north, new_x, new_y);

		break;
	case GO_LEVEL_SOUTH_BUTTON:
		if (EditLevel()->jump_target_south < 0)
			break;
		if (Me.pos.x < curShip.AllLevels[EditLevel()->jump_target_south]->xlen - 1)
			new_x = Me.pos.x;
		else
			new_x = 3;
		new_y = 4;
		action_jump_to_level(EditLevel()->jump_target_south, new_x, new_y);
		break;
	case GO_LEVEL_EAST_BUTTON:
		if (EditLevel()->jump_target_east < 0)
			break;
		new_x = 3;
		if (Me.pos.y < curShip.AllLevels[EditLevel()->jump_target_east]->ylen - 1)
			new_y = Me.pos.y;
		else
			new_y = 4;
		action_jump_to_level(EditLevel()->jump_target_east, new_x, new_y);
		break;
	case GO_LEVEL_WEST_BUTTON:
		if (EditLevel()->jump_target_west < 0)
			break;
		new_x = curShip.AllLevels[EditLevel()->jump_target_west]->xlen - 4;
		if (Me.pos.y < curShip.AllLevels[EditLevel()->jump_target_west]->ylen - 1)
			new_y = Me.pos.y;
		else
			new_y = 4;
		action_jump_to_level(EditLevel()->jump_target_west, new_x, new_y);
		break;
	case RIGHT_LEVEL_EDITOR_BUTTON:
		leveleditor_toolbar_scroll_right();
		break;
	case LEFT_LEVEL_EDITOR_BUTTON:
		leveleditor_toolbar_scroll_left();
		break;
	default:
		ErrorMessage(__FUNCTION__, "Button type %d unhandled.", PLEASE_INFORM, IS_WARNING_ONLY, idx);
	}
}

static void activate_button_secondary(struct leveleditor_button *b)
{
	int idx = b->btn_index;

	switch (idx) {
	case LEVEL_EDITOR_ZOOM_IN_BUTTON:
		{
			float zf = lvledit_zoomfact();

			zf += 3.0;

			if (zf > 9.0)
				zf = 3.0;

			if (!lvledit_set_zoomfact(zf)) {
				sprintf(VanishingMessage, _("Zoom factor set to %f."), zf);
			} else {
				sprintf(VanishingMessage, _("Could not change zoom factor."));
			}

			VanishingMessageEndDate = SDL_GetTicks() + 1000;
		}
		break;
	default:
		break;
	}
}

void leveleditor_button_mouseenter(SDL_Event * event, struct leveleditor_widget *vb)
{
	struct leveleditor_button *b = vb->ext;
	(void)b;
}

void leveleditor_button_mouseleave(SDL_Event * event, struct leveleditor_widget *vb)
{
	struct leveleditor_button *b = vb->ext;
	b->pressed = 0;
}

void leveleditor_button_mouserelease(SDL_Event * event, struct leveleditor_widget *vb)
{
	struct leveleditor_button *b = vb->ext;
	if (b->pressed) {
		/* Validate the click: ACTION */
		activate_button(b);
		b->pressed = 0;
	}
}

void leveleditor_button_mousepress(SDL_Event * event, struct leveleditor_widget *vb)
{
	struct leveleditor_button *b = vb->ext;

	b->pressed = 1;
}

void leveleditor_button_mouserightrelease(SDL_Event * event, struct leveleditor_widget *vb)
{
	struct leveleditor_button *b = vb->ext;
	if (b->pressed) {
		/* Validate the click: ACTION */
		activate_button_secondary(b);
		b->pressed = 0;
	}
}

void leveleditor_button_mouserightpress(SDL_Event * event, struct leveleditor_widget *vb)
{
	struct leveleditor_button *b = vb->ext;

	b->pressed = 1;
}

void leveleditor_button_mousewheelup(SDL_Event * event, struct leveleditor_widget *vb)
{
	struct leveleditor_button *b = vb->ext;
	(void)b;
	//do nothing;
}

void leveleditor_button_mousewheeldown(SDL_Event * event, struct leveleditor_widget *vb)
{
	struct leveleditor_button *b = vb->ext;
	(void)b;
	//do nothing;
}

void leveleditor_button_display(struct leveleditor_widget *vb)
{
	struct leveleditor_button *b = vb->ext;

	int pushoffset = b->pressed ? 1 : 0;
	int activeoffset = b->active;

	ShowGenericButtonFromList(b->btn_index + pushoffset + activeoffset);

}
