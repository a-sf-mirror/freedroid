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

#ifdef HAVE_LIBGL
static struct dynarray *vtx;
#endif

/**
 * Start rendering images as a batch.
 */
void start_image_batch()
{
	batch_draw = TRUE;
}

#ifdef HAVE_LIBGL
static void gl_emit_quads(void);
#endif

/**
 * End the image batch.
 */
void end_image_batch()
{
	batch_draw = FALSE;
	active_tex = -1;

#ifdef HAVE_LIBGL
	gl_emit_quads();
#endif

	requested_color[0] = -1;
}

#ifdef HAVE_LIBGL
static void gl_emit_quads(void)
{
	if (vtx && vtx->size) {
		glColor4fv(requested_color);

		glVertexPointer(2, GL_FLOAT, 4*sizeof(float), vtx->arr);
		glTexCoordPointer(2, GL_FLOAT, 4*sizeof(float), (void*)((float*)(vtx->arr)+2));
		glDrawArrays(GL_QUADS, 0, vtx->size * 4);

#if DEBUG_QUAD_BORDER
		static float old_r = -1;
		static float old_g = -1;
		static float old_b = -1;
		float r = old_r;
		float g = old_g;
		float b = old_b;
		while (r == old_r) {
			r = ((float)rand_r(&debug_quad_border_seed) / (float)RAND_MAX);
			r = roundf(r * 4.0) / 4.0;
		}
		old_r = r;
		while (g == old_g) {
			g = ((float)rand_r(&debug_quad_border_seed) / (float)RAND_MAX);
			g = roundf(g * 4.0) / 4.0;
		}
		old_g = g;
		while (b == old_b) {
			b = ((float)rand_r(&debug_quad_border_seed) / (float)RAND_MAX);
			b = roundf(b * 4.0) / 4.0;
		}
		old_b = b;
		glColor4f(r, g, b, 1.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_TEXTURE_2D);
		glVertexPointer(2, GL_FLOAT, 4*sizeof(float), vtx->arr);
		glDrawArrays(GL_QUADS, 0, vtx->size * 4);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glColor4fv(requested_color);
		glEnable(GL_TEXTURE_2D);
#endif

		vtx->size = 0;
	}
}
#endif

#ifdef HAVE_LIBGL
static inline void gl_queue_quad(int x1, int y1, int x2, int y2, float tx0, float ty0, float tx1, float ty1)
{
	if (!vtx)
		vtx = dynarray_alloc(16, 16 * sizeof(float));
	
	float v[16] ={ x1, y1, tx0, ty0, x1, y2, tx0, ty1, x2, y2, tx1, ty1, x2, y1, tx1, ty0 }; 

	dynarray_add(vtx, v, 16 * sizeof(float));

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
}
#endif

#ifdef HAVE_LIBGL
static inline void gl_repeat_quad(int x0, int y0, int w, int h, float tx0, float ty0, float tx1, float ty1, float rx, float ry)
{
	int i, j;

	// When an image is displayed in repeated mode, the quad containing the
	// image is to be repeated several times on a rectangular array.
	// The number of repetitions depends on the ratio between the size of the
	// image and the size of the on-screen area to fill, as defined by rx and ry.
	// For example, if the on-screen width is 3 times the image width, then the
	// quad will be repeated 3 times along X.
	// If the ratio between the on-screen width and the image width is a decimal
	// value, then the last repetition of the quad will be 'partial'.
	// For example, if rx = 3.25, we have to repeat the quad 3 times and end
	// with a quarter of the quad width.

	// Tex coords to use on the last column and the last row
	// If rx is an integer value, then decimal_rx = 1.0
	// If rx is not an integer value, then decimal_rx = decimal_part_of(rx)
	float decimal_rx = rx - (ceil(rx) - 1.0);
	float last_column_tx1 = tx0 + (tx1 - tx0) * decimal_rx;
	float decimal_ry = ry - (ceil(ry) - 1.0);
	float last_row_ty1 = ty0 + (ty1 - ty0) * decimal_ry;

	// Create and queue the repeated quad

	int repeat_x = (int)ceil(rx);
	int repeat_y = (int)ceil(ry);

	int current_y0 = y0;
	int current_y1 = y0 + h;
	float current_ty0 = ty0;
	float current_ty1 = ty1;

	for (j = 0; j < repeat_y; j++) {

		int current_x0 = x0;
		int current_x1 = x0 + w;
		float current_tx0 = tx0;
		float current_tx1 = tx1;

		// If last row, use last row values
		if (j == repeat_y - 1) {
			current_y1 = y0 + ry * h;
			current_ty1 = last_row_ty1;
		}

		for (i = 0; i < repeat_x; i++) {

				// If last column, use last column values
				if (i == repeat_x - 1) {
					current_x1 = x0 + rx * w;
					current_tx1 = last_column_tx1;
				}

				gl_queue_quad(current_x0, current_y0, current_x1, current_y1,
				              current_tx0, current_ty0, current_tx1, current_ty1);

				// Prepare for the next column
				current_x0 += w;
				current_x1 += w;
			}

			// Prepare for the next row
			current_y0 += h;
			current_y1 += h;
		}
}
#endif

#ifdef HAVE_LIBGL
/**
 * Draw an image in OpenGL mode.
 *
 * Changes the active texture if necessary, and emits glBegin/glEnd pairs
 * according to the texture switches and whether a batch is required.
 * Applies the image transformation.
 *
 */
static void gl_display_image(struct image *img, int x0, int y0, struct image_transformation *t)
{
	// If the image is empty, don't do anything
	if (!img->texture)
		return;

	// Compute the onscreen position and size of the image according
	// to initial position, image's offset and scale
	int x = x0 + img->offset_x * t->scale_x;
	int y = y0 + img->offset_y * t->scale_y;
	int xmax = x + img->w * t->scale_x;
	int ymax = y + img->h * t->scale_y;

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

	// Queue the image, once or several times depending on the transformation
	// mode to apply

	if (t->mode & REPEATED) {
		gl_repeat_quad(x, y, img->w, img->h, img->tex_x0, img->tex_y0, img->tex_x1, img->tex_y1, t->scale_x, t->scale_y);
	} else {
		gl_queue_quad(x, y, xmax, ymax, img->tex_x0, img->tex_y0, img->tex_x1, img->tex_y1);
	}

	if (!batch_draw) {
		gl_emit_quads();
		active_tex = -1;
		requested_color[0] = -1;
	}

	if (t->mode & HIGHLIGHTED) {
		// Highlight? Draw the texture again with additive blending factors
		// This increases the lightness too much, but is a quick and easy solution
		gl_emit_quads();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		if (t->mode & REPEATED) {
			gl_repeat_quad(x, y, img->w, img->h, img->tex_x0, img->tex_y0, img->tex_x1, img->tex_y1, t->scale_x, t->scale_y);
		} else {
			gl_queue_quad(x, y, xmax, ymax, img->tex_x0, img->tex_y0, img->tex_x1, img->tex_y1);
		}
		gl_emit_quads();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}
#endif

static struct SDL_Surface *repeatSurface(struct image *img, float rx, float ry)
{
	// (see introduction comment of gl_repeat_quad())

	int i, j;

	// Create a new surface containing the repeated image

	struct SDL_Surface *surf = SDL_CreateRGBSurface(0, img->w * rx, img->h * ry, 32, rmask, gmask, bmask, amask);
	struct SDL_Surface *repeated_surf = SDL_DisplayFormatAlpha(surf);
	SDL_FreeSurface(surf);
	SDL_SetAlpha(img->surface, 0, 1);
	SDL_SetAlpha(repeated_surf, SDL_SRCALPHA | SDL_RLEACCEL, 0);

	// Width and height to use on the last column and the last row
	// If rx is an integer value, then decimal_rx = 1.0
	// If rx is not an integer value, then decimal_rx = decimal_part_of(rx)
	float decimal_rx = rx - (ceil(rx) - 1.0);
	int last_column_width = (int)((float)img->w * decimal_rx);
	float decimal_ry = ry - (ceil(ry) - 1.0);
	int last_row_height = (int)((float)img->h * decimal_ry);

	// Create the final image

	int repeat_x = (int)ceil(rx);
	int repeat_y = (int)ceil(ry);

	int blit_y = 0;
	int blit_h = img->h;

	for (j = 0; j < repeat_y; j++) {

		// If last row, adjust the height of the blitted rect
		if (j == repeat_y - 1) {
			blit_h = last_row_height;
		}

		int blit_x = 0;
		int blit_w = img->w;

		for (i = 0; i < repeat_x; i++) {

			// If last column, adjust the width of the blitted rect
			if (i == repeat_x - 1) {
				blit_w = last_column_width;
			}

			SDL_Rect from_rect = { .x = 0, .y = 0, .w = blit_w, .h = blit_h };
			SDL_Rect to_rect   = { .x = blit_x, .y = blit_y, .w = blit_w, .h = blit_h };
			SDL_BlitSurface(img->surface, &from_rect, repeated_surf, &to_rect);

			// Prepare for next column: increment X-position of the destination
			blit_x += img->w;
		}

		// Prepare for next row: increment Y-position of the destination
		blit_y += img->h;
	}

	return repeated_surf;
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

	if (t->scale_x == 1.0 && t->scale_y == 1.0 && !memcmp(&t->c[0], &white[0], sizeof(white)) && !(t->mode & HIGHLIGHTED)) {
		// No transformation
		surf = img->surface;
	} else {
		// Check if the transformation is in cache, and create it if needed
		struct image_transformation *cache = &img->cached_transformation;

		if (!cache->surface || cache->scale_x != t->scale_x || cache->scale_y != t->scale_y || memcmp(&cache->c[0], &t->c[0], sizeof(t->c)) || cache->mode != t->mode) {

			// Transform (if needed) the image, holding it temporarily in the
			// image_transformation structure
			if (t->scale_x == 1.0 && t->scale_y == 1.0) {
				t->surface = img->surface;
			} else {
				if (t->mode & REPEATED) {
					t->surface = repeatSurface(img, t->scale_x, t->scale_y);
				} else {
					t->surface = zoomSurface(img->surface, t->scale_x, t->scale_y, TRUE);
				}
			}

			// Apply color filter on the image
			if (memcmp(&t->c[0], &white[0], sizeof(white)) || (t->mode & HIGHLIGHTED)) {
				SDL_Surface *tmp = sdl_create_colored_surface(t->surface, t->c[0], t->c[1], t->c[2], t->c[3], (t->mode & HIGHLIGHTED) ? 64 : 0);
				if (t->surface != img->surface)
					SDL_FreeSurface(t->surface);
				t->surface = tmp;
			}

			// Cache the transformation we have done
			if (cache->surface) {
				SDL_FreeSurface(cache->surface);
			}
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
#ifdef HAVE_LIBGL
	if (use_open_gl)
		gl_display_image(img, x, y, &t);
	else
#endif
		sdl_display_image(img, x, y, &t);
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
		if (source->texture_type == NO_TEXTURE) {
			new_img->surface = copy_subsurface(source->surface, rect);
			make_texture_out_of_surface(new_img);
			return;
		}

		// Copy structure fields
		new_img->tex_w = source->tex_w;
		new_img->tex_h = source->tex_h;
		new_img->texture = source->texture;
		new_img->texture_type = TEXTURE_CREATED | IS_SUBTEXTURE;

		// Compute texture coordinates for subimage
		new_img->tex_y0 = (float)(source->tex_h - source->h) / source->tex_h;
		new_img->tex_y1 = new_img->tex_y0;
		new_img->tex_x0 = (float)rect->x / (float)new_img->tex_w;
		new_img->tex_x1 = (float)(rect->x + rect->w) / (float)new_img->tex_w;
		new_img->tex_y1 += (float)(rect->y + rect->h) / (float)new_img->tex_h;
		new_img->tex_y0 += (float)(rect->y) / (float)new_img->tex_h;

	} else {

		if (!source->surface) {
			error_message(__FUNCTION__, "Trying to create subimage from image source %p (width %d height %d), but image SDL surface is NULL.", PLEASE_INFORM, source, source->w, source->h);
			return;
		}

		new_img->surface = copy_subsurface(source->surface, rect);
		new_img->texture_type = NO_TEXTURE;
	}

}

/**
 * The concept of an image involves an SDL_Surface or an OpenGL
 * texture and also suitable offset values, such that the image can be
 * correctly placed in an isometric image.
 * This function loads an image SDL surface, as well as its offset.
 */
void load_image_surface(struct image *img, const char *filepath, int mod_flags)
{
	if (image_loaded(img)) {
		error_message(__FUNCTION__,
				"The image has already been loaded: %s.", PLEASE_INFORM, filepath);
		return;
	}

	SDL_Surface *surface = IMG_Load(filepath);
	if (surface == NULL) {
		error_message(__FUNCTION__, "Could not load image.\n File name: %s. IMG_GetError(): %s.", PLEASE_INFORM, filepath, IMG_GetError());
		struct image empty = EMPTY_IMAGE;
		*img = empty;
		return;
	}
	
	SDL_SetAlpha(surface, 0, SDL_ALPHA_OPAQUE);

	img->surface = SDL_DisplayFormatAlpha(surface);
	img->texture_type = NO_TEXTURE;

	SDL_FreeSurface(surface);

	if (mod_flags & USE_OFFSET)
		get_offset_for_iso_image_from_file_and_path(filepath, img);
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
 * \param mod_flags Modifications to apply */
void load_image(struct image *img, int subdir_handle, const char *filename, int mod_flags)
{
	char fpath[PATH_MAX];

	if (mod_flags & USE_WIDE) {
		int need_wide_version = (GameConfig.screen_width / (float)GameConfig.screen_height) >= ((16/9.0 + 4/3.0) / 2.0);
		if (need_wide_version) {
			// Try to load the wide version
			if (find_suffixed_file(filename, "_wide", subdir_handle, fpath, SILENT))
				goto IMAGE_FILE_FOUND;
		}
	}

	// Try to load the narrow version
	if (!find_file(filename, subdir_handle, fpath, PLEASE_INFORM)) {
		struct image empty = EMPTY_IMAGE;
		*img = empty;
		return;
	}

IMAGE_FILE_FOUND:

	load_image_surface(img, fpath, mod_flags);

#ifdef HAVE_LIBGL
	if (use_open_gl && (img->w > gl_max_texture_size || img->h > gl_max_texture_size)) {
		error_message(__FUNCTION__, "Your system only supports %dx%d textures. Image %s is %dx%d and therefore cannot be used as an OpenGL texture.",
				NO_REPORT, gl_max_texture_size, gl_max_texture_size, filename, img->w, img->h);
		return;
	}

	if (use_open_gl) {
		// Create the texture
		make_texture_out_of_surface(img);
	}
#endif
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
		SDL_FreeSurface(img->cached_transformation.surface);
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

#ifdef HAVE_LIBGL
	// Only delete 'master' texture (i.e. no sub-texture)
	if (img->texture_type == TEXTURE_CREATED) {
		glDeleteTextures(1, &img->texture);
	}
#endif

	img->texture =  0;
	img->texture_type = NO_TEXTURE;


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
	if ((img->surface == NULL) && (img->texture_type == NO_TEXTURE)) {
		return FALSE;
	}

	return TRUE;
}

/**
 * Create a struct image_transformation from transformation parameters, for use in image display functions.
 */
struct image_transformation set_image_transformation(float scale_x, float scale_y, float r, float g, float b, float a, int highlight)
{
	enum image_transformation_mode mode = highlight ? HIGHLIGHTED : 0;
	struct image_transformation t = { .surface = NULL, .scale_x = scale_x, .scale_y = scale_y, .c = { r, g, b, a }, .mode = mode };
	return t;
}
