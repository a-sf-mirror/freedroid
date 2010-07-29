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

#include "SDL_rotozoom.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_widgets.h"
#include "lvledit/lvledit_tools.h"

static struct leveleditor_tool *selected_tool = NULL;
static struct leveleditor_tool *active_tool = NULL;

moderately_finepoint mouse_mapcoord;
int mouse_in_level; // is the mouse cursor at an existing position on the level?

void leveleditor_map_init()
{
	leveleditor_init_tools();
	selected_tool = &tool_select;
}

static void forward_event(SDL_Event * event)
{
	if (active_tool) {
		if (active_tool->input_event(event)) {
			active_tool = NULL;
		}
	}
}

void leveleditor_map_mouseenter(SDL_Event * event, struct leveleditor_widget *vm)
{
	(void)vm;
}

void leveleditor_map_mouseleave(SDL_Event * event, struct leveleditor_widget *vm)
{
	(void)vm;
}

void leveleditor_map_mouserelease(SDL_Event * event, struct leveleditor_widget *vm)
{
	(void)vm;

	forward_event(event);
}

void leveleditor_map_mousepress(SDL_Event * event, struct leveleditor_widget *vm)
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

void leveleditor_map_mouserightrelease(SDL_Event * event, struct leveleditor_widget *vm)
{
	(void)vm;

	forward_event(event);
}

void leveleditor_map_mouserightpress(SDL_Event * event, struct leveleditor_widget *vm)
{
	(void)vm;

	if (!active_tool)
		active_tool = &tool_move;

	forward_event(event);
}

void leveleditor_map_mousewheelup(SDL_Event * event, struct leveleditor_widget *vm)
{
	(void)vm;
	leveleditor_toolbar_left();
}

void leveleditor_map_mousewheeldown(SDL_Event * event, struct leveleditor_widget *vm)
{
	(void)vm;
	leveleditor_toolbar_right();
}

void leveleditor_map_mousemove(SDL_Event * event, struct leveleditor_widget *vm)
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

int leveleditor_map_keybevent(SDL_Event * event, struct leveleditor_widget *vm)
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

void leveleditor_map_display_cursor()
{
	int oldmode = global_ingame_mode;
	static int dragging = FALSE;

	if (active_tool)
		active_tool->display();

	if (selected_tool == &tool_select) {
		if ((ShiftPressed() || dragging == TRUE) && MouseLeftPressed()
		    && !selection_empty()) {
			//dragdrop
			global_ingame_mode = GLOBAL_INGAME_MODE_DRAGDROP_TOOL;
			dragging = TRUE;
		} else {
			global_ingame_mode = GLOBAL_INGAME_MODE_SELECT_TOOL;
			dragging = FALSE;
		}
	}

	blit_our_own_mouse_cursor();

	global_ingame_mode = oldmode;
}

void leveleditor_map_display(struct leveleditor_widget *vm)
{
	(void)vm;
}

void leveleditor_update_tool()
{				//time-based updating if relevant
	forward_event(NULL);
}

void leveleditor_reset_tools()
{
	leveleditor_place_reset();

	active_tool = NULL;
}
