
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

#define leveleditor_tool_place_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "SDL_rotozoom.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_display.h"

#include "lvledit/lvledit_widgets.h"
#include "lvledit/lvledit_tools.h"

static enum {
	DISABLED,
	RECTANGLE_FLOOR,
	LINE_WALLS,
	CONNECT_WAYPOINT,
} our_mode;

static struct leveleditor_place {
	/* Line mode */
	int l_direction;
	int l_id;
	line_element l_elements;

	/* Rectangle mode */
	point r_start;
	int r_len_x, r_len_y;
	int r_step_x, r_step_y;
	int r_tile_used;

	/* Waypoint mode */
	int origwp;
	int startwp;
	int nbactions;
} state;

static void end_waypoint_route()
{
	our_mode = DISABLED;
	action_push(ACT_MULTIPLE_ACTIONS, state.nbactions);
}

static int do_waypoint_route(int rspawn)
{
	int wpnum;
	int isnew;

	wpnum = CreateWaypoint(EditLevel(), mouse_mapcoord.x, mouse_mapcoord.y, &isnew);

	if (our_mode != CONNECT_WAYPOINT) {
		//we are starting a new route
		state.startwp = -1;
		state.nbactions = 0;
	}
	if (isnew) {
		action_push(ACT_WAYPOINT_TOGGLE, (int)mouse_mapcoord.x, (int)mouse_mapcoord.y, rspawn);
		state.nbactions++;
	}

	if (state.startwp != -1 && wpnum != state.startwp) {
		// Connect to the previous waypoint
		if (action_toggle_waypoint_connection(EditLevel(), state.startwp, wpnum, 0, 0) == 1)
			state.nbactions++;
		if (action_toggle_waypoint_connection(EditLevel(), wpnum, state.startwp, 0, 0) == 1)
			state.nbactions++;
	}

	EditLevel()->AllWaypoints[wpnum].suppress_random_spawn = rspawn;

	if (wpnum == state.startwp) {
		// Click on the same waypoint ? end the route
		end_waypoint_route();
		return 1;
	}

	if (!isnew && wpnum == state.origwp) {
		end_waypoint_route();
		return 1;
	}

	if (our_mode != CONNECT_WAYPOINT) {
		our_mode = CONNECT_WAYPOINT;
		state.origwp = wpnum;
	}

	state.startwp = wpnum;
	return 0;
}

static void place_single_obstacle(struct leveleditor_typeselect *ts)
{
	moderately_finepoint pos;
	pos.x = mouse_mapcoord.x;
	pos.y = mouse_mapcoord.y;

	action_create_obstacle_user(EditLevel(), pos.x, pos.y, ts->indices[ts->selected_tile_nb]);
}

static void start_rectangle_floor(int findex)
{
	our_mode = RECTANGLE_FLOOR;

	// Starting values
	state.r_start.x = (int)mouse_mapcoord.x;
	state.r_start.y = (int)mouse_mapcoord.y;
	state.r_len_x = 0;
	state.r_len_y = 0;

	// The tile we'll use 
	state.r_tile_used = findex;

	if (state.r_start.x < 0)
		state.r_start.x = 0;
	else if (state.r_start.x >= EditLevel()->xlen - 1)
		state.r_start.x = EditLevel()->xlen - 1;

	if (state.r_start.y < 0)
		state.r_start.y = 0;
	else if (state.r_start.y >= EditLevel()->ylen - 1)
		state.r_start.y = EditLevel()->ylen - 1;

	// Place the first tile 
	action_set_floor(EditLevel(), state.r_start.x, state.r_start.y, state.r_tile_used);
	action_push(ACT_MULTIPLE_ACTIONS, 1);

	return;
}				// void start_rectangle_mode ( leveleditor_state cur_state , int already_defined )

static void handle_rectangle_floor()
{
	int i, j;
	int changed_tiles = 0;

	// If there is something to change
	if (calc_distance(mouse_mapcoord.x, mouse_mapcoord.y,
				 state.r_start.x + state.r_len_x, state.r_start.y + state.r_len_y) > 0.5) {
		// Redefine the rectangle dimensions
		state.r_len_x = (int)mouse_mapcoord.x - state.r_start.x;
		state.r_step_x = (state.r_len_x > 0 ? 1 : -1);
		state.r_len_y = (int)mouse_mapcoord.y - state.r_start.y;
		state.r_step_y = (state.r_len_y > 0 ? 1 : -1);

		// Undo previous rectangle
		level_editor_action_undo();

		if (state.r_start.x < 0)
			state.r_start.x = 0;
		else if (state.r_start.x >= EditLevel()->xlen - 1)
			state.r_start.x = EditLevel()->xlen - 1;

		if (state.r_start.y < 0)
			state.r_start.y = 0;
		else if (state.r_start.y >= EditLevel()->ylen - 1)
			state.r_start.y = EditLevel()->ylen - 1;

		if (state.r_start.x + state.r_len_x < 0)
			state.r_len_x = -state.r_start.x;
		else if (state.r_start.x + state.r_len_x >= EditLevel()->xlen - 1)
			state.r_len_x = EditLevel()->xlen - 1 - state.r_start.x;

		if (state.r_start.y + state.r_len_y < 0)
			state.r_len_y = -state.r_start.y;
		else if (state.r_start.y + state.r_len_y >= EditLevel()->ylen - 1)
			state.r_len_y = EditLevel()->ylen - 1 - state.r_start.y;

		// Then redo a correct one
		for (i = state.r_start.x; i != state.r_start.x + state.r_len_x + state.r_step_x; i += state.r_step_x) {
			for (j = state.r_start.y; j != state.r_start.y + state.r_len_y + state.r_step_y; j += state.r_step_y) {
				action_set_floor(EditLevel(), i, j, state.r_tile_used);
				changed_tiles++;
			}
		}
		action_push(ACT_MULTIPLE_ACTIONS, changed_tiles);
	}
}

static void end_rectangle_floor(int commit)
{
	our_mode = DISABLED;
	if (!commit)
		level_editor_action_undo();
}

static void start_line_walls(int windex)
{
	our_mode = LINE_WALLS;

	// Initialize a line
	INIT_LIST_HEAD(&(state.l_elements.list));

	state.l_direction = UNDEFINED;

	state.l_id = windex;

	state.l_elements.position.x = (int)mouse_mapcoord.x + ((obstacle_map[state.l_id].flags & IS_HORIZONTAL) ? 0.5 : 0);

	state.l_elements.position.y = (int)mouse_mapcoord.y + ((obstacle_map[state.l_id].flags & IS_HORIZONTAL) ? 0 : 0.5);

	state.l_elements.address = action_create_obstacle_user(EditLevel(),
							       state.l_elements.position.x, state.l_elements.position.y, state.l_id);

}

static void handle_line_walls()
{
	line_element *wall;
	int actual_direction;
	float distance;
	moderately_finepoint pos_last;
	moderately_finepoint offset;
	int direction_is_possible;

	wall = list_entry((state.l_elements).list.prev, line_element, list);
	pos_last = wall->position;
	distance = calc_distance(pos_last.x, pos_last.y, mouse_mapcoord.x, mouse_mapcoord.y);

	// Calculate the difference of position since last time
	offset.x = pos_last.x - mouse_mapcoord.x;
	offset.y = pos_last.y - mouse_mapcoord.y;

	// Then we want to find out in which direction the mouse has moved
	// since the last time, and compute the distance relatively to the axis
	if (fabsf(offset.y) > fabsf(offset.x)) {
		if (offset.y > 0) {
			actual_direction = NORTH;
		} else {
			actual_direction = SOUTH;
		}
		distance = fabsf(mouse_mapcoord.y - pos_last.y);
	} else {
		if (offset.x > 0) {
			actual_direction = WEST;
		} else {
			actual_direction = EAST;
		}
		distance = fabsf(mouse_mapcoord.x - pos_last.x);
	}

	// Are we going in a direction possible with that wall?

	if (obstacle_map[state.l_id].flags & IS_HORIZONTAL) {
		direction_is_possible = (actual_direction == WEST) || (actual_direction == EAST);
	} else if (obstacle_map[state.l_id].flags & IS_VERTICAL) {
		direction_is_possible = (actual_direction == NORTH) || (actual_direction == SOUTH);
	} else {
		direction_is_possible = FALSE;
	}

	// If the mouse is far away enough
	if ((distance > 1) && (direction_is_possible) && ((state.l_direction == actual_direction) || (state.l_direction == UNDEFINED))) {
		wall = malloc(sizeof(line_element));

		// Then we calculate the position of the next wall
		wall->position = pos_last;
		switch (actual_direction) {
		case NORTH:
			wall->position.y--;
			break;
		case SOUTH:
			wall->position.y++;
			break;
		case EAST:
			wall->position.x++;
			break;
		case WEST:
			wall->position.x--;
			break;
		default:
			break;
		}
		// And add the wall, to the linked list and to the map
		//

		list_add_tail(&(wall->list), &(state.l_elements.list));
		wall->address = action_create_obstacle_user(EditLevel(), wall->position.x, wall->position.y, state.l_id);

		// If the direction is unknown (ie. we only have one wall), 
		// let's define it
		if (state.l_direction == UNDEFINED) {
			state.l_direction = actual_direction;
		}
	}
	if ((state.l_direction == (-actual_direction)) && (!list_empty(&(state.l_elements.list)))) {
		// Looks like the user wants to go back, so let's remove the line wall
		wall = list_entry(state.l_elements.list.prev, line_element, list);
		action_remove_obstacle_user(EditLevel(), wall->address);
		list_del(state.l_elements.list.prev);
		free(wall);
		if (list_empty(&(state.l_elements.list))) {
			state.l_direction = UNDEFINED;
		}
	}
}

static void end_line_walls(int commit)
{
	line_element *tmp;
	int list_length = 1;

	our_mode = DISABLED;

	// Remove the linked list
	while (!(list_empty(&(state.l_elements.list)))) {
		tmp = list_entry(state.l_elements.list.prev, line_element, list);
		free(tmp);
		if (!commit)
			action_remove_obstacle(EditLevel(), state.l_elements.address);
		list_del(state.l_elements.list.prev);
		list_length++;
	}
	if (!commit)
		action_remove_obstacle(EditLevel(), state.l_elements.address);

	// Remove the sentinel
	list_del(&(state.l_elements.list));

	if (commit)
		action_push(ACT_MULTIPLE_ACTIONS, list_length);

}

int leveleditor_place_input(SDL_Event * event)
{
	struct leveleditor_typeselect *ts;
	ts = get_current_object_type();

	if(!mouse_in_level)
		return 0;

	if (our_mode == DISABLED) {
		// Try to place something
		if (EVENT_LEFT_PRESS(event)) {
			switch (ts->type) {
			case OBJECT_FLOOR:
				start_rectangle_floor(ts->indices[ts->selected_tile_nb]);
				return 0;
			case OBJECT_OBSTACLE:
				//Check whether to do line mode or single obstacle placement
				if (obstacle_map[ts->indices[ts->selected_tile_nb]].flags & (IS_VERTICAL | IS_HORIZONTAL)) {
					start_line_walls(ts->indices[ts->selected_tile_nb]);
					return 0;
				} else {
					place_single_obstacle(ts);
					return 1;
				}
				break;
			case OBJECT_WAYPOINT:
				return do_waypoint_route(ts->indices[ts->selected_tile_nb]);
				break;
			default:
				GiveMouseAlertWindow("Place tool does not support this type of object.");
			}

		}
	} else if (our_mode == RECTANGLE_FLOOR) {
		if (EVENT_LEFT_RELEASE(event)) {
			end_rectangle_floor(1);	//commit the modification
			return 1;
		} else if (EVENT_MOVE(event)) {
			//change rectangle size
			handle_rectangle_floor();
		} else if (EVENT_RIGHT_PRESS(event) || EVENT_KEYPRESS(event, SDLK_SPACE)) {
			end_rectangle_floor(0);
			return 1;
		}
	} else if (our_mode == LINE_WALLS) {
		if (EVENT_LEFT_RELEASE(event)) {
			end_line_walls(1);
			return 1;
		} else if (EVENT_MOVE(event)) {
			handle_line_walls();
		} else if (EVENT_RIGHT_PRESS(event) || EVENT_KEYPRESS(event, SDLK_ESCAPE)) {
			end_line_walls(0);
			return 1;
		}
	} else if (our_mode == CONNECT_WAYPOINT) {
		if (ts->type != OBJECT_WAYPOINT) {
			end_waypoint_route();
			return 1;
		}
		if (EVENT_LEFT_PRESS(event)) {
			return do_waypoint_route(ts->indices[ts->selected_tile_nb]);
		}
	}

	return 0;
}

int leveleditor_place_display()
{
	if (our_mode == CONNECT_WAYPOINT) {
		draw_connection_between_tiles(EditLevel()->AllWaypoints[state.startwp].x + 0.5,
					      EditLevel()->AllWaypoints[state.startwp].y + 0.5, mouse_mapcoord.x, mouse_mapcoord.y,
					      GameConfig.zoom_is_on ? ZOOM_OUT : 0);
	}

	return 0;
}
