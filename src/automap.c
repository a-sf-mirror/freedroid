
/* 
 *
 *   Copyright (c) 2004 Johannes Prix
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
 */

/* ----------------------------------------------------------------------
 * This file contains everything that has got to do with the automap.
 * ---------------------------------------------------------------------- */

#define _automap_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "SDL_rotozoom.h"

//--------------------
// This controls the zoom factor for the automap.  Since this uses
// a different update policy than the level editor, even strong zoom
// will not be a problem here...
//
// However some other considerations must be taken into account:  OPENGL
// DOES IN GENERAL NOT WORK WELL ANY MORE WITH TEXTURES LARGER THAN 1024
// AND SOMETIMES EVEN NOT WELL WITH ANYTHING > 256 (old vodoo cards!!!)
// EVEN GEFORCE II HAS PROBLEMS WITH 2048 TEXTURE SIZE.
//
// Therefore some sanity-factor is introduced here:  it will scale down
// the internal automap texture but scale it up again when the acutal
// part-transparent blit is being done.  Using 0.25 for the sanity factor
// should be the safest bet.  However quality is poor:  you get lots of
// rectangular blocks where a wall/door/tree would be.
// So we're using larger textures, assuming that most people will have
// decen graphics adapters. Vodoo users can recompile the game for their needs...
//
#define AUTOMAP_ZOOM_OUT_FACT 8.0
int AUTOMAP_TEXTURE_WIDTH=2048;
int AUTOMAP_TEXTURE_HEIGHT=1024;


/* ----------------------------------------------------------------------
 * This function clears out the Automap data.
 * ---------------------------------------------------------------------- */
void 
ClearAutomapData( void )
{
/*    int x , y , level ;

    for ( level = 0 ; level < MAX_LEVELS ; level ++ )
    {
	for ( y = 0 ; y < 200 ; y ++ )
	{
	    for ( x = 0 ; x < 200 ; x ++ )
	    {
		Me . Automap[level][y][x] = 0 ;
	    }
	}
    }*/
memset ( Me . Automap, 0, MAX_LEVELS * 100 * 100);
    
}; // void ClearAutomapData ( void )

/* ----------------------------------------------------------------------
 *
 * When OpenGL is used for graphics output, we use one single texture for 
 * the automap.  This texture will be updated and modified again and
 * again.  But this function just makes sure, that the texture has been
 * created at all, and it also checks against double-creation and such...
 *
 * ---------------------------------------------------------------------- */
void
set_up_texture_for_automap ( void )
{
#ifdef HAVE_LIBGL

    static int texture_is_set_up_already = FALSE ;
    SDL_Surface* pure_surface ;

    //--------------------
    // In the non-open-gl case, this function shouldn't be called ever....
    //
    if ( ! use_open_gl ) return;

    //--------------------
    // Some protection against creating this texture twice...
    //
    if ( texture_is_set_up_already ) return ;
    texture_is_set_up_already = TRUE ;

    //--------------------
    // We create an SDL surface, so that we can make the texture for the
    // automap from it...
    //
    int max_size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);

    if ( AUTOMAP_TEXTURE_WIDTH > max_size )
	{
	AUTOMAP_TEXTURE_HEIGHT *= (float)(max_size / (float)AUTOMAP_TEXTURE_WIDTH);
	AUTOMAP_TEXTURE_WIDTH = max_size;
	}
    if ( AUTOMAP_TEXTURE_HEIGHT > max_size )
	{
	AUTOMAP_TEXTURE_WIDTH *= (float)(max_size / (float)AUTOMAP_TEXTURE_HEIGHT);
	AUTOMAP_TEXTURE_HEIGHT = max_size;
	}

    pure_surface = SDL_CreateRGBSurface( 0 , AUTOMAP_TEXTURE_WIDTH , AUTOMAP_TEXTURE_HEIGHT , 32, 0x0FF000000 , 0x000FF0000  , 0x00000FF00 , 0x000FF );

    //--------------------
    // Having prepared the raw image it's now time to create the real
    // textures.
    //
    glPixelStorei( GL_UNPACK_ALIGNMENT,1 );

    //--------------------
    // We must not call glGenTextures more than once in all of Freedroid,
    // according to the nehe docu and also confirmed instances of textures
    // getting overwritten.  So all the gentexture stuff is now in the
    // initialzize_our_default_open_gl_parameters function and we'll use stuff from there.
    //
    // glGenTextures( 1, & our_image -> texture );
    //
    automap_texture = & ( all_freedroid_textures [ next_texture_index_to_use ] ) ;
    next_texture_index_to_use ++ ;

    if ( next_texture_index_to_use >= MAX_AMOUNT_OF_TEXTURES_WE_WILL_USE )
    {
	GiveStandardErrorMessage ( __FUNCTION__  , 
				   "Ran out of initialized texture positions to use for new textures.",
				   PLEASE_INFORM, IS_FATAL );
    }
    else
    {
	DebugPrintf ( 0 , "\nTexture positions remaining: %d." , MAX_AMOUNT_OF_TEXTURES_WE_WILL_USE - next_texture_index_to_use );
    }
    
    //--------------------
    // Typical Texture Generation Using Data From The Bitmap 
    //
    glBindTexture( GL_TEXTURE_2D, * ( automap_texture ) );
  
    //--------------------
    // Setting texture parameters like in NeHe tutorial...
    //
    glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );

    //--------------------
    // We will use the 'GL_REPLACE' texturing environment or get 
    // unusable (and slow) results.
    //
    // glTexEnvi ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
    // glTexEnvi ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND );
    // glTexEnvi ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glTexEnvi ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    // Generate The Texture 
    glTexImage2D( GL_TEXTURE_2D, 0, 4, pure_surface -> w,
		  pure_surface -> h, 0, GL_BGRA,
		  GL_UNSIGNED_BYTE, pure_surface -> pixels );

    SDL_FreeSurface ( pure_surface );

    DebugPrintf ( 1 , "\n%s(): Texture has been set up..." , __FUNCTION__ );

#endif

}; // void set_up_texture_for_automap ( void )

/* ----------------------------------------------------------------------
 *
 * When OpenGL is used for graphics output, we use one single texture for 
 * the automap.  This texture will be updated and modified again and
 * again.  But this function just makes sure, that the texture has been
 * created at all, and it also checks against double-creation and such...
 *
 * ---------------------------------------------------------------------- */
void
clear_automap_texture_completely ( void )
{
#ifdef HAVE_LIBGL

    static int empty_texture_is_available = FALSE ;
    // static SDL_Surface* pure_surface ;
    static void* empty_automap_surface ;
    
    //--------------------
    // In the non-open-gl case, this function shouldn't be called ever....
    //
    if ( ! use_open_gl ) return;

    //--------------------
    // Some protection against creating this texture twice...
    //
    if ( ! empty_texture_is_available ) 
    {
	empty_texture_is_available = TRUE ;
	// pure_surface = SDL_CreateRGBSurface( 0 , AUTOMAP_TEXTURE_WIDTH , AUTOMAP_TEXTURE_HEIGHT , 32, 0x0FF000000 , 0x000FF0000  , 0x00000FF00 , 0x000FF );
	empty_automap_surface = MyMalloc ( 4 * ( ( AUTOMAP_TEXTURE_WIDTH + 2 )  * ( AUTOMAP_TEXTURE_HEIGHT + 2 ) ) ) ;
	memset ( empty_automap_surface , 0 , 4 * AUTOMAP_TEXTURE_WIDTH * AUTOMAP_TEXTURE_HEIGHT ) ;
    }

    DebugPrintf ( 1 , "\n%s(): starting to clear automap." , __FUNCTION__ );

    glEnable ( GL_TEXTURE_2D );
    glBindTexture ( GL_TEXTURE_2D , *automap_texture );
    glTexSubImage2D ( GL_TEXTURE_2D , 0 , 
		      0 , 
		      0 , 
		      AUTOMAP_TEXTURE_WIDTH ,
		      AUTOMAP_TEXTURE_HEIGHT , 
		      GL_BGRA, 
		      GL_UNSIGNED_BYTE, 
		      empty_automap_surface );
    
    open_gl_check_error_status ( __FUNCTION__ );

    DebugPrintf ( 1 , "\n%s(): Texture for AUTOMAP has been cleared..." , __FUNCTION__ );

#endif

}; // void clear_automap_texture_completely ( void )

/* ----------------------------------------------------------------------
 *
 * This function should display the automap data, that was collected so
 * far, by the tux.
 * 
 * In this case the function only uses pixel operations with the screen.
 * This is the method of choice when using SDL for graphics output.
 * For the OpenGL case, there is a completely different function...
 *
 * ---------------------------------------------------------------------- */
void
show_automap_data_sdl ( void )
{
  int x , y ;
#define AUTOMAP_SQUARE_SIZE 3
#define AUTOMAP_COLOR 0x0FFFF
  int i;
  int TuxColor = SDL_MapRGB( Screen->format, 0 , 0 , 255 ); 
  int FriendColor = SDL_MapRGB( Screen->format, 0 , 255 , 0 ); 
  //int BoogyColor = SDL_MapRGB( Screen->format, 255 , 0 , 0 ); 
  Level automap_level = curShip . AllLevels [ Me . pos . z ] ;
  int level = Me . pos . z ;

  //--------------------
  // Of course we only display the automap on demand of the user...
  //
  if ( GameConfig.Automap_Visible == FALSE ) return;

  //--------------------
  // Also if there is no map-maker present in inventory, then we need not
  // do a thing here...
  //
  if ( ! Me . map_maker_is_present ) return;

  //--------------------
  // At first, we only blit the known data about the pure wall-type
  // obstacles on this level.
  //
  // Currently we handle this via putpixel, but later there should be some
  // small images instead of the pixels and some larger surface made out of
  // the smaller pixels..., and then there should be OpenGL-textures to 
  // show the larger surface, updated again and again.  Well, this will
  // have to wait for the 0.9.10 release to be implemented.
  //
  for ( y = 0 ; y < automap_level->ylen ; y ++ )
    {
      for ( x = 0 ; x < automap_level->xlen ; x ++ )
	{
	  if ( Me . Automap [ level ] [ y ] [ x ] & RIGHT_WALL_BIT )
	    {
	      PutPixel ( Screen , 
			 2+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - y ) , 
			 0+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y , AUTOMAP_COLOR );
	      PutPixel ( Screen , 
			 2+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - y ) , 
			 1+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y , AUTOMAP_COLOR );
	      PutPixel ( Screen , 
			 2+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - y ) , 
			 1+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y , AUTOMAP_COLOR );
	      // putpixel ( Screen , 3*x+2 , 3*y+0 , AUTOMAP_COLOR );
	      // putpixel ( Screen , 3*x+2 , 3*y+1 , AUTOMAP_COLOR );
	      // putpixel ( Screen , 3*x+2 , 3*y+2 , AUTOMAP_COLOR );
	    }
	  if ( Me . Automap [ level ] [ y ] [ x ] & LEFT_WALL_BIT )
	    {
	      PutPixel ( Screen , 
			 0+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - y ) , 
			 0+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y , AUTOMAP_COLOR );
	      PutPixel ( Screen , 
			 0+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - y ) , 
			 1+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y , AUTOMAP_COLOR );
	      PutPixel ( Screen , 
			 0+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - y ) , 
			 1+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y , AUTOMAP_COLOR );
	      // putpixel ( Screen , 3*x , 3*y+0 , AUTOMAP_COLOR );
	      // putpixel ( Screen , 3*x , 3*y+1 , AUTOMAP_COLOR );
	      // putpixel ( Screen , 3*x , 3*y+2 , AUTOMAP_COLOR );
	    }
	  if ( Me . Automap [ level ] [ y ] [ x ] & UP_WALL_BIT )
	    {
	      PutPixel ( Screen , 3*x+0 , 3*y , AUTOMAP_COLOR );
	      PutPixel ( Screen , 3*x+1 , 3*y , AUTOMAP_COLOR );
	      PutPixel ( Screen , 3*x+2 , 3*y , AUTOMAP_COLOR );
	    }
	  if ( Me . Automap [ level ] [ y ] [ x ] & DOWN_WALL_BIT )
	    {
	      PutPixel ( Screen , 3*x+0 , 3*y+2 , AUTOMAP_COLOR );
	      PutPixel ( Screen , 3*x+1 , 3*y+2 , AUTOMAP_COLOR );
	      PutPixel ( Screen , 3*x+2 , 3*y+2 , AUTOMAP_COLOR );
	    }
	}
    }

  //--------------------
  // Now that the pure map data has been drawn, we add red dots for 
  // the ememys around.
  //
  // for ( i = 0 ; i < Number_Of_Droids_On_Ship ; i ++ )
  for ( i = 0 ; i < MAX_ENEMYS_ON_SHIP ; i ++ )
  {
      if ( AllEnemys [ i ] . Status  == INFOUT ) continue;
      if ( AllEnemys [ i ] . type == (-1) ) continue;
      if ( AllEnemys [ i ] . pos . z != automap_level -> levelnum ) continue;

      for ( x = 0 ; x < AUTOMAP_SQUARE_SIZE ; x ++ )
	{
	  for ( y = 0 ; y < AUTOMAP_SQUARE_SIZE ; y ++ )
	    {
	      if ( AllEnemys [ i ] . is_friendly )
		{
		  PutPixel ( Screen , AUTOMAP_SQUARE_SIZE * AllEnemys[i].pos.x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - AllEnemys[i].pos.y ) + x , 
			     AUTOMAP_SQUARE_SIZE * AllEnemys[i].pos.x + AUTOMAP_SQUARE_SIZE * AllEnemys[i].pos.y + y , FriendColor );
		}
	      else
		{
		    // PutPixel ( Screen , AUTOMAP_SQUARE_SIZE * AllEnemys[i].pos.x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - AllEnemys[i].pos.y ) + x , 
		    // AUTOMAP_SQUARE_SIZE * AllEnemys[i].pos.x + AUTOMAP_SQUARE_SIZE * AllEnemys[i].pos.y + y , BoogyColor );
		}
	    }
	}
    }

  //--------------------
  // Now that the automap is drawn so far, we add a blue dot for the
  // tux himself and also for colleagues, that are on this level and alive.
  //
  for ( x = 0 ; x < AUTOMAP_SQUARE_SIZE ; x ++ )
    {
      for ( y = 0 ; y < AUTOMAP_SQUARE_SIZE ; y ++ )
	{
	  PutPixel ( Screen , AUTOMAP_SQUARE_SIZE * Me . pos . x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - Me . pos . y ) + x , 
		     AUTOMAP_SQUARE_SIZE * Me . pos . x + AUTOMAP_SQUARE_SIZE * Me . pos . y + y , TuxColor );
	  if (Me . status != INFOUT)
	      PutPixel ( Screen , AUTOMAP_SQUARE_SIZE * Me . pos . x + x , AUTOMAP_SQUARE_SIZE * Me . pos . y + y , FriendColor );
	}
    }

}; // void show_automap_data_sdl ( void )

/* ----------------------------------------------------------------------
 * This function updated the automap texture, such that all info from the
 * current square is on the automap.
 * ---------------------------------------------------------------------- */
void
automap_update_texture_for_square ( int x , int y ) 
{

#ifdef HAVE_LIBGL

    int i;
    Level automap_level = curShip . AllLevels [ Me . pos . z ] ;
    obstacle* our_obstacle ;
    static int first_call = TRUE;

    //---------------------
    // If there is no OpenGL graphics output, then there is also no need
    // to update/edit any OpenGL textures, therefore we quit to prevent 
    // a segmentation fault.
    //
    if ( ! use_open_gl ) return;

    if ( first_call )
    {
	first_call = FALSE ;
	DebugPrintf ( 1 , "\n%s(): iso_floor_tile_width: %f. iso_floor_tile_height: %f." , 
		      __FUNCTION__ , iso_floor_tile_width , iso_floor_tile_height );
    }

    // DebugPrintf ( -4 , "x: %d. y: %d. ** " , x , y ) ;

    for ( i = 0 ; i < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE ; i ++ )
    {
	if ( automap_level -> map [ y ] [ x ] . obstacles_glued_to_here [ i ] == (-1) ) break;
	
	our_obstacle = & ( automap_level -> obstacle_list [ automap_level -> map [ y ] [ x ] . obstacles_glued_to_here [ i ] ] ) ;
	if ( obstacle_map [ our_obstacle -> type ] . block_area_type == COLLISION_TYPE_RECTANGLE )
	{
	    //--------------------
	    // Now it's time to edit the automap texture.
	    //
	    // DebugPrintf ( -4 , "\nType is: %d." , our_obstacle -> type );
	    // DebugPrintf ( -4 , "\nwidth/height:   %d / %d." , 
	    // obstacle_map [ our_obstacle -> type ] . automap_version -> w ,
	    // obstacle_map [ our_obstacle -> type ] . automap_version -> h );
	    // memset ( obstacle_map [ our_obstacle -> type ] . automap_version -> pixels ,
	    // 0 , obstacle_map [ our_obstacle -> type ] . automap_version -> w *
	    // obstacle_map [ our_obstacle -> type ] . automap_version -> h );
	    // 
	    glEnable ( GL_TEXTURE_2D );
	    glBindTexture ( GL_TEXTURE_2D , *automap_texture );
	    glTexSubImage2D ( GL_TEXTURE_2D , 0 , 
			      ( AUTOMAP_TEXTURE_WIDTH / 2 ) + ( our_obstacle -> pos . x - our_obstacle -> pos . y ) * ( iso_floor_tile_width / ( 2.0 * AUTOMAP_ZOOM_OUT_FACT ) ) ,
			      AUTOMAP_TEXTURE_HEIGHT - ( 50 + ( our_obstacle -> pos . x + our_obstacle -> pos.  y ) * ( iso_floor_tile_height / ( 2.0 * AUTOMAP_ZOOM_OUT_FACT ) ) ) ,
			      obstacle_map [ our_obstacle -> type ] . automap_version -> w ,
			      obstacle_map [ our_obstacle -> type ] . automap_version -> h ,
			      GL_BGRA, 
			      GL_UNSIGNED_BYTE, 
			      obstacle_map [ our_obstacle -> type ] . automap_version -> pixels );
    
	    open_gl_check_error_status ( __FUNCTION__ );
	}
    }

#endif

}; // void automap_update_texture_for_square ( int x , int y ) 

/* ----------------------------------------------------------------------
 * This function does the effect of a 'magic mapping scroll' in Nethack,
 * i.e. all the map becomes visible on the automap.
 * ---------------------------------------------------------------------- */
void
full_update_of_automap_texture ( void )
{
    int x , y ;
#define AUTOMAP_SQUARE_SIZE 3
#define AUTOMAP_COLOR 0x0FFFF
    Level automap_level = curShip . AllLevels [ Me . pos . z ] ;

    //--------------------
    // At first, we only blit the known data about the pure wall-type
    // obstacles on this level.
    //
    // Currently we handle this via putpixel, but later there should be some
    // small images instead of the pixels and some larger surface made out of
    // the smaller pixels..., and then there should be OpenGL-textures to 
    // show the larger surface, updated again and again.  Well, this will
    // have to wait for the 0.9.10 release to be implemented.
    //
    for ( y = 0 ; y < automap_level->ylen ; y ++ )
    {
	for ( x = 0 ; x < automap_level->xlen ; x ++ )
	{
	    automap_update_texture_for_square ( x , y ) ;
	}
    }
    
}; // void full_update_of_automap_texture ( void )

/* ----------------------------------------------------------------------
 * When an old level is re-visited, the automap texture must be cleared
 * because it has still all the info from another level.  But after that
 * the map is fresh.  Still there should be some info, since the Tux has
 * been here before.  So we restore the info, using the data from the
 * classical SDL automap concerning which squares have been seen before.
 * ---------------------------------------------------------------------- */
void
insert_old_map_info_into_texture ( void )
{
    int x , y ;
#define AUTOMAP_SQUARE_SIZE 3
#define AUTOMAP_COLOR 0x0FFFF
    Level automap_level = curShip . AllLevels [ Me . pos . z ] ;

    //---------------------
    // If there is no OpenGL graphics output, then there is also no need
    // to update/edit any OpenGL textures, therefore we quit to prevent 
    // a segmentation fault.
    //
    if ( ! use_open_gl ) return;

    set_up_texture_for_automap ( );

    //--------------------
    // At first, we only blit the known data about the pure wall-type
    // obstacles on this level.
    //
    // Currently we handle this via putpixel, but later there should be some
    // small images instead of the pixels and some larger surface made out of
    // the smaller pixels..., and then there should be OpenGL-textures to 
    // show the larger surface, updated again and again.  Well, this will
    // have to wait for the 0.9.10 release to be implemented.
    //
    for ( y = 0 ; y < automap_level->ylen ; y ++ )
    {
	for ( x = 0 ; x < automap_level->xlen ; x ++ )
	{
	    if ( Me . Automap [ Me . pos . z ] [ y ] [ x ] & SQUARE_SEEN_AT_ALL_BIT ) 
		automap_update_texture_for_square ( x , y ) ;
	}
    }

}; // void insert_old_map_info_into_texture ( void )

/* ----------------------------------------------------------------------
 * This function does the effect of a 'magic mapping scroll' in Nethack,
 * i.e. all the map becomes visible on the automap.
 * ---------------------------------------------------------------------- */
void
local_update_of_automap_texture ( void )
{
    int x , y , start_x , start_y , end_x , end_y ;
#define AUTOMAP_SQUARE_SIZE 3
#define AUTOMAP_COLOR 0x0FFFF
    Level automap_level = curShip . AllLevels [ Me . pos . z ] ;
    static float automap_update_counter = 0 ;

    //--------------------
    // This function is a bit costly, so we don't do it every frame,
    // but rather only ever second second :)
    //
    automap_update_counter += Frame_Time();
    if ( automap_update_counter < 1.0 ) return ;
    automap_update_counter = 0 ;

    //--------------------
    // We prepare the area around the Tux, where the map should be
    // updated...
    //
    start_x = Me . pos . x - 7 ; 
    end_x = Me . pos . x + 7 ; 
    start_y = Me . pos . y - 7 ; 
    end_y = Me . pos . y + 7 ; 
    
    if ( start_x < 0 ) start_x = 0 ; 
    if ( end_x >= automap_level->xlen ) end_x = automap_level->xlen-1 ;
    if ( start_y < 0 ) start_y = 0 ; 
    if ( end_y >= automap_level->ylen ) end_y = automap_level->ylen-1 ;
    
    //--------------------
    // Now we do the actual checking for visible wall components.
    //
    for ( y = start_y ; y < end_y ; y ++ )
    {
	for ( x = start_x ; x < end_x ; x ++ )
	{
	    automap_update_texture_for_square ( x , y ) ;
	}
    }
    
}; // void local_update_of_automap_texture ( void )


/* ----------------------------------------------------------------------
 *
 * This function should display the automap data, that was collected so
 * far, by the tux.
 * 
 * In this case the function only uses pixel operations with the screen.
 * This is the method of choice when using SDL for graphics output.
 * For the OpenGL case, there is a completely different function...
 *
 * ---------------------------------------------------------------------- */
void
show_automap_data_ogl ( float scale_factor )
{

#ifdef HAVE_LIBGL

    iso_image local_iso_image;
    static iso_image tux_on_the_map_iso_image = UNLOADED_ISO_IMAGE ;
    char fpath[4096];

    //--------------------
    // Also if there is no map-maker present in inventory, then we need not
    // do a thing here...
    //
    if ( ! Me . map_maker_is_present ) return;
    
    //--------------------
    // Updating the automap is a bit costly.  It should only be done now
    // and then, i.e. say once a second or the like... and of course only
    // the immediate surroundings of the Tux and not the full map should
    // be updated, unless some 'magic mapping scroll' has been used...
    //
    // full_update_of_automap_texture ();
    //
    local_update_of_automap_texture ();

    //--------------------
    // Of course we only display the automap on demand of the user...
    //
    if ( GameConfig . Automap_Visible == FALSE ) return;
    
    //--------------------
    // Now we blit the current automap texture to the screen.  We use standard
    // texture blitting code for this, so we need to embed the automap texture
    // in a surrounting 'iso_image', but that shouldn't be costly or anything...
    //
    local_iso_image . texture = automap_texture ;
    local_iso_image . texture_width = AUTOMAP_TEXTURE_WIDTH ;
    local_iso_image . texture_height = AUTOMAP_TEXTURE_HEIGHT ;
    local_iso_image . original_image_width = AUTOMAP_TEXTURE_WIDTH ;
    local_iso_image . original_image_height = AUTOMAP_TEXTURE_HEIGHT ;

    blit_semitransparent_open_gl_texture_to_screen_position ( 
	&local_iso_image , 
	- ( AUTOMAP_TEXTURE_WIDTH * scale_factor / 2 ) 
	+ GameConfig . screen_width / 2 
	- GameConfig . automap_manual_shift_x 
	- ( Me . pos . x - Me . pos . y ) * 
	  ( iso_floor_tile_width * scale_factor / ( 2.0 * AUTOMAP_ZOOM_OUT_FACT ) ) , 

	// + ( AUTOMAP_TEXTURE_HEIGHT / 2 ) 
	+ GameConfig . screen_height / 2 
	- GameConfig . automap_manual_shift_y 
	- ( Me . pos . x + Me . pos . y ) * 
          ( iso_floor_tile_height * scale_factor / ( 2.0 * AUTOMAP_ZOOM_OUT_FACT ) ) , 
	scale_factor );


    //--------------------
    // Now that the map has been blitted, it's time to add some icon for the
    // current location of the Tux on that map too...
    // 
    //--------------------
    // First we make sure that the icon is available...
    //
    if ( ! tux_on_the_map_iso_image . texture_has_been_created )
    {
	DebugPrintf ( 1 , "\nLoading icon for Tux on the automap." );
	find_file ( "tux_icon_on_automap.png" , GRAPHICS_DIR , fpath, 0 );
	get_iso_image_from_file_and_path ( fpath , & tux_on_the_map_iso_image , TRUE ) ;
	make_texture_out_of_surface ( & ( tux_on_the_map_iso_image ) ) ;
    }
    //--------------------
    // Now we can blit the icon on the automap too
    //
    blit_semitransparent_open_gl_texture_to_screen_position ( 
	&tux_on_the_map_iso_image , 
	// - ( tux_on_the_map_iso_image . original_image_width / 2 ) 
	+ ( GameConfig . screen_width / 2 ) 
	- GameConfig . automap_manual_shift_x ,

	- ( tux_on_the_map_iso_image . original_image_height / 4 )
	+ ( GameConfig . screen_height / 2 ) 
	+ 50 * scale_factor 
	- GameConfig . automap_manual_shift_y , 1.0 );

#endif

}; // void show_automap_data_ogl ( void )

#undef _automap_c
