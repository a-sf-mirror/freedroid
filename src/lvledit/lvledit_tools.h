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

#undef EXTERN
#ifndef leveleditor_tools_c
#define EXTERN extern
#else
#define EXTERN
#endif

struct leveleditor_tool {
	int (*input_event) (SDL_Event *);
	int (*display) ();
};

void leveleditor_init_tools(void);

struct leveleditor_tool tool_place, tool_move, tool_select;

#define EVENT_LEFT_PRESS(e) (((e) && e->type == SDL_MOUSEBUTTONDOWN) && (e->button.button == 1))
#define EVENT_RIGHT_PRESS(e) (((e) && e->type == SDL_MOUSEBUTTONDOWN) && (e->button.button == 3))
#define EVENT_LEFT_RELEASE(e) (((e) && e->type == SDL_MOUSEBUTTONUP) && (e->button.button == 1))
#define EVENT_RIGHT_RELEASE(e) (((e) && e->type == SDL_MOUSEBUTTONUP) && (e->button.button == 3))
#define EVENT_MOVE(e) (((e) && e->type == SDL_MOUSEMOTION))
#define EVENT_NONE(e) ((e == NULL))
#define EVENT_KEYPRESS(e, s) (((e) && (e->type == SDL_KEYDOWN) && (e->key.keysym.sym == s)))
#define EVENT_KEYRELEASE(e, s) (((e) && (e->type == SDL_KEYUP && (e->key.keysym.sym == s)))

#include "lvledit/lvledit_tool_move.h"
#include "lvledit/lvledit_tool_place.h"
#include "lvledit/lvledit_tool_select.h"
