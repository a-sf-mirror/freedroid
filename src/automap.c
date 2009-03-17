
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

/**
 * This file contains everything that has got to do with the automap.
 */


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

static iso_image compass;

/**
 * This function clears out the Automap data.
 */
void ClearAutomapData( void )
{
    memset ( Me . Automap, 0, MAX_LEVELS * 100 * 100);

}; // void ClearAutomapData ( void )

/**
 * Use GL_POINTS primitive in OpenGL mode to render automap points for better performance.
 * In SDL mode, fallback to PutPixel.
 */
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


/**
 * Toggle automap visibility
 */
void toggle_automap( void )
{
    GameConfig.Automap_Visible = !GameConfig.Automap_Visible;
    if ( Me . map_maker_is_present )
	{
	if ( GameConfig.Automap_Visible )
	    append_new_game_message ( _("Automap ON.") );
	else
	    append_new_game_message ( _("Automap OFF.") );
	}
    else
	{
	if ( GameConfig.Automap_Visible )
	    append_new_game_message ( _("Compass ON (no automap yet: map maker not present.)") );
	else
	    append_new_game_message ( _("Compass OFF.") );
	}
}

static void display_automap_compass ()
{
    char fpath[2048];

    //load the compass if necessary
    if (compass.surface == NULL && !compass.texture_has_been_created) {
	find_file("compass.png", GRAPHICS_DIR, fpath, 1);

	get_iso_image_from_file_and_path(fpath, &compass, 0);

	if (use_open_gl) 
	    make_texture_out_of_surface(&compass);
    }


    if (use_open_gl) {
	draw_gl_textured_quad_at_screen_position(&compass, GameConfig.screen_width - compass.original_image_width, 50);
    } else {
	SDL_Rect dr = { .x = GameConfig.screen_width - compass.original_image_width, .y = 50, .w = compass.original_image_width, .h = compass.original_image_height };
	our_SDL_blit_surface_wrapper(compass.surface, NULL, Screen, &dr);
    }

}

/**
 *
 * This function should display the automap.
 * 
 * In this case the function only uses pixel operations with the screen.
 * This is the method of choice when using SDL for graphics output.
 * For the OpenGL case, there is a completely different function...
 *
 */
void display_automap ( void )
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

  // Display the compass
  display_automap_compass();

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
	  if ( Me . Automap [ level ] [ y ] [ x ] & (RIGHT_WALL_BIT | LEFT_WALL_BIT) )
	    {
	      PutPixel_automap_wrapper ( Screen , 
			 1+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - y ) , 
			 1+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y , AUTOMAP_COLOR );
	      PutPixel_automap_wrapper ( Screen , 
			 2+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - y ) , 
			 2+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y , AUTOMAP_COLOR );
	    }

	  if ( Me . Automap [ level ] [ y ] [ x ] & (UP_WALL_BIT | DOWN_WALL_BIT) )
	    {
	      PutPixel_automap_wrapper ( Screen , 
			 1+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - y ) , 
			 2+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y , AUTOMAP_COLOR );
	      PutPixel_automap_wrapper ( Screen , 
			 2+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * ( automap_level -> ylen - y ) , 
			 1+AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y , AUTOMAP_COLOR );
	    }
	}
    }

  // Display enemies
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

  // Display tux
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

