/************************************************************/
/*                                                          */
/*   BFONT.h v. 1.0.3 - Billi Font Library by Diego Billi   */
/*                                                          */
/************************************************************/

#ifndef _BFont_h
#define _BFont_h

#include "system.h"
#include "struct.h"

#define MAX_CHARS_IN_FONT 256

typedef struct BFont_Info {
	/* font height */
	int h;
	
	struct image font_image;
	struct image char_image[MAX_CHARS_IN_FONT];
	unsigned int number_of_chars;
} BFont_Info;

/* Load and store le font in the BFont_Info structure */
int load_bfont(const char *filename, struct font *font);

/* Returns the character width of the specified font */
int font_char_width(struct font *font, unsigned char c);

/* Write a single character on the screen with the specified font */
int put_char(struct font *font, int x, int y, unsigned char c);

/* Returns the width, in pixels, of the text calculated with the specified font*/
int text_width(struct font *font, const char *text);

/* Returns the index of the last character than is inside the width limit, with the specified font */
int limit_text_width(struct font *font, const char *text, int limit);

/* Write a string on the screen with the specified font */
void put_string(struct font *font, int x, int y, const char *text);

#endif // _BFont_h
