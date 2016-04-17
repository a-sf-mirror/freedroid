/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2010 Arthur Huillet 
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
 * This file contains graphics primitives, such as initialisation of SDL
 * and video modes.
 */
#define _graphics_c

#include "pngfuncs.h"

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "widgets/widgets.h"

static const SDL_VideoInfo *vid_info;

/**
 * We always want to blit our own mouse cursor.
 */
void blit_mouse_cursor(void)
{
	static int loaded = FALSE;
	int i;
	static struct image mouse_cursors[16];
	char constructed_filename[2000];
	int cursor_index = (-1);
	point cursoff = { 0, 0 };

	// On the first function call ever, we load the surfaces for the
	// flags into memory.
	//
	if (!loaded) {
		for (i = 0; i < 10; i++) {
			sprintf(constructed_filename, "cursors/mouse_cursor_%04d.png", i);
			load_image(&mouse_cursors[i], GUI_DIR, constructed_filename, NO_MOD);
		}
		loaded = TRUE;
	}

	switch (mouse_cursor) {
	case MOUSE_CURSOR_SCROLL_UP:
		cursor_index = 4;
		cursoff.x = -12;
		break;
	case MOUSE_CURSOR_SCROLL_DOWN:
		cursor_index = 5;
		cursoff.x = -12;
		cursoff.y = -30;
		break;
	case MOUSE_CURSOR_NORMAL:
		cursor_index = 0;
		cursoff.x = -5;
		cursoff.y = -4;
		break;
	case MOUSE_CURSOR_REPAIR:
		cursor_index = 6;
		break;
	case MOUSE_CURSOR_SELECT_TOOL:
		cursor_index = 9;
		cursoff.x = -32;
		cursoff.y = -16;
		break;
	case MOUSE_CURSOR_DRAGDROP_TOOL:
		cursor_index = 3;
		break;
	default:
		error_message(__FUNCTION__, "Illegal mouse cursor encountered: %d",
					 PLEASE_INFORM | IS_FATAL, mouse_cursor);
		break;
	}

	// Blit the mouse cursor
	display_image_on_screen(&mouse_cursors[cursor_index],
									  GetMousePos_x() + cursoff.x, GetMousePos_y() + cursoff.y, IMAGE_NO_TRANSFO);

	// Reset the mouse cursor for next frame
	mouse_cursor = MOUSE_CURSOR_NORMAL;
}

static void fade(int fade_delay, int direction)
{
	SDL_Surface* bg = NULL;

	Activate_Conservative_Frame_Computation();
	AssembleCombatPicture(SHOW_ITEMS | NO_CURSOR);

	if (use_open_gl) {
		StoreMenuBackground(0);
	} else {
		bg = SDL_DisplayFormat(Screen);
	}

	Uint32 now   = SDL_GetTicks();
	Uint32 start = now;
	while (now < start + fade_delay) {
		Uint8 fade = 255 * ((float)(now - start)) / fade_delay;

		if (direction < 0)
			fade = 255 - fade;

		if (!use_open_gl) {
			SDL_SetAlpha(bg, SDL_SRCALPHA | SDL_RLEACCEL, fade);
			SDL_FillRect(Screen, NULL, 0);
			SDL_BlitSurface(bg, NULL, Screen, NULL);
		} else {
#ifdef HAVE_LIBGL
			glColor4ub(fade, fade, fade, 255);
			RestoreMenuBackground(0);
			glColor4ub(255, 255, 255, 255);
#endif
		}
		blit_mouse_cursor();
		our_SDL_flip_wrapper();
		now = SDL_GetTicks();
	}

	if (!use_open_gl && bg)
		SDL_FreeSurface(bg);
}

/**
 * Effect to fade entire screen to black.
 */
void fade_out_screen(void)
{
	if (!GameConfig.do_fadings)
		return;
	fade(180, -1);
}

/**
 * Effect to fade entire screen from black.
 */
void fade_in_screen(void)
{
	if (!GameConfig.do_fadings)
		return;
	fade(180, 1);
}

/**
 * In the shop interface, when an item was selected that could be grouped
 * together in inventory, we showed three mouse buttons to either buy 1,
 * buy 10 or buy 100 or the similar thing for selling items.
 * But now Bastian has proposed a new number selector design with a scale
 * and a small knob to set the right number of items you wish to have and
 * also with small buttons left and right for some fine tuning. 
 * This function is intended to handle this number selection process.
 * It will accept the range allowed and do the complete selection process
 * with the user until he presses 'OK' on the scale screen.
 */
int do_graphical_number_selection_in_range(int lower_range, int upper_range, int default_value, int unit_price)
{
	static struct image selection_knob = EMPTY_IMAGE;
	int ok_button_was_pressed = FALSE;
	int escape_button_was_pressed = FALSE;
	int knob_start_x = UNIVERSAL_COORD_W(200);
	int knob_end_x = UNIVERSAL_COORD_W(390);
	if (!(upper_range - lower_range))
		return upper_range;
	float knob_step_size = (float)(knob_end_x - knob_start_x) / (float)(upper_range - lower_range);
	int knob_offset_x = ceilf((float)(default_value * knob_step_size));
	int knob_is_grabbed = FALSE;
	int knob_at = default_value;
	int delta = 0;
	int old_knob_at = 0;
	SDL_Event event;
	SDL_Rect knob_target_rect;
	int wait_before_repeat = 0; // 0: apply once only - 1: apply once and wait before repeat - 2: repeat after delay
	Uint32 time_before_repeat = 0;

	/* Initialize the text widget. */
	static struct widget_text item_description;
	widget_text_init(&item_description, "");
	widget_set_rect(WIDGET(&item_description), UNIVERSAL_COORD_W(310), UNIVERSAL_COORD_H(180), UNIVERSAL_COORD_W(75), UNIVERSAL_COORD_H(45));
	item_description.font = FPS_Display_Font;
	item_description.line_height_factor = 1.0;

	int old_game_status = game_status;

	StoreMenuBackground(1);

	if (!image_loaded(&selection_knob)) {
		load_image(&selection_knob, GUI_DIR, "mouse_buttons/number_selector_selection_knob.png", NO_MOD);
	}

	knob_target_rect.w = selection_knob.w;
	knob_target_rect.h = selection_knob.h;

	while (!ok_button_was_pressed && !escape_button_was_pressed) {
		RestoreMenuBackground(1);
		blit_background("number_selector.png");
		ShowGenericButtonFromList(NUMBER_SELECTOR_OK_BUTTON);
		knob_target_rect.x = knob_start_x + knob_offset_x - selection_knob.w / 2;
		knob_target_rect.y = UNIVERSAL_COORD_H(260) - selection_knob.h / 2;
		display_image_on_screen(&selection_knob, knob_target_rect.x, knob_target_rect.y, IMAGE_NO_TRANSFO);

		if (old_knob_at != knob_at) {
			item_description.scroll_offset = -3;
			free_autostr(item_description.text);
			item_description.text = alloc_autostr(100);
			autostr_append(item_description.text, "%d\n", knob_at);
			if (unit_price) {
				// TRANSLATORS: Used on object selector's widget to display unit price * number of objects
				autostr_append(item_description.text, _("%d price\n"), unit_price * knob_at);
			}
			old_knob_at = knob_at;
		}
		widget_text_display(WIDGET(&item_description));

		blit_mouse_cursor();
		our_SDL_flip_wrapper();
		SDL_framerateDelay(&SDL_FPSmanager);

		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {
				Terminate(EXIT_SUCCESS);
			} else if (event.type == SDL_KEYDOWN) {
				switch(event.key.keysym.sym) {
					case SDLK_LEFT:
						delta = -1;
						wait_before_repeat = 0; // apply once only
						break;
					case SDLK_RIGHT:
						delta = 1;
						wait_before_repeat = 0; // apply once only
						break;
					case SDLK_DOWN:
						knob_at = lower_range;
						break;
					case SDLK_UP:
						knob_at = upper_range;
						break;
					case SDLK_RETURN:
						ok_button_was_pressed = TRUE;
						break;
					default:
						break;
				}
			} else if (event.type == SDL_KEYUP) {
				delta = 0;
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					escape_button_was_pressed = TRUE;
				}
			} else if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == SDL_BUTTON_WHEELDOWN || event.button.button == SDL_BUTTON_WHEELUP) {
					knob_at = max(lower_range, min(upper_range, knob_at + (event.button.button == SDL_BUTTON_WHEELDOWN ? -1 : 1)));
				} else if (event.button.button == SDL_BUTTON_LEFT) {
					// Is event within knob's height?
					if (abs(event.button.y - (knob_target_rect.y + knob_target_rect.h / 2)) < knob_target_rect.h) {
						if (abs(event.button.x - (knob_target_rect.x + knob_target_rect.w / 2)) < knob_target_rect.w) {
							knob_is_grabbed = TRUE;
						} else if (event.button.x >= knob_start_x && event.button.x <= knob_end_x) {
							knob_at = (event.button.x - knob_start_x) / knob_step_size;
							knob_offset_x = (knob_at - lower_range) * knob_step_size;
							knob_is_grabbed = TRUE;
						}
					}
					
					if (MouseCursorIsOnButton(NUMBER_SELECTOR_OK_BUTTON, event.button.x, event.button.y)) {
						ok_button_was_pressed = TRUE;
					} else if (MouseCursorIsOnButton(NUMBER_SELECTOR_LEFT_BUTTON, event.button.x, event.button.y)) {
						delta = -1;
						wait_before_repeat = 1; // apply once and wait before repeat
					} else if (MouseCursorIsOnButton(NUMBER_SELECTOR_RIGHT_BUTTON, event.button.x, event.button.y)) {
						delta = 1;
						wait_before_repeat = 1; // apply once and wait before repeat
					}
				}
			} else if (event.type == SDL_MOUSEBUTTONUP) {
				if (event.button.button == SDL_BUTTON_LEFT) {
					delta = 0;
					knob_is_grabbed = FALSE;			
				}
			}

			if (knob_is_grabbed) {
				if (GetMousePos_x() - knob_start_x < (knob_at - lower_range) * knob_step_size) {
					knob_at = max(lower_range, min(upper_range, (GetMousePos_x() - knob_start_x + knob_step_size) / knob_step_size));
				} else {
					knob_at = max(lower_range, min(upper_range, (GetMousePos_x() - knob_start_x) / knob_step_size));
				}
				
				if (knob_offset_x < 0) {
					knob_offset_x = 0;
				} else if (knob_offset_x >= knob_end_x - knob_start_x) {
					knob_offset_x = knob_end_x - knob_start_x;
				}
			}
		}

		if (delta != 0) {
			if (wait_before_repeat == 0) { // apply once only (when numkeys are used to change the value)
				knob_at = max(lower_range, min(upper_range, knob_at + delta));
				delta = 0;
			} else if (wait_before_repeat == 1) { // apply once and wait before repeat
				knob_at = max(lower_range, min(upper_range, knob_at + delta));
				wait_before_repeat = 2; // repeat after delay
				time_before_repeat = SDL_GetTicks() + 500; // 500ms before starting to repeat
			} else if ((wait_before_repeat == 2) && (SDL_GetTicks() > time_before_repeat)) { // repeat apply
				knob_at = max(lower_range, min(upper_range, knob_at + delta));
				SDL_Delay(50); // 50ms interval between 2 repeats
			}
		}
		
		knob_offset_x = (knob_at - lower_range) * knob_step_size;
	}

	game_status = old_game_status;
	return (!escape_button_was_pressed ? knob_at : 0);

};				// int do_graphical_number_selection_in_range ( int lower_range , int upper_range )

/**
 * Return the pixel value at (x, y).
 *
 * NOTE: The surface must be locked before calling this!
 *
 * @param surf	The surface where we want to retrieve the pixel.
 * @param x	The X position of the pixel.
 * @param y	The Y position of the pixel.
 * @return The pixel value at (x, y) on success, zero on failure.
 */
uint32_t sdl_get_pixel(SDL_Surface *surf, int x, int y)
{
	int bpp = surf->format->BytesPerPixel;
	uint8_t *p = (uint8_t *)surf->pixels + y * surf->pitch + x * bpp;

	switch (bpp) {
	case 1:
		return *p;
		break;
	case 2:
		return *(uint16_t *)p;
		break;
	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return p[0] << 16 | p[1] << 8 | p[2];
		else
			return p[0] | p[1] << 8 | p[2] << 16;
		break;
	case 4:
		return *(uint32_t *)p;
		break;
	default:
		/* shouldn't happen, but avoids warnings */
		return 0;
	}
}

void sdl_put_pixel(SDL_Surface *surf, int x, int y, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	int bpp = surf->format->BytesPerPixel;
	uint32_t color;
	uint8_t *p;

	if ((x < 0) || (y < 0) || (x >= surf->w) || (y >= surf->h))
		return;

	color = SDL_MapRGBA(surf->format, red, green, blue, alpha);

	p = (Uint8 *) surf->pixels + y * surf->pitch + x * bpp;

	switch (bpp) {
	case 1:
			*p = color;
			break;
	case 2:
			*(Uint16 *) p = color;
			break;
	case 3:
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
				p[0] = (color >> 16) & 0xff;
				p[1] = (color >> 8) & 0xff;
				p[2] = color & 0xff;
			} else {
				p[0] = color & 0xff;
				p[1] = (color >> 8) & 0xff;
				p[2] = (color >> 16) & 0xff;
			}
			break;
	case 4:
			*(Uint32 *) p = color;
			break;
	}
}

static Uint8 add_val_to_component(Uint8 component, int value)
{
	int tmp;

	// Calculate the new value and check overflow/underflow.
	tmp = component + value;
	tmp = (tmp > 255) ? 255 : tmp;
	tmp = (tmp < 0) ? 0 : tmp;

    return (Uint8)tmp;
}
static void get_components(SDL_Surface *surf, int x, int y, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a)
{
	SDL_PixelFormat *fmt = surf->format;

	SDL_LockSurface(surf);
	Uint32 pixel = *((Uint32 *) (((Uint8 *) (surf->pixels)) + (x + y * surf->w) * surf->format->BytesPerPixel));
	SDL_UnlockSurface(surf);

	*r = (pixel >> fmt->Rshift) & 0xFF;
	*g = (pixel >> fmt->Gshift) & 0xFF;
	*b = (pixel >> fmt->Bshift) & 0xFF;
	*a = (pixel >> fmt->Ashift) & 0xFF;
}

SDL_Surface *sdl_create_colored_surface(SDL_Surface *surf, float r, float g, float b, float a, int highlight)
{
	uint8_t red, green, blue, alpha;
	SDL_Surface *colored_surf;
	int x, y;

	colored_surf = SDL_DisplayFormatAlpha(surf);

	for (y = 0; y < surf->h; y++) {
		for (x = 0; x < surf->w; x++) {

			get_components(surf, x, y, &red, &green, &blue, &alpha);

			if (r != 1.0)
				red *= r;
			if (g != 1.0)
				green *= g;
			if (b != 1.0)
				blue *= b;
			if (a != 1.0)
				alpha *= a;

			if (highlight) {
				red = add_val_to_component(red, 64);
				green = add_val_to_component(green, 64);
				blue = add_val_to_component(blue, 64);
			}

			sdl_put_pixel(colored_surf, x, y, red, green, blue, alpha);
		}
	}

	return colored_surf;
}

/**
 *
 *
 */
static void get_standard_iso_floor_tile_size(void)
{
	// iso_miscellaneous_floor_0000 dimensions
#define TILE_WIDTH 136
#define TILE_HEIGHT 69

	if (TILE_WIDTH % 2)
		iso_floor_tile_width = TILE_WIDTH - 3;
	else
		iso_floor_tile_width = TILE_WIDTH - 2;

	if (TILE_HEIGHT % 2)
		iso_floor_tile_height = TILE_HEIGHT	- 3;
	else
		iso_floor_tile_height = TILE_HEIGHT - 2;

	iso_floor_tile_width_over_two = iso_floor_tile_width / 2;
	iso_floor_tile_height_over_two = iso_floor_tile_height / 2;
}

/* -----------------------------------------------------------------
 * This function does all the bitmap initialisation, so that you
 * later have the bitmaps in perfect form in memory, ready for blitting
 * them to the screen.
 * ----------------------------------------------------------------- */
void InitPictures(void)
{
	// First thing to do is get the size of a typical isometric
	// floor tile, i.e. height and width of the corresponding graphics
	// bitmap
	//
	get_standard_iso_floor_tile_size();

	// Loading all these pictures might take a while...
	// and we do not want do deal with huge frametimes, which
	// could box the influencer out of the ship....
	Activate_Conservative_Frame_Computation();

	load_floor_tiles();

	next_startup_percentage(19);

	load_all_obstacles(TRUE);

	if (!GameConfig.lazyload) {
		load_all_items();
		// maybe also load all enemy models here?
	}

	Load_Blast_Surfaces();

	next_startup_percentage(5);

	Load_Mouse_Move_Cursor_Surfaces();

	iso_load_bullet_surfaces();

	next_startup_percentage(3);
}

/**
 * This function initializes the timer subsystem.
 */
void init_timer(void)
{
	// Now SDL_TIMER is initialized here:
	//
	if (SDL_InitSubSystem(SDL_INIT_TIMER) == -1) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		Terminate(EXIT_FAILURE);
	}
}

/**
 * This function checks if the availability of OpenGL libraries (at compile
 * time) and request of OpenGL graphics output are compatible with each
 * other...  If not, we just disable OpenGL output method...
 */
static void check_open_gl_libraries_present(void)
{
	// Here we introduce some warning output in case open_gl output is
	// requested while the game was compiled without having the GL libs...
	//
	// The solution in this case is to force open_gl switch off again and
	// to (forcefully) print out a warning message about this!
	//
	if (use_open_gl) {
#ifndef HAVE_LIBGL
		DebugPrintf(-100, "\n**********************************************************************\
\n*\
\n*  W  A  R  N  I  N  G    !  !  ! \
\n*\
\n* You have requested OpenGL output via command line switch (-o parameter)\
\n* but you (or someone else) compiled this version of FreedroidRPG without\
\n* having the necessary OpenGL libraries on your (his/her) system. \
\n*\
\n* FreedroidRPG will now fallback to normal SDL output (which might be a\
\n* lot slower than the OpenGL method.\n\
\n*\
\n* You might try setting appropriate speed optimization parameters in the\
\n* 'performance tweaks' menu, in case you run into speed trouble.\
\n*\
\n* If you prefer to use OpenGL output, please make sure that you have \
\n* libGL installed on your system and recompile FreedroidRPG.\
\n*\
\n***********************************************************************\
\n");
		use_open_gl = FALSE;
#endif
	}
};				// void check_open_gl_libraries_present ( void )

/**
 * This function should display the driver info obtained from the OpenGL
 * libraries.  This should be in a function of it's own (like now) to 
 * make sure that the OpenGL error checks in the video mode set functions
 * and that seem to be occurring so frequently are not coming from this 
 * chunk of code.
 */
#ifdef HAVE_LIBGL
static void show_open_gl_driver_info(void)
{
	// Since we want to use openGl, it might be good to check the OpenGL vendor string
	// provided by the graphics driver.  Let's see...
	//
	fprintf(stderr, "\n-OpenGL--------------------------------------------------------------------------");
	fprintf(stderr, "\nVendor     : %s", glGetString(GL_VENDOR));
	open_gl_check_error_status(__FUNCTION__);
	fprintf(stderr, "\nRenderer   : %s", glGetString(GL_RENDERER));
	open_gl_check_error_status(__FUNCTION__);
	fprintf(stderr, "\nVersion    : %s", glGetString(GL_VERSION));
	open_gl_check_error_status(__FUNCTION__);
	fprintf(stderr, "\nExtensions : %s", glGetString(GL_EXTENSIONS));
	open_gl_check_error_status(__FUNCTION__);
	fprintf(stderr, "\n\n");
}
#endif

/**
 * This function sets the OpenGL double buffering attribute.  We do this
 * in a separate function, so that eventual errors (and bug reports) from
 * the OpenGL error checking can be attributed to a source more easily.
 */
#ifdef HAVE_LIBGL
static void set_double_buffering_attribute(void)
{
	if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)) {
		error_message(__FUNCTION__, "\
Unable to set SDL_GL_DOUBLEBUFFER attribute!", PLEASE_INFORM | IS_FATAL);
	}
	// Since the OpenGL stuff hasn't been initialized yet, it's normal
	// to get an GL_INVALID_OPERATION here, if we would really do the
	// check.  So better refrain from OpenGL error checking here...
	//
	// open_gl_check_error_status ( __FUNCTION__ );
}
#endif

/**
 * This function is supposed to set the video mode in the case that 
 * OpenGL is used for graphics output.  The function is highly split up
 * into subfunctions now, so that the OpenGL error checking will be more
 * precise.  Typically it's in here that most problems occur when there
 * is a peculiar OpenGL driver used, mostly under the Windows operating
 * system.
 */
static void set_video_mode_for_open_gl(void)
{
#ifdef HAVE_LIBGL

	Uint32 video_flags = 0;	// flags for SDL video mode 
	int video_mode_ok_check_result;
	int buffer_size, depth_size, red_size, green_size, blue_size, alpha_size;
	// We need OpenGL double buffering, so we request it.  If we
	// can't get it, something must be wrong, maybe an extremely bad 
	// card/driver is present or some bad emulation.  Anyway, we'll
	// break off...
	//
	set_double_buffering_attribute();

	// Now we start setting up the proper OpenGL flags to pass to the
	// SDL for creating the initial output window...
	//
	video_flags = SDL_OPENGL;	/* Enable OpenGL in SDL */

	// Now according to the document http://sdldoc.csn.ul.ie/guidevideoopengl.php
	// we do need the SDL_GL_SetAttribute ( SDL_GL_DOUBLEBUFFER, 1 ) and NOT
	// this here...
	//

	if (GameConfig.fullscreen_on)
		video_flags |= SDL_FULLSCREEN;
	if (vid_info->hw_available)
		video_flags |= SDL_HWSURFACE;
	else
		video_flags |= SDL_SWSURFACE;

	if (vid_info->blit_hw)
		video_flags |= SDL_HWACCEL;

	// We have 24 bit (or 32 bit) color depth in some of the graphics used,
	// like e.g. backgrounds produced by Basse, so we try to get close to
	// a target color depth of 24, or 32.
	//
	vid_bpp = 32;

	// First we check to see if the mode we wish to set is really supported.  If it
	// isn't supported, then we cancel the whole operation...
	//
	video_mode_ok_check_result = SDL_VideoModeOK(GameConfig.screen_width, GameConfig.screen_height, vid_bpp, video_flags);
	switch (video_mode_ok_check_result) {
	case 0:
		error_message(__FUNCTION__,
				     "SDL reported that the video mode (%d x %d) mentioned above is not supported\n"
                     "To see all possible resolutions please run 'freedroidRPG -r99'\n"
                     "Resetting to current resolution (%d x %d)...",
                     NO_REPORT,
                     GameConfig.screen_width, GameConfig.screen_height,
                     vid_info->current_w, vid_info->current_h);
		//resetting configuration file to current (desktop) settings
		GameConfig.screen_width =  vid_info->current_w;
		GameConfig.screen_height = vid_info->current_h;
		GameConfig.next_time_width_of_screen = GameConfig.screen_width;
		GameConfig.next_time_height_of_screen = GameConfig.screen_height;
		break;
	default:
		DebugPrintf(-4, "\nTesting if color depth %d bits is available... ", vid_bpp);
		if (video_mode_ok_check_result == vid_bpp) {
			DebugPrintf(-4, "YES.");
		} else {
			DebugPrintf(-4, "NO! \nThe closest we will get is %d bits per pixel.", video_mode_ok_check_result);
			/*
			   ErrorMessage ( __FUNCTION__  , "\
			   SDL reported, that the video mode mentioned \nabove is not supported UNDER THE COLOR DEPTH MENTIONED ABOVE!\n\
			   We'll be using the alternate color depth given above instead...",
			   PLEASE_INFORM );
			 */
			vid_bpp = video_mode_ok_check_result;
		}
	}

	// Now that we know which mode to go for, we can give it a try and get the
	// output surface we want.  Of course, some extra checking will be done, so
	// that we know that the surface we're expecting is really there...
	//
	Screen = SDL_SetVideoMode(GameConfig.screen_width, GameConfig.screen_height, vid_bpp, video_flags);
	if (!Screen) {
		fprintf(stderr, "Video mode set failed: %s\n", SDL_GetError());
		Terminate(EXIT_FAILURE);
	} else {
		//      open_gl_check_error_status ( __FUNCTION__ );
		SDL_GL_GetAttribute(SDL_GL_BUFFER_SIZE, &buffer_size);
		SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &red_size);
		SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &green_size);
		SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &blue_size);
		SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &alpha_size);
		SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depth_size);
		fprintf(stderr, "\n\nvideo mode set (bpp=%d RGBA=%d%d%d%d depth=%d)",
			buffer_size, red_size, green_size, blue_size, alpha_size, depth_size);
	}

	show_open_gl_driver_info();

	safely_initialize_our_default_open_gl_parameters();

	// Maybe resize the window to standard size?
	//
	// resizeWindow( GameConfig . screen_width, GameConfig . screen_height );
	//

	// Now under win32 we're running into problems, because there seems to be some
	// garbage in ONE OF THE TWO BUFFERS from the double-buffering.  Maybe cleaning
	// that out solves part of the problem.  Well, not all, since there is still the
	// dialog background not visible.  But anyway, let's just clear the two buffers
	// for now...
	//
	clear_screen();
	our_SDL_flip_wrapper();
	clear_screen();
	our_SDL_flip_wrapper();
#endif
}

/**
 * This function initialises the video display and opens up a 
 * window for graphics display.
 */
void InitVideo(void)
{
	char vid_driver[81];
	Uint32 video_flags = 0;	// flags for SDL video mode 
	char fpath[PATH_MAX];

	// Tell SDL to center the window once we make it
	putenv("SDL_VIDEO_CENTERED=1");

	// Let's get some info about the whole system here.  Is this a windows or x11 or
	// mac or whatever graphical environment?
	//
	// NOTE:  This has got NOTHING to do with OpenGL and OpenGL venour or the like yet...
	//
	if (SDL_VideoDriverName(vid_driver, 80)) {
		DebugPrintf(-4, "\nVideo system type: %s.", vid_driver);
	} else {
		fprintf(stderr, "Video driver seems not to exist or initialization failure!\nError code: %s\n", SDL_GetError());
		Terminate(EXIT_FAILURE);
	}

	// We check if the program has been compiled with OpenGL libraries present
	// and take care of the case OpenGL output requested when compiled without
	// those libs...
	//
	check_open_gl_libraries_present();

	// We note the screen resolution used.
	//
	DebugPrintf(-4, "\nUsing screen resolution %d x %d.\n", GameConfig.screen_width, GameConfig.screen_height);

	// We query the available video configuration on this system.
	//
	vid_info = SDL_GetVideoInfo();
	if (!vid_info) {
		fprintf(stderr, "Could not obtain video info via SDL: %s\n", SDL_GetError());
		Terminate(EXIT_FAILURE);
	}

	if (use_open_gl) {
		set_video_mode_for_open_gl();
	} else {
		if (GameConfig.fullscreen_on)
			video_flags |= SDL_FULLSCREEN;

		if (!SDL_VideoModeOK(GameConfig.screen_width, GameConfig.screen_height, 32, video_flags))
		{
			error_message(__FUNCTION__,
			             "SDL reported that the video mode (%d x %d) mentioned above is not supported\n"
			             "To see all possible resolutions please run 'freedroidRPG -r99'\n"
			             "Resetting to current resolution (%d x %d)...",
			             NO_REPORT,
			             GameConfig.screen_width, GameConfig.screen_height,
			             vid_info->current_w, vid_info->current_h);
			//resetting configuration file to current (desktop) settings
			GameConfig.screen_width =  vid_info->current_w;
			GameConfig.screen_height = vid_info->current_h;
			GameConfig.next_time_width_of_screen = GameConfig.screen_width;
			GameConfig.next_time_height_of_screen = GameConfig.screen_height;
		}
		if (!(Screen = SDL_SetVideoMode(GameConfig.screen_width, GameConfig.screen_height, 0, video_flags))) {
			fprintf(stderr, "Video mode set failed: %s\n", SDL_GetError());
			Terminate(EXIT_FAILURE);
		}
	}

	vid_bpp = 32;		/* start with the simplest */

	// End of possibly open-gl dependent initialisation stuff...
	//
	if (vid_info->wm_available) {	/* if there's a window-manager */
		char window_title_string[200];
		sprintf(window_title_string, "FreedroidRPG %s", VERSION);
		SDL_WM_SetCaption(window_title_string, "");

		if (find_file(ICON_FILE, GUI_DIR, fpath, PLEASE_INFORM)) {
			SDL_Surface *icon = IMG_Load(fpath);
			SDL_WM_SetIcon(icon, NULL);
			SDL_FreeSurface(icon);
		}
	}

	init_fonts();

	blit_background("startup1.jpg");
	our_SDL_flip_wrapper();

	SDL_SetGamma(GameConfig.current_gamma_correction, GameConfig.current_gamma_correction, GameConfig.current_gamma_correction);

	mouse_cursor = MOUSE_CURSOR_NORMAL;
	SDL_ShowCursor(SDL_DISABLE);

	SDL_initFramerate(&SDL_FPSmanager);
	SDL_setFramerate(&SDL_FPSmanager, 40);
}

/**
 * Draw a colored rectangle on screen with alpha blending in SDL.
 *
 * @param rect The rectangular area.
 * @param r The red color value.
 * @param g The green color value.
 * @param b The blue color value.
 * @param a The alpha color value.
 */
void sdl_draw_rectangle(SDL_Rect *rect, int r, int g, int b, int a)
{
	SDL_PixelFormat *fmt = Screen->format;
	SDL_Surface *surface;

	// SDL_FillRect and SDL_BlitSurface clip the rectangle to fit on the surface, but
	// callers of sdl_draw_rectangle expect it to be unchanged, so store it.
	SDL_Rect old_rect = (*rect);

	if (a == SDL_ALPHA_OPAQUE) {
		// Do a rectangle fill operation if the input rectangle is opaque.
		SDL_FillRect(Screen, rect, SDL_MapRGB(Screen->format, r, g, b));

		// Restore the old rectangle.
		(*rect) = old_rect;
		return;
	}

	// Create an empty surface with 32 bits per pixel in video memory. This will
	// allow SDL to take advantage of video.
	surface = SDL_CreateRGBSurface(SDL_HWSURFACE, rect->w, rect->h, 32, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

	// Perform a fast fill of the whole rectangle with color.
	SDL_FillRect(surface, NULL, SDL_MapRGB(Screen->format, r, g, b));

	// Adjust the alpha properties of a surface and active the acceleration.
	SDL_SetAlpha(surface, SDL_SRCALPHA | SDL_RLEACCEL, a);

	// Blit the surface on screen and free it.
	SDL_BlitSurface(surface, NULL, Screen, rect);
	SDL_FreeSurface(surface);

	// Restore the old rectangle.
	(*rect) = old_rect;
}

void draw_rectangle(SDL_Rect *rect, int r, int g, int b, int a)
{
	if (use_open_gl) {
		gl_draw_rectangle(rect, r, g, b, a);
	} else {
		sdl_draw_rectangle(rect, r, g, b, a);
	}
}

static void sdl_draw_quad(const int16_t vx[4], const int16_t vy[4], int r, int g, int b, int a)
{
	filledPolygonRGBA(Screen, vx, vy, 4, r, g, b, a);
}

/**
 * Pay attention when you use this function because we must not use it when you
 * are inside a start_image_batch()/end_image_batch() operation but at the moment,
 * we do not have the choice.
 */
static void gl_draw_quad(const int16_t vx[4], const int16_t vy[4], int r, int g, int b, int a)
{
#ifdef HAVE_LIBGL
	glDisable(GL_TEXTURE_2D);
	glColor4ub(r, g, b, a);

	glBegin(GL_QUADS);
	glVertex2i(vx[0], vy[0]);
	glVertex2i(vx[1], vy[1]);
	glVertex2i(vx[2], vy[2]);
	glVertex2i(vx[3], vy[3]);
	glEnd();

	glColor4ub(255, 255, 255, 255);
	glEnable(GL_TEXTURE_2D);
#endif
}

void draw_quad(const int16_t vx[4], const int16_t vy[4], int r, int g, int b, int a)
{
	if (use_open_gl) {
		gl_draw_quad(vx, vy, r, g, b, a);
	} else {
		sdl_draw_quad(vx, vy, r, g, b, a);
	}
}

/**
 * This function draws a transparent black rectangle over a specified
 * area on the screen.
 */
void ShadowingRectangle(SDL_Surface * Surface, SDL_Rect Area)
{
	draw_rectangle(&Area, 0, 0, 0, 150);
}

/**
 * This function draws a transparent white rectangle over a specified
 * area on the screen.
 */
void HighlightRectangle(SDL_Surface * Surface, SDL_Rect Area)
{
	draw_rectangle(&Area, 255, 255, 255, 100);
}

/*
 * Draw an 'expanded' pixel.
 * Used to draw thick lines.
 */
static void draw_expanded_pixel(SDL_Surface * Surface, int x, int y, int xincr, int yincr, int thickness, int r, int g, int b)
{
	int i;

	sdl_put_pixel(Surface, x, y, r, g, b, 255);

	if (thickness <= 1)
		return;
	for (i = x + xincr; i != x + thickness * xincr; i += xincr) {
		sdl_put_pixel(Surface, i, y, r, g, b, 255);
	}
	for (i = y + yincr; i != y + thickness * yincr; i += yincr) {
		sdl_put_pixel(Surface, x, i, r, g, b, 255);
	}
}

/**
 * This function draws a line in SDL mode.
 * Classical Bresenham algorithm 
 */
static void draw_line_sdl(SDL_Surface *Surface, int x1, int y1, int x2, int y2, int r, int g, int b, int thickness)
{
	if (use_open_gl)
		return;

	int delta_x, incr_x;
	int delta_y, incr_y;
	int error_accum;

	// Algorithm initialization

	delta_x = x2 - x1;
	incr_x = 1;
	if (delta_x < 0) {
		delta_x = -delta_x;
		incr_x = -1;
	}

	delta_y = y2 - y1;
	incr_y = 1;
	if (delta_y < 0) {
		delta_y = -delta_y;
		incr_y = -1;
	}
	// Incremental line drawing

	if (delta_y < delta_x) {
		error_accum = delta_x >> 1;
		while (x1 != x2) {
			draw_expanded_pixel(Surface, x1, y1, incr_x, incr_y, thickness, r, g, b);
			error_accum += delta_y;
			if (error_accum > delta_x) {
				error_accum -= delta_x;
				y1 += incr_y;
			}
			x1 += incr_x;
		}
		draw_expanded_pixel(Surface, x1, y1, incr_x, incr_y, thickness, r, g, b);
	} else {
		error_accum = delta_y >> 1;
		while (y1 != y2) {
			draw_expanded_pixel(Surface, x1, y1, incr_x, incr_y, thickness, r, g, b);
			error_accum += delta_x;
			if (error_accum > delta_y) {
				error_accum -= delta_y;
				x1 += incr_x;
			}
			y1 += incr_y;
		}
		draw_expanded_pixel(Surface, x1, y1, incr_x, incr_y, thickness, r, g, b);
	}
}

/**
 * This function draws a line in OpenGL mode.
 */
static void draw_line_opengl(int x1, int y1, int x2, int y2, int r, int g, int b, int width)
{
#ifdef HAVE_LIBGL
	glLineWidth(width);
	glColor3ub(r, g, b);

	glDisable(GL_TEXTURE_2D);

	glBegin(GL_LINES);
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glEnd();

	glEnable(GL_TEXTURE_2D);

	glColor3ub(255, 255, 255);
#endif
}

void draw_line(float x1, float y1, float x2, float y2, uint8_t r, uint8_t g, uint8_t b, int width)
{
	if (!use_open_gl) {
		draw_line_sdl(Screen, x1, y1, x2, y2, r, g, b, width);
	} else {
		draw_line_opengl(x1, y1, x2, y2, r, g, b, width);
	}
}

void draw_line_on_map(float x1, float y1, float x2, float y2, uint8_t r, uint8_t g, uint8_t b, int width)
{
	int c1, c2, r1, r2;

	translate_map_point_to_screen_pixel(x1, y1, &r1, &c1);
	translate_map_point_to_screen_pixel(x2, y2, &r2, &c2);

	draw_line(r1, c1, r2, c2, r, g, b, width);
}

/**
 * This function saves a screenshot of the current game, scaled to the width number of pixels.
 * If width is set to 0, then no scaling is used.
 */
void save_screenshot(const char *filename, int width)
{
	Activate_Conservative_Frame_Computation();
	float scale_factor = width ? (float) width / GameConfig.screen_width : 1.0;
	SDL_Surface *screenshot = NULL;

#ifdef HAVE_LIBGL
	if (use_open_gl) {
		void *imgdata = malloc(GameConfig.screen_width * GameConfig.screen_height * 3);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(0, 0, GameConfig.screen_width, GameConfig.screen_height, GL_RGB, GL_UNSIGNED_BYTE, imgdata);

		// Now we need to make a real SDL surface from the raw image data we
		// have just extracted.
		//
		SDL_Surface *screen_surf =
		    SDL_CreateRGBSurfaceFrom(imgdata, GameConfig.screen_width, GameConfig.screen_height, 24, 3 * GameConfig.screen_width,
					     bmask, gmask, rmask, 0);
		screenshot = zoomSurface(screen_surf, scale_factor, scale_factor, 0);
		SDL_FreeSurface(screen_surf);
		free(imgdata);
	}
#endif
	if (!use_open_gl)
		screenshot = zoomSurface(Screen, scale_factor, scale_factor, 0);

	if (screenshot == NULL) {
		error_message(__FUNCTION__, "Cannot save image: %s", PLEASE_INFORM, filename);
		return;
	}

#ifdef HAVE_LIBGL
	if (use_open_gl) {
		flip_image_vertically(screenshot);
	}
#endif

	png_save_surface(filename, screenshot);
	SDL_FreeSurface(screenshot);
}

void reload_graphics(void)
{
	free_floor_tiles();
	load_floor_tiles();
	free_obstacle_graphics();
	load_all_obstacles(FALSE);
	// Free all items graphics. Graphics will be loaded when needed.
	free_item_graphics();
	// Free all enemies graphics. Graphics for an enemy will be loaded
	// when the enemy is encountered.
	free_enemy_graphics();
	reload_tux_graphics();
}

/**
 * Clear the screen.
 * This function will use either the glClear() or the SDL_FillRect()
 * depending on the current output method.
 */
void clear_screen(void)
{
#ifdef HAVE_LIBGL
	if (use_open_gl) {
		glClear(GL_COLOR_BUFFER_BIT);
		return;
	}
#endif

	SDL_FillRect(Screen, NULL, SDL_MapRGB(Screen->format, 0, 0, 0));
}

#undef _graphics_c
