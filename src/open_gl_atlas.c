/*
 *
 *   Copyright (c) 2008 Arthur Huillet
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
 * This file provides texture atlas manipulation facility. 
 */

#define _open_gl_atlas_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

extern int gl_max_texture_size;	//defined in open_gl.c 

/** 
 * This function opens a texture atlas and returns its contents.
 * The string returned has to be freed by the caller.
 *
 * @param path Path of the atlas (will be searched for in graphics dir)
 * @return Contents of the file in a freshly malloc'd string.
 */
static char *get_texture_atlas(const char *path)
{
	char fpath[2048];
	find_file(path, GRAPHICS_DIR, fpath, 0);

	char *dataout = ReadAndMallocAndTerminateFile(fpath, NULL);

	if (memcmp(dataout, "size ", 5))
		ErrorMessage(__FUNCTION__, "Atlas file %s did not seem to start with the size of the atlas. Corrupted?\n", PLEASE_INFORM,
			     IS_FATAL, path);

	return dataout;
}

/** 
 * Read the total size of the texture atlas. (FreedroidRPG convention is not to go above 2048x2048.)
 *
 * @param atlasdat The atlas contents as returned by get_texture_atlas.
 * @param atlas_w Pointer to the atlas width to write.
 * @param atlas_h Pointer to the atlas height to write.
 *
 */
static void read_atlas_size(const char *atlasdat, int *atlas_w, int *atlas_h)
{
	char *pos, *epos;

	if (memcmp(atlasdat, "size ", 5))
		ErrorMessage(__FUNCTION__, "Got incorrect string when trying to read atlas size: %10s\n", PLEASE_INFORM, IS_FATAL,
			     atlasdat);

	/* read atlas width and height, and place 'pos' on the first file line */
	pos = (char *)atlasdat + 5;
	epos = pos;
	while (*epos != ' ')
		epos++;
	*epos = 0;
	*atlas_w = atoi(pos);
	*epos = ' ';
	epos++;
	pos = epos;
	while (*epos != '\n')
		epos++;
	*epos = 0;
	*atlas_h = atoi(pos);
	*epos = '\n';
	pos = epos + 1;
}

/**
 * Read a field (x and y position of given file in the atlas).
 *
 * @param atlasdat The atlas contents as returned by get_texture_atlas.
 * @param pathname The name of the file to look for
 * @param field_x Pointer to the X position of the image in the atlas, to be written
 * @param field_y Pointer to the Y position of the image in the atlas, to be written
 */
static void read_atlas_field(const char *atlasdat, const char *pathname, short int *field_x, short int *field_y)
{
	char *field = strstr(atlasdat, pathname);
	char *epos;

	if (!field)
		ErrorMessage(__FUNCTION__, "Atlas file for floor tiles does not contain file %s which is needed.\n", PLEASE_INFORM,
			     IS_FATAL, pathname);

	while (*field != ' ')
		field++;
	field++;
	epos = field;
	while (*epos != ' ')
		epos++;
	*epos = 0;
	*field_x = atoi(field);
	*epos = ' ';
	field = epos + 1;
	epos++;
	while (*epos != '\n')
		epos++;
	*epos = 0;
	*field_y = atoi(field);
	*epos = '\n';
}

int load_texture_atlas(const char *atlas_name, const char *directory, char *filenames[], struct image atlasmembers[], int count)
{
#ifdef HAVE_LIBGL
	// Initialization : read atlas, check if size is ok
	char *dat = get_texture_atlas(atlas_name);
	int atlas_w, atlas_h;
	int a;

	read_atlas_size(dat, &atlas_w, &atlas_h);

	if (atlas_w > gl_max_texture_size || atlas_h > gl_max_texture_size) {
		free(dat);
		ErrorMessage(__FUNCTION__, "Your system only supports %dx%d textures. Atlas %s is %dx%d and therefore will not be used.\n",
			     NO_NEED_TO_INFORM, IS_WARNING_ONLY, gl_max_texture_size, gl_max_texture_size, atlas_name, atlas_w, atlas_h);
		return 1;
	}
	// Create the big atlas surface
	struct image atlas_surf;
	memset(&atlas_surf, 0, sizeof(struct image));
	atlas_surf.w = atlas_w;
	atlas_surf.tex_w = atlas_w;
	atlas_surf.h = atlas_h;
	atlas_surf.tex_h = atlas_h;

	atlas_surf.surface = SDL_CreateRGBSurface(SDL_SWSURFACE, atlas_w, atlas_h, 32, rmask, gmask, bmask, amask);
	SDL_SetAlpha(atlas_surf.surface, 0, SDL_ALPHA_OPAQUE);

	// Iterate over our filenames
	SDL_Rect dest_rect;	//temporary "destination rect"
	for (a = 0; a < count; a++) {
		//printf("Treating %s\n", filenames[a]);
		// Build filename
		char ConstructedFileName[1000];
		char fpath[2048];
		strcpy(ConstructedFileName, (directory != NULL) ? directory : "");
		strcat(ConstructedFileName, filenames[a]);
		find_file(ConstructedFileName, GRAPHICS_DIR, fpath, 0);

		// Load the image
		get_iso_image_from_file_and_path(fpath, &atlasmembers[a], TRUE);

		// Get the destination rect on the atlas surface
		read_atlas_field(dat, filenames[a], &dest_rect.x, &dest_rect.y);
		dest_rect.w = atlasmembers[a].w;
		dest_rect.h = atlasmembers[a].h;

		// Do not ask why. Without this it does not work.
		SDL_SetAlpha(atlasmembers[a].surface, 0, SDL_ALPHA_OPAQUE);

		// Blit on the big atlas surface
		SDL_BlitSurface(atlasmembers[a].surface, NULL, atlas_surf.surface, &dest_rect);

		// Register the texture coordinates of the image in the atlas
		atlasmembers[a].tex_x0 = (float)dest_rect.x / (float)atlas_w;
		atlasmembers[a].tex_y0 = (float)dest_rect.y / (float)atlas_h;
		atlasmembers[a].tex_x1 = atlasmembers[a].tex_x0 + (float)dest_rect.w / (float)atlas_w;
		atlasmembers[a].tex_y1 = atlasmembers[a].tex_y0 + (float)dest_rect.h / (float)atlas_h;

		// Free the temporary
		SDL_FreeSurface(atlasmembers[a].surface);
	}

	// Now we generate the texture
	make_texture_out_of_prepadded_image(&atlas_surf);

	// Mark the texture ID in atlas members
	for (a = 0; a < count; a++)
		atlasmembers[a].texture = atlas_surf.texture;

	// Free atlas data and the temporary surface
	free(dat);
	SDL_FreeSurface(atlas_surf.surface);
#endif
	return 0;
}

#undef _open_gl_atlas_c
