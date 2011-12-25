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

#include "widgets/widgets.h"

#define PUSH_ROSTER 2
#define POP_ROSTER 3
#define CHAT_TOPIC_STACK_SIZE 10

static char *chat_initialization_code;	//first time with a character-code
static char *chat_startup_code;	//every time we start this dialog-code

// Topic stack
static char *topic_stack[CHAT_TOPIC_STACK_SIZE];
static unsigned int topic_stack_slot;

static void run_chat(enemy *ChatDroid, int is_subdialog);

/**
 * This function resets a dialog option to "empty" default values. It does NOT free the strings, this has to be done
 * prior to calling this function when needed.
 */
static void clear_dialog_option(dialogue_option * d)
{
	d->topic = "";
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
		if (strlen(ChatRoster[i].topic))
			free(ChatRoster[i].topic);
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
 * \brief Push a new topic in the topic stack.
 * \param topic The topic name.
 */
void chat_push_topic(const char *topic)
{
	if (topic_stack_slot < CHAT_TOPIC_STACK_SIZE - 1) {
		topic_stack[++topic_stack_slot] = strdup(topic);
	} else {
		ErrorMessage(__FUNCTION__,
			     "The maximum depth of %hd topics has been maxed out.",
			     PLEASE_INFORM, IS_WARNING_ONLY, CHAT_TOPIC_STACK_SIZE);
	}
}

/**
 * \brief Pop the top topic in the topic stack.
 */
void chat_pop_topic() {
	if (topic_stack_slot > 0) {
		free(topic_stack[topic_stack_slot]);
		topic_stack_slot--;
	} else {
		ErrorMessage(__FUNCTION__,
			     "Root topic can't be popped.",
			     PLEASE_INFORM, IS_WARNING_ONLY);
	}
}

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
	chat_initialization_code = ReadAndMallocStringFromDataOptional(SectionPointer, "<FirstTime LuaCode>", "</LuaCode>");

	if (chat_startup_code) {
		free(chat_startup_code);
		chat_startup_code = NULL;
	}
	chat_startup_code = ReadAndMallocStringFromDataOptional(SectionPointer, "<EveryTime LuaCode>", "</LuaCode>");

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

		ChatRoster[OptionIndex].topic = ReadAndMallocStringFromDataOptional(SectionPointer, "Topic=\"", "\"");

		if (!ChatRoster[OptionIndex].topic) {
			ChatRoster[OptionIndex].topic = "";
		}

		ChatRoster[OptionIndex].no_text = strstr(SectionPointer, NO_TEXT_OPTION_STRING) > 0;

		if (strstr(SectionPointer, "LuaCode")) {
			ChatRoster[OptionIndex].lua_code = ReadAndMallocStringFromData(SectionPointer, "<LuaCode>", "</LuaCode>");
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
	// having to restart FreedroidRPG!  Very cool!
	//
	free(ChatData);
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
	struct image *img = get_droid_portrait_image(chat_enemy->type);
	float scale;
	moderately_finepoint pos;

	blit_background("conversation.png");

	// Compute the maximum uniform scale to apply to the bot image so that it fills
	// the droid portrait image, and center the image.
	scale = min((float)Droid_Image_Window.w / (float)img->w, (float)Droid_Image_Window.h / (float)img->h);
	pos.x = (float)Droid_Image_Window.x + ((float)Droid_Image_Window.w - (float)img->w * scale) / 2.0;
	pos.y = (float)Droid_Image_Window.y + ((float)Droid_Image_Window.h - (float)img->h * scale) / 2.0;

	display_image_on_screen(img, pos.x, pos.y, IMAGE_SCALE_TRANSFO(scale));

	chat_log.content_above_func = show_chat_up_button;
	chat_log.content_below_func = show_chat_down_button;
	ShowGenericButtonFromList(CHAT_LOG_SCROLL_OFF_BUTTON);
	ShowGenericButtonFromList(CHAT_LOG_SCROLL_OFF2_BUTTON);

	widget_text_display(WIDGET(&chat_log));
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
	autostr_append(chat_log.text, "%s", response);
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
	uint8_t dummyflags[MAX_ANSWERS_PER_PERSON];
	uint8_t *old_chat_flags = chat_control_chat_flags;

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
		run_lua(LUA_DIALOG, ChatRoster[menu_selection].lua_code);
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

	// Reset topic stack variables
	for(i = 1; i <= topic_stack_slot ; i++) {
		free(topic_stack[i]);
	}
	topic_stack_slot = 0;
	topic_stack[0] = "";

	/* Initialize the chat log widget. */
	if (!is_subdialog)
		widget_text_init(&chat_log, _("\3--- Start of Dialog ---\n"));

	widget_set_rect(WIDGET(&chat_log), CHAT_SUBDIALOG_WINDOW_X, CHAT_SUBDIALOG_WINDOW_Y, CHAT_SUBDIALOG_WINDOW_W, CHAT_SUBDIALOG_WINDOW_H);
	chat_log.font = FPS_Display_BFont;
	chat_log.line_height_factor = LINE_HEIGHT_FACTOR;

	if (chat_initialization_code)
		run_lua(LUA_DIALOG, chat_initialization_code);

	// We load the option texts into the dialog options variable..
	//
	for (i = 0; i < MAX_ANSWERS_PER_PERSON; i++) {
		if (strlen(ChatRoster[i].option_text)) {
			DialogMenuTexts[i] = L_(ChatRoster[i].option_text);
		}
	}

	while (1) {
		// Now we run the startup code
		if (chat_startup_code && chat_control_next_node == -1) {
			run_lua(LUA_DIALOG, chat_startup_code);
			free(chat_startup_code);
			chat_startup_code = NULL;	//and free it immediately so as not to run it again in this session

			if (chat_control_end_dialog)
				goto wait_click_and_out;
 		}

		if (chat_control_next_node == -1) {
			chat_control_next_node =
				chat_do_menu_selection_filtered(DialogMenuTexts, ChatDroid, 
								topic_stack[topic_stack_slot]);
			// We do some correction of the menu selection variable:
			// The first entry of the menu will give a 1 and so on and therefore
			// we need to correct this to more C style.
			//
			chat_control_next_node--;
		}

		if ((chat_control_next_node >= MAX_ANSWERS_PER_PERSON) || (chat_control_next_node < 0)) {
			DebugPrintf(0, "%s: Error: chat_control_next_node %i out of range!\n", __FUNCTION__, chat_control_next_node);
			goto wait_click_and_out;
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
	char fpath[2048];
	char tmp_filename[5000];
	struct npc *npc;

	chat_control_chat_droid = ChatDroid;

	Activate_Conservative_Frame_Computation();

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
	} else {
		free(chat_initialization_code);
		chat_initialization_code = NULL;
	}

	run_chat(ChatDroid, FALSE);

	if (!npc->chat_character_initialized)
		npc->chat_character_initialized = TRUE;
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

	find_file("levels.dat", MAP_DIR, fpath, 0);
	LoadShip(fpath, 0);
	PrepareStartOfNewCharacter("NewTuxStartGameSquare");

	/* Disable sound to speed up validation. */
	int old_sound_on = sound_on;
	sound_on = FALSE;

	/* Temporarily disable screen fadings to speed up validation. */
	GameConfig.do_fadings = FALSE;

	/* _says functions are not run by the validator, as they display
	   text on screen and wait for clicks */
	run_lua(LUA_DIALOG, "function npc_says(a)\nend\n");
	run_lua(LUA_DIALOG, "function tux_says(a)\nend\n");
	run_lua(LUA_DIALOG, "function cli_says(a)\nend\n");

	/* Subdialogs currently call run_chat and we cannot do that when validating dialogs */
	run_lua(LUA_DIALOG, "function run_subdialog(a)\nend\n");

	/* Shops must not be run (display + wait for clicks) */
	run_lua(LUA_DIALOG, "function trade_with(a)\nend\n");
	
	/* drop_dead cannot be tested because it means we would try to kill our dummy bot
	 several times, which is not allowed by the engine */
	run_lua(LUA_DIALOG, "function drop_dead(a)\nend\n");

	run_lua(LUA_DIALOG, "function user_input_string(a)\nreturn \"dummy\";\nend\n");

	run_lua(LUA_DIALOG, "function upgrade_items(a)\nend\n");
	run_lua(LUA_DIALOG, "function craft_addons(a)\nend\n");
	/* set_bot_destination cannot be tested because this may be invoked when the bot is
	 on a different level than where the bot starts */
	run_lua(LUA_DIALOG, "function set_bot_destination(a)\nend\n");

	/* takeover requires user input - hardcode it to win */
	run_lua(LUA_DIALOG, "function takeover(a)\nreturn true\nend\n");

	/* push_topic() and pop_topic() cannot be tested because the validator doesn't follow 
	 the logical order of nodes */
	run_lua(LUA_DIALOG, "function topic(a)\nend\n");
	run_lua(LUA_DIALOG, "function pop_topic(a)\nend\n");

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
			run_lua(LUA_DIALOG, chat_initialization_code);
		}


		if (chat_startup_code) {
			printf("\tstartup code\n");
			run_lua(LUA_DIALOG, chat_startup_code);
		}
		k = 0;
		for (j = 0; j < MAX_DIALOGUE_OPTIONS_IN_ROSTER; j++) {
			if (ChatRoster[j].lua_code) {
				if (!(k%5)) {
					printf("\n");
				}
				printf("|node %2d|\t", j);
				run_lua(LUA_DIALOG, ChatRoster[j].lua_code);
				k++;
			}
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
