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
 * @file input.c
 *
 * @brief Handles all the keybindings and input.
 */


#define _keyboard_c

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"


#define KEY_PRESS    ( 1.) /**< Key is pressed. */
#define KEY_RELEASE  (-1.) /**< Key is released. */


/* keybinding structure */
typedef struct Keybind_ {
    char *name; /**< keybinding name, taken from keybindNames */
    SDLKey key; /**< key/axis/button event number */
    SDLMod mod; /**< Key modifiers */
} Keybind;


static Keybind** input_keybinds; /**< contains the players keybindings */

/* name of each keybinding */
const char *keybindNames[] = {
    /* General */
    "fullscreen", "keychart", "quit", "wall_transparency", 

    /* Ingame */
    "inventory", "skill", "character", "quests",
    "reload", "autorun",
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
    "change_map_label", "zoom_out",
    "cycle_marked_obstacle",
    "next_tab", "undo", "redo", "beautify_grass",
    "connect_waypoint",

    /* Cheat keys */
    "cheat_xp+_1k", "cheat_xp*_2",
    "cheat_melee", "cheat_range", "cheat_programing", "cheat_melee_down", "cheat_range_down", "cheat_programing_down",
    "cheat_identify_all",
    "cheat_drop_random_item", "cheat_drop_random_magical_item",
    "cheat_respawn_level",
    "cheat_menu",
    "end" }; /* must terminate in "end" */



/**
 * @fn void input_init (void)
 *
 * @brief Initializes the input subsystem (does not set keys).
 */
void input_keyboard_init (void)
{
    Keybind *temp;
    int i;
    for (i=0; strcmp(keybindNames[i],"end"); i++); /* gets number of bindings */
    input_keybinds = MyMalloc(i*sizeof(Keybind*));

    /* creates a null keybinding for each */
    for (i=0; strcmp(keybindNames[i],"end"); i++) {
	temp = MyMalloc(sizeof(Keybind));
	temp->name = (char*)keybindNames[i];
	temp->key = SDLK_UNKNOWN;
	temp->mod = KMOD_NONE;
	input_keybinds[i] = temp;
    }
}


/**
 * @fn void input_exit (void)
 *
 * @brief Exits the input subsystem.
 */
void input_exit (void)
{
    int i;
    for (i=0; strcmp(keybindNames[i],"end"); i++)
	free(input_keybinds[i]);
    free(input_keybinds);
}


/**
 *
 * @brief Binds key to action keybind.
 *
 *    @param keybind The name of the keybind defined above.
 *    @param key The key to bind to.
 *    @param mod the modifier associated to the key
 */
void input_set_keybind( char *keybind, SDLKey key, SDLMod mod)
{
    int i;
    for (i=0; strcmp(keybindNames[i],"end"); i++)
	if (strcmp(keybind, input_keybinds[i]->name)==0) {
	    input_keybinds[i]->key = key;
	    input_keybinds[i]->mod = mod;
	    return;
	}
    ErrorMessage(__FUNCTION__, "Unable to set keybinding '%s', that command doesn't exist.\n", PLEASE_INFORM, IS_FATAL, keybind);
}

void input_get_keybind(char *cmdname, SDLKey *key, SDLMod *mod)
{
    int i;
    for (i=0; strcmp(keybindNames[i],"end"); i++) {
	if (!strcmp(cmdname, input_keybinds[i]->name)) {
	    if (key)
		*key = input_keybinds[i]->key;
	    if (mod)
		*mod = input_keybinds[i]->mod;
	    return;
	}
    }
    ErrorMessage(__FUNCTION__, "Unable to get keybind for command '%s', that command does not exist.\n", PLEASE_INFORM, IS_FATAL, cmdname);
}

/**
 * @fn void input_setDefault (void)
 *
 * @brief Sets the default input keys.
 */
void input_set_default (void)
{
    input_set_keybind("fullscreen", SDLK_F2, KMOD_NONE);
    input_set_keybind("keychart", SDLK_F1, KMOD_NONE);
    input_set_keybind("quit", SDLK_q, KMOD_LCTRL);
    input_set_keybind("wall_transparency", SDLK_t, KMOD_NONE);

    /* Game */
    input_set_keybind("inventory", SDLK_i, KMOD_NONE);
    input_set_keybind("skill", SDLK_s, KMOD_NONE);
    input_set_keybind("character", SDLK_c, KMOD_NONE);
    input_set_keybind("quests", SDLK_q, KMOD_NONE);
    input_set_keybind("reload", SDLK_r, KMOD_NONE);
    input_set_keybind("autorun", SDLK_u, KMOD_NONE);
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
    input_set_keybind("change_map_label", SDLK_l, KMOD_NONE);
    input_set_keybind("zoom_out", SDLK_o, KMOD_NONE);
    input_set_keybind("cycle_marked_obstacle", SDLK_n, KMOD_NONE);
    input_set_keybind("next_tab", SDLK_f, KMOD_NONE);
    input_set_keybind("undo", SDLK_u, KMOD_NONE);
    input_set_keybind("redo", SDLK_r, KMOD_NONE);
    input_set_keybind("beautify_grass", SDLK_b, KMOD_LCTRL);
    input_set_keybind("connect_waypoint", SDLK_c, KMOD_NONE);

    /* Cheat */
    input_set_keybind("cheat_xp+_1k", SDLK_KP1, KMOD_NONE);
    input_set_keybind("cheat_xp*_2", SDLK_KP2, KMOD_NONE);
    input_set_keybind("cheat_melee", SDLK_KP7, KMOD_NONE);
    input_set_keybind("cheat_range", SDLK_KP8, KMOD_NONE);
    input_set_keybind("cheat_programing", SDLK_KP9, KMOD_NONE);
    input_set_keybind("cheat_melee_down", SDLK_KP4, KMOD_NONE);
    input_set_keybind("cheat_range_down", SDLK_KP5, KMOD_NONE);
    input_set_keybind("cheat_programing_down", SDLK_KP6, KMOD_NONE);
    input_set_keybind("cheat_identify_all", SDLK_i, KMOD_LSHIFT);
    input_set_keybind("cheat_drop_random_item", SDLK_r, KMOD_LCTRL);
    input_set_keybind("cheat_drop_random_magical_item", SDLK_r, KMOD_LCTRL | KMOD_LSHIFT);
    input_set_keybind("cheat_respawn_level", SDLK_r, KMOD_LCTRL | KMOD_LALT | KMOD_LSHIFT);
    input_set_keybind("cheat_menu", SDLK_c, KMOD_LCTRL | KMOD_LALT | KMOD_LSHIFT);

}


/**
 * Display the keychart
 */
static void display_keychart(unsigned int startidx, unsigned int cursor, int highlight)
{
    SDL_Rect our_rect = { GameConfig.screen_width / 10, GameConfig.screen_height / 10, 0.8 * GameConfig.screen_width, 0.8 * GameConfig.screen_height };
    char txt[1024];
    int i;
    int ypos = our_rect.y;
    int xpos = our_rect.x;
    BFont_Info * our_font = FPS_Display_BFont;

    blit_special_background(SHOP_BACKGROUND_IMAGE_CODE);

    ShadowingRectangle(Screen, our_rect);

    CenteredPutStringFont(Screen, Para_BFont, FontHeight(Para_BFont), "Key chart");

    if (startidx >= sizeof(keybindNames)/sizeof(keybindNames[0]))
	return;

    ypos = our_rect.y;

    for (i=startidx; strcmp(keybindNames[i], "end"); i++) {
	sprintf(txt, "%c%s%s - %s\n", (i==cursor && highlight) ? '\1' : '\2', (i == cursor) ? "** " : "   ", input_keybinds[i]->name, SDL_GetKeyName(input_keybinds[i]->key));
	PutStringFont(Screen, our_font, xpos, ypos, txt);

	ypos += FontHeight(our_font);

	if (ypos > our_rect.y + our_rect.h - FontHeight(our_font)) {
	    if (xpos > our_rect.x)
		break;
	    else {
		xpos += 300;
		ypos = our_rect.y;
	    }
	}
    }

    PutStringFont(Screen, FPS_Display_BFont, 100, GameConfig.screen_height - FontHeight(FPS_Display_BFont), _("ARROWS to select, ENTER to remap, ESCAPE to exit"));
    our_SDL_flip_wrapper();
}


/**
 * Display a keychart and allow modification of keys
 */
void keychart()
{
    int done = 0;
    int startpos = 0;
    int cursor = 0;
    SDL_Event event;
    const int maxcmds = sizeof(keybindNames)/sizeof(keybindNames[0]) - 2;

    while (!done) {
	while (SDL_PollEvent(&event)) {
            
	    if (event.type == SDL_QUIT) {
		Terminate(0);
	    }

	    display_keychart(startpos, cursor, FALSE);

	    if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_ESCAPE)
		    done = 1;
		if (event.key.keysym.sym == SDLK_RIGHT) {
		    if (startpos < maxcmds)
			startpos ++;
		    if (cursor < startpos)
			cursor = startpos;
		}
		if (event.key.keysym.sym == SDLK_LEFT) {
		    startpos --;
		    if (startpos < 0)
			startpos = 0;
		}
		if (event.key.keysym.sym == SDLK_UP) {
		    cursor --;
		    if (cursor < 0)
			cursor = 0;
		}
		if (event.key.keysym.sym == SDLK_DOWN) {
		    if (cursor < maxcmds)
			cursor ++;
		}
		if (event.key.keysym.sym == SDLK_RETURN) {
		    display_keychart(startpos, cursor, TRUE);
		    input_keybinds[cursor]->key = getchar_raw();
		}
	    }
	}
    }
}

/**
 * @fn static void input_key( int keynum, int value)
 *
 * @brief Runs the input command.
 *
 *    @param keynum The index of the  keybind.
 *    @param value The value of the keypress (defined above).
 *    @param abs Whether or not it's an absolute value (for them joystick).
 */
#define KEYPRESS(s)    ((strcmp(input_keybinds[keynum]->name,s)==0) && value==KEY_PRESS)
#define INGAME()	(game_status == INSIDE_GAME)
#define INMENU()	(game_status == INSIDE_MENU)
#define INLVLEDIT()	(game_status == INSIDE_LVLEDITOR)
static int input_key( int keynum, int value)
{
    /* Cheat keys - those all fall through to normal game keys! */
    if (GameConfig.enable_cheatkeys && INGAME()) {
	if (KEYPRESS("cheat_xp+_1k")) {
	    Me.Experience += 1000;
	} else if (KEYPRESS("cheat_xp*_2")) {
	    Me.Experience *= 2;
	} else if (KEYPRESS("cheat_melee")) {
	    if (Me.melee_weapon_skill < 9) Me.melee_weapon_skill += 1;
	} else if (KEYPRESS("cheat_range")) {
	    if (Me.ranged_weapon_skill < 9) Me.ranged_weapon_skill += 1;
	} else if (KEYPRESS("cheat_programing")) {
	    if (Me.spellcasting_skill < 9) Me.spellcasting_skill += 1;
	} else if (KEYPRESS("cheat_melee_down")) {
	    if (Me.melee_weapon_skill > 0) Me.melee_weapon_skill -= 1;
	} else if (KEYPRESS("cheat_range_down")) {
	    if (Me.ranged_weapon_skill > 0) Me.ranged_weapon_skill -= 1;
	} else if (KEYPRESS("cheat_programing_down")) {
	    if (Me.spellcasting_skill > 0) Me.spellcasting_skill -= 1;
	} else if (KEYPRESS("cheat_identify_all")) {
	    int i;
	    for ( i = 0 ; i < MAX_ITEMS_IN_INVENTORY ; i ++ ) Me.Inventory[i].is_identified = TRUE;
	    Me.weapon_item.is_identified = TRUE;
	    Me.shield_item.is_identified = TRUE;
	    Me.armour_item.is_identified = TRUE;
	    Me.special_item.is_identified = TRUE;
	    Me.drive_item.is_identified = TRUE;
	} else if (KEYPRESS("cheat_drop_random_item")) {
	    DropRandomItem( Me . pos . z , Me . pos . x , Me . pos . y , 3 , FALSE);
	} else if (KEYPRESS("cheat_drop_random_magical_item")) {
	    DropRandomItem( Me . pos . z , Me . pos . x , Me . pos . y , 3 , TRUE);
	} else if (KEYPRESS("cheat_respawn_level")) {
	    respawn_level(Me.pos.z);
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
	    quest_browser_interface();
	    return 0;
	} else if (KEYPRESS("autorun")) {
	    extern int autorun_activated;
	    autorun_activated = !autorun_activated;
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
	    always_show_items_text = ! always_show_items_text;
	    return 0;
	} else if (!strncmp(input_keybinds[keynum]->name, "activate_program", strlen("activate_program")) && value==KEY_PRESS) {
	    int number = atoi(input_keybinds[keynum]->name + strlen("activate_program"));
	    if (number >= 10 || number < 0) {
		ErrorMessage(__FUNCTION__, "Tried to activate skill number %d - only shortcuts from F1 to F9 are supported\n", PLEASE_INFORM, IS_WARNING_ONLY, number);
		return 0;
	    }

	    activate_nth_aquired_skill(number);
	} else if (!strncmp(input_keybinds[keynum]->name, "quick_inventory", strlen("quick_inventory")) && value==KEY_PRESS) {
	    int number = atoi(input_keybinds[keynum]->name + strlen("quick_inventory"));
	    if (number >= 10 || number < 0) {
		ErrorMessage(__FUNCTION__, "Tried to use quick inventory item %d - only 1-9 + 0 are supported.\n", PLEASE_INFORM, IS_WARNING_ONLY, number);
		return 0;
	    }

	    Quick_ApplyItem ( number );
	    return 0;
	} else if (KEYPRESS("automap")) {
	    toggle_automap();
	    return 0;
	} else if (KEYPRESS("keychart")) {
	    keychart();
	    return 0;
	} else if (KEYPRESS("cheat_menu")) {
	    Cheatmenu();
	    return 0;
	} else if (KEYPRESS("close_windows")) {
	    if ( GameConfig . Inventory_Visible ||
		    GameConfig . CharacterScreen_Visible ||
		    GameConfig . SkillScreen_Visible || GameConfig . skill_explanation_screen_visible) {
		GameConfig . Inventory_Visible = FALSE ;
		GameConfig . CharacterScreen_Visible = FALSE ;
		GameConfig . SkillScreen_Visible = FALSE ;
		GameConfig . skill_explanation_screen_visible = FALSE;
            }
	} else if (KEYPRESS("game_escape_menu")) {
	    if ( GameConfig . Inventory_Visible ||
		    GameConfig . CharacterScreen_Visible ||
		    GameConfig . SkillScreen_Visible || GameConfig . skill_explanation_screen_visible) {
		GameConfig . Inventory_Visible = FALSE ;
		GameConfig . CharacterScreen_Visible = FALSE ;
		GameConfig . SkillScreen_Visible = FALSE ;
		GameConfig . skill_explanation_screen_visible = FALSE;
	    }
	    else {
		EscapeMenu ();
	    }
	    return 0;
	}
    } else if (INLVLEDIT()) {
	if (KEYPRESS("drop_item")) {
	    ItemDropFromLevelEditor();
	    return 0;
	} else if (KEYPRESS("change_map_label")) {
	    level_editor_action_change_map_label_user();
	    return 0;
	} else if (KEYPRESS("zoom_out")) {
	    GameConfig . zoom_is_on = !GameConfig . zoom_is_on ;
	    return 0;
	} else if (KEYPRESS("cycle_marked_obstacle")) {
	    level_editor_cycle_marked_obstacle();
	    return 0;
	} else if (KEYPRESS("next_tab")) {
	    level_editor_next_tab();
	    return 0;
	} else if (KEYPRESS("undo")) {
	    level_editor_action_undo();
	    return 0;
	} else if (KEYPRESS("redo")) {
	    level_editor_action_redo();
	    return 0;
	} else if (KEYPRESS("beautify_grass")) {
	    level_editor_beautify_grass_tiles();
	    return 0;
	} else if (!strncmp(input_keybinds[keynum]->name, "place_obstacle_kp", strlen("place_obstacle_kp")) && value==KEY_PRESS) {
	    int number = atoi(input_keybinds[keynum]->name + strlen("place_obstacle_kp"));
	    if (number >= 10 || number < 0) {
		ErrorMessage(__FUNCTION__, "Tried to place obstacle using kp%d - only supported are kp1 to kp9\n", PLEASE_INFORM, IS_WARNING_ONLY, number);
		return 0;
	    }
	    level_editor_place_aligned_obstacle(number);
	    return 0;
	} else if (KEYPRESS("connect_waypoint")) {
	    level_editor_action_toggle_waypoint_connection_user();
	    return 0;
	}
    }

    /* Global options */
    if (KEYPRESS("fullscreen")) {
	/* This doesn't seem to work so well on Windows */
#ifndef __WIN32__
	SDL_WM_ToggleFullScreen ( Screen );
	GameConfig . fullscreen_on = ! GameConfig . fullscreen_on;
#endif
	return 0;
    } else if (KEYPRESS("wall_transparency")) {
	    GameConfig.transparency = ! GameConfig.transparency;
	    return 0;
    } else if (KEYPRESS("quit")) {
	Terminate(0);
	return 0;
    }

    return -1;
}

static int input_key_event( SDLKey key, SDLMod mod, int value )
{
    int i;
    mod &= ~(KMOD_CAPS | KMOD_NUM | KMOD_MODE); /* We want to ignore "global" modifiers. */

    for (i=0; strcmp(keybindNames[i],"end"); i++)
	if ((input_keybinds[i]->key == key) && (input_keybinds[i]->mod==mod)) {
	    if(!input_key(i, value))
		break;
	}
    return 0;
}

int input_key_press (SDLKey key, SDLMod mod)
{
    return input_key_event(key, mod, KEY_PRESS);
}

/**
 * This function reads a character from the keyboard
 * and returns the SDLKey that was pressed.
 */
int getchar_raw (void)
{
    SDL_Event event;
    int Returnkey;

    while (1)
    {
	SDL_WaitEvent (&event);    /* wait for next event */

	if (event.type == SDL_QUIT) {
	    Terminate(0);
	}

	if (event.type == SDL_KEYDOWN)
	{
	    /*
	     * here we use the fact that, I cite from SDL_keyboard.h:
	     * "The keyboard syms have been cleverly chosen to map to ASCII"
	     * ... I hope that this design feature is portable, and durable ;)
	     */
	    Returnkey = (int) event.key.keysym.sym;
	    if ( event.key.keysym.mod & KMOD_SHIFT ) Returnkey = toupper( (int)event.key.keysym.sym );
	    return ( Returnkey );
	}
    } /* while(1) */
} /* getchar_raw() */
