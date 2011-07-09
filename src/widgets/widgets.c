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

#define _widgets_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "widgets/widgets.h"

/**
 * @struct image_resource
 * Used by widget_load_image_resource to store images.
 */
struct image_resource {
	char *name;
	struct image img;
	struct list_head node;
};

LIST_HEAD(widget_list);
LIST_HEAD(image_resource_list);

/**
 * This function used to store images used by the widget system.
 */
struct image *widget_load_image_resource(char *name, int use_offset_file) 
{
	struct image_resource *res;
	
	// Check if image is already loaded.
	list_for_each_entry(res, &image_resource_list, node) {
		if (!strcmp(name, res->name))
			return &res->img;
	}

	// Image not found, allocate memory and load it from its file.
	res = MyMalloc(sizeof(struct image_resource));
	load_image(&res->img, name, use_offset_file);
	res->name = name;
	list_add(&res->node, &image_resource_list);
	return &res->img;
}

/**
 * This function pushes events to the currently active top level containers. 
 *
 * NOTE: EVENT_UPDATE events are passed to both active and inactive top level containers.
 */
void handle_widget_event(SDL_Event *event)
{
	struct widget *w;
	int is_update_event = 0;
	if (event->type == SDL_USEREVENT && event->user.code == EVENT_UPDATE)
		is_update_event = 1;
	list_for_each_entry(w, &widget_list, node) {
		if (is_update_event || w->enabled)
			w->handle_event(w, event);
	}
}

void widget_set_rect(struct widget *w, int x, int y, int width, int height)
{
	w->rect.x = x;
	w->rect.y = y;
	w->rect.w = width;
	w->rect.h = height;
}

/**
 * This functions pushes an EVENT_UPDATE event through the widget system.
 * This should be called once every few frames to update widgets' state.
 *
 * NOTE: Update events are handled by disabled widgets but are not sent to
 * children widgets. This allows for inactive widgets to become active on
 * update events. 
 */
void update_widgets()
{
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = EVENT_UPDATE;
	handle_widget_event(&event);
}

/**
 * This function displays the currently active top level widget groups.
 */
void display_widgets() 
{
	struct widget *w;
	list_for_each_entry(w, &widget_list, node) {
		if (w->enabled && w->display)
			w->display(w);
	}
}
