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

/** 
 * This function opens a texture atlas and returns its contents.
 * The string returned has to be freed by the caller.
 *
 * @param path Path of the atlas (will be searched for in graphics dir)
 * @return Contents of the file in a freshly malloc'd string.
 */
static char *get_texture_atlas(const char *path)
{
	char fpath[PATH_MAX];
	if (find_file(path, GRAPHICS_DIR, fpath, PLEASE_INFORM)) {
		char *dataout = ReadAndMallocAndTerminateFile(fpath, NULL);
		return dataout;
	}
	return NULL;
}

int load_texture_atlas(const char *atlas_name, const char *directory, struct image *(*get_storage_for_key)(const char *key))
{
	int loaded_any_subimage = FALSE;

	// Ensure that 'directory' length is not too large, to have enough room to
	// read the image names
	char atlas_path[2048];
	int directory_len = strlen(directory);
	if (directory_len > 1023) {
		error_message(__FUNCTION__, "The path of atlas files is too large (>1023): %s", NO_REPORT, directory);
		return 1;
	}
	strcpy(atlas_path, directory);

	// Open atlas file
	char *dat = get_texture_atlas(atlas_name);
	char *atlas_data = dat;

	while (*dat) {
		// Read data from each atlas described in the file
		int width, height;
		char filename[1024];
		if (!sscanf(dat, "* %1023s size %d %d", filename, &width, &height)) {
			// Done reading atlas
			break;
		}
		strcpy(&atlas_path[directory_len], filename);

		while (*dat != '\n')
			dat++;
		dat++;

		// Load the atlas in memory
		struct image atlas_img = EMPTY_IMAGE;
		load_image(&atlas_img, atlas_path, FALSE);

		while (*dat && *dat != '*') {
			// Read each element in the atlas
			SDL_Rect dest_rect;
			char element_key[1024];
			int x, y, w, h;
			int xoff, yoff;
			if (!sscanf(dat, "%1023s %d %d %d %d off %d %d", &element_key[0], &x, &y, &w, &h, &xoff, &yoff)) {
				break;
			}

			dest_rect.x = x;
			dest_rect.y = y;
			dest_rect.w = w;
			dest_rect.h = h;

			struct image *img = get_storage_for_key(element_key);
			if (img) {
				// Fill in element struct image
				delete_image(img);
				create_subimage(&atlas_img, img, &dest_rect);

				// Set image offset
				img->offset_x = xoff;
				img->offset_y = yoff;

				loaded_any_subimage = TRUE;
			}

			// Move on to the next element
			while (*dat && *dat != '\n')
			   dat++;
			dat++;
		}

		// Free atlas SDL surface.
		// If no subimage was loaded the OpenGL texture is also deleted
		// in order to prevent a potential resource leak.
		if (!loaded_any_subimage)
			delete_image(&atlas_img);
		else
			free_image_surface(&atlas_img);
	}

	free(atlas_data);
	return 0;
}

#undef _open_gl_atlas_c
