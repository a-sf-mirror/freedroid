

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

#define leveleditor_tool_select_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "SDL_rotozoom.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"

#include "lvledit/lvledit_tools.h"
#include "lvledit/lvledit_widgets.h"

static struct leveleditor_select {
    point rect_start;
    point rect_len;
    point rect_step;

    moderately_finepoint drag_start;
    moderately_finepoint cur_drag_pos;
} state;

enum { DISABLED, RECT, RECTDONE, DRAGDROP } mode; 

struct selected_element {
    enum leveleditor_object_type type;
    struct list_head node;
    void *data;
};

static LIST_HEAD(selected_elements);

int element_in_selection(void *data)
{
    struct selected_element *e;
    list_for_each_entry(e, &selected_elements, node) {
	if (e->data == data)
	    return 1;
    }

    return 0;
}

static void add_obstacle_to_selection(obstacle * a)
{
    struct selected_element *e = MyMalloc(sizeof(struct selected_element));
    e->type = OBJECT_OBSTACLE;
    e->data = a;

    list_add(&e->node, &selected_elements);
}

static void clear_selection()
{
    struct selected_element *e, *ne;
    list_for_each_entry_safe(e, ne, &selected_elements, node) {
	list_del(&e->node);
	free(e);
    }
}

static void start_rect_select()
{
    mode = RECT;

    state.rect_start.x = mouse_mapcoord.x;
    state.rect_start.y = mouse_mapcoord.y;
    state.rect_len.x = 1;
    state.rect_len.y = 1;
}

static void do_rect_select()
{
    int i, j, a;
    // If there is something to change
    if (calc_euklid_distance(mouse_mapcoord.x, mouse_mapcoord.y,
		state.rect_start.x + state.rect_len.x,
		state.rect_start.y + state.rect_len.y) > 0.5) {
	// Redefine the rectangle dimensions
	float xlen = mouse_mapcoord.x - state.rect_start.x; 
	float ylen = mouse_mapcoord.y - state.rect_start.y; 

	if (xlen < 0) {
	    xlen -= 0.99;
	    if (xlen > -1)
		xlen = -1;
	} else {
	    xlen += 0.99;
	    if (xlen < 1)
		xlen = 1;
	}
	if (ylen < 0) {
	    ylen -= 0.99;
	    if (ylen > -1)
		ylen = -1;
	} else {
	    ylen += 0.99;
	    if (ylen < 1)
		ylen = 1;
	}

	state.rect_step.x = (xlen > 0 ? 1 : -1);
	state.rect_step.y = (ylen > 0 ? 1 : -1);
	state.rect_len.x = xlen;
	state.rect_len.y = ylen;

	// Undo previous rectangle
	clear_selection();

	// Then redo a correct one
	for (i = state.rect_start.x; i != state.rect_start.x + state.rect_len.x + state.rect_step.x;
		i += state.rect_step.x) {
	    for (j = state.rect_start.y;
		    j != state.rect_start.y + state.rect_len.y + state.rect_step.y;
		    j += state.rect_step.y) {
		int idx;
		for (a = 0; a < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE && EditLevel()->map[j][i].obstacles_glued_to_here[a] != -1; a++) {
		    idx = EditLevel()->map[j][i].obstacles_glued_to_here[a];
		    add_obstacle_to_selection(&EditLevel()->obstacle_list[idx]);
		}
	    }
	}
    }
}

static void end_rect_select()
{
    struct selected_element *e;
    mode = RECTDONE;
    list_for_each_entry(e, &selected_elements, node) {
    }
}

static void start_drag_drop()
{
    mode = DRAGDROP;
    state.drag_start.x = mouse_mapcoord.x;
    state.drag_start.y = mouse_mapcoord.y;
    state.cur_drag_pos.x = mouse_mapcoord.x;
    state.cur_drag_pos.y = mouse_mapcoord.y;
}

static void do_drag_drop()
{
    struct selected_element *e;
    list_for_each_entry(e, &selected_elements, node) {
	((obstacle *)(e->data))->pos.x += (mouse_mapcoord.x - state.cur_drag_pos.x);
	((obstacle *)(e->data))->pos.y += (mouse_mapcoord.y - state.cur_drag_pos.y);
    }
    state.cur_drag_pos.x = mouse_mapcoord.x;
    state.cur_drag_pos.y = mouse_mapcoord.y;
}

static void end_drag_drop()
{
    struct selected_element *e;
    int enb = 0;

    list_for_each_entry(e, &selected_elements, node) {
	double x = (((obstacle *)(e->data))->pos.x - state.cur_drag_pos.x + state.drag_start.x);
	double y = (((obstacle *)(e->data))->pos.y - state.cur_drag_pos.y + state.drag_start.y);
	action_push(ACT_MOVE_OBSTACLE, (obstacle *)e->data, x, y);
	enb ++;
    }
    
    glue_obstacles_to_floor_tiles_for_level(EditLevel()->levelnum);

    action_push (ACT_MULTIPLE_ACTIONS, enb);

    mode = RECTDONE; 
}

static void copy()
{
}

static void cut()
{
}

static void paste()
{
}


/*
 * LMB : rect selection
 * then:
 * 	RMB : discard and quit
 * 	LMB : start drag and drop
 * 	X : cut
 * 	C : copy
 */

int leveleditor_select_input(SDL_Event *event)
{
    switch (mode) {
	case DISABLED:
	    if (EVENT_LEFT_PRESS(event)) {
		start_rect_select();
		return 0;
	    }
	    break;
	case RECT:
	    if (EVENT_LEFT_RELEASE(event)) {
		end_rect_select();
		return 0;
	    } else if (EVENT_MOVE(event)) {
		do_rect_select();
		return 0;
	    }
	    break;
	case RECTDONE:
	    if (EVENT_RIGHT_PRESS(event)) {
		clear_selection();
		mode = DISABLED;
		return 1;
	    } else if (EVENT_LEFT_PRESS(event)) {
		start_drag_drop();
		return 0;
	    } 
	    break;
	case DRAGDROP:
	    if (EVENT_LEFT_RELEASE(event)) {
		end_drag_drop();
		return 0;
	    } else if (EVENT_MOVE(event)) {
		do_drag_drop();
		return 0;
	    }
	    break;
    }

    return 0;
}

int leveleditor_select_display()
{
    int r1, r2, r3, r4, c1, c2, c3, c4 ;
    switch(mode) {
	case RECT:
	    //display the selection rectangle
	    //XXX clean that up...
	    translate_map_point_to_screen_pixel (state.rect_start.x , state.rect_start.y , &r1, &c1, 1.0);
	    translate_map_point_to_screen_pixel ( state.rect_start.x , state.rect_start.y + state.rect_len.y , &r2, &c2, 1.0);
	    translate_map_point_to_screen_pixel ( state.rect_start.x + state.rect_len.x , state.rect_start.y + state.rect_len.y , &r3, &c3, 1.0);
	    translate_map_point_to_screen_pixel ( state.rect_start.x + state.rect_len.x , state.rect_start.y , &r4, &c4, 1.0);

#ifdef HAVE_LIBGL
	    if (use_open_gl) {
		glDisable ( GL_TEXTURE_2D );
		glEnable ( GL_BLEND );

		glColor4ub( 0x1f , 0x7f , 0x8f , 0x8f);

		glBegin(GL_QUADS);
		glVertex2i(r1, c1);
		glVertex2i(r2, c2);
		glVertex2i(r3, c3);
		glVertex2i(r4, c4);
		glEnd();

		glEnable( GL_TEXTURE_2D );
		glDisable ( GL_BLEND );
	    }
#endif
	    break;
	default:
	    break;
    }
    return 0;
}
