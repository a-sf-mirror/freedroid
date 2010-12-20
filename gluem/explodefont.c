/* 
 *
 *  Copyright (c) 2010 Arthur Huillet
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

#include "pngfuncs.h"
#include "../src/system.h"
#include "../src/defs.h"
#include "../src/getopt.h"
#include "../src/struct.h"
#include "../src/proto.h"
#include "../src/BFont.h"

char *font_name;
const char *output_path;

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
static int rmask = 0x00FF0000;
static int gmask = 0x0000FF00;
static int bmask = 0x000000FF;
static int amask = 0xFF000000;
#else
static int rmask = 0x0000FF00;
static int gmask = 0x00FF0000;
static int bmask = 0xFF000000;
static int amask = 0x000000FF;
#endif


int CharWidth(BFont_Info * Font, unsigned char c)
{
    if (c < ' ' || c > Font->number_of_chars - 1)
	        c = '.';
    return Font->Chars[c].w;
}

static void init_sdl(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
		exit(1);
	}

	if (SDL_InitSubSystem(SDL_INIT_TIMER) == -1) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

//	SDL_SetVideoMode(640, 480, 0, 0);
	atexit(SDL_Quit);
}

Uint32 FdGetPixel(SDL_Surface * Surface, Sint32 X, Sint32 Y)
{

	Uint8 *bits;
	Uint32 Bpp;

	Bpp = Surface->format->BytesPerPixel;

	bits = ((Uint8 *) Surface->pixels) + Y * Surface->pitch + X * Bpp;

	// Get the pixel
	switch (Bpp) {
		case 1:
			return *((Uint8 *) Surface->pixels + Y * Surface->pitch + X);
			break;
		case 2:
			return *((Uint16 *) Surface->pixels + Y * Surface->pitch / 2 + X);
			break;
		case 3:
				{       // Format/endian independent
				Uint8 r, g, b;
				r = *((bits) + Surface->format->Rshift / 8);
				g = *((bits) + Surface->format->Gshift / 8);
				b = *((bits) + Surface->format->Bshift / 8);
				return SDL_MapRGB(Surface->format, r, g, b);
				}
			break;
		case 4:
			return *((Uint32 *) Surface->pixels + Y * Surface->pitch / 4 + X);
			break;
	}

	return -1;
}

static void InitFont(BFont_Info * Font)
{
	unsigned int x = 0, i = 0;
	Uint32 sentry;
	SDL_Surface *tmp_char1;

	Font->h = Font->Surface->h;

	i = '!';
	sentry = SDL_MapRGB(Font->Surface->format, 255, 0, 255);

	if (SDL_MUSTLOCK(Font->Surface))
		SDL_LockSurface(Font->Surface);

	x = 0;
	while (x < (Font->Surface->w - 1) && i < MAX_CHARS_IN_FONT) {
		if (FdGetPixel(Font->Surface, x, 0) != sentry) {
			Font->Chars[i].x = x;
			Font->Chars[i].y = 1;
			Font->Chars[i].h = Font->Surface->h;
			while (x < (Font->Surface->w)) {
				if (FdGetPixel(Font->Surface, x, 0) == sentry)
					break;
				x++;
			}
			Font->Chars[i].w = (x - Font->Chars[i].x);

			Font->number_of_chars = i + 1;

			tmp_char1 = SDL_CreateRGBSurface(0, CharWidth(Font, i), Font->h - 1, 32, rmask, gmask, bmask, amask);
			Font->char_image[i].surface = tmp_char1;

			SDL_BlitSurface(Font->Surface, &(Font->Chars[i]), Font->char_image[i].surface, NULL);
			SDL_SetAlpha(Font->char_image[i].surface, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
			SDL_SetColorKey(Font->char_image[i].surface, 0, 0);

			char name[4096];
			sprintf(name, "%s/%s_%03d.png", output_path, basename(font_name), i);
			printf("Saving %s\n", name);
			png_save_surface(name, Font->char_image[i].surface);

			SDL_FreeSurface(tmp_char1);
			i++;
		} else {
			x++;
		}

	}
	if (SDL_MUSTLOCK(Font->Surface))
		SDL_UnlockSurface(Font->Surface);

}

/**
 * Load the font and stores it in the BFont_Info structure 
 */
BFont_Info *LoadFont(const char *filename)
{
	int x;
	BFont_Info *Font = NULL;
	SDL_Surface *tmp = NULL;

	Font = (BFont_Info *) calloc(1, sizeof(BFont_Info));
	tmp = (SDL_Surface *) IMG_Load(font_name);

	if (tmp != NULL) {
		Font->Surface = tmp;
		for (x = 0; x < 256; x++) {
			Font->Chars[x].x = 0;
			Font->Chars[x].y = 0;
			Font->Chars[x].h = 0;
			Font->Chars[x].w = 0;
		}
		SDL_SetAlpha(Font->Surface, 0, 255);
		SDL_SetColorKey(Font->Surface, 0, 0);
		/* Init the font */
		InitFont(Font);

	} else {
		fprintf(stderr, "Loading font \"%s\":%s\n", filename, IMG_GetError());
		/* free memory allocated for the BFont_Info structure */
		free(Font);
		exit(1);
	}

	return NULL;
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <font_file> <output_path>\n", argv[0]);
		exit(1);
	}

	font_name = argv[1];
	output_path = argv[2];
	

	init_sdl();

	printf("Exploding font %s into %s\n", font_name, output_path);
	LoadFont(NULL);

	return 0;
}
