/* 
 *
 *  Copyright (c) 2010 Arthur Huillet
 *                2015 Samuel Degrande
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

#include <math.h>
#include "codeset.h"

char *font_name;
char *codeset;
char *input_path;
int	sentry_horiz = 0;
int	sentry_vert = 0;

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

	atexit(SDL_Quit);
}

void PutPixel(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p;

	if ((x < 0) || (y < 0) || (x >= surface->w) || (y >= surface->h))
		return;

	/* Here p is the address to the pixel we want to set */
	p = (Uint8 *) surface->pixels + y * surface->pitch + x * bpp;

	switch (bpp) {
	case 1:
		*p = pixel;
		break;

	case 2:
		*(Uint16 *) p = pixel;
		break;

	case 3:
		// pixel = pixel & 0x0ffffff ;
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
			p[0] = (pixel >> 16) & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = pixel & 0xff;
		} else {
			p[0] = pixel & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = (pixel >> 16) & 0xff;
		}
		break;

	case 4:
		*(Uint32 *) p = pixel;
		break;
	}

};				// void PutPixel ( ... )

static void next_destination(SDL_Surface *font, int w, int *next_x, int *next_y, int *max_h)
{
	*next_x += w + 1;
	if (*next_x >= font->w) {
		// Reached limit on the right, start another line
		*next_x = 0;
		*next_y += *max_h;
		*max_h = 3; // Minimum height

		int a;
		for (a = 0; a < font->w; a++) {
			PutPixel(font, a, *next_y, sentry_vert);
		}

		(*next_y)++;
	}
}

static int pot(int v)
{
	float l2_v = log2f(v);
	return (int)pow(2, ceilf(l2_v));
}

void create_font()
{
	SDL_Surface *font = SDL_CreateRGBSurface(0, 512, 512, 32, rmask, gmask, bmask, amask);
	SDL_Surface *character;
	SDL_Surface empty_char = { .w = 1, .h = 1 };
	SDL_Rect target;
	int next_x = 0;
	int next_y = 0;
	int max_h = 0;
	unsigned int i = FIRST_FONT_CHAR;
			
	SDL_SetAlpha(font, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
	SDL_SetColorKey(font, 0, 0);

	sentry_horiz = SDL_MapRGBA(font->format, 255, 0, 255, 255);
	sentry_vert = SDL_MapRGBA(font->format, 0, 255, 0, 255);

	while (1) {
		int is_empty = 0;

		if (cs_code_is_empty(i, codeset)) {
			// 'empty' character size
			character = &empty_char;
			is_empty = 1;
		} else {
			// Open character PNG file
			char *file_name = cs_font_char_name(font_name, codeset, i, input_path);

			character = (SDL_Surface *)IMG_Load(file_name);
			if (!character) {
				fprintf(stderr, "Could not load %s: %s. Adding an empty char.\n", file_name, IMG_GetError());
				character = &empty_char;
				is_empty = 1;
			}
		}

		// Copy onto font surface
		int dummy = max_h;
		if (next_x + character->w >= font->w) {
			int a;
			for (a = next_x; a < font->w; a++) {
				PutPixel(font, a, next_y, sentry_horiz);
			}
			next_destination(font, character->w, &next_x, &next_y, &dummy);
		}

		target.x = next_x;
		target.y = next_y;

		if (!is_empty) {
			SDL_SetAlpha(character, 0, 255);
			SDL_SetColorKey(character, 0, 0);
			if (SDL_BlitSurface(character, NULL, font, &target))
				fprintf(stderr, "Error blitting character %c\n", i);
		}

		// Write sentry at the right
		PutPixel(font, target.x + character->w, target.y, sentry_horiz);

		// Update max h for this line
		if (max_h < character->h)
			max_h = character->h;

		i++;
		if (i > 255)
			break;

		// Compute next destination
		next_destination(font, character->w, &next_x, &next_y, &max_h);
	}

	if (i != '!') {
		int a;
		for (a = 0; a < font->w; a++) {
			PutPixel(font, a, next_y + max_h, sentry_vert);
		}
		printf("Glued %d characters\n", i - '!' - 1);
		// Hack - change the SDL_Surface's height attribute to save only
		// the useful part of the image
		font->h = pot(next_y + max_h + 1);
		png_save_surface(font_name, font);
		font->h = 512;
	} else {
		printf("No characters were glued, therefore no output was produced.\n");
	}

	SDL_FreeSurface(font);

}

int main(int argc, char **argv)
{
	if (argc != 4) {
		fprintf(stderr, "Usage: %s <font_file> <codeset> <input_path>\n"
		                "\t<font_file>:   path of the bitmap file to create, relative to current directory.\n"
		                "\t<codeset>:     codeset of the bitmap file to create (see list below)\n"
		                "\t<input_path>:  path of the directory from where to read the individuals bitmaps, relative to current directory\n"
		                "\n"
		                "Example, from fdrpg top src dir: %s graphics/font/ISO-8859-15/cpuFont.png ISO-8859-15 graphics/font/chars\n"
		                "\n"
		                "Available codesets: %s\n", argv[0], argv[0], cs_available_codesets());
		exit(1);
	}

	font_name = argv[1];
	codeset = argv[2];
	cs_check_codeset(codeset);
	input_path = argv[3];

	init_sdl();

	printf("Gluing characters in %s to create font %s. ", input_path, font_name);

	create_font();
	return 0;
}
