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
 * Add a waypoint on the map if it doesn't exists
 * \param lvl Pointer towards the level where the waypoint lies
 * \param x The x position of the waypoint
 * \param y The y position of the waypoint
 * \param suppress_random_spawn TRUE if the waypoint is used for the random enemies
 * \return Return the index of the waypoint
 */
int add_waypoint(level *lvl, int x, int y, int suppress_random_spawn)
{
	waypoint w;
	int wpnum;

	wpnum = get_waypoint(lvl, x, y);
	if (wpnum != -1) {
		// When the waypoint already exists on the level, we are done, because 
		// we must not add a waypoint at the same position in order to avoid
		// the conflicts
		return wpnum;
	}

	// Create a new waypoint
	w.x = x;
	w.y = y;
	w.suppress_random_spawn = suppress_random_spawn;
 
 	// Initialize the connections of the waypoint
	dynarray_init(&w.connections, 2, sizeof(int));
 
	// Add the waypoint on the level
	dynarray_add(&lvl->waypoints, &w, sizeof(struct waypoint));

	// Return the index of the new waypoint
	return lvl->waypoints.size - 1;
}

/**
 * Delete a waypoint
 * \param lvl Pointer towards the level where the waypoint lies
 * \param x The x position of the waypoint
 * \param y The y position of the waypoint
 */
void del_waypoint(level *lvl, int x, int y)
{
	// Delete the waypoint on the map
	int wpnum = get_waypoint(lvl, x, y);
	if (wpnum < 0) {
		// When the waypoint doesn't exist on the map, we are done
		return;
	}
	dynarray_del(&lvl->waypoints, wpnum, sizeof(struct waypoint));

	// Delete the connections of the waypoint
	waypoint *wpts = lvl->waypoints.arr;
	int i, j;

	for (i = 0; i < lvl->waypoints.size; i++) {
		int *connections = wpts[i].connections.arr;
 
		for (j = 0; j < wpts[i].connections.size; j++) {
			if (connections[j] == wpnum) {
				// Delete the connection
				dynarray_del(&wpts[i].connections, j, sizeof(int));

				// Just to be sure... check the next connection as well...(they have been shifted!)
				j--;
			} else if (connections[j] > wpnum) {
				connections[j]--;
			}
		}
	}
}

/**
 * Return the index of the waypoint
 * \param lvl Pointer towards the level where the waypoint lies
 * \param x The x position of the waypoint
 * \param y The y position of the waypoint
 * \return Return -1 if it doesn't exists on the level
 */
int get_waypoint(level *lvl, int x, int y)
{
	waypoint *wpts = lvl->waypoints.arr;
 	int i;

	for (i = 0; i < lvl->waypoints.size; i++) {
		if (wpts[i].x == x && wpts[i].y == y) {
			return i;
 		}
 	}

	return -1;
}

void move_waypoint(level *lvl, waypoint *w, int newx, int newy)
{
	if (!pos_inside_level(newx, newy, lvl))
		return;

	w->x = newx;
	w->y = newy;
}
