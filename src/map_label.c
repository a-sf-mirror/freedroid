/* 
 *
 *   Copyright (c) 2010 Samuel Pitoiset
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
#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

/**
 * Add a new map label
 * \param lvl Pointer towards the level where the map label lies
 * \param x The x position of the map label
 * \param y The y position of the map label
 * \param label_name Name of the map label
 */
void add_map_label(level *lvl, int x, int y, char *label_name)
{
	struct map_label map_label;

	// Create a new map label
	map_label.pos.x = x;
	map_label.pos.y = y;
	map_label.label_name = label_name;

	// Add the new map label on the map position
	dynarray_add(&lvl->map_labels, &map_label, sizeof(struct map_label));

	DebugPrintf(0, "\nNew map label added: label_name=%s, pos.x=%d, pos.y=%d, pos.z=%d",
			label_name, x, y, lvl->levelnum);
}

/**
 * Delete the map label
 * \param lvl Pointer towards the level where the map label lies
 * \param label_name Name of the map label
 */
void del_map_label(struct level *lvl, const char *label_name)
{
	int i;

	for (i = 0; i < lvl->map_labels.size; i++) {
		// Get the map label on this level
		struct map_label *map_label = &ACCESS_MAP_LABEL(lvl->map_labels, i);

		if (!strcmp(map_label->label_name, label_name)) {
			// Delete the map label
			free(map_label->label_name);
			map_label->label_name = NULL;
			dynarray_del(&lvl->map_labels, i, sizeof(struct map_label));
			return;
		}
	}
}

void free_map_labels(struct level *lvl)
{
	int i;

	for (i = 0; i < lvl->map_labels.size; i++) {
		// Get the map label on this level
		struct map_label *map_label = &ACCESS_MAP_LABEL(lvl->map_labels, i);
		
		free(map_label->label_name);
	}

	dynarray_free(&lvl->map_labels);
}

/**
 * Retrieve the map_label
 * \param lvl Pointer towards the level where the map label lies
 * \param label_name Name of the map label
 */
struct map_label *get_map_label(struct level *lvl, const char *label_name)
{
	int i;

	for (i = 0; i < lvl->map_labels.size; i++) {
		// Get the map label on this level
		struct map_label *map_label = &ACCESS_MAP_LABEL(lvl->map_labels, i);

		if (!strcmp(map_label->label_name, label_name))
			return map_label;
	}

	return NULL;
}

struct map_label *get_map_label_from_coords(struct level *lvl, float x, float y)
{
	int i;

	for (i = 0; i < lvl->map_labels.size; i++) {
		struct map_label *m = &ACCESS_MAP_LABEL(lvl->map_labels, i);

		if (m->pos.x == floor(x) && m->pos.y == floor(y))
			return m;
	}

	return NULL;
}

/**
 * @brief Check if a map label exists.
 *
 * @param label_name The name of the map label.
 * @return A pointer towards the map label or NULL if it doesn't exist.
 */
struct map_label *map_label_exists(const char *label_name)
{
	map_label *m;
	int i;

	for (i = 0; i < curShip.num_levels; i++) {
		if (!level_exists(i))
			continue;

		m = get_map_label(curShip.AllLevels[i], label_name);
		if (m)
			return m;
	}

	return NULL;
}
