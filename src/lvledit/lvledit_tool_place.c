
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

#define leveleditor_tool_place_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

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
	struct {
		int h;
		int v;
		int se;
		int sw;
		int ne;
		int nw;
	} l_type;	
	struct list_head l_elements_head;

	/* Rectangle mode */
	point r_start;
	int r_len_x, r_len_y;
	int r_step_x, r_step_y;
	int r_tile_used;

	/* Waypoint mode */
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
	int isnew = 0;

	wpnum = get_waypoint(EditLevel(), (int)mouse_mapcoord.x, (int)mouse_mapcoord.y);
	if (wpnum < 0) {
		// When the waypoint doesn't exists at the map position, we want to create it
		wpnum = add_waypoint(EditLevel(), (int)mouse_mapcoord.x, (int)mouse_mapcoord.y, rspawn);
		isnew = 1;
	}

	if (our_mode != CONNECT_WAYPOINT) {
		//we are starting a new route
		state.startwp = -1;
		state.nbactions = 0;
	}
	if (isnew) {
		waypoint *wpts = EditLevel()->waypoints.arr;
		action_push(ACT_REMOVE_WAYPOINT, wpts[wpnum].x, wpts[wpnum].y);
		state.nbactions++;
	}

	if (state.startwp != -1 && wpnum != state.startwp) {
		// Connect to the previous waypoint
		if (action_toggle_waypoint_connection(EditLevel(), state.startwp, wpnum, 0, 1) == 1)
			state.nbactions++;
		if (action_toggle_waypoint_connection(EditLevel(), wpnum, state.startwp, 0, 1) == 1)
			state.nbactions++;
	}

	waypoint *wpts = EditLevel()->waypoints.arr;
	wpts[wpnum].suppress_random_spawn = rspawn;

	if (wpnum == state.startwp) {
		// Click on the same waypoint ? end the route
		end_waypoint_route();
		return 1;
	}

	if (our_mode != CONNECT_WAYPOINT) {
		our_mode = CONNECT_WAYPOINT;
	}

	state.startwp = wpnum;
	return 0;
}

/**
 * Place an obstacle on the map
 * \param type The type of the obstacle
 */
static void place_single_obstacle(int type)
{
	// Create a new obstacle at the position of the mouse
	action_create_obstacle_user(EditLevel(), mouse_mapcoord.x, mouse_mapcoord.y, type);
}

/**
 * Place an item on the map
 * \param type The type of the item
 */
static void place_item(int type)
{
	// Create an item on the map at the position of the mouse
	action_create_item(EditLevel(), mouse_mapcoord.x, mouse_mapcoord.y, type);
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

}

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

/**
 * Place an enemy on the map
 * @param droid_type Type of droid to pe placed
 */
void place_enemy(int droid_type)
{
	gps pos = { mouse_mapcoord.x, mouse_mapcoord.y, EditLevel()->levelnum };
	enemy *en = create_special_force(pos, droid_type);
	action_create_enemy(EditLevel(), en);
}

/**
 * Place a map label on the map.
 */
static void place_map_label()
{
	level_editor_action_change_map_label_user(EditLevel(), mouse_mapcoord.x, mouse_mapcoord.y);
}

/*
 * Pick type of walls (vertical and horizontal).
 */
static void pick_walls(int windex)
{
	int i;
	struct obstacle_spec *spec = get_obstacle_spec(windex);
	struct obstacle_group *group = find_obstacle_group(windex);

	state.l_type.nw = state.l_type.ne = -1;
	state.l_type.sw = state.l_type.se = -1;
	state.l_type.h = state.l_type.v = -1;

	// If obstacle doesn't belong to any group it's assumed that
	// it has only vertical and horizontal wall types. In this case,
	// horizontal and vertical obstacle types should be subsequent
	// obstacle types.
	if (!group) {
		if (spec->flags & IS_HORIZONTAL) {
			state.l_type.h = windex;
			state.l_type.v = windex - 1;
		} else {
			state.l_type.v = windex;
			state.l_type.h = windex + 1;
		}
		return;
	}

	// Find corners, horizontal and vertical wall types in the obstacle
	// group.
	for (i = 0; i < group->members.size; i++) {
		int member = ((int *)(group->members.arr))[i];
		unsigned int member_flags = get_obstacle_spec(member)->flags;
		if (member_flags & CORNER_NW)
			state.l_type.nw = member;
		else if (member_flags & CORNER_NE)
			state.l_type.ne = member;
		else if (member_flags & CORNER_SE)
			state.l_type.se = member;
		else if (member_flags & CORNER_SW)
			state.l_type.sw = member;
		else if (member_flags & IS_HORIZONTAL)
			state.l_type.h = member;
		else if (member_flags & IS_VERTICAL)
			state.l_type.v = member;
	}
}

/**
 * Returns TRUE if the wall is horizontal
 * \param type The type of the wall
 */
static int horizontal_wall(int type)
{
	return (get_obstacle_spec(type)->flags & IS_HORIZONTAL);
}

/**
 * Returns TRUE if the wall is vertical
 * \param type The type of the wall
 */
static int vertical_wall(int type)
{
	return (get_obstacle_spec(type)->flags & IS_VERTICAL);
}

/**
 * Returns TRUE if the currently selected wall style has corners
 */
static int has_corners()
{
	if (state.l_type.nw != -1)
		return 1;
	return 0;
}

/**
 * Add a wall and change the direction of the line of walls
 * \param x The x position of the wall
 * \param y The y position of the wall
 * \param type The type of the wall
 * \param direction The direction of the wall
 */
static void add_wall(float x, float y, int type, enum _level_editor_directions direction)
{
	line_element *wall;

	// Keep wall elements in a linked list so as to be able to remove them
	// if the users goes backwards
	wall = malloc(sizeof(line_element));

	// Set the position of the new wall
	wall->position.x = x;
	wall->position.y = y;

	// Create the new wall on the map
	wall->address = add_obstacle(EditLevel(), x, y, type);

	if (wall->address) {
		// Add the new wall in the linked list
		list_add_tail(&(wall->list), &(state.l_elements_head));
		// Define the new direction of the line of walls
		state.l_direction = direction;
	} else {
		free(wall);
	}
	// cppcheck-suppress memleak
}

/**
 * Start freehand line drawing
 * \param windex The type of the wall
 */
static void start_wall_line(int windex)
{
	our_mode = LINE_WALLS;

	// Initialize the linked list
	INIT_LIST_HEAD(&(state.l_elements_head));

	// Before placing the first wall section, we must pick the types of the
	// selected wall
	pick_walls(windex);

	// Calculate the position of the first wall, it is placed at
	// the edge of the nearest square, where the mouse is located
	float x = (int)mouse_mapcoord.x + (horizontal_wall(windex) ? 0.5 : 0);
	float y = (int)mouse_mapcoord.y + (horizontal_wall(windex) ? 0 : 0.5);

	// Find the type of the first wall
	int type = (horizontal_wall(windex)) ? state.l_type.h : state.l_type.v;

	// Add a wall on the map and do not define the direction because it's the 
	// first wall of the line
	add_wall(x, y, type, UNDEFINED);
}

/**
 * The user is moving backwards (ie. remove the last wall in the line)
 */
static void line_moving_backwards(void)
{
	line_element *last_wall;			// The last wall of the line
	int last_type;							// The type of the last wall
	moderately_finepoint last_pos;	// The position of the last wall
	moderately_finepoint offset;		// Difference of position

	// Get the last wall in the line
	last_wall = list_entry(state.l_elements_head.prev, line_element, list);

	last_type = last_wall->address->type;
	last_pos = last_wall->position;

	// Remove the last wall
	del_obstacle(last_wall->address);
	list_del(&last_wall->list);
	free(last_wall);

	if (list_empty(&(state.l_elements_head))) {
		// In case we've removed the only element in the list, 
		// we want to put it back in order to make sure the "base" position
		// of the line does not change.

		add_wall(last_pos.x, last_pos.y, last_type, UNDEFINED);

		// Nothing more to do
		return;
	}

	// Get the previous (last but one) wall
	line_element *prev_wall = list_entry(state.l_elements_head.prev, line_element, list);

	// Calculate the offset between the two last walls
	offset.x = prev_wall->position.x - last_pos.x;
	offset.y = prev_wall->position.y - last_pos.y;

	// After removing the last wall in the line, we must find the new direction
	// of the line of walls
	if (fabsf(offset.y) > fabsf(offset.x)) {
		// Find the new direction on the vertical axis
		state.l_direction = (offset.y > 0) ? NORTH : SOUTH;
	} else if (fabsf(offset.y) < fabsf(offset.x)) {
		// Find the new direction on the horizontal axis
		state.l_direction = (offset.x > 0) ? WEST : EAST;		
	} else {
		// We have to find a right angle between the two last walls of the line 
		if (state.l_direction == WEST || state.l_direction == EAST) {
			// Find the new direction on the vertical axis
			state.l_direction = (last_pos.y < prev_wall->position.y) ? NORTH : SOUTH;
		} else {
			// Find the new direction on the horizontal axis
			state.l_direction = (last_pos.x < prev_wall->position.x) ? WEST : EAST;
		}
	}
}

/**
 * The user is moving forwards (ie. create a new wall in the line)
 * \param offset The difference of position since last time
 */
static void line_moving_forwards(moderately_finepoint offset)
{
	line_element *last_wall;	// The last wall of the line
	int new_direction;			// New direction of the wall line
	float distance;				// Difference of distance since last time
	int type = 0;						// The type of the new wall

	// Get the last wall in the line of walls
	last_wall = list_entry(state.l_elements_head.prev, line_element, list);

	// We want to find out in which direction the mouse has moved
	// since the last time, and compute the distance relatively to the axis
	if (fabsf(offset.y) > fabsf(offset.x)) {
		// Find the new direction on the vertical axis
		new_direction = (offset.y > 0) ? NORTH : SOUTH;

		// Calculate the vertical displacement
		distance = fabsf(offset.y);
	} else {
		// Find the new direction on the horizontal axis
		new_direction = (offset.x > 0) ? WEST : EAST;

		// Calculate the horizontal displacement
		distance = fabsf(offset.x);
	}

	if (distance < 1) {
		// When the displacement since last time is less than the length of a tile,
		// do not create a new wall
		return;
	}

	if (state.l_direction == UNDEFINED) {
		// The user wants to create a line of walls for the first time, we must find
		// if he wants to create a straight line or create a first corner in the line

		// When the current selected wall and the new direction of the line are
		// horizontal/vertical, the user wants to create a straight line, define
		// the direction of this line as the new direction
		if ((horizontal_wall(last_wall->address->type) && (new_direction == WEST || new_direction == EAST))
			|| (vertical_wall(last_wall->address->type) && (new_direction == SOUTH || new_direction == NORTH))) {
			// The user wants to create a straight line
			state.l_direction = new_direction;
		}
	}

	// Initialize the position of the new wall
	float x = last_wall->position.x;
	float y = last_wall->position.y;

	// Find the step of displacement in order to calculate the new position
	int step_x = (offset.x > 0) ? -1 : 1;
	int step_y = (offset.y > 0) ? -1 : 1;

	if (state.l_direction == new_direction) {
		// The user wants to create a straight wall line, create a new wall in the
		// same direction as the last wall

		// Calculate the new position on the horizontal/vertical axis when
		// the new direction is horizontal/vertical and find the type of the new
		// wall
		if (new_direction == WEST || new_direction == EAST) {
			x += step_x;
			type = state.l_type.h;
		} else {
			y += step_y;
			type = state.l_type.v;
		}
	} else {
		// The user doesn't want to create a straight wall line, therefore the
		// direction and the type of wall line will be changed, and we must create
		// a new wall perpendicular to the last wall

		if (has_corners()) {
			// When the style of the current selected wall has corners, find the
			// type (ie. NW, SW, NE, SE) of the new wall and calculate the position

			// Find the type of the corner and calculate the position
			switch (new_direction) {
				case WEST:
					type = (offset.y < 0) ? state.l_type.se : state.l_type.ne;
					y += step_y;
					break;
				case EAST:
					type = (offset.y < 0) ? state.l_type.sw : state.l_type.nw;
					y += step_y;
					break;
				case SOUTH:
					type = (offset.x < 0) ? state.l_type.ne : state.l_type.nw;
					x += step_x;
					break;
				case NORTH:
					type = (offset.x < 0) ? state.l_type.se : state.l_type.sw;
					x += step_x;
					break;
			}
		} else {
			// When the style of the current selected wall have not corners,
			// find the type (ie. horizontal/vertical) and calculate the position

			// Find the type (ie. horizontal or vertical) of the new wall
			type = (horizontal_wall(last_wall->address->type)) ? state.l_type.v : state.l_type.h;

			// Calculate the position of the new wall
			x += step_x * 0.5;
			y += step_y * 0.5;
		}
	}

	// Add the new wall in the line of walls and change the direction
	add_wall(x, y, type, new_direction);
}

/**
 * Create the freehand line drawing
 */
static void handle_wall_line()
{
	line_element *last_wall;			// The last wall of the line
	moderately_finepoint offset;		// Difference of position since last time
	int moving_backwards = FALSE;

	// Get the last wall in the line of walls
	last_wall = list_entry(state.l_elements_head.prev, line_element, list);

	// Calculate the position offset since last time
	offset.x = last_wall->position.x - mouse_mapcoord.x;
	offset.y = last_wall->position.y - mouse_mapcoord.y;

	// Check if the user wants to go backwards
	switch (state.l_direction) {
		case NORTH:
			if (offset.y < 0)
				moving_backwards = TRUE;
			break;
		case SOUTH:
			if (offset.y > 0)
				moving_backwards = TRUE;
			break;
		case WEST:
			if (offset.x < 0)
				moving_backwards = TRUE;
			break;
		case EAST:
			if (offset.x > 0)
				moving_backwards = TRUE;
			break;
	}

	if (moving_backwards) {
		// The user is moving backwards, we must delete the previous wall in the line
		line_moving_backwards();
	} else {
		// The user is moving forwards, we must create a new wall in the line
		line_moving_forwards(offset);
	}
}

/**
 * Stop the freehand line drawing
 * \param commit FALSE if the user wants cancel the line
 */
static void end_wall_line(int commit)
{
	our_mode = DISABLED;

	line_element *e, *ne;
	int nb_actions = 0;

	// Remove the linked list
	list_for_each_entry_safe(e, ne, &state.l_elements_head, list) {
		if (!commit) {
			// When the user wants cancel the line of walls, we must remove all
			// the walls
			del_obstacle(e->address);
		} else {
			/* Add all the actions needed to be able to delete
			 * the entire line of walls. */
			action_push(ACT_REMOVE_OBSTACLE, e->address);
		}
		nb_actions++;

		list_del(&e->list);
		free(e);
	}

	if (commit)
		action_push(ACT_MULTIPLE_ACTIONS, nb_actions);
}

int leveleditor_place_input(SDL_Event *event)
{
	struct widget_lvledit_categoryselect *cs = get_current_object_type();
	int type = cs->indices[cs->selected_tile_nb];

	if (!mouse_in_level) {
		// We must not place objects outside of the level
		// And we stop drawing walls if mouse leaved the level
		if (our_mode == LINE_WALLS) {
			end_wall_line(1);
		}
		return 0;
	}

	if (our_mode != DISABLED) {
		if (EVENT_RIGHT_PRESS(event) || EVENT_KEYPRESS(event, SDLK_ESCAPE)) {
			// Cancel the current operation
			leveleditor_place_reset();
			return 1;
		}
	}

	if (our_mode == DISABLED) {
		if (EVENT_LEFT_PRESS(event)) {
			switch (cs->type) {
			case OBJECT_FLOOR:
				start_rectangle_floor(type);
				return 0;
			case OBJECT_OBSTACLE:
				if (horizontal_wall(type) || vertical_wall(type)) {
					start_wall_line(type);
					return 0;
				} else {
					place_single_obstacle(type);
					return 1;
				}
				break;
			case OBJECT_WAYPOINT:
				return do_waypoint_route(type);
				break;
			case OBJECT_ITEM:
				place_item(type);
				return 1;
				break;
			case OBJECT_ENEMY:
				place_enemy(type);
				return 1;
				break;
			case OBJECT_MAP_LABEL:
				place_map_label();
				return 1;
				break;
			default:
				alert_window(_("Place tool does not support this type of object."));
			}
		}
	} else if (our_mode == RECTANGLE_FLOOR) {
		if (EVENT_LEFT_RELEASE(event)) {
			end_rectangle_floor(1);
			return 1;
		} else if (EVENT_MOVE(event)) {
			handle_rectangle_floor();
		}
	} else if (our_mode == LINE_WALLS) {
		if (EVENT_LEFT_RELEASE(event)) {
			end_wall_line(1);
			return 1;
		} else if (EVENT_MOVE(event)) {
			handle_wall_line();
		}
	} else if (our_mode == CONNECT_WAYPOINT) {
		if (EVENT_LEFT_PRESS(event)) {
			return do_waypoint_route(type);
		}
	}

	return 0;
}

int leveleditor_place_display()
{
	if (our_mode == CONNECT_WAYPOINT) {
		waypoint *wpts = EditLevel()->waypoints.arr;

		draw_connection_between_tiles(wpts[state.startwp].x + 0.5, wpts[state.startwp].y + 0.5, mouse_mapcoord.x, mouse_mapcoord.y,
					      GameConfig.zoom_is_on ? ZOOM_OUT : 0, wpts[state.startwp].suppress_random_spawn);
	}

	return 0;
}

void leveleditor_place_reset()
{
	switch (our_mode) {
	case LINE_WALLS:
			end_wall_line(0);
			break;
	case RECTANGLE_FLOOR:
			end_rectangle_floor(0);
			break;
	case CONNECT_WAYPOINT:
			end_waypoint_route();
			break;
	default:
			break;
	}
	OriginWaypoint = -1;
}

void leveleditor_place_reset_waypoint_route(int wpnum)
{
	if (our_mode == CONNECT_WAYPOINT && state.startwp == wpnum) {
		end_waypoint_route();
	}
	OriginWaypoint = -1;
}
