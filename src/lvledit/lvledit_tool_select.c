

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
    int rect_nbelem_selected;

    int single_tile_mark_index;

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
static LIST_HEAD(clipboard_elements);

/**
 * Check whether the selection is currently empty.
 */
int selection_empty()
{
    if (list_empty(&selected_elements))
	return 1;
    return 0;
}

/**
 * Check if there is exactly one element selected.
 * Returns this element, or NULL.
 */
void * single_tile_selection()
{
    if (list_empty(&selected_elements))
	return NULL;
    if (selected_elements.next->next != &selected_elements)
	return NULL;

    return list_entry(selected_elements.next, struct selected_element, node)->data;
}

/**
 * Check whether the given element is among the selected objects.
 */
int element_in_selection(void *data)
{
    struct selected_element *e;
    list_for_each_entry(e, &selected_elements, node) {
	if (e->data == data)
	    return 1;
    }

    return 0;
}

static void add_obstacle_to_list(struct list_head *list, obstacle * a)
{
    struct selected_element *e = MyMalloc(sizeof(struct selected_element));
    e->type = OBJECT_OBSTACLE;
    e->data = a;

    list_add(&e->node, list);
}

static void __clear_selected_list(struct list_head *lst, int nbelem)
{
    struct selected_element *e, *ne;
    list_for_each_entry_safe(e, ne, lst, node) {
	if (nbelem-- == 0)
	    return;
	list_del(&e->node);
	free(e);
    }
}

/**
 * Clear the current selection
 *
 * @param nbelem -1 : Clear the whole list, any other number indicates
 * the number of elements to remove from the list.
 */
void clear_selection(int nbelem)

{
    __clear_selected_list(&selected_elements, nbelem);
}

void clear_clipboard(int nbelem)

{
    __clear_selected_list(&clipboard_elements, nbelem);
}

static void start_rect_select()
{
    mode = RECT;

    state.rect_start.x = mouse_mapcoord.x;
    state.rect_start.y = mouse_mapcoord.y;
    state.rect_len.x = 1;
    state.rect_len.y = 1;
    state.rect_nbelem_selected = 0;
    
    if (state.rect_start.x < 0)
	state.rect_start.x = 0;
    else if (state.rect_start.x >= EditLevel()->xlen -1)
	state.rect_start.x = EditLevel()->xlen -1;

    if (state.rect_start.y < 0)
	state.rect_start.y = 0;
    else if (state.rect_start.y >= EditLevel()->ylen -1)
	state.rect_start.y = EditLevel()->ylen -1;
    
    int idx, a;
    for (a = 0; a < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE && EditLevel()->map[state.rect_start.y][state.rect_start.x].obstacles_glued_to_here[a] != -1; a++) {
	idx = EditLevel()->map[state.rect_start.y][state.rect_start.x].obstacles_glued_to_here[a];
	if (!element_in_selection(&EditLevel()->obstacle_list[idx])) {
	    add_obstacle_to_list(&selected_elements, &EditLevel()->obstacle_list[idx]);
	    state.rect_nbelem_selected ++;
	}
    }
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
	clear_selection(state.rect_nbelem_selected);
	state.rect_nbelem_selected = 0;
    
	if (state.rect_start.x < 0)
	    state.rect_start.x = 0;
	else if (state.rect_start.x >= EditLevel()->xlen -1)
	    state.rect_start.x = EditLevel()->xlen -1;

	if (state.rect_start.y < 0)
	    state.rect_start.y = 0;
	else if (state.rect_start.y >= EditLevel()->ylen -1)
	    state.rect_start.y = EditLevel()->ylen -1;

	if (state.rect_start.x + state.rect_len.x < 0)
	    state.rect_len.x = -state.rect_start.x;
	else if (state.rect_start.x + state.rect_len.x >= EditLevel()->xlen -1)
	    state.rect_len.x = EditLevel()->xlen -1 - state.rect_start.x;

	if (state.rect_start.y + state.rect_len.y < 0)
	    state.rect_len.y = -state.rect_start.y;
	else if (state.rect_start.y + state.rect_len.y >= EditLevel()->ylen -1)
	    state.rect_len.y = EditLevel()->ylen -1 - state.rect_start.y;

	// Then redo a correct one
	for (i = state.rect_start.x; i != state.rect_start.x + state.rect_len.x + state.rect_step.x;
		i += state.rect_step.x) {
	    for (j = state.rect_start.y;
		    j != state.rect_start.y + state.rect_len.y + state.rect_step.y;
		    j += state.rect_step.y) {
		int idx;
		for (a = 0; a < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE && EditLevel()->map[j][i].obstacles_glued_to_here[a] != -1; a++) {
		    idx = EditLevel()->map[j][i].obstacles_glued_to_here[a];
		    add_obstacle_to_list(&selected_elements, &EditLevel()->obstacle_list[idx]);
		    state.rect_nbelem_selected++;
		}
	    }
	}
    }
}

static void end_rect_select()
{
    if (!list_empty(&selected_elements))
	mode = RECTDONE;
    else mode = DISABLED;

    state.single_tile_mark_index = 0;
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

int level_editor_can_cycle_obs()
{
    if (mode != RECTDONE) {
	// We get called from the outside so check mode coherency first
	return 0;
    }

    // N key does nothing on a selection larger than one tile
    if (abs(state.rect_len.x) != 1 || abs(state.rect_len.y) != 1)
	return 0;

    if ((EditLevel()->map[state.rect_start.y][state.rect_start.x].obstacles_glued_to_here[0] == -1) ||
        (EditLevel()->map[state.rect_start.y][state.rect_start.x].obstacles_glued_to_here[1] == -1))
	return 0;


    return 1;
}

void level_editor_cycle_marked_obstacle()
{
    //This function will select only one of the obstacles attached to a given tile,
    //and browse through them for each subsequent call.

    if (!level_editor_can_cycle_obs())
	return;

    // clear the selection and take the next obstacle in the list
    clear_selection(state.rect_nbelem_selected);
    state.rect_nbelem_selected = 0;

    if (EditLevel()->map[state.rect_start.y][state.rect_start.x].obstacles_glued_to_here[state.single_tile_mark_index] == -1) {
	state.single_tile_mark_index = 0;
    }

    int idx = EditLevel()->map[state.rect_start.y][state.rect_start.x].obstacles_glued_to_here[state.single_tile_mark_index];
    add_obstacle_to_list(&selected_elements, &EditLevel()->obstacle_list[idx]);
    state.single_tile_mark_index ++;
    state.rect_nbelem_selected ++;
}


void level_editor_copy_selection()
{
    struct selected_element *e;

    if (mode != RECTDONE) {
	// We get called from the outside so check mode coherency first
	return;
    }
    
    clear_clipboard(-1);

    list_for_each_entry(e, &selected_elements, node) {
	obstacle *a = MyMalloc(sizeof(obstacle));
	memcpy(a, e->data, sizeof(obstacle));

	add_obstacle_to_list(&clipboard_elements, a);
    }
}

void level_editor_cut_selection()
{
    struct selected_element *e;
    int nbelem = 0;
    if (mode != RECTDONE) {
	// We get called from the outside so check mode coherency first
	return;
    }

    level_editor_copy_selection();

    /*XXX for now we simply delete the selection*/
    list_for_each_entry(e, &selected_elements, node) {
	action_remove_obstacle_user(EditLevel(), e->data);

	nbelem ++;
    }

    action_push(ACT_MULTIPLE_ACTIONS, nbelem);

    clear_selection(-1);
}

void level_editor_paste_selection()
{
    struct selected_element *e;
    obstacle *o;
    int nbact = 0;
    moderately_finepoint cmin = { 77777, 7777 }, cmax = { 0, 0}, center;

    if (mode != RECTDONE) {
	// We get called from the outside so check mode coherency first
	return;
    }


    list_for_each_entry(e, &clipboard_elements, node) {
	o = e->data;

	if (o->pos.x < cmin.x)
	    cmin.x = o->pos.x;
	if (o->pos.y < cmin.y)
	    cmin.y = o->pos.y;
	if (o->pos.x > cmax.x)
	    cmax.x = o->pos.x;
	if (o->pos.y > cmax.y)
	    cmax.y = o->pos.y;
    }

    center.x = (cmax.x + cmin.x) / 2;
    center.y = (cmax.y + cmin.y) / 2;
   
    list_for_each_entry(e, &clipboard_elements, node) {

	o = e->data;
	o->pos.x -= center.x;
	o->pos.y -= center.y;

	o->pos.x += mouse_mapcoord.x;
	o->pos.y += mouse_mapcoord.y;

	// Add and select
	add_obstacle_to_list(&selected_elements, action_create_obstacle_user(EditLevel(), o->pos.x, o->pos.y, o->type)); 

	nbact ++;
    }

    action_push(ACT_MULTIPLE_ACTIONS, nbact);
}

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
		return 1;
	    } else if (EVENT_MOVE(event)) {
		do_rect_select();
		return 0;
	    }
	    break;
	case RECTDONE:
	    if (EVENT_LEFT_PRESS(event)) {
		if (ShiftPressed()) {
		    start_drag_drop();
		} else {
		    if (!CtrlPressed()) {
			clear_selection(-1);
		    }
		    start_rect_select();
		}
		return 0;
	    } 
	    break;
	case DRAGDROP:
	    if (EVENT_LEFT_RELEASE(event)) {
		end_drag_drop();
		return 1;
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
    float zf = GameConfig . zoom_is_on ? ONE_OVER_LEVEL_EDITOR_ZOOM_OUT_FACT : 1.0;
    switch(mode) {
	case RECT:
	    //display the selection rectangle
	    translate_map_point_to_screen_pixel (state.rect_start.x , state.rect_start.y , &r1, &c1, zf);
	    translate_map_point_to_screen_pixel ( state.rect_start.x , state.rect_start.y + state.rect_len.y , &r2, &c2, zf);
	    translate_map_point_to_screen_pixel ( state.rect_start.x + state.rect_len.x , state.rect_start.y + state.rect_len.y , &r3, &c3, zf);
	    translate_map_point_to_screen_pixel ( state.rect_start.x + state.rect_len.x , state.rect_start.y , &r4, &c4, zf);

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
