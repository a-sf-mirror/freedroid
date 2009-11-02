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
#ifndef leveleditor_widget_map_c
#define EXTERN extern
#else
#define EXTERN
#endif

void leveleditor_map_mouseenter(SDL_Event *, struct leveleditor_widget *);
void leveleditor_map_mouseleave(SDL_Event *, struct leveleditor_widget *);
void leveleditor_map_mouserelease(SDL_Event *, struct leveleditor_widget *);
void leveleditor_map_mousepress(SDL_Event *, struct leveleditor_widget *);
void leveleditor_map_mouserightrelease(SDL_Event *, struct leveleditor_widget *);
void leveleditor_map_mouserightpress(SDL_Event *, struct leveleditor_widget *);
void leveleditor_map_mousewheelup(SDL_Event *, struct leveleditor_widget *);
void leveleditor_map_mousewheeldown(SDL_Event *, struct leveleditor_widget *);
void leveleditor_map_mousemove(SDL_Event *, struct leveleditor_widget *);
int leveleditor_map_keybevent(SDL_Event *, struct leveleditor_widget *);

void leveleditor_map_display(struct leveleditor_widget *);
void leveleditor_map_display_cursor();

void leveleditor_map_init(void);

void leveleditor_update_tool(void);
EXTERN moderately_finepoint mouse_mapcoord;
EXTERN int mouse_in_level;
