/*
 *
 *  Copyright (c) 2015 Samuel Degrande
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
#include <SDL/SDL_ttf.h>
#include "codeset.h"

char *ttf_file;
char *font_name;
char *codeset;
int font_size;
SDL_Color font_color = { 0, 0, 0 };
int outlined;
char *input_path;

static void extract_font_color(char *color_triplet)
{
	if (sscanf(color_triplet, "%hhu:%hhu:%hhu", &font_color.r, &font_color.g, &font_color.b) != 3) {
		fprintf(stderr, "Couldn't extract colors from \"%s\"\n.", color_triplet);
		exit(1);
	}
}

static void init_sdl(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
	    fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);

	if (TTF_Init() == -1) {
	    fprintf(stderr, "Couldn't initialize TTF: %s\n", TTF_GetError());
	    exit(1);
	}
	atexit(TTF_Quit);

	if (SDL_InitSubSystem(SDL_INIT_TIMER) == -1) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
}

static Uint32 FdGetPixel(SDL_Surface * Surface, Sint32 X, Sint32 Y)
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

static void fix_rect(SDL_Rect *rect, TTF_Font *outline_font, uint16_t glyph)
{
	// The outline glyph bitmap can be shifted compared to the plain glyph bitmap,
	// implying a needed extra-shift for the plain glyph.
	// To get the shift, we use TTF_RenderGlyph, which does not apply the minx
	// and miny metrics when rendering the glyph.

	int shiftx = 0;
	int shifty = 0;
	int x, y;

	SDL_Color color = { 0, 0, 0 };
	SDL_Surface *surf = TTF_RenderGlyph_Blended(outline_font, glyph, color);
	if (!surf) {
		fprintf(stderr, "Couldn't render glyph of code %hu: %s\n", glyph, TTF_GetError());
		exit(1);
	}

	for (x = 0; x < surf->w; x++) {
		shiftx++;
		for (y = 0; y < surf->h; y++) {
			if (FdGetPixel(surf, x, y) != 0) {
				shiftx--;
				goto NEXT;
			}
		}
	}

NEXT:
	for (y = 0; y < surf->h; y++) {
		shifty++;
		for (x = 0; x < surf->w; x++) {
			if (FdGetPixel(surf, x, y) != 0) {
				shifty--;
				goto DONE;
			}
		}
	}

DONE:
	rect->x += shiftx;
	rect->y += shifty;

	SDL_FreeSurface(surf);
}

static void create_bmfont()
{
	TTF_Font *font = TTF_OpenFont(ttf_file, font_size);
	if (!font) {
		fprintf(stderr, "Couldn't load font %s: %s\n", ttf_file, TTF_GetError());
		exit(1);
	}
	TTF_SetFontHinting(font, TTF_HINTING_MONO);

	int i;
	for (i = 0; i <= 255; i++) {
		if (!cs_code_is_empty(i, codeset)) {
			uint16_t glyphs[] = { cs_code_to_unicode(i, codeset), 0 };

			// We use RenderUNICODE, in order to apply the glyph metrics
			SDL_Surface *surf = TTF_RenderUNICODE_Blended(font, glyphs, font_color);
			if (!surf) {
				fprintf(stderr, "Couldn't render glyph of code %d: %s\n", i, TTF_GetError());
				exit(1);
			}

			char *bm_name = cs_font_char_name(font_name, codeset, i, input_path);
			png_save_surface(bm_name, surf);
			if (surf) SDL_FreeSurface(surf);
		}
	}

	TTF_CloseFont(font);
}

static void create_outlined_bmfont()
{
	const int outline_width = 1;

	// To create an outlined font, we need to superpose 3 bitmaps:
	// 1- The RGBA bitmap of the blended outline (in black) - called 'outsurf'
	// 2- THE RGB bitmap of the plain glyph (in black, without alpha) - called 'bgsurf'
	// 3- The RGBA bitmap of the blended colored plain glyph - called 'surf'

	TTF_Font *font = TTF_OpenFont(ttf_file, font_size);
	if (!font) {
		fprintf(stderr, "Couldn't load font %s: %s\n", ttf_file, TTF_GetError());
		exit(1);
	}
	TTF_SetFontHinting(font, TTF_HINTING_MONO);

	TTF_Font *outline_font = TTF_OpenFont(ttf_file, font_size);
	TTF_SetFontHinting(outline_font, TTF_HINTING_MONO);
	TTF_SetFontOutline(outline_font, outline_width);

	SDL_Color bgcolor = { 0, 0, 0 };

	int i;
	for (i = 0; i <= 255; i++) {
		if (!cs_code_is_empty(i, codeset)) {
			uint16_t glyphs[] = { cs_code_to_unicode(i, codeset), 0 };

			// We use RenderUNICODE, in order to apply the glyph metrics
			SDL_Surface *surf = TTF_RenderUNICODE_Blended(font, glyphs, font_color);
			SDL_Surface *bgsurf = TTF_RenderUNICODE_Solid(font, glyphs, bgcolor);
			SDL_Surface *outsurf = TTF_RenderUNICODE_Blended(outline_font, glyphs, bgcolor);

			if (!surf || !bgsurf) {
				fprintf(stderr, "Couldn't render glyph of code %d: %s\n", i, TTF_GetError());
				exit(1);
			}
			if (!outsurf) {
				fprintf(stderr, "Couldn't render outline glyph of code %d: %s\n", i, TTF_GetError());
				exit(1);
			}

			SDL_Rect rect = { outline_width, outline_width, surf->w, surf->h };
			fix_rect(&rect, outline_font, cs_code_to_unicode(i, codeset));
			SDL_BlitSurface(bgsurf, NULL,outsurf, &rect);
			SDL_BlitSurface(surf, NULL, outsurf, &rect);

			char *bm_name = cs_font_char_name(font_name, codeset, i, input_path);
			png_save_surface(bm_name, outsurf);
			if (surf) SDL_FreeSurface(surf);
			if (bgsurf) SDL_FreeSurface(bgsurf);
			if (outsurf) SDL_FreeSurface(outsurf);
		}
	}

	TTF_CloseFont(outline_font);
	TTF_CloseFont(font);
}

int main(int argc, char **argv)
{
	if (argc != 8) {
		fprintf(stderr, "Usage: %s <ttf_file> <font_name> <codeset> <size> <red:green:blue> <outlined> <output_path>\n"
		                "\t<ttf_file>       : path to the TTF used to create the glyph bitmaps\n"
		                "\t<font_name>      : font name (prefix of the glyph files)\n"
		                "\t<codeset>        : codeset of the glyph bitmaps to create (see list below)\n"
		                "\t<size>           : glyph height, in pixels\n"
		                "\t<red:green:blue> : base foreground color of the glyph bitmaps\n"
		                "\t<outlined>       : 0=plain, 1=plain+outline\n"
		                "\t<output_path>    : directory to store the generated files (must exist)\n"
		                "\n"
		                "Example, to create the small_blue glyph bitmaps:\n"
		                "     %s /usr/share/fonts/DejaVuSans.ttf small_blue ISO-8859-15 11 187:187:245 0 small_blue_chars\n"
		                "To then create the small_blue.png:\n"
		                "     gluefont graphics/font/ISO-8859-15/small_blue.png ISO-8859-15 small_blue_chars\n"
		                "\n"
		                "Available codesets: %s\n", argv[0], argv[0], cs_available_codesets());
		exit(1);
	}

	ttf_file = argv[1];
	font_name = argv[2];
	codeset = argv[3];
	cs_check_codeset(codeset);
	font_size = atoi(argv[4]);
	extract_font_color(argv[5]);
	outlined = atoi(argv[6]);
	input_path = argv[7];

	init_sdl();

	if (outlined == 1)
		create_outlined_bmfont();
	else
		create_bmfont();

	return 0;
}
