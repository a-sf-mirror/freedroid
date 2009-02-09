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

void leveleditor_menu_mouseenter(SDL_Event *event, struct leveleditor_widget *w)
{
    struct leveleditor_menu *m = w->ext;
    (void) m;
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

    // autodisable
    w->enabled = 0;
}

void leveleditor_menu_mousepress(SDL_Event *event, struct leveleditor_widget *w)
{
    struct leveleditor_menu *m = w->ext;
    m->ispressed = 1;
}

void leveleditor_menu_mouserightrelease(SDL_Event *event, struct leveleditor_widget *w)
{
    struct leveleditor_menu *m = w->ext;
    (void) m;
}

void leveleditor_menu_mouserightpress(SDL_Event *event, struct leveleditor_widget *w)
{
    struct leveleditor_menu *m = w->ext;
    (void) m;
}

void leveleditor_menu_mousewheelup(SDL_Event *event, struct leveleditor_widget *w)
{
    struct leveleditor_menu *m = w->ext;
    (void) m;
}

void leveleditor_menu_mousewheeldown(SDL_Event *event, struct leveleditor_widget *w)
{
    struct leveleditor_menu *m = w->ext;
    (void) m;
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
    SDL_Rect separ, txt, high;
    int i;

    our_SDL_fill_rect_wrapper(Screen, &w->rect, SDL_MapRGB(Screen->format, 0x65, 0x65, 0x65));

    BFont_Info * PreviousFont;
    PreviousFont = GetCurrentFont();
    SetCurrentFont( Messagevar_BFont );

    separ.x = w->rect.x;
    separ.y = w->rect.y + FontHeight(GetCurrentFont()) + 4;
    separ.w = w->rect.w;
    separ.h = 2;

    txt.x = w->rect.x;
    txt.y = w->rect.y + 1;
    txt.w = w->rect.w;
    txt.h = FontHeight(GetCurrentFont()) + 6;

    high.x = w->rect.x;
    high.y = w->rect.y;
    high.w = w->rect.w;
    high.h = FontHeight(GetCurrentFont()) + 6;

    for (i=0; i < 10; i++) {
	if (!strlen(m->text[i]))
		break;

	if (i == m->currently_selected_idx) {
	    if (m->ispressed)
		our_SDL_fill_rect_wrapper(Screen, &high, SDL_MapRGB(Screen->format, 0x45, 0x58, 0x79));
	    else
		our_SDL_fill_rect_wrapper(Screen, &high, SDL_MapRGB(Screen->format, 0x55, 0x68, 0x89));
	}

	DisplayText (m->text[i], txt.x, txt.y, &txt, TEXT_STRETCH);
	our_SDL_fill_rect_wrapper(Screen,&separ, SDL_MapRGBA(Screen->format, 0x00, 0x00, 0x00, 0x88));
	txt.y += txt.h;
	high.y += txt.h;
	separ.y += txt.h;
    }
    SetCurrentFont( PreviousFont );

    //update widget height according to what we know
    w->rect.h = txt.y - w->rect.y;
}


