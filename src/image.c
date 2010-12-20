/*
 *
 *   Copyright (c) 2010 Arthur Huillet
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

#define _image_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

/**
 * This file contains image related functions.
 * An image can be rendered using SDL, or using OpenGL.
 * In OpenGL mode, we define a "batch" as being a series of images
 * drawn in sequence. OpenGL is designed in such a way that batches should be 
 * emitted without changing the active texture, and in a single glBegin/glEnd pair.
 */

// Currently active OpenGL texture
static int active_tex = -1;
// Do we want to draw as a batch? (ie. not emit glBegin/glEnd pairs every time)
static int batch_draw = FALSE;
// Are we currently inside a glBegin/glEnd pair?
static int emit_begun = FALSE;

static void gl_begin()
{
#ifdef HAVE_LIBGL
	if (!emit_begun) {
		glBegin(GL_QUADS);
		emit_begun = TRUE;
	}
#endif
}

static void gl_end()
{
#ifdef HAVE_LIBGL
	if (emit_begun) {
		glEnd();
		emit_begun = FALSE;
	}
#endif
}

/**
 * Start rendering images as a batch.
 */
void start_image_batch()
{
	batch_draw = TRUE;
}

/**
 * End the image batch.
 */
void end_image_batch()
{
	batch_draw = FALSE;
	gl_end();
	active_tex = -1;
}
 
/**
 * Draw an image in OpenGL mode.
 * Changes the active texture if necessary, and emits glBegin/glEnd pairs
 * according to the texture switches and whether a batch is required.
 */
static void gl_display_image(struct image *img, int x, int y, float scale)
{
	// Compute image coordinates according to scale factor
	int xmax = img->w;
	int ymax = img->h;
	int xoff = img->offset_x;
	int yoff = img->offset_y;

	if (scale != 1.0) {
		xmax *= scale;
		ymax *= scale;
		xoff *= scale;
		yoff *= scale;
	}

	x += xoff;
	y += yoff;
	xmax += x;
	ymax += y;

#ifdef HAVE_LIBGL
	// Bind the texture if required
	if (img->texture != active_tex) {
			gl_end();
			glBindTexture(GL_TEXTURE_2D, img->texture);
			active_tex = img->texture;
			gl_begin();
	}
			
	// Draw the image	
	gl_begin();
	glTexCoord2f(img->tex_x0, img->tex_y1);
	glVertex2i(x, y);
	glTexCoord2f(img->tex_x0, img->tex_y0);
	glVertex2i(x, ymax);
	glTexCoord2f(img->tex_x1, img->tex_y0);
	glVertex2i(xmax, ymax);
	glTexCoord2f(img->tex_x1, img->tex_y1);
	glVertex2i(xmax, y);

	// glEnd() is only required if we are not doing a batch
	if (!batch_draw) {
		gl_end();
	}
#endif
}

/**
 * Draw an image in SDL mode.
 */
static void sdl_display_image(struct image *img, int x, int y, int zoomed)
{
	SDL_Rect target_rectangle = { .x = x, .y = y };
	SDL_Surface *surf;

	if (zoomed) {
		float zoom_factor = lvledit_zoomfact_inv();
		make_sure_zoomed_surface_is_there(img);
		target_rectangle.x += img->offset_x * zoom_factor;
		target_rectangle.y += img->offset_y * zoom_factor;
		surf = img->zoomed_out_surface;
	} else {
		target_rectangle.x += img->offset_x;
		target_rectangle.y += img->offset_y;
		surf = img->surface;
	}

	SDL_BlitSurface(surf, NULL, Screen, &target_rectangle);
}

/**
 * Draw an image at a given screen position, using SDL or OpenGL.
 */
void display_image_on_screen_scaled(struct image *img, int x, int y, float scale)
{
	if (use_open_gl) {
		gl_display_image(img, x, y, scale);
	} else {
		if (scale == 1.0) {
			sdl_display_image(img, x, y, 0);
		} else {
			sdl_display_image(img, x, y, 1);
		}
	}
}

void display_image_on_screen(struct image *img, int x, int y)
{
	display_image_on_screen_scaled(img, x, y, 1.0);
}

void display_image_on_map_scaled(struct image *img, float X, float Y, float scale)
{
	int x, y;
	translate_map_point_to_screen_pixel(X, Y, &x, &y);
	display_image_on_screen_scaled(img, x, y, scale);
}

void display_image_on_map(struct image *img, float X, float Y)
{
	display_image_on_map_scaled(img, X, Y, 1.0);
}


