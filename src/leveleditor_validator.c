/* 
 *
 *   Copyright (c) 2002, 2003 Johannes Prix
 *   Copyright (c) 2004-2008 Arthur Huillet
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
 * This file contains all functions to validate level maps.
 * Used by the level editor.
 */

#define _leveleditor_validator_c

#include "leveleditor_validator.h"
#include "lang.h"

#define IS_CHEST(t)  ( (t) >= ISO_H_CHEST_CLOSED && (t) <= ISO_V_CHEST_OPEN )
#define IS_BARREL(t) ( (t) >= ISO_BARREL_1       && (t) <= ISO_BARREL_4     )

static char* bigline = "====================================================================";
static char*    line = "--------------------------------------------------------------------";
static char* sepline = "+------------------------------";

level_validator level_validators[] = { 
		chest_reachable_validator,
		waypoint_validator,
		interface_validator,
		NULL
	};

//===========================================================
// Helper Functions
//===========================================================

void ValidatorPrintHeader( level_validator_ctx* val_ctx, char* title, char* comment )
{
	int cpt = 0;
	char* ptr = comment;

	putchar( '\n' );
	puts( bigline );
	printf( "| %s - Level %d\n", title, val_ctx->this_level->levelnum );
	puts( sepline );
	
	printf( "| " );\
	// Split the text at the first whitespace after the 60th character
	while ( *ptr ) {
		if ( *ptr == '\n' ) { printf( "\n| " ); cpt = 0; ++ptr; continue; }
		if ( cpt < 60 )     { putchar(*ptr); ++cpt; ++ptr; continue; }
		if ( *ptr == ' ')   { printf( "\n| " ); cpt = 0; ++ptr; continue; }
		else                { putchar(*ptr); ++ptr; } // continue until a whitespace is found
	}
	putchar( '\n' );
	puts( line );
}

/**
 * This validator checks if the activable objects (chests, barrels, crates) are reachable
 */

int chest_reachable_validator( level_validator_ctx* ValidatorCtx )
{
	int x_tile, y_tile, glue_index;
	int is_invalid = FALSE;

	for ( y_tile = 0 ; y_tile < ValidatorCtx->this_level->ylen ; ++y_tile )
	{
		for ( x_tile = 0 ; x_tile < ValidatorCtx->this_level->xlen ; ++x_tile )
		{
			for ( glue_index = 0 ; glue_index < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE ; ++glue_index )
			{
				int obs_index = ValidatorCtx->this_level->map[y_tile][x_tile].obstacles_glued_to_here[glue_index];
				if ( obs_index == (-1) ) break;
    			
				obstacle* this_obs = &(ValidatorCtx->this_level->obstacle_list[obs_index]);
    			
				if ( ! (IS_CHEST(this_obs->type) || IS_BARREL(this_obs->type))  ) continue;
    			
				colldet_filter filter = { ObstacleByIdPassFilterCallback, &obs_index, &WalkablePassFilter };
				if ( !SinglePointColldet(this_obs->pos.x, this_obs->pos.y, ValidatorCtx->this_level->levelnum, &filter) )
				{
					if ( !is_invalid )
					{	// First error : print header
						ValidatorPrintHeader(ValidatorCtx, "Unreachable chests/barrels list",
						                                   "The center of the following objects was found to be inside an obstacle, preventing Tux from activating them." );
						is_invalid = TRUE;
					}
					printf( "Idx: %d (%s) - Pos: %f/%f\n", obs_index, obstacle_map[this_obs->type].obstacle_short_name, this_obs->pos.x, this_obs->pos.y );
				}
			}
		}
	}
	if ( is_invalid) puts( line );
	
	return is_invalid;
}

/**
 * This validator checks if waypoints are valid
 */

int waypoint_validator( level_validator_ctx* ValidatorCtx )
{
	int i, j;
	int pos_is_invalid = FALSE;
	int conn_is_invalid = FALSE;
	int path_is_invalid = FALSE;

	// Check waypoints position
	for ( i = 0; i < ValidatorCtx->this_level->num_waypoints; ++i )
	{
		if ( !SinglePointColldet(ValidatorCtx->this_level->AllWaypoints[i].x + 0.5, ValidatorCtx->this_level->AllWaypoints[i].y + 0.5, ValidatorCtx->this_level->levelnum, &WalkablePassFilter) )
		{
			if ( !pos_is_invalid )
			{	// First error : print header
				ValidatorPrintHeader(ValidatorCtx, "Unreachable waypoints list",
				                                   "The following waypoints were found to be inside an obstacle.\n"
				                                   "This could lead to some bots being stuck." );
				pos_is_invalid = TRUE;
			}
			printf( "Pos: %f/%f\n", ValidatorCtx->this_level->AllWaypoints[i].x + 0.5, ValidatorCtx->this_level->AllWaypoints[i].y + 0.5);			
		}
	}
	if ( pos_is_invalid ) puts( line );
	
	// Check waypoints connectivity
	for ( i = 0; i < ValidatorCtx->this_level->num_waypoints; ++i )
	{
		if ( ValidatorCtx->this_level->AllWaypoints[i].num_connections == 0 )
		{
			if ( !conn_is_invalid )
			{	// First error : print header
				ValidatorPrintHeader(ValidatorCtx, "Unconnected waypoints list",
				                                   "The following waypoints were found to be without connection.\n"
						                           "This could lead to some bots being stuck on those waypoints." );
				conn_is_invalid = TRUE;
			}
			printf( "Pos: %f/%f\n", ValidatorCtx->this_level->AllWaypoints[i].x + 0.5, ValidatorCtx->this_level->AllWaypoints[i].y + 0.5 );			
		}
	}
	if ( conn_is_invalid ) puts( line );
	
	// Check waypoint paths walkability
	for ( i = 0; i < ValidatorCtx->this_level->num_waypoints; ++i )
	{
		if ( ValidatorCtx->this_level->AllWaypoints[i].num_connections == 0 ) continue;
		
		for ( j = 0; j < ValidatorCtx->this_level->AllWaypoints[i].num_connections; ++j )
		{
			int wp = ValidatorCtx->this_level->AllWaypoints[i].connections[j];
			
			if ( !DirectLineColldet(ValidatorCtx->this_level->AllWaypoints[i].x + 0.5, ValidatorCtx->this_level->AllWaypoints[i].y + 0.5,
			                        ValidatorCtx->this_level->AllWaypoints[wp].x + 0.5, ValidatorCtx->this_level->AllWaypoints[wp].y + 0.5,
			                        ValidatorCtx->this_level->levelnum, &WalkablePassFilter) )
			{
				gps from_pos = { ValidatorCtx->this_level->AllWaypoints[i].x + 0.5, ValidatorCtx->this_level->AllWaypoints[i].y + 0.5, ValidatorCtx->this_level->levelnum };
				moderately_finepoint to_pos = { ValidatorCtx->this_level->AllWaypoints[wp].x + 0.5, ValidatorCtx->this_level->AllWaypoints[wp].y + 0.5 };
				moderately_finepoint mid_pos[40];
				
				int path_found = set_up_intermediate_course_between_positions ( NULL, FALSE, &from_pos, &to_pos, mid_pos, 40);
				int nb_mp = 0;
				if ( path_found ) while ( mid_pos[nb_mp++].x != -1 );
				
				if ( !path_found || (nb_mp>5) )
				{
					if ( !path_is_invalid )
					{	// First error : print header
						ValidatorPrintHeader(ValidatorCtx, "Invalid waypoint paths list",
						                                   "The pathfinder was not able to find a path between those waypoints.\n"
								                           "This could lead those paths to not being usable." );
						path_is_invalid = TRUE;
					}
					printf( "Path: %f/%f -> %f/%f (%s)\n", 
					        ValidatorCtx->this_level->AllWaypoints[i].x + 0.5, ValidatorCtx->this_level->AllWaypoints[i].y + 0.5,
					        ValidatorCtx->this_level->AllWaypoints[wp].x + 0.5, ValidatorCtx->this_level->AllWaypoints[wp].y + 0.5,
					        (!path_found)?"path not found":"too complex");
				}
			}
		}
	}
	if ( path_is_invalid ) puts( line );

	return ( pos_is_invalid || conn_is_invalid || path_is_invalid );
}

/**
 * This validator checks if level interfaces are valid
 */

int interface_validator( level_validator_ctx* ValidatorCtx )
{
	int x_tile, y_tile, glue_index;
	int is_invalid = FALSE;

	for ( y_tile = 0 ; y_tile < ValidatorCtx->this_level->ylen ; ++y_tile )
	{
		for ( x_tile = 0 ; x_tile < ValidatorCtx->this_level->xlen ; ++x_tile )
		{
			if ( ( y_tile > ValidatorCtx->this_level->jump_threshold_north/2.0 ) && ( y_tile < (ValidatorCtx->this_level->ylen - ValidatorCtx->this_level->jump_threshold_south/2.0) ) &&
			     ( x_tile > ValidatorCtx->this_level->jump_threshold_west/2.0 )  && ( x_tile < (ValidatorCtx->this_level->xlen - ValidatorCtx->this_level->jump_threshold_east/2.0) ) )
				continue;
			
			for ( glue_index = 0 ; glue_index < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE ; ++glue_index )
			{
				int obs_index = ValidatorCtx->this_level->map[y_tile][x_tile].obstacles_glued_to_here[glue_index];
				if ( obs_index == (-1) ) break;
			
				obstacle* this_obs = &(ValidatorCtx->this_level->obstacle_list[obs_index]);
			
				if ( ! (IS_CHEST(this_obs->type) || IS_BARREL(this_obs->type))  ) continue;
			
				if ( !(is_invalid) )
				{	// First error : print header
					ValidatorPrintHeader(ValidatorCtx, "Invalid level-interfaces list",
					                                   "The following objects were found on the interface area.\n"
							                           "The activation of those objects will not be reflected on the neighborhood." );
					is_invalid = TRUE;
				}
				printf( "Idx: %d (%s) - Pos: %f/%f\n", obs_index, obstacle_map[this_obs->type].obstacle_short_name, this_obs->pos.x, this_obs->pos.y );
			}
		}
	}
	if ( is_invalid ) puts( line );
	
	return is_invalid;
}

#undef _leveleditor_validator_c
