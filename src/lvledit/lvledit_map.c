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

#define _widget_lvledit_map_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_map.h"

/**
 * Move all map labels with the rest of the map
 * \param EditLevel Pointer towards the editing level where all map labels lie
 * \param x The displacement on horizontal axis
 * \param y The displacement on vertical axis
 */
static void move_map_labels(level *EditLevel, int x, int y)
{
	map_label *m;
	int i;

	for (i = 0; i < EditLevel->map_labels.size; i++) {
		// Get the map label
		m = &ACCESS_MAP_LABEL(EditLevel->map_labels, i);

		// Move the map label
		m->pos.x += x;
		m->pos.y += y;

		if (!pos_inside_level(m->pos.x, m->pos.y, EditLevel)) {
			// When the map label is outside of the map, we must remove it
			del_map_label(EditLevel, m->label_name);

			// When we remove a map label from the level, we must decrease the
			// index because the total number of the map labels on this level 
			// has been reduced
			i--;
		}
	}
}

/**
 * Move all items with the rest of the map
 * \param EditLevel Pointer towards the editing level where all items lie
 * \param x The displacement on horizontal axis
 * \param y The displacement on vertical axis
 */
static void move_items(level *EditLevel, int x, int y)
{
	item *item;
	int i;

	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		// Get the item
		item = &EditLevel->ItemList[i];

		// Maybe the item entry isn't used at all. That's the simplest
		// case...: do nothing
		if (item->type <= (-1))
			continue;

		// Move the item
		item->pos.x += x;
		item->pos.y += y;
		
		if (!pos_inside_level(item->pos.x, item->pos.y, EditLevel)) {
			// When the item is outside of the map, we must remove it
			DeleteItem(item);
		}
	}
}

/**
 * Move all obstacles with the rest of the map
 * \param EditLevel Pointer towards the editing level where all obstacles lie
 * \param x The displacement on horizontal axis
 * \param y The displacement on vertical axis
 */
static void move_obstacles(level *EditLevel, int x, int y)
{
	obstacle *o;
	int i;

	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		// Get the obstacle
		o = &EditLevel->obstacle_list[i];

		// Maybe the obstacle entry isn't used at all. That's the simplest
		// case...: do nothing
		if (o->type <= (-1))
			continue;

		if (!pos_inside_level(o->pos.x + x, o->pos.y + y, EditLevel)) {
			// When the obstacle is outside of the map, we must remove it
			del_obstacle(o);
		} else {
			move_obstacle(o, o->pos.x + x, o->pos.y + y);
		}
	}
}

/**
 * Move all waypoints with the rest of the map
 * \param EditLevel Pointer towards the editing level where all waypoints lie
 * \param x The displacement on horizontal axis
 * \param y The displacement on vertical axis
 */
static void move_waypoints(level *EditLevel, int x, int y)
{
	waypoint *wpts = EditLevel->waypoints.arr;
	int i;

	for (i = 0; i < EditLevel->waypoints.size; i++) {
		// Move the waypoint
		wpts[i].x += x;
		wpts[i].y += y;

		if (!pos_inside_level(wpts[i].x, wpts[i].y, EditLevel)) {
			// When the waypoint is outside of the map, we must remove it
			del_waypoint(EditLevel, wpts[i].x, wpts[i].y);

			// When we remove a waypoint from the level, we must decrease the
			// index because the total number of the waypoints on this level 
			// has been reduced
			i--;
		}
	}
}

/**
 * Move all objects (ie. obstacles, map_labels, items, waypoints) with 
 * the rest of the map
 * \param EditLevel Pointer towards the editing level where all objects lie
 * \param x The displacement on horizontal axis
 * \param y The displacement on vertical axis
 */
static void move_all_objects(level *EditLevel, int x, int y)
{
	move_obstacles(EditLevel, x, y);
	move_map_labels(EditLevel, x, y);
	move_items(EditLevel, x, y);
	move_waypoints(EditLevel, x, y);
}

/**
 * Insert a line at the very north of a map
 * \param EditLevel Pointer towards the editing level
 */
void insert_line_north(level *EditLevel)
{
	int i;
	map_tile *tmp;

	if (EditLevel->ylen + 1 >= MAX_MAP_LINES)
		return;

	// To insert a north line, we first extend the level to the south, and then
	// we 'rotate' the map lines
	insert_line_south(EditLevel);
	tmp = EditLevel->map[EditLevel->ylen -1];
	for (i = EditLevel->ylen -1; i > 0; i--) {
		EditLevel->map[i] = EditLevel->map[i-1];
	}
	EditLevel->map[0] = tmp;

	// Now we also have to shift the position of all elements
	move_all_objects(EditLevel, 0, 1);
}

/**
 * Insert a line at the very south of a map
 * \param EditLevel Pointer towards the editing level
 */
void insert_line_south(level *EditLevel)
{
	int i;

	if (EditLevel->ylen + 1 >= MAX_MAP_LINES)
		return;

	EditLevel->ylen++;
	
	// Create the new line, and fill it with default values
	EditLevel->map[EditLevel->ylen - 1] = MyMalloc((EditLevel->xlen + 1) * sizeof(map_tile));
	for (i = 0; i < EditLevel->xlen; i++) {
		init_map_tile(&EditLevel->map[EditLevel->ylen - 1][i]);
	}
}

/**
 * Insert a column at the very east of a map
 * \param EditLevel Pointer towards the editing level
 */
void insert_column_east(level *EditLevel)
{
	int i;
	map_tile *MapPointer;

	if (EditLevel->xlen + 1 >= MAX_MAP_LINES)
		return;

	EditLevel->xlen++;

	// We have to enlarge each map line, and fill those new tiles with default values
	for (i = 0; i < EditLevel->ylen; i++) {
		MapPointer = (map_tile*)realloc(EditLevel->map[i], sizeof(map_tile) * (EditLevel->xlen + 1));
		init_map_tile(&MapPointer[EditLevel->xlen - 1]);
		EditLevel->map[i] = MapPointer;
	}
}

/**
 * Insert a column at the very west of a map
 * \param EditLevel Pointer towards the editing level
 */
void insert_column_west(level *EditLevel)
{
	int i;
	map_tile MapTile;

	if (EditLevel->xlen + 1 >= MAX_MAP_LINES)
		return;

	// To insert a west column, we first extend the level to the east, and then
	// we 'rotate' each line
	insert_column_east(EditLevel);

	for (i = 0; i < EditLevel->ylen; i++) {
		memcpy(&MapTile, &(EditLevel->map[i][EditLevel->xlen - 1]), sizeof(map_tile));
		// REMEMBER:  WE MUST NO USE MEMCPY HERE, CAUSE THE AREAS IN QUESTION OVERLAP !!
		memmove(&(EditLevel->map[i][1]), &(EditLevel->map[i][0]), (EditLevel->xlen-1) * sizeof(map_tile));
		memcpy(&(EditLevel->map[i][0]), &MapTile, sizeof(map_tile));
	}

	// Now we also have to shift the position of all elements
	move_all_objects(EditLevel, 1, 0);
}

/**
 * Remove a column at the very east of a map
 * \param EditLevel Pointer towards the editing level
 */
void remove_column_east(level *EditLevel)
{
	if (EditLevel->xlen - 1 < MIN_MAP_LINES)
		return;

	free_glued_obstacles(EditLevel);

	// Remove a column at the east is always easy, we must just modify the
	// value of size on the horizontal axis, allocation of new memory or other
	// things are not necessary
	EditLevel->xlen--;

	// When we remove a column at the very east, we must not move the elements
	// (ie. obstacles, map labels, items, waypoints) but remove those which are
	// outside of the map
	move_all_objects(EditLevel, 0, 0);
	teleport_to_level_center(EditLevel->levelnum);
}

/**
 * Remove a column at the very west of a map
 * \param EditLevel Pointer towards the editing level
 */
void remove_column_west(level *EditLevel)
{
	int i;
	map_tile *MapPointer;

	if (EditLevel->xlen - 1 < MIN_MAP_LINES)
		return;

	free_glued_obstacles(EditLevel);

	// Now the new memory and everything is done.  All we
	// need to do is move the information to the east
	for (i = 0; i < EditLevel->ylen; i++) {
		memmove(&(EditLevel->map[i][0]), &(EditLevel->map[i][1]), (EditLevel->xlen-1) * sizeof(map_tile));
		MapPointer = (map_tile*)realloc(EditLevel->map[i], (EditLevel->xlen-1) * sizeof(map_tile));
		EditLevel->map[i] = MapPointer;
	}
	EditLevel->xlen--;

	// Now we also have to shift the position of all elements
	move_all_objects(EditLevel, -1, 0);
	teleport_to_level_center(EditLevel->levelnum);
}

/**
 * Remove a line at the very north of a map
 * \param EditLevel Pointer towards the editing level
 */
void remove_line_north(level *EditLevel)
{
	int i;

	if (EditLevel->ylen - 1 < MIN_MAP_LINES)
		return;

	free_glued_obstacles(EditLevel);

	// Now we do some shifting of lines
	free(EditLevel->map[0]);
	for (i = 0; i < EditLevel->ylen - 1; i++) {
		EditLevel->map[i] = EditLevel->map[i + 1];
	}
	EditLevel->map[EditLevel->ylen - 1] = NULL;
	EditLevel->ylen--;

	// Now we also have to shift the position of all elements
	move_all_objects(EditLevel, 0, -1);
	teleport_to_level_center(EditLevel->levelnum);
}

/**
 * Remove a line at the very south of a map
 * \param EditLevel Pointer towards the editing level
 */
void remove_line_south(level *EditLevel)
{
	if (EditLevel->ylen - 1 < MIN_MAP_LINES)
		return;

	free_glued_obstacles(EditLevel);

	// Remove a line at the very south is always easy, we must just modify the
	// value of size on the vertical axis, allocation of new memory or other
	// things are not necessary
	EditLevel->ylen--;

	// When we remove a line at the very south, we must not move the elements
	// (ie. obstacles, map labels, items, waypoints) but remove those which are
	// outside of the map
	move_all_objects(EditLevel, 0, 0);
	teleport_to_level_center(EditLevel->levelnum);
}

/**
 * Save the map - either to the real location, or to the home directory 
 * as a fallback if the main location is not writable.
 *
 */
void save_map(void)
{
	char levels_fn[PATH_MAX];
	char forces_fn[PATH_MAX];

	if (!find_file(levels_fn, MAP_DIR, "levels.dat", NULL, PLEASE_INFORM) || !find_file(forces_fn, MAP_DIR, "ReturnOfTux.droids", NULL, PLEASE_INFORM)) {
		error_message(__FUNCTION__,
		              "Saving levels.dat and ReturnOfTux.droids to %s failed, possibly because of permission issues.\n"
		              "Saving your files to %s instead.",
		              NO_REPORT, data_dirs[MAP_DIR].path, data_dirs[CONFIG_DIR].path);
		alert_window(_("Saving map files to %s/ instead of the default location %s/."), data_dirs[CONFIG_DIR].path, data_dirs[MAP_DIR].name);
		find_file(levels_fn, CONFIG_DIR, "levels.dat", NULL, SILENT);
		find_file(forces_fn, CONFIG_DIR, "ReturnOfTux.droids", NULL, SILENT);
	}

	if ((SaveShip(levels_fn, TRUE, 0) == OK) && (save_special_forces(forces_fn) == OK)) {
		put_string_centered(FPS_Display_Font, 11 * get_font_height(Menu_Font), _("Your ship was saved..."));
		our_SDL_flip_wrapper();
		return;
	}

	error_message(__FUNCTION__, "Wasn't able to save even to %s.", PLEASE_INFORM, data_dirs[CONFIG_DIR].path);
	put_string_centered(FPS_Display_Font, 11 * get_font_height(Menu_Font), _("Your ship can not be saved !!! See the console output..."));
	our_SDL_flip_wrapper();
}
