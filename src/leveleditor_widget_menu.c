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

#include "leveleditor.h"
#include "leveleditor_actions.h"
#include "leveleditor_widgets.h"

void leveleditor_menu_mouseenter(SDL_Event *event, struct leveleditor_widget *w)
{
    struct leveleditor_menu *m = w->ext;
}

void leveleditor_menu_mouseleave(SDL_Event *event, struct leveleditor_widget *w)
{
    struct leveleditor_menu *m = w->ext;
    m->currently_selected_idx = -1;
    m->ispressed = 0;
}

void leveleditor_menu_mouserelease(SDL_Event *event, struct leveleditor_widget *w)
{
    struct leveleditor_menu *m = w->ext;
    m->done_cb(m->values[m->currently_selected_idx]);

    //XXX autodestroy
}

void leveleditor_menu_mousepress(SDL_Event *event, struct leveleditor_widget *w)
{
    struct leveleditor_menu *m = w->ext;
    m->ispressed = 1;
}

void leveleditor_menu_mouserightrelease(SDL_Event *event, struct leveleditor_widget *w)
{
    struct leveleditor_menu *m = w->ext;
}

void leveleditor_menu_mouserightpress(SDL_Event *event, struct leveleditor_widget *w)
{
    struct leveleditor_menu *m = w->ext;
}

void leveleditor_menu_mousewheelup(SDL_Event *event, struct leveleditor_widget *w)
{
    struct leveleditor_menu *m = w->ext;
}

void leveleditor_menu_mousewheeldown(SDL_Event *event, struct leveleditor_widget *w)
{
    struct leveleditor_menu *m = w->ext;
}

void leveleditor_menu_mousemove(SDL_Event *event, struct leveleditor_widget *w)
{
    struct leveleditor_menu *m = w->ext;
    m->currently_selected_idx = (event->motion.y - w->rect.y) / 20;
    m->ispressed = 0;
} 

void leveleditor_menu_display(struct leveleditor_widget *w)
{
    struct leveleditor_menu *m = w->ext;
    SDL_Rect tr, hr;
    int i;

    our_SDL_fill_rect_wrapper(Screen, &w->rect, 0x656565);

    BFont_Info * PreviousFont;
    PreviousFont = GetCurrentFont();
    SetCurrentFont( Messagevar_BFont );

    tr.x = w->rect.x;
    tr.y = w->rect.y + FontHeight(GetCurrentFont()) + 4;
    tr.w = w->rect.w;

    tr.h = 2;
    
    hr.x=w->rect.x;
    hr.y = w->rect.y + 1;
    hr.w = w->rect.w;
    hr.h = FontHeight(GetCurrentFont()) + 6;


  
    for (i=0; i < 10; i++) {
	if (!strlen(m->text[i]))
		break;

	if (i == m->currently_selected_idx) {
	    if (m->ispressed)
		our_SDL_fill_rect_wrapper(Screen, &hr, 0x455879);
	    else
		our_SDL_fill_rect_wrapper(Screen, &hr, 0x556889);
	}

	DisplayText (m->text[i], hr.x, hr.y, &hr, TEXT_STRETCH);
	our_SDL_fill_rect_wrapper(Screen,&tr,0x88000000);
	tr.y += hr.h;
	hr.y += hr.h;
    }
    SetCurrentFont( PreviousFont );

    //update widget height according to what we know
    w->rect.h = hr.y - w->rect.y;
}


