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
// Requested OpenGL color
static float requested_color[4];

// Do we want to draw as a batch? (ie. not emit glBegin/glEnd pairs every time)
static int batch_draw = FALSE;

static struct dynarray *vtx;
static struct dynarray *tex;

/**
 * Start rendering images as a batch.
 */
void start_image_batch()
{
	batch_draw = TRUE;
}

static void gl_emit_quads(void);

/**
 * End the image batch.
 */
void end_image_batch()
{
	batch_draw = FALSE;
	active_tex = -1;

	gl_emit_quads();

	requested_color[0] = -1;
}

static void gl_emit_quads(void)
{
#ifdef HAVE_LIBGL
	if (vtx && vtx->size) {
		glColor4fv(requested_color);

		glVertexPointer(2, GL_FLOAT, 0, vtx->arr);
		glTexCoordPointer(2, GL_FLOAT, 0, tex->arr);
		glDrawArrays(GL_QUADS, 0, vtx->size * 4);
		
		vtx->size = 0;
		tex->size = 0;
	}
#endif
}

static inline void gl_queue_quad(int x1, int y1, int x2, int y2, float tx0, float ty0, float tx1, float ty1)
{
#ifdef HAVE_LIBGL

	if (!vtx)
		vtx = dynarray_alloc(16, 8 * sizeof(float));
	if (!tex)
		tex = dynarray_alloc(16, 8 * sizeof(float));

	float tx[8] = { tx0, ty0, tx0, ty1, tx1, ty1, tx1, ty0 };
	float v[8] = { x1, y1, x1, y2, x2, y2, x2, y1 };

	dynarray_add(vtx, v, 8 * sizeof(float));
	dynarray_add(tex, tx, 8 * sizeof(float));

	/* We have to limit the number of vertices in a single glDrawArrays call.
	   With the r300 driver, having more than 65532 vertices in a vertex array:
	   	- Gallium 0.4 on ATI RV370, Mesa 7.10.2): locks up the GPU, forcing a reboot
		- 2.1 Mesa 7.11-devel (git-fc8c4a3) : ignores vertices above the maximal value, corrupting the rendered image
		
	   This workaround is only justified by the state of the r300 driver at the time of this writing, and shall be
	   removed as soon as r300 is confirmed to work fine with arbitrarily large vertex arrays.
	   */
#define MAX_QUADS 16383
	if (vtx->size >= MAX_QUADS) {
		gl_emit_quads();
	}
#endif
}

/**
 * Draw an image in OpenGL mode.
 * Changes the active texture if necessary, and emits glBegin/glEnd pairs
 * according to the texture switches and whether a batch is required.
 */
static void gl_display_image(struct image *img, int x, int y, struct image_transformation *t)
{
	// If the image is empty, don't do anything
	if (!img->texture)
		return;

#ifdef HAVE_LIBGL
	// Compute image coordinates according to scale factor
	int xmax = img->w;
	int ymax = img->h;
	int xoff = img->offset_x;
	int yoff = img->offset_y;

	xmax *= t->scale_x;
	ymax *= t->scale_y;
	xoff *= t->scale_x;
	yoff *= t->scale_y;

	x += xoff;
	y += yoff;
	xmax += x;
	ymax += y;

	// Bind the texture if required
	if (img->texture != active_tex) {
		gl_emit_quads();
		glBindTexture(GL_TEXTURE_2D, img->texture);
		active_tex = img->texture;
	}

	// Change the active color if required
	if (memcmp(&requested_color[0], &t->c[0], sizeof(requested_color))) {
		gl_emit_quads();
		memcpy(&requested_color[0], &t->c[0], sizeof(requested_color));
	}

	// Draw the image	
	gl_queue_quad(x, y, xmax, ymax, img->tex_x0, img->tex_y0, img->tex_x1, img->tex_y1);

	if (!batch_draw) {
		gl_emit_quads();
		active_tex = -1;
		requested_color[0] = -1;
	}

	if (t->highlight) {
		// Highlight? Draw the texture again with additive blending factors
		// This increases the lightness too much, but is a quick and easy solution
		gl_emit_quads();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		gl_queue_quad(x, y, xmax, ymax, img->tex_x0, img->tex_y0, img->tex_x1, img->tex_y1);
		gl_emit_quads();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

#endif
}

/**
 * Draw an image in SDL mode.
 */
static void sdl_display_image(struct image *img, int x, int y, struct image_transformation *t)
{
	SDL_Rect target_rectangle = { .x = x, .y = y };
	SDL_Surface *surf;

	// If the image is empty, don't do anything
	if (!img->surface)
		return;

	// Check if the image must be transformed at all
	float white[4] = { 1.0, 1.0, 1.0, 1.0 };
	if (t->scale_x == 1.0 && t->scale_y == 1.0 && !memcmp(&t->c[0], &white[0], sizeof(white)) && !t->highlight) {
		// No transformation
		surf = img->surface;
	} else {
		// Transformed image

		// Check if the transformation is in cache
		struct image_transformation *cache = &img->cached_transformation;
		if (!cache->surface || cache->scale_x != t->scale_x || cache->scale_y != t->scale_y || memcmp(&cache->c[0], &t->c[0], sizeof(t->c)) || cache->highlight != t->highlight) {
			// Transformed image is not in cache, create it

			// Scale the image
			int scaled;
			if (t->scale_x != 1.0 || t->scale_y != 1.0) {
				t->surface = zoomSurface(img->surface, t->scale_x, t->scale_y, TRUE); 
				scaled = 1;
			} else {
				t->surface = img->surface;
				scaled = 0;
			}

			// Apply color filter on the image
			if (memcmp(&t->c[0], &white[0], sizeof(white)) || t->highlight) {
				SDL_Surface *tmp = sdl_create_colored_surface(t->surface, t->c[0], t->c[1], t->c[2], t->c[3], t->highlight ? 64 : 0);

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

	target_rectangle.x += img->offset_x * t->scale_x;
	target_rectangle.y += img->offset_y * t->scale_y;

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

static SDL_Surface *copy_subsurface(SDL_Surface *surface, SDL_Rect *rect)
{
	// Create new surface for subimage
	SDL_Surface *surf = SDL_CreateRGBSurface(0, rect->w, rect->h, 32, rmask, gmask, bmask, amask);
	SDL_Surface *new_surf = SDL_DisplayFormatAlpha(surf);
	SDL_FreeSurface(surf);

	// Copy subimage
	SDL_SetAlpha(surface, 0, 1);
	SDL_SetAlpha(new_surf, SDL_SRCALPHA | SDL_RLEACCEL, 0);
	SDL_BlitSurface(surface, rect, new_surf, NULL);

	return new_surf;
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

		// If the source isn't a texture, it means that something bad happened,
		// e.g. OpenGL implementation doesn't support big enough textures.
		// In this case, FreedroidRPG will try to create a separate texture
		// for each subimage.
		if (!source->texture_has_been_created) {
			new_img->surface = copy_subsurface(source->surface, rect);
			make_texture_out_of_surface(new_img);
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

		new_img->surface = copy_subsurface(source->surface, rect);
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
	SDL_Surface *surface = IMG_Load(fpath);
	if (surface == NULL) {
		ErrorMessage(__FUNCTION__, "Could not load image.\n File name: %s. IMG_GetError(): %s.\n", PLEASE_INFORM, IS_WARNING_ONLY, fpath, IMG_GetError());
		struct image empty = EMPTY_IMAGE;
		*img = empty;
		return;
	}
	
	SDL_SetAlpha(surface, 0, SDL_ALPHA_OPAQUE);

	img->surface = SDL_DisplayFormatAlpha(surface);
	img->texture_has_been_created = FALSE;

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
		// Create the texture
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

	if (img->cached_transformation.surface) {
		SDL_FreeSurface(img->surface);
		img->cached_transformation.surface = NULL;
		img->cached_transformation.scale_x = 0.0;
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
struct image_transformation set_image_transformation(float scale_x, float scale_y, float r, float g, float b, float a, int highlight)
{
	struct image_transformation t = { .surface = NULL, .scale_x = scale_x, .scale_y = scale_y, .c = { r, g, b, a }, .highlight = highlight };
	return t;
}
