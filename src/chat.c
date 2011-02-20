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
 * This file contains all functions dealing with the dialog interface,
 * including blitting the chat log to the screen and drawing the
 * right portrait images to the screen.
 */

#define _chat_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"
#include "SDL_rotozoom.h"

#define PUSH_ROSTER 2
#define POP_ROSTER 3

char *chat_initialization_code;	//first time with a character-code
char *chat_startup_code;	//every time we start this dialog-code

static void run_chat(enemy *ChatDroid, int is_subdialog);

/**
 * This function resets a dialog option to "empty" default values. It does NOT free the strings, this has to be done
 * prior to calling this function when needed.
 */
static void clear_dialog_option(dialogue_option * d)
{
	d->option_text = "";
	d->no_text = 0;
	d->lua_code = NULL;
	d->exists = 0;
}

/**
 *
 */
static void delete_one_dialog_option(int i, int FirstInitialisation)
{
	// If this is not the first initialisation, we have to free the allocated
	// strings first, or we'll be leaking memory...
	if (!FirstInitialisation) {
		if (strlen(ChatRoster[i].option_text))
			free(ChatRoster[i].option_text);
	}

	if (ChatRoster[i].lua_code)
		free(ChatRoster[i].lua_code);
	ChatRoster[i].lua_code = NULL;

	clear_dialog_option(&ChatRoster[i]);
}

/**
 * This function should init the chat roster with empty values and thereby
 * clean out the remnants of the previous chat dialogue.
 */
static void InitChatRosterForNewDialogue(void)
{
	int i;
	static int FirstInitialisation = TRUE;

	for (i = 0; i < MAX_DIALOGUE_OPTIONS_IN_ROSTER; i++) {
		delete_one_dialog_option(i, FirstInitialisation);
	}

	// Next time, we WILL have to free every used entry before cleaning it
	// out, or we will be leaking memory...
	//
	FirstInitialisation = FALSE;

};				// void InitChatRosterForNewDialogue( void )

/**
 *
 *
 */
void push_or_pop_chat_roster(int push_or_pop)
{
	static dialogue_option LocalChatRoster[MAX_DIALOGUE_OPTIONS_IN_ROSTER];

	if (push_or_pop == PUSH_ROSTER) {
		memcpy(LocalChatRoster, ChatRoster, sizeof(dialogue_option) * MAX_DIALOGUE_OPTIONS_IN_ROSTER);
		int i;
		for (i = 0; i < MAX_DIALOGUE_OPTIONS_IN_ROSTER; i++)
			clear_dialog_option(&ChatRoster[i]);
	} else if (push_or_pop == POP_ROSTER) {
		memcpy(ChatRoster, LocalChatRoster, sizeof(dialogue_option) * MAX_DIALOGUE_OPTIONS_IN_ROSTER);
	} else {
		ErrorMessage(__FUNCTION__, "\
There was an unrecognized parameter handled to this function.", PLEASE_INFORM, IS_FATAL);
	}

};				// push_or_pop_chat_roster ( int push_or_pop )

/**
 * This function plants a cookie, i.e. sets a new text string with the
 * purpose of serving as a flag.  These flags can be set/unset from the dialog
 * file and used from within there and they get stored and loaded with
 * every gave via the tux_t structure.
 */
void PlantCookie(const char *CookieString)
{
	int i;

	// Check if the cookie has already been set. 
	//
	for (i = 0; i < MAX_COOKIES; i++) {
		if (Me.cookie_list[i]) {
			if (!strcmp(Me.cookie_list[i], CookieString)) {
				DebugPrintf(0, "\n\nTHAT COOKIE WAS ALREADY SET... DOING NOTHING...\n\n");
				return;
			}
		}
	}

	// Now we find a good new and free position for our new cookie...
	//
	for (i = 0; i < MAX_COOKIES; i++) {
		if (!Me.cookie_list[i]) {
			break;
		}
	}

	// Maybe the position we have found is the last one.  That would mean too
	// many cookies, a case that should never occur in FreedroidRPG and that is
	// a considered a fatal error...
	if (i >= MAX_COOKIES) {
		fprintf(stderr, "\n\nCookieString: %s\n", CookieString);
		ErrorMessage(__FUNCTION__, "\
There were no more free positions available to store this cookie.\n\
This should not be possible without a severe bug in FreedroidRPG.", PLEASE_INFORM, IS_FATAL);
	}
	
	// Now that we know that we have found a good position for storing our
	// new cookie, we can do it.
	Me.cookie_list[i] = MyMalloc(strlen(CookieString)+1);
	strcpy(Me.cookie_list[i], CookieString);
	DebugPrintf(0, "\n\nNEW COOKIE STORED:  Position=%d Text='%s'.\n\n", i, CookieString);

};

/**
 * This function deletes planted cookie, i.e. delete a text string with the
 * purpose of serving as a flag.  These flags can be set/unset from the dialog
 * file and used from within there and they get stored and loaded with
 * every game via the tux_t structure.
 */
void DeleteCookie(const char *CookieString)
{
	int i;
	for (i = 0; i < MAX_COOKIES; i++) {
		if (!Me.cookie_list[i])
			continue;

		if (!strcmp(Me.cookie_list[i], CookieString))
			break;
	}

	if (i == MAX_COOKIES) {
		ErrorMessage(__FUNCTION__, "The specified cookie \"%s\" was not found.", PLEASE_INFORM, IS_WARNING_ONLY, CookieString);
	} else {
		free(Me.cookie_list[i]);
		for (i = i+1; i < MAX_COOKIES; i++) {
			Me.cookie_list[i - 1] = Me.cookie_list[i];
		}
		Me.cookie_list[MAX_COOKIES - 1] = NULL;

		DebugPrintf(1, "Cookie deleted.");
	}
};				// void DeleteCookie ( char* CookieString )

/**
 * This function should load new chat dialogue information from the 
 * chat info file into the chat roster.
 *
 */
static void load_dialog(const char *fpath)
{
	char *ChatData;
	char *SectionPointer;
	char *EndOfSectionPointer;
	int i;
	int OptionIndex;
	int NumberOfOptionsInSection;

#define CHAT_CHARACTER_BEGIN_STRING "Beginning of new chat dialog for character=\""
#define CHAT_CHARACTER_END_STRING "End of chat dialog for character"
#define NEW_OPTION_BEGIN_STRING "\nNr="
#define NO_TEXT_OPTION_STRING "NO_TEXT"

	// Read the whole chat file information into memory
	ChatData = ReadAndMallocAndTerminateFile(fpath, CHAT_CHARACTER_END_STRING);
	SectionPointer = ChatData;

	// Read the initialization code
	//
	if (chat_initialization_code) {
		free(chat_initialization_code);
		chat_initialization_code = NULL;
	}
	chat_initialization_code = ReadAndMallocStringFromDataOptional(SectionPointer, "FirstTime LuaCode={", "}");

	if (chat_startup_code) {
		free(chat_startup_code);
		chat_startup_code = NULL;
	}
	chat_startup_code = ReadAndMallocStringFromDataOptional(SectionPointer, "EveryTime LuaCode={", "}");

	// At first we go take a look on how many options we have
	// to decode from this section.
	//
	NumberOfOptionsInSection = CountStringOccurences(SectionPointer, NEW_OPTION_BEGIN_STRING);

	// Now we see which option index is assigned to this option.
	// It may happen, that some numbers are OMITTED here!  This
	// should be perfectly ok and allowed as far as the code is
	// concerned in order to give the content writers more freedom.
	//
	for (i = 0; i < NumberOfOptionsInSection; i++) {
		SectionPointer = LocateStringInData(SectionPointer, NEW_OPTION_BEGIN_STRING);
		ReadValueFromString(SectionPointer, NEW_OPTION_BEGIN_STRING, "%d",
				    &OptionIndex, SectionPointer + strlen(NEW_OPTION_BEGIN_STRING) + 50);

		SectionPointer++;

		// Find the end of this dialog option
		EndOfSectionPointer = strstr(SectionPointer, NEW_OPTION_BEGIN_STRING);

		if (EndOfSectionPointer) {	//then we are not the last option
			*EndOfSectionPointer = 0;
		}
		// Anything that is loaded into the chat roster doesn't need to be freed,
		// cause this will be done by the next 'InitChatRoster' function anyway.
		//
		ChatRoster[OptionIndex].option_text = ReadAndMallocStringFromDataOptional(SectionPointer, "Text=\"", "\"");
		if (!ChatRoster[OptionIndex].option_text) {
			ChatRoster[OptionIndex].option_text = ReadAndMallocStringFromData(SectionPointer, "Text=_\"", "\"");
		}

		ChatRoster[OptionIndex].no_text = strstr(SectionPointer, NO_TEXT_OPTION_STRING) > 0;

		if (strstr(SectionPointer, "LuaCode")) {
			ChatRoster[OptionIndex].lua_code = ReadAndMallocStringFromData(SectionPointer, "LuaCode={", "}");
		} else
			ChatRoster[OptionIndex].lua_code = NULL;

		ChatRoster[OptionIndex].exists = 1;

		if (EndOfSectionPointer)
			*EndOfSectionPointer = NEW_OPTION_BEGIN_STRING[0];
	}

	// Now we've got all the information we wanted from the dialogues file.
	// We can now free the loaded file again.  Upon a new character dialogue
	// being initiated, we'll just reload the file.  This is very convenient,
	// for it allows making and testing changes to the dialogues without even
	// having to restart Freedroid!  Very cool!
	//
	free(ChatData);
}

/**
 *
 *
 */
void make_sure_chat_portraits_loaded_for_this_droid(Enemy this_droid)
{
	SDL_Surface *Small_Droid;
	SDL_Surface *Large_Droid;
	char fpath[2048];
	char fname[500];
	int i;
	int model_number;
	static int first_call = TRUE;
	static int this_type_has_been_loaded[ENEMY_ROTATION_MODELS_AVAILABLE];

	// We make sure we only load the portrait files once and not
	// every time...
	//
	if (first_call) {
		for (i = 0; i < ENEMY_ROTATION_MODELS_AVAILABLE; i++)
			this_type_has_been_loaded[i] = FALSE;
	}
	first_call = FALSE;

	// We look up the model number for this chat partner.
	//
	model_number = this_droid->type;

	// We should make sure, that we don't double-load images that we have loaded
	// already, thereby wasting more resources, including OpenGL texture positions.
	//
	if (this_type_has_been_loaded[model_number])
		return;
	this_type_has_been_loaded[model_number] = TRUE;

	// At first we try to load the image, that is named after this
	// chat section.  If that succeeds, perfect.  If not, we'll revert
	// to a default image.
	//
	strcpy(fname, "droids/");
	strcat(fname, PrefixToFilename[Druidmap[model_number].individual_shape_nr]);
	strcat(fname, "/portrait.png");
	find_file(fname, GRAPHICS_DIR, fpath, 0);
	DebugPrintf(2, "\nFilename used for portrait: %s.", fpath);

	Small_Droid = our_IMG_load_wrapper(fpath);
	if (Small_Droid == NULL) {
		fprintf(stderr, "\n\nfpath: %s \n", fpath);
		ErrorMessage(__FUNCTION__, "\
It wanted to load a small portrait file in order to display it in the \n\
chat interface of Freedroid.  But:  Loading this file has ALSO failed.", PLEASE_INFORM, IS_FATAL);
	}

	Large_Droid = zoomSurface(Small_Droid, (float)Droid_Image_Window.w / (float)Small_Droid->w,
				  (float)Droid_Image_Window.w / (float)Small_Droid->w, 0);

	SDL_FreeSurface(Small_Droid);

	if (use_open_gl) {
		chat_portrait_of_droid[model_number].surface =
		    SDL_CreateRGBSurface(0, Large_Droid->w, Large_Droid->h, 32, rmask, gmask, bmask, amask);
		SDL_SetAlpha(Large_Droid, 0, SDL_ALPHA_OPAQUE);
		our_SDL_blit_surface_wrapper(Large_Droid, NULL, chat_portrait_of_droid[model_number].surface, NULL);
		flip_image_vertically(chat_portrait_of_droid[model_number].surface);
		SDL_FreeSurface(Large_Droid);
	} else
		chat_portrait_of_droid[model_number].surface = Large_Droid;

}

static void show_chat_up_button(void)
{
	ShowGenericButtonFromList(CHAT_LOG_SCROLL_UP_BUTTON);
}

static void show_chat_down_button(void)
{
	ShowGenericButtonFromList(CHAT_LOG_SCROLL_DOWN_BUTTON);
}

/**
 * During the Chat with a friendly droid or human, there is a window with
 * the full text transcript of the conversation so far.  This function is
 * here to display said text window and it's content, scrolled to the
 * position desired by the player himself.
 */
void show_chat_log(enemy *chat_enemy)
{
	blit_special_background(CHAT_DIALOG_BACKGROUND_PICTURE_CODE);
	our_SDL_blit_surface_wrapper(chat_portrait_of_droid[chat_enemy->type].surface, NULL, Screen, &Droid_Image_Window);

	chat_log.content_above_func = show_chat_up_button;
	chat_log.content_below_func = show_chat_down_button;
	ShowGenericButtonFromList(CHAT_LOG_SCROLL_OFF_BUTTON);
	ShowGenericButtonFromList(CHAT_LOG_SCROLL_OFF2_BUTTON);

	show_text_widget(&chat_log);
}

/**
 * Wait for a mouse click before continuing, updating the screen while waiting.
 */
static void wait_for_click(enemy *chat_droid)
{
	SDL_Event event;

	// Rectangle for the player response area
	SDL_Rect clip;
	clip.x = UNIVERSAL_COORD_W(37);
	clip.y = UNIVERSAL_COORD_H(336);
	clip.w = UNIVERSAL_COORD_W(640 - 70);
	clip.h = UNIVERSAL_COORD_H(118);

	while (1) {
		show_chat_log(chat_droid);
		SetCurrentFont(Menu_BFont);
		display_text(_("\1Click anywhere to continue..."), clip.x, clip.y, &clip);
		blit_mouse_cursor();
		our_SDL_flip_wrapper();
		SDL_Delay(1);

		SDL_WaitEvent(&event);

		if (event.type == SDL_QUIT) {
			Terminate(EXIT_SUCCESS, TRUE);
		}

		if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == 1)
			goto wait_for_click_return;

		else if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
			case SDLK_SPACE:
			case SDLK_RETURN:
			case SDLK_ESCAPE:
				goto wait_for_click_return;
			default:
				break;
			}
		}
	}
	 	
wait_for_click_return:
	while (EscapePressed() || MouseLeftPressed() || SpacePressed())
		;
	return;
}

/**
 * This function should first display a subtitle and then also a sound
 * sample.  It is not very sophisticated or complicated, but nevertheless
 * important, because this combination does indeed occur so often.
 */
void chat_add_response(const char *response, int no_wait, enemy *chat_droid)
{
	autostr_append(chat_log.text, response);
	chat_log.scroll_offset = 0;

	show_chat_log(chat_droid);
	blit_mouse_cursor();
	our_SDL_flip_wrapper();

	if (!no_wait)
		wait_for_click(chat_droid);
}

void run_subdialog(const char *tmp_filename)
{
	char fpath[2048];
	char finaldir[50];
	unsigned char dummyflags[MAX_ANSWERS_PER_PERSON];
	unsigned char *old_chat_flags = chat_control_chat_flags;

	memset(dummyflags, 0, MAX_ANSWERS_PER_PERSON);
	chat_control_chat_flags = &dummyflags[0];

	push_or_pop_chat_roster(PUSH_ROSTER);

	sprintf(finaldir, "%s", DIALOG_DIR);
	find_file(tmp_filename, finaldir, fpath, 0);

	load_dialog(fpath);

	// we always initialize subdialogs..
	//
	int i;
	for (i = 0; i < MAX_ANSWERS_PER_PERSON; i++) {
		dummyflags[i] = 0;
	}

	if (chat_initialization_code)
		run_lua(chat_initialization_code);

	run_chat(chat_control_chat_droid, TRUE);

	push_or_pop_chat_roster(POP_ROSTER);

	chat_control_chat_flags = old_chat_flags;
	chat_control_end_dialog = 0;
	chat_control_next_node = -1;
	chat_control_partner_started = 0;
}

/**
 * Process a chat option:
 * - Echo the option text of the chosen option, unless NO_TEXT is set.
 * - Run the associated LUA code.
 */
static void process_this_chat_option(int menu_selection, enemy *chat_droid)
{
	// Reset chat control variables for this option
	chat_control_end_dialog = 0;
	chat_control_next_node = -1;

	// Echo option text
	if (!ChatRoster[menu_selection].no_text) {
		autostr_append(chat_log.text, "\1- ");
		chat_add_response(L_(ChatRoster[menu_selection].option_text), FALSE, chat_droid);
		autostr_append(chat_log.text, "\n");
	}

	// Run LUA associated with this selection
	if (ChatRoster[menu_selection].lua_code)
		run_lua(ChatRoster[menu_selection].lua_code);
}

/**
 * This is the most important subfunction of the whole chat with friendly
 * droids and characters.  After the pure chat data has been loaded from
 * disk, this function is invoked to handle the actual chat interaction
 * and the dialog flow.
 */
static void run_chat(enemy *ChatDroid, int is_subdialog)
{
	int i;
	char *DialogMenuTexts[MAX_ANSWERS_PER_PERSON];
	chat_control_partner_started = (ChatDroid->will_rush_tux);

	// Reset chat control variables.
	chat_control_end_dialog = 0;
	chat_control_next_node = -1;

	/* Initialize the chat log widget. */
	if (!is_subdialog)
		init_text_widget(&chat_log, _("\3--- Start of Dialog ---\n"));

	chat_log.rect.x = CHAT_SUBDIALOG_WINDOW_X;
	chat_log.rect.y = CHAT_SUBDIALOG_WINDOW_Y;
	chat_log.rect.w = CHAT_SUBDIALOG_WINDOW_W;
	chat_log.rect.h = CHAT_SUBDIALOG_WINDOW_H;
	chat_log.font = FPS_Display_BFont;
	chat_log.line_height_factor = LINE_HEIGHT_FACTOR;

	// We load the option texts into the dialog options variable..
	//
	for (i = 0; i < MAX_ANSWERS_PER_PERSON; i++) {
		if (strlen(ChatRoster[i].option_text)) {
			DialogMenuTexts[i] = L_(ChatRoster[i].option_text);
		}
	}
	// DialogMenuTexts [ MAX_ANSWERS_PER_PERSON - 1 ] = " END ";

	while (1) {
		// Now we run the startup code
		if (chat_startup_code && chat_control_next_node == -1) {
			run_lua(chat_startup_code);
			free(chat_startup_code);
			chat_startup_code = NULL;	//and free it immediately so as not to run it again in this session

			if (chat_control_end_dialog)
				goto wait_click_and_out;
 		}

		if (chat_control_next_node == -1) {
			chat_control_next_node =
				chat_do_menu_selection_flagged(DialogMenuTexts, ChatDroid);
			// We do some correction of the menu selection variable:
			// The first entry of the menu will give a 1 and so on and therefore
			// we need to correct this to more C style.
			//
			chat_control_next_node--;
		}

		if ((chat_control_next_node >= MAX_ANSWERS_PER_PERSON) || (chat_control_next_node < 0)) {
			DebugPrintf(0, "%s: Error: chat_control_next_node %i out of range!\n", __FUNCTION__, chat_control_next_node);
			chat_control_next_node = END_ANSWER;
		}

		process_this_chat_option(chat_control_next_node, ChatDroid);
			
		if (chat_control_end_dialog)
			goto wait_click_and_out;
	}

wait_click_and_out:
	while (EscapePressed() || MouseLeftPressed() || SpacePressed());
}

/**
 * This is more or less the 'main' function of the chat with friendly 
 * droids and characters.  It is invoked directly from the user interface
 * function as soon as the player requests communication or there is a
 * friendly bot who rushes Tux and opens talk.
 */
void ChatWithFriendlyDroid(enemy * ChatDroid)
{
	int i;
	char *DialogMenuTexts[MAX_ANSWERS_PER_PERSON];
	char fpath[2048];
	char tmp_filename[5000];
	struct npc *npc;

	chat_control_chat_droid = ChatDroid;

	Activate_Conservative_Frame_Computation();

	make_sure_chat_portraits_loaded_for_this_droid(ChatDroid);

	// First we empty the array of possible answers in the
	// chat interface.
	//
	for (i = 0; i < MAX_ANSWERS_PER_PERSON; i++) {
		DialogMenuTexts[i] = "";
	}

	// We clean out the chat roster from any previous use
	//
	InitChatRosterForNewDialogue();

	npc = npc_get(ChatDroid->dialog_section_name);
	if (!npc)
		return;

	strcpy(tmp_filename, ChatDroid->dialog_section_name);
	strcat(tmp_filename, ".dialog");
	char finaldir[50];
	sprintf(finaldir, "%s", DIALOG_DIR);
	find_file(tmp_filename, finaldir, fpath, 0);
	load_dialog(fpath);

	chat_control_chat_flags = &npc->chat_flags[0];

	if (!npc->chat_character_initialized) {
		int i;
		for (i = 0; i < MAX_ANSWERS_PER_PERSON; i++) {
			chat_control_chat_flags[i] = 0;
		}

		if (chat_initialization_code)
			run_lua(chat_initialization_code);

		npc->chat_character_initialized = 1;
	}

	run_chat(ChatDroid, FALSE);
}

/**
 * Validate dialogs syntax and Lua code 
 * This validator loads all known dialogs, checking them for syntax. 
 * It tries to compile each Lua code snippet in the dialogs, checking them for syntax as well.
 * It then proceeds to executing each Lua code snippet, which makes it possible to check for some errors
 * such as typos in function calls (calls to non existing functions). However, this runtime check does not cover 100% of 
 * the Lua code because of branches (when it encounters a if, it will not test both the "then" and "else" branches).
 *
 * As a result, the fact that the validator finds no error does not imply there are no errors in dialogs. 
 * Syntax is checked fully, but runtime validation cannot check all of the code.
 */
int validate_dialogs()
{
	int j, k;
	const char *basename;
	char filename[1024];
	char fpath[2048];
	enemy *dummy_partner;
	struct npc *n;
	int error_caught = FALSE;

	skip_initial_menus = 1;

	find_file("freedroid.levels", MAP_DIR, fpath, 0);
	LoadShip(fpath, 0);
	PrepareStartOfNewCharacter("NewTuxStartGameSquare");

	/* Disable sound to speed up validation. */
	int old_sound_on = sound_on;
	sound_on = FALSE;

	/* Temporarily disable screen fadings to speed up validation. */
	GameConfig.do_fadings = FALSE;

	/* _says functions are not run by the validator, as they display
	   text on screen and wait for clicks */
	run_lua("function npc_says(a)\nend\n");
	run_lua("function tux_says(a)\nend\n");
	run_lua("function cli_says(a)\nend\n");

	/* Subdialogs currently call run_chat and we cannot do that when validating dialogs */
	run_lua("function run_subdialog(a)\nend\n");

	/* Shops must not be run (display + wait for clicks) */
	run_lua("function trade_with(a)\nend\n");
	
	/* drop_dead cannot be tested because it means we would try to kill our dummy bot
	 several times, which is not allowed by the engine */
	run_lua("function drop_dead(a)\nend\n");

	run_lua("function user_input_string(a)\nreturn \"dummy\";\nend\n");

	run_lua("function upgrade_items(a)\nend\n");
	run_lua("function craft_addons(a)\nend\n");
	/* set_bot_destination cannot be tested because this may be invoked when the bot is
	 diffrent level then when the bot starts */
	run_lua("function set_bot_destination(a)\nend\n");

	/* This dummy will be used to test break_off_and_attack() functions and such. */
	BROWSE_ALIVE_BOTS(dummy_partner) {
		break;
	}

	chat_control_chat_droid = dummy_partner;

	list_for_each_entry(n, &npc_head, node) {
		basename = n->dialog_basename;
		chat_control_chat_flags = &n->chat_flags[0];

		printf("Testing dialog \"%s\"...\n", basename);
		sprintf(filename, "%s.dialog", basename);

		find_file(filename, DIALOG_DIR, fpath, 0);

		InitChatRosterForNewDialogue();
		load_dialog(fpath);

		if (chat_initialization_code) {
			printf("\tinit code\n");
			run_lua(chat_initialization_code);
		}


		if (chat_startup_code) {
			printf("\tstartup code\n");
			run_lua(chat_startup_code);
		}
		k = 0;
		for (j = 0; j < MAX_DIALOGUE_OPTIONS_IN_ROSTER; j++) {
			if (ChatRoster[j].lua_code) {
				if (!(k%5)) {
					printf("\n");
				}
				printf("|node %2d|\t", j);
				run_lua(ChatRoster[j].lua_code);
				k++;
			}
		}
		if(!ChatRoster[END_ANSWER].lua_code){
			printf("\n\nERROR: \"%s\" Dialog missing END NODE (node %d).\n\n", n->dialog_basename, END_ANSWER);
			error_caught = TRUE;
			break;
			//ErrorMessage(__FUNCTION__, " NO_NEED_TO_INFORM, IS_FATAL,
			//		
		}

		printf("\n... dialog OK\n");
	}

	/* Re-enable sound as needed. */
	sound_on = old_sound_on;

	/* Re-enable screen fadings. */
	GameConfig.do_fadings = TRUE;

	return error_caught;
}

#undef _chat_c
