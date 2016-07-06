/* 
 *
 *   Copyright (c) 2002, 2003 Johannes Prix
 *   Copyright (c) 2004-2009 Arthur Huillet
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

#define _leveleditor_input_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_map.h"
#include "lvledit/lvledit_menu.h"
#include "widgets/widgets.h"

static void HandleLevelEditorCursorKeys()
{
	level *EditLevel;

	EditLevel = curShip.AllLevels[Me.pos.z];
	static int PressedSince[4] = { 0, 0, 0, 0 };
	int DoAct[4] = { 0, 0, 0, 0 };
	int i;
	int repeat = 1;

	// Keys that are released have to be marked as such
	if (!LeftPressed())
		PressedSince[0] = 0;
	if (!RightPressed())
		PressedSince[1] = 0;
	if (!DownPressed())
		PressedSince[2] = 0;
	if (!UpPressed())
		PressedSince[3] = 0;

	if (LeftPressed() && PressedSince[0] == 0) {
		PressedSince[0] = SDL_GetTicks();
		DoAct[0] = 1;
	}
	if (RightPressed() && PressedSince[1] == 0) {
		PressedSince[1] = SDL_GetTicks();
		DoAct[1] = 1;
	}
	if (DownPressed() && PressedSince[2] == 0) {
		PressedSince[2] = SDL_GetTicks();
		DoAct[2] = 1;
	}
	if (UpPressed() && PressedSince[3] == 0) {
		PressedSince[3] = SDL_GetTicks();
		DoAct[3] = 1;
	}

	for (i = 0; i < 4; i++) {
		if (PressedSince[i] && SDL_GetTicks() - PressedSince[i] > 500) {
			DoAct[i] = 1;
			PressedSince[i] = SDL_GetTicks() - 450;
		}
	}

	if (CtrlPressed())
		repeat = FLOOR_TILES_VISIBLE_AROUND_TUX;

	if (DoAct[0]) {
		for (i = 0; i < repeat; i++)
			if (rintf(Me.pos.x) > 0)
				Me.pos.x -= 1;
	}
	if (DoAct[1]) {
		for (i = 0; i < repeat; i++)
			if (rintf(Me.pos.x) < EditLevel->xlen - 1)
				Me.pos.x += 1;
	}
	if (DoAct[2]) {
		for (i = 0; i < repeat; i++)
			if (rintf(Me.pos.y) < EditLevel->ylen - 1)
				Me.pos.y += 1;
	}
	if (DoAct[3]) {
		for (i = 0; i < repeat; i++)
			if (rintf(Me.pos.y) > 0)
				Me.pos.y -= 1;
	}
};				// void HandleLevelEditorCursorKeys ( void )

/**
 * This function automatically scrolls the leveleditor window when the
 * mouse reaches an edge 
 */
static void level_editor_auto_scroll()
{
	float chx = 0, chy = 0;	/*Value of the change to player position */
	static int edgedate[4] = { 0, 0, 0, 0 };

#define AUTOSCROLL_DELAY 500
#define AUTOSCROLL_SPEED 20

	if (GetMousePos_x() < 5) {
		// scroll to the left
		if (edgedate[0] == 0) {
			edgedate[0] = SDL_GetTicks();
		} else if (SDL_GetTicks() - edgedate[0] > AUTOSCROLL_DELAY) {
			chx -= AUTOSCROLL_SPEED * Frame_Time();
			chy += AUTOSCROLL_SPEED * Frame_Time();
		}
	} else {
		edgedate[0] = 0;
	}

	if (GameConfig.screen_width - GetMousePos_x() < 5) {
		// scroll to the right
		if (edgedate[1] == 0) {
			edgedate[1] = SDL_GetTicks();
		} else if (SDL_GetTicks() - edgedate[1] > AUTOSCROLL_DELAY) {
			chx += AUTOSCROLL_SPEED * Frame_Time();
			chy -= AUTOSCROLL_SPEED * Frame_Time();
		}
	} else {
		edgedate[1] = 0;
	}

	if (GameConfig.screen_height - GetMousePos_y() < 5) {
		// scroll down
		if (edgedate[2] == 0) {
			edgedate[2] = SDL_GetTicks();
		} else if (SDL_GetTicks() - edgedate[2] > AUTOSCROLL_DELAY) {
			chx += AUTOSCROLL_SPEED * Frame_Time();
			chy += AUTOSCROLL_SPEED * Frame_Time();
		}
	} else {
		edgedate[2] = 0;
	}

	if (GetMousePos_y() < 5) {
		//scroll up
		if (edgedate[3] == 0) {
			edgedate[3] = SDL_GetTicks();
		} else if (SDL_GetTicks() - edgedate[3] > AUTOSCROLL_DELAY) {
			chx -= AUTOSCROLL_SPEED * Frame_Time();
			chy -= AUTOSCROLL_SPEED * Frame_Time();
		}
	} else {
		edgedate[3] = 0;
	}

	Me.pos.x += chx;
	Me.pos.y += chy;

	if (Me.pos.x >= curShip.AllLevels[Me.pos.z]->xlen - 1)
		Me.pos.x = curShip.AllLevels[Me.pos.z]->xlen - 1;
	if (Me.pos.x <= 0)
		Me.pos.x = 0;
	if (Me.pos.y >= curShip.AllLevels[Me.pos.z]->ylen - 1)
		Me.pos.y = curShip.AllLevels[Me.pos.z]->ylen - 1;
	if (Me.pos.y <= 0)
		Me.pos.y = 0;
}

void leveleditor_process_input()
{
	save_mouse_state();
	input_handle();

	HandleLevelEditorCursorKeys();

	level_editor_auto_scroll();

	if (EscapePressed()) {
		level_editor_done = DoLevelEditorMainMenu();
	}
	while (EscapePressed())
		SDL_Delay(1);

}
