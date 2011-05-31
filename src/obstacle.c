/**
 *
 *  Copyright (c) 2011 Samuel Pitoiset
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

#define _obstacle_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"

/**
 * This files contains obstacles related functions.
 */

/**
 * \brief Add a new obstacle without glue it on the map.
 */
obstacle *add_obstacle(level *lvl, float x, float y, int type)
{
	int i;

	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		if (lvl->obstacle_list[i].type != -1)
			continue;

		lvl->obstacle_list[i].pos.x = x;
		lvl->obstacle_list[i].pos.y = y;
		lvl->obstacle_list[i].pos.z = lvl->levelnum;
		lvl->obstacle_list[i].type = type;
		lvl->obstacle_list[i].timestamp = 0;

		return &lvl->obstacle_list[i];
	}

	ErrorMessage(__FUNCTION__, "\
	    Ran out of obstacle positions (%d) in level %d !", PLEASE_INFORM, IS_FATAL, MAX_OBSTACLES_ON_MAP, lvl->levelnum);

	return NULL;
}

/**
 * \brief Remove an obstacle with its extensions and unglue it from the map.
 */
void del_obstacle(obstacle *o)
{
	level *lvl = curShip.AllLevels[o->pos.z];

	o->type = -1;

	del_obstacle_extensions(lvl, o);

	// Now doing that must have shifted the glue!  That is a problem.  We need to
	// reglue everything to the map...
	glue_obstacles_to_floor_tiles_for_level(lvl->levelnum);
}

obstacle_spec *get_obstacle_spec(int index)
{
	return (obstacle_spec *)obstacle_map.arr + index;
}

#undef _obstacle_c
