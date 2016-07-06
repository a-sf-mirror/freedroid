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

#define leveleditor_tools_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit_tools.h"

/**
 * Initialize the tools (ie. move, place, select) of the level editor
 */
void leveleditor_init_tools(void)
{
	static int init_done = 0;
	if (init_done) {
		printf("Tools already initialized\n");
		return;
	}

	tool_move.input_event = leveleditor_move_input;
	tool_move.display = leveleditor_move_display;

	tool_place.input_event = leveleditor_place_input;
	tool_place.display = leveleditor_place_display;

	tool_select.input_event = leveleditor_select_input;
	tool_select.display = leveleditor_select_display;

	init_done = 1;
}
