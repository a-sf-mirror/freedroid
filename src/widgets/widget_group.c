/* 
 *
 *   Copyright (c) 2011 Catalin Badea
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

#define _widget_group_c

#include "system.h"
#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "widgets/widgets.h"

static void group_display(struct widget *w) 
{
	struct widget *widget;
	list_for_each_entry(widget, &WIDGET_GROUP(w)->list, node) {
		if (widget->enabled && widget->display)
			widget->display(widget);	
	}
}

struct widget_group *widget_group_create() 
{
	struct widget_group *wb = (struct widget_group *)MyMalloc(sizeof(struct widget_group));	
	WIDGET(wb)->display = group_display;
	WIDGET(wb)->enabled = 1;
	WIDGET(wb)->mouseenter = NULL;
	WIDGET(wb)->mouseleave = NULL;
	WIDGET(wb)->mousepress = NULL;
	WIDGET(wb)->mouserelease = NULL;
	WIDGET(wb)->mouserightpress = NULL;
	WIDGET(wb)->mouserightrelease = NULL;
	wb->list = (struct list_head)LIST_HEAD_INIT(wb->list);	
	wb->last_focused = NULL;
	return wb;
}

int widget_group_add(struct widget_group *wg, struct widget *w) 
{
	list_add_tail(&w->node, &wg->list);
	return 0;
}

#undef _widget_group_c
