/* 
 *
 *   Copyright (c) 2009-2010 Arthur Huillet
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

/**
 * @file lvledit_widgets.h
 * This file contains function declaration used by the level editor.
 */

#undef EXTERN
#ifndef _lvledit_widgets_c
#define EXTERN extern
#else
#define EXTERN
#endif

void lvledit_select_type(enum lvledit_object_type);
void lvledit_categoryselect_switch(int direction);
struct widget *get_active_widget(int, int);

EXTERN struct list_head *lvledit_widget_list;
EXTERN struct widget *previously_active_widget;

#include "widgets/widgets.h"
#include "lvledit/lvledit_widget_map.h"
#include "lvledit/lvledit_widget_toolbar.h"
#include "lvledit/lvledit_widget_categoryselect.h"
#include "lvledit/lvledit_widget_minimap.h"
