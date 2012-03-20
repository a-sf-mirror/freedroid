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
char *input_path;

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
	int	sentry_vert = SDL_MapRGBA(font->format, 0, 255, 0, 255);

	*next_x += w + 1;
	if (*next_x >= font->w) {
		// Reached limit on the right, start another line
		*next_x = 0;
		*next_y += *max_h;
		*max_h = 0;

		int a;
		for (a = 0; a < font->w; a++) {
			PutPixel(font, a, *next_y, sentry_vert);
		}

		(*next_y)++;
	}
}

void create_font()
{
	SDL_Surface *font = SDL_CreateRGBSurface(0, 1024, 512, 32, rmask, gmask, bmask, amask);
	SDL_Surface *character;
	SDL_Rect target;
	int next_x = 0;
	int next_y = 0;
	int max_h = 0;
	int	sentry_horiz = SDL_MapRGBA(font->format, 255, 0, 255, 255);

	char i = '!'; //start character for FreedroidRPG fonts
			
	SDL_SetAlpha(font, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
	SDL_SetColorKey(font, 0, 0);

	while (1) {
			// Open character PNG file
			char name[4096];
			sprintf(name, "%s/%s_%03d.png", input_path, basename(font_name), i);
			//printf("Reading %s\n", name);

			character = (SDL_Surface *)IMG_Load(name);
			if (!character) {
				fprintf(stderr, "Could not load %s: %s\n", name, IMG_GetError());
				break;
			}
			
			SDL_SetAlpha(character, 0, 255);;
			SDL_SetColorKey(character, 0, 0);

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
			if (SDL_BlitSurface(character, NULL, font, &target)) {
				fprintf(stderr, "Error blitting character %c\n", i);
			}

			// Write sentry at the right and at the bottom
			PutPixel(font, target.x + character->w, target.y, sentry_horiz);
			PutPixel(font, target.x, target.y + character->h, sentry_horiz);

			// Update max h for this line
			if (max_h < character->h)
				max_h = character->h;

			// Compute next destination
			next_destination(font, character->w, &next_x, &next_y, &max_h);

			i++;

			if (i >= 255)
				break;
	}

	if (i != '!') {
		printf("Glued %d characters\n", i - '!' - 1);
		png_save_surface(font_name, font);
	} else {
		printf("No characters were glued, therefore no output was produced.\n");
	}

	SDL_FreeSurface(font);

}

int main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <font_file> <input_path>\n", argv[0]);
		exit(1);
	}

	font_name = argv[1];
	input_path = argv[2];
	

	init_sdl();

	printf("Gluing characters in %s to create font %s\n", input_path, font_name);

	create_font();
	return 0;
}
