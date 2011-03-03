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

extern int gl_max_texture_size;	//defined in open_gl.c 

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
		active_tex = -1;
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
}
 
/**
 * Draw an image in OpenGL mode.
 * Changes the active texture if necessary, and emits glBegin/glEnd pairs
 * according to the texture switches and whether a batch is required.
 */
static void gl_display_image(struct image *img, int x, int y, struct image_transformation *t)
{
	// Compute image coordinates according to scale factor
	int xmax = img->w;
	int ymax = img->h;
	int xoff = img->offset_x;
	int yoff = img->offset_y;

	if (t->scale != 1.0) {
		xmax *= t->scale;
		ymax *= t->scale;
		xoff *= t->scale;
		yoff *= t->scale;
	}

	glColor4f(t->r, t->g, t->b, t->a);

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
	glTexCoord2f(img->tex_x0, img->tex_y0);
	glVertex2i(x, y);
	glTexCoord2f(img->tex_x0, img->tex_y1);
	glVertex2i(x, ymax);
	glTexCoord2f(img->tex_x1, img->tex_y1);
	glVertex2i(xmax, ymax);
	glTexCoord2f(img->tex_x1, img->tex_y0);
	glVertex2i(xmax, y);

	// glEnd() is only required if we are not doing a batch
	if (!batch_draw) {
		gl_end();
	}
#endif
}

static void get_components(SDL_Surface *surf, int x, int y, int *r, int *g, int *b, int *a)
{
	SDL_PixelFormat *fmt = surf->format;

	SDL_LockSurface(surf);
	Uint32 pixel = *((Uint32 *) (((Uint8 *) (surf->pixels)) + (x + y * surf->w) * surf->format->BytesPerPixel));
	SDL_UnlockSurface(surf);

	*r = (pixel >> fmt->Rshift) & 0xFF;
	*g = (pixel >> fmt->Gshift) & 0xFF;
	*b = (pixel >> fmt->Bshift) & 0xFF;
	*a = (pixel >> fmt->Ashift) & 0xFF;
}

static SDL_Surface *sdl_create_colored_surface(SDL_Surface *surf, float r, float g, float b, float a)
{
	int red, green, blue, alpha;
	SDL_Surface *colored_surf;
	int x, y;

	colored_surf = our_SDL_display_format_wrapperAlpha(surf);

	for (y = 0; y < surf->h; y++) {
		for (x = 0; x < surf->w; x++) {

			get_components(surf, x, y, &red, &green, &blue, &alpha);

			if (r != 1.0)
				red *= r;
			if (g != 1.0)
				green *= g;
			if (b != 1.0)
				blue *= b;
			if (a != 1.0)
				alpha *= a;

			pixelRGBA(colored_surf, x, y, red, green, blue, alpha);
		}
	}

	return colored_surf;
}

/**
 * Draw an image in SDL mode.
 */
static void sdl_display_image(struct image *img, int x, int y, struct image_transformation *t)
{
	SDL_Rect target_rectangle = { .x = x, .y = y };
	SDL_Surface *surf;

	// Check if the image must be transformed
	if (t->scale == 1.0 && t->r == 1.0 && t->g == 1.0 && t->b == 1.0 && t->a == 1.0) {
		// No transformation
		surf = img->surface;
	} else {
		// Transformed image

		// Check if the transformation is in cache
		struct image_transformation *cache = &img->cached_transformation;
		if (!cache->surface || cache->scale != t->scale || cache->r != t->r || cache->g != t->g || cache->b != t->b || cache->a != t->a) {
			// Transformed image is not in cache, create it

			int scaled;
			if (t->scale != 1.0) {
				t->surface = zoomSurface(img->surface, t->scale, t->scale, FALSE); 
				scaled = 1;
			} else {
				t->surface = img->surface;
				scaled = 0;
			}

			if (t->r != 1.0 || t->g != 1.0 || t->b != 1.0 || t->a != 1.0) {
				SDL_Surface *tmp = sdl_create_colored_surface(t->surface, t->r, t->g, t->b, t->a);

				if (scaled)
					SDL_FreeSurface(t->surface);

				t->surface = tmp;
			}

			// Cache the transformation we have done
			if (cache->surface)
				SDL_FreeSurface(cache->surface);

			*cache = *t;
		}

		// Use the transformed surface in the cache
		surf = cache->surface;
	}

	target_rectangle.x += img->offset_x * t->scale;
	target_rectangle.y += img->offset_y * t->scale;

	SDL_BlitSurface(surf, NULL, Screen, &target_rectangle);
}

/**
 * Display an image on the screen.
 */
void display_image_on_screen(struct image *img, int x, int y, struct image_transformation t)
{
	if (use_open_gl)
		gl_display_image(img, x, y, &t);
	else sdl_display_image(img, x, y, &t);
}

/**
 * Display an image on the map.
 */
void display_image_on_map(struct image *img, float X, float Y, struct image_transformation t)
{
	int x, y;
	translate_map_point_to_screen_pixel(X, Y, &x, &y);
	display_image_on_screen(img, x, y, t);
}

/**
 * Create an image as a part of another (texture atlas element).
 */
void create_subimage(struct image *source, struct image *new_img, SDL_Rect *rect)
{
	// Set image width and height
	*new_img = *source;
	new_img->w = rect->w;
	new_img->h = rect->h;

	if (use_open_gl) {

		// In OpenGL mode, require the source to be a texture.
		if (!source->texture_has_been_created) {
			ErrorMessage(__FUNCTION__, "Trying to create subimage from image source %x (width %d height %d), but image does not have a GL texture.", PLEASE_INFORM, IS_WARNING_ONLY, source, source->w, source->h);
	return;
		}

		// Copy structure fields
		new_img->tex_w = source->tex_w;
		new_img->tex_h = source->tex_h;
		new_img->texture = source->texture;
		new_img->texture_has_been_created = TRUE;

		// Compute texture coordinates for subimage
		new_img->tex_y0 = (float)(source->tex_h - source->h) / source->tex_h;
		new_img->tex_y1 = new_img->tex_y0;
		new_img->tex_x0 = (float)rect->x / (float)new_img->tex_w;
		new_img->tex_x1 = (float)(rect->x + rect->w) / (float)new_img->tex_w;
		new_img->tex_y1 += (float)(rect->y + rect->h) / (float)new_img->tex_h;
		new_img->tex_y0 += (float)(rect->y) / (float)new_img->tex_h;

	} else {

		if (!source->surface) {
			ErrorMessage(__FUNCTION__, "Trying to create subimage from image source %x (width %d height %d), but image SDL surface is NULL.", PLEASE_INFORM, IS_WARNING_ONLY, source, source->w, source->h);
			return;
		}

		// Create new surface for subimage
		SDL_Surface *surf = SDL_CreateRGBSurface(0, rect->w, rect->h, 32, rmask, gmask, bmask, amask);
		new_img->surface = SDL_DisplayFormatAlpha(surf);
		SDL_FreeSurface(surf);
				
		// Copy subimage
		SDL_SetAlpha(source->surface, 0, 1);
		SDL_SetAlpha(new_img->surface, SDL_SRCALPHA, 0);
		SDL_SetColorKey(new_img->surface, 0, 0);
		SDL_BlitSurface(source->surface, rect, new_img->surface, NULL);
	}

}

/**
 * The concept of an image involves an SDL_Surface or an OpenGL
 * texture and also suitable offset values, such that the image can be
 * correctly placed in an isometric image.
 * This function loads an image SDL surface, as well as its offset.
 */
void load_image_surface(struct image *img, const char *filename, int use_offset_file)
{
	char fpath[2048];

	if (image_loaded(img)) {
		ErrorMessage(__FUNCTION__, 
				"The image has already been loaded: %s.", PLEASE_INFORM, IS_WARNING_ONLY, filename);
		return;
	}

	find_file(filename, GRAPHICS_DIR, fpath, 0);
	SDL_Surface *surface = our_IMG_load_wrapper(fpath);
	if (surface == NULL) {
		ErrorMessage(__FUNCTION__, "Could not load image\n File name: %s \n", PLEASE_INFORM, IS_FATAL, fpath);
	}
	
	SDL_SetAlpha(surface, 0, SDL_ALPHA_OPAQUE);

	img->surface = our_SDL_display_format_wrapperAlpha(surface);
	img->zoomed_out_surface = NULL;
	img->texture_has_been_created = FALSE;

	SDL_SetColorKey(img->surface, 0, 0);

	SDL_FreeSurface(surface);

	if (use_offset_file)
		get_offset_for_iso_image_from_file_and_path(fpath, img);
	else {
		img->offset_x = 0;
		img->offset_y = 0;
	}

	img->w = img->surface->w;
	img->h = img->surface->h;
}

/**
 * Load an image: load the SDL surface, and make a texture from it in OpenGL mode.
 * \param img Pointer towards the iso_image struct to fill in
 * \param filename Filename of the image
 * \param use_offset_file TRUE if the image uses offset information 
 */
void load_image(struct image *img, const char *filename, int use_offset_file)
{
	load_image_surface(img, filename, use_offset_file);

	if (use_open_gl && (img->w > gl_max_texture_size || img->h > gl_max_texture_size)) {
		ErrorMessage(__FUNCTION__, "Your system only supports %dx%d textures. Image %s is %dx%d and therefore cannot be used as an OpenGL texture.\n", NO_NEED_TO_INFORM, IS_WARNING_ONLY, gl_max_texture_size, gl_max_texture_size, filename, img->w, img->h);
		return;
	}

	if (use_open_gl) {
		make_texture_out_of_surface(img);
	}
}

/**
 * Free the SDL surface associated to an image.
 */
void free_image_surface(struct image *img)
{
	if (img->surface) {
		SDL_FreeSurface(img->surface);
		img->surface = NULL;
	}

	if (img->zoomed_out_surface) {
		SDL_FreeSurface(img->zoomed_out_surface);
		img->zoomed_out_surface = NULL;
	}
}

/**
 * Delete an image completely (free the SDL surface and delete the OpenGL texture).
 */
void delete_image(struct image *img)
{
	free_image_surface(img);

	if (img->texture_has_been_created) {
#ifdef HAVE_LIBGL
		glDeleteTextures(1, &img->texture);
#endif
		img->texture = 0;
	}

	struct image empty = EMPTY_IMAGE;
	memcpy(img, &empty, sizeof(struct image));
}

/**
 * Check if an image has been loaded
 * \param img An image
 * \return TRUE if the image has already been loaded
 */
int image_loaded(struct image *img)
{
	if ((img->surface == NULL) && (!img->texture_has_been_created)) {
		return FALSE;
	}

	return TRUE;
}

/**
 * Create a struct image_transformation from transformation parameters, for use in image display functions.
 */
struct image_transformation set_image_transformation(float scale, float r, float g, float b, float a)
{
	struct image_transformation t = { .surface = NULL, .scale = scale, .r = r, .g = g, .b = b, .a = a };
	return t;
}

