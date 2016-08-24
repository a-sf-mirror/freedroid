/*
 *
 *   Copyright (c) 2016 Samuel Degrande
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
 * This file contains all functions dealing with titles' screen.
 */

#define _title_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"
#include "lua.h"

#include "widgets/widgets.h"

static struct widget_group *title_screen = NULL;
static struct widget_background *title_screen_bkg = NULL;
static struct widget_autoscroll_text *title_screen_text = NULL;
static struct title_screen *screen_data = NULL;

//////////////////////////////////////////////////////////////////////
// Widgets' callbacks
//////////////////////////////////////////////////////////////////////

/*
 * Scroll up button's update callback:
 * Enable the scroll up button if the text can be scrolled up
 */
static void _enable_if_text_can_scroll_up(struct widget *w)
{
	WIDGET_BUTTON(w)->active = widget_autoscroll_text_can_scroll_up(title_screen_text);
}

/*
 * Scroll up button's activate callback:
 * Change the scrolling speed, in the 'up' direction
 */
static void _scroll_up_text(struct widget_button *wb)
{
	if (!wb->active)
		return;
	widget_autoscroll_text_scroll_up(title_screen_text);
}

/*
 * Scroll down button's update callback:
 * Enable the scroll down button if the text can be scrolled down
 */
static void _enable_if_text_can_scroll_down(struct widget *w)
{
	WIDGET_BUTTON(w)->active = widget_autoscroll_text_can_scroll_down(title_screen_text);
}

/*
 * Scroll down button's activate callback:
 * Change the scrolling speed, in the 'down' direction
 */
static void _scroll_down_text(struct widget_button *wb)
{
	if (!wb->active)
		return;
	widget_autoscroll_text_scroll_down(title_screen_text);
}

/*
 * Exit button's activate callback:
 * Disable the title screen, to exit it
 */
static void _exit_title_screen(struct widget_button *wb)
{
	WIDGET(title_screen)->enabled = FALSE;
}

//////////////////////////////////////////////////////////////////////
// Title screen
//////////////////////////////////////////////////////////////////////

/**
 * This function builds the title's screen interface if it hasn't already
 * been initialized.
 */
struct widget_group *title_screen_create()
{
	if (title_screen)
		return title_screen;

	// Container, fills the whole screen
	title_screen = widget_group_create();
	widget_set_rect(WIDGET(title_screen), User_Rect.x, User_Rect.y, User_Rect.w, User_Rect.h);
	WIDGET(title_screen)->enabled = FALSE;

	// Background (the image to use as background is defined later)
	title_screen_bkg = widget_background_create();
	widget_set_rect(WIDGET(title_screen_bkg), User_Rect.x, User_Rect.y, User_Rect.w, User_Rect.h);
	widget_group_add(title_screen, WIDGET(title_screen_bkg));

	// Scrolling text widget (a right padding of 73px is kept for the buttons)
	title_screen_text = widget_autoscroll_text_create();
	widget_set_rect(WIDGET(title_screen_text), User_Rect.x + 10, User_Rect.y, User_Rect.w - 83, User_Rect.h);
	widget_group_add(title_screen, WIDGET(title_screen_text));

	// Buttons
	int screen_right  = WIDGET(title_screen)->rect.x + WIDGET(title_screen)->rect.w;
	int screen_top    = WIDGET(title_screen)->rect.y;
	int screen_bottom = WIDGET(title_screen)->rect.y + WIDGET(title_screen)->rect.h;

	struct {
			char *image[3];
			SDL_Rect rect;
			void (*activate_button)(struct widget_button *);
			void (*update) (struct widget *);
	} b[] = {
			// Scroll up
			{
				{"widgets/autoscroll_text_up_off.png", NULL, "widgets/autoscroll_text_up.png"},
				{screen_right - 73, screen_top + 10, 73, 98},
				_scroll_up_text,
				_enable_if_text_can_scroll_up
			},
			// Scroll down
			{
				{"widgets/autoscroll_text_down_off.png", NULL, "widgets/autoscroll_text_down.png"},
				{screen_right - 73, screen_top + 10 + 98 + 10, 73, 98},
				_scroll_down_text,
				_enable_if_text_can_scroll_down
			},
			// Exit
			{
				{"widgets/autoscroll_text_exit.png", NULL, NULL},
				{screen_right - 72, screen_bottom - 10 - 108, 72, 108},
				_exit_title_screen,
				NULL
			}
	};

	int i;
	for (i = 0; i < sizeof(b) / sizeof(b[0]); i++) {
		struct widget_button *button = widget_button_create();

		WIDGET(button)->rect = b[i].rect;
		WIDGET(button)->update = b[i].update;

		button->image[0][DEFAULT] = widget_load_image_resource(b[i].image[0], NO_MOD);
		button->image[0][PRESSED] = widget_load_image_resource(b[i].image[1], NO_MOD);
		button->image[1][DEFAULT] = widget_load_image_resource(b[i].image[2], NO_MOD);
		button->activate_button = b[i].activate_button;

		widget_group_add(title_screen, WIDGET(button));
	}

	return title_screen;
}

/**
 * Delete the title screen
 */
void title_screen_free()
{
	WIDGET(title_screen)->free(WIDGET(title_screen));
	free(title_screen);
	title_screen = NULL;
}

/**
 * Set the background image of the title screen.
 * On a wide screen, the wide version of the backgound is used, if it exists.
 * @param bkgd_name Filename of the background image (in data/graphics/backgrounds)
 */
void title_screen_set_background(const char *bkgd_name)
{
	struct background *bg = get_background(bkgd_name);
	if (!bg) {
		error_message(__FUNCTION__, "Received a request to display background %s which is unknown.", PLEASE_INFORM, bkgd_name);
		return;
	}
	widget_background_clear(title_screen_bkg);
	widget_background_add(title_screen_bkg, &bg->img,
	                      WIDGET(title_screen_bkg)->rect.x, WIDGET(title_screen_bkg)->rect.y,
						  WIDGET(title_screen_bkg)->rect.w, WIDGET(title_screen_bkg)->rect.h,
						  0);
}

/**
 * Set the text to display in the title screen
 * @param text Text to display (a copy is done)
 * @param font Font to use
 */
void title_screen_set_text(const char *text, struct font *font)
{
	widget_autoscroll_set_text(title_screen_text, text, font);
}

/**
 * Scroll callback, called after preroll, to start voice acting
 * @param w        Pointer to the autoscroll widget
 * @param at_line  Index of the reached text line
 */
static void _resume_voice_acting(struct widget_autoscroll_text *w, int at_line)
{
	resume_voice(screen_data->voice_channel);
}

/**
 * Scroll callback, called on postroll, to re-enable user's interaction and reset
 * scroll speed
 * @param w        Pointer to the autoscroll widget
 * @param at_line  Index of the reached text line
 */
static void _enable_user_interaction(struct widget_autoscroll_text *w, int at_line)
{
	widget_autoscroll_set_scrolling_speed(w, 0.0f);
	widget_autoscroll_disable_scroll_interaction(w, FALSE);
}

/**
 * Display the title_screen and handle the user's interaction.
 */
void title_screen_run(struct title_screen *data)
{
	struct widget *title_screen_ui = WIDGET(title_screen);

	screen_data = data;
	screen_data->voice_channel = -1;
	float voice_duration = 0.0f;

	// Set the content of the title screen
	title_screen_set_background(screen_data->background);
	title_screen_set_text(screen_data->text, Para_Font);

	switch_background_music(screen_data->song);
	if (screen_data->voice_acting) {
		if (screen_data->preroll_text > 0) {
			// Do not immediately start playing the voice acting, and register
			// a callback to resume the audio channel once preroll is done.
			screen_data->voice_channel = play_voice(screen_data->voice_acting, TRUE, &voice_duration);
			widget_autoscroll_call_at_line(title_screen_text, screen_data->preroll_text, _resume_voice_acting);
		} else {
			screen_data->voice_channel = play_voice(screen_data->voice_acting, FALSE, &voice_duration);
		}
		widget_autoscroll_set_scrolling_duration(title_screen_text, voice_duration, screen_data->preroll_text + screen_data->postroll_text);

		// Disable user's interaction, and register a callback to reactivate it
		// and reset the scrolling speed once the postroll line is reached.
		widget_autoscroll_disable_scroll_interaction(title_screen_text, TRUE);
		widget_autoscroll_call_at_line(title_screen_text, -1, _enable_user_interaction);
	}

	// Pump all remaining SDL events
	SDL_Event event;
	while (SDL_PollEvent(&event));

	// Activate the title screen
	title_screen_ui->enabled = TRUE;

	// Loop until the title screen is disabled.
	// This can be due to the user pressing the 'escape' key,
	// or clicking on the Exit button.
	while (title_screen_ui->enabled) {

		// The scrolling of the text is 'controled by speed', so we need to
		// know the actual frame rate
		StartTakingTimeForFPSCalculation();
		update_frames_displayed();

		// Title screen rendering
		// Note: the title screen GUI fills the whole screen, so there is no
		// need to call clear_screen()
		title_screen_ui->update_tree(title_screen_ui);
		title_screen_ui->display(title_screen_ui);
		blit_mouse_cursor();
		our_SDL_flip_wrapper();

		// User's interaction
		while (SDL_PollEvent(&event)) {
			// Title screen specific events handling
			if (event.type == SDL_QUIT) {
				Terminate(EXIT_SUCCESS);
			}
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
				title_screen_ui->enabled = FALSE;
			}
			// Push the event to the widgets
			title_screen_ui->handle_event(title_screen_ui, &event);
		}

		ComputeFPSForThisFrame();
	}

	// Pump all remaining SDL events
	WaitNoEvent();

	if (screen_data->voice_channel != -1)
		stop_voice(screen_data->voice_channel);
}
