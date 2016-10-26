/* 
 *
 *   Copyright (c) 2004-2010 Arthur Huillet
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

/**
 * This file contains all the actions performed by the level editor, ie. the functions that act on the level.
 */

#define _leveleditor_beautify_actions_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_tool_select.h"

enum _vertical_neighbors {
	NORTH_T = -1,
	SOUTH_T = 1
};

enum _horizontal_neighbors {
	WEST_T = -1,
	EAST_T = 1
};

static int tile_change_count = 0;

/**
 * This function returns the map brick code of the tile that occupies
 * the given position in the top layer.
 * It searches floor layers at the given position to find the first
 * floor tile other than the default floor tile.
 */
static uint16_t get_top_map_brick(level *l, int x, int y)
{
	uint16_t *tiles = get_map_brick(l, x, y);
	int i = l->floor_layers - 1;

	if (!tiles) {
		return ISO_FLOOR_EMPTY;
	}

	while (i && tiles[i] == ISO_FLOOR_EMPTY)
		i--;

	return tiles[i];
}

static void change_transparent_floor(level *l, int x, int y, int transparent_type, int opaque_type)
{
	action_set_floor(l, x, y, opaque_type);
	tile_change_count++;
	action_set_floor(l, x, y, transparent_type);
	tile_change_count++;
}

static void change_opaque_floor(level *l, int x, int y, int type)
{
	change_transparent_floor(l, x, y, type, ISO_FLOOR_EMPTY);
}

static void done_beautify_floor_tiles()
{
	action_push(ACT_MULTIPLE_ACTIONS, tile_change_count);
	tile_change_count = 0;
}

/**
 * Is this tile a 'full' grass tile, i.e. a grass tile with ABSOLUTELY
 * NO SAND on it?
 */
static int is_full_grass_tile(int floor_value)
{
	if (ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_1 <= floor_value && floor_value <= ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_27)
		return TRUE;

	return FALSE;
}

static int is_full_water_tile(int floor_value)
{
	return floor_value == ISO_WATER;
}

/**
 * Is this tile 'some' grass tile, i.e. a grass tile with JUST ANY BIT
 * OF GRASS ON IT?
 */
static int is_some_grass_tile(int floor_value)
{
	if (ISO_OVERLAY_GRASS_01 <= floor_value && floor_value <= ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_27)
		return TRUE;

	return FALSE;
}

static int is_some_water_tile(int floor_value)
{
	switch (floor_value) {
	case ISO_WATER:
	case ISO_WATER_EDGE_1:
	case ISO_WATER_EDGE_2:
	case ISO_WATER_EDGE_3:
	case ISO_WATER_EDGE_4:
	case ISO_WATER_EDGE_5:
	case ISO_WATER_EDGE_6:
	case ISO_WATER_EDGE_7:
	case ISO_WATER_EDGE_8:
	case ISO_WATER_EDGE_9:
	case ISO_WATER_EDGE_10:
	case ISO_WATER_EDGE_11:
	case ISO_WATER_EDGE_12:
	case ISO_WATER_EDGE_13:
	case ISO_WATER_EDGE_14:
		return TRUE;
		break;
	default:
		break;
	}

	return FALSE;
}

/**
 *
 *
 */
static void fix_corners_in_this_grass_tile(level * EditLevel, int x, int y)
{
	int north_grass = 0;
	int south_grass = 0;
	int east_grass = 0;
	int west_grass = 0;

	if (is_full_grass_tile(get_top_map_brick(EditLevel, x, y + NORTH_T)))
		north_grass = TRUE;
	if (is_full_grass_tile(get_top_map_brick(EditLevel, x, y + SOUTH_T)))
		south_grass = TRUE;
	if (is_full_grass_tile(get_top_map_brick(EditLevel, x + EAST_T, y)))
		east_grass = TRUE;
	if (is_full_grass_tile(get_top_map_brick(EditLevel, x + WEST_T, y)))
		west_grass = TRUE;

	// Upper left corner:
	//
	if ((north_grass && west_grass && get_top_map_brick(EditLevel, x + EAST_T, y) == ISO_FLOOR_SAND)
	    && (get_top_map_brick(EditLevel, x, y + SOUTH_T) == ISO_FLOOR_SAND)) {
		change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_10, ISO_FLOOR_SAND);
	}
	// Upper right corner
	//
	if ((north_grass && east_grass && get_top_map_brick(EditLevel, x + WEST_T, y) == ISO_FLOOR_SAND)
	    && (get_top_map_brick(EditLevel, x, y + SOUTH_T) == ISO_FLOOR_SAND)) {
		change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_11, ISO_FLOOR_SAND);
	}
	// Lower left corner:
	//
	if ((south_grass && west_grass && get_top_map_brick(EditLevel, x + EAST_T, y) == ISO_FLOOR_SAND)
	    && (get_top_map_brick(EditLevel, x, y + NORTH_T) == ISO_FLOOR_SAND)) {
		change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_09, ISO_FLOOR_SAND);
	}
	// Lower right corner
	//
	if ((south_grass && east_grass && get_top_map_brick(EditLevel, x + WEST_T, y) == ISO_FLOOR_SAND)
	    && (get_top_map_brick(EditLevel, x, y + NORTH_T) == ISO_FLOOR_SAND)) {
		change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_12, ISO_FLOOR_SAND);
	}
}

static void fix_corners_in_this_water_tile(level *lvl, int x, int y)
{
	int north_water = 0;
	int south_water = 0;
	int east_water = 0;
	int west_water = 0;

	if (is_some_water_tile(get_top_map_brick(lvl, x, y + NORTH_T)))
		north_water = TRUE;
	if (is_some_water_tile(get_top_map_brick(lvl, x, y + SOUTH_T)))
		south_water = TRUE;
	if (is_some_water_tile(get_top_map_brick(lvl, x + EAST_T, y)))
		east_water = TRUE;
	if (is_some_water_tile(get_top_map_brick(lvl, x + WEST_T, y)))
		west_water = TRUE;

	if ((north_water && west_water && get_top_map_brick(lvl, x + EAST_T, y) == ISO_FLOOR_SAND)
	    && (get_top_map_brick(lvl, x, y + SOUTH_T) == ISO_FLOOR_SAND))
		change_opaque_floor(lvl, x, y, ISO_WATER_EDGE_11);

	if ((north_water && east_water && get_top_map_brick(lvl, x + WEST_T, y) == ISO_FLOOR_SAND)
	    && (get_top_map_brick(lvl, x, y + SOUTH_T) == ISO_FLOOR_SAND))
		change_opaque_floor(lvl, x, y, ISO_WATER_EDGE_9);

	if ((south_water && west_water && get_top_map_brick(lvl, x + EAST_T, y) == ISO_FLOOR_SAND)
	    && (get_top_map_brick(lvl, x, y + NORTH_T) == ISO_FLOOR_SAND))
		change_opaque_floor(lvl, x, y, ISO_WATER_EDGE_5);

	if ((south_water && east_water && get_top_map_brick(lvl, x + WEST_T, y) == ISO_FLOOR_SAND)
	    && (get_top_map_brick(lvl, x, y + NORTH_T) == ISO_FLOOR_SAND))
		change_opaque_floor(lvl, x, y, ISO_WATER_EDGE_7);
}

/**
 * Now we fix those grass tiles, that have only very little contact to
 * pure sand tiles, i.e. only 1/8 of the area is grass.
 */
static void fix_anticorners_in_this_grass_tile(level * EditLevel, int x, int y)
{
	int north_grass = 0;
	int south_grass = 0;
	int east_grass = 0;
	int west_grass = 0;

	if (is_some_grass_tile(get_top_map_brick(EditLevel, x, y + NORTH_T))) {
		north_grass = TRUE;
	}
	if (is_some_grass_tile(get_top_map_brick(EditLevel, x, y + SOUTH_T))) {
		south_grass = TRUE;
	}
	if (is_some_grass_tile(get_top_map_brick(EditLevel, x + EAST_T, y))) {
		east_grass = TRUE;
	}
	if (is_some_grass_tile(get_top_map_brick(EditLevel, x + WEST_T, y))) {
		west_grass = TRUE;
	}

	// Upper left corner:
	if (north_grass && west_grass && get_top_map_brick(EditLevel, x + WEST_T, y + NORTH_T) == ISO_FLOOR_SAND) {
		change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_16, ISO_FLOOR_SAND);
	}

	// Upper right corner
	if (north_grass && east_grass && get_top_map_brick(EditLevel, x + EAST_T, y + NORTH_T) == ISO_FLOOR_SAND) {
		change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_15, ISO_FLOOR_SAND);
	}

	// Lower left corner:
	if (south_grass && west_grass && get_top_map_brick(EditLevel, x + WEST_T, y + SOUTH_T) == ISO_FLOOR_SAND) {
		change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_13, ISO_FLOOR_SAND);
	}

	// Lower right corner
	if (south_grass && east_grass && get_top_map_brick(EditLevel, x + EAST_T, y + SOUTH_T) == ISO_FLOOR_SAND) {
		change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_14, ISO_FLOOR_SAND);
	}
}

static void fix_anticorners_in_this_water_tile(level *lvl, int x, int y)
{
	int north_water = 0;
	int south_water = 0;
	int east_water = 0;
	int west_water = 0;

	if (is_some_water_tile(get_top_map_brick(lvl, x, y + NORTH_T)))
		north_water = TRUE;
	if (is_some_water_tile(get_top_map_brick(lvl, x, y + SOUTH_T)))
		south_water = TRUE;
	if (is_some_water_tile(get_top_map_brick(lvl, x + EAST_T, y)))
		east_water = TRUE;
	if (is_some_water_tile(get_top_map_brick(lvl, x + WEST_T, y)))
		west_water = TRUE;

	if (north_water && west_water && get_top_map_brick(lvl, x + WEST_T, y + NORTH_T) == ISO_FLOOR_SAND)
		change_opaque_floor(lvl, x, y, ISO_WATER_EDGE_8);

	if (north_water && east_water && get_top_map_brick(lvl, x + EAST_T, y + NORTH_T) == ISO_FLOOR_SAND)
		change_opaque_floor(lvl, x, y, ISO_WATER_EDGE_6);

	if (south_water && west_water && get_top_map_brick(lvl, x + WEST_T, y + SOUTH_T) == ISO_FLOOR_SAND)
		change_opaque_floor(lvl, x, y, ISO_WATER_EDGE_10);

	if (south_water && east_water && get_top_map_brick(lvl, x + EAST_T, y + SOUTH_T) == ISO_FLOOR_SAND)
		change_opaque_floor(lvl, x, y, ISO_WATER_EDGE_12);

	if (north_water && south_water && west_water && east_water) {
		if (get_top_map_brick(lvl, x + EAST_T, y + NORTH_T) == ISO_FLOOR_SAND
			&& get_top_map_brick(lvl, x + WEST_T, y + SOUTH_T) == ISO_FLOOR_SAND)
			change_opaque_floor(lvl, x, y, ISO_WATER_EDGE_13);

		if (get_top_map_brick(lvl, x + WEST_T, y + NORTH_T) == ISO_FLOOR_SAND
			&& get_top_map_brick(lvl, x + EAST_T, y + SOUTH_T) == ISO_FLOOR_SAND)
			change_opaque_floor(lvl, x, y, ISO_WATER_EDGE_14);
	}
}


/**
 * Now we fix those grass tiles, that have only very little contact to
 * pure sand tiles, i.e. only 1/8 of the area is grass.
 */
static void fix_halfpieces_in_this_grass_tile(level * EditLevel, int x, int y)
{
	int north_grass = 0;
	int south_grass = 0;
	int east_grass = 0;
	int west_grass = 0;

	if (is_some_grass_tile(get_top_map_brick(EditLevel, x, y + NORTH_T)))
		north_grass = TRUE;
	if (is_some_grass_tile(get_top_map_brick(EditLevel, x, y + SOUTH_T)))
		south_grass = TRUE;
	if (is_some_grass_tile(get_top_map_brick(EditLevel, x + EAST_T, y)))
		east_grass = TRUE;
	if (is_some_grass_tile(get_top_map_brick(EditLevel, x + WEST_T, y)))
		west_grass = TRUE;

	// Fix sand on the west:
	//
	if (east_grass && !west_grass) {
		if (MyRandom(100) < 50)
			change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_01, ISO_FLOOR_SAND);
		else
			change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_02, ISO_FLOOR_SAND);
	}
	// Fix sand on the east:
	//
	if (!east_grass && west_grass) {
		if (MyRandom(100) < 50)
			change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_04, ISO_FLOOR_SAND);
		else
			change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_03, ISO_FLOOR_SAND);
	}
	// Fix sand on the north:
	//
	if (south_grass && !north_grass) {
		if (MyRandom(100) < 50)
			change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_08, ISO_FLOOR_SAND);
		else
			change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_07, ISO_FLOOR_SAND);
	}
	// Fix sand on the south:
	//
	if (!south_grass && north_grass) {
		if (MyRandom(100) < 50)
			change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_06, ISO_FLOOR_SAND);
		else
			change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_05, ISO_FLOOR_SAND);
	}
}

static void fix_halfpieces_in_this_water_tile(level *lvl, int x, int y)
{
	int north_water = 0;
	int south_water = 0;
	int east_water = 0;
	int west_water = 0;

	if (is_some_water_tile(get_top_map_brick(lvl, x, y + NORTH_T)))
		north_water = TRUE;
	if (is_some_water_tile(get_top_map_brick(lvl, x, y + SOUTH_T)))
		south_water = TRUE;
	if (is_some_water_tile(get_top_map_brick(lvl, x + EAST_T, y)))
		east_water = TRUE;
	if (is_some_water_tile(get_top_map_brick(lvl, x + WEST_T, y)))
		west_water = TRUE;

	// Fix sand on the west:
	//
	if (east_water && !west_water)
		change_opaque_floor(lvl, x, y, ISO_WATER_EDGE_4);
	// Fix sand on the east:
	//
	if (!east_water && west_water)
		change_opaque_floor(lvl, x, y, ISO_WATER_EDGE_2);
	// Fix sand on the north:
	//
	if (south_water && !north_water)
		change_opaque_floor(lvl, x, y, ISO_WATER_EDGE_1);
	// Fix sand on the south:
	//
	if (!south_water && north_water)
		change_opaque_floor(lvl, x, y, ISO_WATER_EDGE_3);
}

/**
 *
 *
 */
static void fix_isolated_grass_tile(level * EditLevel, int x, int y)
{
	int north_grass = 0;
	int south_grass = 0;
	int east_grass = 0;
	int west_grass = 0;

	if (is_some_grass_tile(get_top_map_brick(EditLevel, x, y + NORTH_T)))
		north_grass = TRUE;
	if (is_some_grass_tile(get_top_map_brick(EditLevel, x, y + SOUTH_T)))
		south_grass = TRUE;
	if (is_some_grass_tile(get_top_map_brick(EditLevel, x + EAST_T, y)))
		east_grass = TRUE;
	if (is_some_grass_tile(get_top_map_brick(EditLevel, x + WEST_T, y)))
		west_grass = TRUE;

	if (!north_grass && !south_grass && !east_grass && !west_grass) {
		int our_rand = MyRandom(100);
		if (our_rand < 33)
			change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_17, ISO_FLOOR_SAND);
		else if (our_rand < 66)
			change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_18, ISO_FLOOR_SAND);
		else
			change_transparent_floor(EditLevel, x, y, ISO_OVERLAY_GRASS_19, ISO_FLOOR_SAND);
	}
}

/**
 * When planting grass tiles, taking care of the smooth borders for these
 * grass tiles can be a tedious job.  A short function might help to 
 * beautify the grass.  If focuses on replacing 'full' grass tiles with 
 * proper full and part-full grass tiles.
 */
void level_editor_beautify_grass_tiles(level * EditLevel)
{
	int x;
	int y;
	int our_rand;
	int this_tile_value;
	int xstart = 0, xend = EditLevel->xlen;
	int ystart = 0, yend = EditLevel->ylen;

	if (selection_type() == OBJECT_FLOOR && !selection_empty()) {
		point start = selection_start(), len = selection_len();
		xstart = start.x;
		xend = start.x + len.x;
		ystart = start.y;
		yend = start.y + len.y;
	}

	// Make sure the level has multilayer floor
	if (EditLevel->floor_layers == 1)
		EditLevel->floor_layers++;

	// First we fix the pure corner pieces, cutting away quit some grass
	// 
	for (x = xstart; x < xend; x++) {
		for (y = ystart; y < yend; y++) {
			this_tile_value = get_top_map_brick(EditLevel, x, y);

			if (is_full_grass_tile(this_tile_value)) {
				fix_corners_in_this_grass_tile(EditLevel, x, y);
			}
		}
	}

	// Now we fix the anticorner pieces, cutting away much less grass
	// 
	for (x = xstart; x < xend; x++) {
		for (y = ystart; y < yend; y++) {
			this_tile_value = get_top_map_brick(EditLevel, x, y);

			if (is_full_grass_tile(this_tile_value)) {
				fix_anticorners_in_this_grass_tile(EditLevel, x, y);
			}
		}
	}

	// Now we fix the halftile pieces, cutting away much less grass
	// 
	for (x = xstart; x < xend; x++) {
		for (y = ystart; y < yend; y++) {
			this_tile_value = get_top_map_brick(EditLevel, x, y);

			if (is_full_grass_tile(this_tile_value)) {
				fix_halfpieces_in_this_grass_tile(EditLevel, x, y);
			}
		}
	}

	// Finally we randomize the full grass tiles
	// 
	for (x = xstart; x < xend; x++) {
		for (y = ystart; y < yend; y++) {
			this_tile_value = get_top_map_brick(EditLevel, x, y);

			if (is_full_grass_tile(this_tile_value)) {
				our_rand = MyRandom(106);
				if (our_rand < 25)
					change_opaque_floor(EditLevel, x, y, ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_1);
				else if (our_rand < 50)
					change_opaque_floor(EditLevel, x, y, ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_2);
				else if (our_rand < 75)
					change_opaque_floor(EditLevel, x, y, ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_3);
				else if (our_rand < 100)
					change_opaque_floor(EditLevel, x, y, ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_4);
				else if (our_rand < 102)
					change_opaque_floor(EditLevel, x, y, ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_25);
				else if (our_rand < 104)
					change_opaque_floor(EditLevel, x, y, ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_26);
				else
					change_opaque_floor(EditLevel, x, y, ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_27);
			}
		}
	}

	// Finally we randomize the isolated grass tiles
	// 
	for (x = xstart; x < xend; x++) {
		for (y = ystart; y < yend; y++) {
			this_tile_value = get_top_map_brick(EditLevel, x, y);

			if (is_full_grass_tile(this_tile_value)) {
				fix_isolated_grass_tile(EditLevel, x, y);
			}
		}
	}

	done_beautify_floor_tiles();
}

void level_editor_beautify_water_tiles(level *lvl)
{
	int x, y;
	int xstart = 0, xend = lvl->xlen;
	int ystart = 0, yend = lvl->ylen;

	if (selection_type() == OBJECT_FLOOR && !selection_empty()) {
		point start = selection_start(), len = selection_len();
		xstart = start.x;
		xend = start.x + len.x;
		ystart = start.y;
		yend = start.y + len.y;
	}

	// Water anticorners
	for (x = xstart; x < xend; x++) {
		for (y = ystart; y < yend; y++) {
			int floor_value = get_top_map_brick(lvl, x, y);

			if (is_full_water_tile(floor_value))
				fix_anticorners_in_this_water_tile(lvl, x, y);
		}
	}

	// Water corners
	for (x = xstart; x < xend; x++) {
		for (y = ystart; y < yend; y++) {
			int floor_value = get_top_map_brick(lvl, x, y);

			if (is_full_water_tile(floor_value))
				fix_corners_in_this_water_tile(lvl, x, y);
		}
	}

	// Water halfpieces
	for (x = xstart; x < xend; x++) {
		for (y = ystart; y < yend; y++) {
			int floor_value = get_top_map_brick(lvl, x, y);

			if (is_full_water_tile(floor_value))
				fix_halfpieces_in_this_water_tile(lvl, x, y);
		}
	}

	done_beautify_floor_tiles();
}

#undef _leveleditor_beautify_actions_c
