/* 
 *
 *   Copyright (c) 2008 Arthur Huillet
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

#define _colldet_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"

/* This function is Copyright (c) 1999 ID Software, from Quake3 source code, released under GPL */
static inline float Q_rsqrt( float number )
{
  float x2;
  union 
      {
      int32_t i;
      float y;
      } a;

  const float threehalfs = 1.5F;

  x2 = number * 0.5F;
  a.y  = number;
  a.i  = 0x5f3759df - ( a.i >> 1 );
  a.y  = a.y * ( threehalfs - ( x2 * a.y * a.y ) );
  a.y  = a.y * ( threehalfs - ( x2 * a.y * a.y ) );

  return a.y;
}

inline int normalize_vect( float x1, float y1, float * x2, float *y2 )
{
    /* Normalize X_0, X_1 */ 
    float tmplen2 = (x1-*x2)*(x1-*x2)+(y1-*y2)*(y1-*y2);
    if ( tmplen2 == 1 )
       return 0;

    if ( fabsf(tmplen2) < 0.00005 ) 
	return 1; //could not normalize
    *x2 = x1 + (*x2 - x1) * Q_rsqrt(tmplen2); 
    *y2 = y1 + (*y2 - y1) * Q_rsqrt(tmplen2);
    return 0;
}

/** 
 * Compute the distance between a segment and a point.
 *
 * @param x1 segment x1
 * @param y1 segment y1
 * @param x2 segment x2
 * @param y2 segment y2
 * @param px point x
 * @param py point y
 *
 * @return distance
 */
static inline float calc_distance_seg_point_normalized ( float x1, float y1, float x2, float y2, float x2n, float y2n, float px, float py )
{
   
    float dotprod = (x2 - x1) * (px - x1) + (y2 - y1) * (py - y1);
    /* Check if we are on the segment with dotproduct */
    if ( dotprod > 0 && dotprod < (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1) )
	{
	/* Distance formula taken at http://www.softsurfer.com/Archive/algorithm_0102/Eqn_dcross2.gif */
	/* the distance basically is the cross product of the normalized (X_0, X_1) vector with (X_0, X) */
	float distline = fabsf(( ( y1 - y2n ) * px + ( x2n - x1 ) * py + (x1 * y2n - x2n * y1 ) ));
	return distline;
	}
    else 
	{
	/* We are not quite done yet ! */
	if ( dotprod < 0 )
	    {
	    return sqrt(( px - x1 ) * ( px - x1 ) + ( py - y1 ) * ( py - y1 ));
	    }
	else //dotprod > length^2
	    {
	    return sqrt(( px - x2 ) * ( px - x2 ) + ( py - y2 ) * ( py - y2 ));
	    }
	}
}

/**
 * This colldet filter is used to ignore the obstacles (such as doors)
 * that can be traversed by walking bot.
 */
int FilterWalkableCallback(colldet_filter* this, obstacle* obs, int obs_idx )
{
	if ( obstacle_map[obs->type].flags & IS_WALKABLE ) return TRUE;

	if (this->next) return(this->next->callback(this->next, obs, obs_idx));
	
	return FALSE;
}
colldet_filter FilterWalkable = { FilterWalkableCallback, NULL, NULL };

/**
 * This colldet filter is used to ignore the obstacles (such as water)
 * that can be traversed by a flying object.
 */
int FilterFlyableCallback(colldet_filter* this, obstacle* obs, int obs_idx )
{
	if ( obstacle_map[obs->type].flags & GROUND_LEVEL ) return TRUE;

	if (this->next) return(this->next->callback(this->next, obs, obs_idx));
	
	return FALSE;
}
colldet_filter FilterFlyable = { FilterFlyableCallback, NULL, NULL };

/**
 * This colldet filter is used to ignore the obstacles
 * that can be traversed by light.
 * Thus, it merely checks for visibility.
 */
int FilterVisibleCallback(colldet_filter* this, obstacle* obs, int obs_idx )
{
	if ( ! ( obstacle_map[obs->type].flags & BLOCKS_VISION_TOO) ) return TRUE;

	if (this->next) return(this->next->callback(this->next, obs, obs_idx));
	
	return FALSE;
}
colldet_filter FilterVisible = { FilterVisibleCallback, NULL, NULL };

/**
 * This colldet filter is used to ignore a given obstacle during
 * collision detection
 */
int FilterObstacleByIdCallback(colldet_filter* this, obstacle* obs, int obs_idx )
{
	if ( obs_idx == *(int*)(this->data) ) return TRUE;

	if (this->next) return(this->next->callback(this->next, obs, obs_idx));

	return FALSE;
}
colldet_filter FilterObstacleById = { FilterObstacleByIdCallback, NULL, NULL };

/**
 * This function checks if the connection between two points is free of
 * droids.  
 *
 * friendly_check can take three values : check all droids, only friendly droids
 * (with respect to tux), or only enemy droids
 *
 * OBSTACLES ARE NOT TAKEN INTO CONSIDERATION, ONLY DROIDS!!!
 *
 */
int 
CheckIfWayIsFreeOfDroids (char test_tux, float x1 , float y1 , float x2 , float y2 , int OurLevel , 
					  Enemy ExceptedRobot ) 
{
    const float Druid_Radius = 0.5;
     
    float x2n = x2;
    float y2n = y2;
    normalize_vect ( x1, y1, &x2n, &y2n);

    enemy * this_enemy;
    BROWSE_LEVEL_BOTS(this_enemy, OurLevel)
	{
	if (( this_enemy -> pure_wait > 0 )  || (this_enemy == ExceptedRobot ) )
	    continue;

	float dist = calc_distance_seg_point_normalized ( x1, y1, x2, y2, x2n, y2n, this_enemy->pos.x, this_enemy->pos.y);

	if ( dist < Druid_Radius )
	    return FALSE;
	}

    return TRUE;

}; // CheckIfWayIsFreeOfDroids ( char test_tux, float x1 , float y1 , float x2 , float y2 , int OurLevel , int ExceptedDroid )

enum {
	RECT_IN = 0,
	RECT_OUT_LEFT = 1,
	RECT_OUT_RIGHT = 2,
	RECT_OUT_UP = 4,
	RECT_OUT_DOWN = 8,
};

static inline char get_point_flag ( float xmin, float ymin, float xmax, float ymax, float px, float py )
{
	char out = RECT_IN;

	//check if we are left
	if ( px < xmin )
		out |= RECT_OUT_LEFT;
	else if ( px > xmax )
		out |= RECT_OUT_RIGHT;

	//check up/down
	if ( py < ymin )
		out |= RECT_OUT_DOWN;
	else if ( py > ymax )
		out |= RECT_OUT_UP;

	return out;
}

/**
 * This function checks if a given position is free of obstacles
 */
int SinglePointColldet ( float x, float y, int z, colldet_filter* filter )
{
    return DirectLineColldet (x, y, x, y, z, filter);
} // int SinglePointColldet ( float x, float y, int z, colldet_filter* filter )

/** This function checks if the line can be traversed along directly against obstacles.
 * It also handles the case of point tests (x1 == x2 && y1 == y2) properly.
 */
int DirectLineColldet ( float x1, float y1, float x2, float y2, int z, colldet_filter* filter)
{

    //Browse all obstacles around the rectangle
    int x_tile_start, y_tile_start;
    int x_tile_end, y_tile_end;
    int x_tile, y_tile;
    Level PassLevel = curShip . AllLevels [ z ] ;

    x_tile_start = floor(min(x1,x2)) - 2;
    y_tile_start = floor(min(y1,y2)) - 2;
    x_tile_end = ceil(max(x1,x2)) + 2;
    y_tile_end = ceil(max(y1,y2)) + 2;
    
    if ( x_tile_start < 0 ) x_tile_start = 0 ;
    if ( y_tile_start < 0 ) y_tile_start = 0 ;
    if ( x_tile_end >= PassLevel -> xlen ) x_tile_end = PassLevel->xlen -1 ;
    if ( y_tile_end >= PassLevel -> ylen ) y_tile_end = PassLevel->ylen -1 ;

	char ispoint = ( (x1 == x2) && (y1 == y2) );

	for ( y_tile = y_tile_start; y_tile <= y_tile_end; y_tile ++ )
	{
    for ( x_tile = x_tile_start; x_tile <= x_tile_end; x_tile ++ )
	{
	    // We can list all obstacles here  
	    int glue_index;
	    
	    for ( glue_index = 0 ; glue_index < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE ; glue_index ++ )
		{
		int obstacle_index = PassLevel -> map [ y_tile ] [ x_tile ] . obstacles_glued_to_here [ glue_index ] ;
		
		if ( obstacle_index == (-1) ) break;
				
		obstacle * our_obs = &(PassLevel -> obstacle_list [ obstacle_index ]);

		// If the obstacle doesn't even have a collision rectangle, then
		// of course it's easy, cause then there can't be any collsision
		//
		if ( obstacle_map [ our_obs->type ] . block_area_type == COLLISION_TYPE_NONE )
		    continue;

		// Filter out some obstacles, if asked
		if ( filter && filter->callback(filter, our_obs, obstacle_index ) ) continue;

		//So we have our obstacle 

		//Check the flags of both points against the rectangle of the object
		moderately_finepoint rect1 = { our_obs -> pos . x + obstacle_map [ our_obs->type ] . upper_border, 
		                               our_obs -> pos . y + obstacle_map [ our_obs->type ] . left_border };
		moderately_finepoint rect2 = { our_obs -> pos . x + obstacle_map [ our_obs->type ] . lower_border, 
		                               our_obs -> pos . y + obstacle_map [ our_obs->type ] . right_border };

		char p1flags = get_point_flag( rect1.x, rect1.y, rect2.x, rect2.y, x1, y1 );
		char p2flags = get_point_flag( rect1.x, rect1.y, rect2.x, rect2.y, x2, y2 );

		// Handle obvious cases
		if ( p1flags & p2flags ) // both points on the same side, no collision
		    continue;

		if ( (p1flags == RECT_IN) || (p2flags == RECT_IN) ) //we're in? collision without a doubt
		    return FALSE;

		if ( ispoint ) // degenerated line, and outside the rectangle, no collision
		    continue;
		
		//----------
		// possible collision, check if the line is crossing the rectangle
		// by looking if all vertices of the rectangle are in the same half-space
		//
		// 1- line equation : a*x + b*y + c = 0
		float line_a = -(y1 - y2);
		float line_b = x1 - x2;
		float line_c = x2*y1 - y2*x1;

		// 2- check each vertex, halt as soon as one is on the other halfspace -> collision
		char first_vertex_sign = ( line_a * rect1.x + line_b * rect1.y + line_c ) > 0 ? 0 : 1;
		char other_vertex_sign = ( line_a * rect2.x + line_b * rect1.y + line_c ) > 0 ? 0 : 1;
		if ( first_vertex_sign != other_vertex_sign ) return FALSE;
		other_vertex_sign = ( line_a * rect2.x + line_b * rect2.y + line_c ) > 0 ? 0 : 1;
		if ( first_vertex_sign != other_vertex_sign ) return FALSE;
		other_vertex_sign = ( line_a * rect1.x + line_b * rect2.y + line_c ) > 0 ? 0 : 1;
		if ( first_vertex_sign != other_vertex_sign ) return FALSE;

		}

	    } //for x_tile
	    } //for y_tile

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
static int
cmp_dist_elt( const void* elt1, const void* elt2 )
{
	return ( ((struct dist_elt*)elt1)->value - ((struct dist_elt*)elt1)->value );
}
/**
 * Move a character outside of a collision rectangle.
 * (Used during escaping)
 * We have no idea of where the character came from and where it was going to
 * when it gets stuck.
 * Thus, we assume that the character is actually quite near a free position.
 * We will then test each rectangle's edges, starting from the closest one.
 */
int
MoveOutOfObstacle( float* posX, float* posY, int posZ, obstacle* ThisObstacle, colldet_filter* filter )
{
	enum { RIGHT, DOWN, LEFT, TOP };
	unsigned int i;
	moderately_finepoint new_pos = { 0.5, 0.5 };

	//--------------------
	// Construct a sorted list of distance between character's position and the rectangle's edges
	//
	moderately_finepoint rect1 = { ThisObstacle->pos.x + obstacle_map[ThisObstacle->type]. upper_border,
	                               ThisObstacle->pos.y + obstacle_map[ThisObstacle->type]. left_border };
	moderately_finepoint rect2 = { ThisObstacle->pos.x + obstacle_map[ThisObstacle->type]. lower_border,
	                               ThisObstacle->pos.y + obstacle_map[ThisObstacle->type]. right_border };

	struct dist_elt dist_arr[] = { { rect2.x - (*posX), RIGHT },
	                               { (*posX) - rect1.x, LEFT  },
	                               { rect2.y - (*posY), TOP   },
	                               { (*posY) - rect1.y, DOWN  }
	};

	qsort(dist_arr, 4, sizeof(struct dist_elt), cmp_dist_elt);

	//--------------------
	// Check each direction of the sorted list
	//
	for ( i=0; i<5; ++i )
	{
		switch (dist_arr[i].direction)
		{
		case(RIGHT):
		{
			new_pos.x = rect2.x + 0.01; // small increment to really be outside the rectangle
			new_pos.y = *posY;
			break;
		}
		case (DOWN):
		{
			new_pos.x = *posX;
			new_pos.y = rect1.y - 0.01;
			break;
		}
		case (LEFT):
		{
			new_pos.x = rect1.x - 0.01;
			new_pos.y = *posY;
			break;
		}
		case (TOP):
		{
			new_pos.x = *posX;
			new_pos.y = rect2.y + 0.01;
			break;
		}
		}
		if ( SinglePointColldet(new_pos.x, new_pos.y, posZ, filter) )
		{ // got a free position -> move the character and returns
			*posX = new_pos.x;
			*posY = new_pos.y;
			return TRUE;
		}
	}

	// No free position was found outside the obstacle ???
	// The caller will need to find a fallback
	//
	return FALSE;

} // void MoveOutOfObstacle( float* posX, float* posY, int posZ, obstacle* ThisObstacle )

/**
 * This function escapes a character from an obstacle, by
 * computing a new position
 */
int EscapeFromObstacle( float* posX, float* posY, int posZ, colldet_filter* filter )
{
	int start_x, end_x, start_y, end_y;
	int x , y , i, obst_index ;
	Level ThisLevel;

	start_x = (int)(*posX) - 2;
	start_y = (int)(*posY) - 2;
	end_x = start_x + 4;
	end_y = start_y + 4;

	ThisLevel = curShip.AllLevels[posZ] ;

	if ( start_x < 0 ) start_x = 0 ;
	if ( start_y < 0 ) start_y = 0 ;
	if ( end_x >= ThisLevel->xlen ) end_x = ThisLevel->xlen - 1 ;
	if ( end_y >= ThisLevel->ylen ) end_y = ThisLevel->ylen - 1 ;

	for ( y = start_y; y < end_y; ++y )
	{
		for ( x = start_x; x < end_x; ++x )
		{
			for ( i = 0 ; i < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE ; ++i )
			{
				if ( ThisLevel->map[y][x].obstacles_glued_to_here[i] == (-1) ) break;

				obst_index = ThisLevel->map[y][x].obstacles_glued_to_here[i];
				
				obstacle* our_obs = &(ThisLevel->obstacle_list[obst_index]);

				if ( filter && filter->callback(filter, our_obs, obst_index) ) continue;

				// If the obstacle doesn't even have a collision rectangle, then
				// of course it's easy, cause then there can't be any collsision
				//
				if ( obstacle_map[our_obs->type].block_area_type == COLLISION_TYPE_NONE ) continue;

				//--------------------
				// Now if the position lies inside the collision rectangle, then there's
				// a collision.
				//
				if ( ( *posX > our_obs->pos.x + obstacle_map[our_obs->type].upper_border ) && 
					 ( *posX < our_obs->pos.x + obstacle_map[our_obs->type].lower_border ) && 
					 ( *posY > our_obs->pos.y + obstacle_map[our_obs->type].left_border ) && 
					 ( *posY < our_obs->pos.y + obstacle_map[our_obs->type].right_border ) )
				{
					// Find a new position for the character
					return MoveOutOfObstacle( posX, posY, posZ, our_obs, filter );
				}
			}
		}
	}

	return FALSE;
} // int EscapeFromObstacle( float* posX, float* posY, int posZ, colldet_filter* filter )

#undef _colldet_c
