/* 
 *
 *   Copyright (c) 2008-2010 Arthur Huillet
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

#define _colldet_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"

/* This function is Copyright (c) 1999 ID Software, from Quake3 source code, released under GPL */
static inline float Q_rsqrt(float number)
{
	float x2;
	union {
		int32_t i;
		float y;
	} a;

	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	a.y = number;
	a.i = 0x5f3759df - (a.i >> 1);
	a.y = a.y * (threehalfs - (x2 * a.y * a.y));
	a.y = a.y * (threehalfs - (x2 * a.y * a.y));

	return a.y;
}

inline int normalize_vect(float x1, float y1, float *x2, float *y2)
{
	/* Normalize X_0, X_1 */
	float tmplen2 = (x1 - *x2) * (x1 - *x2) + (y1 - *y2) * (y1 - *y2);
	if (tmplen2 == 1)
		return 0;

	if (fabsf(tmplen2) < 0.00005f)
		return 1;	//could not normalize
	*x2 = x1 + (*x2 - x1) * Q_rsqrt(tmplen2);
	*y2 = y1 + (*y2 - y1) * Q_rsqrt(tmplen2);
	return 0;
}

/** 
 * Compute the squared distance between a segment and a point.
 *
 * @param x1 segment x1
 * @param y1 segment y1
 * @param x2 segment x2
 * @param y2 segment y2
 * @param px point x
 * @param py point y
 *
 * @return squared distance
 */
static inline float calc_squared_distance_seg_point_normalized(float x1, float y1, float x2, float y2, float x2n, float y2n, float px, float py)
{

	float dotprod = (x2 - x1) * (px - x1) + (y2 - y1) * (py - y1);
	/* Check if we are on the segment with dotproduct */
	if (dotprod > 0 && dotprod < (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)) {
		/* Distance formula taken at http://www.softsurfer.com/Archive/algorithm_0102/Eqn_dcross2.gif */
		/* the distance basically is the cross product of the normalized (X_0, X_1) vector with (X_0, X) */
		float distline = fabsf(((y1 - y2n) * px + (x2n - x1) * py + (x1 * y2n - x2n * y1)));
		return distline;
	} else {
		/* We are not quite done yet ! */
		if (dotprod < 0) {
			return ((px - x1) * (px - x1) + (py - y1) * (py - y1));
		} else		//dotprod > length^2
		{
			return ((px - x2) * (px - x2) + (py - y2) * (py - y2));
		}
	}
}

/**
 * This colldet filter is used to ignore the obstacles (such as doors)
 * that can be traversed by walking bot.
 */
int WalkablePassFilterCallback(colldet_filter * this, obstacle * obs, int obs_idx)
{
	if (get_obstacle_spec(obs->type)->flags & IS_WALKABLE)
		return TRUE;

	if (this->next)
		return (this->next->callback(this->next, obs, obs_idx));

	return FALSE;
}
colldet_filter WalkablePassFilter = { WalkablePassFilterCallback, NULL, 0, NULL };
colldet_filter WalkableWithMarginPassFilter = { WalkablePassFilterCallback, NULL, COLLDET_WALKABLE_MARGIN, NULL };

/**
 * This colldet filter is used to ignore the obstacles (such as water)
 * that can be traversed by a flying object.
 */
int FlyablePassFilterCallback(colldet_filter * this, obstacle * obs, int obs_idx)
{
	if (get_obstacle_spec(obs->type)->flags & GROUND_LEVEL)
		return TRUE;

	if (this->next)
		return (this->next->callback(this->next, obs, obs_idx));

	return FALSE;
}
colldet_filter FlyablePassFilter = { FlyablePassFilterCallback, NULL, 0, NULL };

/**
 * This colldet filter is used to ignore the obstacles
 * that can be traversed by light.
 * Thus, it merely checks for visibility.
 */
int VisiblePassFilterCallback(colldet_filter * this, obstacle * obs, int obs_idx)
{
	if (!(get_obstacle_spec(obs->type)->flags & BLOCKS_VISION_TOO))
		return TRUE;

	if (this->next)
		return (this->next->callback(this->next, obs, obs_idx));

	return FALSE;
}
colldet_filter VisiblePassFilter = { VisiblePassFilterCallback, NULL, 0, NULL };

/**
 * This colldet filter is used to ignore a given obstacle during
 * collision detection
 */
int ObstacleByIdPassFilterCallback(colldet_filter * this, obstacle * obs, int obs_idx)
{
	if (obs_idx == *(int *)(this->data))
		return TRUE;

	if (this->next)
		return (this->next->callback(this->next, obs, obs_idx));

	return FALSE;
}
colldet_filter ObstacleByIdPassFilter = { ObstacleByIdPassFilterCallback, NULL, 0, NULL };
colldet_filter WalkableExceptIdPassFilter = { ObstacleByIdPassFilterCallback, NULL, 0, &WalkablePassFilter };
colldet_filter FlyableExceptIdPassFilter = { ObstacleByIdPassFilterCallback, NULL, 0, &FlyablePassFilter };

static const float Droid_Radius = 0.5;

/**
 * This function checks if there are any droids at a given location.
 */
int location_free_of_droids(float x, float y, int levelnum, freeway_context *ctx)
{
	if (ctx->check_tux) {
		float dist = (Me.pos.x - x)*(Me.pos.x - x) + (Me.pos.y - y)*(Me.pos.y - y);
		if (dist < Droid_Radius*Droid_Radius)
			return FALSE;
	}

	enemy *this_enemy;
	BROWSE_LEVEL_BOTS(this_enemy, levelnum) {
		if ((this_enemy->pure_wait > 0) ||
		    (ctx->except_bots[0] != NULL && ctx->except_bots[0] == this_enemy) ||
		    (ctx->except_bots[1] != NULL && ctx->except_bots[1] == this_enemy))
			continue;

		float dist = (this_enemy->pos.x - x)*(this_enemy->pos.x - x) + (this_enemy->pos.y - y)*(this_enemy->pos.y - y);
		if (dist < Droid_Radius*Droid_Radius)
			return FALSE;
	}

	return TRUE;
}

/**
 * This function checks if the connection between two points is free of
 * droids.
 *
 * OBSTACLES ARE NOT TAKEN INTO CONSIDERATION, ONLY DROIDS!!!
 *
 * TODO: The 2 points of the segment can be outside of the 'levelnum'.
 * We should thus also check bots on neighbor levels (using same principle
 * than in DirectLineColldet)
 */
int way_free_of_droids(float x1, float y1, float x2, float y2, int levelnum, freeway_context * ctx)
{
	float minx = min(x1, x2) - 0.5;
	float miny = min(y1, y2) - 0.5;
	float maxx = max(x1, x2) + 0.5;
	float maxy = max(y1, y2) + 0.5;

	float x2n = x2;
	float y2n = y2;
	normalize_vect(x1, y1, &x2n, &y2n);

	if (ctx->check_tux) {
		float dist = calc_squared_distance_seg_point_normalized(x1, y1, x2, y2, x2n, y2n, Me.pos.x, Me.pos.y);
		if (dist < Droid_Radius*Droid_Radius)
			return FALSE;
	}

	enemy *this_enemy;
	BROWSE_LEVEL_BOTS(this_enemy, levelnum) {
		if ((this_enemy->pure_wait > 0) ||
		    (ctx->except_bots[0] != NULL && ctx->except_bots[0] == this_enemy) ||
		    (ctx->except_bots[1] != NULL && ctx->except_bots[1] == this_enemy))
			continue;

		if (this_enemy->pos.x < minx || this_enemy->pos.y < miny || this_enemy->pos.y > maxy || this_enemy->pos.x > maxx)
			continue;

		float dist = calc_squared_distance_seg_point_normalized(x1, y1, x2, y2, x2n, y2n, this_enemy->pos.x, this_enemy->pos.y);

		if (dist < Droid_Radius*Droid_Radius)
			return FALSE;
	}

	return TRUE;
}

enum {
	RECT_IN = 0,
	RECT_OUT_LEFT = 1,
	RECT_OUT_RIGHT = 2,
	RECT_OUT_UP = 4,
	RECT_OUT_DOWN = 8,
};

static inline char get_point_flag(float xmin, float ymin, float xmax, float ymax, float px, float py)
{
	char out = RECT_IN;

	//check if we are left
	if (px < xmin)
		out |= RECT_OUT_LEFT;
	else if (px > xmax)
		out |= RECT_OUT_RIGHT;

	//check up/down
	if (py < ymin)
		out |= RECT_OUT_DOWN;
	else if (py > ymax)
		out |= RECT_OUT_UP;

	return out;
}

/**
 * This function checks if a given position is free of obstacles
 * 
 * Return FALSE if collision detected.
 */
int SinglePointColldet(float x, float y, int z, colldet_filter * filter)
{
	return DirectLineColldet(x, y, x, y, z, filter);
}				// int SinglePointColldet ( float x, float y, int z, colldet_filter* filter )

/** This function checks if the line can be traversed along directly against obstacles.
 * It also handles the case of point tests (x1 == x2 && y1 == y2) properly.
 * 
 * This function scans all the obstacles that are inside the bounding box of the
 * segment, and check if the segment intersects these obstacles.
 * 
 * Note: this function compute the intersection between a segment and the obstacles
 * of one given level. This is a helper function used by DirectLineColldet(), which
 * is the real function to use. see below.
 * 
 * Return FALSE if collision detected.
  */
static int dlc_on_one_level(int x_tile_start, int x_tile_end, int y_tile_start, int y_tile_end,
			    gps * p1, gps * p2, level * lvl, colldet_filter * filter)
{
	int x_tile, y_tile;
	int tstamp = next_glue_timestamp();

	char ispoint = ((p1->x == p2->x) && (p1->y == p2->y));

	for (y_tile = y_tile_start; y_tile <= y_tile_end; y_tile++) {
		for (x_tile = x_tile_start; x_tile <= x_tile_end; x_tile++) {
			// We can list all obstacles here  
			int glue_index;

			for (glue_index = 0; glue_index < lvl->map[y_tile][x_tile].glued_obstacles.size; glue_index++) {
				int *glued_obstacles = lvl->map[y_tile][x_tile].glued_obstacles.arr;
				int obstacle_index = glued_obstacles[glue_index];

				if (obstacle_index == (-1))
					break;

				obstacle *our_obs = &(lvl->obstacle_list[obstacle_index]);

				if (our_obs->timestamp == tstamp) {
					continue;
				}

				our_obs->timestamp = tstamp;

				// If the obstacle doesn't even have a collision rectangle, then
				// of course it's easy, cause then there can't be any collision
				//
				obstacle_spec *obstacle_spec = get_obstacle_spec(our_obs->type);
				if (obstacle_spec->block_area_type == COLLISION_TYPE_NONE)
					continue;

				// Filter out some obstacles, if asked
				if (filter && filter->callback(filter, our_obs, obstacle_index))
					continue;

				// So we have our obstacle 

				// Check the flags of both points against the rectangle of the object
				pointf rect1 = { our_obs->pos.x + obstacle_spec->left_border,
					our_obs->pos.y + obstacle_spec->upper_border
				};
				pointf rect2 = { our_obs->pos.x + obstacle_spec->right_border,
					our_obs->pos.y + obstacle_spec->lower_border
				};

				// When DLC is called by the pathfinder, we grow a bit the obstacle's size.
				// Motivation: the path followed by a character (Tux or a bot) can diverge a bit
				// from the path computed by the pathfinder. If that computed path is passing very close
				// to an obstacle, then the actual path could go inside the obstacle, leading the character
				// to be stuck.
				//
				// Also, we will want to use this when dropping items.
				if (filter && filter->extra_margin != 0.0) {
					rect1.x -= filter->extra_margin;
					rect1.y -= filter->extra_margin;
					rect2.x += filter->extra_margin;
					rect2.y += filter->extra_margin;
				}

				char p1flags = get_point_flag(rect1.x, rect1.y, rect2.x, rect2.y, p1->x, p1->y);
				char p2flags = get_point_flag(rect1.x, rect1.y, rect2.x, rect2.y, p2->x, p2->y);

				// Handle obvious cases
				if (p1flags & p2flags)	// both points on the same side, no collision
					continue;

				if ((p1flags == RECT_IN) || (p2flags == RECT_IN))	//we're in? collision without a doubt
					return FALSE;

				if (ispoint)	// degenerated line, and outside the rectangle, no collision
					continue;

				// possible collision, check if the line is crossing the rectangle
				// by looking if all vertices of the rectangle are in the same half-space
				//
				// 1- line equation : a*x + b*y + c = 0
				float line_a = -(p1->y - p2->y);
				float line_b = p1->x - p2->x;
				float line_c = p2->x * p1->y - p2->y * p1->x;

				// 2- check each vertex, halt as soon as one is on the other halfspace -> collision
				char first_vertex_sign = (line_a * rect1.x + line_b * rect1.y + line_c) > 0 ? 0 : 1;
				char other_vertex_sign = (line_a * rect2.x + line_b * rect1.y + line_c) > 0 ? 0 : 1;
				if (first_vertex_sign != other_vertex_sign)
					return FALSE;
				other_vertex_sign = (line_a * rect2.x + line_b * rect2.y + line_c) > 0 ? 0 : 1;
				if (first_vertex_sign != other_vertex_sign)
					return FALSE;
				other_vertex_sign = (line_a * rect1.x + line_b * rect2.y + line_c) > 0 ? 0 : 1;
				if (first_vertex_sign != other_vertex_sign)
					return FALSE;

			}

		}		//for x_tile
	}			//for y_tile

	return TRUE;

}

/** This function checks if the line can be traversed along directly against obstacles.
 * It also handles the case of point tests (x1 == x2 && y1 == y2) properly.
 * 
 * Return FALSE if collision detected.
 * 
 * (see dlc_on_one_level() for the core collision detection)
 * 
 * The bounding box around the segment can span up to 4 levels. We thus have to use
 * virtual gps coordinates.
 * 
 * The overall algorithm should be :
 * foreach tile of the bounding box along Y axis {
 *   foreach tile of the bounding box along X axis {
 *     resolve virtual coord (X,Y,Z) into real coord (X1,Y1,Z1)
 *     foreach obstacle at (X1,Y1,Z1) {
 *       get obstacle's virtual position according to Z level
 *       check collision between segment and obstacle
 *     }
 *   }
 * }
 * There will thus be a lot of gps transformations, with a real impact performance.
 * 
 * To avoid all those transformations, we will rather split the segment's bbox
 * into (up to) 4 parts, one part per spanned level. We then just have to transform
 * the segment's endpoints according to each level. For example, if part of the bbox
 * is on the north neighbor, we will have :
 * 
 * get segment's virtual position according to north neighbor
 * foreach tile of the bounding box on the north neighbor along Y axis {
 *   foreach tile of the bounding box on the north neighbor along X axis {
 *     foreach obstacle at (X,Y,north level) {
 *       check collision between 'virtual segment' and obstacle
 *     }
 *   }
 * }
 * 
 * We don't however want to check all the neighborhood (current level + its 8 neighbors), 
 * since at most 4 levels can be spanned by the segment's bbox. 
 * We will here use the same trick than the one in resolved_virtual_position():
 *   conceptually, if (X,Y) is a virtual position, the (i,j) pair defined by i = (int)(X/xlen)
 *   and j = (int)(Y/ylen), is a reference to one of the 9 levels in the neighborhood, where 
 *   the (X,Y) point lies.
 * 
 * We can then transform the bbox's top-left and bottom-right points into 2 pairs
 * that will reference the top-left and bottom-right levels spanned by the segment, and
 * we just have to loop on those levels.
 * 
 * Small optimization : chances are that the biggest part of the bbox is on the current
 * level. We will then first check collision on the current level, before to test neighbors.
 */

int DirectLineColldet(float x1, float y1, float x2, float y2, int z, colldet_filter * filter)
{
	gps p1 = { x1, y1, z };	// segment real starting gps point
	gps p2 = { x2, y2, z };	// segment real ending gps point
	gps vp1, vp2;		// segment's virtual endpoints

	// Segment's bbox.
	int x_tile_start = floor(min(x1, x2));
	int y_tile_start = floor(min(y1, y2));
	int x_tile_end = ceil(max(x1, x2)) - 1;
	int y_tile_end = ceil(max(y1, y2)) - 1;

	// Special case: if x1==x2 and x1 is an integral value, the previous
	// definition ends up with x_tile_end = x_tile_start - 1, which is definitely
	// not intended (same remark with Y values)
	if (x_tile_end < x_tile_start)
		x_tile_end = x_tile_start;
	if (y_tile_end < y_tile_start)
		y_tile_end = y_tile_start;

	int x_start, y_start, x_end, y_end;	// intersection between the bbox and one level's limits
	struct neighbor_data_cell *ngb_cell = NULL;

	// First check on current level (small optimization)
	//

	level *lvl = curShip.AllLevels[z];

	// compute the intersection between the bbox and the current level
	x_start = max(0, x_tile_start);
	x_end = min(lvl->xlen - 1, x_tile_end);
	y_start = max(0, y_tile_start);
	y_end = min(lvl->ylen - 1, y_tile_end);

	// check collision
	if (!dlc_on_one_level(x_start, x_end, y_start, y_end, &p1, &p2, lvl, filter))
		return FALSE;

	// Get the neighborhood covered by the segment's bbox (see function's comment)
	// and check each of the neighbors
	//
	int x_ngb_start = NEIGHBOR_IDX(x_tile_start, lvl->xlen);
	int x_ngb_end = NEIGHBOR_IDX(x_tile_end, lvl->xlen);
	int y_ngb_start = NEIGHBOR_IDX(y_tile_start, lvl->ylen);
	int y_ngb_end = NEIGHBOR_IDX(y_tile_end, lvl->ylen);

	// Loop on the neighbors

	int j, i;
	for (j = y_ngb_start; j <= y_ngb_end; j++) {
		for (i = x_ngb_start; i <= x_ngb_end; i++) {

			if (j == 1 && i == 1)
				continue;	// this references the current level, already checked

			// if the neighbor exists...

			if ((ngb_cell = level_neighbors_map[z][j][i])) {

				// get neighbor's level struct
				lvl = curShip.AllLevels[ngb_cell->lvl_idx];

				// transform bbox corners into virtual positions according to lvl
				// and compute intersection with lvl's limits
				x_start = max(0, x_tile_start + ngb_cell->delta_x);
				x_end = min(lvl->xlen - 1, x_tile_end + ngb_cell->delta_x);
				y_start = max(0, y_tile_start + ngb_cell->delta_y);
				y_end = min(lvl->ylen - 1, y_tile_end + ngb_cell->delta_y);

				// transform segment's endpoints into virtual positions according to lvl
				update_virtual_position(&vp1, &p1, ngb_cell->lvl_idx);
				update_virtual_position(&vp2, &p2, ngb_cell->lvl_idx);

				// check collision
				if (!dlc_on_one_level(x_start, x_end, y_start, y_end, &vp1, &vp2, lvl, filter))
					return FALSE;
			}

		}
	}

	return TRUE;
}

/**************************************************************
 * Bots and Tux escaping code
 */

/**
 * Small structure used to sort a list of directions
 * (Used during escaping)
 */
struct dist_elt {
	float value;
	unsigned char direction;
};

/**
 * Comparison function of two dist_elt.
 * (Used by qsort during escaping)
 */
static int cmp_dist_elt(const void *elt1, const void *elt2)
{
	return (((struct dist_elt *)elt1)->value - ((struct dist_elt *)elt2)->value);
}

/**
 * Move a character outside of a collision rectangle.
 * (Used during escaping)
 * We have no idea of where the character came from and where it was going to
 * when it gets stuck.
 * Thus, we assume that the character is actually quite near a free position.
 * We will then test each rectangle's edges, starting from the closest one.
 */
int MoveOutOfObstacle(float *posX, float *posY, int posZ, obstacle * ThisObstacle, colldet_filter * filter)
{
	enum { RIGHT, DOWN, LEFT, TOP };
	unsigned int i;
	pointf new_pos = { 0.5, 0.5 };
	obstacle_spec *obstacle_spec = get_obstacle_spec(ThisObstacle->type);

	// Construct a sorted list of distance between character's position and the rectangle's edges
	//
	pointf rect1 = { ThisObstacle->pos.x + obstacle_spec->left_border,
		ThisObstacle->pos.y + obstacle_spec->upper_border
	};
	pointf rect2 = { ThisObstacle->pos.x + obstacle_spec->right_border,
		ThisObstacle->pos.y + obstacle_spec->lower_border
	};

	struct dist_elt dist_arr[] = { {rect2.x - (*posX), RIGHT},
	{(*posX) - rect1.x, LEFT},
	{rect2.y - (*posY), TOP},
	{(*posY) - rect1.y, DOWN}
	};

	qsort(dist_arr, 4, sizeof(struct dist_elt), cmp_dist_elt);

	// Check each direction of the sorted list
	//
	for (i = 0; i < 4; ++i) {
		switch (dist_arr[i].direction) {
		case (RIGHT):
			{
				new_pos.x = rect2.x + 0.1;	// small increment to really be outside the rectangle
				new_pos.y = *posY;
				break;
			}
		case (DOWN):
			{
				new_pos.x = *posX;
				new_pos.y = rect1.y - 0.1;
				break;
			}
		case (LEFT):
			{
				new_pos.x = rect1.x - 0.1;
				new_pos.y = *posY;
				break;
			}
		case (TOP):
			{
				new_pos.x = *posX;
				new_pos.y = rect2.y + 0.1;
				break;
			}
		}
		if (SinglePointColldet(new_pos.x, new_pos.y, posZ, filter)) {	// got a free position -> move the character and returns
			*posX = new_pos.x;
			*posY = new_pos.y;
			return TRUE;
		}
	}

	// No free position was found outside the obstacle ???
	// The caller will need to find a fallback
	//
	return FALSE;
}

/**
 * This function escapes a character from an obstacle, by
 * computing a new position
 */
int EscapeFromObstacle(float *posX, float *posY, int posZ, colldet_filter * filter)
{
	int start_x, end_x, start_y, end_y;
	int x, y, i, obst_index;
	Level ThisLevel;

	start_x = (int)(*posX) - 2;
	start_y = (int)(*posY) - 2;
	end_x = start_x + 4;
	end_y = start_y + 4;

	ThisLevel = curShip.AllLevels[posZ];

	if (start_x < 0)
		start_x = 0;
	if (start_y < 0)
		start_y = 0;
	if (end_x >= ThisLevel->xlen)
		end_x = ThisLevel->xlen - 1;
	if (end_y >= ThisLevel->ylen)
		end_y = ThisLevel->ylen - 1;

	for (y = start_y; y < end_y; ++y) {
		for (x = start_x; x < end_x; ++x) {
			for (i = 0; i < ThisLevel->map[y][x].glued_obstacles.size; i++) {
				int *glued_obstacles = ThisLevel->map[y][x].glued_obstacles.arr;

				obst_index = glued_obstacles[i];

				obstacle *our_obs = &(ThisLevel->obstacle_list[obst_index]);

				if (filter && filter->callback(filter, our_obs, obst_index))
					continue;

				// If the obstacle doesn't even have a collision rectangle, then
				// of course it's easy, cause then there can't be any collision
				//
				obstacle_spec *obstacle_spec = get_obstacle_spec(our_obs->type);
				if (obstacle_spec->block_area_type == COLLISION_TYPE_NONE)
					continue;

				// Now if the position lies inside the collision rectangle, then there's
				// a collision.
				//
				if ((*posX > our_obs->pos.x + obstacle_spec->left_border) &&
				    (*posX < our_obs->pos.x + obstacle_spec->right_border) &&
				    (*posY > our_obs->pos.y + obstacle_spec->upper_border) &&
				    (*posY < our_obs->pos.y + obstacle_spec->lower_border)) {
					// Find a new position for the character
					return MoveOutOfObstacle(posX, posY, posZ, our_obs, filter);
				}
			}
		}
	}

	return FALSE;
}				// int EscapeFromObstacle( float* posX, float* posY, int posZ, colldet_filter* filter )

#undef _colldet_c
