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

#include "../../src/pngfuncs.h"
#include "../../src/system.h"
#include "../../src/defs.h"
#include "../../src/getopt.h"
#include <stdlib.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480 

void Terminate (int exit_code, int save_config);

char* output_filename = NULL;
char* input_filename = NULL;
char* background_filename = "croppy_background_fill.png";
SDL_Surface *Screen;   // the graphics display 
SDL_Surface *input_surface;   // the graphics display 
SDL_Surface *output_surface;   // the graphics display 
SDL_Surface *background_surface;   // the graphics display 
int cut_left;
int cut_right;
int cut_up;
int cut_down;
int no_graphics_output = TRUE;

char copyright[] = "\nCopyright (C) 2002 Johannes Prix, Reinhard Prix\n\
Freedroid comes with NO WARRANTY to the extent permitted by law.\n\
You may redistribute copies of Freedroid\n\
under the terms of the GNU General Public License.\n\
For more information about these matters, see the file named COPYING.\n";


char usage_string[] =
  "Usage: croppy    [-v|--version] \n\
                    [-o|--output_file] \n\
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

int debug_level = 0;
int vid_bpp;

#define UNDEFINED_OFFSET -30000 // something completely unlikely

int offset_x_override = UNDEFINED_OFFSET;
int offset_y_override = UNDEFINED_OFFSET;

//--------------------
// We add this dummy, so that dialog editor and item editor
// will properly compile...
//
void print_trace(int signum)
{
}

/* ----------------------------------------------------------------------
 * This function gives the alpha component of a pixel, using a value of
 * 255 for the most opaque pixel and 0 for the least opaque pixel.
 * ---------------------------------------------------------------------- */
Uint8 GetAlphaComponent(SDL_Surface* surface, int x, int y)
{
	SDL_PixelFormat *fmt;
	Uint32 temp, pixel;
	Uint8 alpha;
	int bpp = surface->format->BytesPerPixel;

	//--------------------
	// First we extract the pixel itself and the
	// format information we need.
	//
	fmt = surface->format;
	SDL_LockSurface(surface);

	//--------------------
	// Now for the longest time we had this command here (which can actually segfault!!)
	//
	// pixel = * ( ( ( Uint32* ) surface -> pixels ) + x + y * surface->w )  ;
	// 
	pixel = *((Uint32*)(((Uint8*)(surface->pixels)) + (x + y * surface->w) * bpp));
	SDL_UnlockSurface(surface);

	//--------------------
	// Now we can extract the alpha component
	//
	temp = pixel & fmt->Amask;  /* Isolate alpha component */
	temp = temp >> fmt->Ashift; /* Shift it down to 8-bit */
	temp = temp << fmt->Aloss;  /* Expand to a full 8-bit number */
	alpha = (Uint8)temp;

	return alpha;

} // int GetAlphaComponent ( SDL_Surface* SourceSurface , int x , int y )

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void MyWait(float wait_time) 
{
	long start_ticks = SDL_GetTicks();

	while ((SDL_GetTicks() - start_ticks) < 1000.0 * wait_time);

} // void MyWait ( float wait_time )

/* ----------------------------------------------------------------------
 * This function works a malloc, except that it also checks for
 * success and terminates in case of "out of memory", so we don't
 * need to do this always in the code.
 * ---------------------------------------------------------------------- */
void *MyMalloc(long Mamount)
{
	void *Mptr = NULL;

	if ((Mptr = malloc ((size_t) Mamount)) == NULL) {
		printf (" MyMalloc(%ld) did not succeed!\n", Mamount);
		Terminate(EXIT_FAILURE, TRUE);
	}

	return Mptr;
} // void* MyMalloc ( long Mamount )

/* ----------------------------------------------------------------------
 * This function is used for debugging purposes.  It writes the
 * given string either into a file, on the screen, or simply does
 * nothing according to currently set debug level.
 * ---------------------------------------------------------------------- */
void DebugPrintf(int db_level, char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	if (db_level <= debug_level) {
		vfprintf(stdout, fmt, args);
		fflush(stdout);
	}

	va_end(args);
} // void DebugPrintf (int db_level, char *fmt, ...)

/* ----------------------------------------------------------------------
 * This function is used for terminating freedroid.  It will close
 * the SDL submodules and exit.
 * ---------------------------------------------------------------------- */
void Terminate(int exit_code, int save_config)
{
	DebugPrintf(2, "\nvoid Terminate(int ExitStatus) was called....");
	DebugPrintf(1, "\n----------------------------------------------------------------------");
	DebugPrintf(1, "\nTermination of Croppy initiated...");
		
	DebugPrintf(1,"Thank you for using the FreedroidRPG Croppy Tool.\n\n");
  
	SDL_Quit();
	exit (exit_code);
} // void Terminate ( int ExitCode )

/* -----------------------------------------------------------------
 *  parse command line arguments and set global switches 
 *  exit on error, so we don't need to return success status
 * -----------------------------------------------------------------*/
void parse_command_line(int argc, char *const argv[])
{
	static struct option long_options[] = 
	{
		{ "version",          0, 0,  'v' },
		{ "help",             0, 0,  'h' },
		{ "nographicsoutput", 0, 0,  'n' },
		{ "graphicsoutput",   0, 0,  'g' },
		{ "offset_x",         optional_argument, 0,  'x' },
		{ "offset_y",         optional_argument, 0,  'y' },
		{ "output_file",      required_argument , 0,  'o' },
		{ "input_file",       required_argument , 0,  'i' },
		{ "debug",            optional_argument, 0,  'd' },
		{  0,                 0, 0,   0  }
	};

	while (1) {
		int c;
		c = getopt_long(argc, argv, "gqnvi:o:h?d::", long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
		// version statement -v or --version
		// following gnu-coding standards for command line interfaces 
		case 'v':
			DebugPrintf(0, "\nFreedroid Croppy Tool, Version 1.1.\n");
			DebugPrintf(0, copyright);
			exit (EXIT_SUCCESS);
			break;

		case 'h':
		case '?':
			printf("%s", usage_string);
			exit (EXIT_SUCCESS);
			break;

		case 'g':
			no_graphics_output = FALSE;
			break;

		case 'o':
			if (optarg) {
				output_filename = strdup(optarg);
				DebugPrintf(1, "\nOutput file name set to : %s ", output_filename);
			} else {
				printf("\nERROR! -o specified, but no output file given... Exiting.\n\n");
				exit (EXIT_FAILURE);
			}
			break;

		case 'i':
			if (optarg) {
				input_filename = optarg;
				DebugPrintf(1, "\nInput file name set to : %s ", input_filename);
			} else {
				printf("\nERROR! -i specified, but no input file given... Exiting.\n\n");
				exit (EXIT_FAILURE);
			}
			break;
			
		case 'x':
			if (optarg) {
				sscanf(optarg, "%d", &offset_x_override);
				DebugPrintf(1, "\nOffset in x overridden: now set to %d. ", offset_x_override);
			} else {
				printf("\nERROR! -x specified, but no offset x argument given... Exiting.\n\n");
				exit (EXIT_FAILURE);
			}
			break;

		case 'y':
			if (optarg) {
				sscanf(optarg, "%d", &offset_y_override);
				DebugPrintf(1, "\nOffset in y overridden: now set to %d. ", offset_y_override);
			} else {
				printf("\nERROR! -y specified, but no offset y argument given... Exiting.\n\n");
				exit (EXIT_FAILURE);
			}
			break;

		case 'd':
			if (!optarg) {
				debug_level = 1;
			} else {
				debug_level = atoi(optarg);
			}
			break;

		default:
			printf("\nOption %c not implemented yet! Ignored.\n", c);
			break;
		} // switch(c) 
	}  // while(1) 

	if (input_filename == NULL) {
		DebugPrintf(-1, "\nERROR:  No input file specified... Terminating...\n");
		printf("\n%s", usage_string);
		Terminate(EXIT_FAILURE, TRUE);
  }

	if (output_filename == NULL) {
		output_filename = strdup(input_filename);
  }

} // ParseCommandLine 

/* -----------------------------------------------------------------
 * This funciton initialises the video display and opens up a 
 * window for graphics display.
 * -----------------------------------------------------------------*/
void init_video (void)
{
	/* Initialize the SDL library */
	if (SDL_Init ( SDL_INIT_VIDEO ) == -1) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		Terminate(EXIT_FAILURE, FALSE);
	} else {
		DebugPrintf(1, "\nSDL Video initialisation successful.\n");
	}
	// Now SDL_TIMER is initialized here:

	if (SDL_InitSubSystem(SDL_INIT_TIMER) == -1) {
		fprintf(stderr, "Couldn't initialize SDL Timer: %s\n",SDL_GetError());
		Terminate(EXIT_FAILURE, FALSE);
	} else {
      DebugPrintf(1, "\nSDL Timer initialisation successful.\n");
	}
	
	if (!no_graphics_output) {
		if (!(Screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0))) {
			fprintf(stderr, "Couldn't set (2*) 320x240*SCALE_FACTOR video mode: %s\n",
					SDL_GetError()); 
			exit (EXIT_FAILURE);
		}
		SDL_Flip(Screen);
	}

	/* clean up on exit */
	atexit (SDL_Quit);

} // InitVideo () 

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void create_output_surface(void)
{
	SDL_Rect target_rect;
	SDL_Rect source_rect;

	output_surface = SDL_CreateRGBSurface( 
			input_surface->flags, 
			input_surface->w - cut_left - cut_right, 
			input_surface->h - cut_up - cut_down,
			input_surface->format->BitsPerPixel, 
			input_surface->format->Rmask, 
			input_surface->format->Gmask,
			input_surface->format->Bmask, 
			input_surface->format->Amask);

	DebugPrintf(1, "\nNew surface was created with dimensions:  %d:%d. ",
			output_surface->w, output_surface->h);

	//--------------------
	// Now we can copy the information from the input surface to the output surface...
	//
	source_rect.x = cut_left; 
	source_rect.y = cut_up;
	source_rect.w = input_surface->w - cut_left - cut_right; 
	source_rect.h = input_surface->h - cut_up - cut_down;

	target_rect.x = 0;
	target_rect.y = 0;
	target_rect.w = output_surface->w;
	target_rect.h = output_surface->h;
	
	SDL_SetClipRect(output_surface, NULL);

	SDL_SetAlpha(input_surface, 0, SDL_ALPHA_OPAQUE);
	SDL_BlitSurface(input_surface, &source_rect, output_surface, &target_rect);
} // void create_output_surface ( void )

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void examine_input_surface(void)
{
	int column_not_empty = FALSE;
	int line_not_empty = FALSE;
	int i, j;

#define INPUT_EXAM_DEBUG 1

	DebugPrintf(INPUT_EXAM_DEBUG, "\nInput surface dimension %d/%d", input_surface->w, input_surface->h);

	//--------------------
	// Now we examine the left side..
	//
	column_not_empty = FALSE;
	cut_left = 0;
	
	for (i = 0; i < input_surface->w; i++) {
		for (j = 0; j < input_surface->h; j++) {
			//--------------------
			// We allow alpha values of 0 AND of 1 to be considered transparent, so
			// that also some blender settings with 'almost no' background color will
			// be treated correctly.
			//
			if (GetAlphaComponent(input_surface, i, j) > 1) {
				column_not_empty = TRUE;
				DebugPrintf(INPUT_EXAM_DEBUG, "\nFound alpha value of %d at location (%d/%d).", 
						GetAlphaComponent(input_surface, i, j), i, j);
				break;
			}
		}
		if (column_not_empty) {
			DebugPrintf(INPUT_EXAM_DEBUG, "\nEnd of excessive space on left side found.");
			DebugPrintf(INPUT_EXAM_DEBUG, "\n%d columns of pixels can be cropped away...", i);
			cut_left = i; 
			break;
		}
	}

	//--------------------
	// Now we examine the right side..
	//
	cut_right = 0;
	column_not_empty = FALSE;
	
	for (i = 0; i < input_surface->w; i++) {
		for (j = 0; j < input_surface->h; j++) {
			//--------------------
			// We allow alpha values of 0 AND of 1 to be considered transparent, so
			// that also some blender settings with 'almost no' background color will
			// be treated correctly.
			//
			if (GetAlphaComponent(input_surface, input_surface->w - i - 1, j) > 1) {
				column_not_empty = TRUE;
				DebugPrintf(INPUT_EXAM_DEBUG, "\nFound alpha value of %d at location (%d/%d).",
						GetAlphaComponent(input_surface, input_surface->w - i - 1, j), input_surface->w - i - 1, j);
				break;
			}
		}
		if (column_not_empty) {
			DebugPrintf(INPUT_EXAM_DEBUG, "\nEnd of excessive space on right side found." );
			DebugPrintf(INPUT_EXAM_DEBUG, "\n%d columns of pixels can be cropped away..." , i);
			cut_right = i;
			break;
		}
	}

	//--------------------
	// Now we examine the upper side..
	//
	cut_up = 0;
	line_not_empty = FALSE;
	
	for (i = 0; i < input_surface->h; i++) {
		for (j = 0; j < input_surface->w; j++) {
			//--------------------
			// We allow alpha values of 0 AND of 1 to be considered transparent, so
			// that also some blender settings with 'almost no' background color will
			// be treated correctly.
			//
			if (GetAlphaComponent(input_surface, j, i) > 1) {
				line_not_empty = TRUE;
				DebugPrintf(INPUT_EXAM_DEBUG, "\nFound alpha value of %d at location (%d/%d).",
						GetAlphaComponent(input_surface, j, i), j, i);
				break;
			}
		}
		if (line_not_empty) {
			DebugPrintf(INPUT_EXAM_DEBUG, "\nEnd of excessive space on upper side found.");
			DebugPrintf(INPUT_EXAM_DEBUG, "\n%d lines of pixels can be cropped away...", i);
			cut_up = i;
			break;
		}
	}

	//--------------------
	// Now we examine the lower side..
	//
	cut_down = 0;
	line_not_empty = FALSE;
	
	for (i = 0; i < input_surface->h; i++) {
		for (j = 0; j < input_surface->w; j++) {
			//--------------------
			// We allow alpha values of 0 AND of 1 to be considered transparent, so
			// that also some blender settings with 'almost no' background color will
			// be treated correctly.
			//
			if (GetAlphaComponent(input_surface, j, input_surface -> h - i - 1) > 1) {
				line_not_empty = TRUE;
				DebugPrintf(INPUT_EXAM_DEBUG, "\nFound alpha value of %d at location (%d/%d).", 
						GetAlphaComponent(input_surface, j, input_surface -> h - i - 1), j, input_surface -> h - i - 1);
				break;
			}
		}
		if (line_not_empty) {
			DebugPrintf(INPUT_EXAM_DEBUG, "\nEnd of excessive space on lower side found.");
			DebugPrintf(INPUT_EXAM_DEBUG, "\n%d lines of pixels can be cropped away...", i);
			cut_down = i; 
			break;
		}
	}

	DebugPrintf(1, "\nAmount cropped on side N/S/E/W: %d/%d/%d/%d.", cut_up, cut_down, cut_right, cut_left);

} // void examine_input_surface ( void )

int get_default_center(int *default_center_x, int *default_center_y)
{
	//--------------------
	// Now the center of the object, what position in the png 
	// file does it have?
	//
	// int default_center_x = 359;
	// int default_center_y = 436;
	// int default_center_x = 98;
	// int default_center_y = 167;
	*default_center_x = 99;
	*default_center_y = 162;

	//--------------------
	// If the given image has some special size, we use different
	// default offset values
	//
	if ((input_surface->w == 60) && (input_surface->h == 60)) {
		*default_center_x = 30;
		*default_center_y = 45;
		DebugPrintf(1, "\nCROPPY: Image size 60x60 recognized.  Using %d/%d default origin.", *default_center_x, *default_center_y);	
		return TRUE;
	}
	
	if ((input_surface->w == 64) && (input_surface->h == 64)) {
		*default_center_x = 32;
		*default_center_y = 37;
		DebugPrintf(1, "\nCROPPY: Image size 64x64 recognized.  Using %d/%d default origin.", *default_center_x, *default_center_y);
		return TRUE;
	}
	
	if ((input_surface->w == 80) && (input_surface->h == 80)) {
		*default_center_x = 40;
		*default_center_y = 40;
		DebugPrintf(1, "\nCROPPY: Image size 80x80 recognized.  Using %d/%d default origin.", *default_center_x, *default_center_y);
		return TRUE;
	}
	
	if ((input_surface->w == 100) && (input_surface->h == 100)) {
		*default_center_x = 50;
		*default_center_y = 71;
		DebugPrintf(1, "\nCROPPY: Image size 100x100 recognized.  Using %d/%d default origin.", *default_center_x, *default_center_y);
		return TRUE;
	}
	
	if ((input_surface->w == 120) && (input_surface->h == 120)) {
		*default_center_x = 60;
		*default_center_y = 100;
		DebugPrintf(1, "\nCROPPY: Image size 120x120 recognized.  Using %d/%d default origin.", *default_center_x, *default_center_y);
		return TRUE;
	}
	
	if ((input_surface->w == 128) && (input_surface->h == 128)) {
		*default_center_x = 64;
		*default_center_y = 100;
		DebugPrintf(1, "\nCROPPY: Image size 128x128 recognized.  Using %d/%d default origin.", *default_center_x, *default_center_y);
		return TRUE;
	}
	
	if ((input_surface->w == 200) && (input_surface->h == 240)) {
		*default_center_x = 99;
		*default_center_y = 190;
		DebugPrintf(1, "\nCROPPY: Image size 200x240 (typical smaller tux part rendering).  Using %d/%d default origin.",
				*default_center_x, *default_center_y);
		return TRUE;
	}
	
	if ((input_surface->w == 400) && (input_surface->h == 480)) {
		*default_center_x = 199;
		*default_center_y = 381;
		DebugPrintf(1, "\nCROPPY: Image size 400x480 (typical larger tux part rendering).  Using %d/%d default origin.",
				*default_center_x, *default_center_y);
		return TRUE;
	}

	DebugPrintf(1, "\nCROPPY: Images size %dx%d unrecognized. Using %d/%d default origin. The offset value will be inaccurate.",
		*default_center_x, *default_center_y);
	return FALSE;
}

int read_offset_file(int *center_x, int *center_y)
{
	char filename[10000];
	int offset_x = 0, offset_y = 0;
	FILE *offset_file;

	// Determine offset file name
	strcpy(filename, input_filename);
	if (strstr(filename, ".png"))
		filename[strlen(filename) - 4] = '\0';
	strcat(filename, ".offset");

	// Read offset file
	if ((offset_file = fopen(filename, "r")) == NULL)
		return 0;

	fseek(offset_file, 0, SEEK_END);
	long file_size = ftell(offset_file);
	fseek(offset_file, 0, SEEK_SET);

	char *offset_data = malloc(file_size + 1);
	size_t bytes_read = fread(offset_data, 1, file_size, offset_file);
	fclose(offset_file);
	offset_data[file_size] = '\0';

	if (bytes_read != file_size) {
		free(offset_data);
		return 0;
	}

	char *offset_x_string = strstr(offset_data, OFFSET_FILE_OFFSETX_STRING);
	char *offset_y_string = strstr(offset_data, OFFSET_FILE_OFFSETY_STRING);

	if (offset_x_string)
		sscanf(offset_x_string, OFFSET_FILE_OFFSETX_STRING "%d", &offset_x);
	if (offset_y_string)
		sscanf(offset_y_string, OFFSET_FILE_OFFSETY_STRING "%d", &offset_y);
	free(offset_data);

	fprintf(stderr, "Using offset %d/%d from file %s\n", offset_x, offset_y, filename);

	*center_x = -offset_x;
	*center_y = -offset_y;
	return 1;
}

void write_offset_file(int default_center_x, int default_center_y) 
{
	char filename[10000];
	
	FILE *OffsetFile;  // to this file we will save all the ship data...

#define OFFSET_EXPLANATION_STRING "FreedroidRPG uses isometric viewpoint and at the same time images of various sizes for objects within the game.  To determine the correct location for each of these images in the main game screen, FreedroidRPG must somehow know where the 'origin' of the object in question is within the given graphics file.  This is what these offset files are for:  They describe how much and in which direction the top left corner of the visible object is shifted away from the 'origin' or rather 'feet point' of the object in the image displayed.\n\n"

	//--------------------
	// Maybe some extra information was passed via the command line, which overrides
	// the default origin settings for this image.  We take that possibility into
	// account here.
	//
	if (offset_x_override != UNDEFINED_OFFSET)
		default_center_x = offset_x_override;
	if (offset_y_override != UNDEFINED_OFFSET)
		default_center_y = offset_y_override;
	
	//--------------------
	// Now we must determine the output filename
	//
	sprintf(filename, "%s", output_filename);
	if (strstr(filename, ".png") == NULL) {
		strcat(filename, ".offset");
	} else {
		filename[strlen(filename) - 4] = 0;
		strcat(filename, ".offset");
	}

	//--------------------
	// Now that we know which filename to use, we can open the save file for writing
	//
	if ((OffsetFile = fopen(filename, "w")) == NULL) {
		DebugPrintf(-1, "\n\nError opening save game file for writing...\n\nTerminating...\n\n");
		Terminate(EXIT_FAILURE, TRUE);
	}

	fprintf(OffsetFile, "** Start of iso_image offset file **\n");

	fprintf(OffsetFile, "%s%d\n", OFFSET_FILE_OFFSETX_STRING, cut_left - default_center_x);

	fprintf(OffsetFile, "%s%d\n", OFFSET_FILE_OFFSETY_STRING, cut_up - default_center_y);

	fprintf(OffsetFile, "%s\n", END_OF_OFFSET_FILE_STRING);

	fflush(OffsetFile);

	if (fclose(OffsetFile) == EOF) {
		DebugPrintf(-1, "\n\nClosing of .offset file failed...\n\nTerminating\n\n");
		Terminate(EXIT_FAILURE, TRUE);
	}

	DebugPrintf(1, "\nSaving of '.offset' file successful.\n");

} // void write_offset_file ( ) 

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void save_output_surface() 
{
	if (png_save_surface(output_filename, output_surface) != 0) {
		DebugPrintf(-1, "\nCROPPY ERROR: unable to save the cropped png");      
		Terminate(EXIT_FAILURE, TRUE);
	}
} // void copy_and_crop_input_file ( ) ;

/* -----------------------------------------------------------------
 * This function is the heart of the game.  It contains the main
 * game loop.
 * ----------------------------------------------------------------- */
int main(int argc, char *argv[])
{
	SDL_Rect fill_rect;
	int default_center_x, default_center_y;

#define MAIN_DEBUG 1 

	DebugPrintf(MAIN_DEBUG, "\nFreedroidRPG 'Croppy' Tool, starting to read command line....\n");

	parse_command_line(argc, argv);

	DebugPrintf(MAIN_DEBUG, "\nFreedroidRPG 'Croppy' Tool, initializing video....\n");

	if (!no_graphics_output)
		init_video();

	DebugPrintf(MAIN_DEBUG, "\nFreedroidRPG 'Croppy' Tool, now loading input file...\n");

	input_surface = IMG_Load(input_filename);
	if (input_surface == NULL) {
		DebugPrintf(-1, "\n\nERROR:  Unable to load input file... ");
		DebugPrintf(-1, "\nFile name was : %s . ", input_filename);
		Terminate(EXIT_FAILURE, TRUE);
	}

	get_default_center(&default_center_x, &default_center_y);
	read_offset_file(&default_center_x, &default_center_y);

	if (!no_graphics_output) {
		SDL_BlitSurface(input_surface, NULL, Screen, NULL);
		SDL_Flip(Screen);
		MyWait(1.2);
	}

	DebugPrintf(MAIN_DEBUG, "\nFreedroidRPG 'Croppy' Tool, now examining and cropping surface...\n");

	examine_input_surface();

	DebugPrintf(MAIN_DEBUG, "\nFreedroidRPG 'Croppy' Tool, now cropping surface...\n");
	
	create_output_surface(); 

	DebugPrintf(MAIN_DEBUG, "\nFreedroidRPG 'Croppy' Tool, now saving output surface...\n");

	save_output_surface();

	write_offset_file(default_center_x, default_center_y);

	if (!no_graphics_output) {
		background_surface = IMG_Load(background_filename);
		if (background_surface == NULL) {
			DebugPrintf(-1, "\n\nERROR:  Unable to load background file... ");
			DebugPrintf(-1, "\nFile name was : %s . ", background_filename);
			Terminate(EXIT_FAILURE, TRUE);
		}
		SDL_BlitSurface(background_surface, NULL, Screen, NULL);
		fill_rect.x = 0;
		fill_rect.y = 0;
		fill_rect.w = output_surface->w;
		fill_rect.h = output_surface->h;
		SDL_FillRect(Screen, &fill_rect, 0x00);
		SDL_BlitSurface(output_surface, NULL, Screen, NULL);
		SDL_Flip(Screen);

		MyWait(1.1);
	}

	DebugPrintf(MAIN_DEBUG, "\nCroppy finished.  Exiting...\n\n");

	DebugPrintf(1, "\n");

	if (output_filename) {
		free(output_filename);
	}
	
	return (EXIT_SUCCESS);

} // int main ( ... )
