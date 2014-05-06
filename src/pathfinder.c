/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002       Reinhard Prix
 *   Copyright (c) 2004-2008        Arthur Huillet
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

#define _pathfinder_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

static moderately_finepoint last_sight_contact;

static int recursive_find_walkable_point(int, float, float, float, float, int, moderately_finepoint *, int *, int, pathfinder_context *);
static void streamline_intermediate_course(gps *, moderately_finepoint *, int, pathfinder_context *);

#define TILE_IS_UNPROCESSED 3
#define TILE_IS_PROCESSED 4

/**
 * In case that Tux or a bot cannot walk the direct line from his current
 * position to the mouse move target, we must set up a path composed of
 * several smaller direct line.
 *
 * The pathfinder algorithm is based on the classical A* algorithm
 *
 * curpos and move_target are 'virtual positions' defined relatively to Tux's or bot's
 * current level.
 */
int set_up_intermediate_course_between_positions(gps *curpos, moderately_finepoint *move_target, moderately_finepoint *waypoints,
						 int maxwp, pathfinder_context *ctx)
{
	int i;
	moderately_finepoint tmp;

	// If the target position cannot be reached at all, because of being inside an obstacle
	// or blocked by a bot for example, then no path can be computed.
	//
	if (!SinglePointColldet(move_target->x, move_target->y, curpos->z, ctx->dlc_filter) ||
        ((ctx->frw_ctx != NULL) && !location_free_of_droids(move_target->x, move_target->y, curpos->z, ctx->frw_ctx))) {
		return (FALSE);
	}

	// First we clear out the position grid and initialize the target
	// point, which will be the result of the recursion.
	//
	ctx->timestamp = next_pathfinder_timestamp();

	clear_out_intermediate_points(curpos, waypoints, maxwp);

	int next_index_to_set_up = 0;

	if (!recursive_find_walkable_point
	    (curpos->z, curpos->x, curpos->y, move_target->x, move_target->y, 0, waypoints, &next_index_to_set_up, maxwp, ctx)) {
		return FALSE;
	}

	// We invert the intermediate waypoint list, because the Tux is coming from the
	// other side of course (that's due to the recursion call)...
	//
	for (i = 0; i < next_index_to_set_up / 2; i++) {
		tmp.x = waypoints[i].x;
		tmp.y = waypoints[i].y;

		waypoints[i].x = waypoints[next_index_to_set_up - 1 - i].x;
		waypoints[i].y = waypoints[next_index_to_set_up - 1 - i].y;

		waypoints[next_index_to_set_up - 1 - i].x = tmp.x;
		waypoints[next_index_to_set_up - 1 - i].y = tmp.y;
	}

	streamline_intermediate_course(curpos, waypoints, maxwp, ctx);

	return (TRUE);
}

/**
 *
 *
 */
void clear_out_intermediate_points(gps * curpos, moderately_finepoint * intermediate_points, int size)
{
	int i;

	// We clear out the waypoint list for the Tux and initialize the
	// very first entry.
	//
	intermediate_points[0].x = curpos->x;
	intermediate_points[0].y = curpos->y;
	for (i = 1; i < size; i++) {
		intermediate_points[i].x = (-1);
		intermediate_points[i].y = (-1);
	}

}				// void clear_out_intermediate_points ( )

/**
 * All positions are 'virtual positions' defined relatively to Tux's or bot's
 * current level
 */
static int recursive_find_walkable_point(int levelnum, float x1, float y1, float x2, float y2, int recursion_depth,
					 moderately_finepoint * waypoints, int *next_index_to_set_up, int maxwp, pathfinder_context * ctx)
{
	moderately_finepoint ordered_moves[4];
	int i;

#define MAX_RECUSION_DEPTH 50

	// At first we mark the current position as processed...
	{
		// We need to convert (X1,Y1,L) into a real position to get
		// the reference of the map tile to mark.
		gps vpos = { x1, y1, levelnum };
		gps rpos;
		resolve_virtual_position(&rpos, &vpos);
		level *lvl = curShip.AllLevels[rpos.z];
		lvl->map[(int)rpos.y][(int)rpos.x].timestamp = ctx->timestamp;
	}

	// Maybe the recursion is too deep already.  Then we just return and report
	// failure.
	//
	if (recursion_depth > MAX_RECUSION_DEPTH)
		return (FALSE);

	// If we can reach the final destination from here, then there is no need to
	// go any further, but instead we select the current position as the preliminary
	// walkable target for the Tux.
	//
	if (DirectLineColldet(x1, y1, x2, y2, levelnum, ctx->dlc_filter)
	    && ((ctx->frw_ctx == NULL) || way_free_of_droids(x1, y1, x2, y2, levelnum, ctx->frw_ctx))
	    ) {
		// if the target position is directly reachable then we are done !
		waypoints[0].x = x2;
		waypoints[0].y = y2;
		waypoints[1].x = x1;
		waypoints[1].y = y1;
		*next_index_to_set_up = 2;
		return (TRUE);
	}

	// So at this point we know that the current position is not one from where
	// we would be able to reach our goal.
	//
	// Therefore we will try other positions that might bring us more luck, but
	// we only try such positions, as we can reach from here...
	//
	// And also we will try the 'more promising' directions before the 'less promising'
	// ones...
	//
	if (fabsf(x1 - x2) >= fabsf(y1 - y2)) {
		// More priority on x move into the right direction, least
		// priority on x move into the wrong direction.
		//
		if (x1 <= x2) {
			ordered_moves[0].x = 1.0;
			ordered_moves[0].y = 0.0;
			ordered_moves[3].x = -1.0;
			ordered_moves[3].y = 0.0;
		} else {
			ordered_moves[3].x = 1.0;
			ordered_moves[3].y = 0.0;
			ordered_moves[0].x = -1.0;
			ordered_moves[0].y = 0.0;
		}
		if (y1 <= y2) {
			ordered_moves[1].x = 0.0;
			ordered_moves[1].y = 1.0;
			ordered_moves[2].x = 0.0;
			ordered_moves[2].y = -1.0;
		} else {
			ordered_moves[2].x = 0.0;
			ordered_moves[2].y = 1.0;
			ordered_moves[1].x = 0.0;
			ordered_moves[1].y = -1.0;
		}
	} else {
		// More prority on x move into the right direction, least
		// priority on x move into the wrong direction.
		//
		if (x1 <= x2) {
			ordered_moves[1].x = 1.0;
			ordered_moves[1].y = 0.0;
			ordered_moves[2].x = -1.0;
			ordered_moves[2].y = 0.0;
		} else {
			ordered_moves[2].x = 1.0;
			ordered_moves[2].y = 0.0;
			ordered_moves[1].x = -1.0;
			ordered_moves[1].y = 0.0;
		}
		if (y1 <= y2) {
			ordered_moves[0].x = 0.0;
			ordered_moves[0].y = 1.0;
			ordered_moves[3].x = 0.0;
			ordered_moves[3].y = -1.0;
		} else {
			ordered_moves[3].x = 0.0;
			ordered_moves[3].y = 1.0;
			ordered_moves[0].x = 0.0;
			ordered_moves[0].y = -1.0;
		}
	}

	// Now that we have set up our walk preferences, we can start to try out the directions we have...
	//

	for (i = 0; i < 4; i++) {
		float centered_x = rintf(x1 + ordered_moves[i].x + 0.5) - 0.5;
		float centered_y = rintf(y1 + ordered_moves[i].y + 0.5) - 0.5;
		gps vpos = { centered_x, centered_y, levelnum };
		gps rpos;
		resolve_virtual_position(&rpos, &vpos);
		level *lvl = curShip.AllLevels[rpos.z];

		if ((lvl->map[(int)rpos.y][(int)rpos.x].timestamp != ctx->timestamp)
		    && DirectLineColldet(x1, y1, centered_x, centered_y, levelnum, ctx->dlc_filter)
		    && ((ctx->frw_ctx == NULL) || way_free_of_droids(x1, y1, centered_x, centered_y, levelnum, ctx->frw_ctx))
		    ) {
			last_sight_contact.x = x1;
			last_sight_contact.y = y1;

			if (recursive_find_walkable_point(levelnum,
							  centered_x, centered_y,
							  x2, y2, recursion_depth + 1, waypoints, next_index_to_set_up, maxwp, ctx)) {
				// If there is still sight contact to the waypoint closer to the target, we just set this
				// waypoint.
				// Otherwise we set THE NEXT WAYPOINT.
				//
				waypoints[*next_index_to_set_up].x = centered_x;
				waypoints[*next_index_to_set_up].y = centered_y;
				(*next_index_to_set_up)++;

				if ((*next_index_to_set_up) >= maxwp) {
					gps a = { x1, y1, 0 };
					clear_out_intermediate_points(&a, waypoints, maxwp);
					(*next_index_to_set_up) = 0;
				}

				return (TRUE);
			}
		}
	}

	// Here we know, that we didn't have any success finding some possible point...
	//
	return (FALSE);

}

/**
 * After a course has been set up, the Tux (or the bot) can start to
 * proceed towards his target.  However, the unmodified recursive course
 * is often a bit awkward and goes back and forth a lot.
 *
 * Therefore it will be a good idea to streamline the freshly set up
 * course first, once and for all, before the Tux or the bot is finally set in
 * motion.
 *
 */
static void streamline_intermediate_course(gps * curpos, moderately_finepoint * waypoints, int maxwp, pathfinder_context * ctx)
{
	int start_index;
	int last_index = -10;
	int scan_index;
	int cut_away;

	DebugPrintf(DEBUG_TUX_PATHFINDING, "\nOPTIMIZATION --> streamline_tux_intermediate_course: starting...");

	// We process each index position of the course, starting with the point
	// where the tux will be starting.
	//
	for (start_index = 0; start_index < maxwp; start_index++) {
		// If the end of the course is reached in the outer loop, then we're done indeed
		// with the streamlining process and therefore can go home now...
		//
		if (waypoints[start_index].x == (-1))
			break;

		// Start of inner streamlining loop:
		// We eliminate every point from here on up to the last point in the
		// course, that can still be reached from here.
		//
		last_index = (-1);
		for (scan_index = start_index + 1; scan_index < maxwp; scan_index++) {
			// If we've reached the end of the course this way, then we know how much
			// we can cut away and can quit the inner loop here.
			//
			if (waypoints[scan_index].x == (-1))
				break;

			// Otherwise we check if maybe this is (another) reachable intermediate point (AGAIN?)
			//
			if (DirectLineColldet
			    (waypoints[start_index].x, waypoints[start_index].y, waypoints[scan_index].x, waypoints[scan_index].y,
			     curpos->z, ctx->dlc_filter)
			    && ((ctx->frw_ctx == NULL)
				|| way_free_of_droids(waypoints[start_index].x, waypoints[start_index].y, waypoints[scan_index].x,
							    waypoints[scan_index].y, curpos->z, ctx->frw_ctx))
			    ) {
				last_index = scan_index;
			}
		}

		// Maybe the result of the scan indicated, that there is nothing to cut away at this
		// point.  Then we must continue right after this point.
		//
		if (last_index == (-1))
			continue;

		// Now we know how much to cut away.  So we'll do it.
		//
		for (cut_away = 0; (start_index + 1 + cut_away) < maxwp; cut_away++) {
			if (last_index + cut_away < maxwp) {
				waypoints[start_index + 1 + cut_away].x = waypoints[last_index + 0 + cut_away].x;
				waypoints[start_index + 1 + cut_away].y = waypoints[last_index + 0 + cut_away].y;
			} else {
				waypoints[start_index + 1 + cut_away].x = (-1);
				waypoints[start_index + 1 + cut_away].y = (-1);
			}
		}
	}

	// At this point the waypoint history is fairly good.  However, it might
	// be, that the very first waypoint entry (index 0) is not necessary and
	// did not get overwritten in the process above, because the original course
	// setup function does not usually include the current position of the Tux
	// as the very first entry.
	//
	// Therefore we do some extra optimization check here for this special case...
	//
	if (waypoints[1].x == -1)
		return;

	if (DirectLineColldet(curpos->x, curpos->y, waypoints[1].x, waypoints[1].y, curpos->z, ctx->dlc_filter)
	    && ((ctx->frw_ctx == NULL)
		|| way_free_of_droids(curpos->x, curpos->y, waypoints[1].x, waypoints[1].y, curpos->z, ctx->frw_ctx))
	    ) {
		DebugPrintf(DEBUG_TUX_PATHFINDING, "\nVERY FIRST INTERMEDIATE POINT CUT MANUALLY!!!!");
		for (cut_away = 1; cut_away < maxwp; cut_away++) {
			waypoints[cut_away - 1].x = waypoints[cut_away].x;
			waypoints[cut_away - 1].y = waypoints[cut_away].y;
		}
	} else {
		DebugPrintf(DEBUG_TUX_PATHFINDING, "\nOPTIMIZATION --> streamline_tux_intermediate_course: no final shortcut.");
	}

}				// void streamline_tux_intermediate_course ( )

#undef _pathfinder_c
