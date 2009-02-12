/* 
 *
 *   Copyright (c) 2008-2009 Samuel Degrande
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

#include "lvledit/lvledit_validator.h"
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
		jumptarget_validator,
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
 * Check if the connection between two waypoints is valid
 * This is an helper function for waypoint_validator()
 */
enum connect_validity {
	DIRECT_CONN  = 0,
	NEED_PATH    = 1,
	NO_PATH      = 2,
	COMPLEX_PATH = 4
};

static enum connect_validity waypoints_connection_valid( gps* from_pos, gps* to_pos )
{
	if ( DirectLineColldet(from_pos->x, from_pos->y, to_pos->x, to_pos->y, from_pos->z, &WalkablePassFilter) )
		return DIRECT_CONN;
	
	moderately_finepoint mfp_to_pos = { to_pos->x, to_pos->y };
	moderately_finepoint mid_pos[40];
	
	pathfinder_context pf_ctx = { &WalkablePassFilter, NULL };
	
	int path_found = set_up_intermediate_course_between_positions ( from_pos, &mfp_to_pos, mid_pos, 40, &pf_ctx);
	if ( !path_found ) return NO_PATH;
		
	int nb_mp = 0;
	while ( mid_pos[nb_mp++].x != -1 );
	if ( nb_mp>5 ) return (NO_PATH & COMPLEX_PATH);
		
	return NEED_PATH; 
}

/**
 * This validator checks if waypoints are valid
 */

int waypoint_validator( level_validator_ctx* ValidatorCtx )
{
	int i, j;
	int pos_is_invalid = FALSE;
	int conn_is_invalid = FALSE;
	int dist_is_invalid = FALSE;
	int path_is_invalid = FALSE;
	int path_warning = FALSE;
#	define TRSL_FACT 0.02
#	define MIN_DIST 1.0
	
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
			printf( "wp#%d (%f/%f)\n", i, ValidatorCtx->this_level->AllWaypoints[i].x + 0.5, ValidatorCtx->this_level->AllWaypoints[i].y + 0.5);			
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
				                                   "The following waypoints were found to be without connection (O).\n"
						                           "or self-connected (S).\n"
						                           "This could lead to some bots being stuck on those waypoints." );
				conn_is_invalid = TRUE;
			}
			printf( "(O) wp#%d (%f/%f)\n", i, ValidatorCtx->this_level->AllWaypoints[i].x + 0.5, ValidatorCtx->this_level->AllWaypoints[i].y + 0.5 );			
		}
		for ( j = 0; j < ValidatorCtx->this_level->AllWaypoints[i].num_connections; ++j )
		{
			int wp = ValidatorCtx->this_level->AllWaypoints[i].connections[j];
			if ( wp == i )
			{
				if ( !conn_is_invalid )
				{	// First error : print header
					ValidatorPrintHeader(ValidatorCtx, "Unconnected waypoints list",
					                                   "The following waypoints were found to be without connection (O).\n"
							                           "or self-connected (S).\n"
							                           "This could lead to some bots being stuck on those waypoints." );
					conn_is_invalid = TRUE;
				}
				printf( "(S) wp#%d (%f/%f)\n", i, ValidatorCtx->this_level->AllWaypoints[i].x + 0.5, ValidatorCtx->this_level->AllWaypoints[i].y + 0.5 );
				continue;
			}
		}
	}
	if ( conn_is_invalid ) puts( line );
	
	// Check waypoints distance
	for ( i = 0; i < ValidatorCtx->this_level->num_waypoints - 1; ++i )
	{
		gps wp_i = { ValidatorCtx->this_level->AllWaypoints[i].x + 0.5,  ValidatorCtx->this_level->AllWaypoints[i].y + 0.5,  ValidatorCtx->this_level->levelnum };

		for ( j = i+1; j < ValidatorCtx->this_level->num_waypoints; ++j )
		{			
			gps wp_j = { ValidatorCtx->this_level->AllWaypoints[j].x + 0.5,  ValidatorCtx->this_level->AllWaypoints[j].y + 0.5,  ValidatorCtx->this_level->levelnum };
			float dist = sqrt( (wp_j.x - wp_i.x)*(wp_j.x - wp_i.x) + (wp_j.y - wp_i.y)*(wp_j.y - wp_i.y) );
			
			if ( dist < MIN_DIST ) {
				if ( !dist_is_invalid )
				{	// First error : print header
					ValidatorPrintHeader(ValidatorCtx, "Invalid waypoints distance",
					                                   "Two waypoints were found to be too close" );
					dist_is_invalid = TRUE;
				}
				printf( "wp#%d (%f/%f) - wp#%d (%f/%f) : distance = %.3f\n", 
				        i, wp_i.x, wp_i.y, j, wp_j.x, wp_j.y, dist);
			}
		}
	}
	
	// Check waypoint paths walkability
	for ( i = 0; i < ValidatorCtx->this_level->num_waypoints; ++i )
	{
		if ( ValidatorCtx->this_level->AllWaypoints[i].num_connections == 0 ) continue;
		
		gps from_pos = { ValidatorCtx->this_level->AllWaypoints[i].x + 0.5,  ValidatorCtx->this_level->AllWaypoints[i].y + 0.5,  ValidatorCtx->this_level->levelnum };

		for ( j = 0; j < ValidatorCtx->this_level->AllWaypoints[i].num_connections; ++j )
		{
			int wp = ValidatorCtx->this_level->AllWaypoints[i].connections[j];

			gps to_pos   = { ValidatorCtx->this_level->AllWaypoints[wp].x + 0.5, ValidatorCtx->this_level->AllWaypoints[wp].y + 0.5, ValidatorCtx->this_level->levelnum };

			enum connect_validity rtn = waypoints_connection_valid(&from_pos, &to_pos);
			if ( rtn & NO_PATH )
			{
				if ( !path_is_invalid )
				{	// First error : print header
					ValidatorPrintHeader(ValidatorCtx, "Invalid waypoint paths list",
					                                   "The pathfinder was not able to find a path between those waypoints.\n"
							                           "This could lead those paths to not being usable." );
					path_is_invalid = TRUE;
				}
				printf( "Path: wp#%d (%f/%f) -> wp#%d (%f/%f) (%s)\n", 
				        i, from_pos.x, from_pos.y, wp, to_pos.x, to_pos.y,
				        (rtn & COMPLEX_PATH)?"too complex":"path not found");
			}			
		}
	}
	if ( path_is_invalid ) puts( line );

	// Sometimes, a bot does not exactly follow the path between the waypoints
	// (perhaps due to floating point approximations).
	// So, even if the connection is in theory walkable, a bot could get stuck.
	// By translating a bit the waypoints positions, we simulate such a behavior.

	for ( i = 0; i < ValidatorCtx->this_level->num_waypoints; ++i )
	{
		if ( ValidatorCtx->this_level->AllWaypoints[i].num_connections == 0 ) continue;
		
		for ( j = 0; j < ValidatorCtx->this_level->AllWaypoints[i].num_connections; ++j )
		{
			int wp = ValidatorCtx->this_level->AllWaypoints[i].connections[j];

			gps from_pos = { ValidatorCtx->this_level->AllWaypoints[i].x + 0.5,  ValidatorCtx->this_level->AllWaypoints[i].y + 0.5,  ValidatorCtx->this_level->levelnum };
			gps to_pos   = { ValidatorCtx->this_level->AllWaypoints[wp].x + 0.5, ValidatorCtx->this_level->AllWaypoints[wp].y + 0.5, ValidatorCtx->this_level->levelnum };
			
			// Translation vector
			moderately_finepoint line_vector;
			line_vector.x = to_pos.x - from_pos.x;
			line_vector.y = to_pos.y - from_pos.y;
			
			float length = sqrtf(line_vector.x * line_vector.x + line_vector.y * line_vector.y);
			if ( length < MIN_DIST ) continue; // Too close waypoints. Already handled

			line_vector.x = (line_vector.x * TRSL_FACT) / length;
			line_vector.y = (line_vector.y * TRSL_FACT) / length;
			
			// Translation normal
			moderately_finepoint line_normal = { -line_vector.y, line_vector.x };

			// 1- Augment the length
			gps trsl_from_pos = { from_pos.x - line_vector.x, from_pos.y - line_vector.y, from_pos.z };
			gps trsl_to_pos   = { to_pos.x   + line_vector.x, to_pos.y   + line_vector.y, to_pos.z   };

			enum connect_validity rtn = waypoints_connection_valid(&trsl_from_pos, &trsl_to_pos);
			if ( rtn & NO_PATH )
			{
				if ( !path_warning )
				{	// First error : print header
					ValidatorPrintHeader(ValidatorCtx, "Waypoint paths Warning list",
					                                   "The pathfinder was not able to find a path between two variations of those waypoints.\n"
							                           "This could lead some bots to get stuck along those paths." );
					path_warning = TRUE;
				}
				printf( "Path: wp#%d (%f/%f) -> wp#%d (%f/%f) (warning)\n", 
				        i, from_pos.x, from_pos.y, wp, to_pos.x, to_pos.y);
				
				continue; // Next connection
			}
			
			// 2- Translate up in the direction of the normal
			trsl_from_pos.x += line_normal.x;
			trsl_from_pos.y += line_normal.y;
			trsl_to_pos.x   += line_normal.x;
			trsl_to_pos.y   += line_normal.y;

			rtn = waypoints_connection_valid(&trsl_from_pos, &trsl_to_pos);
			if ( rtn & NO_PATH )
			{
				if ( !path_warning )
				{	// First error : print header
					ValidatorPrintHeader(ValidatorCtx, "Waypoint paths Warning list",
					                                   "The pathfinder was not able to find a path between two variations of those waypoints.\n"
							                           "This could lead some bots to get stuck along those paths." );
					path_warning = TRUE;
				}
				printf( "Path: wp#%d (%f/%f) -> wp#%d (%f/%f) (warning)\n", 
				        i, from_pos.x, from_pos.y, wp, to_pos.x, to_pos.y);
					
				continue; // Next connection
			}
			
			// 3- Translate down in the direction of the normal
			trsl_from_pos.x -= 2 * line_normal.x;
			trsl_from_pos.y -= 2 * line_normal.y;
			trsl_to_pos.x   -= 2 * line_normal.x;
			trsl_to_pos.y   -= 2 * line_normal.y;

			rtn = waypoints_connection_valid(&trsl_from_pos, &trsl_to_pos);
			if ( rtn & NO_PATH )
			{
				if ( !path_warning )
				{	// First error : print header
					ValidatorPrintHeader(ValidatorCtx, "Waypoint paths Warning list",
					                                   "The pathfinder was not able to find a path between two variations of those waypoints.\n"
							                           "This could lead some bots to get stuck along those paths." );
					path_warning = TRUE;
				}
				printf( "Path: wp#%d (%f/%f) -> wp#%d (%f/%f) (warning)\n", 
				        i, from_pos.x, from_pos.y, wp, to_pos.x, to_pos.y);
			}
		}
	}
	if ( path_warning ) puts( line );

	return ( pos_is_invalid || conn_is_invalid || dist_is_invalid || path_is_invalid || path_warning );
}

/**
 * This validator checks if level interfaces are valid
 */
int jumptarget_validator(level_validator_ctx *ValidatorCtx) 
{
    int is_invalid = FALSE;

    if (ValidatorCtx->this_level->jump_target_north != -1 && curShip.AllLevels[ValidatorCtx->this_level->jump_target_north] == NULL) {
	ValidatorPrintHeader(ValidatorCtx, "Non existant jump target", "The north jump target on a level points to a non existing level.");
	is_invalid = TRUE;

	printf("Level %d has north interface pointing to %d which does not exist.\n", ValidatorCtx->this_level->levelnum, ValidatorCtx->this_level->jump_target_north);
    }

    if (ValidatorCtx->this_level->jump_target_west != -1 && curShip.AllLevels[ValidatorCtx->this_level->jump_target_west] == NULL) {
	ValidatorPrintHeader(ValidatorCtx, "Non existant jump target", "The west jump target on a level points to a non existing level.");
	is_invalid = TRUE;

	printf("Level %d has west interface pointing to %d which does not exist.\n", ValidatorCtx->this_level->levelnum, ValidatorCtx->this_level->jump_target_west);
    }

    if (ValidatorCtx->this_level->jump_target_east != -1 && curShip.AllLevels[ValidatorCtx->this_level->jump_target_east] == NULL) {
	ValidatorPrintHeader(ValidatorCtx, "Non existant jump target", "The east jump target on a level points to a non existing level.");
	is_invalid = TRUE;

	printf("Level %d has east interface pointing to %d which does not exist.\n", ValidatorCtx->this_level->levelnum, ValidatorCtx->this_level->jump_target_east);
    }

    if (ValidatorCtx->this_level->jump_target_south != -1 && curShip.AllLevels[ValidatorCtx->this_level->jump_target_south] == NULL) {
	ValidatorPrintHeader(ValidatorCtx, "Non existant jump target", "The south jump target on a level points to a non existing level.");
	is_invalid = TRUE;

	printf("Level %d has south interface pointing to %d which does not exist.\n", ValidatorCtx->this_level->levelnum, ValidatorCtx->this_level->jump_target_south);
    }

    return is_invalid;
}


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
				printf( "obs#%d (%f/%f) : %s\n", obs_index, this_obs->pos.x, this_obs->pos.y, obstacle_map[this_obs->type].obstacle_short_name );
			}
		}
	}
	if ( is_invalid ) puts( line );
	
	return is_invalid;
}


/**
 * Run several validations
 */
void LevelValidation()
{
	int is_invalid = FALSE;

	SDL_Rect BackgroundRect = { UNIVERSAL_COORD_W(20), UNIVERSAL_COORD_H(20), UNIVERSAL_COORD_W(600), UNIVERSAL_COORD_H(440) };
	SDL_Rect ReportRect     = { UNIVERSAL_COORD_W(30), UNIVERSAL_COORD_H(30), UNIVERSAL_COORD_W(580), UNIVERSAL_COORD_H(420) };

	BFont_Info* current_font = GetCurrentFont();
	int raw_height = FontHeight( current_font );
	int max_raws = (ReportRect.h / raw_height) - 4; // 4 lines are reserved for header and footer 
	int column_width = TextWidth( "Level 000: empty" );

	AssembleCombatPicture ( ONLY_SHOW_MAP_AND_TEXT | SHOW_GRID | SKIP_LIGHT_RADIUS );
	ShadowingRectangle ( Screen, BackgroundRect );

	//--------------------
	// Title
	//
	CenteredPutString( Screen, ReportRect.y, "Level Validation tests - Summary\n" );

	//--------------------
	// Loop on each level
	//
	int l;
	int col_pos = 0;
	int raw_pos = 0;
	
	for ( l = 0; l < curShip.num_levels; ++l )
	{
		level_validator_ctx ValidatorCtx = { &ReportRect, curShip.AllLevels[l] };

		// Compute raw and column position, when a new column of text starts
		if ( (l % max_raws) == 0 )
		{
			col_pos = ReportRect.x + (l/max_raws) * column_width;
			raw_pos = ReportRect.y + 2 * raw_height; // 2 lines are reserved for the header
			SetTextCursor( col_pos, raw_pos);
		}
		
		if ( curShip.AllLevels[l] == NULL )
		{
			// Empty level
			char txt[40];
			sprintf(txt, "%s %3d: \2empty\n", "Level", l );
			DisplayText( txt, col_pos, -1, &ReportRect, 1.0 );			
			SetCurrentFont( current_font ); // Reset font
		}
		else
		{
		// Loop on each validation function
		int v = 0;
		level_validator one_validator;
		int level_is_invalid = FALSE;

		while ( (one_validator = level_validators[v++]) != NULL ) level_is_invalid |= one_validator(&ValidatorCtx);

		// Display report
		char txt[40];
		sprintf(txt, "%s %3d: %s\n", "Level", l, (level_is_invalid)?"\1fail":"pass" );
		DisplayText( txt, col_pos, -1, &ReportRect, 1.0 );
		SetCurrentFont( current_font ); // Reset font in case of the red "fail" was displayed

		// Set global is_invalid flag
		is_invalid |= level_is_invalid;
	}
	}

	//--------------------
	// This was it.  We can say so and return.
	//
	if ( is_invalid ) CenteredPutString( Screen, ReportRect.y + ReportRect.h - 2.0*raw_height, "\1Some tests were invalid. See the report in the console\3" );

	CenteredPutString( Screen, ReportRect.y + ReportRect.h - raw_height, "--- End of List --- Press Space to return to leveleditor ---" );

	our_SDL_flip_wrapper();

} // LevelValidation( int levelnum )

#undef _leveleditor_validator_c
