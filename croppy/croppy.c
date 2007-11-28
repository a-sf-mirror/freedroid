/* 
 *
 *  Copyright (c) 2003 Johannes Prix
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

#include "../src/system.h"
#include "../src/defs.h"
#include "../src/getopt.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480 

void Terminate (int ExitCode);

char* output_filename = NULL ;
char* input_filename = NULL ;
char* background_filename = "croppy_background_fill.png" ;
SDL_Surface *Screen;   // the graphics display 
SDL_Surface *input_surface;   // the graphics display 
SDL_Surface *output_surface;   // the graphics display 
SDL_Surface *background_surface;   // the graphics display 
int cut_left;
int cut_right;
int cut_up;
int cut_down;
int no_graphics_output = TRUE ;

char copyright[] = "\nCopyright (C) 2002 Johannes Prix, Reinhard Prix\n\
Freedroid comes with NO WARRANTY to the extent permitted by law.\n\
You may redistribute copies of Freedroid\n\
under the terms of the GNU General Public License.\n\
For more information about these matters, see the file named COPYING.\n";


char usage_string[] =
  "Usage: croppy    [-v|--version] \n\
                    [-i|--input_file] \n\
                    [-n|--nographicsoutput] (default!)\n\
                    [-g|--graphicsoutput] \n\
                    [-d|--debug=LEVEL]\n\
                    [-x|--offset_x=OFFSET_X]\n\
                    [-y|--offset_y=OFFSET_Y]\n\
\n\
EXAMPLE:  croppy -i my_test_file.png\n\
\n\
Please report bugs by sending e-mail to:\n\n\
freedroid-discussion@lists.sourceforge.net\n\n\
Thanks a lot in advance, the Freedroid dev team.\n\n";

int debug_level = 0 ;
int vid_bpp;

int offset_x_override = -30000; // something completely unlikely
int offset_y_override = -30000; // something completely unlikely

//--------------------
// We add this dummy, so that dialog editor and item editor
// will properly compile...
//
void
print_trace ( int signum )
{
};

/* ----------------------------------------------------------------------
 * This function gives the alpha component of a pixel, using a value of
 * 255 for the most opaque pixel and 0 for the least opaque pixel.
 * ---------------------------------------------------------------------- */
Uint8
GetAlphaComponent ( SDL_Surface* surface , int x , int y )
{
  SDL_PixelFormat *fmt;
  Uint32 temp, pixel;
  Uint8 alpha;
  int bpp = surface->format->BytesPerPixel;

  //--------------------
  // First we extract the pixel itself and the
  // format information we need.
  //
  fmt = surface -> format ;
  SDL_LockSurface ( surface ) ;
  //--------------------
  // Now for the longest time we had this command here (which can actually segfault!!)
  //
  // pixel = * ( ( ( Uint32* ) surface -> pixels ) + x + y * surface->w )  ;
  // 
  pixel = * ( ( Uint32* ) ( ( (Uint8*) ( surface -> pixels ) ) + ( x + y * surface->w ) * bpp ) ) ;
  SDL_UnlockSurface ( surface ) ;

  //--------------------
  // Now we can extract the alpha component
  //
  temp = pixel&fmt->Amask;  /* Isolate alpha component */
  temp = temp>>fmt->Ashift; /* Shift it down to 8-bit */
  temp = temp<<fmt->Aloss;  /* Expand to a full 8-bit number */
  alpha = ( Uint8 ) temp ;

  return ( alpha ) ;

}; // int GetAlphaComponent ( SDL_Surface* SourceSurface , int x , int y )

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
MyWait ( float wait_time ) 
{
  long start_ticks = SDL_GetTicks();

  while ( ( SDL_GetTicks() - start_ticks ) < 1000.0 * wait_time );

}; // void MyWait ( float wait_time )

/* ----------------------------------------------------------------------
 * This function works a malloc, except that it also checks for
 * success and terminates in case of "out of memory", so we dont
 * need to do this always in the code.
 * ---------------------------------------------------------------------- */
void *
MyMalloc (long Mamount)
{
  void *Mptr = NULL;

  if ((Mptr = malloc ((size_t) Mamount)) == NULL)
    {
      printf (" MyMalloc(%ld) did not succeed!\n", Mamount);
      Terminate(ERR);
    }

  return Mptr;
}; // void* MyMalloc ( long Mamount )

/* ----------------------------------------------------------------------
 * This function is used for debugging purposes.  It writes the
 * given string either into a file, on the screen, or simply does
 * nothing according to currently set debug level.
 * ---------------------------------------------------------------------- */
void
DebugPrintf (int db_level, char *fmt, ...)
{
  va_list args;
  char *tmp;
  va_start (args, fmt);

  if (db_level <= debug_level)
    {
      tmp = (char *) MyMalloc (1000000 + 1);
      vsprintf (tmp, fmt, args);
      fprintf ( stdout , tmp );
      fflush ( stdout );
      free (tmp);
    }

  va_end (args);
}; // void DebugPrintf (int db_level, char *fmt, ...)

/* ----------------------------------------------------------------------
 * This function is used for terminating freedroid.  It will close
 * the SDL submodules and exit.
 * ---------------------------------------------------------------------- */
void
Terminate (int ExitCode)
{
  DebugPrintf (2, "\nvoid Terminate(int ExitStatus) was called....");
  printf("\n----------------------------------------------------------------------");
  printf("\nTermination of Croppy initiated...");

  printf("Thank you for using the FreedroidRPG Croppy Tool.\n\n");
  SDL_Quit();
  exit (ExitCode);
  return;
}; // void Terminate ( int ExitCode )


/* -----------------------------------------------------------------
 *  parse command line arguments and set global switches 
 *  exit on error, so we don't need to return success status
 * -----------------------------------------------------------------*/
void
ParseCommandLine (int argc, char *const argv[])
{
  int c;

  static struct option long_options[] = 
    {
      { "version",          0, 0,  'v' },
      { "help",             0, 0,  'h' },
      { "nographicsoutput", 0, 0,  'n' },
      { "graphicsoutput",   0, 0,  'g' },
      { "offset_x",         2, 0,  'x' },
      { "offset_y",         2, 0,  'y' },
      { "input_file",       required_argument , 0,  'i' },
      { "debug",            2, 0,  'd' },
      {  0,                 0, 0,   0  }
  };

  while (1)
    {
      c = getopt_long (argc, argv, "gnvi:h?d::", long_options, NULL);
      if (c == -1)
	break;

      switch (c)
	{
	  // version statement -v or --version
	  // following gnu-coding standards for command line interfaces 
	case 'v':
	  DebugPrintf ( 0 , "\nFreedroid Croppy Tool, Version 1.1.\n" );
	  DebugPrintf ( 0 , copyright );
	  exit (0);
	  break;

	case 'h':
	case '?':
	  printf (usage_string);
	  exit (0);
	  break;

	case 'g':
	  no_graphics_output = FALSE ;
	  break;

	case 'i':
	  if ( optarg )
	    {
	      input_filename = optarg;
	      DebugPrintf ( 1 , "\nInput file name set to : %s " , input_filename );
	    }
	  else
	    {
	      printf ("\nERROR! -i specified, but no input file given... Exiting.\n\n" );
	      exit ( 0 );
	    }
	  break;

	case 'x':
	  if ( optarg )
	    {
	      sscanf ( optarg , "%d" , &offset_x_override );
	      DebugPrintf ( 0 , "\nOffset in x overridden: now set to %d. " , offset_x_override );
	    }
	  else
	    {
	      printf ("\nERROR! -x specified, but no offset x argument given... Exiting.\n\n" );
	      exit ( 0 );
	    }
	  break;

	case 'y':
	  if ( optarg )
	    {
	      sscanf ( optarg , "%d" , &offset_y_override );
	      DebugPrintf ( 0 , "\nOffset in y overridden: now set to %d. " , offset_y_override );
	    }
	  else
	    {
	      printf ("\nERROR! -y specified, but no offset y argument given... Exiting.\n\n" );
	      exit ( 0 );
	    }
	  break;

	case 'd':
	  // if (!optarg) 
	    // debug_level = 1;
	  // else
	  // debug_level = atoi (optarg);
	  break;

	default:
	  printf ("\nOption %c not implemented yet! Ignored.", c);
	  break;
	} // switch(c) 
    }  // while(1) 

  if ( input_filename == NULL )
    {
      DebugPrintf ( 0 , "\nERROR:  No input file specified... Terminating... " );
      Terminate ( ERR );
    }

}; // ParseCommandLine 

/* -----------------------------------------------------------------
 * This funciton initialises the video display and opens up a 
 * window for graphics display.
 * -----------------------------------------------------------------*/
void
InitVideo (void)
{
  const SDL_VideoInfo *vid_info;
  SDL_Rect **vid_modes;
  char vid_driver[81];
  Uint32 flags;  // flags for SDL video mode 
  // char *fpath;

  /* Initialize the SDL library */
  // if ( SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) == -1 ) 

  if ( SDL_Init ( SDL_INIT_VIDEO ) == -1 ) 
    {
      fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
      Terminate(ERR);
    } else
      DebugPrintf(1, "\nSDL Video initialisation successful.\n");

  // Now SDL_TIMER is initialized here:

  if ( SDL_InitSubSystem ( SDL_INIT_TIMER ) == -1 ) 
    {
      fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
      Terminate(ERR);
    } else
      DebugPrintf(1, "\nSDL Timer initialisation successful.\n");

  /* clean up on exit */
  atexit (SDL_Quit);

  vid_info = SDL_GetVideoInfo (); /* just curious */
  SDL_VideoDriverName (vid_driver, 80);
  
  flags = SDL_SWSURFACE | SDL_HWPALETTE ;
  // if (fullscreen_on) flags |= SDL_FULLSCREEN;
  flags &= !SDL_FULLSCREEN;

  vid_modes = SDL_ListModes (NULL, SDL_SWSURFACE);

  /* 
   * currently only the simple 320x200 mode is supported for 
   * simplicity, as all our graphics are in this format
   * once this is up and running, we'll provide others modes
   * as well.
   */
  vid_bpp = 16; /* start with the simplest */

  #define SCALE_FACTOR 2

  if ( !no_graphics_output )
    {
      if( !(Screen = SDL_SetVideoMode ( SCREEN_WIDTH, SCREEN_HEIGHT , 0 , flags)) )
	{
	  fprintf(stderr, "Couldn't set (2*) 320x240*SCALE_FACTOR video mode: %s\n",
		  SDL_GetError()); 
	  exit(-1);
	}
      SDL_Flip ( Screen ) ;
    }

  vid_info = SDL_GetVideoInfo (); /* info about current video mode */

  SDL_SetGamma( 1 , 1 , 1 );

}; // InitVideo () 

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
create_output_surface ( void )
{
  SDL_Rect target_rect;
  SDL_Rect source_rect;

  output_surface = SDL_CreateRGBSurface( 
					 input_surface -> flags, 
					 input_surface -> w - cut_left - cut_right , 
					 input_surface -> h - cut_up - cut_down ,
					 input_surface -> format -> BitsPerPixel , 
					 input_surface -> format -> Rmask, 
					 input_surface -> format -> Gmask, 
					 input_surface -> format -> Bmask, 
					 input_surface -> format -> Amask);


  DebugPrintf ( 1 , "\nNew surface was created with dimensions:  %d:%d. " ,
		output_surface -> w , output_surface-> h );

  //--------------------
  // Now we can copy the information from the input surface to the output surface...
  //
  source_rect . x = cut_left; 
  source_rect . y = cut_up ;
  source_rect . w = input_surface->w - cut_left - cut_right ; 
  source_rect . h = input_surface->h - cut_up - cut_down ;

  target_rect . x = 0;
  target_rect . y = 0;
  target_rect . w = output_surface->w;
  target_rect . h = output_surface->h;
  SDL_SetClipRect ( output_surface , NULL ) ;

  SDL_SetAlpha( input_surface, 0 , SDL_ALPHA_OPAQUE );
  SDL_BlitSurface ( input_surface , &source_rect , output_surface , &target_rect );

}; // void create_output_surface ( void )

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
examine_input_surface ( void )
{
  int column_not_empty = FALSE;
  int line_not_empty = FALSE;
  int i , j ;

  // #define INPUT_EXAM_DEBUG -1000
#define INPUT_EXAM_DEBUG 1

  //--------------------
  // Now we examine the left side..
  //
  column_not_empty = FALSE;
  for ( i = 0 ; i < input_surface -> w ; i ++ )
    {
      for ( j = 0 ; j < input_surface -> h ; j ++ )
	{
	  // if ( GetAlphaComponent ( input_surface , i , j ) != 0 )
	  //--------------------
	  // We allow alpha values of 0 AND of 1 to be considered transparent, so
	  // that also some blender settings with 'almost no' background color will
	  // be treated correctly.
	  //
	  if ( GetAlphaComponent ( input_surface , i , j ) > 1 )
	    {
	      column_not_empty = TRUE ;
	      DebugPrintf ( INPUT_EXAM_DEBUG , "\nFound alpha value of %d at location (%d/%d)." , 
			    GetAlphaComponent ( input_surface , i , j ) , i , j );
	    }
	}
      if ( column_not_empty ) 
	{
	  DebugPrintf ( INPUT_EXAM_DEBUG , "\nEnd of excessive space on left side found." );
	  DebugPrintf ( INPUT_EXAM_DEBUG , "\n%d columns of pixels can be cropped away..." , i );
	  cut_left = i ; 
	  break;
	}
    }

  //--------------------
  // Now we examine the right side..
  //
  column_not_empty = FALSE;
  for ( i = 0 ; i < input_surface -> w ; i ++ )
    {
      for ( j = 0 ; j < input_surface -> h ; j ++ )
	{
	  // if ( GetAlphaComponent ( input_surface , input_surface->w - i - 1 , j ) != 0 )
	  //--------------------
	  // We allow alpha values of 0 AND of 1 to be considered transparent, so
	  // that also some blender settings with 'almost no' background color will
	  // be treated correctly.
	  //
	  if ( GetAlphaComponent ( input_surface , input_surface->w - i - 1 , j ) > 1 )
	    {
	      column_not_empty = TRUE ;
	      DebugPrintf ( INPUT_EXAM_DEBUG , "\nFound alpha value of %d at location (%d/%d)." , 
			    GetAlphaComponent ( input_surface , input_surface->w - i - 1 , j ) , input_surface->w - i - 1 , j );
	    }
	}
      if ( column_not_empty ) 
	{
	  DebugPrintf ( INPUT_EXAM_DEBUG , "\nEnd of excessive space on right side found." );
	  DebugPrintf ( INPUT_EXAM_DEBUG , "\n%d columns of pixels can be cropped away..." , i );
	  cut_right = i ; 
	  break;
	}
    }

  //--------------------
  // Now we examine the upper side..
  //
  line_not_empty = FALSE;
  for ( i = 0 ; i < input_surface -> h ; i ++ )
    {
      for ( j = 0 ; j < input_surface -> w ; j ++ )
	{
	  // if ( GetAlphaComponent ( input_surface , j , i ) != 0 )
	  //--------------------
	  // We allow alpha values of 0 AND of 1 to be considered transparent, so
	  // that also some blender settings with 'almost no' background color will
	  // be treated correctly.
	  //
	  if ( GetAlphaComponent ( input_surface , j , i ) > 1 )
	    {
	      line_not_empty = TRUE ;
	      DebugPrintf ( INPUT_EXAM_DEBUG , "\nFound alpha value of %d at location (%d/%d)." , 
			    GetAlphaComponent ( input_surface , j , i ) , j , i );
	    }
	}
      if ( line_not_empty ) 
	{
	  DebugPrintf ( INPUT_EXAM_DEBUG , "\nEnd of excessive space on upper side found." );
	  DebugPrintf ( INPUT_EXAM_DEBUG , "\n%d lines of pixels can be cropped away..." , i );
	  cut_up = i ; 
	  break;
	}
    }

  //--------------------
  // Now we examine the lower side..
  //
  line_not_empty = FALSE;
  for ( i = 0 ; i < input_surface -> h ; i ++ )
    {
      for ( j = 0 ; j < input_surface -> w ; j ++ )
	{
	  // if ( GetAlphaComponent ( input_surface , j , input_surface -> h - i - 1 ) != 0 )
	  //--------------------
	  // We allow alpha values of 0 AND of 1 to be considered transparent, so
	  // that also some blender settings with 'almost no' background color will
	  // be treated correctly.
	  //
	  if ( GetAlphaComponent ( input_surface , j , input_surface -> h - i - 1 ) > 1 )
	    {
	      line_not_empty = TRUE ;
	      DebugPrintf ( INPUT_EXAM_DEBUG , "\nFound alpha value of %d at location (%d/%d)." , 
			    GetAlphaComponent ( input_surface , j , input_surface -> h - i - 1 ) , j , input_surface -> h - i - 1 );
	    }
	}
      if ( line_not_empty ) 
	{
	  DebugPrintf ( INPUT_EXAM_DEBUG , "\nEnd of excessive space on lower side found." );
	  DebugPrintf ( INPUT_EXAM_DEBUG , "\n%d lines of pixels can be cropped away..." , i );
	  cut_down = i ; 
	  break;
	}
    }

  DebugPrintf ( 1 , "\nAmount cropped on side N/S/E/W: %d/%d/%d/%d." , 
		cut_up , cut_down , cut_right , cut_left );

}; // void examine_input_surface ( void )

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
write_offset_file ( ) 
{
  char filename[10000];
  char linebuf[10000];

  //--------------------
  // Now the center of the object, what position in the png 
  // file does it have?
  //
  // int default_center_x = 359;
  // int default_center_y = 436;
  // int default_center_x = 98;
  // int default_center_y = 167;
  int default_center_x = 99;
  int default_center_y = 162;

  FILE *OffsetFile;  // to this file we will save all the ship data...

#define OFFSET_EXPLANATION_STRING "FreedroidRPG uses isometric viewpoint and at the same time images of various sizes for objects within the game.  To determine the correct location for each of these images in the main game screen, FreedroidRPG must somehow know where the 'origin' of the object in question is within the given graphics file.  This is what these offset files are for:  They describe how much and in which direction the top left corner of the visible object is shifted away from the 'origin' or rather 'feet point' of the object in the image displayed.\n\n"

  //--------------------
  // If the given image has some special size, we use different
  // default offset values
  //
  if ( ( input_surface -> w == 60 ) &&
       ( input_surface -> h == 60 ) )
    {
      default_center_x = 30 ;
      default_center_y = 45 ;
      DebugPrintf ( 0 , "\nCROPPY: Image size 60x60 recognized.  Using %d/%d default origin." ,
		    default_center_x , default_center_y );
    } 
  else if ( ( input_surface -> w == 64 ) &&
	    ( input_surface -> h == 64 ) )
    {
      default_center_x = 32 ;
      default_center_y = 37 ;
      DebugPrintf ( 0 , "\nCROPPY: Image size 64x64 recognized.  Using %d/%d default origin." ,
		    default_center_x , default_center_y );
    }
  else if ( ( input_surface -> w == 80 ) &&
	    ( input_surface -> h == 80 ) )
    {
      default_center_x = 40 ;
      default_center_y = 40 ;
      DebugPrintf ( 0 , "\nCROPPY: Image size 80x80 recognized.  Using %d/%d default origin." ,
		    default_center_x , default_center_y );
    }
  else if ( ( input_surface -> w == 100 ) &&
	    ( input_surface -> h == 100 ) )
    {
      default_center_x = 50 ;
      default_center_y = 71 ;
      DebugPrintf ( 0 , "\nCROPPY: Image size 100x100 recognized.  Using %d/%d default origin." ,
		    default_center_x , default_center_y );
    }
  else if ( ( input_surface -> w == 120 ) &&
	    ( input_surface -> h == 120 ) )
    {
      default_center_x = 60 ;
      default_center_y = 100 ;
      DebugPrintf ( 0 , "\nCROPPY: Image size 120x120 recognized.  Using %d/%d default origin." ,
		    default_center_x , default_center_y );
    }
  else if ( ( input_surface -> w == 128 ) &&
	    ( input_surface -> h == 128 ) )
    {
      default_center_x = 64 ;
      default_center_y = 100 ;
      DebugPrintf ( 0 , "\nCROPPY: Image size 128x128 recognized.  Using %d/%d default origin." ,
		    default_center_x , default_center_y );
    }
  else if ( ( input_surface -> w == 200 ) &&
	    ( input_surface -> h == 240 ) )
    {
      default_center_x = 99 ;
      default_center_y = 190 ;
      DebugPrintf ( 0 , "\nCROPPY: Image size 200x240 (typical smaller tux part rendering).  Using %d/%d default origin." ,
		    default_center_x , default_center_y );
    }
  else if ( ( input_surface -> w == 400 ) &&
	    ( input_surface -> h == 480 ) )
    {
      default_center_x = 199 ;
      default_center_y = 381 ;
      DebugPrintf ( 0 , "\nCROPPY: Image size 400x480 (typical larger tux part rendering).  Using %d/%d default origin." ,
		    default_center_x , default_center_y );
    }
  else
    {
      DebugPrintf ( 0 , "\nCROPPY ERROR:  Unrecognized image format received... terminating in order to prevent accidents..." , 
		    default_center_x , default_center_y );
      Terminate ( ERR );
    }
  //--------------------
  // Maybe some extra information was passed via the command line, which overrides
  // the default origin settings for this image.  We take that possibility into
  // account here.
  //
  if ( offset_x_override != -30000 )
    default_center_x = offset_x_override ;
  if ( offset_y_override != -30000 )
    default_center_y = offset_y_override ;

  //--------------------
  // Now we must determine the output filename
  //
  sprintf ( filename , input_filename );
  if ( strstr ( filename , ".png" ) == NULL )
    strcat ( filename , ".offset" );
  else
    {
      filename [ strlen ( filename ) - 4 ] = 0 ;
      strcat ( filename , ".offset" );
    }

  //--------------------
  // Now that we know which filename to use, we can open the save file for writing
  //
  if( ( OffsetFile = fopen(filename, "w")) == NULL) {
    printf("\n\nError opening save game file for writing...\n\nTerminating...\n\n");
    Terminate(ERR);
    // return ERR;
  }

  fwrite ( "\n\n" , strlen( "\n\n" ), 
	   sizeof(char), OffsetFile );

  fwrite ( START_OF_OFFSET_FILE_STRING , strlen( START_OF_OFFSET_FILE_STRING ), 
	   sizeof(char), OffsetFile );

  // fwrite ( OFFSET_EXPLANATION_STRING , strlen( OFFSET_EXPLANATION_STRING ), 
  // sizeof(char), OffsetFile );

  fwrite ( "\n\n" , strlen( "\n\n" ), 
	   sizeof(char), OffsetFile );

  sprintf ( linebuf , "\n%s%d\n" , OFFSET_FILE_OFFSETX_STRING , cut_left - default_center_x );
  fwrite ( linebuf , strlen( linebuf ), sizeof(char), OffsetFile);  

  sprintf ( linebuf , "\n%s%d\n" , OFFSET_FILE_OFFSETY_STRING , cut_up - default_center_y );
  fwrite ( linebuf , strlen( linebuf ), sizeof(char), OffsetFile);  

  sprintf ( linebuf , "\nGraphicsFileName=%s\n" , input_filename );
  fwrite ( linebuf , strlen( linebuf ), sizeof(char), OffsetFile);  

  fwrite ( "\n\n" , strlen( "\n\n" ), 
	   sizeof(char), OffsetFile );

  fwrite ( END_OF_OFFSET_FILE_STRING , strlen( END_OF_OFFSET_FILE_STRING ), 
	   sizeof(char), OffsetFile );

  fwrite ( "\n\n" , strlen( "\n\n" ), 
	   sizeof(char), OffsetFile );

  if( fclose( OffsetFile ) == EOF) 
    {
      printf("\n\nClosing of .offset file failed...\n\nTerminating\n\n");
      Terminate(ERR);
      // return ERR;
    }

  DebugPrintf( 1 , "\nSaving of '.offset' file successful.\n" );

}; // void write_offset_file ( ) 

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
copy_and_crop_input_file ( ) 
{
  char parameter_buf[5000];
  char mgk_filename[5000];

  sprintf ( parameter_buf , "mogrify -crop %dx%d+%d+%d %s" , input_surface->w - cut_left - cut_right ,
	    input_surface->h - cut_up - cut_down , cut_left , cut_up , input_filename );
  DebugPrintf ( 0 , "\nCROPPY:  Now executing command : %s" , parameter_buf );
  system ( parameter_buf );

  //--------------------
  // Now we must determine mgk-filename
  //
  strcpy ( mgk_filename , input_filename );
  if ( strstr ( mgk_filename , ".png" ) == NULL )
    {
      DebugPrintf ( -1000 , "\nCROPPY ERROR:  input_filename doesn't have .png?  strange!" );      
      Terminate ( ERR );
    }
  else
    {
      mgk_filename [ strlen ( mgk_filename ) - 4 ] = 0 ;
      strcat ( mgk_filename , ".mgk" );
    }

  //--------------------
  // Now we can copy any existing .mgk file over the surely existing .png file.
  // (This measure seems to be nescessary with mogrify versions like the one used
  // on basse's machine.)
  //
  sprintf ( parameter_buf , "mv %s %s 2>/dev/null" , mgk_filename , input_filename );
  DebugPrintf ( 0 , "\nCROPPY:  Now executing command : %s" , parameter_buf );
  system ( parameter_buf );

}; // void copy_and_crop_input_file ( ) ;

/* -----------------------------------------------------------------
 * This function is the heart of the game.  It contains the main
 * game loop.
 * ----------------------------------------------------------------- */
int
main (int argc, char *const argv[])
{
  SDL_Rect fill_rect ;

#define MAIN_DEBUG 1 

  DebugPrintf ( MAIN_DEBUG , "\nFreedroidRPG 'Croppy' Tool, starting to read command line....\n" );

  ParseCommandLine (argc, argv); 

  DebugPrintf ( MAIN_DEBUG , "\nFreedroidRPG 'Croppy' Tool, initializing video....\n" );

  InitVideo () ;

  DebugPrintf ( MAIN_DEBUG , "\nFreedroidRPG 'Croppy' Tool, now loading input file...\n" );

  input_surface = IMG_Load ( input_filename ) ;
  if ( input_surface == NULL )
    {
      DebugPrintf ( 0 , "\n\nERROR:  Unable to load input file... " );
      DebugPrintf ( 0 , "\nFile name was : %s . " , input_filename );
      Terminate ( ERR );
    }

  if ( !no_graphics_output )
    {
      SDL_BlitSurface ( input_surface , NULL , Screen , NULL ) ;
      SDL_Flip ( Screen );
      MyWait ( 1.2 ) ;
    }

  DebugPrintf ( MAIN_DEBUG , "\nFreedroidRPG 'Croppy' Tool, now examining and cropping surface...\n" );

  examine_input_surface ( );

  DebugPrintf ( MAIN_DEBUG , "\nFreedroidRPG 'Croppy' Tool, now cropping surface...\n" );

  if ( !no_graphics_output )
    create_output_surface ( ) ; 

  DebugPrintf ( MAIN_DEBUG , "\nFreedroidRPG 'Croppy' Tool, now saving output surface...\n" );

  // save_output_surface ( ) ;
  
  copy_and_crop_input_file ( ) ;

  write_offset_file ( ) ;

  if ( !no_graphics_output )
    {
      // SDL_FillRect( Screen , NULL , 0x0FFFF );
      background_surface = IMG_Load ( background_filename ) ;
      if ( background_surface == NULL )
	{
	  DebugPrintf ( 0 , "\n\nERROR:  Unable to load background file... " );
	  DebugPrintf ( 0 , "\nFile name was : %s . " , background_filename );
	  Terminate ( ERR );
	}
      SDL_BlitSurface ( background_surface , NULL , Screen , NULL ) ;
      fill_rect.x = 0 ;  fill_rect.y = 0 ; fill_rect.w = output_surface->w ; fill_rect.h = output_surface->h ;
      SDL_FillRect( Screen , &fill_rect , 0x00 );
      SDL_BlitSurface ( output_surface , NULL , Screen , NULL );
      SDL_Flip ( Screen );

      MyWait( 1.1 );
    }

  DebugPrintf ( MAIN_DEBUG , "\nCroppy finished.  Exiting...\n\n" );

  // Terminate ( OK );

  DebugPrintf ( 0 , "\n" );

  return ( 0 );

}; // int main ( ... )
