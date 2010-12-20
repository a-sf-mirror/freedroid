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
#include "SDL_rotozoom.h"

/* Current font */
static BFont_Info *CurrentFont;

#define FIRST_FONT_CHAR '!'

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
			if (FdGetPixel(font_surf, x, y) != sentry_horiz) {
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

	if (use_open_gl) {
		make_texture_out_of_surface(&font->font_image);
	}

	// Create each char_image
	for (i = FIRST_FONT_CHAR; i < font->number_of_chars; i++) {
		struct image *img = &font->char_image[i];

		create_subimage(&font->font_image, img, &char_rect[i]);
	}
	
	// Space is a special case	
	create_subimage(&font->font_image, &font->char_image[' '], &char_rect[' ']);
	
	// Delete font_image (already done if using OpenGL)
	if (!use_open_gl) {
		SDL_FreeSurface(font->font_image.surface);
		font->font_image.surface = NULL;
	}
}

/**
 * Load the font and stores it in the BFont_Info structure 
 */
BFont_Info *LoadFont(char *filename)
{
	BFont_Info *font = MyMalloc(sizeof(BFont_Info));

	// Load the font image
	SDL_Surface *tmp = (SDL_Surface *) IMG_Load(filename);
	if (!tmp) {
		fprintf(stderr, "Loading font \"%s\":%s\n", filename, IMG_GetError());
		free(font);
		return NULL;
	}
	font->font_image.surface = SDL_DisplayFormatAlpha(tmp);
	font->font_image.w = font->font_image.surface->w;
	font->font_image.h = font->font_image.surface->h;
	SDL_SetAlpha(font->font_image.surface, 0, 255);
	SDL_SetColorKey(font->font_image.surface, 0, 0);
	SDL_FreeSurface(tmp);

	// Find character coordinates in the image
	SDL_Rect char_rect[MAX_CHARS_IN_FONT];
	memset(char_rect, 0, sizeof(char_rect));
	find_character_positions(font, char_rect);

	// Prepare the data structures according to the rendering mode
	prepare_font(font, char_rect);	
	
	// XXX should not be here
	SetCurrentFont(font);

	return font;
}

/**
 * Set the current font 
 */
void SetCurrentFont(BFont_Info * Font)
{
	CurrentFont = Font;
};				// void SetCurrentFont (BFont_Info * Font)

/**
 * Returns the pointer to the current font strucure in use 
 */
BFont_Info *GetCurrentFont(void)
{
	return CurrentFont;
};				// BFont_Info * GetCurrentFont (void)

/**
 * Return the font height 
 */
int FontHeight(BFont_Info * Font)
{
	return (Font->h);
};				// int FontHeight (BFont_Info * Font)

/**
 * Return the width of specified character
 */
int CharWidth(BFont_Info * Font, unsigned char c)
{
	if (c < ' ' || c > Font->number_of_chars - 1)
		c = '.';
	return Font->char_image[c].w;
}

/**
 * Get letter-spacing for specified font.
 *
 * Letter-spacing refers to the overall spacing of a word or block of text
 * affecting its overall density and texture.
 */
int get_letter_spacing(BFont_Info *font) {
	if (font == FPS_Display_BFont || font == Blue_BFont || font == Red_BFont)
		return -2;
	else if (font == Menu_BFont)
		return -4;
	else
		return 0;
}

/**
 * Handle font switching on special characters. Returns 1 if the font was
 * changed and 0 if it was not.
 */
int handle_switch_font_char(unsigned char c)
{
	if (c == font_switchto_red[0]) {
		SetCurrentFont(Red_BFont);
		return TRUE;
	} else if (c == font_switchto_blue[0]) {
		SetCurrentFont(Blue_BFont);
		return TRUE;
	} else if (c == font_switchto_neon[0]) {
		SetCurrentFont(FPS_Display_BFont);
		return TRUE;
	} else if (c == font_switchto_msgstat[0]) {
		SetCurrentFont(Messagestat_BFont);
		return TRUE;
	} else if (c == font_switchto_msgvar[0]) {
		SetCurrentFont(Messagevar_BFont);
		return TRUE;
	}
	return FALSE;
}

/**
 * Puts a single char on the surface with the specified font 
 */
int PutCharFont(SDL_Surface * Surface, BFont_Info * Font, int x, int y, unsigned char c)
{
	SDL_Rect dest;
	SDL_Rect clipping_rect;
	struct image *img;

	dest.w = CharWidth(Font, ' ');
	dest.h = FontHeight(Font);
	dest.x = x;
	dest.y = y;

	if (c < ' ' || c > Font->number_of_chars - 1)
		c = '.';

	img = &Font->char_image[c];

	if ((c != ' ') && (c != '\n')) {
		if (use_open_gl) {
	

			SDL_GetClipRect(Surface, &clipping_rect);

			if ((dest.x < clipping_rect.x + clipping_rect.w) && (dest.x >= clipping_rect.x)) {
#ifdef HAVE_LIBGL
				// We need to simulate the clipping as would be done by SDL.
				// We simply do "all or none" clipping for left and right, but for the top/bottom of the screen,
				// we use GL to do proper clipping in order not to see a whole line disappear at once.

				GLdouble eqntop[4] = { 0.0, 1.0, 0.0, -clipping_rect.y };
				GLdouble eqnbottom[4] = { 0.0, -1.0, 0.0, clipping_rect.y + clipping_rect.h };
				glClipPlane(GL_CLIP_PLANE0, eqntop);
				glEnable(GL_CLIP_PLANE0);
				glClipPlane(GL_CLIP_PLANE1, eqnbottom);
				glEnable(GL_CLIP_PLANE1);
				
				display_image_on_screen(img, dest.x, dest.y);
				glDisable(GL_CLIP_PLANE0);
				glDisable(GL_CLIP_PLANE1);
#endif

			}
		} else {
			display_image_on_screen(img, dest.x, dest.y);
		}
	}

	return CharWidth(Font, c);

};				// int PutCharFont (SDL_Surface * Surface, BFont_Info * Font, int x, int y, int c)

/**
 *
 */
void PutString(SDL_Surface *surface, int x, int y, char *text)
{
	int i = 0;

	while (text[i] != '\0') {
		int letter_spacing = get_letter_spacing(GetCurrentFont());
		if (!handle_switch_font_char(text[i])) {
			x += PutCharFont(surface, GetCurrentFont(), x, y, text[i]) + letter_spacing;
		}
		i++;
	}
}

/**
 * Write a string on a surface using specified font, taking letter-spacing
 * into account.
 */
void PutStringFont(SDL_Surface *surface, BFont_Info *font, int x, int y, char *text)
{
	SetCurrentFont(font);
	PutString(surface, x, y, text);
}

/**
 *
 *
 */
int TextWidth(char *text)
{
	return TextWidthFont(CurrentFont, text);
}

/**
 * Calculate the width of a string using a certain font, taking letter-spacing
 * into account.
 */
int TextWidthFont(BFont_Info *font, char *text)
{
	int i = 0, width = 0;
	int letter_spacing = get_letter_spacing(font);
	while (text[i] != '\0') {
		if (handle_switch_font_char(text[i]))
			letter_spacing = get_letter_spacing(font);
		else
			width += CharWidth(font, text[i]) + letter_spacing;
		i++;
	}
	return width;
}

/**
 *
 *
 */
static int LimitTextWidthFont(BFont_Info *font, char *text, int limit)
{
	int i = 0, width = 0;
	int letter_spacing = get_letter_spacing(font);
	while (text[i] != '\0') {
		if (handle_switch_font_char(text[i])) {
			letter_spacing = get_letter_spacing(font);
			i++;
			continue;
		}
		width += CharWidth(font, text[i]) + letter_spacing;
		i++;
		if (width >= limit)
			return i;
	}
	return -1;
}

/**
 *
 *
 */
int LimitTextWidth(char *text, int limit)
{
	return (LimitTextWidthFont(CurrentFont, text, limit));
}

/* counts the spaces of the strings */
int count(char *text)
{
	char *p = NULL;
	int pos = -1;
	int i = 0;
	/* Calculate the space occupied by the text without spaces */
	while ((p = strchr(&text[pos + 1], ' ')) != NULL) {
		i++;
		pos = p - text;
	}
	return i;
}

void JustifiedPutString(SDL_Surface * Surface, int y, char *text)
{
	JustifiedPutStringFont(Surface, CurrentFont, y, text);
}

void JustifiedPutStringFont(SDL_Surface * Surface, BFont_Info * Font, int y, char *text)
{
	int spaces = 0;
	int gap;
	int single_gap;
	int dif;

	char *strtmp;
	char *p;
	int pos = -1;
	int xpos = 0;

	if (strchr(text, ' ') == NULL) {
		PutStringFont(Surface, Font, 0, y, text);
	} else {
		gap = (Surface->w - 1) - TextWidthFont(Font, text);

		if (gap <= 0) {
			PutStringFont(Surface, Font, 0, y, text);
		} else {
			spaces = count(text);
			dif = gap % spaces;
			single_gap = (gap - dif) / spaces;
			xpos = 0;
			pos = -1;
			while (spaces > 0) {
				p = strstr(&text[pos + 1], " ");
				strtmp = NULL;
				strtmp = (char *)calloc((p - &text[pos + 1]) + 1, sizeof(char));
				if (strtmp != NULL) {
					strncpy(strtmp, &text[pos + 1], (p - &text[pos + 1]));
					PutStringFont(Surface, Font, xpos, y, strtmp);
					xpos = xpos + TextWidthFont(Font, strtmp) + single_gap + CharWidth(Font, ' ');
					if (dif >= 0) {
						xpos++;
						dif--;
					}
					pos = p - text;
					spaces--;
					free(strtmp);
				}
			}
			strtmp = NULL;
			strtmp = (char *)calloc(strlen(&text[pos + 1]) + 1, sizeof(char));

			if (strtmp != NULL) {
				strncpy(strtmp, &text[pos + 1], strlen(&text[pos + 1]));
				PutStringFont(Surface, Font, xpos, y, strtmp);
				free(strtmp);
			}
		}
	}
}

void CenteredPutString(SDL_Surface * Surface, int y, char *text)
{
	CenteredPutStringFont(Surface, CurrentFont, y, text);
}

void CenteredPutStringFont(SDL_Surface * Surface, BFont_Info * Font, int y, char *text)
{
	PutStringFont(Surface, Font, Surface->w / 2 - TextWidthFont(Font, text) / 2, y, text);
}

void RightPutString(SDL_Surface * Surface, int y, char *text)
{
	RightPutStringFont(Surface, CurrentFont, y, text);
}

void RightPutStringFont(SDL_Surface * Surface, BFont_Info * Font, int y, char *text)
{
	PutStringFont(Surface, Font, Surface->w - TextWidthFont(Font, text) - 1, y, text);
}

void LeftPutString(SDL_Surface * Surface, int y, char *text)
{
	LeftPutStringFont(Surface, CurrentFont, y, text);
}

void LeftPutStringFont(SDL_Surface * Surface, BFont_Info * Font, int y, char *text)
{
	PutStringFont(Surface, Font, 0, y, text);
}

/******/

void PrintString(SDL_Surface * Surface, int x, int y, const char *fmt, ...)
{
	va_list args;
	char *temp;
	va_start(args, fmt);

	if ((temp = (char *)malloc(1000 + 1)) != NULL) {
		vsprintf(temp, fmt, args);

		PutStringFont(Surface, CurrentFont, x, y, temp);

		free(temp);
	}
	va_end(args);
}

/*********************************************************************************************************/
/*********************************************************************************************************/
/*********************************************************************************************************/

inline void PutPixel32(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
	Uint8 *p;
	p = (Uint8 *) surface->pixels + y * surface->pitch + x * 4;

	*(Uint32 *) p = pixel;

}				// void PutPixel ( ... )

void PutPixel(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p;

	if (use_open_gl) {
		if (surface == Screen) {
			PutPixel_open_gl(x, y, pixel);
			return;
		}
	}

	// Here I add a security query against segfaults due to writing
	// perhaps even far outside of the surface pixmap data.
	//
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

/**
 * NOTE:  I THINK THE SURFACE MUST BE LOCKED FOR THIS!
 *
 */
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
		{		// Format/endian independent
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

#undef _bfont_c
