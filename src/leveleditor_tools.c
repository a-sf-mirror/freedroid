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

#define leveleditor_tools_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "SDL_rotozoom.h"

#include "leveleditor.h"
#include "leveleditor_actions.h"

#include "leveleditor_widget_map.h"

#include "leveleditor_tools.h"
#include "leveleditor_tool_move.h"
#include "leveleditor_tool_place.h"
#include "leveleditor_tool_select.h"

LIST_HEAD(leveleditor_tool_list);

static struct leveleditor_tool * create_move()
{
    struct leveleditor_tool *a = MyMalloc(sizeof(struct leveleditor_tool));
    a->type = TOOL_MOVE;
    a->input_event = leveleditor_move_input;
    a->display = leveleditor_move_display;

    struct leveleditor_move *m = MyMalloc(sizeof(struct leveleditor_move));
    a->ext = m; 

    return a;
}

static struct leveleditor_tool * create_place()
{
    struct leveleditor_tool *a = MyMalloc(sizeof(struct leveleditor_tool));
    a->type = TOOL_PLACE;
    a->input_event = leveleditor_place_input;
    a->display = leveleditor_place_display;

    struct leveleditor_place *m = MyMalloc(sizeof(struct leveleditor_place));
    a->ext = m; 

    return a;
}

static struct leveleditor_tool * create_select()
{
    struct leveleditor_tool *a = MyMalloc(sizeof(struct leveleditor_tool));
    a->type = TOOL_SELECT;
    a->input_event = leveleditor_select_input;
    a->display = leveleditor_select_display;

    struct leveleditor_select *m = MyMalloc(sizeof(struct leveleditor_select));
    a->ext = m; 

    return a;
}

void leveleditor_init_tools()
{
    struct leveleditor_tool *place, *move;
    if (!list_empty(&leveleditor_tool_list)) {
	printf("Tools already initialized\n");
	return;
    }

    move = create_move();
    place = create_place();
    list_add(&move->node, &leveleditor_tool_list);
    list_add(&place->node, &leveleditor_tool_list);
    list_add(&create_select()->node, &leveleditor_tool_list);
}

