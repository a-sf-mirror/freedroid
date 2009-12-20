/* 
 *
 *   Copyright (c) 2004-2009 Arthur Huillet
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

#define _leveleditor_grass_actions_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "SDL_rotozoom.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"

static int grass_change_count = 0;

static void grass_change_floor(level *l, int x, int y, int type)
{
	action_set_floor(l, x, y, type);
	grass_change_count++;
}

static void done_beautify_grass()
{
	action_push(ACT_MULTIPLE_ACTIONS, grass_change_count);
	grass_change_count = 0;
}

/**
 * Is this tile a 'full' grass tile, i.e. a grass tile with ABSOLUTELY
 * NO SAND on it?
 */
static int is_full_grass_tile(map_tile * this_tile)
{

	switch (this_tile->floor_value) {
	case ISO_FLOOR_SAND_WITH_GRASS_1:
	case ISO_FLOOR_SAND_WITH_GRASS_2:
	case ISO_FLOOR_SAND_WITH_GRASS_3:
	case ISO_FLOOR_SAND_WITH_GRASS_4:
	case ISO_FLOOR_SAND_WITH_GRASS_25:
	case ISO_FLOOR_SAND_WITH_GRASS_26:
	case ISO_FLOOR_SAND_WITH_GRASS_27:
		return (TRUE);
		break;

	default:
		return (FALSE);
		break;
	}

};				// int is_full_grass_tile ( map_tile* this_tile )

/**
 * Is this tile 'some' grass tile, i.e. a grass tile with JUST ANY BIT
 * OF GRASS ON IT?
 */
static int is_some_grass_tile(map_tile * this_tile)
{

	if (this_tile->floor_value < ISO_FLOOR_SAND_WITH_GRASS_1)
		return (FALSE);
	if (this_tile->floor_value > ISO_FLOOR_SAND_WITH_GRASS_29)
		return (FALSE);

	switch (this_tile->floor_value) {
	case ISO_WATER:
	case ISO_COMPLETELY_DARK:
	case ISO_RED_WAREHOUSE_FLOOR:
		return (FALSE);
		break;
	default:
		break;
	}

	return (TRUE);

};				// int is_full_grass_tile ( map_tile* this_tile )

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

	if (is_full_grass_tile(&(EditLevel->map[y - 1][x])))
		north_grass = TRUE;
	if (is_full_grass_tile(&(EditLevel->map[y + 1][x])))
		south_grass = TRUE;
	if (is_full_grass_tile(&(EditLevel->map[y][x + 1])))
		east_grass = TRUE;
	if (is_full_grass_tile(&(EditLevel->map[y][x - 1])))
		west_grass = TRUE;

	//--------------------
	// Upper left corner:
	//
	if (north_grass && west_grass && (EditLevel->map[y][x + 1].floor_value == ISO_FLOOR_SAND)
	    && (EditLevel->map[y + 1][x].floor_value == ISO_FLOOR_SAND)) {
		grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_15);
	}
	//--------------------
	// Upper right corner
	//
	if (north_grass && east_grass && (EditLevel->map[y][x - 1].floor_value == ISO_FLOOR_SAND)
	    && (EditLevel->map[y + 1][x].floor_value == ISO_FLOOR_SAND)) {
		grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_16);
	}
	//--------------------
	// Lower left corner:
	//
	if (south_grass && west_grass && (EditLevel->map[y][x + 1].floor_value == ISO_FLOOR_SAND)
	    && (EditLevel->map[y - 1][x].floor_value == ISO_FLOOR_SAND)) {
		grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_14);
	}
	//--------------------
	// Lower right corner
	//
	if (south_grass && east_grass && (EditLevel->map[y][x - 1].floor_value == ISO_FLOOR_SAND)
	    && (EditLevel->map[y - 1][x].floor_value == ISO_FLOOR_SAND)) {
		grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_17);
	}

};				// void fix_corners_in_this_grass_tile ( EditLevel , x , y ) 

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
	int x_offset, y_offset;

	if (is_some_grass_tile(&(EditLevel->map[y - 1][x]))) {
		north_grass = TRUE;
		y_offset = -1;
	}
	if (is_some_grass_tile(&(EditLevel->map[y + 1][x]))) {
		south_grass = TRUE;
		y_offset = 1;
	}
	if (is_some_grass_tile(&(EditLevel->map[y][x + 1]))) {
		east_grass = TRUE;
		x_offset = 1;
	}
	if (is_some_grass_tile(&(EditLevel->map[y][x - 1]))) {
		west_grass = TRUE;
		x_offset = -1;
	}
	//--------------------
	// Upper left corner:
	//
	if (north_grass && west_grass && (EditLevel->map[y - 1][x - 1].floor_value == ISO_FLOOR_SAND)) {
		grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_21);
	}
	//--------------------
	// Upper right corner
	//
	if (north_grass && east_grass && (EditLevel->map[y - 1][x + 1].floor_value == ISO_FLOOR_SAND)) {
		grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_20);
	}
	//--------------------
	// Lower left corner:
	//
	if (south_grass && west_grass && (EditLevel->map[y + 1][x - 1].floor_value == ISO_FLOOR_SAND)) {
		grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_18);
	}
	//--------------------
	// Lower right corner
	//
	if (south_grass && east_grass && (EditLevel->map[y + 1][x + 1].floor_value == ISO_FLOOR_SAND)) {
		grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_19);
	}

};				// void fix_anticorners_in_this_grass_tile ( EditLevel , x , y ) 

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

	if (is_some_grass_tile(&(EditLevel->map[y - 1][x])))
		north_grass = TRUE;
	if (is_some_grass_tile(&(EditLevel->map[y + 1][x])))
		south_grass = TRUE;
	if (is_some_grass_tile(&(EditLevel->map[y][x + 1])))
		east_grass = TRUE;
	if (is_some_grass_tile(&(EditLevel->map[y][x - 1])))
		west_grass = TRUE;

	//--------------------
	// Fix sand on the west:
	//
	if (east_grass && !west_grass) {
		if (MyRandom(100) < 50)
			grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_6);
		else
			grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_7);
	}
	//--------------------
	// Fix sand on the east:
	//
	if (!east_grass && west_grass) {
		if (MyRandom(100) < 50)
			grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_9);
		else
			grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_8);
	}
	//--------------------
	// Fix sand on the north:
	//
	if (south_grass && !north_grass) {
		if (MyRandom(100) < 50)
			grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_13);
		else
			grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_12);
	}
	//--------------------
	// Fix sand on the south:
	//
	if (!south_grass && north_grass) {
		if (MyRandom(100) < 50)
			grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_11);
		else
			grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_10);
	}

};				// void fix_halfpieces_in_this_grass_tile ( EditLevel , x , y ) 

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
	int our_rand;

	if (is_some_grass_tile(&(EditLevel->map[y - 1][x])))
		north_grass = TRUE;
	if (is_some_grass_tile(&(EditLevel->map[y + 1][x])))
		south_grass = TRUE;
	if (is_some_grass_tile(&(EditLevel->map[y][x + 1])))
		east_grass = TRUE;
	if (is_some_grass_tile(&(EditLevel->map[y][x - 1])))
		west_grass = TRUE;

	if (!north_grass && !south_grass && !east_grass && !west_grass) {
		DebugPrintf(-4, "\nFixed an isolated grass tile.");
		our_rand = MyRandom(100);
		if (our_rand < 33)
			grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_22);
		else if (our_rand < 66)
			grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_23);
		else
			grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_24);
	}

};				// void fix_isolated_grass_tile ( EditLevel , x , y ) 

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
	map_tile *this_tile;

	DebugPrintf(-4, "\nlevel_editor_beautify_grass_tiles (...): process started...");

	//--------------------
	// First we fix the pure corner pieces, cutting away quit some grass
	// 
	for (x = 1; x < EditLevel->xlen - 1; x++) {
		for (y = 1; y < EditLevel->ylen - 1; y++) {
			this_tile = &(EditLevel->map[y][x]);

			if (is_full_grass_tile(this_tile)) {
				fix_corners_in_this_grass_tile(EditLevel, x, y);
				DebugPrintf(1, "\nlevel_editor_beautify_grass_tiles (...): found a grass tile.");
			}
		}
	}

	//--------------------
	// Now we fix the anticorner pieces, cutting away much less grass
	// 
	for (x = 1; x < EditLevel->xlen - 1; x++) {
		for (y = 1; y < EditLevel->ylen - 1; y++) {
			this_tile = &(EditLevel->map[y][x]);

			if (is_full_grass_tile(this_tile)) {
				fix_anticorners_in_this_grass_tile(EditLevel, x, y);
				DebugPrintf(1, "\nlevel_editor_beautify_grass_tiles (...): found a grass tile.");
			}
		}
	}

	//--------------------
	// Now we fix the halftile pieces, cutting away much less grass
	// 
	for (x = 1; x < EditLevel->xlen - 1; x++) {
		for (y = 1; y < EditLevel->ylen - 1; y++) {
			this_tile = &(EditLevel->map[y][x]);

			if (is_full_grass_tile(this_tile)) {
				fix_halfpieces_in_this_grass_tile(EditLevel, x, y);
				DebugPrintf(1, "\nlevel_editor_beautify_grass_tiles (...): found a grass tile.");
			}
		}
	}

	//--------------------
	// Finally we randomize the full grass tiles
	// 
	for (x = 1; x < EditLevel->xlen - 1; x++) {
		for (y = 1; y < EditLevel->ylen - 1; y++) {
			this_tile = &(EditLevel->map[y][x]);

			if (is_full_grass_tile(this_tile)) {
				our_rand = MyRandom(106);
				if (our_rand < 25)
					grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_1);
				else if (our_rand < 50)
					grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_2);
				else if (our_rand < 75)
					grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_3);
				else if (our_rand < 100)
					grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_4);
				else if (our_rand < 102)
					grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_25);
				else if (our_rand < 104)
					grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_26);
				else
					grass_change_floor(EditLevel, x, y, ISO_FLOOR_SAND_WITH_GRASS_27);
			}
		}
	}

	//--------------------
	// Finally we randomize the full grass tiles
	// 
	for (x = 1; x < EditLevel->xlen - 1; x++) {
		for (y = 1; y < EditLevel->ylen - 1; y++) {
			this_tile = &(EditLevel->map[y][x]);

			if (is_full_grass_tile(this_tile)) {
				fix_isolated_grass_tile(EditLevel, x, y);
				DebugPrintf(1, "\nlevel_editor_beautify_grass_tiles (...): found a grass tile.");
			}
		}
	}

	done_beautify_grass();

};				// void level_editor_beautify_grass_tiles ( void )

#undef _leveleditor_grass_actions_c
