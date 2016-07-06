
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

#define leveleditor_tool_move_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"

#include "widgets/widgets.h"

#include "lvledit/lvledit_tools.h"

static struct leveleditor_move {
	/* click&drag */
	point origin;
	moderately_finepoint c_corresponding_position;
} state;

/**
 * Input events get forwarded to us this way.
 *
 * @return 1 if we are done (disabled), 0 if we still want
 * to be forwarded input events.
 */
int leveleditor_move_input(SDL_Event * event)
{
	moderately_finepoint a, b;

	if (EVENT_RIGHT_PRESS(event) || EVENT_LEFT_PRESS(event)) {
		// Start a movement
		state.origin.x = event->button.x;
		state.origin.y = event->button.y;
	} else if (EVENT_RIGHT_RELEASE(event) || EVENT_LEFT_RELEASE(event)) {
		// We are done
		return 1;
	}

	if (EVENT_NONE(event)) {	//time-based periodic updating
		a = translate_point_to_map_location(state.origin.x - (GameConfig.screen_width / 2),
						    state.origin.y - (GameConfig.screen_height / 2), GameConfig.zoom_is_on);

		b = translate_point_to_map_location(GetMousePos_x() - (GameConfig.screen_width / 2),
						    GetMousePos_y() - (GameConfig.screen_height / 2), GameConfig.zoom_is_on);

#define SCROLL_SPEED_FACTOR 4

		// Calculate the new position
		Me.pos.x += SCROLL_SPEED_FACTOR * Frame_Time() * (b.x - a.x);
		Me.pos.y += SCROLL_SPEED_FACTOR * Frame_Time() * (b.y - a.y);

		if (Me.pos.x > curShip.AllLevels[Me.pos.z]->xlen)
			Me.pos.x = curShip.AllLevels[Me.pos.z]->xlen - 1;
		if (Me.pos.x < 0)
			Me.pos.x = 0;
		if (Me.pos.y > curShip.AllLevels[Me.pos.z]->ylen)
			Me.pos.y = curShip.AllLevels[Me.pos.z]->ylen - 1;
		if (Me.pos.y < 0)
			Me.pos.y = 0;
	}

	return 0;
}

int leveleditor_move_display()
{
	blit_leveleditor_point(state.origin.x, state.origin.y);

	return 0;
}
