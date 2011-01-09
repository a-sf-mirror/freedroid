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
	char fpath[2048];
	find_file(path, GRAPHICS_DIR, fpath, 0);

	char *dataout = ReadAndMallocAndTerminateFile(fpath, NULL);

	return dataout;
}

static int read_atlas_header(const char *buf, char *path)
{
	int width, height;
	if (!sscanf(buf, "* %s size %d %d", path, &width, &height))
		return 1;

	return 0;
}

static int find_array_index(char *filenames[], int count, char *key)
{
	int i;

	for (i = 0; i < count; i++) {
		if (!strcmp(filenames[i], key))
			return i;
	}

	return -1;
}

int load_texture_atlas(const char *atlas_name, const char *directory, char *filenames[], struct image atlasmembers[], int count)
{
	// Open atlas file
	char *dat = get_texture_atlas(atlas_name);

	while (*dat) {
		// Read data from each atlas described in the file
		char atlas_path[2048];
		strcpy(atlas_path, directory);
		if (read_atlas_header(dat, &atlas_path[strlen(directory)])) {
			// Done reading atlas
			break;
		}

		while (*dat != '\n')
			dat++;
		dat++;

		// Load the atlas in memory
		struct image atlas_img = EMPTY_IMAGE;
		load_image(&atlas_img, atlas_path, FALSE);

		while (*dat && *dat != '*') {
			// Read each element in the atlas
			SDL_Rect dest_rect;
			char element_path[2048];
			int x, y, w, h;
			int xoff, yoff;
			if (!sscanf(dat, "%s %d %d %d %d off %d %d", &element_path[0], &x, &y, &w, &h, &xoff, &yoff)) {
				break;
			}

			dest_rect.x = x;
			dest_rect.y = y;
			dest_rect.w = w;
			dest_rect.h = h;

			int index = find_array_index(filenames, count, element_path);

			if (index == -1) {
				ErrorMessage(__FUNCTION__, "Atlas file %s specifies element %s which is not expected for this atlas.", PLEASE_INFORM, IS_FATAL, atlas_path, element_path);
			}

			// Fill in element struct image
			delete_image(&atlasmembers[index]);
			create_subimage(&atlas_img, &atlasmembers[index], &dest_rect);

			// Set image offset
			atlasmembers[index].offset_x = xoff;
			atlasmembers[index].offset_y = yoff;

			// Move on to the next element
			while (*dat && *dat != '\n')
			   dat++;
			dat++;
		}

		free_image_surface(&atlas_img);
	}

	return 0;
}

#undef _open_gl_atlas_c
