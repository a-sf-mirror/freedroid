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

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "SDL_rotozoom.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_widgets.h"

static struct leveleditor_typeselect *currently_selected_list = NULL;

void leveleditor_typeselect_mouseenter(SDL_Event *event, struct leveleditor_widget *vm)
{
    struct leveleditor_typeselect *m = vm->ext;
    (void)m;
}

void leveleditor_typeselect_mouseleave(SDL_Event *event, struct leveleditor_widget *vm)
{
    struct leveleditor_typeselect *m = vm->ext;
    (void)m;
}

void leveleditor_typeselect_mouserelease(SDL_Event *event, struct leveleditor_widget *vm)
{
    struct leveleditor_typeselect *m = vm->ext;
    (void)m;
}

void leveleditor_typeselect_mousepress(SDL_Event *event, struct leveleditor_widget *vm)
{
    struct leveleditor_typeselect *m = vm->ext;
    currently_selected_list = m;
}

void leveleditor_typeselect_mouserightrelease(SDL_Event *event, struct leveleditor_widget *vm)
{
    struct leveleditor_typeselect *m = vm->ext;
    (void)m;
}

void leveleditor_typeselect_mouserightpress(SDL_Event *event, struct leveleditor_widget *vm)
{
    struct leveleditor_typeselect *m = vm->ext;
    (void)m;
}

void leveleditor_typeselect_mousewheelup(SDL_Event *event, struct leveleditor_widget *vm)
{
    struct leveleditor_typeselect *m = vm->ext;
    (void)m;
}

void leveleditor_typeselect_mousewheeldown(SDL_Event *event, struct leveleditor_widget *vm)
{
    struct leveleditor_typeselect *m = vm->ext;
    (void)m;
}

void leveleditor_typeselect_display(struct leveleditor_widget *vm)
{
    struct leveleditor_typeselect *m = vm->ext;
    SDL_Rect tr, hr;
    int tab_width = 80;

    our_SDL_fill_rect_wrapper(Screen, &vm->rect, SDL_MapRGB(Screen->format, 0x65, 0x65, 0x65));

    BFont_Info * PreviousFont;
    PreviousFont = GetCurrentFont();
    SetCurrentFont( Messagevar_BFont );

    tr.y = vm->rect.y;    
    tr . w = 2;
    tr . h = vm->rect.h;
    hr . y = vm->rect.y ; 
    hr.w = tab_width-2; 
    hr.h = vm->rect.h;

    hr.x=vm->rect.x;

    if (m == currently_selected_list)
		our_SDL_fill_rect_wrapper(Screen, &hr, SDL_MapRGB(Screen->format, 0x55, 0x68, 0x89));
    
    DisplayText (m->title, hr.x+2 , hr.y , &hr , TEXT_STRETCH);
    tr.x = hr.x + tab_width - 2;
    our_SDL_fill_rect_wrapper(Screen,&tr,SDL_MapRGBA(Screen->format, 0x00, 0x00, 0x00, 0x88));
    SetCurrentFont( PreviousFont );

}

struct leveleditor_typeselect *get_current_object_type()
{
    return currently_selected_list;
}

void leveleditor_typeselect_init_selected_list(struct leveleditor_typeselect *e)
{
    currently_selected_list = e;
}
