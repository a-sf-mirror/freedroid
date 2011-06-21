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

#ifndef _widgets_c
#endif

struct widget_lvledit_categoryselect {
	unsigned int selected_tile_nb;
	unsigned int toolbar_first_block;
	char *title;
	enum lvledit_object_type type;
	int *indices;
};

void widget_lvledit_categoryselect_display(struct widget *);

struct widget *widget_lvledit_categoryselector_create(int, char *, enum lvledit_object_type, int *);
struct widget_lvledit_categoryselect *get_current_object_type(void);
void widget_lvledit_categoryselect_activate(struct widget_lvledit_categoryselect *);
