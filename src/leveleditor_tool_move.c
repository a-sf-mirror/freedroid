
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

#define leveleditor_tool_move_c

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

/**
 * Input events get forwarded to us this way.
 *
 * @return 1 if we are done (disabled), 0 if we still want
 * to be forwarded input events.
 */
int leveleditor_move_input(SDL_Event *event, void *am)
{
    struct leveleditor_move *m = am;

    if (EVENT_RIGHT_PRESS(event)) {
	// Start a movement
	m->origin.x = event->button.x;
	m->origin.y = event->button.y;
    } else if (EVENT_RIGHT_RELEASE(event)) {
	// We are done
	return 1;
    } else if (EVENT_MOVE(event)) {
	m -> c_corresponding_position = translate_point_to_map_location (
		m->origin.x - (GameConfig.screen_width/2), 
		m->origin.y - (GameConfig.screen_height/2),
		GameConfig.zoom_is_on);

	// Calculate the new position
	Me . pos . x += (mouse_mapcoord.x - m->c_corresponding_position.x) / 30 ;
	Me . pos . y += (mouse_mapcoord.y - m->c_corresponding_position.y) / 30 ;
	
	if ( Me . pos . x > curShip.AllLevels[Me.pos.z]->xlen )
	    Me . pos . x = curShip.AllLevels[Me.pos.z]->xlen-1 ;
	if ( Me . pos . x < 0 )
	    Me . pos . x = 0;
	if ( Me . pos . y > curShip.AllLevels[Me.pos.z]->ylen )
	    Me . pos . y = curShip.AllLevels[Me.pos.z]->ylen-1 ;
	if ( Me . pos . y < 0 )
	    Me . pos . y = 0;
    }


    return 0;    
}

int leveleditor_move_display(void *am)
{
    struct leveleditor_move *m = am;
    blit_leveleditor_point (m->origin.x, m->origin.y);
}
