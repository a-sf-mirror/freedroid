
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

#define leveleditor_tool_select_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_display.h"
#include "lvledit/lvledit_tools.h"
#include "lvledit/lvledit_widgets.h"

static struct leveleditor_select {
	point rect_start;
	point rect_len;
	int rect_nbelem_selected;

	int single_tile_mark_index;
	int drag_drop_floor_undoable;

	moderately_finepoint drag_start;
	moderately_finepoint cur_drag_pos;
} state;

enum { DISABLED, FD_RECT, FD_RECTDONE, DRAGDROP } mode;

struct selected_element {
	enum lvledit_object_type type;
	struct list_head node;
	void *data;
};

#define ALL_FLOOR_LAYERS (-1)

/* Selected floor tiles are wrapped into a struct lvledit_map_tile, that holds 
 * coordinate information. struct map_tile does not have coordinate information,
 * and the game doesn't need it, only the leveleditor. */
struct lvledit_map_tile {
	map_tile *tile;
	point coord;
	int layer;
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
 * Check if there is exactly one element selected of the given type.
 * Returns this element, or NULL.
 */
void *single_tile_selection(int type)
{
	struct selected_element *s;
	if (list_empty(&selected_elements))
		return NULL;
	if (selected_elements.next->next != &selected_elements)
		return NULL;

	s = list_entry(selected_elements.next, struct selected_element, node);
	if (s->type != type)
		return NULL;

	return s->data;
}

/**
 * Check whether the given element is among the selected objects.
 */
int element_in_selection(void *data)
{
	struct selected_element *e;
	list_for_each_entry(e, &selected_elements, node) {
		switch(e->type) {
		case OBJECT_FLOOR:
			// We must compare the floor tiles in the wrapper structure
			if (((struct lvledit_map_tile *) (e->data))->tile == data)
				return 1;
			break;
		case OBJECT_WAYPOINT:
			// We must compare the coordinates of the two waypoints
			if (((waypoint *)e->data)->x == ((waypoint *)data)->x &&
				((waypoint *)e->data)->y == ((waypoint *)data)->y)
				return 1;
			break;
		case OBJECT_MAP_LABEL:
			// We must compare the coordinates of the two map labels
			if (((map_label *)e->data)->pos.x == ((map_label *)data)->pos.x &&
				((map_label *)e->data)->pos.y == ((map_label *)data)->pos.y)
				return 1;
			break;
		default:
			if (e->data == data)
				return 1;
		}		
	}

	return 0;
}

static void __calc_min_max(float x, float y, moderately_finepoint *cmin, moderately_finepoint *cmax)
{
	if (x < cmin->x)
		cmin->x = x;
	if (y < cmin->y)
		cmin->y = y;
	if (x > cmax->x)
		cmax->x = x;
	if (y > cmax->y)
		cmax->y = y;
}

static void calc_min_max_selection(struct list_head *list, moderately_finepoint *cmin, moderately_finepoint *cmax)
{
	struct selected_element *e;
	struct lvledit_map_tile *t;
	waypoint *w;
	obstacle *o;
	item *it;
	map_label *m;
	enemy *en;
	
	cmin->x = 9999;
	cmin->y = 9999;
	cmax->x = 0;
	cmax->y = 0;
	
	list_for_each_entry(e, list, node) {
		switch (e->type) {
		case OBJECT_OBSTACLE:
			o = e->data;
		
			__calc_min_max(o->pos.x, o->pos.y, cmin, cmax);
			break;
		case OBJECT_FLOOR:	
			t = e->data;

			__calc_min_max(t->coord.x, t->coord.y, cmin, cmax);
			break;
		case OBJECT_ITEM:
			it = e->data;

			__calc_min_max(it->pos.x, it->pos.y, cmin, cmax);
			break;
		case OBJECT_WAYPOINT:
			w = e->data;

			__calc_min_max(w->x, w->y, cmin, cmax);
			break;
		case OBJECT_MAP_LABEL:
			m = e->data;

			__calc_min_max(m->pos.x, m->pos.y, cmin, cmax);
			break;
		case OBJECT_ENEMY:
			en = e->data;

			__calc_min_max(en->pos.x, en->pos.y, cmin, cmax);
			break;
		default:
			;
		}
	}
}

static int set_floor_layers(level *lvl, struct lvledit_map_tile *tile)
{
	int i;
	struct map_tile *t = tile->tile;

	if (tile->layer == ALL_FLOOR_LAYERS) {
		for (i = 0; i < lvl->floor_layers; i++)
			action_set_floor_layer(EditLevel(), tile->coord.x, tile->coord.y, i, t->floor_values[i]);
		return i;
	} else {
		action_set_floor_layer(lvl, tile->coord.x, tile->coord.y, current_floor_layer, t->floor_values[tile->layer]);
		return 1;
	}
}

point selection_start() {
	return state.rect_start;
}

point selection_len() {
	return state.rect_len;
}

int selection_type()
{
	struct widget_lvledit_categoryselect *cs = get_current_object_type();

	if (!cs) {
		return OBJECT_NONE;
	}

	return cs->type;
}

static void add_object_to_list(struct list_head *list, void *data, enum lvledit_object_type type)
{
	struct selected_element *e = MyMalloc(sizeof(struct selected_element));
	e->type = type;
	e->data = data;

	list_add(&e->node, list);
}

static void __clear_selected_list(struct list_head *lst, int nbelem)
{
	struct selected_element *e, *ne;
	list_for_each_entry_safe(e, ne, lst, node) {
		if (nbelem-- == 0)
			return;
		if (e->type == OBJECT_FLOOR || e->type == OBJECT_WAYPOINT || e->type == OBJECT_MAP_LABEL) {
			/* Unselecting floor tiles, waypoints or map labels requires freeing the duplicated data */
			free(e->data);
		}
		list_del(&e->node);
		free(e);
	}
}

static void select_floor_on_tile(int x, int y)
{
	if (!element_in_selection(&EditLevel()->map[y][x])) {
		struct lvledit_map_tile *t = MyMalloc(sizeof(struct lvledit_map_tile));
		t->tile = &EditLevel()->map[y][x];
		t->coord.x = x;
		t->coord.y = y;
		
		add_object_to_list(&selected_elements, t, OBJECT_FLOOR);
		state.rect_nbelem_selected++;
	}
}

static void select_obstacles_on_tile(int x, int y)
{
	int idx, a;
	for (a = 0; a < EditLevel()->map[y][x].glued_obstacles.size; a++) {
		idx = ((int *)(EditLevel()->map[y][x].glued_obstacles.arr))[a];
		if (!element_in_selection(&EditLevel()->obstacle_list[idx])) {
		add_object_to_list(&selected_elements, &EditLevel()->obstacle_list[idx], OBJECT_OBSTACLE);
			state.rect_nbelem_selected++;
		}
	}
}

static void select_waypoint_on_tile(int x, int y)
{
	waypoint *wpts = EditLevel()->waypoints.arr;
	int i;

	for (i = 0; i < EditLevel()->waypoints.size; i++) {
		if (wpts[i].x == x && wpts[i].y == y) {
			if (!element_in_selection(&wpts[i])) {
				waypoint *w = MyMalloc(sizeof(waypoint));
				memcpy(w, &wpts[i], sizeof(waypoint));

				add_object_to_list(&selected_elements, w, OBJECT_WAYPOINT);
				state.rect_nbelem_selected++;
			}
		}
	}
}

static void select_item_on_tile(int x, int y)
{
	item *it;
	int i;

	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		// Get the item
		it = &EditLevel()->ItemList[i];

		if (it->type == -1)
			continue;

		if ((fabsf(x - it->pos.x) <= 0.5) && (fabsf(y - it->pos.y) <= 0.5)) {
			if (!element_in_selection(it)) {
				add_object_to_list(&selected_elements, it, OBJECT_ITEM);
				state.rect_nbelem_selected++;
			}
		}
	}
}

static void select_map_label_on_tile(int x, int y)
{
	map_label *labels = EditLevel()->map_labels.arr;
	int i;

	for (i = 0; i < EditLevel()->map_labels.size; i++) {
		if (labels[i].pos.x == x && labels[i].pos.y == y) {
			if (!element_in_selection(&labels[i])) {
				map_label *m = MyMalloc(sizeof(map_label));
				memcpy(m, &labels[i], sizeof(map_label));

				add_object_to_list(&selected_elements, m, OBJECT_MAP_LABEL);
				state.rect_nbelem_selected++;
			}
		}
	}
}

static void select_special_forces_on_tile(int x, int y)
{
	enemy *en;

	BROWSE_LEVEL_BOTS(en, EditLevel()->levelnum) {
		if (!en->SpecialForce)
			continue;

		if ((int)en->pos.x == x && (int)en->pos.y == y) {
			if (!element_in_selection(en)) {
				add_object_to_list(&selected_elements, en, OBJECT_ENEMY);
				state.rect_nbelem_selected++;
			}
		}
	}
}

static void select_object_on_tile(int x, int y)
{
	switch (selection_type()) {
	case OBJECT_OBSTACLE:
		select_obstacles_on_tile(x, y);
		break;
	case OBJECT_FLOOR:
		select_floor_on_tile(x, y);
		break;
	case OBJECT_WAYPOINT:
		select_waypoint_on_tile(x, y);
		break;
	case OBJECT_ITEM:
		select_item_on_tile(x, y);
		break;
	case OBJECT_MAP_LABEL:
		select_map_label_on_tile(x, y);
		break;
	case OBJECT_ENEMY:
		select_special_forces_on_tile(x, y);
		break;
	default:
		ErrorMessage(__FUNCTION__,
				"Abstract object type %d for leveleditor not supported.\n", PLEASE_INFORM, IS_WARNING_ONLY, selection_type());
		break;
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
	mode = FD_RECT;

	// Store mouse position
	state.drag_start.x = (int)mouse_mapcoord.x;
	state.drag_start.y = (int)mouse_mapcoord.y;
	state.cur_drag_pos.x = (int)mouse_mapcoord.x;
	state.cur_drag_pos.y = (int)mouse_mapcoord.y;

	// Selection area = current tile
	state.rect_start.x = (int)mouse_mapcoord.x;
	state.rect_start.y = (int)mouse_mapcoord.y;
	state.rect_len.x = 1;
	state.rect_len.y = 1;
	state.rect_nbelem_selected = 0;

	// Create selected obstacles list
	if (!pos_inside_level(state.rect_start.x, state.rect_start.y, EditLevel()))
		return;

	// Select elements on the starting tile
	select_object_on_tile(state.rect_start.x, state.rect_start.y);
}

static void do_rect_select()
{
	int i, j;

	// If there is something to change
	if (((int)mouse_mapcoord.x != state.cur_drag_pos.x) || ((int)mouse_mapcoord.y != state.cur_drag_pos.y)) {

		// Store new mouse position
		state.cur_drag_pos.x = (int)mouse_mapcoord.x;
		state.cur_drag_pos.y = (int)mouse_mapcoord.y;

		// Redefine the selection rectangle (start corner is always at north-west)
		float xlen = state.cur_drag_pos.x - state.drag_start.x;
		float ylen = state.cur_drag_pos.y - state.drag_start.y;

		if (xlen >= 0) {
			state.rect_start.x = state.drag_start.x;
		} else {
			xlen = -xlen;
			state.rect_start.x = state.drag_start.x - xlen;
		}
		state.rect_len.x = xlen + 1;

		if (ylen >= 0) {
			state.rect_start.y = state.drag_start.y;
		} else {
			ylen = -ylen;
			state.rect_start.y = state.drag_start.y - ylen;
		}
		state.rect_len.y = ylen + 1;

		// Undo previous selection
		clear_selection(state.rect_nbelem_selected);
		state.rect_nbelem_selected = 0;

		// Then redo a correct one      
		for (j = state.rect_start.y; j < state.rect_start.y + state.rect_len.y; ++j) {
			if (j < 0 || j >= EditLevel()->ylen)
				continue;

			for (i = state.rect_start.x; i < state.rect_start.x + state.rect_len.x; ++i) {
				if (i < 0 || i >= EditLevel()->xlen)
					continue;

				select_object_on_tile(i, j);
			}

		}
	}
}

static void end_rect_select()
{
	if (!list_empty(&selected_elements))
		mode = FD_RECTDONE;
	else
		mode = DISABLED;

	state.single_tile_mark_index = 0;
	
	// A drag&drop operation for the floor is finished when the floor tiles are 
	// unselected.
	// Do not allow to undo the previous actions for drag&drop
	state.drag_drop_floor_undoable = 0;
}

static void start_drag_drop()
{
	if (selection_type() == OBJECT_FLOOR) {
		// Copy the original selection in order not to change it during a drag&drop
		// operation
		level_editor_copy_selection();
	}
	
	mode = DRAGDROP;
	state.drag_start.x = mouse_mapcoord.x;
	state.drag_start.y = mouse_mapcoord.y;
	state.cur_drag_pos.x = mouse_mapcoord.x;
	state.cur_drag_pos.y = mouse_mapcoord.y;
}

static void do_drag_drop_obstacle(moderately_finepoint diff)
{
	struct selected_element *e;

	list_for_each_entry(e, &selected_elements, node) {
		if (e->type != OBJECT_OBSTACLE)
			return;

		obstacle *o = ((obstacle *)(e->data));

		move_obstacle(o, o->pos.x + diff.x, o->pos.y + diff.y);
	}	
	state.cur_drag_pos.x = mouse_mapcoord.x;
	state.cur_drag_pos.y = mouse_mapcoord.y;
}

static void do_drag_drop_floor(moderately_finepoint diff)
{
	struct selected_element *e;
	struct lvledit_map_tile *t;
	int changed_tiles = 0;

	// Move the selection if the displacement exceeds half a tile
	if (fabsf(diff.x) >= 0.5 || fabsf(diff.y) >= 0.5 ) {

		if ((state.drag_start.x != state.cur_drag_pos.x) 
			|| (state.drag_start.y != state.cur_drag_pos.y)) {
			// When the selection has already moved during the drag&drop, 
			// we must undo the actions to the old selection
			level_editor_action_undo();
		}

		if (state.drag_drop_floor_undoable)	{
			// When the last action is a floor drag&drop operation, we need to undo
			// it in order not to overwrite the tiles underneath
			level_editor_action_undo();
			state.drag_drop_floor_undoable = 0;
		}
		
		// When moving the selection, new tiles will be selected, so, we must 
		// unselect the previous selection because we must not change the original 
		// selection
		clear_selection(-1);
		
		// Browse the original selection in order to set the floor of the new selection
		list_for_each_entry(e, &clipboard_elements, node) {
			if (e->type != OBJECT_FLOOR)
				return;

			t = e->data;

			// Calculate the new coordinates of the tile
			t->coord.x += (int)rintf(diff.x);
			t->coord.y += (int)rintf(diff.y);

			// Set the floor for the new current tile
			changed_tiles += set_floor_layers(EditLevel(), t);

			// Select the new tile
			select_floor_on_tile(t->coord.x, t->coord.y);
		}
		state.cur_drag_pos.x += (int)rintf(diff.x);
		state.cur_drag_pos.y += (int)rintf(diff.y);
		
		action_push(ACT_MULTIPLE_ACTIONS, changed_tiles);
	}
}

static void do_drag_drop_item(moderately_finepoint diff)
{
	struct selected_element *e;

	list_for_each_entry(e, &selected_elements, node) {
		if (e->type != OBJECT_ITEM)
			return;

		// Calculate the new coordinates of the item
		((item *) (e->data))->pos.x += diff.x;
		((item *) (e->data))->pos.y += diff.y;
	}	
	state.cur_drag_pos.x = mouse_mapcoord.x;
	state.cur_drag_pos.y = mouse_mapcoord.y;
}

static void do_drag_drop_waypoint(moderately_finepoint diff)
{
	struct selected_element *e;
	waypoint *w, *w2;
	int wp_pos;

	// Move the selection if the displacement exceeds half a tile
	if (fabsf(diff.x) >= 0.5 || fabsf(diff.y) >= 0.5 ) {
		list_for_each_entry(e, &selected_elements, node) {
			if (e->type != OBJECT_WAYPOINT)
				return;

			w = e->data;

			// In order to retrieve the actual waypoint we must obtain its index in the level's waypoint array
			wp_pos = get_waypoint(EditLevel(), w->x, w->y);

			// Modify the selection coordinates
			w->x += (int)rintf(diff.x);
			w->y += (int)rintf(diff.y);

			// Retrieve the original waypoint
			w2 = &(((waypoint *)(EditLevel()->waypoints.arr))[wp_pos]);

			// Modify the actual waypoint
			move_waypoint(EditLevel(), w2, w->x, w->y);
		}

		state.cur_drag_pos.x += (int)rintf(diff.x);
		state.cur_drag_pos.y += (int)rintf(diff.y);
	}
}

static void do_drag_drop()
{
	moderately_finepoint cmin, cmax, diff;

	// Calculate the coordinates min/max of the selection.
	calc_min_max_selection(&selected_elements, &cmin, &cmax);

	// Calculate the new displacement of the selection.
	diff.x = mouse_mapcoord.x - state.cur_drag_pos.x;
	diff.y = mouse_mapcoord.y - state.cur_drag_pos.y;

	if ((cmax.y + diff.y) >= EditLevel()->ylen || (cmax.x + diff.x) >= EditLevel()->xlen
		|| (cmin.x + diff.x) < 0 || (cmin.y + diff.y) < 0) {
		// Do not place objects outside the level.
		return;
	}

	switch (selection_type()) {
	case OBJECT_OBSTACLE:
		do_drag_drop_obstacle(diff);
		break;
	case OBJECT_FLOOR:
		do_drag_drop_floor(diff);
		break;
	case OBJECT_ITEM:
		do_drag_drop_item(diff);
		break;
	case OBJECT_WAYPOINT:
		do_drag_drop_waypoint(diff);
		break;
	default:
		break;
	}
}

static void end_drag_drop()
{
	struct selected_element *e;
	int enb = 0;
	double x, y;
	int wp_pos;
	waypoint *w, *w2;

	// Set up undo actions
	list_for_each_entry(e, &selected_elements, node) {
		switch (e->type) {
		case OBJECT_OBSTACLE:
			x = (((obstacle *) (e->data))->pos.x - state.cur_drag_pos.x + state.drag_start.x);
			y = (((obstacle *) (e->data))->pos.y - state.cur_drag_pos.y + state.drag_start.y);
			action_push(ACT_MOVE_OBSTACLE, (obstacle *) e->data, x, y);
			enb++;
			break;
		case OBJECT_ITEM:
			x = (((item *) (e->data))->pos.x - state.cur_drag_pos.x + state.drag_start.x);
			y = (((item *) (e->data))->pos.y - state.cur_drag_pos.y + state.drag_start.y);
			action_push(ACT_MOVE_ITEM, (item *) e->data, x, y);
			enb++;
			break;
		case OBJECT_WAYPOINT:
			// In order to store the undo action for each waypoint we need to
			// compute the initial position of the waypoint
			w = e->data;
			x = (w->x - (int)(state.cur_drag_pos.x - state.drag_start.x));
			y = (w->y - (int)(state.cur_drag_pos.y - state.drag_start.y));

			// In order to retrieve the actual waypoint we must obtain its index in the level's waypoints array
			wp_pos = get_waypoint(EditLevel(), w->x, w->y);

			// Retrieve the original waypoint and push the undo action
			w2 = &(((waypoint *)(EditLevel()->waypoints.arr))[wp_pos]);
			action_push(ACT_MOVE_WAYPOINT, w2, (int)x, (int)y);
			enb++;
			break;
		default:
			;
		}
	}

	if (enb)
		action_push(ACT_MULTIPLE_ACTIONS, enb);

	mode = FD_RECTDONE;
	
	if (selection_type() == OBJECT_FLOOR) {
		// Remember that we have just dragged some floor tiles.
		// As long as they remain selected, a subsequent drag operation
		// will need to undo the placement of those tiles, in order
		// to avoid seeing existing tiles overwritten.
		state.drag_drop_floor_undoable = 1;
	}
}

static int level_editor_number_of_marked_items()
{
	item *it;
	int i, num = 0;

	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		it = &EditLevel()->ItemList[i];

		if (it->type == -1)
			continue;

		if ((fabsf(state.rect_start.x - it->pos.x) <= 0.5) && (fabsf(state.rect_start.y - it->pos.y) <= 0.5)) {
			num++;
		}
	}

	return num;
}

int level_editor_can_cycle_marked_object()
{
	if (mode != FD_RECTDONE) {
		// We get called from the outside so check mode coherency first
		return 0;
	}
	// N key does nothing on a selection larger than one tile
	if (abs(state.rect_len.x) != 1 || abs(state.rect_len.y) != 1)
		return 0;
	
	if (!pos_inside_level(state.rect_start.x, state.rect_start.y, EditLevel()))
		return 0;

	switch (selection_type()) {	
		case OBJECT_OBSTACLE: 		
			if (EditLevel()->map[state.rect_start.y][state.rect_start.x].glued_obstacles.size <= 1)
				return 0;
			break;		
		case OBJECT_ITEM:
			if (level_editor_number_of_marked_items() <= 1)
				return 0;
			break;
		default:
			return 0;
			break;				
	}

	return 1;
}

static void level_editor_cycle_marked_obstacle()
{
	//This function will select only one of the obstacles attached to a given tile,
	//and browse through them for each subsequent call.

	if (state.single_tile_mark_index >= EditLevel()->map[state.rect_start.y][state.rect_start.x].glued_obstacles.size) {
		state.single_tile_mark_index = 0;
	}

	int idx = ((int *)(EditLevel()->map[state.rect_start.y][state.rect_start.x].glued_obstacles.arr))[state.single_tile_mark_index];
	add_object_to_list(&selected_elements, &EditLevel()->obstacle_list[idx], OBJECT_OBSTACLE);
}

static void level_editor_cycle_marked_item()
{
	item *it;
	int i, j;	

	if (state.single_tile_mark_index >= level_editor_number_of_marked_items()) {
		state.single_tile_mark_index = 0;
	}

	for (i = 0,j = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		it = &EditLevel()->ItemList[i];

		if (it->type == -1)
			continue;

		if ((fabsf(state.rect_start.x - it->pos.x) <= 0.5) && (fabsf(state.rect_start.y- it->pos.y) <= 0.5)) {
			if (state.single_tile_mark_index == j) {
				break;
			}

			j++;
		}
 
	}

	add_object_to_list(&selected_elements, it, OBJECT_ITEM);
}

void level_editor_cycle_marked_object()
{
	if (!level_editor_can_cycle_marked_object())
		return;

	clear_selection(state.rect_nbelem_selected);
	state.rect_nbelem_selected = 0;	
	
	switch (selection_type()) {
		case OBJECT_OBSTACLE:
			level_editor_cycle_marked_obstacle();
			break;
		case OBJECT_ITEM:
			level_editor_cycle_marked_item();
			break;
		default:
			ErrorMessage(__FUNCTION__, "Unhandled selection type. \n", IS_WARNING_ONLY, NO_NEED_TO_INFORM);
			break;
	}
	
	state.single_tile_mark_index++;
	state.rect_nbelem_selected = 1;
}

void level_editor_copy_selection()
{
	struct selected_element *e;
	struct lvledit_map_tile *t;	
	waypoint *w;
	obstacle *o;
	item *it;
	
	if (mode != FD_RECTDONE) {
		// We get called from the outside so check mode coherency first
		return;
	}

	if (selection_empty()) {
		// When the list of the selected elements is empty, we do not want to clear
		// selection from the clipboard
		return;
	}

	clear_clipboard(-1);

	list_for_each_entry(e, &selected_elements, node) {
		switch (e->type) {
		case OBJECT_OBSTACLE:
			o = MyMalloc(sizeof(obstacle));
			memcpy(o, e->data, sizeof(obstacle));

			add_object_to_list(&clipboard_elements, o, OBJECT_OBSTACLE);
			break;
		case OBJECT_FLOOR:	
			t = MyMalloc(sizeof(struct lvledit_map_tile));
			t->tile = MyMalloc(sizeof(map_tile));		
				
			memcpy(t->tile, ((struct lvledit_map_tile *) (e->data))->tile, sizeof(map_tile));
			t->coord.x = ((struct lvledit_map_tile *) (e->data))->coord.x;
			t->coord.y = ((struct lvledit_map_tile *) (e->data))->coord.y;		
			t->layer = GameConfig.show_all_floor_layers ? ALL_FLOOR_LAYERS : current_floor_layer;
			
			add_object_to_list(&clipboard_elements, t, OBJECT_FLOOR);
			break;
		case OBJECT_ITEM:
			it = MyMalloc(sizeof(item));
			memcpy(it, e->data, sizeof(item));
			
			add_object_to_list(&clipboard_elements, it, OBJECT_ITEM);
			break;	
		case OBJECT_WAYPOINT:
			w = MyMalloc(sizeof(waypoint));
			memcpy(w, e->data, sizeof(waypoint));

			add_object_to_list(&clipboard_elements, w, OBJECT_WAYPOINT);
			break;
		default:
			;
		}
	}
}

static int clear_current_floor_layers(level *lvl, int coord_x, int coord_y)
{
	int i;

	if (GameConfig.show_all_floor_layers) {
		for (i = 0; i < lvl->floor_layers; i++)
			action_set_floor_layer(lvl, coord_x, coord_y, i, ISO_FLOOR_EMPTY);
		return i;
	} else {
		action_set_floor_layer(lvl, coord_x, coord_y, current_floor_layer, ISO_FLOOR_EMPTY);
		return 1;
	}
}

void level_editor_cut_selection()
{
	struct selected_element *e;
	int nbelem = 0;
	if (mode != FD_RECTDONE) {
		// We get called from the outside so check mode coherency first
		return;
	}

	level_editor_copy_selection();

	list_for_each_entry(e, &selected_elements, node) {
		switch (e->type) {
		case OBJECT_OBSTACLE:
			action_remove_obstacle_user(EditLevel(), e->data);
			nbelem++;
			break;
		case OBJECT_FLOOR:
			// We replace the tile we cut by a black tile
			nbelem += clear_current_floor_layers(EditLevel(),
				((struct lvledit_map_tile *) (e->data))->coord.x,
				((struct lvledit_map_tile *) (e->data))->coord.y);
			break;
		case OBJECT_ITEM:
			action_remove_item(EditLevel(), e->data);
			nbelem++;
			break;
		case OBJECT_WAYPOINT:
			action_remove_waypoint(EditLevel(), ((waypoint *)(e->data))->x, ((waypoint *)(e->data))->y);
			nbelem++;
			break;
		default:
			break;
		}
	}
	
	action_push(ACT_MULTIPLE_ACTIONS, nbelem);

	clear_selection(-1);
}

void level_editor_paste_selection()
{		
	struct selected_element *e;
	struct lvledit_map_tile *t;	
	waypoint *w;
	obstacle *o;
	item *it;
	int nbact = 0;
	moderately_finepoint cmin, cmax, center;

	if (mode == FD_RECT || mode == DRAGDROP) {
		// We get called from the outside so check mode coherency first
		return;
	}

	if (!mouse_in_level) {
		// We must not paste objects outside of the level
		return;
	}
	
	// Before a paste operation we want to unselect the previous selection
	clear_selection(-1);
	
	// Calculate the coordinates min/max of the selection
	calc_min_max_selection(&clipboard_elements, &cmin, &cmax);

	// Calculate the center of the selection
	center.x = (cmax.x + cmin.x) / 2;
	center.y = (cmax.y + cmin.y) / 2;
		
	list_for_each_entry(e, &clipboard_elements, node) {
		switch (e->type) {
		case OBJECT_OBSTACLE:	
			o = e->data;
			
			o->pos.x -= center.x;
			o->pos.y -= center.y;

			o->pos.x += mouse_mapcoord.x;
			o->pos.y += mouse_mapcoord.y;
			
			if (o->pos.x >= EditLevel()->xlen || o->pos.y >= EditLevel()->ylen
				|| o->pos.x < 0 || o->pos.y < 0) {
				// We must not paste obstacles outside the boundaries of level
				break;
			}
			
			// Add and select
			add_object_to_list(&selected_elements, action_create_obstacle_user(EditLevel(), o->pos.x, o->pos.y, o->type), OBJECT_OBSTACLE);
			
			nbact++;
			break;
		case OBJECT_FLOOR:
			t = e->data;
			
			t->coord.x -= ceil(center.x);
			t->coord.y -= ceil(center.y);
					
			t->coord.x += (int)Me.pos.x;
			t->coord.y += (int)Me.pos.y;
						
			if (t->coord.x >= EditLevel()->xlen || t->coord.y >= EditLevel()->ylen
				|| t->coord.x < 0 || t->coord.y < 0) {
				// We must not paste tiles outside the boundaries of level
				break;
			}

			// Set and select	current tile
			nbact += set_floor_layers(EditLevel(), t);
			select_floor_on_tile(t->coord.x, t->coord.y);
			break;
		case OBJECT_ITEM:
			it = e->data;

			it->pos.x -= center.x;
			it->pos.y -= center.y;

			it->pos.x += mouse_mapcoord.x;
			it->pos.y += mouse_mapcoord.y;

			if (it->pos.x >= EditLevel()->xlen || it->pos.y >= EditLevel()->ylen
				|| it->pos.x < 0 || it->pos.y < 0) {
				// We must not paste items outside the boundaries of level
				break;
			}

			// Add and select
		add_object_to_list(&selected_elements, action_create_item(EditLevel(), it->pos.x, it->pos.y, it->type), OBJECT_ITEM);

			nbact++;
			break;
		case OBJECT_WAYPOINT:
			w = e->data;

			w->x -= ceil(center.x);
			w->y -= ceil(center.y);

			w->x += (int)Me.pos.x;
			w->y += (int)Me.pos.y;

			if (w->x >= EditLevel()->xlen || w->y >= EditLevel()->ylen
				|| w->x < 0 || w->y < 0) {
				// We must not paste waypoints outside the boundaries of level
				break;
			}

			// Add and select
			action_create_waypoint(EditLevel(), w->x, w->y, w->suppress_random_spawn);
			select_waypoint_on_tile(w->x, w->y);

			nbact++;
			break;
		default:
			break;
		}
	}
	
	action_push(ACT_MULTIPLE_ACTIONS, nbact);
}

int leveleditor_select_input(SDL_Event * event)
{
	if(!mouse_in_level)
		return 0;

	if (mode != DISABLED) {
		if (EVENT_LEFT_RELEASE(event) || EVENT_KEYPRESS(event, SDLK_ESCAPE)) {
			// Cancel the current operation
			leveleditor_select_reset();
			return 1;
		}
	}

	switch (mode) {
	case DISABLED:
		if (EVENT_LEFT_PRESS(event)) {
			start_rect_select();
			return 0;
		}
		break;
	case FD_RECT:
		if (EVENT_MOVE(event)) {
			do_rect_select();
			return 0;
		}
		break;
	case FD_RECTDONE:
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
		if (EVENT_MOVE(event)) {
			do_drag_drop();
			return 0;
		}
		break;
	}

	return 0;
}

int leveleditor_select_display()
{
	int r1, r2, r3, r4, c1, c2, c3, c4;
	switch (mode) {
	case FD_RECT:
		//display the selection rectangle
		translate_map_point_to_screen_pixel(state.rect_start.x, state.rect_start.y, &r1, &c1);
		translate_map_point_to_screen_pixel(state.rect_start.x, state.rect_start.y + state.rect_len.y, &r2, &c2);
		translate_map_point_to_screen_pixel(state.rect_start.x + state.rect_len.x, state.rect_start.y + state.rect_len.y, &r3, &c3);
		translate_map_point_to_screen_pixel(state.rect_start.x + state.rect_len.x, state.rect_start.y, &r4, &c4);

		short x[4] = { r1, r2, r3, r4 };
		short y[4] = { c1, c2, c3, c4 };

		draw_quad(x, y, 31, 127, 143, 143);
		break;
	default:
		break;
	}
	return 0;
}

void leveleditor_select_reset()
{
	switch (mode) {
	case FD_RECT:
			end_rect_select();
			break;
	case DRAGDROP:
			end_drag_drop();
			break;
	default:
			break;
	}
}

void level_editor_switch_selection_type(int direction)
{
	int type = selection_type() + direction;
	if (type < 0)
		type = OBJECT_MAP_LABEL;
	else if (type > OBJECT_MAP_LABEL)
		type = OBJECT_OBSTACLE;
	lvledit_select_type(type);
}
