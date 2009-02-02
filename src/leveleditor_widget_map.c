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

#include "leveleditor.h"
#include "leveleditor_actions.h"
#include "leveleditor_widgets.h"
#include "leveleditor_tools.h"

static struct leveleditor_tool *selected_tool = NULL;
static struct leveleditor_tool *active_tool = NULL;

moderately_finepoint mouse_mapcoord;

static struct leveleditor_widget *tool_selection_menu;

static void select_tool(void *t) 
{
    if (!active_tool)
	selected_tool = t;
    else 
	GiveMouseAlertWindow("Cannot select another tool: busy\n");

    if (tool_selection_menu)
	tool_selection_menu->enabled = 0;
}

static void select_other_tool(int whichway)
{
    struct list_head *a = whichway == 1 ? selected_tool->node.next : selected_tool->node.prev;

    selected_tool = list_entry(a, struct leveleditor_tool, node);

    if (selected_tool->type == TOOL_NONE) {
	//the sentinel, skip it
	a = whichway == 1 ? selected_tool->node.next : selected_tool->node.prev;
	selected_tool = list_entry(a, struct leveleditor_tool, node);
    }

    switch (selected_tool->type) {
	case TOOL_MOVE:
	    printf("Current tool is MOVE\n");
	    break;
	case TOOL_PLACE:
	    printf("Current tool is PLACE\n");
	    break;
	case TOOL_SELECT:
	    printf("Current tool is SELECT\n");
	    break;
    }

}

void leveleditor_map_init()
{
    leveleditor_init_tools();
    selected_tool = tool_place;
}

static void forward_event(SDL_Event *event)
{
    if (active_tool) {
	if (active_tool->input_event(event, active_tool->ext)) {
	    active_tool = NULL;
	}
    }
}

void leveleditor_map_mouseenter(SDL_Event *event, struct leveleditor_widget *vm)
{
    (void)vm;
}

void leveleditor_map_mouseleave(SDL_Event *event, struct leveleditor_widget *vm)
{
    (void)vm;

    /* XXX do we disable the current tool when we lose focus ? */
}

void leveleditor_map_mouserelease(SDL_Event *event, struct leveleditor_widget *vm)
{
    (void)vm;
    
    forward_event(event);
}

void leveleditor_map_mousepress(SDL_Event *event, struct leveleditor_widget *vm)
{
    (void)vm;
    if (!active_tool)
	active_tool = selected_tool;

    forward_event(event);
}

void leveleditor_map_mouserightrelease(SDL_Event *event, struct leveleditor_widget *vm)
{
    (void)vm;
    
    forward_event(event);
}

void leveleditor_map_mouserightpress(SDL_Event *event, struct leveleditor_widget *vm)
{
    (void)vm;

    if (!active_tool)
	active_tool = tool_move;

    forward_event(event);
}

void leveleditor_map_mousewheelup(SDL_Event *event, struct leveleditor_widget *vm)
{
    (void)vm;
    //select_other_tool(-1);
}

void leveleditor_map_mousewheeldown(SDL_Event *event, struct leveleditor_widget *vm)
{
    (void)vm;
    //select_other_tool(1);
}

void leveleditor_map_mousemove(SDL_Event *event, struct leveleditor_widget *vm) 
{
    (void)vm;
    mouse_mapcoord =  translate_point_to_map_location ((float)event->motion.x -(GameConfig.screen_width/2), (float)event->motion.y - (GameConfig.screen_height/2),
 GameConfig.zoom_is_on);

    forward_event(event);
}

int leveleditor_map_keybevent(SDL_Event *event, struct leveleditor_widget *vm) 
{
    (void)vm;

    // Tool selection menu via space
    if (EVENT_KEYPRESS(event, SDLK_SPACE) && !active_tool) {
	// No active tool? Spawn a menu

	if (!tool_selection_menu) { 
	    //We do not have a menu
	    tool_selection_menu = create_menu();
	    list_add(&tool_selection_menu->node, &leveleditor_widget_list);
	} 

	if (!tool_selection_menu->enabled) {

	    struct leveleditor_menu *m = tool_selection_menu->ext;
	    int i;
	    SDL_Rect r = { .x = GetMousePos_x(), .y = GetMousePos_y(), .w = 100, .h = 150 };
	    tool_selection_menu->rect = r;
	    sprintf(m->text[0], "Cancel");
	    sprintf(m->text[1], "Place");
	    sprintf(m->text[2], "Move");
	    sprintf(m->text[3], "Select");
	    for (i = 4; i < 10; i++)
		m->text[i][0] = 0;
	    m->values[0] = selected_tool;
	    m->values[1] = tool_place;
	    m->values[2] = tool_move;
	    m->values[3] = tool_select;

	    m->done_cb = select_tool;

	    tool_selection_menu->enabled = 1;
	} else {
	    //Disable the existing menu
	    tool_selection_menu->enabled = 0;
	}
    }

    // Forward the key to the active tool
    forward_event(event);

    return 0;
}

void leveleditor_map_display(struct leveleditor_widget *vm)
{
    (void)vm;

    if (active_tool)
	active_tool->display(active_tool->ext);
}

void leveleditor_update_tool()
{ //time-based updating if relevant
    if (active_tool)
	active_tool->input_event(NULL, active_tool->ext);
}
