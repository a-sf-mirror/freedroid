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

#ifndef WIDGET_GROUP_H
#define WIDGET_GROUP_H

/** 
 * @struct widget_group
 * @brief Type used for managing multiple widgets. 
 *
 * This structure type is used for storing, displaying and updating a group of
 * widgets.
 * NOTE: Widget types inheriting widget_group must have it as their first attribute.
 */
struct widget_group {
	struct widget base;		/**< Base widget type, containing callback functions. */
	struct widget *last_focused;	/**< Widget pointer used for handling enter/leave events. */
	struct list_head list;		/**< Linked list used for storing children. */
};

int widget_group_add(struct widget_group *, struct widget *);
int widget_group_handle_event(struct widget *, SDL_Event *);
struct widget_group *widget_group_create(void);
void widget_group_init(struct widget_group *);

#define WIDGET_GROUP(x) ((struct widget_group *)x)

#endif

