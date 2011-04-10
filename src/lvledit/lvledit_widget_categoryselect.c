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

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_widgets.h"

static struct leveleditor_categoryselect *currently_selected_category = NULL;

void leveleditor_categoryselect_mouseenter(SDL_Event *event, struct leveleditor_widget *w)
{
	struct leveleditor_categoryselect *cs = w->ext;
	(void)cs;
}

void leveleditor_categoryselect_mouseleave(SDL_Event *event, struct leveleditor_widget *w)
{
	struct leveleditor_categoryselect *cs = w->ext;
	(void)cs;
}

void leveleditor_categoryselect_mouserelease(SDL_Event *event, struct leveleditor_widget *w)
{
	struct leveleditor_categoryselect *cs = w->ext;
	(void)cs;
}

void leveleditor_categoryselect_mousepress(SDL_Event *event, struct leveleditor_widget *w)
{
	struct leveleditor_categoryselect *cs = w->ext;
	currently_selected_category = cs;
}

void leveleditor_categoryselect_mouserightrelease(SDL_Event *event, struct leveleditor_widget *w)
{
	struct leveleditor_categoryselect *cs = w->ext;
	(void)cs;
}

void leveleditor_categoryselect_mouserightpress(SDL_Event *event, struct leveleditor_widget *w)
{
	struct leveleditor_categoryselect *cs = w->ext;
	(void)cs;
}

void leveleditor_categoryselect_mousewheelup(SDL_Event *event, struct leveleditor_widget *w)
{
	struct leveleditor_categoryselect *cs = w->ext;
	(void)cs;
}

void leveleditor_categoryselect_mousewheeldown(SDL_Event *event, struct leveleditor_widget *w)
{
	struct leveleditor_categoryselect *cs = w->ext;
	(void)cs;
}

void leveleditor_categoryselect_display(struct leveleditor_widget *w)
{
	struct leveleditor_categoryselect *cs = w->ext;
	SDL_Rect tr, hr;
	int tab_width = 80;

	draw_rectangle(&w->rect, 80, 100, 100, 150);

	BFont_Info *PreviousFont;
	PreviousFont = GetCurrentFont();
	SetCurrentFont(Messagevar_BFont);

	tr.y = w->rect.y;
	tr.w = 2;
	tr.h = w->rect.h;
	hr.y = w->rect.y;
	hr.w = tab_width - 2;
	hr.h = w->rect.h;

	hr.x = w->rect.x;

	if (cs == currently_selected_category)
		draw_rectangle(&hr, 100, 220, 220, 350);

	display_text(cs->title, hr.x + 2, hr.y, &hr);
	tr.x = hr.x + tab_width - 2;
	draw_rectangle(&tr, 0, 0, 0, 136);
	SetCurrentFont(PreviousFont);
}

struct leveleditor_categoryselect *get_current_object_type()
{
	return currently_selected_category;
}

void leveleditor_categoryselect_activate(struct leveleditor_categoryselect *e)
{
	currently_selected_category = e;
}
