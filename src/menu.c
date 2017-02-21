/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2010 Arthur Huillet 
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
 * This file contains all menu functions and their subfunctions
 */

#define _menu_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "widgets/widgets.h"

int Single_Player_Menu(void);

#define SINGLE_PLAYER_STRING _("Play")
#define LOAD_EXISTING_HERO_STRING _("Your characters: ")
#define DELETE_EXISTING_HERO_STRING _("Select character to delete: ")

#define MENU_SELECTION_DEBUG 1

#define AUTO_SCROLL_RATE (0.02f)

#define MAX_MENU_ITEMS 100
#define MAX_SAVEGAMES_PER_PAGE 5

static int game_needs_restart = FALSE;

/**
 * This function tells over which menu item the mouse cursor would be,
 * if there were infinitely many menu items.
 */
static int MouseCursorIsOverMenuItem(int first_menu_item_pos_y, int h)
{
	int PureFraction;

	PureFraction = (GetMousePos_y() - first_menu_item_pos_y) / h;

	// Now it can be that the pure difference is negative or that it is positive.
	// However we should not always round towards zero here, but rather always to
	// the next LOWER integer!  This will be done here:
	//
	if ((GetMousePos_y() - first_menu_item_pos_y) < 0)
		PureFraction--;
	else
		PureFraction++;

	return (PureFraction);

};				// void MouseCursorIsOverMenuItem( first_menu_item_pos_y )

/**
 * This function performs a menu for the player to select from, using the
 * keyboard only, currently, sorry.
 *
 * This function EXPECTS, that MenuTexts[] is an array of strings, THE 
 * LAST OF WHICH IS AN EMPTY STRING TO DENOTE THE END OF THE ARRAY!  If 
 * this is not respected, segfault errors are likely.
 * 
 */
int do_menu_selection(char *header_text, char **items_texts, int first_item_idx, const char *background_name, struct font *menu_font)
{
	int old_game_status = game_status;
	game_status = INSIDE_MENU;

	// A menu contains 2 parts:
	// - if a header is set, a header text and a background rect around it
	// - the menu's items and a background rect around it
	// A small gap separates those two parts.
	//
	// If there are too many items to fit inside the screen's height, a part of
	// the list is displayed, which can be scrolled up or down using the arrow
	// keys or the mouse wheel.
	//
	// If an item is too large to fit inside the screen's width, it is auto-scrolled
	// when highlighted.

	// Menu drawing style:
	// - a minimum of 50 pixels between the screen and the menu
	// - one 'line-height' padding between the menu's background and its content
	// - one 'half-line' gap between the header's rect and the items' rect
	static const int screen_gap = 50;
	static const int rect_padding = 1;
	static const int header_items_gap = 1;

	// We set the given font, if appropriate, and get the font height...

	if (menu_font == NULL)
		set_current_font(Menu_Font);
	else
		set_current_font(menu_font);
	int font_height = get_font_height(get_current_font());

	// Find out how may items we have been given for the menu.

	int nb_items = 0;
	for (nb_items = 0; nb_items < 1000; ++nb_items) {
		if (items_texts[nb_items][0] == '\0') {
			break;
		}
	}

	//----------
	// 1- Compute the background rect and the content rect of the header and the items
	//----------

	int max_menu_width = GameConfig.screen_width - 2 * screen_gap;
	int max_menu_height = GameConfig.screen_height - 2 * screen_gap;
	int max_content_width = max_menu_width - 2 * rect_padding * font_height;

	// Compute the width needed to render the items and the header.
	// A minimum width is set to be a quarter of the screen's width.
	// This minimum ensures that the header's text will not be split
	// into too much small parts.
	// For each item, compute its pixel-width, and keep the largest width.
	// (the loop's end value is to avoid an 'infinite loop' - we will never have
	// a menu with 1.000 items, will we ?)
	// Finally, clamp to the available max width of the menu's content.
	// (items content are auto-scrolled if they are larger)

	int content_width = min(GameConfig.screen_width / 4, max_content_width);
	int *items_texts_widths = (int *)malloc(sizeof(int) * (nb_items + 1));

	for (int i = 0; i < nb_items; ++i) {
		items_texts_widths[i] = text_width(get_current_font(), items_texts[i]);
		content_width = max(content_width, items_texts_widths[i]);
	}
	items_texts_widths[nb_items] = 0;

	content_width = min(content_width, max_content_width);

	// Find a satisfying width for the header:
	// If it is not larger than the width of the items, all is good.
	// Else, we accept to extend the width to a third of the screen.
	// If it's still not enough, we keep the items' width, and the header
	// will be split.

	if (strlen(header_text) != 0) {
		char *tmp = strdup(header_text); // longest_line_width() needs to be able to modify the text
		int actual_width = longest_line_width(tmp);
		free(tmp);
		if (actual_width > content_width) {
			if (actual_width < (GameConfig.screen_width / 3)) {
				content_width = actual_width;
			}
		}
	}

	// Compute the height needed to render the header.
	// The header text will be split into several lines (if needed) to fit
	// the menu width

	int header_content_height = 0;
	int offset_to_items_rect = 0;

	if (strlen(header_text) != 0) {
		SDL_Rect clipping_rect = { .x = 0, .y = 0, .w = content_width, .h = 0 };
		int header_nb_lines = get_lines_needed(header_text, clipping_rect, 1.0);
		header_content_height = header_nb_lines * font_height;
		offset_to_items_rect = header_content_height + (2 * rect_padding + header_items_gap) * font_height;
	}

	// Find how many items you can fit in the menu, given the room left by the header

	int available_height = max_menu_height - offset_to_items_rect;
	int max_fitting_items = min((available_height / font_height - 2 * rect_padding), nb_items);
	int items_background_height = (max_fitting_items + 2 * rect_padding) * font_height;

	// We can now set the 4 rects used to render the menu

	int menu_width   = content_width + 2 * rect_padding * font_height;
	int menu_left    = (GameConfig.screen_width - menu_width) / 2;

	int whole_height = offset_to_items_rect + items_background_height;

	SDL_Rect header_background_rect = {
		.x = menu_left,
		.y = (GameConfig.screen_height - 2 * screen_gap - whole_height) / 2,
		.w = menu_width,
		.h = header_content_height + 2 * rect_padding * font_height
	};
	SDL_Rect header_content_rect    = {
		.x = menu_left + rect_padding * font_height,
		.y = header_background_rect.y + rect_padding * font_height,
		.w = content_width,
		.h = header_content_height
	};
	SDL_Rect items_background_rect  = {
		.x = menu_left,
		.y = header_background_rect.y + offset_to_items_rect,
		.w = menu_width,
		.h = items_background_height
	};
	SDL_Rect items_content_rect     = {
		.x = menu_left + rect_padding * font_height,
		.y = items_background_rect.y + rect_padding * font_height,
		.w = content_width,
		.h = items_background_height - 2 * rect_padding * font_height
	};

	//----------
	// 2- Prepare a screen background, which will be reused during the
	//    interaction loop
	//----------

	struct font* store_font = get_current_font();
	InitiateMenu(background_name);

	if (!strcmp(items_texts[0], SINGLE_PLAYER_STRING)) {
		// If this is the very first startup menu, we should also print out some
		// status variables like whether using OpenGL or not...

		set_current_font(FPS_Display_Font);
		int line_height = get_font_height(get_current_font());

		// display version string
		put_string_right(FPS_Display_Font, GameConfig.screen_height - 1.4 * line_height, freedroid_version);

#ifdef HAVE_LIBGL
		char *open_gl_string = _("OpenGL support compiled: YES");
#else
		char *open_gl_string = _("OpenGL support compiled: NO");
#endif
		put_string_left(FPS_Display_Font, GameConfig.screen_height - 2.4 * line_height, open_gl_string);

		if (use_open_gl)
			open_gl_string = _("OpenGL output active: YES");
		else
			open_gl_string = _("OpenGL output active: NO");
		put_string_left(FPS_Display_Font, GameConfig.screen_height - 1.4 * line_height, open_gl_string);
	}

	set_current_font(store_font);
	StoreMenuBackground(0);

	// Some menus are intended to start with the default setting of the
	// first menu item selected. However this is not always desired.
	// It might happen, that a submenu returns to the upper menu and then
	// the upper menu should not be reset to the first position selected.
	// For this case we have some special '-1' entry reserved as the marked
	// menu entry. This means, that the menu position from last time will
	// simply be re-used.
	// In those cases where we don't reset the menu position upon
	// initialization of the menu, we must check for menu positions
	// outside the bounds of the current menu.

	static int selected_item_idx = 1;
	int vert_scroll_offset = 0;

	if (first_item_idx != -1)
		selected_item_idx = first_item_idx;

	if (selected_item_idx > nb_items) {
		selected_item_idx = 1;
	} else if (selected_item_idx > max_fitting_items) {
		vert_scroll_offset = (selected_item_idx - 1)/max_fitting_items;
		selected_item_idx = selected_item_idx % max_fitting_items;
	}

	// Interaction's loop - wait for the user to select an option or quit
	// the menu

	float auto_scroll_start = 0.0f;
	int auto_scroll_run = TRUE;
	int ret = -1;

	for (;;) {
		save_mouse_state();

		// Display the background of the menu, either by doing it once more
		// in the open_gl case or by restoring what we have saved earlier, in the 
		// SDL output case.

		RestoreMenuBackground(0);

		// Maybe we should display some thumbnails with the saved games entries?
		// But this will only apply for the load_hero and the delete_hero menus,
		// and only if a saved game option is currently highlighted

		if (((!strcmp(header_text, LOAD_EXISTING_HERO_STRING)) ||
		     (!strcmp(header_text, DELETE_EXISTING_HERO_STRING))) &&
		    (selected_item_idx + vert_scroll_offset < nb_items) &&
		    strcmp(items_texts[selected_item_idx - 1 + vert_scroll_offset], _("[down]")) &&
		    strcmp(items_texts[selected_item_idx - 1 + vert_scroll_offset], _("[up]")) &&
		    strcmp(items_texts[selected_item_idx - 1 + vert_scroll_offset], " ")) {
			load_and_show_thumbnail(items_texts[selected_item_idx - 1 + vert_scroll_offset]);
			load_and_show_stats(items_texts[selected_item_idx - 1 + vert_scroll_offset]);
		}

		// Display the header text and the menu items

		if (strlen(header_text) > 0) {
			draw_shadowing_rectangle(header_background_rect);
			display_text(header_text, header_content_rect.x, header_content_rect.y, &header_content_rect, 1.0);
		}

		draw_shadowing_rectangle(items_background_rect);

		for (int i = 0; i < max_fitting_items; i++) {
			int item_idx = i + vert_scroll_offset;

			if (items_texts_widths[item_idx] == 0)
				break;

			// Don't select empty menu entries
			if (strcmp(items_texts[item_idx], " ") == 0)
				continue;

			// Define the actual text to display
			// If the text is too long, handle autoscroll and clip it
			char *str = strdup(items_texts[item_idx] + (i == selected_item_idx - 1 ? (int)auto_scroll_start : 0));

			if (!CutDownStringToMaximalSize(str, items_content_rect.w)) {
				// if cutting was not needed, we are at the end of the text,
				// so stop autoscroll
				if (i == selected_item_idx - 1) {
					auto_scroll_run = FALSE;
					auto_scroll_start = 0.0f;
				}
			}

			// Highlighting of the currently selected menu item
			if (i == selected_item_idx - 1) {
				SDL_Rect highlight_rect = { .x = items_background_rect.x + (rect_padding * font_height) / 2,
				                            .y = items_content_rect.y + (i * font_height),
				                            .w = items_background_rect.w - rect_padding * font_height,
				                            .h = font_height };
				draw_highlight_rectangle(highlight_rect);

				if (auto_scroll_run == TRUE) {
					auto_scroll_start += AUTO_SCROLL_RATE;
				}
			}

			// Draw the option's text
			put_string_centered(get_current_font(), items_content_rect.y + (i * font_height), str);

			free(str);
		}

		// Now the mouse cursor must be brought to the screen

		blit_mouse_cursor();

		// Image should be ready now, so we can show it...

		our_SDL_flip_wrapper();

		// Now it's time to handle the possible keyboard and mouse input from
		// the user...

		int old_menu_position = selected_item_idx;
		int old_scroll_offset = vert_scroll_offset;
		SDL_Event event;

		if (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {
				Terminate(EXIT_SUCCESS);
			}
			//(clever?) hack : mouse wheel up and down behave
			//exactly like UP and DOWN arrow, so we mangle the event
			if (event.type == SDL_MOUSEBUTTONDOWN) {
				switch (event.button.button) {
				case SDL_BUTTON_WHEELUP:
					event.type = SDL_KEYDOWN;
					event.key.keysym.sym = SDLK_UP;
					break;
				case SDL_BUTTON_WHEELDOWN:
					event.type = SDL_KEYDOWN;
					event.key.keysym.sym = SDLK_DOWN;
					break;
				default:
					break;
				}
			}

			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					MenuItemDeselectedSound();
					ret = -1;
					goto out;
					break;

				case SDLK_RETURN:
				case SDLK_SPACE:
				case SDLK_LEFT:
				case SDLK_RIGHT:
					// The space key or enter key or arrow keys all indicate, that
					// the user has made a selection.
					MenuItemSelectedSound();
					ret = selected_item_idx + vert_scroll_offset;
					goto out;
					break;

				case SDLK_UP:
				{
					if ((selected_item_idx == 1)
						&& (vert_scroll_offset > 0)
						&& (nb_items > max_fitting_items)) {
						--vert_scroll_offset;
						continue;
					}
					if (selected_item_idx > 1)
						selected_item_idx--;

					// Skip any blank positions when moving with the keyboard
					if (strcmp(items_texts[selected_item_idx - 1 + vert_scroll_offset], " ") == 0)
						selected_item_idx = (selected_item_idx == 1) ? selected_item_idx + 1 : selected_item_idx - 1;

					Uint16 warp_x = GameConfig.screen_width / 2;
					Uint16 warp_y = items_content_rect.y + (selected_item_idx - 1) * font_height;
					SDL_WarpMouse(warp_x, warp_y + font_height/2);
					break;
				}

				case SDLK_DOWN:
				{
					if ((selected_item_idx == max_fitting_items)
						&& (vert_scroll_offset + max_fitting_items < nb_items)
						&& (nb_items > max_fitting_items)) {
						++vert_scroll_offset;
						continue;
					}
					if (selected_item_idx < max_fitting_items)
						selected_item_idx++;

					// Skip any blank positions when moving with the keyboard
					if (strcmp(items_texts[selected_item_idx - 1 + vert_scroll_offset], " ") == 0)
						selected_item_idx = (selected_item_idx == max_fitting_items) ? selected_item_idx - 1 : selected_item_idx + 1;

					Uint16 warp_x = GameConfig.screen_width / 2;
					Uint16 warp_y = items_content_rect.y + (selected_item_idx - 1) * font_height;
					SDL_WarpMouse(warp_x, warp_y + font_height/2);
					break;
				}

				default:
					break;
				}
			}

		}

		if (MouseLeftClicked()) {
			// Only when the mouse click really occurred on the menu do we
			// interpret it as a menu choice.  Otherwise we'll just ignore
			// it. Also, we completely ignore any clicks if the clicked
			// position is blank.
			if (MouseCursorIsOverMenuItem(items_content_rect.y, font_height) == selected_item_idx &&
				strcmp(items_texts[selected_item_idx - 1 + vert_scroll_offset], " ") != 0) {

				MenuItemSelectedSound();
				ret = selected_item_idx + vert_scroll_offset;
				goto out;
			}
		}

		selected_item_idx = MouseCursorIsOverMenuItem(items_content_rect.y, font_height);
		if (selected_item_idx < 1)
			selected_item_idx = 1;
		if (selected_item_idx > min(max_fitting_items, nb_items))
			selected_item_idx = min(max_fitting_items, nb_items);

		// If the selected option has changed, halt eventual current autoscrolling
		if (selected_item_idx != old_menu_position || vert_scroll_offset != old_scroll_offset) {
			MoveMenuPositionSound();
			auto_scroll_run = TRUE;
			auto_scroll_start = 0.0f;
		}
		// At this the while (1) overloop ends.  But for the menu, we really do not
		// need to hog the CPU.  Therefore some waiting should be introduced here.
		//
		SDL_Delay(1);
	}

 out:
	free(items_texts_widths);
	while (MouseLeftPressed())	//eat the click that may have happened
		SDL_Delay(1);
	input_handle();
	game_status = old_game_status;
	return ret;
};				// int DoMenuSelection( ... )

/**
 * This function prepares the screen for the big Escape menu and 
 * its submenus.  This means usual content of the screen, i.e. the 
 * combat screen and top status bar, is "faded out", the rest of 
 * the screen is cleared.  This function resolves some redundancy 
 * that occurred since there are so many submenus needing this.
 */
void InitiateMenu(const char *background_name)
{
	// Here comes the standard initializer for all the menus and submenus
	// of the big escape menu.  This prepares the screen, so that we can
	// write on it further down.
	//
	SDL_SetClipRect(Screen, NULL);

	if (!strcmp(background_name, "--GAME_BACKGROUND--")) {		// Show game background
		AssembleCombatPicture(SHOW_ITEMS | NO_CURSOR);
	} else if (!strcmp(background_name, "--EDITOR_BACKGROUND--")) {	// Show editor background
		AssembleCombatPicture(ONLY_SHOW_MAP_AND_TEXT | SHOW_ITEMS | OMIT_TUX | GameConfig.omit_obstacles_in_level_editor *
				OMIT_OBSTACLES | GameConfig.omit_enemies_in_level_editor * OMIT_ENEMIES | OMIT_BLASTS | SKIP_LIGHT_RADIUS |
				NO_CURSOR | OMIT_ITEMS_LABEL);
	} else {
		blit_background(background_name);
	}

	SDL_SetClipRect(Screen, NULL);
};				// void InitiateMenu(void)

/**
 * This function provides a convenient cheat menu, so that any 
 * tester does not have to play all through the game again and again
 * to see if a bug in a certain position has been removed or not.
 */
void Cheatmenu(void)
{
	int can_continue;
	int i, l;
	int x0, y0;
	int skip_dead = 0;
	item cheat_gun;

	// Prevent distortion of framerate by the delay coming from 
	// the time spend in the menu.
	//
	Activate_Conservative_Frame_Computation();

	// Some small font is needed, such that we can get a lot of lines on
	// one single cheat menu page...
	//
	set_current_font(FPS_Display_Font);

	x0 = 50;
	y0 = 20;

	can_continue = FALSE;
	while (!can_continue) {
		clear_screen();
		printf_SDL(Screen, x0, y0, "Current position: Level=%d, X=%d, Y=%d\n", Me.pos.z, (int)Me.pos.x, (int)Me.pos.y);
		printf_SDL(Screen, -1, -1, " f. xray_vision_for_tux: %s", GameConfig.xray_vision_for_tux ? "ON\n" : "OFF\n");
		printf_SDL(Screen, -1, -1, " g. god mode: %s\n", Me.god_mode ? "ON" : "OFF");
		printf_SDL(Screen, -1, -1, " i. Make tux invisible: %s", (Me.invisible_duration > 1) ? "ON\n" : "OFF\n");
		printf_SDL(Screen, -1, -1, " l. robot list of current level\n");
		printf_SDL(Screen, -1, -1, " L. alive robot list of current level\n");
		printf_SDL(Screen, -1, -1, " k. dead robot list of current level\n");
		printf_SDL(Screen, -1, -1, " d. destroy robots on current level\n");
		printf_SDL(Screen, -1, -1, " h. Auto-acquire all skills\n");
		printf_SDL(Screen, -1, -1, " c. Acquire 1 million circuits, current worth: %d\n", Me.Gold);
		printf_SDL(Screen, -1, -1, " n. No hidden droids: %s", show_all_droids ? "ON\n" : "OFF\n");
		printf_SDL(Screen, -1, -1, " r. Infinite running stamina: %s", GameConfig.cheat_running_stamina ? "ON\n" : "OFF\n");
		printf_SDL(Screen, -1, -1, " s. Double speed running: %s\n", GameConfig.cheat_double_speed ? "ON" : "OFF");
		printf_SDL(Screen, -1, -1, " t. Give a cheat gun\n");
		printf_SDL(Screen, -1, -1, " x. Cheatkeys : %s", GameConfig.enable_cheatkeys ? "ON\n" : "OFF\n");
		printf_SDL(Screen, -1, -1, " T. Add a training point. Current training points: %d\n", Me.points_to_distribute);
		printf_SDL(Screen, -1, -1, " e. Show enemies' states: %s", GameConfig.All_Texts_Switch ? "ON\n" : "OFF\n");
		printf_SDL(Screen, -1, -1, " q. RESUME game\n");

		// Now we show it...
		//
		our_SDL_flip_wrapper();

		switch (getchar_raw(NULL)) {
		case 'f':
			GameConfig.xray_vision_for_tux = !GameConfig.xray_vision_for_tux;
			break;
		case 'g':
			Me.god_mode = !Me.god_mode;
			break;
		case 'T':
			Me.points_to_distribute++; 
			break;
		case 'i':
			if (Me.invisible_duration == 0) {
				Me.invisible_duration += 50000;
				break;
			}
			if (Me.invisible_duration > 0) {
				Me.invisible_duration = 0.0;
				break;
			}
			break;
		case 'k':
			skip_dead = 2;
			break;
		case 'L':
			if (skip_dead == 0)
				skip_dead = 1;
			break;
		case 'l':	// robot list of this deck 
			l = 0;	// l is counter for lines of display of enemy output
			for (i = ((skip_dead == 1) ? 1 : 0); i < ((skip_dead == 2) ? 1 : 2); i++) {
				enemy *erot;
				list_for_each_entry(erot, (i) ? &alive_bots_head : &dead_bots_head, global_list) {
					if (erot->pos.z == Me.pos.z) {

						if (l && !(l % ((GameConfig.screen_height == 768) ? 25 : 16))) {
							printf_SDL(Screen, -1, -1, " --- MORE --- \n");
							our_SDL_flip_wrapper();
							if (getchar_raw(NULL) == 'q')
								break;
						}
						if (!(l % ((GameConfig.screen_height == 768) ? 25 : 16))) {
							clear_screen();
							printf_SDL(Screen, 15, y0,
								   "ID    X     Y    ENERGY   speedX  Status  Friendly An-type An-Phase \n");
							printf_SDL(Screen, -1, -1, "---------------------------------------------\n");
						}

						l++;
						if ((erot->type >= 0) && (erot->type <= Number_Of_Droid_Types)) {
							printf_SDL(Screen, 15, -1,
								   "%s  %3.1f  %3.1f  %4d  %g ",
								   Droidmap[erot->type].droidname,
								   erot->pos.x, erot->pos.y, (int)erot->energy, erot->speed.x);
						} else {
							printf_SDL(Screen, 15, -1, "SEVERE ERROR: Type=%d. ", erot->type);
						}
						if (is_friendly(erot->faction, FACTION_SELF))
							printf_SDL(Screen, -1, -1, " YES");
						else
							printf_SDL(Screen, -1, -1, "  NO");
						switch (erot->animation_type) {
						case WALK_ANIMATION:
							printf_SDL(Screen, -1, -1, " Walk");
							break;
						case ATTACK_ANIMATION:
							printf_SDL(Screen, -1, -1, " Atta");
							break;
						case GETHIT_ANIMATION:
							printf_SDL(Screen, -1, -1, " GHit");
							break;
						case DEATH_ANIMATION:
							printf_SDL(Screen, -1, -1, " Deth");
							break;
						case DEAD_ANIMATION:
							printf_SDL(Screen, -1, -1, " Dead");
							break;
						case STAND_ANIMATION:
							printf_SDL(Screen, -1, -1, " Stnd");
							break;
						default:
							printf_SDL(Screen, -1, -1, " ERROR!");
							break;
						}
						printf_SDL(Screen, -1, -1, " %4.1f", erot->animation_phase);
						printf_SDL(Screen, -1, -1, "\n");

					}	// if (enemy on current level)
				}
			}

			printf_SDL(Screen, 15, -1, " --- END --- \n");
			CountNumberOfDroidsOnShip();
			printf_SDL(Screen, 15, -1, " BTW:  Number_Of_Droids_On_Ship: %d \n", Number_Of_Droids_On_Ship);
			our_SDL_flip_wrapper();
			while ((!SpacePressed()) && (!EscapePressed()) && (!MouseLeftPressed())) ;
			while (SpacePressed() || EscapePressed() || MouseLeftPressed()) ;
			break;

		case 'd':	// destroy all robots on this level, very useful
			{
				enemy *erot, *nerot;
				BROWSE_ALIVE_BOTS_SAFE(erot, nerot) {
					if (erot->pos.z == Me.pos.z)
						hit_enemy(erot, erot->energy + 1, 0, -1, 0);
				}

				printf_SDL(Screen, -1, -1, "All robots on this deck killed!\n");
				our_SDL_flip_wrapper();
				getchar_raw(NULL);
			}
			break;

		case 'h':	// auto-aquire all skills
			for (i = 0; i < number_of_skills; i++)
				Me.skill_level[i]++;
			break;

		case 'c':	// Add 1 million circuits
			Me.Gold += 1000000;
			break;

		case 'n':	// toggle display of all droids
			show_all_droids = !show_all_droids;
			break;

		case 'r':
			GameConfig.cheat_running_stamina = !GameConfig.cheat_running_stamina;
			break;

		case 's':
			GameConfig.cheat_double_speed = !GameConfig.cheat_double_speed;
			break;

		case 't':
			cheat_gun = create_item_with_id("Cheat Gun", TRUE, 1);
			equip_item(&cheat_gun);
			break;

		case 'e':
			GameConfig.All_Texts_Switch = !GameConfig.All_Texts_Switch;
			break;

		case 'x':
			GameConfig.enable_cheatkeys = !GameConfig.enable_cheatkeys;
			break;

		case ' ':
		case 'q':
			while (QPressed())
				SDL_Delay(1);
			can_continue = 1;
			break;
		}		/* switch (getchar_raw()) */
	}			/* while (!can_continue) */

	clear_screen();
	our_SDL_flip_wrapper();

	return;
};				// void Cheatmenu() 

/***********************
 * Menus : Most of the menus are split into two parts
 * - handle, that handles the user selection
 * - fill, that creates the names of the button.
 *
 * Because there is much code that must be generated for each "menu"
 * like prototypes, enums, I use this trick to do the job, and this way
 * I'm sure there is no mismatch in the orders
 */

#define MENU_LIST MENU(Startup, STARTUP) MENU(Escape, ESCAPE)		\
    MENU(Options, OPTIONS) MENU(Resolution, RESOLUTION)			\
    MENU(Graphics, GRAPHICS) MENU(Sound, SOUND)				\
    MENU(Performance, PERFORMANCE) \
    MENU(OSD, OSD) \
    MENU(Game, GAME)

enum {
	EXIT_MENU = -2,
	CONTINUE_MENU = -1,
#define MENU(x, y) MENU_##y,
	MENU_LIST
#ifdef ENABLE_NLS
	MENU(Language, LANGUAGE)
#endif
#undef MENU
	MENU_NUM
};

#define MENU(x, y) static int x##_handle (int); static void x##_fill (char *[MAX_MENU_ITEMS]);
MENU_LIST
#ifdef ENABLE_NLS
MENU(Language, LANGUAGE)
#endif
#undef MENU
    struct Menu {
	int (*HandleSelection) (int);
	void (*FillText) (char *[MAX_MENU_ITEMS]);
};

struct Menu menus[] = {
#define MENU(x, y) { x##_handle, x##_fill },
	MENU_LIST
#ifdef ENABLE_NLS
	MENU(Language, LANGUAGE)
#endif
#undef MENU
	{NULL, NULL}
};

static void RunSubMenu(int startup, int menu_id)
{
	int can_continue = 0;
	char *texts[MAX_MENU_ITEMS];
	int i = 0;
	int pos = 1;
	while (!can_continue) {
		// We need to fill at each loop because
		// several menus change their contents
		for (i = 0; i < MAX_MENU_ITEMS; i++)
			texts[i] = (char *)malloc(1024);
		menus[menu_id].FillText(texts);

		if (startup)
			pos = do_menu_selection("", (char**)texts, -1, "title.jpg", Menu_Font);
		else
			pos = do_menu_selection("", (char**)texts, pos, "--GAME_BACKGROUND--", Menu_Font);

		int ret = menus[menu_id].HandleSelection(pos);

		for (i = 0; i < MAX_MENU_ITEMS; i++)
			free(texts[i]);

		if (ret == EXIT_MENU) {
			can_continue = TRUE;
		}

		if (ret > 0)
			RunSubMenu(startup, ret);
	}
}

static void RunMenu(int is_startup)
{
	int start_menu = is_startup ? MENU_STARTUP : MENU_ESCAPE;
	set_current_font(Menu_Font);
	Activate_Conservative_Frame_Computation();
	if (is_startup) {
		// Can the music be disabled by a submenu ?
		switch_background_music("menu.ogg");
		SDL_SetClipRect(Screen, NULL);

		if (skip_initial_menus && Single_Player_Menu())
			return;
	} else {
		while (EscapePressed()) ;
	}
	RunSubMenu(is_startup, start_menu);
	clear_screen();
}

void StartupMenu(void)
{
	RunMenu(1);
}

void EscapeMenu(void)
{
	MenuItemSelectedSound();
	RunMenu(0);
}

static int Startup_handle(int n)
{
	enum {
		SINGLE_PLAYER_POSITION = 1,
		TUTORIAL_POSITION,
		LVLEDIT_POSITION,
		OPTIONS_POSITION,
		CREDITS_POSITION,
		CONTRIBUTE_POSITION,
		EXIT_FREEDROID_POSITION
	};
	switch (n) {
	case SINGLE_PLAYER_POSITION:
		game_root_mode = ROOT_IS_GAME;
		if (Single_Player_Menu()) {
			return EXIT_MENU;
		}
		break;
	case LVLEDIT_POSITION:	//allow starting directly in leveleditor - the hack is a little dirty but it does its work.
		prepare_level_editor(game_act_get_starting());
		return EXIT_MENU;
		break;
	case TUTORIAL_POSITION:	//Similar hack to start Tutorial.
		game_root_mode = ROOT_IS_GAME;
		skip_initial_menus = 1;
		game_act_set_current(game_act_get_starting());
		char fpp[PATH_MAX];
		find_file(fpp, MAP_DIR, "levels.dat", NULL, PLEASE_INFORM | IS_FATAL);
		LoadShip(fpp, 0);
		prepare_start_of_new_game("TutorialTuxStart", TRUE);
		skip_initial_menus = 0;
		free(Me.character_name);
		Me.character_name = strdup("TutorialTux");
		return EXIT_MENU;
		break;
	case OPTIONS_POSITION:
		return MENU_OPTIONS;
	case CREDITS_POSITION:
		play_title_file(BASE_TITLES_DIR, "Credits.lua");
		break;
	case CONTRIBUTE_POSITION:
		play_title_file(BASE_TITLES_DIR, "Contribute.lua");
		break;
	case (-1):
	case EXIT_FREEDROID_POSITION:
		Terminate(EXIT_SUCCESS);
		break;
	default:
		break;
	}
	return CONTINUE_MENU;
}

static void Startup_fill(char *MenuTexts[MAX_MENU_ITEMS])
{
	int i = 0;
	strncpy(MenuTexts[i++], _("Play"), 1024);
	strncpy(MenuTexts[i++], _("Tutorial"), 1024);
	strncpy(MenuTexts[i++], _("Level Editor"), 1024);
	strncpy(MenuTexts[i++], _("Options"), 1024);
	strncpy(MenuTexts[i++], _("Credits"), 1024);
	strncpy(MenuTexts[i++], _("Contribute"), 1024);
	strncpy(MenuTexts[i++], _("Exit FreedroidRPG"), 1024);
	MenuTexts[i][0] = '\0';
}

static int Game_handle(int n)
{
	enum {
		DIFFICULTY = 1,
		LEAVE_MENU
	};
	switch (n) {
	case (-1):
		return EXIT_MENU;
		break;
	case DIFFICULTY:
		GameConfig.difficulty_level++;
		GameConfig.difficulty_level %= 3;
		game_needs_restart = TRUE;
		return CONTINUE_MENU;
	case LEAVE_MENU:
		return EXIT_MENU;
	default:
		break;
	}
	return CONTINUE_MENU;
}

static void Game_fill(char *MenuTexts[MAX_MENU_ITEMS])
{
	const char *difficulty_str[] = {
		[DIFFICULTY_EASY] = N_("easy"),
		[DIFFICULTY_NORMAL] = N_("normal"),
		[DIFFICULTY_HARD] = N_("hard"),
	};

	sprintf(MenuTexts[0], _("Difficulty: %s"), _(difficulty_str[GameConfig.difficulty_level]));
	strncpy(MenuTexts[1], _("Back"), 1024);
	MenuTexts[2][0] = '\0';
}

static int Options_handle(int n)
{
	enum {
		GAME_OPTIONS = 1,
		GRAPHICS_OPTIONS,
		SOUND_OPTIONS,
		KEYMAP_OPTIONS,
#ifdef ENABLE_NLS
		LANGUAGE_OPTIONS,
#endif
		ON_SCREEN_DISPLAYS,
		PERFORMANCE_TWEAKS_OPTIONS,
		LEAVE_OPTIONS_MENU
	};
	switch (n) {
	case (-1):
	case LEAVE_OPTIONS_MENU:
		if (game_needs_restart) {
			alert_window(_("You need to restart FreedroidRPG for some changes to take effect.\n\nSorry for the inconvenience."));
			game_needs_restart = 0;
		}
		return EXIT_MENU;
		break;
#ifdef ENABLE_NLS
	case LANGUAGE_OPTIONS:
		return MENU_LANGUAGE;
#endif
	case GAME_OPTIONS:
		return MENU_GAME;
	case GRAPHICS_OPTIONS:
		return MENU_GRAPHICS;
	case SOUND_OPTIONS:
		return MENU_SOUND;
	case KEYMAP_OPTIONS:
		keychart();
		return CONTINUE_MENU;
	case ON_SCREEN_DISPLAYS:
		return MENU_OSD;
	case PERFORMANCE_TWEAKS_OPTIONS:
		return MENU_PERFORMANCE;
	default:
		break;
	}
	return CONTINUE_MENU;
}

static void Options_fill(char *MenuTexts[MAX_MENU_ITEMS])
{
	int i = 0;
	strncpy(MenuTexts[i++], _("Gameplay"), 1024);
	strncpy(MenuTexts[i++], _("Graphics"), 1024);
	strncpy(MenuTexts[i++], _("Sound"), 1024);
	strncpy(MenuTexts[i++], _("Keys"), 1024);
#ifdef ENABLE_NLS
	strncpy(MenuTexts[i++], _("Languages"), 1024);
#endif
	strncpy(MenuTexts[i++], _("On-Screen displays"), 1024);
	strncpy(MenuTexts[i++], _("Performance tweaks"), 1024);
	strncpy(MenuTexts[i++], _("Back"), 1024);
	MenuTexts[i][0] = '\0';
}

static int Escape_handle(int n)
{
	enum {
		RESUME_GAME_POSITION = 1,
		SAVE_GAME_POSITION,
		OPTIONS_POSITION,
		LOAD_GAME_POSITION,
		LOAD_BACKUP_POSITION,
		NEW_GAME_POSITION,
		QUIT_POSITION
	};
	switch (n) {
	case (-1):
	case (RESUME_GAME_POSITION):
		return EXIT_MENU;
	case OPTIONS_POSITION:
		return MENU_OPTIONS;
	case LOAD_GAME_POSITION:
		load_game();
		return EXIT_MENU;
	case LOAD_BACKUP_POSITION:
		load_backup_game();
		return EXIT_MENU;
	case NEW_GAME_POSITION:
		GameOver = TRUE;
		return EXIT_MENU;
	case SAVE_GAME_POSITION:
		save_game();
		break;
	case QUIT_POSITION:
		DebugPrintf(2, "\nvoid EscapeMenu( void ): Quit requested by user.  Terminating...");
		Terminate(EXIT_SUCCESS);
		break;
	default:
		break;
	}

	return CONTINUE_MENU;
}

static void Escape_fill(char *MenuTexts[MAX_MENU_ITEMS])
{
	if (game_root_mode == ROOT_IS_GAME)
		strncpy(MenuTexts[0], _("Resume Play"), 1024);
	else
		strncpy(MenuTexts[0], _("Resume Playtest"), 1024);

	strncpy(MenuTexts[1], _("Save Hero"), 1024);
	strncpy(MenuTexts[2], _("Options"), 1024);
	strncpy(MenuTexts[3], _("Load Latest"), 1024);
	strncpy(MenuTexts[4], _("Load Backup"), 1024);
	if (game_root_mode == ROOT_IS_GAME)
		strncpy(MenuTexts[5], _("Quit to Main Menu"), 1024);
	else
		strncpy(MenuTexts[5], _("Return to Level Editor"), 1024);
	strncpy(MenuTexts[6], _("Exit FreedroidRPG"), 1024);
	MenuTexts[7][0] = '\0';
}

#ifdef ENABLE_NLS
static int Language_handle(int n)
{
	if (n == -1)
		return EXIT_MENU;

	n -= 2; // Second menu entry is index 0 of the language list

	// Do nothing if 'Back' was selected
	if (n >= lang_specs.size)
		return EXIT_MENU;

	int encoding_changed = FALSE;

	if (n == -1) {
		// 'System default' was selected
		lang_set("", &encoding_changed);
	}  else {
		// Else, use the selected language
		struct langspec *lang = dynarray_member(&lang_specs, n, sizeof(struct langspec));
		lang_set(lang->locale, &encoding_changed);
	}

	if (encoding_changed) {
		free_fonts();
		init_fonts();
	}

	return CONTINUE_MENU;
}
#endif

#ifdef ENABLE_NLS
static void Language_fill(char *MenuTexts[MAX_MENU_ITEMS])
{
	int i = 0;
	int l;
	char mark;

	mark = ' ';
	if (!GameConfig.locale || strlen(GameConfig.locale) == 0)
		mark = '*';
	snprintf(MenuTexts[i++], 1024, "%c %s  ", mark, _("System default"));

	for (l = 0; l < lang_specs.size; l++) {
		struct langspec *lang = dynarray_member(&lang_specs, l, sizeof(struct langspec));
		mark = ' ';
		if (GameConfig.locale && !strcmp(GameConfig.locale, lang->locale))
			mark = '*';
		snprintf(MenuTexts[i++], 1024, "%c %s  ", mark, lang->name);
	}
	snprintf(MenuTexts[i++], 1024, "  %s  ", _("Back"));
	MenuTexts[i][0] = '\0';
}
#endif

extern screen_resolution screen_resolutions[];

static int Resolution_handle(int n)
{
	if (n == -1)
		return EXIT_MENU;

	// First value of 'n' is '1', so decrement it.
	--n;

	int nb_res = 0;
	int nb_supported_res = 0;
	while (screen_resolutions[nb_res].xres != -1) {
		if (screen_resolutions[nb_res].supported)
			++nb_supported_res;
		++nb_res;
	}

	// + 2 here accounts for 'Back' and end-of-list marker
	int offset = max(0, nb_supported_res - MAX_MENU_ITEMS + 2);

	// Last menu entry is 'Back'
	if (n == nb_supported_res - offset) {
		while (EnterPressed() || SpacePressed()) ;
		return EXIT_MENU;
	}
	// Wrong entry. How is it possible ???
	if (n < 0 || n > nb_supported_res)
		return CONTINUE_MENU;

	int i, j;
	for (i = offset, j = 0; i < nb_res; ++i) {
		// Only supported screen resolution are displayed
		if (!screen_resolutions[i].supported)
			continue;

		// Is the current supported resolution the selected one ?
		if (n == j) {
			while (EnterPressed() || SpacePressed()) ;
			GameConfig.next_time_width_of_screen = screen_resolutions[i].xres;
			GameConfig.next_time_height_of_screen = screen_resolutions[i].yres;
			game_needs_restart = TRUE;
			return CONTINUE_MENU;
		}
		++j;
	}

	return CONTINUE_MENU;
}

static void Resolution_fill(char *MenuTexts[MAX_MENU_ITEMS])
{
	int i = 0;
	int j = 0;
	int nb_res = 0;

	// Count the resolutions, stop on our delimiter
	for (i = 0; screen_resolutions[i].xres != -1; i++);
	nb_res = i;

	// we require an offset in case we need to truncate the beginning of the list
	// '-2' to make room for 'Back' and end-of-list marker
	int offset = max(0, nb_res - (MAX_MENU_ITEMS - 2));
	i = offset;

	// '-2' here accounts for 'Back' and end-of-list marker
	while (i < nb_res && j < (MAX_MENU_ITEMS - 2)) {
		//Only supported screen resolution are displayed
		if (screen_resolutions[i].supported) {
			char flag = ' ';
			if (GameConfig.next_time_width_of_screen == screen_resolutions[i].xres &&
			    GameConfig.next_time_height_of_screen == screen_resolutions[i].yres) {
				flag = '*';
			}
			sprintf(MenuTexts[j++], "%c %dx%d", flag, screen_resolutions[i].xres, screen_resolutions[i].yres);
		}
		++i;
	}
	// Append 'Back' option
	strncpy(MenuTexts[j++], _("Back"), 1024);
	MenuTexts[j][0] = '\0';
}

static int Graphics_handle(int n)
{
	enum {
		CHANGE_SCREEN_RESOLUTION = 1,
		SET_FULLSCREEN_FLAG,
		SET_GAMMA_CORRECTION,
		SET_SHOW_BLOOD_FLAG,
		LEAVE_OPTIONS_MENU
	};
	switch (n) {
	case (-1):
		return EXIT_MENU;
	case SET_GAMMA_CORRECTION:
		if (RightPressed()) {
			while (RightPressed()) ;
			GameConfig.current_gamma_correction += 0.05;
			SDL_SetGamma(GameConfig.current_gamma_correction, GameConfig.current_gamma_correction,
				     GameConfig.current_gamma_correction);
		}

		if (LeftPressed()) {
			while (LeftPressed()) ;
			GameConfig.current_gamma_correction -= 0.05;
			if (GameConfig.current_gamma_correction < 0.0) {
				GameConfig.current_gamma_correction = 0.0;
			}
			SDL_SetGamma(GameConfig.current_gamma_correction, GameConfig.current_gamma_correction,
				     GameConfig.current_gamma_correction);
		}

		break;

	case SET_FULLSCREEN_FLAG:
		while (EnterPressed() || SpacePressed()) ;
		GameConfig.fullscreen_on = !GameConfig.fullscreen_on;
#ifndef __WIN32__
		SDL_WM_ToggleFullScreen(Screen);
#else
		game_needs_restart = TRUE;
#endif
		break;

	case CHANGE_SCREEN_RESOLUTION:
		while (EnterPressed() || SpacePressed()) ;
		return MENU_RESOLUTION;

	case SET_SHOW_BLOOD_FLAG:
		while (EnterPressed() || SpacePressed()) ;
		GameConfig.show_blood = !GameConfig.show_blood;
		break;

	case LEAVE_OPTIONS_MENU:
		while (EnterPressed() || SpacePressed()) ;
		return EXIT_MENU;
		break;

	default:
		break;

	}
	return CONTINUE_MENU;
}

static void Graphics_fill(char *MenuTexts[MAX_MENU_ITEMS])
{
	char Options[20][1000];
	int i = 0;
	sprintf(MenuTexts[i++], _("Change screen resolution"));

	sprintf(Options[i], _("Fullscreen mode: %s"), GameConfig.fullscreen_on ? _("ON") : _("OFF"));
	strncpy(MenuTexts[i], Options[i], 1024);
	i++;

	sprintf(Options[i], _("<-- Gamma correction: %1.2f -->"), GameConfig.current_gamma_correction);
	strncpy(MenuTexts[i], Options[i], 1024);
	i++;

	sprintf(Options[i], _("Show blood: %s"), GameConfig.show_blood ? _("YES") : _("NO"));
	strncpy(MenuTexts[i], Options[i], 1024);
	i++;

	strncpy(MenuTexts[i++], _("Back"), 1024);
	MenuTexts[i][0] = '\0';
}

static int Sound_handle(int n)
{
	enum {
		SET_BG_MUSIC_VOLUME = 1,
		SET_SOUND_FX_VOLUME,
		SET_SOUND_OUTPUT_FMT,
		LEAVE_OPTIONS_MENU
	};
	switch (n) {
	case (-1):
		return EXIT_MENU;
	case SET_BG_MUSIC_VOLUME:
		if (RightPressed()) {
			while (RightPressed()) ;
			GameConfig.Current_BG_Music_Volume += 0.05;
			if (GameConfig.Current_BG_Music_Volume > 1.0)
				GameConfig.Current_BG_Music_Volume = 1.0;
			set_music_volume(GameConfig.Current_BG_Music_Volume);
		}

		if (LeftPressed()) {
			while (LeftPressed()) ;
			GameConfig.Current_BG_Music_Volume -= 0.05;
			if (GameConfig.Current_BG_Music_Volume < 0.0)
				GameConfig.Current_BG_Music_Volume = 0.0;
			set_music_volume(GameConfig.Current_BG_Music_Volume);
		}

		break;

	case SET_SOUND_FX_VOLUME:
		if (RightPressed()) {
			while (RightPressed()) ;
			GameConfig.Current_Sound_FX_Volume += 0.05;
			if (GameConfig.Current_Sound_FX_Volume > 1.0)
				GameConfig.Current_Sound_FX_Volume = 1.0;
			set_SFX_volume(GameConfig.Current_Sound_FX_Volume);
		}

		if (LeftPressed()) {
			while (LeftPressed()) ;
			GameConfig.Current_Sound_FX_Volume -= 0.05;
			if (GameConfig.Current_Sound_FX_Volume < 0.0)
				GameConfig.Current_Sound_FX_Volume = 0.0;
			set_SFX_volume(GameConfig.Current_Sound_FX_Volume);
		}
		break;

	case SET_SOUND_OUTPUT_FMT:
		if (RightPressed()) {
			while (RightPressed()) ;
			GameConfig.Current_Sound_Output_Fmt =
				(GameConfig.Current_Sound_Output_Fmt + 1) % ALL_SOUND_OUTPUTS;
			game_needs_restart = TRUE;
		}

		if (LeftPressed()) {
			while (LeftPressed()) ;
			GameConfig.Current_Sound_Output_Fmt =
				(GameConfig.Current_Sound_Output_Fmt - 1) == -1 ? ALL_SOUND_OUTPUTS-1 : (GameConfig.Current_Sound_Output_Fmt - 1);
			game_needs_restart = TRUE;
		}
		break;

	case LEAVE_OPTIONS_MENU:
		while (EnterPressed() || SpacePressed()) ;
		return EXIT_MENU;
		break;

	default:
		break;
	}
	return CONTINUE_MENU;
}

static void Sound_fill(char *MenuTexts[MAX_MENU_ITEMS])
{
	char Options[20][1000];
	int i = 0;
	sprintf(Options[i], _("<-- Background music volume: %1.2f -->"), GameConfig.Current_BG_Music_Volume);
	strncpy(MenuTexts[i], Options[i], 1024);
	i++;

	sprintf(Options[i], _("<-- Sound effects volume: %1.2f -->"), GameConfig.Current_Sound_FX_Volume);
	strncpy(MenuTexts[i], Options[i], 1024);
	i++;

	switch (GameConfig.Current_Sound_Output_Fmt) {
	case SOUND_OUTPUT_FMT_STEREO:
		sprintf(Options[i], _("<-- Output: Stereo -->"));
		break;

	case SOUND_OUTPUT_FMT_SURROUND40:
		sprintf(Options[i], _("<-- Output: 4.0 Surround -->"));
		break;

	case SOUND_OUTPUT_FMT_SURROUND51:
		sprintf(Options[i], _("<-- Output: 5.1 Surround -->"));
		break;
		
	default:
		sprintf(Options[i], _("<-- Output: Error -->"));
		break;
	}
	strncpy(MenuTexts[i], Options[i], 1024);
	i++;

	strncpy(MenuTexts[i++], _("Back"), 1024);
	MenuTexts[i][0] = '\0';
}

static int Performance_handle(int n)
{
	enum {
		SET_FRAMERATE_LIMIT = 1,
		SKIP_LIGHT_RADIUS_MODE,
		SKIP_SHADOWS,
		TOGGLE_LAZYLOAD,
		LEAVE_PERFORMANCE_TWEAKS_MENU
	};
	switch (n) {
	case (-1):
		return EXIT_MENU;

	case SET_FRAMERATE_LIMIT:
		if (LeftPressed()) {
			while (LeftPressed());
			if (GameConfig.framerate_limit >= 10) {
				GameConfig.framerate_limit -= 10;
				SDL_setFramerate(&SDL_FPSmanager, GameConfig.framerate_limit);
			}
		} else if (RightPressed()) {
			while (RightPressed());
			if (GameConfig.framerate_limit <= (FPS_UPPER_LIMIT - 10)) {
				GameConfig.framerate_limit += 10;
				SDL_setFramerate(&SDL_FPSmanager, GameConfig.framerate_limit);
			}
		}
		break;

	case SKIP_LIGHT_RADIUS_MODE:
		while (EnterPressed() || SpacePressed() || MouseLeftPressed()) ;
		GameConfig.skip_light_radius = !GameConfig.skip_light_radius;
		break;

	case SKIP_SHADOWS:
		while (EnterPressed() || SpacePressed() || MouseLeftPressed()) ;
		GameConfig.skip_shadow_blitting = !GameConfig.skip_shadow_blitting;
		break;

	case LEAVE_PERFORMANCE_TWEAKS_MENU:
		while (EnterPressed() || SpacePressed() || MouseLeftPressed()) ;
		return EXIT_MENU;
		break;

	case TOGGLE_LAZYLOAD:
		GameConfig.lazyload = !GameConfig.lazyload;
		break;

	default:
		break;

	}

	return CONTINUE_MENU;
}

static void Performance_fill(char *MenuTexts[])
{
	char Options[20][1000];
	int i = 0;
	if (GameConfig.framerate_limit == 0)
		sprintf(Options[i], _("FPS limit: none"));
	else
		sprintf(Options[i], _("FPS limit: %d"), GameConfig.framerate_limit);
	strncpy(MenuTexts[i], Options[i], 1024);
	i++;
	sprintf(Options[i], _("Show light radius: %s"), GameConfig.skip_light_radius ? _("NO") : _("YES"));
	strncpy(MenuTexts[i], Options[i], 1024);
	i++;
	sprintf(Options[i], _("Show obstacle shadows: %s"), GameConfig.skip_shadow_blitting ? _("NO") : _("YES"));
	strncpy(MenuTexts[i], Options[i], 1024);
	i++;
	// was before:  sprintf(MenuTexts[i], _("Graphics lazy loading: %s"), GameConfig.lazyload ? _("YES") : _("NO"));
	// moved to performance settings from graphics settings
	sprintf(MenuTexts[i], _("Precache item graphics: %s"), GameConfig.lazyload ? _("NO") : _("YES"));
	i++;
	strncpy(MenuTexts[i++], _("Back"), 1024);
	MenuTexts[i][0] = '\0';
}

static int OSD_handle(int n)
{
	enum {
		SHOW_POSITION = 1,
		SHOW_FRAMERATE,
		SHOW_ENEMY_ENERGY_BARS,
		SHOW_EFFECT_COUNTDOWNS,
		PARALLEL_BIG_SCREEN_MESSAGES_AT_MOST_POSITION,
		BIG_SCREEN_MESSAGES_DURATION_POSITION,
		LEAVE_OPTIONS_MENU
	};

	int *shows[5] = {
		&GameConfig.Draw_Position, &GameConfig.Draw_Framerate,
		&GameConfig.enemy_energy_bars_visible, &GameConfig.effect_countdowns_visible
	};

	if (n >= 1 && n < PARALLEL_BIG_SCREEN_MESSAGES_AT_MOST_POSITION) {
		while (EnterPressed() || SpacePressed()) ;
		*shows[n - 1] = !*shows[n - 1];
	} else {
		switch (n) {
		case -1:
			return EXIT_MENU;
		case PARALLEL_BIG_SCREEN_MESSAGES_AT_MOST_POSITION:
			if (LeftPressed()) {
				while (LeftPressed()) ;
				if (GameConfig.number_of_big_screen_messages > 0) {
					GameConfig.number_of_big_screen_messages--;
				}
			} else if (RightPressed()) {
				while (RightPressed()) ;
				if (GameConfig.number_of_big_screen_messages < MAX_BIG_SCREEN_MESSAGES) {
					GameConfig.number_of_big_screen_messages++;
				}
			}
			break;

		case BIG_SCREEN_MESSAGES_DURATION_POSITION:
			if (LeftPressed()) {
				while (LeftPressed()) ;
				if (GameConfig.delay_for_big_screen_messages >= 0.5) {
					GameConfig.delay_for_big_screen_messages -= 0.5;
				}
			} else if (RightPressed()) {
				while (RightPressed()) ;
				GameConfig.delay_for_big_screen_messages += 0.5;
			}
			break;

		case LEAVE_OPTIONS_MENU:
			while (EnterPressed() || SpacePressed()) ;
			return EXIT_MENU;
		default:
			break;
		}
	}
	return CONTINUE_MENU;
}

static void OSD_fill(char *MenuTexts[MAX_MENU_ITEMS])
{
	int i = 0;
	sprintf(MenuTexts[i++], _("Show position: %s"), GameConfig.Draw_Position ? _("YES") : _("NO"));
	sprintf(MenuTexts[i++], _("Show framerate: %s"), GameConfig.Draw_Framerate ? _("YES") : _("NO"));
	sprintf(MenuTexts[i++], _("Show enemies' energy bars: %s"), GameConfig.enemy_energy_bars_visible ? _("YES") : _("NO"));
	sprintf(MenuTexts[i++], _("Show effect countdowns: %s"), GameConfig.effect_countdowns_visible ? _("YES") : _("NO"));
	sprintf(MenuTexts[i++], _("<-- Max. screen messages: %d -->"), GameConfig.number_of_big_screen_messages);
	sprintf(MenuTexts[i++], _("<-- Screen message lifetime: %3.1fs -->"), GameConfig.delay_for_big_screen_messages);
	strncpy(MenuTexts[i++], _("Back"), 1024);
	MenuTexts[i][0] = '\0';
}





#define FIRST_MIS_SELECT_ITEM_POS_X (0.0)
#define FIRST_MIS_SELECT_ITEM_POS_Y (BANNER_HEIGHT + FontHeight(Menu_BFont))

/**
 * @return true if there is already a save game using specified name
 */
static int savegame_exists(const char *name)
{
	struct dirent **eps = NULL;
	int n = find_saved_games(&eps);
	int exists = 0;
	int i;
	for (i = 0; i < n; i++) {
		if (strcmp(eps[i]->d_name, name) == 0) {
			exists = 1;
			break;
		}
	}
	for (i = 0; i <n; i++)
		free(eps[i]);
	free(eps);
	return exists;
}

/**
 * Prompt the user for the name of a new character.
 */
static char *get_new_character_name(void)
{
	char *str;
	InitiateMenu("title.jpg");

	// Loop until the player enters a name that does not already exist.
	int loop = 1;
	while (loop) {
		if (!skip_initial_menus)
			str = get_string(MAX_CHARACTER_NAME_LENGTH - 1, "title.jpg", _("\n\
     Please enter a name\n\
     for the new hero: \n\n\
        ENTER to accept.\n\
        ESCAPE to cancel.\n\n\
     > "));
		else
			str = strdup("MapEd");

		// User cancelled -- abort here
		if (!str)
			break;

		// Parse string for illegal chars
		int i;
		for (i = 0; i < strlen(str); i++)
			if (!isalnum(str[i]) && str[i] != '-')
				str[i] = '-';

		// Check if name already exists
		loop = savegame_exists(str);
		if (loop) {
			alert_window(_("A character named \"%s\" already exists.\nPlease choose another name."), str);
			free(str);
		}
	}

	return str;
}

int load_named_game(const char *name)
{
	if (strlen(name) >= MAX_CHARACTER_NAME_LENGTH) {
		error_message(__FUNCTION__, "The saved game name is too long.", NO_REPORT);
		return ERR;
	}

	if (!Me.character_name || strcmp(name, Me.character_name)) {
		free(Me.character_name);
		Me.character_name = strdup(name);
	}

	if (load_game() != OK)
		return ERR;

	GetEventTriggers("events.dat");
	item_held_in_hand = NULL;
	return OK;
}

enum {
	SAVEGAME_LOAD,
	SAVEGAME_DELETE,
};

static int do_savegame_selection_and_act(int action)
{
	char *menu_title = NULL;
	switch (action) {
	case SAVEGAME_LOAD:
		menu_title = LOAD_EXISTING_HERO_STRING;
		break;
	case SAVEGAME_DELETE:
		menu_title = DELETE_EXISTING_HERO_STRING;
		break;
	}

	InitiateMenu("title.jpg");

	// We use empty strings to denote the end of any menu selection, 
	// therefore also for the end of the list of saved characters.
	//
	char *MenuTexts[MAX_MENU_ITEMS];
	for (int n = 0; n < MAX_SAVEGAMES_PER_PAGE + 1; n++) {
		MenuTexts[n] = "";
	}

	struct dirent **eps;
	int savegames_count = find_saved_games(&eps);

	if (savegames_count > 0) {
		int list_offset = 0;

		for(;;) {
			int up_position = -1;
			int down_position = -1;
			int back_position = -1;
			int menu_cpt = 0;

			if (list_offset != 0) {
				/* Display "up" */
				up_position = menu_cpt;
				MenuTexts[menu_cpt++] = _("[up]");
			} else {
				up_position = -1;
				MenuTexts[menu_cpt++] = " ";
			}

			for (int i = 0, savegame_idx = list_offset; (i < MAX_SAVEGAMES_PER_PAGE) && (savegame_idx < savegames_count); i++, savegame_idx++) {
				MenuTexts[menu_cpt++] = eps[savegame_idx]->d_name;
			}

			if (list_offset + MAX_SAVEGAMES_PER_PAGE < savegames_count) {
				/* Display "down" */
				down_position = menu_cpt;
				MenuTexts[menu_cpt++] = _("[down]");
			} else {
				down_position = -1;
				MenuTexts[menu_cpt++] = " ";
			}

			back_position = menu_cpt;
			MenuTexts[menu_cpt++] = _("Back");
			MenuTexts[menu_cpt] = "";

			// Note: DoMenuSelection() returned value is the menu index + 1,
			// or -1 if the menu was escaped
			int MenuPosition = do_menu_selection(menu_title, (char **)MenuTexts, 1, "title.jpg", NULL) - 1;
			if (MenuPosition == -2 || MenuPosition == back_position) {
				for (int i = 0; i < savegames_count; i++)
					free(eps[i]);
				free(eps);
				return FALSE;
			}

			if (MenuPosition == down_position) {
				list_offset++;
			} else if (MenuPosition == up_position) {
				list_offset--;
			} else {
				free(Me.character_name);
				Me.character_name = strdup(MenuTexts[MenuPosition]);
				break;
			}
		}

		for (int i = 0; i < savegames_count; i++)
			free(eps[i]);
		free(eps);

		goto do_action;

	} else {

		MenuTexts[0] = _("Back");
		MenuTexts[1] = "";

		do_menu_selection(_("No saved games found!"), (char **)MenuTexts, 1, "title.jpg", NULL);

		return FALSE;
	}

	return FALSE;

do_action:
	;
	int rtn = FALSE;
	switch (action) {
	case SAVEGAME_LOAD:
		if (load_named_game(Me.character_name) == OK) {
			rtn = TRUE;
		} else {
			rtn = FALSE;
		}
		break;
	case SAVEGAME_DELETE:

		// We do a final safety check to ask for confirmation.
		MenuTexts[0] = _("Sure!");
		MenuTexts[1] = _("Back");
		MenuTexts[2] = "";
		char SafetyText[500];
		sprintf(SafetyText, _("Really delete hero '%s'?"), Me.character_name);
		int FinalDecision = do_menu_selection(SafetyText, (char **)MenuTexts, 1, "title.jpg", NULL);

		if (FinalDecision == 1)
			delete_game();
		rtn = TRUE;
		break;
	}

	our_SDL_flip_wrapper();
	return rtn;
}

/**
 * Load a savegame
 */
static int Load_Existing_Hero_Menu(void)
{
	return do_savegame_selection_and_act(SAVEGAME_LOAD);
}

/** 
 * Delete a savegame
 */
static int Delete_Existing_Hero_Menu(void)
{
	return do_savegame_selection_and_act(SAVEGAME_DELETE);
}

/**
 * This function provides the single player menu.  It offers to start a
 * new hero, to load an old one and to go back.
 *
 * The return value indicates, whether the calling function (StartupMenu)
 * can really enter the new game after this function (cause a new player
 * has been set up properly) or not, cause no player is specified yet and
 * nothing is known.
 *
 */
int Single_Player_Menu(void)
{
	int can_continue = FALSE;
	int MenuPosition = 1;
	char *MenuTexts[MAX_MENU_ITEMS];

	enum {
		NEW_HERO_POSITION = 1,
		LOAD_EXISTING_HERO_POSITION,
		DELETE_EXISTING_HERO_POSITION,
		BACK_POSITION
	};

	MenuTexts[0] = _("New Hero");
	MenuTexts[1] = _("Load existing Hero");
	MenuTexts[2] = _("Delete existing Hero");
	MenuTexts[3] = _("Back");
	MenuTexts[4] = "";

	while (!can_continue) {

		if (!skip_initial_menus)
			MenuPosition = do_menu_selection("", (char **)MenuTexts, 1, "title.jpg", Menu_Font);
		else
			MenuPosition = NEW_HERO_POSITION;

		switch (MenuPosition) {
		case NEW_HERO_POSITION:
			while (EnterPressed() || SpacePressed()) ;
			char *char_name = get_new_character_name();
			if (char_name && strlen(char_name)) {
				game_act_set_current(game_act_get_starting());
				char fp[PATH_MAX];
				find_file(fp, MAP_DIR, "levels.dat", NULL, PLEASE_INFORM | IS_FATAL);
				LoadShip(fp, 0);
				prepare_start_of_new_game("NewTuxStartGameSquare", TRUE);
				free(Me.character_name);
				Me.character_name = strdup(char_name);
				can_continue = TRUE;
			}
			if (char_name)
				free(char_name);
			break;

		case LOAD_EXISTING_HERO_POSITION:
			while (EnterPressed() || SpacePressed()) ;
			if (Load_Existing_Hero_Menu() == TRUE)
				can_continue = TRUE;
			break;

		case DELETE_EXISTING_HERO_POSITION:
			while (EnterPressed() || SpacePressed()) ;
			Delete_Existing_Hero_Menu();
			break;

		case (-1):
		case BACK_POSITION:
			while (EnterPressed() || SpacePressed() || EscapePressed()) ;
			return FALSE;
			break;
		default:
			break;
		}
	}
	return TRUE;
}

#undef _menu_c
