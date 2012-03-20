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

#include "../src/pngfuncs.h"
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

#define FIRST_FONT_CHAR '!'

static void InitFont(BFont_Info *font)
{
	SDL_Rect char_rect[MAX_CHARS_IN_FONT];
	memset(char_rect, 0, sizeof(char_rect));
	SDL_Surface *tmp_char1;

	unsigned int x = 0, i = 0, y = 0, max_h = 1;
	SDL_Rect *rect;
	SDL_Surface *font_surf = font->font_image.surface;

	i = FIRST_FONT_CHAR;
	int sentry_horiz = SDL_MapRGB(font_surf->format, 255, 0, 255);
	int sentry_vert = SDL_MapRGB(font_surf->format, 0, 255, 0);

	if (SDL_MUSTLOCK(font_surf))
		SDL_LockSurface(font_surf);

	while (1) {
		if (i == MAX_CHARS_IN_FONT)
			break;

		// Read this line of characters
		while (x < font_surf->w - 1 && i < MAX_CHARS_IN_FONT) {
			if (FdGetPixel(font_surf, x, y) != sentry_horiz) {
				printf("Reading character at %d %d...", x, y);
				// Found a character
				rect = &char_rect[i];
				rect->x = x;
				rect->y = y;

				// Compute character width
				int x2 = x;
				while (x2 < font_surf->w) {
					if (FdGetPixel(font_surf, x2, y) == sentry_horiz)
						break;
					x2++;
				}
				rect->w = x2 - x;

				printf("width %d\n", x2-x);
				if (x2 == font_surf->w)
					break;

				// Compute character height
				int y2 = y;
				while (y2 < font_surf->h) {
					if (FdGetPixel(font_surf, x, y2) == sentry_horiz ||
							FdGetPixel(font_surf, x, y2) == sentry_vert)
						break;
					y2++;
				}
				rect->h = y2 - y;

				// Update maximal h
				if (max_h < y2 - y)
					max_h = y2 - y;

				// Create character surface
				tmp_char1 = SDL_CreateRGBSurface(0, char_rect[i].w, char_rect[i].h, 32, rmask, gmask, bmask, amask);
				font->char_image[i].surface = tmp_char1;

				SDL_BlitSurface(font_surf, &(char_rect[i]), font->char_image[i].surface, NULL);
				SDL_SetAlpha(font->char_image[i].surface, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
				SDL_SetColorKey(font->char_image[i].surface, 0, 0);

				char name[4096];
				sprintf(name, "%s/%s_%03d.png", output_path, basename(font_name), i);
				printf("Saving %s\n", name);
				png_save_surface(name, font->char_image[i].surface);

				SDL_FreeSurface(tmp_char1);
				font->number_of_chars = i + 1;
				i++;
				x = x2;
			} else {
				// On a sentry? Move right.
				x++;
			}
		}

		// Find the next line of characters
		y += max_h + 1;
		max_h = 1;
		x = 0;

		if (y >= font_surf->h)
			break;	

	}
	if (SDL_MUSTLOCK(font_surf))
		SDL_UnlockSurface(font_surf);

}

/**
 * Load the font and stores it in the BFont_Info structure 
 */
static void explode_font()
{
	BFont_Info *Font = NULL;
	SDL_Surface *tmp = NULL;

	Font = (BFont_Info *) calloc(1, sizeof(BFont_Info));
	tmp = (SDL_Surface *) IMG_Load(font_name);

	if (tmp != NULL) {
		Font->font_image.surface = tmp;
		SDL_SetAlpha(Font->font_image.surface, 0, 255);
		SDL_SetColorKey(Font->font_image.surface, 0, 0);
		/* Init the font */
		InitFont(Font);

	} else {
		fprintf(stderr, "Loading font \"%s\":%s\n", font_name, IMG_GetError());
		/* free memory allocated for the BFont_Info structure */
		free(Font);
		exit(1);
	}

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
	explode_font();

	return 0;
}
