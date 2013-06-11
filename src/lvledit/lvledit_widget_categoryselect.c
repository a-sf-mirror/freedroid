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

static struct widget_lvledit_categoryselect *currently_selected_category = NULL;

static int categoryselect_handle_event(struct widget *w, SDL_Event *event)
{
	if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == MOUSE_BUTTON_1) {
		struct widget_lvledit_categoryselect *cs = w->ext;
		currently_selected_category = cs;
		return 1;
	}
	return 0;
}

static void categoryselect_display(struct widget *w)
{
	struct widget_lvledit_categoryselect *cs = w->ext;
	SDL_Rect tr, hr;
	int tab_width = 70;

	draw_rectangle(&w->rect, 70, 100, 100, 150);

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
		draw_rectangle(&hr, 90, 220, 220, 350);

	display_text(cs->title, hr.x + 2, hr.y, &hr);
	tr.x = hr.x + tab_width - 2;
	draw_rectangle(&tr, 0, 0, 0, 136);
	SetCurrentFont(PreviousFont);
}

struct widget_lvledit_categoryselect *get_current_object_type()
{
	return currently_selected_category;
}

void widget_lvledit_categoryselect_activate(struct widget_lvledit_categoryselect *e)
{
	currently_selected_category = e;
}

struct widget *widget_lvledit_categoryselector_create(int x, char *text, enum lvledit_object_type type, int *olist)
{
	struct widget *a = MyMalloc(sizeof(struct widget));
	widget_init(a);
	a->type = WIDGET_CATEGORY_SELECTOR;
	widget_set_rect(a, x * 70, 73, 70, 17);
	a->display = categoryselect_display;
	a->handle_event = categoryselect_handle_event;

	struct widget_lvledit_categoryselect *cs = MyMalloc(sizeof(struct widget_lvledit_categoryselect));
	cs->type = type;
	cs->indices = olist;
	cs->title = text;

	a->ext = cs;
	return a;
}
