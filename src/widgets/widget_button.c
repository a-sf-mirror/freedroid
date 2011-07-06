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

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_display.h"
#include "lvledit/lvledit_grass_actions.h"
#include "lvledit/lvledit_map.h"
#include "lvledit/lvledit_menu.h"
#include "lvledit/lvledit_tools.h"
#include "widgets.h"

static void activate_button(struct widget_button *b)
{
	int idx = b->btn_index;
	
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
			find_file("levels.dat", MAP_DIR, fp, 0);
			SaveShip(fp, TRUE, 0);

			alert_window("%s", _("M E S S A G E\n\nYour ship was saved to file 'levels.dat' in the map directory.\n\nIf you have set up something cool and you wish to contribute it to FreedroidRPG, please contact the FreedroidRPG dev team."));
		} else
			alert_window("%s", _("M E S S A G E\n\nE R R O R ! Your ship was not saved.\n\nPlaying on a map leaves the world in an unclean state not suitable for saving. Enter the editor from the main menu to be able to save."));
		break;
	case LEVEL_EDITOR_TOGGLE_WAYPOINT_CONNECTIONS_BUTTON:
		GameConfig.show_wp_connections = !GameConfig.show_wp_connections;
		break;
	case LEVEL_EDITOR_BEAUTIFY_GRASS_BUTTON:
		level_editor_beautify_grass_tiles(EditLevel());
		break;
	case LEVEL_EDITOR_DELETE_OBSTACLE_BUTTON:
		level_editor_cut_selection();
		break;
	case LEVEL_EDITOR_NEXT_OBJECT_BUTTON:
		level_editor_cycle_marked_object();
		break;
	case LEVEL_EDITOR_NEW_OBSTACLE_LABEL_BUTTON:
		if (single_tile_selection(OBJECT_OBSTACLE)) {
			action_change_obstacle_label_user(EditLevel(), single_tile_selection(OBJECT_OBSTACLE));
		}
		break;
	case LEVEL_EDITOR_NEW_MAP_LABEL_BUTTON:
		level_editor_action_change_map_label_user(EditLevel());
		break;
	case LEVEL_EDITOR_EDIT_CHEST_BUTTON:
		level_editor_edit_chest(single_tile_selection(OBJECT_OBSTACLE));
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
		GameConfig.show_grid = (GameConfig.show_grid + 1) % 3;
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
	case RIGHT_LEVEL_EDITOR_BUTTON:
		widget_lvledit_toolbar_scroll_right();
		break;
	case LEFT_LEVEL_EDITOR_BUTTON:
		widget_lvledit_toolbar_scroll_left();
		break;
	case LEVEL_EDITOR_TYPESELECT_OBSTACLE_BUTTON:
		lvledit_select_type(OBJECT_OBSTACLE);
		break;
	case LEVEL_EDITOR_TYPESELECT_FLOOR_BUTTON:
		lvledit_select_type(OBJECT_FLOOR);
		break;
	case LEVEL_EDITOR_TYPESELECT_ITEM_BUTTON:
		lvledit_select_type(OBJECT_ITEM);
		break;
	case LEVEL_EDITOR_TYPESELECT_WAYPOINT_BUTTON:
		lvledit_select_type(OBJECT_WAYPOINT);
		break;
	case LEVEL_EDITOR_TYPESELECT_MAP_LABEL_BUTTON:
		lvledit_select_type(OBJECT_MAP_LABEL);
		break;
	default:
		ErrorMessage(__FUNCTION__, "Button type %d unhandled.", PLEASE_INFORM, IS_WARNING_ONLY, idx);
	}
}

static void activate_button_secondary(struct widget_button *b)
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

/**
 * Generic handle event function for buttons.
 * 
 * Buttons have two primary states: normal and pressed.
 * Pressed state is switched on on mouse press event and switched off
 * on mouse release or mouse leave event.
 * 
 * Buttons have one primary and one secondary action. The primary action is
 * triggered on left mouse button release while the secondary action is 
 * triggered on right mouse button release. Both actions require
 * the button to be in pressed state when receiving the release event
 * to be triggered.
 */
static int button_handle_event(struct widget *w, SDL_Event *event)
{
	struct widget_button *b = w->ext;
	switch (event->type) {
		case SDL_MOUSEBUTTONDOWN:
			// Button has been pressed.
			b->pressed = 1;
			return 1;

		case SDL_MOUSEBUTTONUP:
			// Trigger the button if it is currently pressed.
			if (b->pressed) {
				b->pressed = 0;
				if (event->button.button == MOUSE_BUTTON_1)
					activate_button(b);
				if (event->button.button == MOUSE_BUTTON_3)
					activate_button_secondary(b);
			}
			break;

		case SDL_USEREVENT:
			// Mouse left the button's rectangle, switch to normal state.
			if (event->user.code == EVENT_MOUSE_LEAVE) {
				b->pressed = 0;
			}
			if (event->user.code == EVENT_UPDATE && w->update)
				w->update(w);
			break;

		default:
			break;
	}
	return 0;
}

static void button_display(struct widget *vb)
{
	struct widget_button *b = vb->ext;

	int pushoffset = b->pressed ? 1 : 0;
	int activeoffset = b->active;

	ShowGenericButtonFromList(b->btn_index + pushoffset + activeoffset);

	if (b->text) {
		SDL_Rect btn_rect = AllMousePressButtons[b->btn_index].button_rect;

		// Calculate the position of the text, we want to display the text at the
		// center of the button
		SDL_Rect rect;
		rect.x = btn_rect.x + (btn_rect.w - TextWidth(b->text)) / 2;
		rect.y = btn_rect.y + 8;
		rect.w = btn_rect.w;
		rect.h = btn_rect.h;

		// Draw the text on the button
		display_text(b->text, rect.x, rect.y, &rect);
	}
}

/**
 * @brief Creates a widget_button.
 * This function creates a widget_button using the default callbacks.
 * @return A pointer to the newly created widget_button.
 */
struct widget *widget_button_create(int btype, char *text, char *tooltip)
{
	struct widget *a = MyMalloc(sizeof(struct widget));
	a->type = WIDGET_BUTTON;
	a->display = button_display;
	a->rect = AllMousePressButtons[btype].button_rect;
	a->handle_event = button_handle_event;
	a->enabled = 1;

	struct widget_button *b = MyMalloc(sizeof(struct widget_button));
	b->btn_index = btype;
	b->pressed = 0;
	b->text = text;
	b->tooltip = tooltip;

	a->ext = b;

	return a;
}
