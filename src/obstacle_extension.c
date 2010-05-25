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


#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

int get_obstacle_index(level *lvl, obstacle *o)
{
	int obstacle_index = (o - &(lvl->obstacle_list[0]));

	return obstacle_index;
}

/**
 * Retrieve the obstacle extension of a given type associated to an obstacle.
 */
void *get_obstacle_extension(level *lvl, obstacle *obs, enum obstacle_extension_type type)
{
	int i;
	struct obstacle_extension *ext;

	for (i = 0; i < lvl->obstacle_extensions.size; i++) {
		ext = &ACCESS_OBSTACLE_EXTENSION(lvl->obstacle_extensions, i);
		if (ext->obs == obs) 
			if (ext->type == type)
				return ext->data;
	}

	return NULL;
}

/**
 * Delete the obstacle extension of a given type.
 * This function does not free the associated data - it must have been freed after a call 
 * to get_obstacle_extension.
 * \param lvl Pointer towards the level where the obstacle lies
 * \param type Type of the extension to be removed
 */
void del_obstacle_extension(level *lvl, obstacle *obs, enum obstacle_extension_type type)
{
	int i;
	struct obstacle_extension *ext;

	for (i = 0; i < lvl->obstacle_extensions.size; i++) {
		ext = &ACCESS_OBSTACLE_EXTENSION(lvl->obstacle_extensions, i);

		if (ext->obs != obs)
			continue;

		if (ext->type != type)
			continue;

		dynarray_del(&lvl->obstacle_extensions, i, sizeof(struct obstacle_extension));
	}
}

/**
 * Add a new extension to an obstacle.
 * \param lvl Pointer towards the level where the obstacle lies
 * \param type Type of the extension to add
 * \param data Data of the extension
 */
void add_obstacle_extension(level *lvl, obstacle *obs, enum obstacle_extension_type type, void *data)
{
	struct obstacle_extension ext;
	ext.type = type;
	ext.obs = obs;
	ext.data = data;


	dynarray_add(&lvl->obstacle_extensions, &ext, sizeof(struct obstacle_extension));
}
