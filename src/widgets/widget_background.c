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

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "widgets/widgets.h"

/**
 * Structure type used for storing images by background widgets.
 */
struct tile {
	SDL_Rect rect;		/**< Position rectangle. */
	struct image *image;
};

/**
 * Display a background widget.
 */
static void background_display(struct widget *w)
{
	// A background can be composed of several tiles.
	int i;
	struct tile *tile = WIDGET_BACKGROUND(w)->tiles.arr;
	for (i = 0; i < WIDGET_BACKGROUND(w)->tiles.size; i++) {
		struct image *img = tile[i].image;
		SDL_Rect rect = tile[i].rect;

		if (!img)
			return;

		display_image_on_screen(img, rect.x, rect.y, set_image_transformation(rect.w / (float)img->w, rect.h / (float)img->h, 1, 1, 1, 1, 0));
	}
}

/**
 * Background widgets don't handle events.
 */
static int background_handle_event(struct widget *w, SDL_Event *event)
{
	return 0;
}

/**
 * This function creates a background widget using default values.
 * @return A pointer to the newly created widget.
 */
struct widget_background *widget_background_create()
{
	struct widget_background *wb = MyMalloc(sizeof(struct widget_background));
	widget_set_rect(WIDGET(wb), 0, 0, 0, 0);
	WIDGET(wb)->handle_event = background_handle_event;
	WIDGET(wb)->display = background_display;
	WIDGET(wb)->update = NULL;
	WIDGET(wb)->enabled = 1;
	
	dynarray_init(&wb->tiles, 2, sizeof(struct tile));
	return wb;
}

/**
 * This function adds a new tile to a background widget.
 */
void widget_background_add(struct widget_background *wb, struct image *img, int x, int y, int w, int h)
{
	struct tile tile;	
	tile.image = img;
	Set_Rect(tile.rect, x, y, w, h);
	dynarray_add(&wb->tiles, &tile, sizeof(struct tile));		
}
