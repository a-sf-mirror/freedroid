/* 
 *
 *   Copyright (c) 2010 Arthur Huillet
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

#define _floor_tiles_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"

/**
 * This files contains floor tiles related functions.
 */

/**
 * The pathfinder needs to mark the map tiles that were already explored.
 * We use a timestamp as such a mark, in order to avoid to reinitialize all
 * map tiles each time the pathfinder is run.
 * This function increments to current timestamp value.
 */
int next_pathfinder_timestamp(void)
{
	static int pathfinder_timestamp = 0;

	pathfinder_timestamp++;

	if (pathfinder_timestamp == 0) {
		error_message(__FUNCTION__, "The pathfinder timestamp overflowed. This might not be properly handled.", PLEASE_INFORM);
	}

	return pathfinder_timestamp;
}

/**
 * In order to make sure that an obstacle is only displayed/checked once, the collision detection and display code use a timestamp.
 * Every time a new collision detection is started, and every time a new frame is displayed, the timestamp is increased.
 * Obstacles with the same timestamp are obstacles that have already been checked.
 */
int next_glue_timestamp(void)
{
	static int glue_timestamp = 0;

	glue_timestamp++;

	if (glue_timestamp == 0) {
		error_message(__FUNCTION__, "The glue timestamp overflowed. This might not be properly handled.", PLEASE_INFORM);
	}

	return glue_timestamp;
}

void free_glued_obstacles(level *lvl)
{
	int x, y;

	for (x = 0; x < lvl->xlen; x++) {
		for (y = 0; y < lvl->ylen; y++) {
			dynarray_free(&lvl->map[y][x].glued_obstacles);
		}
	}
}

/**
 * Return a floor tile image for a given floor type.
 * A floor type can be either underlay or overlay floor tile.
 */
struct image *get_floor_tile_image(int floor_value)
{
	struct dynarray *floor_tiles = &underlay_floor_tiles;
	// Overlay floor tiles are numbered starting with MAX_UNDERLAY_FLOOR_TILES.
	// This offset is subtracted from floor_value to get index in the floor tiles
	// image array.
	if (floor_value >= MAX_UNDERLAY_FLOOR_TILES) {
		floor_value -= MAX_UNDERLAY_FLOOR_TILES;
		floor_tiles = &overlay_floor_tiles;
	}
	struct floor_tile_spec *floor_tile = dynarray_member(floor_tiles, floor_value, sizeof(struct floor_tile_spec));
	return floor_tile->current_image;
}

#undef _floor_tiles_c
