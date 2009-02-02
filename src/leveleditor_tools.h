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
#ifndef _leveleditor_tools_c
#define EXTERN extern
#else
#define EXTERN
#endif

enum leveleditor_tools {
    TOOL_NONE,
    TOOL_MOVE,
    TOOL_PLACE,
    TOOL_SELECT,
};

struct leveleditor_tool {
    int type;
    struct list_head node;
    int cursor;
    int (*input_event)(SDL_Event *, void *);
    int (*display)(void *);
    void *ext;
};

struct leveleditor_place {
    enum { MODE_LINE, MODE_RECTANGLE, MODE_SINGLE } mode;

    /* Line mode */
    int l_direction;
    int l_selected_mode;
    int l_id;
    line_element l_elements;

    /* Rectangle mode */
    point r_start;
    int r_len_x, r_len_y;
    int r_step_x, r_step_y;
    int r_tile_used;
};

struct leveleditor_move {
    /* click&drag */
    point origin;
    moderately_finepoint c_corresponding_position;
};

struct leveleditor_select {
    /*XXX*/
    int Iamnotimplemented;
};

EXTERN void leveleditor_init_tools(void);
EXTERN struct list_head leveleditor_tool_list;

#define EVENT_LEFT_PRESS(e) (((e) && e->type == SDL_MOUSEBUTTONDOWN) && (e->button.button == 1))
#define EVENT_RIGHT_PRESS(e) (((e) && e->type == SDL_MOUSEBUTTONDOWN) && (e->button.button == 3))
#define EVENT_LEFT_RELEASE(e) (((e) && e->type == SDL_MOUSEBUTTONUP) && (e->button.button == 1))
#define EVENT_RIGHT_RELEASE(e) (((e) && e->type == SDL_MOUSEBUTTONUP) && (e->button.button == 3))
#define EVENT_MOVE(e) (((e) && e->type == SDL_MOUSEMOTION))
#define EVENT_NONE(e) ((e == NULL))
