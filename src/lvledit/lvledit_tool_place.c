
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
	int l_orientation;
	int l_id; /* Selected wall */
	int l_type_h; /* Horizontal wall */
	int l_type_v; /* Vertical wall */
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

enum obstacle_direction {
	WE, // Horizontal
	SN, // Vertical
	
	// Corners
	NW,
	SW,
	NE,
	SE
};

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

/*
 * Pick type of walls (vertical and horizontal).
 * This function is temporary, because it's terrible hack ...
 */
static void pick_walls(int windex)
{
	switch (windex) {
	case ISO_BRICK_WALL_CABLES_V:
	case ISO_BRICK_WALL_CABLES_H:
		state.l_type_v = ISO_BRICK_WALL_CABLES_V;
		state.l_type_h = ISO_BRICK_WALL_CABLES_H;
		break;
	case ISO_THICK_WALL_V:
	case ISO_THICK_WALL_H:
		state.l_type_v = ISO_THICK_WALL_V;
		state.l_type_h = ISO_THICK_WALL_H;
		break;
	case ISO_V_DOOR_LOCKED:
	case ISO_H_DOOR_LOCKED:
		state.l_type_v = ISO_V_DOOR_LOCKED;
		state.l_type_h = ISO_H_DOOR_LOCKED;
		break;
	case ISO_V_DOOR_000_OPEN:
	case ISO_H_DOOR_000_OPEN:
		state.l_type_v = ISO_V_DOOR_000_OPEN;
		state.l_type_h = ISO_H_DOOR_000_OPEN;
		break;
	case ISO_OUTER_WALL_W1:
	case ISO_OUTER_WALL_N1:
		state.l_type_v = ISO_OUTER_WALL_W1;
		state.l_type_h = ISO_OUTER_WALL_N1;
		break;
	case ISO_OUTER_WALL_W2:
	case ISO_OUTER_WALL_N2:
		state.l_type_v = ISO_OUTER_WALL_W2;
		state.l_type_h = ISO_OUTER_WALL_N2;
		break;
	case ISO_OUTER_WALL_W3:
	case ISO_OUTER_WALL_N3:
		state.l_type_v = ISO_OUTER_WALL_W3;
		state.l_type_h = ISO_OUTER_WALL_N3;
		break;
	case ISO_OUTER_WALL_E1:
	case ISO_OUTER_WALL_S1:
		state.l_type_v = ISO_OUTER_WALL_E1;
		state.l_type_h = ISO_OUTER_WALL_S1;
		break;
	case ISO_OUTER_WALL_E2:
	case ISO_OUTER_WALL_S2:
		state.l_type_v = ISO_OUTER_WALL_E2;
		state.l_type_h = ISO_OUTER_WALL_S2;
		break;
	case ISO_OUTER_WALL_E3:
	case ISO_OUTER_WALL_S3:
		state.l_type_v = ISO_OUTER_WALL_E3;
		state.l_type_h = ISO_OUTER_WALL_S3;
		break;
	case ISO_CAVE_WALL_V:
	case ISO_CAVE_WALL_H:
		state.l_type_v = ISO_CAVE_WALL_V;
		state.l_type_h = ISO_CAVE_WALL_H;
		break;
	default:
		if (obstacle_map[windex].flags & IS_HORIZONTAL) {
			state.l_type_h = windex;
			state.l_type_v = windex - 1;
		}
		else {
			state.l_type_v = windex;
			state.l_type_h = windex + 1;
		}
	}
}

static int horizontal_wall(int type)
{
	return (obstacle_map[type].flags & IS_HORIZONTAL);
}

static int vertical_wall(int type)
{
	return (obstacle_map[type].flags & IS_VERTICAL);
}

static void create_wall_tile(float x, float y, enum _level_editor_directions orientation)
{
	line_element *wall = malloc(sizeof(line_element));

	// Set the position of wall
	wall->position.x = x;
	wall->position.y = y;

	// Set the new orientation of wall line
	state.l_orientation = orientation;

	// Add the wall in the linked list and on the map
	list_add_tail(&(wall->list), &(state.l_elements.list));
	wall->address = action_create_obstacle_user(EditLevel(), wall->position.x, wall->position.y, state.l_id);
}

static void wall_line_change_orientation(moderately_finepoint new_pos, moderately_finepoint last_pos, int type)
{
	// Set the new type of wall
	state.l_id = type;

	if (vertical_wall(state.l_id)) {
		// The new direction of wall line is vertical
		state.l_direction = SN;

		// When the direction is vertical, we must define the new
		// orientation on the vertical axis
		state.l_orientation = (new_pos.y > last_pos.y) ? NORTH : SOUTH;
	}
	else {	
		// The new direction of wall line is horizontal
		state.l_direction = WE;

		// When the direction is horizontal, we must define the new
		// orientation on the horizontal axis
		state.l_orientation = (new_pos.x > last_pos.x) ? WEST : EAST;
	}
}

static void start_line_walls(int windex)
{
	our_mode = LINE_WALLS;

	// Initialize a line
	INIT_LIST_HEAD(&(state.l_elements.list));

	state.l_orientation = UNDEFINED;

	state.l_id = windex;

	// Before placing a first wall section, we must pick the horizontal and vertical type
	pick_walls(windex);

	// Before placing a first wall, we must define the direction of wall line
	state.l_direction = (horizontal_wall(state.l_id)) ? WE: SN;

	state.l_elements.position.x = (int)mouse_mapcoord.x + (horizontal_wall(state.l_id) ? 0.5 : 0);

	state.l_elements.position.y = (int)mouse_mapcoord.y + (horizontal_wall(state.l_id) ? 0 : 0.5);

	state.l_elements.address = action_create_obstacle_user(EditLevel(),
							       state.l_elements.position.x, state.l_elements.position.y, state.l_id);
}

static void handle_line_walls()
{
	line_element *last_wall;			// The last wall of the line
	moderately_finepoint offset;		// Difference of position since last time
	moderately_finepoint new_pos;		// The position of the new wall
	moderately_finepoint last_pos;	// Position of the previous tile
	int new_orientation;					// New orientation of the wall line
	int new_direction;					// New direction of the wall line
	float distance;						// Difference of distance since last time

	// Get the last wall of the line
	last_wall = list_entry((state.l_elements).list.prev, line_element, list);
	last_pos.x = last_wall->position.x;
	last_pos.y = last_wall->position.y;
	
	// Calculate the difference of position since last time
	offset.x = last_pos.x - mouse_mapcoord.x;
	offset.y = last_pos.y - mouse_mapcoord.y;

	// Then we want to find out in which direction the mouse has moved
	// since the last time, and compute the distance relatively to the axis
	if (fabsf(offset.y) > fabsf(offset.x)) {
		// The new direction is vertical
		new_direction = SN;

		// Then we define the new orientation on the vertical axis
		new_orientation = (offset.y > 0) ? NORTH : SOUTH;

		// Calculate the vertical displacement
		distance = fabsf(mouse_mapcoord.y - last_wall->position.y);
	} else {
		// The new direction is horizontal
		new_direction = WE;

		// Then we define the new orientation on the horizontal axis
		new_orientation = (offset.x > 0) ? WEST : EAST;

		// Calculate the horizontal displacement
		distance = fabsf(mouse_mapcoord.x - last_wall->position.x);
	}

	if (distance < 1) {
		// When the displacement since last time is less than the width of a tile,
		// we must not create or remove a wall
		return;
	}

	if (state.l_direction == new_direction) {
		if (state.l_orientation == new_orientation || state.l_orientation == UNDEFINED) {
			// The user want to create a straight wall line, we must create a new 
			// wall in the same direction than the last wall

			// Initialize the position of the new wall
			new_pos.x = last_pos.x;
			new_pos.y = last_pos.y;

			// Then we calculate the new position
			switch (new_orientation) {
			case NORTH:
					new_pos.y--;
					break;
			case SOUTH:
					new_pos.y++;
					break;
			case EAST:
					new_pos.x++;
					break;
			case WEST:
					new_pos.x--;
					break;
			default:
					break;
			}

			// Add the new wall on the map and change the orientation of wall line
			create_wall_tile(new_pos.x, new_pos.y, new_orientation);
		} else if (state.l_orientation == (-new_orientation) && (!list_empty(&(state.l_elements.list)))) {
			// Looks like the user wants to go back, so let's remove the line of walls

			// Get the type of the last wall in order to say if two last walls form 
			// a perpendicular corner
			int last_wall_type = last_wall->address->type;

			// Remove the last wall
			action_remove_obstacle_user(EditLevel(), last_wall->address);
			list_del(state.l_elements.list.prev);
			free(last_wall);

			// Get the previous wall
			line_element *prev_wall = list_entry(state.l_elements.list.prev, line_element, list);

			if (horizontal_wall(last_wall_type) && vertical_wall(prev_wall->address->type)) {
				// When the last removed wall is horizontal and the previous is vertical,
				// these two walls form a perpendicular corner, we must change the
				// orientation of wall line in order to go back
				wall_line_change_orientation(prev_wall->position, last_pos, state.l_type_v);
			}

			if (vertical_wall(last_wall_type) && horizontal_wall(prev_wall->address->type)) {
				// When the last removed wall is vertical and the previous is horizontal,
				// these two walls form a perpendicular corner, we must change the 
				// orientation of wall line in order to go back
				wall_line_change_orientation(prev_wall->position, last_pos, state.l_type_h);
			}

			if (list_empty(&(state.l_elements.list)))
				state.l_orientation = UNDEFINED;
		}
	} else {
		// The user doesn't want to create a straight wall line, therefore the
		// direction of wall line will be changed, and we must create a new wall
		// perpendicular to the last wall

		// Set the new direction of wall line
		state.l_direction = new_direction;

		// Define the type (ie. horizontal or vertical) of the new wall
		state.l_id = (horizontal_wall(state.l_id)) ? state.l_type_v : state.l_type_h;

		// Calculate the orientation of the new wall
		int xsign = (offset.x > 0) ? -1 : 1;
		int ysign = (offset.y > 0) ? -1 : 1;

		// Then we calculate the new position
		new_pos.x = last_pos.x + xsign * 0.5;
		new_pos.y = last_pos.y + ysign * 0.5;

		// Add the new wall on the map and change the orientation of wall line
		create_wall_tile(new_pos.x, new_pos.y, new_orientation);
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
				alert_window("Place tool does not support this type of object.");
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
