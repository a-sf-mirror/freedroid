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
 * and video modes and fonts.
 */
/*
 * This file has been checked for remains of german comments in the code
 * I you still find some, please just kill it mercilessly.
 */
#define _graphics_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "SDL_rotozoom.h"

static const SDL_VideoInfo *vid_info;

/**
 * We always want to blit our own mouse cursor.
 */
void blit_our_own_mouse_cursor(void)
{
	static int loaded = FALSE;
	int i;
	static iso_image mouse_cursors[16];
	char constructed_filename[2000];
	int cursor_index = (-1);
	point cursoff = { 0, 0 };

	// On the first function call ever, we load the surfaces for the
	// flags into memory.
	//
	if (!loaded) {
		for (i = 0; i < 10; i++) {
			sprintf(constructed_filename, "mouse_cursor_%04d.png", i);
			load_iso_image(&mouse_cursors[i], constructed_filename, FALSE);
		}
		loaded = TRUE;
	}

	switch (global_ingame_mode) {
	case GLOBAL_INGAME_MODE_SCROLL_UP:
		cursor_index = 4;
		cursoff.x = -12;
		break;
	case GLOBAL_INGAME_MODE_SCROLL_DOWN:
		cursor_index = 5;
		cursoff.x = -12;
		break;
	case GLOBAL_INGAME_MODE_NORMAL:
		cursor_index = 0;
		cursoff.x = -5;
		cursoff.y = -4;
		break;
	case GLOBAL_INGAME_MODE_REPAIR:
		cursor_index = 6;
		break;
	case GLOBAL_INGAME_MODE_SELECT_TOOL:
		cursor_index = 9;
		cursoff.x = -32;
		cursoff.y = -16;
		break;
	case GLOBAL_INGAME_MODE_DRAGDROP_TOOL:
		cursor_index = 3;
		break;
	default:
		ErrorMessage(__FUNCTION__, "Illegal global ingame mode encountered: %d", PLEASE_INFORM, IS_FATAL, global_ingame_mode);
		break;
	}

	// We can now blit the mouse cursor...
	//
	if (use_open_gl) {
		draw_gl_textured_quad_at_screen_position(&mouse_cursors[cursor_index],
							 GetMousePos_x() + cursoff.x, GetMousePos_y() + cursoff.y);
	} else {
		blit_iso_image_to_screen_position(&mouse_cursors[cursor_index], GetMousePos_x() + cursoff.x, GetMousePos_y() + cursoff.y);
	}
}

/**
 * Occasionally it might come in handly to have the whole image fading
 * out when something time-consuming is happening, which is not displayed.
 * This function is intended to provide a mechanism for this using the
 * gamma adjustment.  Note, that the SDL documentation clearly mentions,
 * that NOT ALL HARDWARE SUPPORTS THIS.  But if it isn't supported, we
 * should still be safe and it should just mean that nothing will happen
 * in here except for some (unexplained) delay.
 */
void fade_out_using_gamma_ramp(void)
{
	if (!GameConfig.do_fadings)
		return;
	int i = 0;
	Activate_Conservative_Frame_Computation();
	if (!use_open_gl)
		for (i = 0; i < 100; i++) {
			SDL_SetGamma(GameConfig.current_gamma_correction * 0.01 * ((float)(100 - i)),
				     GameConfig.current_gamma_correction * 0.01 * ((float)(100 - i)),
				     GameConfig.current_gamma_correction * 0.01 * ((float)(100 - i)));
			SDL_Delay(4);
		}
#ifdef HAVE_LIBGL
	else {
		i = 255;
		StoreMenuBackground(0);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		while (--i) {
			glColor4ub(i, i, i, i);
			RestoreMenuBackground(0);
			SDL_GL_SwapBuffers();
		}
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}

#endif
};				// void fade_out_using_gamma_ramp ( void )

/**
 * Occasionally it might come in handly to have the whole image fading
 * out when something time-consuming is happening, which is not displayed.
 * This function is intended to provide a mechanism for this using the
 * gamma adjustment.  Note, that the SDL documentation clearly mentions,
 * that NOT ALL HARDWARE SUPPORTS THIS.  But if it isn't supported, we
 * should still be safe and it should just mean that nothing will happen
 * in here except for some (unexplained) delay.
 */
void fade_in_using_gamma_ramp(void)
{
	if (!GameConfig.do_fadings)
		return;
	int i;
	Activate_Conservative_Frame_Computation();

#ifdef HAVE_LIBGL
	if (!use_open_gl)
#endif
		for (i = 0; i < 100; i++) {
			SDL_SetGamma(GameConfig.current_gamma_correction * 0.01 * ((float)i),
				     GameConfig.current_gamma_correction * 0.01 * ((float)i),
				     GameConfig.current_gamma_correction * 0.01 * ((float)i));
			SDL_Delay(4);
		}
#ifdef HAVE_LIBGL
	else {
		i = 0;
		StoreMenuBackground(0);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		while (i++ < 255) {
			glColor4ub(i, i, i, i);
			RestoreMenuBackground(0);
			SDL_GL_SwapBuffers();
		}
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	}
#endif

};				// void fade_in_using_gamma_ramp ( void )

SDL_Surface *rip_rectangle_from_alpha_image(SDL_Surface * our_surface, SDL_Rect our_rect)
{
	SDL_Surface *padded_surf;
	SDL_Surface *tmp_surf;
	SDL_Rect dest;

	our_rect.h -= 0;

	padded_surf =
	    SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, our_rect.w, our_rect.h, 32, 0x0FF000000, 0x000FF0000, 0x00000FF00, 0x000FF);
	tmp_surf = SDL_DisplayFormatAlpha(padded_surf);
	SDL_FreeSurface(padded_surf);

	// SDL_SetAlpha( our_surface , 0 , 0 );
	SDL_SetAlpha(our_surface, 0, SDL_ALPHA_OPAQUE);
	SDL_SetAlpha(tmp_surf, 0, SDL_ALPHA_OPAQUE);
	SDL_SetColorKey(our_surface, 0, 0x0FF);

	dest.x = 0;
	dest.y = 0;
	dest.w = our_rect.w;
	dest.h = our_rect.h;

	our_SDL_blit_surface_wrapper(our_surface, &our_rect, tmp_surf, NULL);

	SDL_SetAlpha(tmp_surf, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);

	if (use_open_gl) {
		flip_image_vertically(tmp_surf);
	}

	return (tmp_surf);

};				// SDL_Surface* rip_rectangle_from_alpha_image ( SDL_Surface* our_surface , SDL_Rect our_rect ) 

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
int do_graphical_number_selection_in_range(int lower_range, int upper_range, int default_value)
{
	static SDL_Surface *SelectionKnob = NULL;
	int ok_button_was_pressed = FALSE;
	int knob_start_x = UNIVERSAL_COORD_W(200);
	int knob_end_x = UNIVERSAL_COORD_W(390);
	if (!(upper_range - lower_range))
		return upper_range;
	int knob_offset_x = ceilf((float)(default_value * (knob_end_x - knob_start_x)) / (float)(upper_range - lower_range + 1));
	int knob_is_grabbed = FALSE;
	char number_text[1000];
	static SDL_Rect knob_target_rect;
	SDL_Event event;

	int old_game_status = game_status;

	StoreMenuBackground(1);

	// Next we prepare the selection knob for all later operations
	//
	if (SelectionKnob == NULL) {
		char fpath[2048];
		find_file("mouse_buttons/number_selector_selection_knob.png", GRAPHICS_DIR, fpath, 0);
		SelectionKnob = our_IMG_load_wrapper(fpath);
	}
	if (SelectionKnob == NULL) {
		fprintf(stderr, "\n\nSDL_GetError: %s \n", SDL_GetError());
		ErrorMessage(__FUNCTION__, "\
ERROR LOADING SELECTION KNOB IMAGE FILE!", PLEASE_INFORM, IS_FATAL);
	}

	knob_target_rect.w = SelectionKnob->w;
	knob_target_rect.h = SelectionKnob->h;

	while (!ok_button_was_pressed) {
		RestoreMenuBackground(1);
		blit_special_background(NUMBER_SELECTOR_BACKGROUND_CODE);
		ShowGenericButtonFromList(NUMBER_SELECTOR_OK_BUTTON);
		knob_target_rect.x = knob_start_x + knob_offset_x - knob_target_rect.w / 2;
		knob_target_rect.y = UNIVERSAL_COORD_H(260) - knob_target_rect.h / 2;
		our_SDL_blit_surface_wrapper(SelectionKnob, NULL, Screen, &knob_target_rect);
		sprintf(number_text, "%d", knob_offset_x * (upper_range - lower_range + 1) / (knob_end_x - knob_start_x));
		PutStringFont(Screen, FPS_Display_BFont, UNIVERSAL_COORD_W(320), UNIVERSAL_COORD_H(190), number_text);
		blit_our_own_mouse_cursor();
		our_SDL_flip_wrapper();

		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {
				Terminate(0);
			}

			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_RIGHT) {
					if (knob_end_x - knob_start_x - knob_offset_x > ((knob_end_x - knob_start_x) / (upper_range - lower_range))) {
						knob_offset_x += (knob_end_x - knob_start_x) / (upper_range - lower_range + 1);
					}
					if (knob_offset_x < knob_end_x - knob_start_x - 1)
						knob_offset_x++;
				}

				if (event.key.keysym.sym == SDLK_LEFT) {
					if (knob_offset_x > ((knob_end_x - knob_start_x) / (upper_range - lower_range + 1))) {
						knob_offset_x -= (knob_end_x - knob_start_x) / (upper_range - lower_range + 1);
					}
					if (knob_offset_x > 0)
						knob_offset_x--;
				}

				if (event.key.keysym.sym == SDLK_RETURN) {
					ok_button_was_pressed = TRUE;
				}
			}

			if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == SDL_BUTTON_LEFT) {
					// Maybe the user has just 'grabbed the knob?  Then we need to
					// mark the knob as grabbed.
					//
					if ((abs(event.button.x - (knob_target_rect.x + knob_target_rect.w / 2)) < knob_target_rect.w) &&
							(abs(event.button.y - (knob_target_rect.y + knob_target_rect.h / 2)) < knob_target_rect.h)) {
						knob_is_grabbed = TRUE;
					}
					// OK pressed?  Then we can return the current scale value and
					// that's it...
					//
					if (MouseCursorIsOnButton(NUMBER_SELECTOR_OK_BUTTON, event.button.x, event.button.y))
						ok_button_was_pressed = TRUE;
					if (MouseCursorIsOnButton(NUMBER_SELECTOR_LEFT_BUTTON, event.button.x, event.button.y)) {
						if (knob_offset_x > ((knob_end_x - knob_start_x) / (upper_range - lower_range + 1))) {
							knob_offset_x -= (knob_end_x - knob_start_x) / (upper_range - lower_range + 1);
						}
						if (knob_offset_x > 0)
							knob_offset_x--;
					}

					if (MouseCursorIsOnButton(NUMBER_SELECTOR_RIGHT_BUTTON, event.button.x, event.button.y)) {
						if (knob_end_x - knob_start_x - knob_offset_x > ((knob_end_x - knob_start_x) / (upper_range - lower_range))) {
							knob_offset_x += (knob_end_x - knob_start_x) / (upper_range - lower_range + 1);
						}
						if (knob_offset_x < knob_end_x - knob_start_x - 1)
							knob_offset_x++;
					}
				}
			}

			if (event.type == SDL_MOUSEBUTTONUP) {
				if (event.button.button == SDL_BUTTON_LEFT) { 
					knob_is_grabbed = FALSE;
				}
			}

			if (knob_is_grabbed) {
				knob_offset_x = GetMousePos_x() - knob_start_x;
				if (knob_offset_x >= knob_end_x - knob_start_x)
					knob_offset_x = knob_end_x - knob_start_x - 1;
				if (knob_offset_x <= 0)
					knob_offset_x = 0;
			}
		}
	}

	game_status = old_game_status;
	int result = (knob_offset_x * (upper_range - lower_range + 1) / (knob_end_x - knob_start_x));
	return (result > upper_range ? upper_range : result);

};				// int do_graphical_number_selection_in_range ( int lower_range , int upper_range )

/**
 * This function gives the green component of a pixel, using a value of
 * 255 for the most green pixel and 0 for the least green pixel.
 */
static Uint8 GetGreenComponent(SDL_Surface * surface, int x, int y)
{
	SDL_PixelFormat *fmt;
	Uint32 temp, pixel;
	Uint8 green;
	int bpp = surface->format->BytesPerPixel;

	// First we extract the pixel itself and the
	// format information we need.
	//
	fmt = surface->format;
	SDL_LockSurface(surface);
	// pixel = * ( ( Uint32* ) surface -> pixels ) ;
	//
	// Now for the longest time we had this command here (which can actually segfault!!)
	//
	// pixel = * ( ( ( Uint32* ) surface -> pixels ) + x + y * surface->w )  ;
	// 
	pixel = *((Uint32 *) (((Uint8 *) (surface->pixels)) + (x + y * surface->w) * bpp));

	SDL_UnlockSurface(surface);

	// Now we can extract the green component
	//
	temp = pixel & fmt->Gmask;	/* Isolate green component */
	temp = temp >> fmt->Gshift;	/* Shift it down to 8-bit */
	temp = temp << fmt->Gloss;	/* Expand to a full 8-bit number */
	green = (Uint8) temp;

	return (green);

};				// int GetGreenComponent ( SDL_Surface* SourceSurface , int x , int y )

/**
 * This function gives the red component of a pixel, using a value of
 * 255 for the most red pixel and 0 for the least red pixel.
 */
static Uint8 GetRedComponent(SDL_Surface * surface, int x, int y)
{
	SDL_PixelFormat *fmt;
	Uint32 temp, pixel;
	Uint8 red;
	int bpp = surface->format->BytesPerPixel;

	// First we extract the pixel itself and the
	// format information we need.
	//
	fmt = surface->format;
	SDL_LockSurface(surface);
	// pixel = * ( ( Uint32* ) surface -> pixels ) ;
	// Now for the longest time we had this command here (which can actually segfault!!)
	//
	// pixel = * ( ( ( Uint32* ) surface -> pixels ) + x + y * surface->w )  ;
	// 
	pixel = *((Uint32 *) (((Uint8 *) (surface->pixels)) + (x + y * surface->w) * bpp));
	SDL_UnlockSurface(surface);

	// Now we can extract the red component
	//
	temp = pixel & fmt->Rmask;	/* Isolate red component */
	temp = temp >> fmt->Rshift;	/* Shift it down to 8-bit */
	temp = temp << fmt->Rloss;	/* Expand to a full 8-bit number */
	red = (Uint8) temp;

	return (red);

};				// int GetRedComponent ( SDL_Surface* SourceSurface , int x , int y )

/**
 * This function gives the blue component of a pixel, using a value of
 * 255 for the most blue pixel and 0 for the least blue pixel.
 */
static Uint8 GetBlueComponent(SDL_Surface * surface, int x, int y)
{
	SDL_PixelFormat *fmt;
	Uint32 temp, pixel;
	Uint8 blue;
	int bpp = surface->format->BytesPerPixel;

	// First we extract the pixel itself and the
	// format information we need.
	//
	fmt = surface->format;
	SDL_LockSurface(surface);
	// pixel = * ( ( Uint32* ) surface -> pixels ) ;
	// Now for the longest time we had this command here (which can actually segfault!!)
	//
	// pixel = * ( ( ( Uint32* ) surface -> pixels ) + x + y * surface->w )  ;
	// 
	pixel = *((Uint32 *) (((Uint8 *) (surface->pixels)) + (x + y * surface->w) * bpp));
	SDL_UnlockSurface(surface);

	// Now we can extract the blue component
	//
	temp = pixel & fmt->Bmask;	/* Isolate blue component */
	temp = temp >> fmt->Bshift;	/* Shift it down to 8-bit */
	temp = temp << fmt->Bloss;	/* Expand to a full 8-bit number */
	blue = (Uint8) temp;

	return (blue);

};				// int GetBlueComponent ( SDL_Surface* SourceSurface , int x , int y )

/**
 * This function gives the alpha component of a pixel, using a value of
 * 255 for the most opaque pixel and 0 for the least opaque pixel.
 */
Uint8 GetAlphaComponent(SDL_Surface * surface, int x, int y)
{
	SDL_PixelFormat *fmt;
	Uint32 temp, pixel;
	Uint8 alpha;
	int bpp = surface->format->BytesPerPixel;

	// First we extract the pixel itself and the
	// format information we need.
	//
	fmt = surface->format;
	SDL_LockSurface(surface);
	// Now for the longest time we had this command here (which can actually segfault!!)
	//
	// pixel = * ( ( ( Uint32* ) surface -> pixels ) + x + y * surface->w )  ;
	// 
	pixel = *((Uint32 *) (((Uint8 *) (surface->pixels)) + (x + y * surface->w) * bpp));
	SDL_UnlockSurface(surface);

	// Now we can extract the alpha component
	//
	temp = pixel & fmt->Amask;	/* Isolate alpha component */
	temp = temp >> fmt->Ashift;	/* Shift it down to 8-bit */
	temp = temp << fmt->Aloss;	/* Expand to a full 8-bit number */
	alpha = (Uint8) temp;

	return (alpha);

};				// Uint8 GetAlphaComponent ( SDL_Surface* SourceSurface , int x , int y )

static Uint8 add_val_to_component(Uint8 component, int value)
{
	int tmp;

	// Calculate the new value and check overflow/underflow.
	tmp = component + value;
	tmp = (tmp > 255) ? 255 : tmp;
	tmp = (tmp < 0) ? 0 : tmp;

    return (Uint8)tmp;
}

/**
 * This function can be used to create a new surface that has a certain
 * color filter applied to it.  The default so far will be that the blue
 * color filter will be applied.
 */
SDL_Surface *CreateColorFilteredSurface(SDL_Surface * FirstSurface, int FilterType)
{
	SDL_Surface *ThirdSurface;	// this will be the surface we return to the calling function.
	int x, y;		// for processing through the surface...
	Uint8 red = 0;
	Uint8 green = 0;
	Uint8 blue = 0;
	float alpha3;

	// First we check for NULL surfaces given...
	//
	if (FirstSurface == NULL) {
		DebugPrintf(0, "\nERROR in SDL_Surface* CreateBlueColorFilteredSurface ( ... ) : NULL PARAMETER GIVEN.\n");
		Terminate(ERR);
	}
	// Now we create a new surface, best in display format with alpha channel
	// ready to be blitted.
	//
	ThirdSurface = our_SDL_display_format_wrapperAlpha(FirstSurface);

	// Now we start to process through the whole surface and examine each
	// pixel.
	//
	for (y = 0; y < FirstSurface->h; y++) {
		for (x = 0; x < FirstSurface->w; x++) {

			alpha3 = GetAlphaComponent(FirstSurface, x, y);

			red = GetRedComponent(FirstSurface, x, y);
			green = GetGreenComponent(FirstSurface, x, y);
			blue = GetBlueComponent(FirstSurface, x, y);

			if (FilterType == FILTER_BLUE) {
				blue = (red + green + blue) / 3;
				red = 0;
				green = 0;
			} else if (FilterType == FILTER_GREEN) {
				green = (red + green + blue) / 3;
				red = 0;
				blue = 0;
			} else if (FilterType == FILTER_RED) {
				red = (red + green + blue) / 3;
				green = 0;
				blue = 0;
			} else if (FilterType == FILTER_HIGHLIGHT) {
				// Set luminosity.
				red = add_val_to_component(red, 64);
				green = add_val_to_component(green, 64);
				blue = add_val_to_component(blue, 64);
			}

			PutPixel(ThirdSurface, x, y, SDL_MapRGBA(ThirdSurface->format, red, green, blue, alpha3));

		}
	}

	return (ThirdSurface);

};				// SDL_Surface* CreateBlueColorFilteredSurface ( SDL_Surface* FirstSurface )

/**
 *
 *
 */
static void get_standard_iso_floor_tile_size(void)
{
	SDL_Surface *standard_floor_tile;
	char fp[2048];
	find_file("floor_tiles/iso_miscellaneous_floor_0000.png", GRAPHICS_DIR, fp, 0);
	standard_floor_tile = our_IMG_load_wrapper(fp);
	if (standard_floor_tile == NULL) {
		fprintf(stderr, "\n\nSDL_GetError: %s \n", SDL_GetError());
		ErrorMessage(__FUNCTION__, "\
UNABLE TO LOAD STANDARD TILE!", PLEASE_INFORM, IS_FATAL);
	}
	// Warning!  The standard tile sizes should be a multiple of 2 in
	//           both directions to prevent jittering from numerical 
	//           rounding.
	//
	//           They also should be a bit less than a real tile size
	//           to hide some of the gap from the shading and anti-aliasing
	//           of the rendering process.
	//
	if (standard_floor_tile->w % 2)
		iso_floor_tile_width = standard_floor_tile->w - 3;
	else
		iso_floor_tile_width = standard_floor_tile->w - 2;
	if (standard_floor_tile->h % 2)
		iso_floor_tile_height = standard_floor_tile->h - 3;
	else
		iso_floor_tile_height = standard_floor_tile->h - 2;

	iso_floor_tile_width_over_two = iso_floor_tile_width / 2;
	iso_floor_tile_height_over_two = iso_floor_tile_height / 2;
	SDL_FreeSurface(standard_floor_tile);

};				// void get_standard_iso_floor_tile_size ( void )

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

	next_startup_percentage(33);

	init_obstacle_data();
	if (!GameConfig.lazyload)
		load_all_obstacles();

	Load_Enemy_Surfaces();

	clear_all_loaded_tux_images(FALSE);

	Load_Blast_Surfaces();

	next_startup_percentage(19);

	Load_Mouse_Move_Cursor_Surfaces();

	iso_load_bullet_surfaces();

	next_startup_percentage(5);
}

/**
 * This function should load all the fonts we'll be using via the SDL
 * BFont library in Freedroid.
 */
void InitOurBFonts(void)
{
#define ALL_BFONTS_WE_LOAD 7

#define PARA_FONT_FILE 		"font/parafont"
#define MENU_FONT_FILE 		"font/cpuFont"
#define MESSAGEVAR_FONT_FILE 	"font/small_white"
#define MESSAGESTAT_FONT_FILE 	"font/small_blue"
#define RED_FONT_FILE 		"font/font05_red"
#define BLUE_FONT_FILE 		"font/font05_white"
#define FPS_FONT_FILE 		"font/font05"

	char fpath[2048];
	int i;
	const char *MenuFontFiles[ALL_BFONTS_WE_LOAD] = {
		MENU_FONT_FILE,
		MESSAGEVAR_FONT_FILE,
		MESSAGESTAT_FONT_FILE,
		PARA_FONT_FILE,
		FPS_FONT_FILE,
		RED_FONT_FILE,
		BLUE_FONT_FILE,
	};
	BFont_Info **MenuFontPointers[ALL_BFONTS_WE_LOAD] = {
		&Menu_BFont,
		&Messagevar_BFont,
		&Messagestat_BFont,
		&Para_BFont,
		&FPS_Display_BFont,
		&Red_BFont,
		&Blue_BFont
	};

	for (i = 0; i < ALL_BFONTS_WE_LOAD; i++) {
		char constructed_fname[2048];
		sprintf(constructed_fname, "%s", MenuFontFiles[i]);
		strcat(constructed_fname, ".png");

		if (find_file(constructed_fname, GRAPHICS_DIR, fpath, 0) != 0) {	//if the file wasn't found, default to the standard ASCII7bit file
			sprintf(constructed_fname, "%s.png", MenuFontFiles[i]);
			if (find_file(constructed_fname, GRAPHICS_DIR, fpath, 0) != 0) {
				fprintf(stderr, "\n\nFont file: '%s'.\n", MenuFontFiles[i]);
				ErrorMessage(__FUNCTION__, "\
A font file for the BFont library was not found.", PLEASE_INFORM, IS_FATAL);
			}
		}

		if ((*MenuFontPointers[i] = LoadFont(fpath)) == NULL) {
			fprintf(stderr, "\n\nFont file: '%s'.\n", MenuFontFiles[i]);
			ErrorMessage(__FUNCTION__, "\
A font file for the BFont library could not be loaded.", PLEASE_INFORM, IS_FATAL);
		} else {
			DebugPrintf(1, "\nSDL Menu Font initialisation successful.\n");
		}
	}

};				// InitOurBFonts ( void )

void FreeOurBFonts(void)
{
	int i;
	BFont_Info **MenuFontPointers[ALL_BFONTS_WE_LOAD] = {
		&Menu_BFont,
		&Messagevar_BFont,
		&Messagestat_BFont,
		&Para_BFont,
		&FPS_Display_BFont,
		&Red_BFont,
		&Blue_BFont
	};

	for (i = 0; i < ALL_BFONTS_WE_LOAD; i++) {
		if (*MenuFontPointers[i] != NULL) {
			FreeFont(*MenuFontPointers[i]);
			*MenuFontPointers[i] = NULL;
		}
	}
};				// FreeOurBFonts ( void )

/* -----------------------------------------------------------------
 * This funciton initialises the timer subsystem.
 * -----------------------------------------------------------------*/
void InitTimer(void)
{
	// Now SDL_TIMER is initialized here:
	//
	if (SDL_InitSubSystem(SDL_INIT_TIMER) == -1) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		Terminate(ERR);
	} else
		DebugPrintf(1, "\nSDL Timer initialisation successful.\n");

};				// void InitTimer (void)

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
\n* having the nescessary OpenGL libraries on your (his/her) system. \
\n*\
\n* FreedroidRPG will now fallback to normal SDL output (which might be a\
\n* lot slower than the OpenGL method.\n\
\n*\
\n* You might try setting appropriate speed optimisation parameters in the\
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
static void show_open_gl_driver_info(void)
{
#ifdef HAVE_LIBGL
	// Since we want to use openGl, it might be good to check the OpenGL vendor string
	// provided by the graphics driver.  Let's see...
	//
	fprintf(stderr, "\n-OpenGL-------------------------------------------------------");
	fprintf(stderr, "\nVendor     : %s", glGetString(GL_VENDOR));
	open_gl_check_error_status(__FUNCTION__);
	fprintf(stderr, "\nRenderer   : %s", glGetString(GL_RENDERER));
	open_gl_check_error_status(__FUNCTION__);
	fprintf(stderr, "\nVersion    : %s", glGetString(GL_VERSION));
	open_gl_check_error_status(__FUNCTION__);
	fprintf(stderr, "\nExtensions : %s", glGetString(GL_EXTENSIONS));
	open_gl_check_error_status(__FUNCTION__);
	fprintf(stderr, "\n\n");
#endif
};				// void safely_show_open_gl_driver_info ( void )

/**
 * This function sets the OpenGL double buffering attribute.  We do this
 * in a separate function, so that eventual errors (and bug reports) from
 * the OpenGL error checking can be attributed to a source more easily.
 */
static void set_double_buffering_attribute(void)
{

#ifdef HAVE_LIBGL

	if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)) {
		ErrorMessage(__FUNCTION__, "\
Unable to set SDL_GL_DOUBLEBUFFER attribute!", PLEASE_INFORM, IS_FATAL);
	}
	// Since the OpenGL stuff hasn't been initialized yet, it's normal
	// to get an GL_INVALID_OPERATION here, if we would really do the
	// check.  So better refrain from OpenGL error checking here...
	//
	// open_gl_check_error_status ( __FUNCTION__ );

#endif

};				// void safely_set_double_buffering_attribute ( void )

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
		ErrorMessage(__FUNCTION__, "\
SDL reported that the video mode (%d x %d) mentioned above is not supported\n\
To see all possible resolutions please run 'freedroidRPG -r99'\n\
Resetting to default resolution (800 x 600)...", NO_NEED_TO_INFORM, IS_WARNING_ONLY, GameConfig.screen_width, GameConfig.screen_height);
		//resetting configuration file to default settings
		GameConfig.screen_width =  800;
		GameConfig.screen_height = 600;
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
			   PLEASE_INFORM, IS_WARNING_ONLY );
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
		Terminate(ERR);
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
	our_SDL_fill_rect_wrapper(Screen, NULL, 0);
	our_SDL_flip_wrapper();
	our_SDL_fill_rect_wrapper(Screen, NULL, 0);
	our_SDL_flip_wrapper();

#endif				// HAVE_LIBGL

};				// void set_video_mode_for_open_gl ( void )

/* -----------------------------------------------------------------
 * This funciton initialises the video display and opens up a 
 * window for graphics display.
 * -----------------------------------------------------------------*/
void InitVideo(void)
{
	char vid_driver[81];
	Uint32 video_flags = 0;	// flags for SDL video mode 
	char fpath[2048];
	char window_title_string[200];

	// Initialize the SDL library 
	//
	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		Terminate(ERR);
	} else {
		DebugPrintf(1, "\nSDL Video initialisation successful.\n");
		// So the video library could be initialized.  So it should also be
		// cleaned up and closed once we're done and quit FreedroidRPG.
		//
		atexit(SDL_Quit);
	}

	// Let's get some info about the whole system here.  Is this a windows or x11 or
	// mac or whatever graphical environment?
	//
	// NOTE:  This has got NOTHING to do with OpenGL and OpenGL venour or the like yet...
	//
	if (SDL_VideoDriverName(vid_driver, 80)) {
		DebugPrintf(-4, "\nVideo system type: %s.", vid_driver);
	} else {
		fprintf(stderr, "Video driver seems not to exist or initialisation failure!\nError code: %s\n", SDL_GetError());
		Terminate(ERR);
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
		Terminate(ERR);
	}

	if (use_open_gl) {
		set_video_mode_for_open_gl();
	} else {
		if (GameConfig.fullscreen_on)
			video_flags |= SDL_FULLSCREEN;

		if (!SDL_VideoModeOK(GameConfig.screen_width, GameConfig.screen_height, 32, video_flags))
		{
			ErrorMessage(__FUNCTION__, "\
SDL reported that the video mode (%d x %d) mentioned above is not supported\n\
To see all possible resolutions please run 'freedroidRPG -r99'\n\
Resetting to default resolution (800 x 600)...", NO_NEED_TO_INFORM, IS_WARNING_ONLY, GameConfig.screen_width, GameConfig.screen_height);
			//resetting configuration file to default settings
			GameConfig.screen_width =  800;
			GameConfig.screen_height = 600;
			GameConfig.next_time_width_of_screen = GameConfig.screen_width;
			GameConfig.next_time_height_of_screen = GameConfig.screen_height;
		}
		if (!(Screen = SDL_SetVideoMode(GameConfig.screen_width, GameConfig.screen_height, 0, video_flags))) {
			fprintf(stderr, "Video mode set failed: %s\n", SDL_GetError());
			Terminate(ERR);		
		}
	}

	vid_bpp = 32;		/* start with the simplest */

	// End of possibly open-gl dependant initialisation stuff...
	//
	sprintf(window_title_string, "FreedroidRPG %s", VERSION);
	if (vid_info->wm_available) {	/* if there's a window-manager */
		SDL_Surface *icon;
		SDL_WM_SetCaption(window_title_string, "");
		find_file(ICON_FILE, GRAPHICS_DIR, fpath, 0);
		icon = IMG_Load(fpath);
		SDL_WM_SetIcon(icon, NULL);
		SDL_FreeSurface(icon);
	}

	InitOurBFonts();

	blit_special_background(FREEDROID_LOADING_PICTURE_CODE);
	our_SDL_flip_wrapper();

	SDL_SetGamma(GameConfig.current_gamma_correction, GameConfig.current_gamma_correction, GameConfig.current_gamma_correction);

	SDL_ShowCursor(SDL_DISABLE);

};				// void InitVideo () 

/**
 * This function fills all the screen or the freedroid window with a 
 * black color.  The name of the function originates from earlier, when
 * we still wrote directly to the vga memory using memset under ms-dos.
 */
void ClearGraphMem(void)
{
	SDL_SetClipRect(Screen, NULL);

	// Now we fill the screen with black color...
	our_SDL_fill_rect_wrapper(Screen, NULL, 0);
};				// void ClearGraphMem( void )

/**
 * Draw a colored rectangle on screen with alpha blending in SDL.
 *
 * @param rect The rectangular area.
 * @param r The red color value.
 * @param g The green color value.
 * @param b The blue color value.
 * @param a The alpha color value.
 */
static void sdl_draw_rectangle(SDL_Rect *rect, int r, int g, int b, int a)
{
	SDL_Surface *surface;

	if (a == SDL_ALPHA_OPAQUE) {
		// Do a rectangle fill operation if the input rectangle is opaque.
		SDL_FillRect(Screen, rect, SDL_MapRGB(Screen->format, r, g, b));
		return;
	}

	// Create an empty surface with 32 bits per pixel in video memory. This will
	// allow SDL to take advantage of video.
	surface = SDL_CreateRGBSurface(SDL_HWSURFACE, rect->w, rect->h, 32, 0, 0, 0, 0);

	// Perform a fast fill of the whole rectangle with color.
	SDL_FillRect(surface, NULL, SDL_MapRGB(Screen->format, r, g, b));

	// Adjust the alpha properties of a surface and active the acceleration.
	SDL_SetAlpha(surface, SDL_SRCALPHA | SDL_RLEACCEL, a);

	// Blit the surface on screen and free it.
	SDL_BlitSurface(surface, NULL, Screen, rect);
	SDL_FreeSurface(surface);
}

/**
 * This function draws a transparent black rectangle over a specified
 * area on the screen.
 */
void ShadowingRectangle(SDL_Surface * Surface, SDL_Rect Area)
{
	if (use_open_gl) {
		gl_draw_rectangle(&Area, 0, 0, 0, 150);
	} else {
		sdl_draw_rectangle(&Area, 0, 0, 0, 150);
	}
}

/**
 * This function draws a transparent white rectangle over a specified
 * area on the screen.
 */
void HighlightRectangle(SDL_Surface * Surface, SDL_Rect Area)
{
	if (use_open_gl) {
		gl_draw_rectangle(&Area, 255, 255, 255, 100);
	} else {
		sdl_draw_rectangle(&Area, 255, 255, 255, 100);
	}
}

/*
 * Draw an 'expanded' pixel.
 * Used to draw thick lines.
 */
static void draw_expanded_pixel(SDL_Surface * Surface, int x, int y, int xincr, int yincr, int color, int thickness)
{
	int i;

	PutPixel(Surface, x, y, color);

	if (thickness <= 1)
		return;
	for (i = x + xincr; i != x + thickness * xincr; i += xincr) {
		PutPixel(Surface, i, y, color);
	}
	for (i = y + yincr; i != y + thickness * yincr; i += yincr) {
		PutPixel(Surface, x, i, color);
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

	Uint32 color = SDL_MapRGB(Surface->format, r, g, b);

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
			draw_expanded_pixel(Surface, x1, y1, incr_x, incr_y, color, thickness);
			error_accum += delta_y;
			if (error_accum > delta_x) {
				error_accum -= delta_x;
				y1 += incr_y;
			}
			x1 += incr_x;
		}
		draw_expanded_pixel(Surface, x1, y1, incr_x, incr_y, color, thickness);
	} else {
		error_accum = delta_y >> 1;
		while (y1 != y2) {
			draw_expanded_pixel(Surface, x1, y1, incr_x, incr_y, color, thickness);
			error_accum += delta_x;
			if (error_accum > delta_y) {
				error_accum -= delta_y;
				x1 += incr_x;
			}
			y1 += incr_y;
		}
		draw_expanded_pixel(Surface, x1, y1, incr_x, incr_y, color, thickness);
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
#endif
}

void draw_line(float x1, float y1, float x2, float y2, Uint32 color, int width)
{
	float rr, gg, bb, zoom_factor;
	zoom_factor = (GameConfig.zoom_is_on ? lvledit_zoomfact_inv() : 1.0);
	rr = (color & 0xff0000) >> 16;
	gg = (color & 0xff00) >> 8;
	bb = color & 0xff;

	if (!use_open_gl) {
		draw_line_sdl(Screen, x1, y1, x2, y2, rr, gg, bb, width);
	} else {
		draw_line_opengl(x1, y1, x2, y2, rr, gg, bb, width);
	}
}

/*
 * Draw an horizontally hatched quad in SDL mode
 * 
 * Note : this function is not completely generic.
 * The 4 vertices must be in clockwise or anti-clockwise order.
 * 
 * Here we use floating computations. This could definitively be
 * improved with incremental methods
 */
void DrawHatchedQuad(SDL_Surface * Surface, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int r, int g, int b)
{
	if (use_open_gl)
		return;

	// Draw edges
	//
	draw_line_sdl(Surface, x1, y1, x2, y2, r, g, b, 1);
	draw_line_sdl(Surface, x2, y2, x3, y3, r, g, b, 1);
	draw_line_sdl(Surface, x3, y3, x4, y4, r, g, b, 1);
	draw_line_sdl(Surface, x4, y4, x1, y1, r, g, b, 1);

	// Reorder vertices, so that bottom-most is the first one
	//
	struct vertex {
		int x;
		int y;
	} vertices[4] = { {
	x1, y1}, {
	x2, y2}, {
	x3, y3}, {
	x4, y4}};
	int vindex[4] = { 0, 1, 2, 3 };

	/* Init */
	int min_index = 0;
	int y_min = y1;

	/* Check each vertex */
	if (y2 < y_min) {
		min_index = 1;
		y_min = y2;
	}
	if (y3 < y_min) {
		min_index = 2;
		y_min = y3;
	}
	if (y4 < y_min) {
		min_index = 3;
		y_min = y4;
	}

	/* Rotate indices, until vertices[min_index] is at 0 position */
	while (min_index != 0) {
		int tmp = vindex[0];
		vindex[0] = vindex[1];
		vindex[1] = vindex[2];
		vindex[2] = vindex[3];
		vindex[3] = tmp;

		--min_index;
	}

	// Store the 2 left edges, and the 2 right edges
	//
	// Note : since spans can be drawn from left to right or right to left
	// we do not take care of the direction of drawing.
	// So 'left' and 'right' below are pure conventional names.
	//
	struct edge {
		int x;
		int y;
		int ymax;
		int deltax;
		int deltay;
	} left_edges[2], right_edges[2];

	left_edges[0].x = vertices[vindex[0]].x;
	left_edges[0].y = vertices[vindex[0]].y;
	left_edges[0].ymax = vertices[vindex[1]].y;
	left_edges[0].deltax = (vertices[vindex[1]].x - vertices[vindex[0]].x);
	left_edges[0].deltay = (vertices[vindex[1]].y - vertices[vindex[0]].y);

	left_edges[1].x = vertices[vindex[1]].x;
	left_edges[1].y = vertices[vindex[1]].y;
	left_edges[1].ymax = vertices[vindex[2]].y;
	left_edges[1].deltax = (vertices[vindex[2]].x - vertices[vindex[1]].x);
	left_edges[1].deltay = (vertices[vindex[2]].y - vertices[vindex[1]].y);

	right_edges[0].x = vertices[vindex[0]].x;
	right_edges[0].y = vertices[vindex[0]].y;
	right_edges[0].ymax = vertices[vindex[3]].y;
	right_edges[0].deltax = (vertices[vindex[3]].x - vertices[vindex[0]].x);
	right_edges[0].deltay = (vertices[vindex[3]].y - vertices[vindex[0]].y);

	right_edges[1].x = vertices[vindex[3]].x;
	right_edges[1].y = vertices[vindex[3]].y;
	right_edges[1].ymax = vertices[vindex[2]].y;
	right_edges[1].deltax = (vertices[vindex[2]].x - vertices[vindex[3]].x);
	right_edges[1].deltay = (vertices[vindex[2]].y - vertices[vindex[3]].y);

	// Quad filling, by creating horizontal spans between the left and right edges
	// There is a 3 line step between two spans, to create hatches
	//
#define LEDGE left_edges[left_idx]
#define REDGE right_edges[right_idx]

	int left_idx = 0;	// Active left edge
	int right_idx = 0;	// Active right edge
	int y_curr;

	for (y_curr = left_edges[0].y + 3; y_curr < left_edges[1].ymax; y_curr += 3) {
		// Change active left edge, if needed
		if (LEDGE.ymax <= y_curr) {
			++left_idx;
		}
		// Change active right edge, if needed
		if (REDGE.ymax <= y_curr) {
			++right_idx;
		}
		// Start and end position of the span (this is the part that could
		// be improved with an incremental code
		float x_left = (float)LEDGE.x + (float)(y_curr - LEDGE.y) * (float)LEDGE.deltax / (float)LEDGE.deltay;
		float x_right = (float)REDGE.x + (float)(y_curr - REDGE.y) * (float)REDGE.deltax / (float)REDGE.deltay;

		// Draw the span
		draw_line_sdl(Surface, x_left, y_curr, x_right, y_curr, r, g, b, 1);
	}

#undef LEDGE
#undef REDGE
}

#undef _graphics_c
