/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2007 Arthur Huillet 
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
 * This file contains all the functions managing the lighting inside
 * the freedroidRPG game window, i.e. the 'light radius' of the Tux and
 * of other light emanating objects.
 */

#define _light_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "map.h"
#include "proto.h"
#include "SDL_rotozoom.h"

//--------------------
// 
#define MAX_NUMBER_OF_LIGHT_SOURCES 100
moderately_finepoint light_sources [ MAX_NUMBER_OF_LIGHT_SOURCES ] ;
int light_source_strengthes [ MAX_NUMBER_OF_LIGHT_SOURCES ] ;

light_radius_config LightRadiusConfig = { -1, -1, -1, -1, 0.0 };
int* light_strength_buffer = NULL;
#define LIGHT_RADIUS_TEXTURE_DIVISOR 10 // Default number of screen pixels per cell
#define LIGHT_RADIUS_TEXTURE_MAX_SIZE 64 // texture max size is 64x64
#define LIGHT_STRENGTH_CELL_ADDR(x,y) (light_strength_buffer + (y)*LightRadiusConfig.cells_w + (x))
#define LIGHT_STRENGTH_CELL(x,y) *(int*)(light_strength_buffer + (y)*LightRadiusConfig.cells_w + (x))

#define UNIVERSAL_TUX_HEIGHT 100.0 // Empirical Tux height, in the universal coordinate system

/**
 * Compute the light_radius texture size, and allocate it
 */
void
LightRadiusInit()
{
	// Divide to screen dimensions by default divisor, to compute the default
	// number of cells in the light_radius texture
	LightRadiusConfig.cells_w = round( (float)GameConfig.screen_width / (float)LIGHT_RADIUS_TEXTURE_DIVISOR );
	LightRadiusConfig.cells_h = round( (float)GameConfig.screen_height / (float)LIGHT_RADIUS_TEXTURE_DIVISOR );
	
	// Find the the nearest power-of-two greater than of equal to the number of cells
	// and limit the texture's size
	int pot_w = pot_gte( LightRadiusConfig.cells_w );
	if ( pot_w > LIGHT_RADIUS_TEXTURE_MAX_SIZE ) pot_w = LIGHT_RADIUS_TEXTURE_MAX_SIZE;
	int pot_h = pot_gte( LightRadiusConfig.cells_h  );
	if ( pot_h > LIGHT_RADIUS_TEXTURE_MAX_SIZE ) pot_h = LIGHT_RADIUS_TEXTURE_MAX_SIZE;
	
	if ( GameConfig.screen_width >= GameConfig.screen_height )
	{
		// Use the whole texture width
		LightRadiusConfig.cells_w = pot_w;
		LightRadiusConfig.texture_w = pot_w;
		// Compute the actual scale factor when using the whole texture width
		LightRadiusConfig.scale_factor = (float)GameConfig.screen_width / (float)LightRadiusConfig.cells_w;
		// Since we have an homogeneous scale factor in X and Y,
		// compute the new number of cells in Y
		LightRadiusConfig.cells_h = (int)ceilf( (float)GameConfig.screen_height / LightRadiusConfig.scale_factor);
		// Check if the texture's height is now big enough. If not, take the next power-of-two
		if ( LightRadiusConfig.cells_h <= pot_h ) LightRadiusConfig.texture_h = pot_h;
		else LightRadiusConfig.texture_h = pot_h << 1;
	}
	else
	{
		// Same thing, if screen's height > screen's width
		// Is it ever a used screen ratio ? Well, when fdrpg will run on smartphones,
		// it could be :-)
		LightRadiusConfig.cells_h = pot_h; 
		LightRadiusConfig.texture_h = pot_h;
		LightRadiusConfig.scale_factor = (float)GameConfig.screen_height / (float)LightRadiusConfig.cells_h;
		LightRadiusConfig.cells_w = (int)ceilf( (float)GameConfig.screen_width / LightRadiusConfig.scale_factor);
		if ( LightRadiusConfig.cells_w <= pot_w ) LightRadiusConfig.texture_w = pot_w;
		else LightRadiusConfig.texture_w = pot_w << 1;
	}
	
	//----------
	// The center of the light_radius texture will be translated along the Y axe,
	// to simulate a light coming from bot/tux heads, instead of their feet.
	
	// First: convert a universal unit into a screen height. 
	// But because the UNIVERSAL_COORD macros currently works only for 4:3 screen,
	// we take the real screen ration into account
	float unit_screen_height = UNIVERSAL_COORD_H( ((float)GameConfig.screen_width/(float)GameConfig.screen_height) / (4.0/3.0) );
	
	// Second : transform Tux universal height into screen height
	LightRadiusConfig.translate_y = (int)( UNIVERSAL_TUX_HEIGHT / unit_screen_height );
	
	//----------
	// Allocate the light_radius buffer
	light_strength_buffer = malloc( LightRadiusConfig.cells_w * LightRadiusConfig.cells_h * sizeof(int) );
	
} // void LightRadiusInit();

/**
 * Clean and deallocate light_radius texture
 */
void
LightRadiusClean()
{
	if ( light_strength_buffer ) 
	{
		free( light_strength_buffer );
		light_strength_buffer = NULL;
	}
}

/**
 * There might be some obstacles that emit some light.  Yet, we can't
 * go through all the obstacle list of a level every frame at sufficiently
 * low cost.  Therefore we create a reduced list of light emitters for
 * usage in the light computation code.
 */
void 
update_light_list ( )
{
    int i;
    Level light_level = curShip . AllLevels [ Me . pos . z ] ;
    int map_x, map_y, map_x_end, map_y_end, map_x_start, map_y_start ;
    int glue_index;
    int obs_index;
    int next_light_emitter_index;
    obstacle* emitter;
    int blast;

    //--------------------
    // At first we fill out the light sources array with 'empty' information,
    // i.e. such positions, that won't affect our location for sure.
    //
    for ( i = 0 ; i < MAX_NUMBER_OF_LIGHT_SOURCES ; i ++ )
    {
	light_sources [ i ] . x = -200 ;
	light_sources [ i ] . y = -200 ;
	light_source_strengthes [ i ] = 0 ;
    }
    
    //--------------------
    // Now we fill in the Tux position as the very first light source, that will
    // always be present.
    //
    light_sources [ 0 ] . x = Me . pos . x ;
    light_sources [ 0 ] . y = Me . pos . y ;
    light_source_strengthes [ 0 ] = 
	light_level -> light_radius_bonus + Me . light_bonus_from_tux  ;
    //--------------------
    // We must not in any case tear a hole into the beginning of
    // the list though...
    //
    if ( light_source_strengthes [ 0 ] <= 0 )
	light_source_strengthes [ 0 ] = 1;
    next_light_emitter_index = 1 ;

    //--------------------
    // Now we can fill in any explosions, that are currently going on.
    // These will typically emanate a lot of light.
    //
    for ( blast = 0 ; blast < MAXBLASTS ; blast ++ )
    {
	if ( ! ( AllBlasts [ blast ] . type == DRUIDBLAST ) ) continue ;

	light_sources [ next_light_emitter_index ] . x = AllBlasts [ blast ] . pos . x ;
	light_sources [ next_light_emitter_index ] . y = AllBlasts [ blast ] . pos . y ;

	//--------------------
	// We add some light strength according to the phase of the blast
	//
	light_source_strengthes [ next_light_emitter_index ] = 10 - AllBlasts [ blast ] . phase / 2 ;
	if ( light_source_strengthes [ next_light_emitter_index ] < 0 )
	    continue;
	next_light_emitter_index ++ ;
	
	//--------------------
	// We must not write beyond the bounds of our light sources array!
	//
	if ( next_light_emitter_index >= MAX_NUMBER_OF_LIGHT_SOURCES - 1 )
	{
	    ErrorMessage ( __FUNCTION__  , "\
WARNING!  End of light sources array reached!",
				       NO_NEED_TO_INFORM, IS_WARNING_ONLY );
	    return;
	}
    }

    //--------------------
    // Now we can fill in the remaining light sources of this level.
    // First we do all the obstacles:
    //
    map_x_start = Me . pos . x - 12 ;
    map_y_start = Me . pos . y - 12 ;
    map_x_end = Me . pos . x + 12 ;
    map_y_end = Me . pos . y + 12 ;
    if ( map_x_start < 0 ) map_x_start = 0 ;
    if ( map_y_start < 0 ) map_y_start = 0 ;
    if ( map_x_end >= light_level -> xlen ) map_x_end = light_level -> xlen - 1 ;
    if ( map_y_end >= light_level -> ylen ) map_y_end = light_level -> ylen - 1 ;
	for ( map_y = map_y_start ; map_y < map_y_end ; map_y ++ )
	{
    for ( map_x = map_x_start ; map_x < map_x_end ; map_x ++ )
    {
	    for ( glue_index = 0 ; glue_index < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE ; glue_index ++ )
	    {
		//--------------------
		// end of obstacles glued to here?  great->next square
		//
		if ( light_level -> map [ map_y ] [ map_x ] . obstacles_glued_to_here [ glue_index ] == (-1) )
		    break;

		obs_index = light_level -> map [ map_y ] [ map_x ] . obstacles_glued_to_here [ glue_index ] ;
		emitter = & ( light_level -> obstacle_list [ obs_index ] );

		if ( obstacle_map [ emitter -> type ] . emitted_light_strength == 0 )
		    continue;

		//--------------------
		// Now we know that this one needs to be inserted!
		//
		light_sources [ next_light_emitter_index ] . x = emitter -> pos . x ;
		light_sources [ next_light_emitter_index ] . y = emitter -> pos . y ;
		light_source_strengthes [ next_light_emitter_index ] = 
		    obstacle_map [ emitter -> type ] . emitted_light_strength ;
		next_light_emitter_index ++ ;

		//--------------------
		// We must not write beyond the bounds of our light sources array!
		//
		if ( next_light_emitter_index >= MAX_NUMBER_OF_LIGHT_SOURCES - 1 )
		{
		    ErrorMessage ( __FUNCTION__  , "\
WARNING!  End of light sources array reached!",
					       NO_NEED_TO_INFORM, IS_WARNING_ONLY );
		    return;
		}
	    }
	}
    }
   
    enemy *erot;
    BROWSE_LEVEL_BOTS(erot, Me.pos.z) 
    {
	if ( fabsf ( Me . pos . x - erot->pos . x ) >= 12 ) 
	    continue;

	if ( fabsf ( Me . pos . y - erot->pos . y ) >= 12 )
	    continue;

	//--------------------
	// Now we know that this one needs to be inserted!
	//
	light_sources [ next_light_emitter_index ] . x = erot->pos . x ;
	light_sources [ next_light_emitter_index ] . y = erot->pos . y ;
	light_source_strengthes [ next_light_emitter_index ] = -14 ;
	next_light_emitter_index ++ ;

	//--------------------
	// We must not write beyond the bounds of our light sources array!
	//
	if ( next_light_emitter_index >= MAX_NUMBER_OF_LIGHT_SOURCES - 1 )
	{
	    ErrorMessage ( __FUNCTION__  , "\
WARNING!  End of light sources array reached!",
				       NO_NEED_TO_INFORM, IS_WARNING_ONLY );
	    return;
	}
	
    }
    
}; // void update_light_list ( )

/**
 * This function is used to find the light intensity at any given point
 * on the map.
 * Take care : despite it's name, it computes a darkness value, which is the opposite of
 * an intensity (Darkness = -Intensity)
 */
static int 
calculate_light_strength ( moderately_finepoint target_pos )
{
	int i;
	float xdist;
	float ydist;
	float squared_dist;

	// Full bright: final_darkness = 0 / Full dark: final_darkness = (NUMBER_OF_SHADOW_IMAGES - 1) or -(minimum light value)
	int final_darkness = min( NUMBER_OF_SHADOW_IMAGES - 1, -(curShip.AllLevels[Me.pos.z]->minimum_light_value) );

	//------------------------------------------
	// Nota: the following code being quite obscure, so some explanations are quite mandatory:
	//
	// 1) Despite its name, this function compute a darkness, and not a light intensity
	//
	// 2) The light emitted from a light source decreases linearly with the distance.
	//    So, at a given target position, the perceived intensity of a light is: 
	//    I = Light_strength - 4.0*distance(Light_pos, Target_pos)
	//        (the 4.0 factor means that a light source with a strength of 1 has a radius of 0.25)
	//    And the resulting darkness is:
	//    D = -I 
	//    D is clamped between 0 and the full darkness value of this level (due to final_darkness initialization)
	//
	// 3) The code loops on each light source, and keeps the minimum darkness (and so maximum light intensity).
	//    It does not accumulate light intensity.
	//
	// So the initial code is :
	//
	// 1: foreach this_light in (set of lights)
	// 2: {
	// 3:   if ( this_light is visible from the target )
	// 4:   {
	// 5:     this_light_darkness = 4.0 * distance( this_light.pos, target.pos) - this_light.strength;
	// 6:     if ( this_ligh_darkness < final_darkness ) final_darkness = this_light_darkness;
	// 7:   }
	// 8: }
	//
	// However, this function is time-critical, simply because it's used a lot every frame for 
	// the light radius. Therefore we want to avoid the sqrt() needed to compute a distance, if the test fails,
	// and instead use squared values as much as possible.
	//
	// So, lines 5 and 6 are transformed into:
	//       if ( 4.0 * distance - strength < final_darkness ) final_darkness = 4.0 * distance - strength;
	// which are then transformed into:
	//       if ( 4.0 * distance < final_darkness + strength ) final_darkness = 4.0 * distance - strength;
	// and finally, in order to remove sqrt(), into:
	//       if ( (4.0 * distance)^2 < (final_darkness + strength)^2 ) final_darkness = 4.0 * distance - strength;
	// (note: we will ensure that final_darkness + strength is > 0, so there is no problem
	//  with the sign of the inequality. see optimization 1 in the code)
	//
	// Visibility checking being far more costly than the darkness test, we revert the 2 tests :
	//
	// 1: foreach this_light in (set of lights)
	// 2: {
	// 3:   if ( (4.0 * distance( this_light.pos, target.pos))^2 < (final_darkness + this_light.strength)^2 )
	// 4:   {
	// 5:     if ( this_light is visible from the target ) 
	// 6:       final_darkness = 4.0 * distance( this_light.pos, target.pos) - this_light.strength;
	// 7:   }
	// 8: }
	//------------------------------------------

	//--------------------
	// Now, here is the final algorithm
	//
	for ( i = 0 ; i < MAX_NUMBER_OF_LIGHT_SOURCES ; i ++ )
	{
		//--------------------
		// If we've reached the end of the current list of light
		// sources, then we can stop immediately.
		//
		if ( light_source_strengthes [ i ] == 0 ) break;

		//--------------------
		// Otimization 1:
		// If the absolute strength of the light source (i.e. it's intensity
		// at the source position) is less than the current intensity, then it's
		// even not needed to continue with this light source.
		if ( light_source_strengthes [ i ] <= (-final_darkness) ) continue;

		//--------------------
		// Some pre-computations
		xdist = light_sources [ i ] . x - target_pos . x ;
		ydist = light_sources [ i ] . y - target_pos . y ;
		squared_dist = xdist * xdist + ydist * ydist;

		//--------------------
		// Comparison between current darkness and the one from the source (line 3 of the pseudo-code)
		if ( ( squared_dist * 4.0 * 4.0 ) >= ( final_darkness + light_source_strengthes[i] ) * ( final_darkness + light_source_strengthes[i] ) )
			continue;

		//--------------------
		// Visibility check (line 5 of pseudo_code)
		// with a small optimization : no visibility check if the target is very closed to the light
		if ( ( squared_dist > (0.5*0.5) ) && curShip.AllLevels[Me.pos.z]->use_underground_lighting )
		{
			if ( ! DirectLineColldet( light_sources[i].x, light_sources[i].y, target_pos.x, target_pos.y, Me.pos.z, &FilterVisible ) )
				continue;
		}

		final_darkness = ( sqrt(squared_dist) * 4.0 ) - light_source_strengthes[i];

		// Full bright, no need to test any other light source
		// Note: this comparison cannot be transformed into (16*squared_dist < light_source_stengthes^2), 
		// because light_source_strengthes can be a positive or a negative value
		if ( final_darkness < 0 ) return 0;
	}
    
	return ( final_darkness );
    
}; // int calculate_light_strength ( moderately_finepoint target_pos )

/**
 * When the light radius (i.e. the shadow values for the floor) has been
 * set up, the shadows usually are very 'hard' in the sense that extreme
 * darkness can be right next to very bright light.  This does not look
 * very real.  Therefore we 'soften' the shadow a bit, by allowing only
 * a limited step size from one shadow square to the next.  Of course 
 * these changes have to be propagated, so we run through the whole
 * shadow grid twice and propagate in 'both' directions.  
 * The hardness of the shadow can be controlled in the definition of
 * MAX_LIGHT_STEP.  Higher values will lead to harder shadows, while 
 * lower values will give very smooth and flourescent shadows propagating
 * even a bit under walls (which doesn't look too good).  4 seems like
 * a reasonable setting.
 */
static void
soften_light_distribution ( void )
{
#define MAX_LIGHT_STEP 3
    uint32_t x , y ;

    //--------------------
    // Now that the light buffer has been set up properly, we can start to
    // smoothen it out a bit.  We do so in the direction of more light.
	// Propagate from top-left to bottom-right
    //
	for ( y = 0 ; y < (LightRadiusConfig.cells_h-1) ; ++y )
    {
		for ( x = 0 ; x < (LightRadiusConfig.cells_w-1) ; ++x )
	{
			if ( LIGHT_STRENGTH_CELL(x+1, y) > LIGHT_STRENGTH_CELL(x, y) + MAX_LIGHT_STEP )
				LIGHT_STRENGTH_CELL(x+1, y) = LIGHT_STRENGTH_CELL(x, y) + MAX_LIGHT_STEP;
			if ( LIGHT_STRENGTH_CELL(x, y+1) > LIGHT_STRENGTH_CELL(x, y) + MAX_LIGHT_STEP )
				LIGHT_STRENGTH_CELL(x, y+1) = LIGHT_STRENGTH_CELL(x, y) + MAX_LIGHT_STEP;
			if ( LIGHT_STRENGTH_CELL(x+1, y+1) > LIGHT_STRENGTH_CELL(x, y) + MAX_LIGHT_STEP )
				LIGHT_STRENGTH_CELL(x+1, y+1) = LIGHT_STRENGTH_CELL(x, y) + MAX_LIGHT_STEP;
	}
    }
	// now the same again, this time from bottom-right to top-left
	for ( y = (LightRadiusConfig.cells_h-1) ; y > 0 ; --y )
    {
		for ( x = (LightRadiusConfig.cells_w-1) ; x > 0 ; --x )
	{
			if ( LIGHT_STRENGTH_CELL(x-1, y) > LIGHT_STRENGTH_CELL(x, y) + MAX_LIGHT_STEP )
				LIGHT_STRENGTH_CELL(x-1, y) = LIGHT_STRENGTH_CELL(x, y) + MAX_LIGHT_STEP;
			if ( LIGHT_STRENGTH_CELL(x, y-1) > LIGHT_STRENGTH_CELL(x, y) + MAX_LIGHT_STEP )
				LIGHT_STRENGTH_CELL(x, y-1) = LIGHT_STRENGTH_CELL(x, y) + MAX_LIGHT_STEP;
			if ( LIGHT_STRENGTH_CELL(x-1, y-1) > LIGHT_STRENGTH_CELL(x, y) + MAX_LIGHT_STEP )
				LIGHT_STRENGTH_CELL(x-1, y-1) = LIGHT_STRENGTH_CELL(x, y) + MAX_LIGHT_STEP;
	}
    }
    
}; // void soften_light_distribution ( void )

/**
 * This function is used to find the light intensity at any given point
 * on the map.
 */
void 
set_up_light_strength_buffer ( void )
{
    uint32_t x ;
    uint32_t y ;
    moderately_finepoint target_pos ;
    int screen_x ;
    int screen_y ;

	for ( y = 0 ; y < LightRadiusConfig.cells_h ; ++y  )
    {
		for ( x = 0 ; x < LightRadiusConfig.cells_w ; ++x )
	{
			screen_x = (int)(x * LightRadiusConfig.scale_factor) - UserCenter_x;
			// Apply a translation to Y coordinate, to simulate a light coming from bot/tux heads, instead
			// of their feet.
			screen_y = (int)(y * LightRadiusConfig.scale_factor) - UserCenter_y + LightRadiusConfig.translate_y;

	    target_pos . x = translate_pixel_to_map_location ( screen_x , screen_y , TRUE ) ;
	    target_pos . y = translate_pixel_to_map_location ( screen_x , screen_y , FALSE ) ;

			LIGHT_STRENGTH_CELL(x,y) = calculate_light_strength ( target_pos );
	}
    }
    
    soften_light_distribution();

}; // void set_up_light_strength_buffer ( void )


/**
 * This function is used to find the light intensity at any given point
 * on the screen.
 */
int 
get_light_strength_cell ( uint32_t x, uint32_t y )
{
	return LIGHT_STRENGTH_CELL(x, y);
}

/**
 * This function is used to find the light intensity at any given point
 * on the screen.
 */
int 
get_light_strength_screen ( int x, int y )
{
	x = x / LightRadiusConfig.scale_factor;
	y = y / LightRadiusConfig.scale_factor;

	if ( ( x >=  0 ) && ( x < LightRadiusConfig.cells_w ) && ( y >= 0 ) && ( y < LightRadiusConfig.cells_h ) )
    {
		return ( LIGHT_STRENGTH_CELL(x, y) );
	}
	else
	{
	//--------------------
	// If a request reaches outside the prepared buffer, we use the
	// nearest point, if we can get it easily, otherwise we'll use
	// blackness by default.

		if ( ( x >= 0 ) && ( x < LightRadiusConfig.cells_w ) )
	{
			if ( y >= LightRadiusConfig.cells_h ) return ( LIGHT_STRENGTH_CELL(x, LightRadiusConfig.cells_h - 1) );
			else if ( y < 0 )                     return ( LIGHT_STRENGTH_CELL(x, 0) );
	}
		else if ( ( y >= 0 ) && ( y < LightRadiusConfig.cells_h ) )
	{
			if ( x >= LightRadiusConfig.cells_w ) return ( LIGHT_STRENGTH_CELL(LightRadiusConfig.cells_w - 1, y) );
			else if ( x < 0 )                     return ( LIGHT_STRENGTH_CELL(0, y) );
	}

	return ( NUMBER_OF_SHADOW_IMAGES );
    }

}; // int get_light_screen_strength ( moderately_finepoint target_pos )


/**
 * This function is used to find the light intensity at any given point
 * on the map.
 */
int 
get_light_strength ( moderately_finepoint target_pos )
{
   int x , y ;

   x = translate_map_point_to_screen_pixel_x ( target_pos . x ,  target_pos . y );
   y = translate_map_point_to_screen_pixel_y ( target_pos . x ,  target_pos . y );

   return get_light_strength_screen ( x, y );
}

/**
 * This function should blit the shadows on the floor, that are used to
 * generate the impression of a 'light radius' around the players 
 * character.
 */
void
blit_classic_SDL_light_radius( void )
{
    static int first_call = TRUE ;
    int i, j ;
char fpath[2048];
    char constructed_file_name[2000];
    int our_height, our_width, our_max_height, our_max_width;
    int light_strength;
    static int pos_x_grid [ (int)(MAX_FLOOR_TILES_VISIBLE_AROUND_TUX * ( 1.0 / LIGHT_RADIUS_CHUNK_SIZE ) * 2) ] [ (int)(MAX_FLOOR_TILES_VISIBLE_AROUND_TUX * ( 1.0 / LIGHT_RADIUS_CHUNK_SIZE ) * 2 ) ] ;
    static int pos_y_grid [ (int)(MAX_FLOOR_TILES_VISIBLE_AROUND_TUX * ( 1.0 / LIGHT_RADIUS_CHUNK_SIZE ) * 2) ] [ (int)(MAX_FLOOR_TILES_VISIBLE_AROUND_TUX * ( 1.0 / LIGHT_RADIUS_CHUNK_SIZE ) * 2 ) ] ;
    static SDL_Rect target_rectangle;
    int chunk_size_x;
    int chunk_size_y;
    SDL_Surface* tmp;
    
    //--------------------
    // If the darkness chunks have not yet been loaded, we load them...
    //
    if ( first_call )
    {
	first_call = FALSE;
	for ( i = 0 ; i < NUMBER_OF_SHADOW_IMAGES ; i ++ )
	{
	    sprintf ( constructed_file_name , "light_radius_chunks/iso_light_radius_darkness_%04d.png" , i + 1 );
	    find_file (constructed_file_name , GRAPHICS_DIR , fpath, 0 );
	    get_iso_image_from_file_and_path ( fpath , & ( light_radius_chunk [ i ] ) , TRUE ) ;
	    tmp = light_radius_chunk [ i ] . surface ;
	    light_radius_chunk [ i ] . surface = SDL_DisplayFormatAlpha ( light_radius_chunk [ i ] . surface ) ; 
	    SDL_FreeSurface ( tmp ) ;
	}
	
	pos_x_grid [ 0 ] [ 0 ] = translate_map_point_to_screen_pixel_x ( Me . pos . x - ( FLOOR_TILES_VISIBLE_AROUND_TUX ) , Me . pos . y - ( FLOOR_TILES_VISIBLE_AROUND_TUX ) ) - 10 ;
	pos_y_grid [ 0 ] [ 0 ] = translate_map_point_to_screen_pixel_y ( Me . pos . x - ( FLOOR_TILES_VISIBLE_AROUND_TUX ) , Me . pos . y - ( FLOOR_TILES_VISIBLE_AROUND_TUX ) ) - 42 ;
	
	chunk_size_x = 26 /2 + 1 ;
	chunk_size_y = 14 /2 ; 
	
	for ( i = 0 ; i < (int)(FLOOR_TILES_VISIBLE_AROUND_TUX * ( 1.0 / LIGHT_RADIUS_CHUNK_SIZE ) * 2) ; i ++ )
	{
	    for ( j = 0 ; j < (int)(FLOOR_TILES_VISIBLE_AROUND_TUX * ( 1.0 / LIGHT_RADIUS_CHUNK_SIZE ) * 2) ; j ++ )
	    {
		pos_x_grid [ i ] [ j ] = pos_x_grid [ 0 ] [ 0 ] + ( i - j ) * chunk_size_x ;
		pos_y_grid [ i ] [ j ] = pos_y_grid [ 0 ] [ 0 ] + ( i + j ) * chunk_size_y ;
	    }
	}
    }
    
    //--------------------
    // Now it's time to apply the light radius
    //
    our_max_width = FLOOR_TILES_VISIBLE_AROUND_TUX * ( 1.0 / LIGHT_RADIUS_CHUNK_SIZE ) * 2 ;
    our_max_height = our_max_width;

    for ( our_height = 0 ; our_height < our_max_height ; our_height ++ )
    {
	for ( our_width = 0 ; our_width < our_max_width ; our_width ++ )
	{
	    if ( our_width % LIGHT_RADIUS_CRUDENESS_FACTOR ) continue;
	    if ( our_height % LIGHT_RADIUS_CRUDENESS_FACTOR ) continue;
	    
	    target_rectangle . x = pos_x_grid [ our_width ] [ our_height ] ;
	    target_rectangle . y = pos_y_grid [ our_width ] [ our_height ] ;
	    light_strength = get_light_strength_screen ( (uint32_t)target_rectangle . x, (uint32_t)target_rectangle . y ) ;
	    
	    our_SDL_blit_surface_wrapper( light_radius_chunk [ light_strength ] . surface , NULL , Screen, &target_rectangle );
	}
    }
}; // void blit_classic_SDL_light_radius( void )

/**
 * FreedroidRPG does some light/shadow computations and then draws some
 * shadows/light on the floor.  There is a certain amount of light around
 * the Tux.  This light is called the 'light radius'.  
 * 
 * This function is supposed to compute all light/shadow values for the
 * the floor and then draw the light radius on the floor.  Note, that 
 * this is something completely different from the prerendered shadows
 * that are to be blitted for obstacles, when they are exposed to 
 * daylight.
 */
void
blit_light_radius ( void )
{

    //--------------------
    // Before making any reference to the light, it's best to 
    // first calculate the values in the light buffer, because
    // those will be used when drawing the light radius.
    //
    set_up_light_strength_buffer ( );

    if ( use_open_gl )
    {
	// blit_open_gl_light_radius ();
	// blit_open_gl_cheap_light_radius ();
	blit_open_gl_stretched_texture_light_radius ();
    }
    else
    {
	blit_classic_SDL_light_radius();
    }
    
}; // void blit_light_radius ( void )

#undef _light_c
