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
	else //dotprod > length²
	    {
	    return sqrt(( px - x2 ) * ( px - x2 ) + ( py - y2 ) * ( py - y2 ));
	    }
	}
}

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
    RECT_IN = 1,
    RECT_OUT_LEFT = 2,
    RECT_OUT_RIGHT = 4,
    RECT_OUT_UP = 8,
    RECT_OUT_DOWN = 16,
};

static inline char get_point_flag ( float x1, float y1, float x2, float y2, float px, float py )
{
char out = 0;
char is_in = 0; 
//check if we are left
if ( px < x1 && px < x2 )
    out |= RECT_OUT_LEFT;
else if ( px > x1 && px > x2 )
    out |= RECT_OUT_RIGHT;
else is_in++;

//check up/down
if ( py < y1 && py < y2 )
    out |= RECT_OUT_UP;
else if ( py > y1 && py > y2 )
    out |= RECT_OUT_DOWN;
else is_in++;

if (is_in == 2)
    {
    out |= RECT_IN;
    } 
return out;
}

int IsPassable ( float x, float y, int z )
{
    return DirectLineWalkable (x, y, x, y, z);
}

int IsPassableForDroid ( float x, float y, int z )
{
    global_ignore_doors_for_collisions_flag = TRUE;
    int a = DirectLineWalkable (x, y, x, y, z);
    global_ignore_doors_for_collisions_flag = FALSE;
    return a;
}

/** This function checks if the line can be walked along directly against obstacles.
 * It also handles the case of point tests (x1 == x2 && y1 == y2) properly.
 */
int DirectLineWalkable ( float x1, float y1, float x2, float y2, int z)
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
    
    float x2n = x2, y2n = y2;
    char ispoint = normalize_vect(x1,y1, &x2n, &y2n); //normalize vect will tell us if we are working on a line or a point
    
    for ( x_tile = x_tile_start; x_tile <= x_tile_end; x_tile ++ )
	{
	for ( y_tile = y_tile_start; y_tile <= y_tile_end; y_tile ++ )
	    {
	    // We can list all obstacles here  
	    int glue_index;
	    
	    for ( glue_index = 0 ; glue_index < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE ; glue_index ++ )
		{
		int obstacle_index = PassLevel -> map [ y_tile ] [ x_tile ] . obstacles_glued_to_here [ glue_index ] ;
		
		if ( obstacle_index == (-1) ) break;
		obstacle * our_obs = &(PassLevel -> obstacle_list [ obstacle_index ]);

		//--------------------
		// Now we check if maybe it's a door.  Doors should get ignored, 
		// if the global ignore_doors_for_collisions flag is set.  This
		// flag is introduced the reduce function overhead, especially
		// in the recursions used here and there.
		//
		if ( global_ignore_doors_for_collisions_flag )
		    {
		    if ( ( our_obs->type >= ISO_H_DOOR_000_OPEN ) && ( our_obs->type <= ISO_V_DOOR_100_OPEN ) )
			continue;
		    if ( ( our_obs->type >= ISO_OUTER_DOOR_V_00 ) && ( our_obs->type <= ISO_OUTER_DOOR_H_100 ) )
			continue;
		    }

		if ( ( ! ( obstacle_map [ our_obs->type ] . flags & BLOCKS_VISION_TOO) ) && ( global_check_for_light_only_collisions_flag ) )
		    continue;

		// If the obstacle doesn't even have a collision rectangle, then
		// of course it's easy, cause then there can't be any collsision
		//
		if ( obstacle_map [ our_obs->type ] . block_area_type == COLLISION_TYPE_NONE )
		    continue;

		//So we have our obstacle 
		//Check the radial distance between the center of the obstacle and the line
		float distance_to_center;
		if( ! ispoint )
		    distance_to_center = calc_distance_seg_point_normalized(x1, y1, x2, y2, x2n, y2n, our_obs->pos.x, our_obs->pos.y);
		else distance_to_center = sqrt((our_obs->pos.x - x1)*(our_obs->pos.x - x1)+(our_obs->pos.y - y1)*(our_obs->pos.y - y1));

		if ( obstacle_map [ our_obs->type ] . diaglength < distance_to_center - 0.1) //radial distance
		    continue;

		//Check the flags of both points against the rectangle of the object
		moderately_finepoint rect1 = { our_obs -> pos . x + obstacle_map [ our_obs->type ] . upper_border, our_obs -> pos . y + obstacle_map [ our_obs->type ] . left_border };
		moderately_finepoint rect2 = { our_obs -> pos . x + obstacle_map [ our_obs->type ] . lower_border, our_obs -> pos . y + obstacle_map [ our_obs->type ] . right_border };
		char p1flags = get_point_flag( rect1.x, rect1.y, rect2.x, rect2.y, x1, y1);
		char p2flags = get_point_flag( rect1.x, rect1.y, rect2.x, rect2.y, x2, y2);

		if ( !ispoint && (p1flags & p2flags) ) // if we're testing a line: both points on the same side? don't test
		    continue;

		if (( p1flags & RECT_IN ) || (p2flags & RECT_IN)) //we're in? collision without a doubt
		    return FALSE;

		if ( ispoint ) //don't test "line crosses an edge ?" for points obviously 
		    continue;

		// now determine the edges we have to test
		char to_test = 0;
		if (( p1flags & RECT_OUT_UP ) || (p2flags & RECT_OUT_UP))
		    to_test |= RECT_OUT_UP;

		if (( p1flags & RECT_OUT_DOWN ) || (p2flags & RECT_OUT_DOWN))
		    to_test |= RECT_OUT_DOWN;

		if (( p1flags & RECT_OUT_LEFT ) || (p2flags & RECT_OUT_LEFT))
		    to_test |= RECT_OUT_LEFT;

		if (( p1flags & RECT_OUT_RIGHT ) || (p2flags & RECT_OUT_RIGHT))
		    to_test |= RECT_OUT_RIGHT;

		if ( to_test != 0 )
		    return FALSE;
		}

	    } //for y_tile
	} //for x_tile

    return TRUE;

}

#undef _colldet_c
