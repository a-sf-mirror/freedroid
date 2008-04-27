
/* 
 *   Copyright (c) 2004-2007 Arthur Huillet
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


#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "SDL_rotozoom.h"

int AUTOMAP_TEXTURE_WIDTH=2048;
int AUTOMAP_TEXTURE_HEIGHT=1024;

#define AUTOMAP_SQUARE_SIZE 3
#define AUTOMAP_COLOR 0x0FFFF

/* ----------------------------------------------------------------------
 * This function clears out the Automap data.
 * ---------------------------------------------------------------------- */
void 
ClearAutomapData( void )
{
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

    automap_texture = & ( all_freedroid_textures [ next_texture_index_to_use ] ) ;
    next_texture_index_to_use ++ ;

    if ( next_texture_index_to_use >= MAX_AMOUNT_OF_TEXTURES_WE_WILL_USE )
    {
	ErrorMessage ( __FUNCTION__  , 
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
    // Generate The Texture 
    glTexImage2D( GL_TEXTURE_2D, 0, 4, pure_surface -> w,
		  pure_surface -> h, 0, GL_BGRA,
		  GL_UNSIGNED_BYTE, pure_surface -> pixels );

    SDL_FreeSurface ( pure_surface );

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
	empty_automap_surface = MyMalloc ( 4 * ( ( AUTOMAP_TEXTURE_WIDTH + 2 )  * ( AUTOMAP_TEXTURE_HEIGHT + 2 ) ) ) ;
	memset ( empty_automap_surface , 0 , 4 * AUTOMAP_TEXTURE_WIDTH * AUTOMAP_TEXTURE_HEIGHT ) ;
    }

    DebugPrintf ( 1 , "\n%s(): starting to clear automap." , __FUNCTION__ );

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

static inline void PutPixel_automap_wrapper(SDL_Surface * abc, int x, int y, Uint32 pixel)
{
if ( ! use_open_gl )
    PutPixel ( abc, x, y, pixel);
#ifdef HAVE_LIBGL
else
    {
    glColor3ub(((pixel >> 16) & 0xff), (pixel >> 8) & 0xff, (pixel) & 0xff);
    glVertex2i(x, y);
    }
#endif
}

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
  int TuxColor = SDL_MapRGB( Screen->format, 0 , 0 , 255 ); 
  int FriendColor = SDL_MapRGB( Screen->format, 0 , 255 , 0 ); 
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

#ifdef HAVE_LIBGL
  if ( use_open_gl )
      {
      glDisable (GL_TEXTURE_2D);
      glBegin ( GL_POINTS );
      }
#endif

  //--------------------
  // At first, we only blit the known data about the pure wall-type
  // obstacles on this level.
  for ( y = 0 ; y < automap_level->ylen ; y ++ )
    {
      for ( x = 0 ; x < automap_level->xlen ; x ++ )
	{
	  if ( Me . Automap [ level ] [ y ] [ x ] & RIGHT_WALL_BIT )
	    {
	      PutPixel_automap_wrapper ( Screen , 
			 2+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - y ) , 
			 0+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y , AUTOMAP_COLOR );
	      PutPixel_automap_wrapper ( Screen , 
			 2+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - y ) , 
			 1+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y , AUTOMAP_COLOR );
	      PutPixel_automap_wrapper ( Screen , 
			 2+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - y ) , 
			 1+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y , AUTOMAP_COLOR );
	    }
	  if ( Me . Automap [ level ] [ y ] [ x ] & LEFT_WALL_BIT )
	    {
	      PutPixel_automap_wrapper ( Screen , 
			 0+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - y ) , 
			 0+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y , AUTOMAP_COLOR );
	      PutPixel_automap_wrapper ( Screen , 
			 0+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - y ) , 
			 1+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y , AUTOMAP_COLOR );
	      PutPixel_automap_wrapper ( Screen , 
			 0+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - y ) , 
			 1+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y , AUTOMAP_COLOR );
	    }
	  if ( Me . Automap [ level ] [ y ] [ x ] & UP_WALL_BIT )
	    {
	      PutPixel_automap_wrapper ( Screen , 3*x+0 , 3*y , AUTOMAP_COLOR );
	      PutPixel_automap_wrapper ( Screen , 3*x+1 , 3*y , AUTOMAP_COLOR );
	      PutPixel_automap_wrapper ( Screen , 3*x+2 , 3*y , AUTOMAP_COLOR );
	    }
	  if ( Me . Automap [ level ] [ y ] [ x ] & DOWN_WALL_BIT )
	    {
	      PutPixel_automap_wrapper ( Screen , 3*x+0 , 3*y+2 , AUTOMAP_COLOR );
	      PutPixel_automap_wrapper ( Screen , 3*x+1 , 3*y+2 , AUTOMAP_COLOR );
	      PutPixel_automap_wrapper ( Screen , 3*x+2 , 3*y+2 , AUTOMAP_COLOR );
	    }
	}
    }

  enemy *erot;
  BROWSE_LEVEL_BOTS(erot, automap_level->levelnum)
      {
      if ( erot->type == (-1))
	  continue;

      for ( x = 0 ; x < AUTOMAP_SQUARE_SIZE ; x ++ )
	  {       
	  for ( y = 0 ; y < AUTOMAP_SQUARE_SIZE ; y ++ )
	      {   
	      if ( erot->is_friendly )
		  {
		  PutPixel_automap_wrapper ( Screen , AUTOMAP_SQUARE_SIZE * erot->pos.x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - erot->pos.y ) + x ,
			  AUTOMAP_SQUARE_SIZE * erot->pos.x + AUTOMAP_SQUARE_SIZE * erot->pos.y + y , FriendColor );
		  }
	      }
	  }
      }

  for ( x = 0 ; x < AUTOMAP_SQUARE_SIZE ; x ++ )
    {
      for ( y = 0 ; y < AUTOMAP_SQUARE_SIZE ; y ++ )
	{
	  PutPixel_automap_wrapper ( Screen , AUTOMAP_SQUARE_SIZE * Me . pos . x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - Me . pos . y ) + x , 
		     AUTOMAP_SQUARE_SIZE * Me . pos . x + AUTOMAP_SQUARE_SIZE * Me . pos . y + y , TuxColor );
	}
    }

#ifdef HAVE_LIBGL
  if ( use_open_gl )
      {
      glEnd();
      glEnable (GL_TEXTURE_2D);
      }
#endif

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

    //---------------------
    // If there is no OpenGL graphics output, then there is also no need
    // to update/edit any OpenGL textures, therefore we quit to prevent 
    // a segmentation fault.
    //
    if ( ! use_open_gl ) return;

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
 * ---------------------------------------------------------------------- */
void
insert_old_map_info_into_texture ( void )
{
    int x , y ;
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

