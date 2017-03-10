/* 
 *
 *   Copyright (c) 2004-2007 Arthur Huillet 
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
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
 *
 */

/**
 * This file contains all functions dealing with texts on the screen,
 * that have to be blitted somehow, using bitmaps or OpenGL texturers,
 * but at least strongly rely on graphics concepts, not pure internal
 * text-processing.
 */

#define _text_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"

#include "lvledit/lvledit_display.h"

// current text insertion position
static int MyCursorX;
static int MyCursorY;

static int display_char_disabled;

static struct {
	struct auto_string *text;
	struct font *font;
	float duration;
	float countdown_time;
	SDL_Rect text_rect;
	SDL_Rect back_rect;
} transient_text = { NULL, NULL, 0, 0, { 0 }, { 0 } };

/**
 * This function determines how many lines of text it takes to write a string
 * with the current font into the given rectangle, provided one would start at
 * the left side.
 */
int get_lines_needed(const char *text, SDL_Rect t_rect, float line_height_factor)
{
	t_rect.h = 32767; // arbitrary large number
	display_char_disabled = TRUE;
	int lines = display_text(text, t_rect.x, t_rect.y, &t_rect, line_height_factor);
	display_char_disabled = FALSE;
	return lines;
}

/**
 *
 *
 */
void show_backgrounded_label_at_pixel_position(char *LabelText, int pos_x, int pos_y)
{
	SDL_Rect background_rect;

	background_rect.x = pos_x - 1;
	background_rect.y = pos_y;
	background_rect.w = text_width(get_current_font(), LabelText) + 2;
	background_rect.h = 20;

	draw_rectangle(&background_rect, 0, 0, 0, 255);

	set_current_font(FPS_Display_Font);
	put_string(get_current_font(), pos_x, pos_y, LabelText);

};				// void show_backgrounded_label_at_pixel_position ( char* LabelText , float fill_status , int pos_x , int pos_y )

/**
 *
 *
 */
void show_backgrounded_label_at_map_position(char *LabelText, float fill_status, float pos_x, float pos_y, int zoom_is_on)
{
	int pixel_x, pixel_y;

	translate_map_point_to_screen_pixel(pos_x, pos_y, &pixel_x, &pixel_y);
	show_backgrounded_label_at_pixel_position(LabelText, pixel_x, pixel_y);

};				// void show_backgrounded_label_at_map_position ( char* LabelText , float fill_status , float pos_x , float pos_y )

#define IN_WINDOW_TEXT_OFFSET 15

/**
 * Show text inside a filled rectangle. The specified height is taken as a
 * minimum value, and it will be expanded vertically to fit the given text.
 * @return The height of the rectangle
 */
int show_backgrounded_text_rectangle(const char *text, struct font *font, int x, int y, int w, int h)
{
	struct font *old_font = get_current_font();
	set_current_font(font);

	SDL_Rect t_rect;
	t_rect.x = x;
	t_rect.y = y;
	
	// Find out the number of lines the text will occupy. This is done by
	// using the drawing function with drawing temporarily disabled.
	t_rect.w = w - (IN_WINDOW_TEXT_OFFSET * 2);
	t_rect.h = GameConfig.screen_height;
	int lines = get_lines_needed(text, t_rect, 1.0);

	// Calculate the rectangle height
	int f_height = get_font_height(get_current_font());
	int r_height = (lines * f_height) + (IN_WINDOW_TEXT_OFFSET * 2);

	// Set up and fill the rectangle.
	t_rect.w = w;
	t_rect.h = r_height;
	draw_rectangle(&t_rect, 0, 0, 0, 255);
	
	// Show the text inside our newly drawn rectangle.
	t_rect.w -= IN_WINDOW_TEXT_OFFSET * 2;
	t_rect.h -= IN_WINDOW_TEXT_OFFSET;
	t_rect.x += IN_WINDOW_TEXT_OFFSET;
	t_rect.y += IN_WINDOW_TEXT_OFFSET;
	display_text(text, t_rect.x, t_rect.y, &t_rect, 1.0);

	set_current_font(old_font);
	return r_height;
}

void transient_text_set_centered_text(float duration, struct font *font, const char *fmt, ...)
{
	static const int screen_gap = 50;
	static const int rect_padding = 1;

	va_list args;

	// Clean previously used data and prepare it for a new use

	if (transient_text.text) {
		free_autostr(transient_text.text);
		transient_text.text = NULL;
	}

	transient_text.text = alloc_autostr(16);
	if (!transient_text.text)
		return;

	// Store the text to display and its config

	va_start(args, fmt);

	autostr_vappend(transient_text.text, fmt, args);
	transient_text.font = font;
	transient_text.duration = duration;
	transient_text.countdown_time = duration;

	//==========
	// Compute the dimension of the text rect and of the background rect
	//==========

	struct font *current_font = get_current_font();
	set_current_font(font);
	int font_height = get_font_height(get_current_font());

	int max_width = GameConfig.screen_width - 2 * screen_gap;
	int max_text_width = max_width - 2 *rect_padding * font_height;

	// The actual text width is defined by the width of the longuest line
	// clamped between a quarter of the screen's width and the max screen's
	// width minus a gap

	char *tmp = strdup(transient_text.text->value); // longest_line_width() needs to modify the text
	int actual_text_width = longest_line_width(tmp);
	free(tmp);
	int used_text_width = clamp(actual_text_width, GameConfig.screen_width / 4, max_text_width);
	if (actual_text_width > used_text_width)
		actual_text_width = used_text_width;

	SDL_Rect clipping_rect = { .x = 0, .y = 0, .w = actual_text_width, .h = 0 };
	int nb_lines = get_lines_needed(transient_text.text->value, clipping_rect, 1.0);

	int text_left = (GameConfig.screen_width - actual_text_width) / 2;
	int text_top = (GameConfig.screen_height - nb_lines * font_height) / 2;
	int back_left = (GameConfig.screen_width - used_text_width) / 2;

	transient_text.text_rect.x = text_left;
	transient_text.text_rect.y = text_top;
	transient_text.text_rect.w = actual_text_width;
	transient_text.text_rect.h = nb_lines * font_height;
	transient_text.back_rect.x = back_left - rect_padding * font_height;
	transient_text.back_rect.y = text_top - rect_padding * font_height;
	transient_text.back_rect.w = used_text_width + 2 * rect_padding * font_height;
	transient_text.back_rect.h = (nb_lines + 2 * rect_padding) * font_height;

	set_current_font(current_font);

	va_end(args);
}

void transient_text_free(void)
{
	if (transient_text.text)
		free_autostr(transient_text.text);
	transient_text.text = NULL;
	transient_text.countdown_time = 0.0;
}

void transient_text_show(void)
{
	if (!transient_text.text)
		return;

	struct font *current_font = get_current_font();
	set_current_font(transient_text.font);

	float opacity = 255.0 * transient_text.countdown_time / transient_text.duration;
	draw_rectangle(&transient_text.back_rect, 0, 0, 0, (int)opacity);
	display_text(transient_text.text->value, transient_text.text_rect.x, transient_text.text_rect.y,
	             &transient_text.text_rect, 1.0);

	set_current_font(current_font);

	transient_text.countdown_time -= Frame_Time();
	if (transient_text.countdown_time <= 0) {
		transient_text_free();
	}
}

/**
 * 
 */
int CutDownStringToMaximalSize(char *StringToCut, int LengthInPixels)
{
	int StringIndex = 0;
	int i;

	if (text_width(get_current_font(), StringToCut) <= LengthInPixels)
		return FALSE;

	StringIndex = limit_text_width(get_current_font(), StringToCut, LengthInPixels);
	if (StringIndex < 1)
		return FALSE;

	for (i = 0; i < 3; i++) {
		if (StringToCut[StringIndex + i] != 0) {
			StringToCut[StringIndex + i] = '.';
		} else
			return TRUE;
	}
	StringToCut[StringIndex + 3] = 0;

	return TRUE;
};				// void CutDownStringToMaximalSize ( char* StringToCut , int LengthInPixels )

/**
 * This function sets a new text, that will be displayed in huge font 
 * directly over the combat window for a fixed duration of time, where
 * only the time in midst of combat and with no other windows opened
 * is counted.
 */
void SetNewBigScreenMessage(const char *ScreenMessageText)
{
	int i = MAX_BIG_SCREEN_MESSAGES - 1;

	/* Free the last message that's going to be overwritten */
	if (Me.BigScreenMessage[i]) {
		free(Me.BigScreenMessage[i]);
		Me.BigScreenMessage[i] = NULL;
	}

	while (i > 0) {
		Me.BigScreenMessage[i] = Me.BigScreenMessage[i - 1];
		Me.BigScreenMessageDuration[i] = Me.BigScreenMessageDuration[i - 1];
		i--;
	}

	Me.BigScreenMessage[0] = MyMalloc(strlen(ScreenMessageText) + 1);
	strcpy(Me.BigScreenMessage[0], ScreenMessageText);
	Me.BigScreenMessageDuration[0] = 0;

};				// void SetNewBigScreenMessage( char* ScreenMessageText )

/**
 * This function displays the currently defined Bigscreenmessage on the
 * screen.  It will be called by AssembleCombatWindow.
 */
void DisplayBigScreenMessage(void)
{
	int i;
	int y_pos = 30;

	for (i = 0; i < GameConfig.number_of_big_screen_messages; i++) {
		if (!Me.BigScreenMessage[i])
			continue;

		char *text = Me.BigScreenMessage[i];

		if (Me.BigScreenMessageDuration[i] < GameConfig.delay_for_big_screen_messages) {
			SDL_SetClipRect(Screen, NULL);

			set_current_font(Menu_Font);
			int x_pos = GameConfig.screen_width/2 - text_width(get_current_font(), text)/2;
			display_text(text, x_pos, y_pos, NULL /* clip */, 1.0);

			if (!GameConfig.Inventory_Visible && !GameConfig.SkillScreen_Visible && !GameConfig.CharacterScreen_Visible)
				Me.BigScreenMessageDuration[i] += Frame_Time();

			y_pos += get_font_height(Menu_Font);
		}
	}

};				// void DisplayBigScreenMessage( void )

static void display_countdown(int pixel_x, int pixel_y, const char* message, float duration)
{
	char text[100];
	int minutes = ((int) duration) / 60;
	int seconds = ((int) duration) - (minutes * 60);

	if (minutes >= 60) {
		sprintf(text, "%s", message);
	} else if (minutes) {
		sprintf(text, "%s %dm%ds", message, minutes, seconds);
	} else {
		sprintf(text, "%s %.1fs", message, duration);
	}

	pixel_x -= text_width(get_current_font(), text) / 2;

	display_text(text, pixel_x, pixel_y, NULL /* clip */, 1.0);
}

/**
 * This function displays the remaining seconds of effect on Tux above him.
 */
void display_effect_countdowns(void)
{
	int pixel_x, pixel_y;
	translate_map_point_to_screen_pixel(Me.pos.x, Me.pos.y, &pixel_x, &pixel_y);
	pixel_y -= 128; /* tux height... */

	if (Me.slowdown_duration > 0.0) {
		set_current_font(Blue_Font);
		display_countdown(pixel_x, pixel_y, _("slowed"), Me.slowdown_duration);
		pixel_y -= get_font_height(Blue_Font);
	}

	if (Me.paralyze_duration > 0.0) {
		set_current_font(Red_Font);
		display_countdown(pixel_x, pixel_y, _("paralyzed"), Me.paralyze_duration);
		pixel_y -= get_font_height(Red_Font);
	}

	if (Me.invisible_duration > 0.0) {
		set_current_font(FPS_Display_Font);
		display_countdown(pixel_x, pixel_y, _("invisible"), Me.invisible_duration);
		pixel_y -= get_font_height(FPS_Display_Font);
	}

	if (Me.nmap_duration > 0.0) {
		set_current_font(FPS_Display_Font);
		display_countdown(pixel_x, pixel_y, _("scanning"), Me.nmap_duration);
	}
};		// void display_effect_countdowns( void )

static int display_text_with_cursor(const char *text, int startx, int starty, const SDL_Rect *clip, float line_height_factor, int curpos)
{
	char *tmp;		// mobile pointer to the current position in the string to be printed
	SDL_Rect Temp_Clipping_Rect;	// adding this to prevent segfault in case of NULL as parameter
	SDL_Rect store_clip;
	short int nblines = 1;
	int empty_lines_started = 0;
	int current_curpos = 0;

	if (text == NULL)
		return 0;

	int letter_spacing = get_letter_spacing(get_current_font());
	int tab_width = TABWIDTH * (font_char_width(get_current_font(), TABCHAR) + letter_spacing);

	if (text[0]=='\0')
		nblines = 0;

	// We position the internal text cursor on the right spot for
	// the first character to be printed.
	//
	MyCursorX = startx;
	MyCursorY = starty;

	int cursor_x = MyCursorX;
	int cursor_y = MyCursorY;

	// We make a backup of the current clipping rect, so we can restore
	// it later.
	//
	SDL_GetClipRect(Screen, &store_clip);

	// If we did receive some clipping rect in the parameter list (like e.g. it's
	// always the case with dialog output) we enforce this new clipping rect, otherwise
	// we just set the clipping rect to contain the whole screen.
	//
	if (clip != NULL) {
		SDL_SetClipRect(Screen, clip);
	} else {
		clip = &Temp_Clipping_Rect;
		Temp_Clipping_Rect.x = 0;
		Temp_Clipping_Rect.y = 0;
		Temp_Clipping_Rect.w = GameConfig.screen_width;
		Temp_Clipping_Rect.h = GameConfig.screen_height;
	}

	set_gl_clip_rect(clip);

	start_image_batch();

	// Now we can start to print the actual text to the screen.
	//
	// The running text pointer must be initialized.
	//
	tmp = (char *)text;	// this is no longer a 'const' char*, but only a char*
	while (*tmp && (MyCursorY < clip->y + clip->h)) {
		if (current_curpos == curpos) {
			cursor_x = MyCursorX;
			cursor_y = MyCursorY;
		}

		if (handle_switch_font_char(&tmp)) {
			current_curpos++;
			continue;
		}

		if (((*tmp == ' ') || (*tmp == '\t'))
		    && (ImprovedCheckLineBreak(tmp, clip, line_height_factor) == 1))	// don't write over right border 
		{		/*THE CALL ABOVE HAS DONE THE CARRIAGE RETURN FOR US !!! */
			empty_lines_started++;
			tmp++;
			current_curpos++;
			continue;
		}

		// carriage return in the middle of a word if it is too big to fit on one line
		if (isgraph(*tmp) && (MyCursorX + font_char_width(get_current_font(), *tmp) + letter_spacing) > (clip->x + clip->w)) {
			MyCursorX = clip->x;
			MyCursorY += (int)(get_font_height(get_current_font()) * line_height_factor);
			empty_lines_started++;
		}

		switch (*tmp) {
		case '\n':
			MyCursorX = clip->x;
			MyCursorY += (int)(get_font_height(get_current_font()) * line_height_factor);
			empty_lines_started++;
			break;
		case '\t':
			MyCursorX = (int)ceilf((float)MyCursorX / (float)(tab_width)) * (tab_width);
			break;
		default:
			if (MyCursorY > clip->y - (int)(get_font_height(get_current_font()) * line_height_factor)
				&& !display_char_disabled)
				put_char(get_current_font(), MyCursorX, MyCursorY, *tmp);

			MyCursorX += font_char_width(get_current_font(), *tmp)
				+ get_letter_spacing(get_current_font());

			// At least one visible character must follow a line break or else
			// the line is a trailing empty line not visible to the user. Such
			// empty lines don't contribute to the visual height of the string.
			if (isgraph(*tmp)) {
				nblines += empty_lines_started;
				empty_lines_started = 0;
			}
		}
		tmp++;
		current_curpos++;
	}

	end_image_batch(__FUNCTION__);

	SDL_SetClipRect(Screen, &store_clip);	// restore previous clip-rect 

	if (curpos != -1) {
		if (curpos != 0 && current_curpos == curpos) {
			cursor_x = MyCursorX;
			cursor_y = MyCursorY;
		}

		SDL_Rect cursor_rect;
		cursor_rect.x = cursor_x;
		cursor_rect.y = cursor_y;
		cursor_rect.h = get_font_height(get_current_font());
		cursor_rect.w = 8;
		draw_highlight_rectangle(cursor_rect);
	}
	
	if (use_open_gl)
		unset_gl_clip_rect();

	return nblines;
}

/**
 * Prints text at given positions, automatically word-wrapping at the
 * edges of clip_rect.  If clip_rect is NULL, no clipping is performed.
 *      
 * @return number of lines written (from the first text line up to the
 *         last displayed line)
 */
int display_text(const char *text, int startx, int starty, const SDL_Rect *clip, float line_height_factor)
{
	return display_text_with_cursor(text, startx, starty, clip, line_height_factor, -1);
}

/**
 * This function checks if the next word still fits in this line
 * of text and initiates a carriage return/line feed if not.
 * Very handy and convenient, for that means it is no longer necessary
 * to enter \n in the text every time its time for a newline. cool.
 *  
 * rp: added argument clip, which contains the text-window we're writing in
 *     (formerly known as "TextBorder")
 *
 * ah: added return value : 1 if carriage return was done, FALSE otherwise
 */
int ImprovedCheckLineBreak(char *Resttext, const SDL_Rect * clip, float line_height_factor)
{
	int NeededSpace = 0;

	int letter_spacing = get_letter_spacing(get_current_font());

	if (*Resttext == ' ')
		NeededSpace = font_char_width(get_current_font(), ' ') + letter_spacing;
	else if (*Resttext == '\t')
		NeededSpace = TABWIDTH * (font_char_width(get_current_font(), TABCHAR) + letter_spacing);

	Resttext++;
	while ((*Resttext != ' ') && (*Resttext != '\t') && (*Resttext != '\n') && (*Resttext != 0)) {
		NeededSpace += font_char_width(get_current_font(), *Resttext) + letter_spacing;
		Resttext++;
	}

	if ((MyCursorX + NeededSpace) > (clip->x + clip->w)) {
		MyCursorX = clip->x;
		MyCursorY += (int)(get_font_height(get_current_font()) * line_height_factor);
		return 1;
	}

	return 0;
};				// int ImprovedCheckLineBreak()

/**
 * Prompt the user for a string no longer than MaxLen (excluding terminating \0).
 */
char *get_string(int max_len, const char *background_name, const char *text_for_overhead_promt)
{
	display_text(text_for_overhead_promt, 50, 50, NULL, 1.0);

	// allocate memory for the users input
	char *input = MyMalloc(max_len + 5); // pointer to the string entered by the user

	memset(input, '.', max_len);
	input[max_len] = 0;

	int finished = FALSE;
	int curpos = 0; // counts the characters entered so far

	while (!finished) {
		blit_background(background_name);
		display_text(text_for_overhead_promt, 50, 50, NULL, 1.0);

		int x0 = MyCursorX;
		int y0 = MyCursorY;

		put_string(get_current_font(), x0, y0, input);
		our_SDL_flip_wrapper();

		int key = getchar_raw(NULL); // last 'character' entered

		if (key == SDLK_RETURN) {
			// Display the image again so both buffers are in sync
			// useful for GL drivers that do true pageflipping (win32, nvidia 173.x, ...)
			if (use_open_gl) {
				blit_background(background_name);
				display_text(text_for_overhead_promt, 50, 50, NULL, 1.0);
				x0 = MyCursorX;
				y0 = MyCursorY;
				put_string(get_current_font(), x0, y0, input);
				our_SDL_flip_wrapper();
			}
			input[curpos] = 0;
			finished = TRUE;
		} else if ((key < SDLK_DELETE) && isprint(key) && (curpos < max_len)) {
			/* printable characters are entered in string */
			input[curpos] = (char)key;
			curpos++;
		} else if ((key <= SDLK_KP9) && (key >= SDLK_KP0) && (curpos < max_len)) {
			key -= SDLK_KP0;
			key += '0';

			input[curpos] = (char)key;
			curpos++;
		} else if (key == SDLK_BACKSPACE) {
			if (curpos > 0)
				curpos--;
			input[curpos] = '.';
		} else if (key == SDLK_ESCAPE) {
			free(input);
			while (EscapePressed()) ;
			return (NULL);
		}
	}

	return (input);
}

/* -----------------------------------------------------------------
 * This function reads a string of "MaxLen" from User-input.
 *
 * NOTE: MaxLen is the maximal _strlen_ of the string (excl. \0 !)
 * 
 * ----------------------------------------------------------------- */
char *get_editable_string_in_popup_window(int max_len, const char *popup_window_title, const char *default_string)
{
#define EDIT_WINDOW_TEXT_OFFSET 15

	char *input = MyMalloc(max_len + 5); // pointer to the string entered by the user
	strncpy(input, default_string, max_len - 1);
	input[max_len] = 0;
	int curpos = strlen(input); // counts the characters entered so far

	// Now we prepare a rectangle for our popup window...
	//
	SDL_Rect target_rect;
	target_rect.w = 440;
	target_rect.h = 340;
	target_rect.x = (640 - target_rect.w) / 2;
	target_rect.y = (480 - target_rect.h) / 2;
	target_rect.w -= EDIT_WINDOW_TEXT_OFFSET;
	target_rect.h -= EDIT_WINDOW_TEXT_OFFSET;
	target_rect.x += EDIT_WINDOW_TEXT_OFFSET;
	target_rect.y += EDIT_WINDOW_TEXT_OFFSET;

	// Now we find the right position for the new string to start by writing
	// out the title text once, just to get the cursor positioned right...
	//
	display_text(popup_window_title, target_rect.x, target_rect.y, &target_rect, 1.0);
	int x0 = MyCursorX;
	int y0 = MyCursorY;

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	// Now we can start the enter-string cycle...
	//
	int finished = FALSE;
	while (!finished) {

		target_rect.w = 440;
		target_rect.h = 340;
		target_rect.x = (640 - target_rect.w) / 2;
		target_rect.y = (480 - target_rect.h) / 2;
		draw_rectangle(&target_rect, 0, 0, 0, 255);

		target_rect.w -= EDIT_WINDOW_TEXT_OFFSET;
		target_rect.h -= EDIT_WINDOW_TEXT_OFFSET;
		target_rect.x += EDIT_WINDOW_TEXT_OFFSET;
		target_rect.y += EDIT_WINDOW_TEXT_OFFSET;

		set_current_font(FPS_Display_Font);

		display_text(popup_window_title, target_rect.x, target_rect.y, &target_rect, 1.0);

		if (popup_window_title[strlen(popup_window_title) - 1] != '\n')
			display_text("\n\n", x0, y0, &target_rect, 1.0);

		target_rect.x = MyCursorX;
		target_rect.y = MyCursorY;
		display_text_with_cursor(input, target_rect.x, target_rect.y, &target_rect, 1.0, curpos);

		our_SDL_flip_wrapper();

		int key = getchar_raw(NULL); // last 'character' entered

		if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
			// input[curpos] = 0;
			finished = TRUE;
		} else if (key == SDLK_ESCAPE) {
			while (EscapePressed())
				SDL_Delay(1);
			free(input);
			return NULL;
		} else if (isprint(key) && (curpos < max_len)) {
			// If a printable character has been entered, it is either appended to
			// the end of the current input string or the rest of the string is being
			// moved and the new character inserted at the end.
			//
			if (curpos == ((int)strlen(input))) {
				input[curpos] = (char)key;
				curpos++;
				input[curpos] = 0;
			} else {
				if (((int)strlen(input)) == max_len - 1)
					input[max_len - 2] = 0;
				int i;
				for (i = strlen(input); i >= curpos; i--) {
					input[i + 1] = input[i];
				}
				input[curpos] = (char)key;
				curpos++;
			}
		} else if (key == SDLK_LEFT) {
			if (curpos > 0)
				curpos--;
			// input[curpos] = '.';
		} else if (key == SDLK_RIGHT) {
			if (curpos < ((int)strlen(input)))
				curpos++;
			// input[curpos] = '.';
		} else if (key == SDLK_BACKSPACE) {
			if (curpos > 0) {
				int i = curpos;
				while (input[i - 1] != 0) {
					input[i - 1] = input[i];
					i++;
				}
				curpos--;
			}
		} else if (key == SDLK_DELETE) {
			if (curpos > 0) {
				int i = curpos;
				while (input[i] != 0) {
					input[i] = input[i + 1];
					i++;
				}
			}

		} else if ((key <= SDLK_KP9) && (key >= SDLK_KP0) && (curpos < max_len)) {
			key -= SDLK_KP0;
			key += '0';

			input[curpos] = (char)key;
			curpos++;
		}

	}			// while ( ! finished ) 

	SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);

	return input;
}

/* -----------------------------------------------------------------
 * behaves similarly as gl_printf() of svgalib, using the BFont
 * print function PrintString().
 *  
 *  sets current position of MyCursor[XY],  
 *     if last char is '\n': to same x, next line y
 *     to end of string otherwise
 *
 * Added functionality to PrintString() is: 
 *  o) passing -1 as coord uses previous x and next-line y for printing
 *  o) Screen is updated immediately after print, using SDL_flip()                       
 *
 * ----------------------------------------------------------------- */
void printf_SDL(SDL_Surface * screen, int x, int y, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	if (x == -1)
		x = MyCursorX;
	else
		MyCursorX = x;

	if (y == -1)
		y = MyCursorY;
	else
		MyCursorY = y;

	char *tmp = (char *)MyMalloc(10000 + 1);
	vsprintf(tmp, fmt, args);
	put_string(get_current_font(), x, y, tmp);

	// our_SDL_flip_wrapper (screen);

	if (tmp[strlen(tmp) - 1] == '\n') {
		MyCursorX = x;
		MyCursorY = y + 1.1 * (get_font_height(get_current_font()));
	} else {
		int i;
		for (i = 0; i < ((int)strlen(tmp)); i++)
			MyCursorX += font_char_width(get_current_font(), tmp[i]);
		MyCursorY = y;
	}

	free(tmp);
	va_end(args);

};				// void printf_SDL (SDL_Surface *screen, int x, int y, char *fmt, ...)

/**
 * Find the longest line in text, taking letter-spacing into account. Return the
 * minimum line width needed to print that line, using the current font.
 */
int longest_line_width(char *text)
{
	int width = 0;
	char *line_start = text;

	for (; TRUE; text++) {
		// Find the end of the current line.
		if (*text != '\n' && *text != '\0') {
			continue;
		}

		// Get the width of the current line.
		char tmp = *text;
		*text = '\0';
		int line_width = text_width(get_current_font(), line_start);
		*text = tmp;

		// Update the width of the longest line.
		if (line_width > width) {
			width = line_width;
		}

		// Handle the next line, if any remain.
		if (*text != '\0') {
			line_start = text + 1;
		} else {
			break;
		}
	}

	return width;
}

#undef _text_c
