/***********************************************************/
/*                                                         */
/*   BFONT.c v. 1.0.3 - Billi Font Library by Diego Billi  */
/*   Heavily modified for FreedroidRPG needs over years    */
/*                                                         */
/***********************************************************/

#define _bfont_c

#include "system.h"
#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"

/**
 * Initialize a font.
 */
static void find_character_positions(BFont_Info *font, SDL_Rect char_rect[MAX_CHARS_IN_FONT])
{
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
			if (sdl_get_pixel(font_surf, x, y) != sentry_horiz) {
				// Found a character
				rect = &char_rect[i];
				rect->x = x;
				rect->y = y;

				// Compute character width
				int x2 = x;
				while (x2 < font_surf->w) {
					if (sdl_get_pixel(font_surf, x2, y) == sentry_horiz)
						break;
					x2++;
				}
				rect->w = x2 - x;

				if (x2 == font_surf->w)
					break;

				// Compute character height
				int y2 = y;
				while (y2 < font_surf->h) {
					if (sdl_get_pixel(font_surf, x, y2) == sentry_horiz ||
							sdl_get_pixel(font_surf, x, y2) == sentry_vert)
						break;
					y2++;
				}
				rect->h = y2 - y;

				// Update maximal h
				if (max_h < y2 - y)
					max_h = y2 - y;

				// Move on to the next character
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

	// Set "space" character width
	char_rect[' '].w = char_rect['!'].w;
	char_rect[' '].h = char_rect['!'].h;
	
	// We assume a constant font height
	font->h = char_rect['!'].h;
	font->number_of_chars = i;
}


/**	
 * Prepare font for rendering: create struct image for each character in the font.
 */
static void prepare_font(BFont_Info *font, SDL_Rect char_rect[MAX_CHARS_IN_FONT])
{
	int i;

#ifdef HAVE_LIBGL
	if (use_open_gl) {
		make_texture_out_of_surface(&font->font_image);
	}
#endif

	// Create each char_image
	for (i = FIRST_FONT_CHAR; i < font->number_of_chars; i++) {
		struct image *img = &font->char_image[i];

		create_subimage(&font->font_image, img, &char_rect[i]);
	}
	
	// Space is a special case	
	create_subimage(&font->font_image, &font->char_image[' '], &char_rect[' ']);
	
	// Delete the now unneeded global bitmap
	free_image_surface(&font->font_image);
}

/**
 * Load the font and stores it in the BFont_Info structure 
 */
int load_bfont(const char *filepath, struct font *font)
{
	BFont_Info *bfont = MyMalloc(sizeof(BFont_Info));

	// Load the font_image->surface
	load_image_surface(&bfont->font_image, filepath, FALSE);

	// Find character coordinates in the image
	SDL_Rect char_rect[MAX_CHARS_IN_FONT];
	memset(char_rect, 0, sizeof(char_rect));
	find_character_positions(bfont, char_rect);

	// Prepare the data structures according to the rendering mode
	prepare_font(bfont, char_rect);

	// References the bfont data into the struct font
	font->bfont = bfont;
	font->height = bfont->h;

	return TRUE;
}

void free_bfont( struct font *font)
{
	int i;

	if (!font->bfont)
		return;

	for (i = FIRST_FONT_CHAR; i < font->bfont->number_of_chars; i++) {
		delete_image(&font->bfont->char_image[i]);
	}
	delete_image(&font->bfont->char_image[' ']);

	delete_image(&font->bfont->font_image);

	free(font->bfont);
	font->bfont = NULL;
}

/**
 * Return the width of specified character
 */
int font_char_width(struct font *font, unsigned char c)
{
	if (c < ' ' || c > font->bfont->number_of_chars - 1)
		c = '.';
	return font->bfont->char_image[c].w;
}

/**
 * Puts a single char on the surface with the specified font 
 */
int put_char(struct font *font, int x, int y, unsigned char c)
{
	SDL_Rect dest = {
		.w = font->bfont->char_image[' '].w,
		.h = font->bfont->h,
		.x = x,
		.y = y
	};

	if (c < ' ' || c > font->bfont->number_of_chars - 1)
		c = '.';

	if ((c != ' ') && (c != '\n')) {
		struct image *img = &font->bfont->char_image[c];
		SDL_Rect clipping_rect;
		SDL_GetClipRect(Screen, &clipping_rect);
		if ((dest.x >= clipping_rect.x) && (dest.x < clipping_rect.x + clipping_rect.w)) {
			display_image_on_screen(img, dest.x, dest.y, IMAGE_NO_TRANSFO);
		}
	}

	return font->bfont->char_image[c].w;
}

/**
 * Write a string on a surface using specified font, taking letter-spacing
 * into account.
 */
void put_string(struct font *font, int x, int y, const char *text)
{
	char *ptr = (char *)text;

	set_current_font(font);

#ifdef HAVE_LIBGL
	if (use_open_gl) {
		SDL_Rect clip_rect;
		SDL_GetClipRect(Screen, &clip_rect);
		set_gl_clip_rect(&clip_rect);
	}
#endif

	start_image_batch();

	int letter_spacing = get_letter_spacing(get_current_font());

	while (*ptr != '\0') {
		// handle_switch_font_char() can change the current font, so we need
		// to call get_current_font() at each step
		if (handle_switch_font_char(&ptr)) {
			letter_spacing = get_letter_spacing(get_current_font());
			continue;
		}
		x += put_char(get_current_font(), x, y, *ptr) + letter_spacing;
		ptr++;
	}

	end_image_batch();

#ifdef HAVE_LIBGL
	if (use_open_gl) {
		unset_gl_clip_rect();
	}
#endif
}

/**
 * Calculate the width of a string using a certain font, taking letter-spacing
 * into account.
 */
int text_width(struct font *font, const char *text)
{
	char *ptr = (char *)text;
	int width = 0;
	int letter_spacing = get_letter_spacing(font);

	while (*ptr != '\0') {
		if (handle_switch_font_char(&ptr)) {
			letter_spacing = get_letter_spacing(font);
			continue;
		}
		width += font_char_width(font, *ptr) + letter_spacing;
		ptr++;
	}
	return width;
}

/**
 *
 *
 */
int limit_text_width(struct font *font, const char *text, int limit)
{
	char *ptr = (char *)text;
	int width = 0;
	int letter_spacing = get_letter_spacing(font);

	while (*ptr != '\0') {
		if (handle_switch_font_char(&ptr)) {
			letter_spacing = get_letter_spacing(font);
			continue;
		}
		width += font_char_width(font, *ptr) + letter_spacing;
		ptr++;
		if (width >= limit)
			return (ptr - text);
	}
	return -1;
}

#undef _bfont_c
