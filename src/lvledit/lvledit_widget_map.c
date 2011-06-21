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

#define leveleditor_widget_map_c
#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "widgets/widgets.h"
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

static void widget_lvledit_map_mouseenter(SDL_Event *event, struct widget *vm)
{
	(void)vm;
}

static void widget_lvledit_map_mouseleave(SDL_Event *event, struct widget *vm)
{
	(void)vm;
}

static void widget_lvledit_map_mouserelease(SDL_Event *event, struct widget *vm)
{
	(void)vm;

	forward_event(event);
}

static void widget_lvledit_map_mousepress(SDL_Event *event, struct widget *vm)
{
	(void)vm;
	if (!active_tool && mouse_in_level) {
		active_tool = selected_tool;

		/* disable temporary-switching by CTRL because it conflicts with modifiers used in select tool
		   if (CtrlPressed()) {
		   active_tool = (selected_tool == &tool_place ? &tool_select : &tool_place);
		   }
		 */
	}

	forward_event(event);
}

static void widget_lvledit_map_mouserightrelease(SDL_Event *event, struct widget *vm)
{
	(void)vm;

	forward_event(event);
}

static void widget_lvledit_map_mouserightpress(SDL_Event *event, struct widget *vm)
{
	(void)vm;

	if (!active_tool)
		active_tool = &tool_move;

	forward_event(event);
}

static void widget_lvledit_map_mousewheelup(SDL_Event *event, struct widget *vm)
{
	(void)vm;
	widget_lvledit_toolbar_left();
}

static void widget_lvledit_map_mousewheeldown(SDL_Event *event, struct widget *vm)
{
	(void)vm;
	widget_lvledit_toolbar_right();
}

static void widget_lvledit_map_mousemove(SDL_Event *event, struct widget *vm)
{
	(void)vm;
	mouse_mapcoord =
	    translate_point_to_map_location((float)event->motion.x - (GameConfig.screen_width / 2),
					    (float)event->motion.y - (GameConfig.screen_height / 2), GameConfig.zoom_is_on);

	if (mouse_mapcoord.x < 0 || mouse_mapcoord.x >= EditLevel()->xlen ||
			mouse_mapcoord.y < 0 || mouse_mapcoord.y >= EditLevel()->ylen)
		mouse_in_level = 0;
	else 
		mouse_in_level = 1;

	forward_event(event);
}

int widget_lvledit_map_keybevent(SDL_Event *event, struct widget *vm)
{
	(void)vm;

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

void widget_lvledit_map_display_cursor()
{
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
	a->type = WIDGET_MAP;
	a->rect.x = 0;
	a->rect.y = 68;
	a->rect.w = GameConfig.screen_width;
	a->rect.h = GameConfig.screen_height - 68;
	a->mouseenter = widget_lvledit_map_mouseenter;
	a->mouseleave = widget_lvledit_map_mouseleave;
	a->mouserelease = widget_lvledit_map_mouserelease;
	a->mousepress = widget_lvledit_map_mousepress;
	a->mouserightrelease = widget_lvledit_map_mouserightrelease;
	a->mouserightpress = widget_lvledit_map_mouserightpress;
	a->mousewheelup = widget_lvledit_map_mousewheelup;
	a->mousewheeldown = widget_lvledit_map_mousewheeldown;
	a->mousemove = widget_lvledit_map_mousemove;
	a->keybevent = widget_lvledit_map_keybevent;
	a->enabled = 1;

	struct widget_lvledit_map *m = MyMalloc(sizeof(struct widget_lvledit_map));
	a->ext = m;

	return a;
}

void widget_lvledit_map_display(struct widget *vm)
{
	(void)vm;
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
}
