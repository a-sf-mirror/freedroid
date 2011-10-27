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

/**
 * @struct widget_background
 * This widget is used for displaying a background.
 * The background can be composed of several tiles.
 */
struct widget_background {
	struct widget base;	/**< Base widget containg callbacks and position info. */
	struct dynarray tiles;
};

struct widget_background *widget_background_create(void);
void widget_background_add(struct widget_background *, struct image *, int, int, int, int);
void widget_background_load_3x3_tiles(struct widget_background *, char *);

#define WIDGET_BACKGROUND(x) ((struct widget_background *)x)
