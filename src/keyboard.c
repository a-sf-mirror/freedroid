/*
 * Copyright 2006, 2007, 2008 Edgar Simo Serra
 * Copyright 2008 Arthur Huillet
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file keyboard.c
 *
 * @brief Handles all the keybindings and input.
 */

#define _keyboard_c

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_beautify_actions.h"
#include "lvledit/lvledit_tools.h"
#include "lvledit/lvledit_widgets.h"
#include "lvledit/lvledit_validator.h"

#define KEY_PRESS    ( 1.) /**< Key is pressed. */
#define KEY_RELEASE  (-1.) /**< Key is released. */
#define ISKEYPAD(k) (k >= SDLK_KP0 && k <= SDLK_KP_EQUALS)

static int input_keyboard_locked; // Block the execution of the keybindings.

/* name of each keybinding */
const char *keybindNames[] = {
	/* General */
	"keychart", "fullscreen", "quit", "wall_transparency", "grab_input", "take_screenshot",

	/* Ingame */
	"inventory", "skill", "character", "quests",
	"reload", "autorun",
	"move_north", "move_south", "move_east", "move_west",
	"quicksave", "quickload", "pause",
	"automap",
	"show_item_labels",
	"activate_program0", "activate_program1", "activate_program2", "activate_program3",
	"activate_program4", "activate_program5", "activate_program6", "activate_program7",
	"quick_inventory0", "quick_inventory1", "quick_inventory2",
	"quick_inventory3", "quick_inventory4", "quick_inventory5",
	"quick_inventory6", "quick_inventory7", "quick_inventory8",
	"quick_inventory9", "close_windows", "game_escape_menu",

	/* Leveleditor */
	"drop_item",
	"place_obstacle_kp1", "place_obstacle_kp2", "place_obstacle_kp3",
	"place_obstacle_kp4", "place_obstacle_kp5", "place_obstacle_kp6",
	"place_obstacle_kp7", "place_obstacle_kp8", "place_obstacle_kp9",
	"change_obstacle_label", "change_map_label", "zoom_out",
	"cycle_marked_object",
	"cut", "copy", "paste",
	"next_selection_type", "previous_selection_type",
	"next_tab", "previous_tab", "undo", "redo", "beautify_grass", "beautify_water",
	"toggle_waypoint", "toggle_waypoint_randomspawn", "connect_waypoint",
	"toolbar_scroll_left", "toolbar_scroll_right", "toolbar_step_left", "toolbar_step_right",
	"run_map_validator",

	/* Cheat keys */
	"cheat_xp+_1k", "cheat_xp*_2",
	"cheat_melee", "cheat_range", "cheat_programing", "cheat_melee_down", "cheat_range_down", "cheat_programing_down",
	"cheat_drop_random_item", "cheat_drop_random_magical_item",
	"cheat_respawn_level",
	"cheat_menu", "cheat_level_editor",
	"cheat_reload_graphics",
	NULL
};				/* must terminate with NULL */

/**
 * @fn void input_keyboard_init (void)
 *
 * @brief Initializes the input subsystem (does not set keys).
 */
void input_keyboard_init(void)
{
	int i;

	GameConfig.input_keybinds[0].name = NULL;

	if (sizeof(keybindNames) / sizeof(keybindNames[0]) > sizeof(GameConfig.input_keybinds) / sizeof(GameConfig.input_keybinds[0]))
		ErrorMessage(__FUNCTION__,
			     "There are %zu keyboard commands defined in keyboard.c, but GameConfig structure only supports %zu.\n",
			     PLEASE_INFORM, IS_FATAL,
			     sizeof(keybindNames) / sizeof(keybindNames[0]),
			     sizeof(GameConfig.input_keybinds) / sizeof(GameConfig.input_keybinds[0]));

	/* creates a null keybinding for each */
	for (i = 0; keybindNames[i] != NULL; i++) {
		GameConfig.input_keybinds[i].name = (char *)keybindNames[i];
		GameConfig.input_keybinds[i].key = SDLK_UNKNOWN;
		GameConfig.input_keybinds[i].mod = KMOD_NONE;
	}

	GameConfig.input_keybinds[i].name = NULL;

	input_keyboard_locked = 0;
}

/**
 * @fn void input_exit (void)
 *
 * @brief Exits the input subsystem.
 */
void input_exit(void)
{
}

/**
 *
 * @brief Binds key to action keybind.
 *
 *    @param keybind The name of the keybind defined above.
 *    @param key The key to bind to.
 *    @param mod the modifier associated to the key
 */
void input_set_keybind(char *keybind, SDLKey key, SDLMod mod)
{
	int i;
	for (i = 0; keybindNames[i] != NULL; i++)
		if (strcmp(keybind, GameConfig.input_keybinds[i].name) == 0) {
			GameConfig.input_keybinds[i].key = key;
			GameConfig.input_keybinds[i].mod = mod;
			return;
		}
	ErrorMessage(__FUNCTION__, "Unable to set keybinding '%s', that command doesn't exist.\n", NO_NEED_TO_INFORM, IS_WARNING_ONLY,
		     keybind);
}

void input_get_keybind(const char *cmdname, SDLKey * key, SDLMod * mod)
{
	int i;
	for (i = 0; keybindNames[i] != NULL; i++) {
		if (!strcmp(cmdname, GameConfig.input_keybinds[i].name)) {
			if (key)
				*key = GameConfig.input_keybinds[i].key;
			if (mod)
				*mod = GameConfig.input_keybinds[i].mod;
			return;
		}
	}
	ErrorMessage(__FUNCTION__, "Unable to get keybinding for command '%s', that command does not exist.\n", PLEASE_INFORM, IS_FATAL,
		     cmdname);

	/* This dead code writes "key" and "mod" to shut up GCC */
	*key = GameConfig.input_keybinds[0].key;
	*mod = GameConfig.input_keybinds[0].mod;
}

/**
 * Print the key mapping of a given command into a string.
 * @param cmd name of the command
 * @param out pointer to a sufficiently large buffer to hold the key string
 */
void input_get_keybind_string(const char *cmd, char *out)
{
	SDLKey key;
	SDLMod mod;

	input_get_keybind(cmd, &key, &mod);

	const char *ctrl_modstr = "";
	const char *alt_modstr = "";
	const char *shift_modstr = "";

	if (mod & KMOD_LCTRL || mod & KMOD_RCTRL)
		ctrl_modstr = "C-";
	if (mod & KMOD_LALT || mod & KMOD_RALT)
		alt_modstr = "A-";
	if (mod & KMOD_LSHIFT || mod & KMOD_RSHIFT)
		shift_modstr = "S-";

	sprintf(out, "%s%s%s%s", ctrl_modstr, alt_modstr, shift_modstr, SDL_GetKeyName(key));
}


/**
 * @fn void input_setDefault (void)
 *
 * @brief Sets the default input keys.
 */
void input_set_default(void)
{
	/*
	   If changing defaults, please make sure to update relevant parts in
	   README 
	   map/titles/StartOfGame.title
	   map/titles/level_editor_help.title
	 */

	input_set_keybind("keychart", SDLK_F1, KMOD_NONE);
	input_set_keybind("fullscreen", SDLK_F2, KMOD_NONE);
	input_set_keybind("grab_input", SDLK_g, KMOD_LCTRL);
	input_set_keybind("quit", SDLK_q, KMOD_LCTRL);
	input_set_keybind("wall_transparency", SDLK_t, KMOD_NONE);
	input_set_keybind("take_screenshot", SDLK_PRINT, KMOD_NONE);

	/* Game */
	input_set_keybind("inventory", SDLK_i, KMOD_NONE);
	input_set_keybind("skill", SDLK_s, KMOD_NONE);
	input_set_keybind("character", SDLK_c, KMOD_NONE);
	input_set_keybind("quests", SDLK_q, KMOD_NONE);
	input_set_keybind("reload", SDLK_r, KMOD_NONE);
	input_set_keybind("autorun", SDLK_u, KMOD_NONE);

	input_set_keybind("move_north", SDLK_UP, KMOD_NONE);
	input_set_keybind("move_south", SDLK_DOWN, KMOD_NONE);
	input_set_keybind("move_east", SDLK_RIGHT, KMOD_NONE);
	input_set_keybind("move_west", SDLK_LEFT, KMOD_NONE);

	input_set_keybind("quicksave", SDLK_F3, KMOD_NONE);
	input_set_keybind("quickload", SDLK_F4, KMOD_NONE);
	input_set_keybind("pause", SDLK_p, KMOD_NONE);
	input_set_keybind("show_item_labels", SDLK_z, KMOD_NONE);
	input_set_keybind("automap", SDLK_TAB, KMOD_NONE);

	input_set_keybind("activate_program0", SDLK_F5, KMOD_NONE);
	input_set_keybind("activate_program1", SDLK_F6, KMOD_NONE);
	input_set_keybind("activate_program2", SDLK_F7, KMOD_NONE);
	input_set_keybind("activate_program3", SDLK_F8, KMOD_NONE);
	input_set_keybind("activate_program4", SDLK_F9, KMOD_NONE);
	input_set_keybind("activate_program5", SDLK_F10, KMOD_NONE);
	input_set_keybind("activate_program6", SDLK_F11, KMOD_NONE);
	input_set_keybind("activate_program7", SDLK_F12, KMOD_NONE);

	input_set_keybind("quick_inventory0", SDLK_0, KMOD_NONE);
	input_set_keybind("quick_inventory1", SDLK_1, KMOD_NONE);
	input_set_keybind("quick_inventory2", SDLK_2, KMOD_NONE);
	input_set_keybind("quick_inventory3", SDLK_3, KMOD_NONE);
	input_set_keybind("quick_inventory4", SDLK_4, KMOD_NONE);
	input_set_keybind("quick_inventory5", SDLK_5, KMOD_NONE);
	input_set_keybind("quick_inventory6", SDLK_6, KMOD_NONE);
	input_set_keybind("quick_inventory7", SDLK_7, KMOD_NONE);
	input_set_keybind("quick_inventory8", SDLK_8, KMOD_NONE);
	input_set_keybind("quick_inventory9", SDLK_9, KMOD_NONE);

	input_set_keybind("close_windows", SDLK_SPACE, KMOD_NONE);
	input_set_keybind("game_escape_menu", SDLK_ESCAPE, KMOD_NONE);

	/* Level editor */
	input_set_keybind("drop_item", SDLK_g, KMOD_NONE);
	input_set_keybind("place_obstacle_kp1", SDLK_KP1, KMOD_NONE);
	input_set_keybind("place_obstacle_kp2", SDLK_KP2, KMOD_NONE);
	input_set_keybind("place_obstacle_kp3", SDLK_KP3, KMOD_NONE);
	input_set_keybind("place_obstacle_kp4", SDLK_KP4, KMOD_NONE);
	input_set_keybind("place_obstacle_kp5", SDLK_KP5, KMOD_NONE);
	input_set_keybind("place_obstacle_kp6", SDLK_KP6, KMOD_NONE);
	input_set_keybind("place_obstacle_kp7", SDLK_KP7, KMOD_NONE);
	input_set_keybind("place_obstacle_kp8", SDLK_KP8, KMOD_NONE);
	input_set_keybind("place_obstacle_kp9", SDLK_KP9, KMOD_NONE);
	input_set_keybind("place_obstacle_kp9", SDLK_KP9, KMOD_NONE);
	input_set_keybind("change_obstacle_label", SDLK_h, KMOD_NONE);
	input_set_keybind("change_map_label", SDLK_m, KMOD_NONE);
	input_set_keybind("zoom_out", SDLK_o, KMOD_NONE);
	input_set_keybind("cycle_marked_object", SDLK_n, KMOD_NONE);
	input_set_keybind("next_tab", SDLK_f, KMOD_NONE);
	input_set_keybind("previous_tab", SDLK_f, KMOD_LSHIFT);
	input_set_keybind("undo", SDLK_z, KMOD_NONE);
	input_set_keybind("redo", SDLK_y, KMOD_NONE);
	input_set_keybind("beautify_grass", SDLK_b, KMOD_LCTRL);
	input_set_keybind("beautify_water", SDLK_w, KMOD_LCTRL);
	input_set_keybind("toggle_waypoint", SDLK_w, KMOD_NONE);
	input_set_keybind("toggle_waypoint_randomspawn", SDLK_w, KMOD_LSHIFT);
	input_set_keybind("connect_waypoint", SDLK_c, KMOD_NONE);
	input_set_keybind("toolbar_scroll_left", SDLK_PAGEUP, KMOD_NONE);
	input_set_keybind("toolbar_scroll_right", SDLK_PAGEDOWN, KMOD_NONE);
	input_set_keybind("toolbar_step_left", SDLK_PAGEUP, KMOD_LCTRL);
	input_set_keybind("toolbar_step_right", SDLK_PAGEDOWN, KMOD_LCTRL);
	input_set_keybind("cut", SDLK_x, KMOD_LCTRL);
	input_set_keybind("copy", SDLK_c, KMOD_LCTRL);
	input_set_keybind("paste", SDLK_v, KMOD_LCTRL);
	input_set_keybind("next_selection_type", SDLK_TAB, KMOD_NONE);
	input_set_keybind("previous_selection_type", SDLK_TAB, KMOD_LSHIFT);
	input_set_keybind("run_map_validator", SDLK_e, KMOD_LCTRL);

	/* Cheat */
	input_set_keybind("cheat_xp+_1k", SDLK_KP1, KMOD_NONE);
	input_set_keybind("cheat_xp*_2", SDLK_KP2, KMOD_NONE);
	input_set_keybind("cheat_melee", SDLK_KP7, KMOD_NONE);
	input_set_keybind("cheat_range", SDLK_KP8, KMOD_NONE);
	input_set_keybind("cheat_programing", SDLK_KP9, KMOD_NONE);
	input_set_keybind("cheat_melee_down", SDLK_KP4, KMOD_NONE);
	input_set_keybind("cheat_range_down", SDLK_KP5, KMOD_NONE);
	input_set_keybind("cheat_programing_down", SDLK_KP6, KMOD_NONE);
	input_set_keybind("cheat_drop_random_item", SDLK_r, KMOD_LCTRL);
	input_set_keybind("cheat_drop_random_magical_item", SDLK_r, KMOD_LCTRL | KMOD_LSHIFT);
	input_set_keybind("cheat_respawn_level", SDLK_r, KMOD_LCTRL | KMOD_LALT | KMOD_LSHIFT);
	input_set_keybind("cheat_level_editor", SDLK_e, KMOD_LCTRL | KMOD_LALT);
	input_set_keybind("cheat_menu", SDLK_c, KMOD_LCTRL | KMOD_LALT | KMOD_LSHIFT);
	input_set_keybind("cheat_reload_graphics", SDLK_g, KMOD_LCTRL | KMOD_LALT | KMOD_LSHIFT);

}

#define KEYCHART_RECT_X (GameConfig.screen_width / 250)
#define KEYCHART_RECT_Y (GameConfig.screen_height / 10)
#define KEYCHART_RECT_W (0.99 * GameConfig.screen_width)
#define KEYCHART_RECT_H (0.8 * GameConfig.screen_height)

/**
 * Display the keychart
 *
 * @return The index of the last command displayed on screen.
 */
static int display_keychart(unsigned int startidx, unsigned int cursor, int highlight)
{
	char txt[1024];
	int i;
	SDL_Rect keychart_rect = { KEYCHART_RECT_X, KEYCHART_RECT_Y, KEYCHART_RECT_W, KEYCHART_RECT_H };
	int ypos = keychart_rect.y;
	int xpos = keychart_rect.x;

	blit_background("shoppe.jpg");

	ShadowingRectangle(Screen, keychart_rect);

	put_string_centered(Para_BFont, FontHeight(Para_BFont), "Key chart");

	if (startidx >= sizeof(keybindNames) / sizeof(keybindNames[0]))
		return -1;

	for (i = startidx; keybindNames[i] != NULL; i++) {
		char keystr[100] = "";
		const char *font_str = font_switchto_neon;
		
		if (i == cursor && highlight) {
			font_str = font_switchto_red;
		}

		input_get_keybind_string(keybindNames[i], &keystr[0]);

		sprintf(txt, "%s%s%s - %s\n", font_str, (i == cursor) ? "** " : "   ",
			GameConfig.input_keybinds[i].name, keystr);
		put_string(GetCurrentFont(), xpos, ypos, txt);

		ypos += FontHeight(GetCurrentFont());

		if (ypos > keychart_rect.y + keychart_rect.h - FontHeight(GetCurrentFont())) {
			if (xpos > keychart_rect.x)
				break;
			else {
				xpos += 300;
				ypos = keychart_rect.y;
			}
		}
	}

	put_string(FPS_Display_BFont, 100, GameConfig.screen_height - FontHeight(FPS_Display_BFont),
		      _("ARROWS to select, ENTER to remap, ESCAPE to exit"));
	our_SDL_flip_wrapper();

	return i;
}

/**
 * This function simulates displaying the keychart in order to know how many keys 
 * appear per page
 */
static int get_nb_commands_per_page()
{
	int ypos = KEYCHART_RECT_Y;
	BFont_Info *our_font = FPS_Display_BFont;
	int i = 0;

	while (1) {
		ypos += FontHeight(our_font);
		i++;
		if (ypos > KEYCHART_RECT_Y + KEYCHART_RECT_H - FontHeight(our_font)) {
			break;
		}
	}

	return i;
}

/**
 * Display a keychart and allow modification of keys
 */
void keychart()
{
	int done = 0;
	int startpos = 0;
	int endpos;
	int cursor = 0;
	SDL_Event event;
	const int maxcmds = sizeof(keybindNames) / sizeof(keybindNames[0]) - 2;
	int per_page = get_nb_commands_per_page();

	Activate_Conservative_Frame_Computation();

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	while (!done) {
		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {
				Terminate(EXIT_SUCCESS, TRUE);
			}

			endpos = display_keychart(startpos, cursor, FALSE);

			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE)
					done = 1;
				if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_PAGEDOWN) {
					cursor += per_page;
					if (cursor >= maxcmds)
						cursor = maxcmds;

					if (cursor > endpos) {
						startpos += per_page;
					}
				}
				if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_PAGEUP) {
					cursor -= per_page;

					if (cursor < 0) {
						cursor = 0;
					}

					if (cursor < startpos) {
						startpos -= per_page;
					}

					if (startpos < 0) {
						startpos = 0;
					}
				}
				if (event.key.keysym.sym == SDLK_UP) {
					cursor--;
					if (cursor < 0)
						cursor = 0;
					if (cursor < startpos)
						startpos--;
				}
				if (event.key.keysym.sym == SDLK_DOWN) {
					if (cursor < maxcmds)
						cursor++;
					if (cursor > endpos)
						startpos++;
				}
				if (event.key.keysym.sym == SDLK_RETURN) {
					int newmod;
					int oldkey, oldmod;
					int i;
					display_keychart(startpos, cursor, TRUE);
					oldkey = GameConfig.input_keybinds[cursor].key;
					oldmod = GameConfig.input_keybinds[cursor].mod;

					GameConfig.input_keybinds[cursor].key = getchar_raw(&newmod);
					newmod &= ~(KMOD_CAPS | KMOD_NUM | KMOD_MODE);	/* We want to ignore "global" modifiers. */
					GameConfig.input_keybinds[cursor].mod = newmod;

					for (i = 0; keybindNames[i] != NULL; i++) {
						if (i == cursor)
							continue;

						if (GameConfig.input_keybinds[i].key == GameConfig.input_keybinds[cursor].key
						    && GameConfig.input_keybinds[i].mod == newmod) {
							/* If the key we have just assigned was already assigned to another command... */
							GameConfig.input_keybinds[i].key = oldkey;
							GameConfig.input_keybinds[i].mod = oldmod;
							/* ... swap the keys (assign the old key to the conflicting command) */
						}
					}

				}
			}
		}
	}
	SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
}

/**
 * Hold the keyboard input handler.
 *
 * Some UI panels have their own keyboard input handler, or in some cases
 * we do not want the main keyboard input handler to react to key events.
 * Some keybindings could however be 'un-interruptibles'.
 * The opposite function is input_release_keyboard().
 */
void input_hold_keyboard(void)
{
	// Several UI elements may want to hold the keyboard handler, and
	// thus call this function and its release companion.
	// We thus need to count how many times this function is called, and
	// how many times the release function is called.
	input_keyboard_locked++;
}

/**
 * Release the keyboard input handler.
 *
 * See input_hold_keyboard().
 */
void input_release_keyboard(void)
{
	if (input_keyboard_locked > 0)
		input_keyboard_locked--;
}

/**
 * @fn static void input_key( int keynum, int value)
 *
 * @brief Runs the input command.
 *
 *    @param keynum The index of the  keybind.
 *    @param value The value of the keypress (defined above).
 */
#define KEYPRESS(s)    ((strcmp(GameConfig.input_keybinds[keynum].name,s)==0) && value==KEY_PRESS)
#define INGAME()	(game_status == INSIDE_GAME)
#define INMENU()	(game_status == INSIDE_MENU)
#define INLVLEDIT()	(game_status == INSIDE_LVLEDITOR)
static int input_key(int keynum, int value)
{
	/* Non lockable keybindings */
 	if (KEYPRESS("quit")) {
		Terminate(EXIT_SUCCESS, TRUE);
		return 0;
	}

	/* Do not execute the other key actions, if the the keyboard input is 'held' */
	if (input_keyboard_locked)
		return -1;

	/* Cheat keys - those all fall through to normal game keys! */
	if (GameConfig.enable_cheatkeys && INGAME()) {
		if (KEYPRESS("cheat_xp+_1k")) {
			Me.Experience += 1000;
		} else if (KEYPRESS("cheat_xp*_2")) {
			Me.Experience *= 2;
		} else if (KEYPRESS("cheat_melee")) {
			if (Me.melee_weapon_skill < 9)
				Me.melee_weapon_skill += 1;
		} else if (KEYPRESS("cheat_range")) {
			if (Me.ranged_weapon_skill < 9)
				Me.ranged_weapon_skill += 1;
		} else if (KEYPRESS("cheat_programing")) {
			if (Me.spellcasting_skill < 9)
				Me.spellcasting_skill += 1;
		} else if (KEYPRESS("cheat_melee_down")) {
			if (Me.melee_weapon_skill > 0)
				Me.melee_weapon_skill -= 1;
		} else if (KEYPRESS("cheat_range_down")) {
			if (Me.ranged_weapon_skill > 0)
				Me.ranged_weapon_skill -= 1;
		} else if (KEYPRESS("cheat_programing_down")) {
			if (Me.spellcasting_skill > 0)
				Me.spellcasting_skill -= 1;
		} else if (KEYPRESS("cheat_drop_random_item")) {
			DropRandomItem(Me.pos.z, Me.pos.x, Me.pos.y, 3, FALSE);
		} else if (KEYPRESS("cheat_drop_random_magical_item")) {
			DropRandomItem(Me.pos.z, Me.pos.x, Me.pos.y, 3, TRUE);
		} else if (KEYPRESS("cheat_respawn_level")) {
			respawn_level(Me.pos.z);
		} else if (KEYPRESS("cheat_reload_graphics")) {
			reload_graphics();
		}
	}

	/* Ingame input functions */
	if (INGAME()) {
		if (KEYPRESS("inventory")) {
			toggle_game_config_screen_visibility(GAME_CONFIG_SCREEN_VISIBLE_INVENTORY);
			return 0;
		} else if (KEYPRESS("skill")) {
			toggle_game_config_screen_visibility(GAME_CONFIG_SCREEN_VISIBLE_SKILLS);
			return 0;
		} else if (KEYPRESS("character")) {
			toggle_game_config_screen_visibility(GAME_CONFIG_SCREEN_VISIBLE_CHARACTER);
			return 0;
		} else if (KEYPRESS("quests")) {
			toggle_quest_browser();
			return 0;
		} else if (KEYPRESS("autorun")) {
			GameConfig.autorun_activated = !GameConfig.autorun_activated;
			return 0;
		} else if (KEYPRESS("move_north")) {
			set_movement_with_keys(0, -1);
			return 0;
		} else if (KEYPRESS("move_south")) {
			set_movement_with_keys(0, 1);
			return 0;
		} else if (KEYPRESS("move_east")) {
			set_movement_with_keys(1, 0);
			return 0;
		} else if (KEYPRESS("move_west")) {
			set_movement_with_keys(-1, 0);
			return 0;
		} else if (KEYPRESS("reload")) {
			TuxReloadWeapon();
			return 0;
		} else if (KEYPRESS("quicksave")) {
			SaveGame();
			return 0;
		} else if (KEYPRESS("quickload")) {
			LoadGame();
			return 0;
		} else if (KEYPRESS("pause")) {
			Pause();
			return 0;
		} else if (KEYPRESS("show_item_labels")) {
			GameConfig.show_item_labels = !GameConfig.show_item_labels;
			return 0;
		} else if (!strncmp(GameConfig.input_keybinds[keynum].name, "activate_program", strlen("activate_program"))
			   && value == KEY_PRESS) {
			int number = atoi(GameConfig.input_keybinds[keynum].name + strlen("activate_program"));
			if (number >= 10 || number < 0) {
				ErrorMessage(__FUNCTION__,
					     "Tried to activate skill number %d - only shortcuts from F1 to F9 are supported\n",
					     PLEASE_INFORM, IS_WARNING_ONLY, number);
				return 0;
			}

			int assign_shortcut = GameConfig.SkillScreen_Visible
			    && CursorIsOnWhichSkillButton(GetMousePos_x(), GetMousePos_y()) != -1;

			if (!assign_shortcut) {
				if (Me.program_shortcuts[number] != -1)
					activate_nth_skill(Me.program_shortcuts[number]);
			} else {
				set_nth_quick_skill(number);

			}
		} else if (!strncmp(GameConfig.input_keybinds[keynum].name, "quick_inventory", strlen("quick_inventory"))
			   && value == KEY_PRESS) {
			int number = atoi(GameConfig.input_keybinds[keynum].name + strlen("quick_inventory"));
			if (number >= 10 || number < 0) {
				ErrorMessage(__FUNCTION__, "Tried to use quick inventory item %d - only 1-9 + 0 are supported.\n",
					     PLEASE_INFORM, IS_WARNING_ONLY, number);
				return 0;
			}

			Quick_ApplyItem(number);
			return 0;
		} else if (KEYPRESS("automap")) {
			toggle_automap();
			return 0;
		} else if (KEYPRESS("cheat_level_editor")) {
			LevelEditor();
			return 0;
		} else if (KEYPRESS("cheat_menu")) {
			Cheatmenu();
			return 0;
		} else if (KEYPRESS("close_windows")) {
			if (GameConfig.Inventory_Visible ||
			    GameConfig.CharacterScreen_Visible ||
			    GameConfig.SkillScreen_Visible || GameConfig.skill_explanation_screen_visible) {
				GameConfig.Inventory_Visible = FALSE;
				GameConfig.CharacterScreen_Visible = FALSE;
				GameConfig.SkillScreen_Visible = FALSE;
				GameConfig.skill_explanation_screen_visible = FALSE;
			}
		} else if (KEYPRESS("game_escape_menu")) {
			if (GameConfig.Inventory_Visible ||
			    GameConfig.CharacterScreen_Visible ||
			    GameConfig.SkillScreen_Visible || GameConfig.skill_explanation_screen_visible) {
				GameConfig.Inventory_Visible = FALSE;
				GameConfig.CharacterScreen_Visible = FALSE;
				GameConfig.SkillScreen_Visible = FALSE;
				GameConfig.skill_explanation_screen_visible = FALSE;
			} else {
				EscapeMenu();
			}
			return 0;
		}
	} else if (INLVLEDIT()) {
		if (KEYPRESS("drop_item")) {
			ItemDropFromLevelEditor();
			return 0;
		} else if (KEYPRESS("change_obstacle_label")) {
			if (single_tile_selection(OBJECT_OBSTACLE)) {
				action_change_obstacle_label_user(EditLevel(), single_tile_selection(OBJECT_OBSTACLE));
			}
		} else if (KEYPRESS("change_map_label")) {
			level_editor_action_change_map_label_user(CURLEVEL(), Me.pos.x, Me.pos.y);
			return 0;
		} else if (KEYPRESS("zoom_out")) {
			GameConfig.zoom_is_on = !GameConfig.zoom_is_on;
			return 0;
		} else if (KEYPRESS("cycle_marked_object")) {
			level_editor_cycle_marked_object();
			return 0;
		} else if (KEYPRESS("next_tab")) {
			lvledit_categoryselect_switch(1);
			return 0;
		} else if (KEYPRESS("previous_tab")) {
			lvledit_categoryselect_switch(-1);
			return 0;
		} else if (KEYPRESS("next_selection_type")) {
			level_editor_switch_selection_type(1);
			return 0;
		} else if (KEYPRESS("previous_selection_type")) {
			level_editor_switch_selection_type(-1);
			return 0;
		} else if (KEYPRESS("undo")) {
			level_editor_action_undo();
			return 0;
		} else if (KEYPRESS("redo")) {
			level_editor_action_redo();
			return 0;
		} else if (KEYPRESS("beautify_grass")) {
			level_editor_beautify_grass_tiles(EditLevel());
			return 0;
		} else if (KEYPRESS("beautify_water")) {
			level_editor_beautify_water_tiles(EditLevel());
			return 0;
		} else if (KEYPRESS("toolbar_scroll_left")) {
			widget_lvledit_toolbar_scroll_left();
			return 0;
		} else if (KEYPRESS("toolbar_scroll_right")) {
			widget_lvledit_toolbar_scroll_right();
			return 0;
		} else if (KEYPRESS("toolbar_step_left")) {
			widget_lvledit_toolbar_left();
			return 0;
		} else if (KEYPRESS("toolbar_step_right")) {
			widget_lvledit_toolbar_right();
			return 0;
		} else if (KEYPRESS("run_map_validator")) {
			level_validation();	
			return 0;
		} else if (!strncmp(GameConfig.input_keybinds[keynum].name, "place_obstacle_kp", strlen("place_obstacle_kp"))
			   && value == KEY_PRESS) {
			int number = atoi(GameConfig.input_keybinds[keynum].name + strlen("place_obstacle_kp"));
			if (number >= 10 || number < 0) {
				ErrorMessage(__FUNCTION__, "Tried to place obstacle using kp%d - only supported are kp1 to kp9\n",
					     PLEASE_INFORM, IS_WARNING_ONLY, number);
				return 0;
			}
			level_editor_place_aligned_object(number);
			return 0;
		} else if (KEYPRESS("toggle_waypoint")) {
			lvledit_action_toggle_waypoint(FALSE);
			return 0;
		} else if (KEYPRESS("toggle_waypoint_randomspawn")) {
			lvledit_action_toggle_waypoint(TRUE);
			return 0;
		} else if (KEYPRESS("connect_waypoint")) {
			level_editor_action_toggle_waypoint_connection_user(CURLEVEL(), EditX(), EditY());
			return 0;
		} else if (KEYPRESS("cut")) {
			level_editor_cut_selection();
			return 0;
		} else if (KEYPRESS("copy")) {
			level_editor_copy_selection();
			return 0;
		} else if (KEYPRESS("paste")) {
			level_editor_paste_selection();
			return 0;
		}
	}

	/* Global keybindings */
	if (KEYPRESS("fullscreen")) {
		/* This doesn't seem to work so well on Windows */
#ifndef __WIN32__
		SDL_WM_ToggleFullScreen(Screen);
		GameConfig.fullscreen_on = !GameConfig.fullscreen_on;
#endif
		return 0;
	} else if (KEYPRESS("grab_input")) {
		SDL_GrabMode mode = SDL_WM_GrabInput(SDL_GRAB_QUERY);
		if (mode == SDL_GRAB_OFF)
			mode = SDL_GRAB_ON;
		else
			mode = SDL_GRAB_OFF;

		SDL_WM_GrabInput(mode);
	} else if (KEYPRESS("take_screenshot")) {
		play_sound_cached("effects/CameraTakesPicture.ogg");
		char filename[1000];
		char relative_filename[100];
		sprintf(relative_filename, "%s.screenshot-%d.png", Me.character_name, SDL_GetTicks()/1000);
		sprintf(filename, "%s/%s", our_config_dir, relative_filename);
		save_screenshot(filename, FALSE);
		alert_window(_("Screenshot saved to \"%s\" in your .freedroid_rpg/ directory."), relative_filename);
	} else if (KEYPRESS("wall_transparency")) {
		GameConfig.transparency = !GameConfig.transparency;
		return 0;
	} else if (KEYPRESS("keychart")) {
		keychart();
		return 0;
	}

	return -1;
}

static int input_key_event(SDLKey key, SDLMod mod, int value)
{
	int i;
	int noteaten = -1;
	mod &= ~(KMOD_CAPS | KMOD_NUM | KMOD_MODE);	/* We want to ignore "global" modifiers. */

	for (i = 0; keybindNames[i] != NULL; i++)
		if ((GameConfig.input_keybinds[i].key == key) && (GameConfig.input_keybinds[i].mod == mod)) {
			if (!(noteaten = input_key(i, value)))
				break;
		}
	return noteaten;
}

int input_key_press(SDL_Event * event)
{
	input_key_event(event->key.keysym.sym, event->key.keysym.mod, KEY_PRESS);

	return 0;
}

/**
 * @fn int kptoprint (int)
 *
 * @brief Returns printable keycodes when given a printable keypad code.
 */
static int kptoprint(int key) {
	switch (key) {
		case SDLK_KP_PERIOD: key = SDLK_PERIOD; break;
		case SDLK_KP_DIVIDE: key = SDLK_SLASH; break;
		case SDLK_KP_MULTIPLY: key = SDLK_ASTERISK; break;
		case SDLK_KP_MINUS: key = SDLK_MINUS; break;
		case SDLK_KP_PLUS: key = SDLK_PLUS; break;
		case SDLK_KP_EQUALS: key = SDLK_EQUALS; break;
		default: break;	
	}	
	return key;
}

/**
 * This function reads a character from the keyboard
 * and returns the SDLKey that was pressed.
 */
int getchar_raw(int *mod)
{
	SDL_Event event;
	int Returnkey;

	while (1) {
		SDL_WaitEvent(&event);	/* wait for next event */

		if (event.type == SDL_QUIT) {
			Terminate(EXIT_SUCCESS, TRUE);
		}

		if (event.type == SDL_KEYDOWN) {
			Returnkey = (int)event.key.keysym.sym;
			if (!mod && event.key.keysym.mod & KMOD_SHIFT)
				Returnkey = toupper((int)event.key.keysym.sym);

			if (mod) {
				/* If we have a modifier, we assign it */
				*mod = event.key.keysym.mod;

				/* And we also discard the keypress if the sym is e.g. CTRL because we explicitely
				 * asked to get a modifier, which makes sense only if we ignore CTRL as a key.
				 */
				if (event.key.keysym.sym >= SDLK_NUMLOCK && event.key.keysym.sym <= SDLK_COMPOSE) {
					goto next;
				}
			}
			/*
			 * here we use the fact that, I cite from SDL_keyboard.h:
			 * "The keyboard syms have been cleverly chosen to map to ASCII"
			 * ... I hope that this design feature is portable, and durable ;)
			 */
			if(ISKEYPAD(Returnkey)) {
				Returnkey = kptoprint(Returnkey);
			}
			return (Returnkey);
		}
 next:		;

	}			/* while(1) */
}				/* getchar_raw() */
