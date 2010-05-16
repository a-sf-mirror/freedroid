/* 
 *
 *   Copyright (c) 2004-2009 Arthur Huillet
 *   Copyright (c) 2002, 2003 Johannes Prix
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

#define _leveleditor_map_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "SDL_rotozoom.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_map.h"

/**
 *
 *
 */
void move_obstacles_and_items_north_south(float by_what, level * edit_level)
{
	int i;

	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		// Maybe the obstacle entry isn't used at all.  That's the simplest
		// case...: do nothing.
		//
		if (edit_level->obstacle_list[i].type <= (-1))
			continue;

		// Move the obstacle
		//
		edit_level->obstacle_list[i].pos.y += by_what;
		
		// Maybe the obstacle is now outside of the map, so just remove it
		//
		if ((edit_level->obstacle_list[i].pos.y < 0) || (edit_level->obstacle_list[i].pos.y >= edit_level->ylen)) {
			action_remove_obstacle(edit_level, &(edit_level->obstacle_list[i]));
			DebugPrintf(0, "\nRemoved another obstacle in resizing operation.");
			continue;
		}
	}

	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		// Maybe the item entry isn't used at all.  That's the simplest
		// case...: do nothing.
		//
		if (edit_level->ItemList[i].type <= (-1))
			continue;
		if (edit_level->ItemList[i].pos.y <= (-1))
			continue;

		// Move the item
		//
		edit_level->ItemList[i].pos.y += by_what;

		// Maybe the item is is now outside of the map, so just remove it
		//
		if ((edit_level->ItemList[i].pos.y < 0) || (edit_level->ItemList[i].pos.y >= edit_level->ylen)) {
			DeleteItem(&(edit_level->ItemList[i]));
			DebugPrintf(0, "\nRemoved another item in resizing operation.");
			continue;
		}
	}

	for (i = 0; i < MAX_CHEST_ITEMS_PER_LEVEL; i++) {
		// Maybe the chest item entry isn't used at all.  That's the simplest
		// case...: do nothing.
		//
		if (edit_level->ChestItemList[i].type <= (-1))
			continue;
		if (edit_level->ChestItemList[i].pos.y <= (-1))
			continue;

		// Move the chest item
		//
		edit_level->ChestItemList[i].pos.y += by_what;

		// Maybe the chest item is is now outside of the map, so just remove it
		//
		if ((edit_level->ChestItemList[i].pos.y < 0) || (edit_level->ChestItemList[i].pos.y >= edit_level->ylen)) {
			DeleteItem(&(edit_level->ChestItemList[i]));
			DebugPrintf(0, "\nRemoved another chest item in resizing operation.");
			continue;
		}
	}

	glue_obstacles_to_floor_tiles_for_level(edit_level->levelnum);

}				// void move_obstacles_north_south(float by_what, level *edit_level)

/**
 *
 *
 */
void move_obstacles_and_items_west_east(float by_what, level * edit_level)
{
	int i;

	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		// Maybe the obstacle entry isn't used at all.  That's the simplest
		// case...: do nothing.
		//
		if (edit_level->obstacle_list[i].type <= (-1))
			continue;
		if (edit_level->obstacle_list[i].pos.x <= (-1))
			continue;

		// Move the obstacle
		//
		edit_level->obstacle_list[i].pos.x += by_what;

		// Maybe the obstacle is now outside of the map, then just remove it.
		//
		if ((edit_level->obstacle_list[i].pos.x < 0) || (edit_level->obstacle_list[i].pos.x >= edit_level->xlen)) {
			action_remove_obstacle(edit_level, &(edit_level->obstacle_list[i]));
			DebugPrintf(0, "\nRemoved another obstacle in resizing operation.");
			continue;
		}
	}

	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		// Maybe the item entry isn't used at all.  That's the simplest
		// case...: do nothing.
		//
		if (edit_level->ItemList[i].type <= (-1))
			continue;
		if (edit_level->ItemList[i].pos.x <= (-1))
			continue;

		// Move the item
		//
		edit_level->ItemList[i].pos.x += by_what;
		
		// Maybe the item is now outside of the map, then just remove it.
		//
		if ((edit_level->ItemList[i].pos.x < 0) || (edit_level->ItemList[i].pos.x >= edit_level->xlen)) {
			DeleteItem(&(edit_level->ItemList[i]));
			DebugPrintf(0, "\nRemoved another item in resizing operation.");
			continue;
		}
	}

	for (i = 0; i < MAX_CHEST_ITEMS_PER_LEVEL; i++) {
		// Maybe the chest item entry isn't used at all.  That's the simplest
		// case...: do nothing.
		//
		if (edit_level->ChestItemList[i].type <= (-1))
			continue;
		if (edit_level->ChestItemList[i].pos.x <= (-1))
			continue;

		// Move the chest item
		//
		edit_level->ChestItemList[i].pos.x += by_what;

		// Maybe the item now outside of the map, then just remove it.
		//
		if ((edit_level->ChestItemList[i].pos.x < 0) || (edit_level->ChestItemList[i].pos.x >= edit_level->xlen)) {
			DeleteItem(&(edit_level->ChestItemList[i]));
			DebugPrintf(0, "\nRemoved another chest item in resizing operation.");
			continue;
		}
	}

	glue_obstacles_to_floor_tiles_for_level(edit_level->levelnum);

};				// void move_obstacles_and_items_west_east(float by_what, level *edit_level )

/**
 * When a new line is inserted or removed at north of a map, the map labels must move
 * too with the rest of the map.
 */
static void MoveMapLabelsNorthSouth(int ByWhat, level *EditLevel)
{
	int i;

	for (i = 0; i < MAX_MAP_LABELS_PER_LEVEL; i++) {
		if (EditLevel->labels[i].pos.x <= (-1))
			continue;
		EditLevel->labels[i].pos.y += ByWhat;
	}

};				// void MoveMapLabelsSouthOf ( int FromWhere , int ByWhat, level *EditLevel)

/**
 * When a new column is inserted or removed at west of a map, the map labels must move
 * too with the rest of the map.
 */
static void MoveMapLabelsWestEast(int ByWhat, level *EditLevel)
{
	int i;

	for (i = 0; i < MAX_MAP_LABELS_PER_LEVEL; i++) {
		if (EditLevel->labels[i].pos.x <= (-1))
			continue;
		EditLevel->labels[i].pos.x += ByWhat;
	}

};				// void MoveMapLabelsWestEast(int ByWhat, level *EditLevel)

/**
 * When a new line is inserted or removed at north of a map, the waypoints must move
 * too with the rest of the map.
 */
static void MoveWaypointsNorthSouth(int ByWhat, level *EditLevel)
{
	int i;

	for (i = 0; i < MAXWAYPOINTS; i++) {
		if (EditLevel->AllWaypoints[i].x == (0))
			continue;
		EditLevel->AllWaypoints[i].y += ByWhat;
	}

}				// void MoveWaypointsNorthSouth(int ByWhat, level *EditLevel)

/**
 * When a new column is inserted or removed at west of a map, the waypoints must move
 * too with the rest of the map.
 */
static void MoveWaypointsWestEast(int ByWhat, level *EditLevel)
{
	int i;

	for (i = 0; i < MAXWAYPOINTS; i++) {
		if (EditLevel->AllWaypoints[i].x == (0))
			continue;
		EditLevel->AllWaypoints[i].x += ByWhat;
	}

}				// void MoveWaypointsWestEast(int ByWhat, level *EditLevel)

void InsertLineVeryNorth(level * EditLevel)
{
	int i;
	map_tile *tmp;
	
	if (EditLevel->ylen + 1 >= MAX_MAP_LINES)
		return;

	// To insert a north line, we first extend the level to the south, and then
	// we 'rotate' the map lines
	InsertLineVerySouth(EditLevel);
	tmp = EditLevel->map[EditLevel->ylen -1];
	for (i = EditLevel->ylen -1; i > 0; i--) {
		EditLevel->map[i] = EditLevel->map[i-1];
	}
	EditLevel->map[0] = tmp;

	// Now we also have to shift the position of all elements
	MoveWaypointsNorthSouth(+1, EditLevel);
	MoveMapLabelsNorthSouth(+1, EditLevel);
	move_obstacles_and_items_north_south(+1, EditLevel);

}				// void InsertLineVeryNorth ( EditLevel )

void InsertLineVerySouth(level * EditLevel)
{
	int i, j;

	// The enlargement of levels in y direction is limited by a constant
	// defined in defs.h.  This is carefully checked or no operation at
	// all will be performed.
	//
	if (EditLevel->ylen + 1 >= MAX_MAP_LINES)
		return;

	EditLevel->ylen++;
	
	// Create the new line, and fill it with default values
	EditLevel->map[EditLevel->ylen - 1] = MyMalloc((EditLevel->xlen + 1) * sizeof(map_tile));
	for (i = 0; i < EditLevel->xlen; i++) {
		EditLevel->map[EditLevel->ylen - 1][i].floor_value = ISO_FLOOR_SAND;
		for (j = 0; j < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE; j++) {
			EditLevel->map[EditLevel->ylen - 1][i].obstacles_glued_to_here[j] = (-1);
		}
	}

}				// void InsertLineVerySouth (level *EditLevel )

void InsertColumnVeryEast(level * EditLevel)
{
	int i, j;
	map_tile *MapPointer;

	EditLevel->xlen++;

	// We have to enlarge each map line, and fill those new tiles with default values
	
	for (i = 0; i < EditLevel->ylen; i++) {
		MapPointer = (map_tile*)realloc(EditLevel->map[i], sizeof(map_tile) * (EditLevel->xlen + 1));
		MapPointer[EditLevel->xlen - 1].floor_value = ISO_FLOOR_SAND;
		for (j = 0; j < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE; j++) {
			MapPointer[EditLevel->xlen - 1].obstacles_glued_to_here[j] = (-1);
		}
		EditLevel->map[i] = MapPointer;
	}

}				// void InsertColumnVeryEast (level *EditLevel )

void InsertColumnVeryWest(level * EditLevel)
{
	int i;
	map_tile MapTile;

	// To insert a west column, we first extend the level to the east, and then
	// we 'rotate' each line
	InsertColumnVeryEast(EditLevel);

	for (i = 0; i < EditLevel->ylen; i++) {
		memcpy(&MapTile, &(EditLevel->map[i][EditLevel->xlen - 1]), sizeof(map_tile));
		// REMEMBER:  WE MUST NO USE MEMCPY HERE, CAUSE THE AREAS IN QUESTION OVERLAP !!
		memmove(&(EditLevel->map[i][1]), &(EditLevel->map[i][0]), (EditLevel->xlen-1) * sizeof(map_tile));
		memcpy(&(EditLevel->map[i][0]), &MapTile, sizeof(map_tile));
	}

	// Now we also have to shift the position of all elements
	MoveWaypointsWestEast(+1, EditLevel);
	MoveMapLabelsWestEast(+1, EditLevel);
	move_obstacles_and_items_west_east(+1, EditLevel);
	
}				// void InsertColumnVeryWest ( EditLevel )

void RemoveColumnVeryWest(level *EditLevel)
{
	int i;
	map_tile *MapPointer;

	// First we move the obstacles, cause they will be glued and moved and doing that should
	// be done before the floor to glue them to vanishes in the very east.
	//
	// But of course we should glue once more later...
	//
	move_obstacles_and_items_west_east(-1, EditLevel);

	// Now the new memory and everything is done.  All we
	// need to do is move the information to the east
	//
	for (i = 0; i < EditLevel->ylen; i++) {
		memmove(&(EditLevel->map[i][0]), &(EditLevel->map[i][1]), (EditLevel->xlen-1) * sizeof(map_tile));
		MapPointer = (map_tile*)realloc(EditLevel->map[i], (EditLevel->xlen-1) * sizeof(map_tile));
		EditLevel->map[i] = MapPointer;
	}
	EditLevel->xlen--;

	MoveWaypointsWestEast(-1, EditLevel);
	MoveMapLabelsWestEast(-1, EditLevel);

	glue_obstacles_to_floor_tiles_for_level(EditLevel->levelnum);

}				// void RemoveColumnVeryWest(level *EditLevel)

void RemoveLineVeryNorth(level * EditLevel)
{
	int i;

	// First we move the obstacles, cause they will be glued and moved and doing that should
	// be done before the floor to glue them to vanishes in the very south.
	//
	// But of course we should glue once more later...
	//
	move_obstacles_and_items_north_south(-1, EditLevel);

	// Now we do some shifting of lines
	//
	free(EditLevel->map[0]);
	for (i = 0; i < EditLevel->ylen - 1; i++) {
		EditLevel->map[i] = EditLevel->map[i + 1];
	}
	EditLevel->map[EditLevel->ylen - 1] = NULL;
	EditLevel->ylen--;

	// Now we have the waypoints moved as well
	//
	MoveWaypointsNorthSouth(-1, EditLevel);
	MoveMapLabelsNorthSouth(-1, EditLevel);

	// And finally, re-glue all obstacles to the new map
	//
	glue_obstacles_to_floor_tiles_for_level(EditLevel->levelnum);

}				// void RemoveLineVeryNorth (level *EditLevel)
