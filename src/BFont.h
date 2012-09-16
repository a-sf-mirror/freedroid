/************************************************************/
/*                                                          */
/*   BFONT.h v. 1.0.3 - Billi Font Library by Diego Billi   */
/*                                                          */
/************************************************************/

#include "system.h"

#define MAX_CHARS_IN_FONT 256

typedef struct BFont_Info {
	/* font height */
	int h;
	
	struct image font_image;
	struct image char_image[MAX_CHARS_IN_FONT];
	unsigned int number_of_chars;
} BFont_Info;

/* Load and store le font in the BFont_Info structure */
BFont_Info *LoadFont(const char *filename);

/* Returns a pointer to the current font structure */
BFont_Info *GetCurrentFont(void);

/* Set the current font */
void SetCurrentFont(BFont_Info * Font);

/* Returns the font height */
int FontHeight(BFont_Info * Font);

/* Returns the character width of the specified font */
int CharWidth(BFont_Info * Font, unsigned char c);

/* Get letter-spacing for specified font. */
int get_letter_spacing(BFont_Info *font);

/* Handle font switching on special characters. */
int handle_switch_font_char(unsigned char c);

/* Write a single character on the "Surface" with the specified font */
int PutCharFont(SDL_Surface * Surface, BFont_Info * Font, int x, int y, unsigned char c);

/* Returns the width, in pixels, of the text calculated with the specified font*/
int TextWidthFont(BFont_Info * Font, const char *text);

/* Returns the index of the last character than is inside the width limit, with the specified font */
int LimitTextWidthFont(BFont_Info *font, const char *text, int limit);

/* Write a string on the screen with the specified font */
void PutStringFont(BFont_Info *font, int x, int y, const char *text);

/* Write a left-aligned string on the screen with the specified font */
void LeftPutStringFont(BFont_Info * Font, int y, const char *text);

/* Write a center-aligned string on the screen with the specified font */
void CenteredPutStringFont(BFont_Info * Font, int y, const char *text);

/* Write a right-aligned string on the screen with the specified font */
void RightPutStringFont(BFont_Info * Font, int y, const char *text);
