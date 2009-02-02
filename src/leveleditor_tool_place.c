

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

#include "leveleditor.h"
#include "leveleditor_actions.h"

#include "leveleditor_widgets.h"
#include "leveleditor_widget_toolbar.h"
#include "leveleditor_widget_typeselect.h"
#include "leveleditor_widget_map.h"
#include "leveleditor_tools.h"

static enum {
    DISABLED,
	RECTANGLE_FLOOR,
	LINE_WALLS,
} our_mode;

static void place_single_obstacle(struct leveleditor_typeselect *ts)
{
    point pos;
    pos.x = (int)mouse_mapcoord.x;
    pos.y = (int)mouse_mapcoord.y;
    
    action_create_obstacle_user (EditLevel(), pos.x, pos.y, ts->indices[ts->selected_tile_nb]);
    //  quickbar_use ( GameConfig . level_editor_edit_mode, selected_tile_nb );
}

static void start_rectangle_floor(struct leveleditor_place *m, int findex)
{
    our_mode = RECTANGLE_FLOOR;

    // Starting values
    m->r_start.x = (int)mouse_mapcoord.x;
    m->r_start.y = (int)mouse_mapcoord.y;
    m->r_len_x = 0;
    m->r_len_y = 0;

    // The tile we'll use 
    m->r_tile_used = findex;

    // Place the first tile 
    action_set_floor (EditLevel(), m->r_start.x, m->r_start.y, m->r_tile_used );
    action_push (ACT_MULTIPLE_FLOOR_SETS, 1);

} // void start_rectangle_mode ( leveleditor_state cur_state , int already_defined )

static void handle_rectangle_floor (struct leveleditor_place *m)
{
    int i, j;
    int changed_tiles = 0;
    // If there is something to change
    if (calc_euklid_distance(mouse_mapcoord.x, mouse_mapcoord.y,
		m->r_start.x + m->r_len_x,
		m->r_start.y + m->r_len_y) > 0.5)
	{
	// Redefine the rectangle dimensions
	m->r_len_x = (int)mouse_mapcoord.x - m->r_start.x; 
	m->r_step_x = (m->r_len_x > 0 ? 1 : -1);
	m->r_len_y = (int)mouse_mapcoord.y - m->r_start.y;
	m->r_step_y = (m->r_len_y > 0 ? 1 : -1);

	// Undo previous rectangle
	level_editor_action_undo ();

	// Then redo a correct one
	for (i = m->r_start.x;
		i != m->r_start.x + m->r_len_x + m->r_step_x;
		i += m->r_step_x)
	    {
	    for (j = m->r_start.y;
		    j != m->r_start.y + m->r_len_y + m->r_step_y;
		    j += m->r_step_y)
		{
		action_set_floor ( EditLevel(), i, j, m->r_tile_used );
		changed_tiles++;
		}
	    }
	action_push ( ACT_MULTIPLE_FLOOR_SETS, changed_tiles);
	}
}

static void end_rectangle_floor(int commit)
{
    our_mode = DISABLED;
    if (!commit)
	level_editor_action_undo ();
}

static void start_line_walls(struct leveleditor_place *m, int windex)
{
    our_mode = LINE_WALLS;

    // Initialize a line
    INIT_LIST_HEAD(&(m->l_elements.list));

    m->l_direction = UNDEFINED;

    m->l_id = windex;

    m->l_elements.position.x = (int)mouse_mapcoord.x +
	((obstacle_map[m->l_id].flags & IS_HORIZONTAL) ? 0.5 : 0);

    m->l_elements.position.y = (int)mouse_mapcoord.y + 
	((obstacle_map[m->l_id].flags & IS_HORIZONTAL) ? 0 : 0.5);

    m->l_elements.address = action_create_obstacle_user (EditLevel(), 
	    m->l_elements.position.x, m->l_elements.position.y, 
	    m->l_id);

}

static void handle_line_walls(struct leveleditor_place *m)
{ 
    line_element *wall;
    int actual_direction;
    float distance;
    moderately_finepoint pos_last;
    moderately_finepoint offset;
    int direction_is_possible;

    wall = list_entry((m->l_elements).list.prev, line_element, list);
    pos_last = wall->position;
    distance = calc_euklid_distance(pos_last.x, pos_last.y ,
	    mouse_mapcoord.x,
	    mouse_mapcoord.y );

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

    if (obstacle_map[m->l_id].flags & IS_HORIZONTAL) {
	direction_is_possible =  (actual_direction == WEST) || (actual_direction == EAST);
    } else if ( obstacle_map[m->l_id].flags & IS_VERTICAL ) {
	direction_is_possible = (actual_direction == NORTH) || (actual_direction == SOUTH);
    } else {
	direction_is_possible = FALSE;
    }

    // If the mouse is far away enough
    if ((distance > 1) && (direction_is_possible) &&
	    ((m->l_direction == actual_direction) || 
	     (m->l_direction == UNDEFINED))) {
	wall = malloc(sizeof(line_element));

	// Then we calculate the position of the next wall
	wall->position = pos_last;
	switch(actual_direction) {
	    case NORTH:
		wall->position.y --;
		break;
	    case SOUTH:
		wall->position.y ++; 
		break;
	    case EAST:
		wall->position.x ++;
		break;
	    case WEST:
		wall->position.x --;
		break;
	    default:
		break;
	}
	// And add the wall, to the linked list and to the map
	//

	list_add_tail(&(wall->list), &(m->l_elements.list));
	wall->address = action_create_obstacle_user (EditLevel(),
		wall->position.x, wall->position.y, m->l_id);

	// If the direction is unknown (ie. we only have one wall), 
	// let's define it
	if (m->l_direction == UNDEFINED) {
	    m->l_direction = actual_direction;
	}
    }
    if ( (m->l_direction == (- actual_direction)) && 
	    (!list_empty(&(m->l_elements.list))) ) {
	// Looks like the user wants to go back, so let's remove the line wall
	wall = list_entry(m->l_elements.list.prev, line_element, list);
	action_remove_obstacle_user(EditLevel(), wall->address);
	list_del(m->l_elements.list.prev);
	free(wall);
	if(list_empty(&(m->l_elements.list))) {
	    m->l_direction = UNDEFINED;
	}
    }
}

static void end_line_walls(struct leveleditor_place *m, int commit)
{
    line_element *tmp;
    int list_length = 1;  

    our_mode = DISABLED;

    // Remove the linked list
    while(!(list_empty(&(m->l_elements.list))))
    {
	tmp = list_entry(m->l_elements.list.prev, line_element, list);
	free(tmp);
	if(!commit)
	    action_remove_obstacle(EditLevel(), m->l_elements.address);
	list_del(m->l_elements.list.prev);
	list_length++;
    }
    if(!commit)
	action_remove_obstacle(EditLevel(), m->l_elements.address);

    // Remove the sentinel
    list_del(&(m->l_elements.list));

    if (commit)
	action_push(ACT_MULTIPLE_FLOOR_SETS, list_length);

}

int leveleditor_place_input(SDL_Event *event, struct leveleditor_place *m)
{
    struct leveleditor_typeselect *ts;


    if (our_mode == DISABLED) {
	// Try to place something
	if (EVENT_LEFT_PRESS(event)) {
	    ts = get_current_object_type();

	    switch (ts->type) {
		case OBJECT_FLOOR:
		    start_rectangle_floor(m, ts->indices[ts->selected_tile_nb]);
		    return 0;
		case OBJECT_OBSTACLE:
		    //Check whether to do line mode or single obstacle placement
		    if (obstacle_map[ts->indices[ts->selected_tile_nb]].flags & (IS_VERTICAL | IS_HORIZONTAL)) {
			start_line_walls(m, ts->indices[ts->selected_tile_nb]);
			return 0;
		    } else {
			place_single_obstacle(ts);
			return 1;
		    }
		    break;
		default:
		    GiveMouseAlertWindow("Place tool does not support this type of object.");
	    }

	}
    } else if (our_mode == RECTANGLE_FLOOR) {
	if (EVENT_LEFT_RELEASE(event)) {
	    end_rectangle_floor(1); //commit the modification
	    return 1;
	} else if (EVENT_MOVE(event)) {
	    //change rectangle size
	    handle_rectangle_floor(m);
	} else if (EVENT_RIGHT_PRESS(event) || EVENT_KEYPRESS(event, SDLK_SPACE)) {
	    end_rectangle_floor(0);
	    return 1;
	}
    } else if (our_mode == LINE_WALLS) {
	if (EVENT_LEFT_RELEASE(event)) {
	    end_line_walls(m, 1);
	    return 1;
	} else if (EVENT_MOVE(event)) {
	    handle_line_walls(m);
	} else if (EVENT_RIGHT_PRESS(event) || EVENT_KEYPRESS(event, SDLK_ESCAPE)) {
	    end_line_walls(m, 0);
	    return 1;
	}
    }

    return 0;
}

int leveleditor_place_display(struct leveleditor_place *m)
{

}
