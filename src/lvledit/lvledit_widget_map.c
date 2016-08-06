/* 
 *
 *   Copyright (c) 2009 Arthur Huillet
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

#define leveleditor_widget_map_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_widgets.h"
#include "lvledit/lvledit_tools.h"

static struct leveleditor_tool *selected_tool = NULL;
static struct leveleditor_tool *active_tool = NULL;

moderately_finepoint mouse_mapcoord;
int mouse_in_level; // is the mouse cursor at an existing position on the level?

void widget_lvledit_map_init()
{
	leveleditor_init_tools();
	selected_tool = &tool_select;
}

static void forward_event(SDL_Event *event)
{
	if (active_tool) {
		if (active_tool->input_event(event)) {
			active_tool = NULL;
		}
	}
}

static void map_mousemove(struct widget *vm, SDL_Event *event)
{
	mouse_mapcoord = translate_point_to_map_location((float)event->motion.x - (GameConfig.screen_width / 2), (float)event->motion.y - (GameConfig.screen_height / 2), GameConfig.zoom_is_on);

	if (mouse_mapcoord.x < 0 || mouse_mapcoord.x >= EditLevel()->xlen ||
			mouse_mapcoord.y < 0 || mouse_mapcoord.y >= EditLevel()->ylen)
		mouse_in_level = 0;
	else 
		mouse_in_level = 1;

	forward_event(event);
}

static int map_keybevent(struct widget *vm, SDL_Event *event)
{
	// Tool selection menu via space
	if (EVENT_KEYPRESS(event, SDLK_SPACE) && !active_tool) {
		// No active tool? Cycle the currently selected one.
		//
		selected_tool = (selected_tool == &tool_place ? &tool_select : &tool_place);
	}
	// Forward the key to the active tool
	forward_event(event);

	return 0;
}

static int map_handle_event(struct widget *w, SDL_Event *event)
{
	switch (event->type) {
		case SDL_MOUSEBUTTONDOWN:
			switch (event->button.button) {
				case MOUSE_BUTTON_1:
					if (!active_tool && mouse_in_level)
						active_tool = selected_tool;
					forward_event(event);
					return 1;

				case MOUSE_BUTTON_3:
					if (!active_tool)
					active_tool = &tool_move;
					forward_event(event);
					break;

				case SDL_BUTTON_WHEELUP:
					widget_lvledit_toolbar_left();
					return 1;

				case SDL_BUTTON_WHEELDOWN:
					widget_lvledit_toolbar_right();
					return 1;

				default:
					return 0;
			}
			return 1;

		case SDL_MOUSEBUTTONUP:
			forward_event(event);
			return 1;

		case SDL_MOUSEMOTION:
			map_mousemove(w, event);
			return 1;

		case SDL_KEYDOWN:
			map_keybevent(w, event);
			return 1;

		default:
			break;
	}
	return 0;
}

void widget_lvledit_map_display_cursor()
{
	// cppcheck-suppress variableScope
	static int dragging = FALSE;

	if (active_tool)
		active_tool->display();

	if (selected_tool == &tool_select) {
		if ((ShiftPressed() || dragging == TRUE) && MouseLeftPressed()
		    && !selection_empty()) {
			//dragdrop
			mouse_cursor = MOUSE_CURSOR_DRAGDROP_TOOL;
			dragging = TRUE;
		} else {
			mouse_cursor = MOUSE_CURSOR_SELECT_TOOL;
			dragging = FALSE;
		}
	}

	blit_mouse_cursor();
}

struct widget *widget_lvledit_map_create()
{
	struct widget *a = MyMalloc(sizeof(struct widget));
	widget_init(a);
	a->type = WIDGET_MAP;
	widget_set_rect(a, 0, 68, GameConfig.screen_width, GameConfig.screen_height -68);
	a->handle_event = map_handle_event;

	struct widget_lvledit_map *m = MyMalloc(sizeof(struct widget_lvledit_map));
	a->ext = m;

	return a;
}

void leveleditor_update_tool()
{				//time-based updating if relevant
	forward_event(NULL);
}

void lvledit_reset_tools()
{
	leveleditor_place_reset();
	leveleditor_select_reset();

	active_tool = NULL;

	current_floor_layer = 0;
}
