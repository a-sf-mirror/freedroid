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

/* Some forward declarations */
static void run_chat();
static void chat_delete_context(struct chat_context *);

/* Stack of chat contexts. The current context is at the top of the stack */
static struct list_head chat_context_stack = LIST_HEAD_INIT(chat_context_stack);

static struct widget_group *chat_menu = NULL;
struct widget_text *chat_log = NULL;
struct widget_text_list *chat_selector = NULL;
static struct widget_group *chat_wait = NULL;

static struct image **droid_portrait = NULL;
static SDL_Rect chat_selector_inner_rect;

/**
 * Return the current chat context.
 *
 * The current context is the one at top of the contexts stack.
 *
 * @return A pointer to the current chat context, or NULL if no context is stacked.
 */
struct chat_context *chat_get_current_context(void)
{
	if (list_empty(&chat_context_stack))
		return NULL;

	return (struct chat_context *)list_entry(chat_context_stack.next, struct chat_context, stack_node);
}

/**
 * Push a chat context on top of the stack.
 *
 * The pushed context becomes the current one.
 *
 * @param chat_context Pointer to the chat context to push.
 */
static void chat_push_context(struct chat_context *chat_context)
{
	list_add(&chat_context->stack_node, &chat_context_stack);
}

/** Pop from the chat contexts stack.
 *
 * This function deletes the context at the top of the list.
 * The new top context becomes the current one.
 */
static void chat_pop_context(void)
{
	struct chat_context *top_chat_context = chat_get_current_context();
	if (top_chat_context) {
		list_del(chat_context_stack.next);
		chat_delete_context(top_chat_context);
	}
}

/**
 * Initialize a dialog option to "empty" default values.
 *
 * @param d Pointer to the struct dialogue_option to initialize.
 */
static void init_dialog_option(dialogue_option *d)
{
	d->topic = NULL;
	d->option_text = NULL;
	d->no_text = 0;
	d->lua_code = NULL;
	d->exists = 0;
}

/**
 * Free the memory used by a dialogue_option.
 *
 * @param d Pointer to the struct dialogue_option to free.
 */
static void delete_dialog_option(dialogue_option *d)
{
	free(d->topic);
	free(d->option_text);
	free(d->lua_code);
}

/**
 * Create a chat context and initialize it.
 *
 * @param partner Pointer to the enemy to talk with
 * @param npc Pointer to the npc containing the dialogue definition.
 * @param is_subdialog True if the dialog to start is a subdialog.
 *
 * @return Pointer to the allocated chat context.
 */
static struct chat_context *chat_create_context(enemy *partner, struct npc *npc, int is_subdialog)
{
	int i;
	struct chat_context *new_chat_context = (struct chat_context *)MyMalloc(sizeof(struct chat_context));

	// Init chat control variables.
	// MyMalloc() already zeroed the struct, so we only initialize 'non zero' fields.

	new_chat_context->is_subdialog = is_subdialog;
	new_chat_context->partner = partner;
	new_chat_context->npc = npc;
	new_chat_context->partner_started = partner->will_rush_tux;

	// topic_stack[0] is never replaced, so a static allocation is enough.
	new_chat_context->topic_stack[0] = "";

	for (i = 0; i < MAX_DIALOGUE_OPTIONS_IN_ROSTER; i++) {
		init_dialog_option(&new_chat_context->dialog_options[i]);
	}

	// Only run the init script the first time the dialog is open.
	// An init script on a subdialog does not really make sense, so it is just
	// ignored.
	// The startup script is run every time a dialog or a subdialog is open.
	new_chat_context->state = RUN_INIT_SCRIPT;
	if (npc->chat_character_initialized || is_subdialog) {
		new_chat_context->state = RUN_STARTUP_SCRIPT;
	}

	// dialog_flags points to the npc chat_flags array, so that currently
	// active dialog options are saved thanks to the save of the npc
	// structures.
	// However, subdialogs are not referenced by the npc, and so their states
	// can not be saved. A 'dummy' chat_flags array is thus needed during the
	// run of a subdialog, that dummy array being lost when the subdialog is
	// closed.
	if (!is_subdialog) {
		new_chat_context->dialog_flags = &npc->chat_flags[0];
		// Ensure the initialization of the option flags on the first run of
		// the dialog.
		if (!npc->chat_character_initialized) {
			for (i = 0; i < MAX_DIALOGUE_OPTIONS_IN_ROSTER; i++) {
				new_chat_context->dialog_flags[i] = 0;
			}
		}
	} else {
		uint8_t *dummy_chat_flags = (uint8_t *)MyMalloc(sizeof(uint8_t)*MAX_DIALOGUE_OPTIONS_IN_ROSTER);
		new_chat_context->dialog_flags = dummy_chat_flags;
	}

	// No dialog option currently selected by the user
	new_chat_context->current_option = -1;

	return new_chat_context;
}

/**
 * Delete a chat_context (free all the memory allocations)
 *
 * @param chat_context Pointer to the caht_context to delete.
 */
static void chat_delete_context(struct chat_context *chat_context)
{
	int i;

	free(chat_context->initialization_code);
	free(chat_context->startup_code);

	// topic_stack[0] is not dynamically allocated, so there is no need to free it.
	for (i = chat_context->topic_stack_slot; i > 0; i--)
		free(chat_context->topic_stack[chat_context->topic_stack_slot]);

	for (i = 0; i < MAX_DIALOGUE_OPTIONS_IN_ROSTER; i++) {
		delete_dialog_option(&chat_context->dialog_options[i]);
	}

	if (chat_context->is_subdialog) {
		free(chat_context->dialog_flags);
	}

	free(chat_context);
}

/**
 * \brief Push a new topic in the topic stack.
 * \param topic The topic name.
 */
void chat_push_topic(const char *topic)
{
	struct chat_context *current_chat_context = chat_get_current_context();

	if (current_chat_context->topic_stack_slot < CHAT_TOPIC_STACK_SIZE - 1) {
		current_chat_context->topic_stack[++current_chat_context->topic_stack_slot] = strdup(topic);
	} else {
		ErrorMessage(__FUNCTION__,
			     "The maximum depth of %hd topics has been maxed out.",
			     PLEASE_INFORM, IS_WARNING_ONLY, CHAT_TOPIC_STACK_SIZE);
	}
}

/**
 * \brief Pop the top topic in the topic stack.
 */
void chat_pop_topic()
{
	struct chat_context *current_chat_context = chat_get_current_context();

	if (current_chat_context->topic_stack_slot > 0) {
		free(current_chat_context->topic_stack[current_chat_context->topic_stack_slot]);
		current_chat_context->topic_stack[current_chat_context->topic_stack_slot] = NULL;
		current_chat_context->topic_stack_slot--;
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
static void load_dialog(const char *fpath, struct chat_context *context)
{
	char *ChatData;
	char *SectionPointer;
	char *EndOfSectionPointer;
	int i;
	int OptionIndex;
	int NumberOfOptionsInSection;
	char *text;

#define CHAT_CHARACTER_BEGIN_STRING "Beginning of new chat dialog for character=\""
#define CHAT_CHARACTER_END_STRING "End of chat dialog for character"
#define NEW_OPTION_BEGIN_STRING "\nNr="
#define NO_TEXT_OPTION_STRING "NO_TEXT"

	// Read the whole chat file information into memory
	ChatData = ReadAndMallocAndTerminateFile(fpath, CHAT_CHARACTER_END_STRING);
	SectionPointer = ChatData;

	// Read the initialization code
	//
	context->initialization_code = ReadAndMallocStringFromDataOptional(SectionPointer, "<FirstTime LuaCode>", "</LuaCode>");
	context->startup_code = ReadAndMallocStringFromDataOptional(SectionPointer, "<EveryTime LuaCode>", "</LuaCode>");

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
		text = ReadAndMallocStringFromDataOptional(SectionPointer, "Text=\"", "\"");
		if (!text) {
			text = ReadAndMallocStringFromData(SectionPointer, "Text=_\"", "\"");
		}
		context->dialog_options[OptionIndex].option_text = L_(text);

		text = ReadAndMallocStringFromDataOptional(SectionPointer, "Topic=\"", "\"");
		if (text) {
			context->dialog_options[OptionIndex].topic = text;
		} else {
			// topic content has to be malloc'd.
			context->dialog_options[OptionIndex].topic = strdup("");
		}

		context->dialog_options[OptionIndex].no_text = strstr(SectionPointer, NO_TEXT_OPTION_STRING) > 0;

		context->dialog_options[OptionIndex].lua_code = ReadAndMallocStringFromDataOptional(SectionPointer, "<LuaCode>", "</LuaCode>");

		context->dialog_options[OptionIndex].exists = 1;

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

/**
 * Fill the chat selector widget with the currently selected options
 */
static void fill_chat_selector(struct chat_context *context)
{
	char *empty_entries[] = { NULL };
	int i;

	widget_text_list_init(chat_selector, empty_entries, NULL);
	char *topic = context->topic_stack[context->topic_stack_slot];
	// We filter out those answering options that are allowed by the flag mask
	for (i = 0; i < MAX_DIALOGUE_OPTIONS_IN_ROSTER; i++) {
		if (context->dialog_flags[i] && !(strcmp(topic, context->dialog_options[i].topic))) {
			widget_text_list_add(chat_selector, context->dialog_options[i].option_text, i);
		}
	}
}

/**
 * During the Chat with a friendly droid or human, there is a window with
 * the full text transcript of the conversation so far.  This function is
 * here to display said text window and it's content, scrolled to the
 * position desired by the player himself.
 *
 * TODO: To be removed once the chat screen is integrated into the main widget
 *       hierarchy
 */
void show_chat(struct chat_context *context)
{
	struct widget *ui = WIDGET(chat_menu);

	// Prepare chat screen rendering
	*droid_portrait = get_droid_portrait_image(context->partner->type);
	clear_screen();

	ui->update_tree(ui);
	ui->display(ui);
}

/**
 * This function is used for darkening the screen area outside the chat screen.
 */
static void display_dark_background(struct widget *w)
{
	draw_rectangle(&w->rect, 0, 0, 0, 50);
}

/*
 * Return TRUE if we are waiting for the user to click, after a response was
 * added in the chat log.
 */
static int waiting_user_click()
{
	struct chat_context *context = chat_get_current_context();
	if (!context)
		return FALSE;
	return context->wait_user_click;
}

/*
 * Unset the wait_user_click flag in the current chat context
 */
static void stop_wait_user_click(struct widget_button *w)
{
	struct chat_context *context = chat_get_current_context();
	if (context)
		context->wait_user_click = FALSE;
}

/*
 * Called when an option is selected in the chat_selector.
 * Get the index of the selected dialog node, and store it in the current
 * chat context.
 */
static void dialog_option_selected(struct widget_text_list *wl)
{
	int user_data = widget_text_list_get_data(wl, wl->selected_entry);
	if (user_data >= 0) {
		struct chat_context *context = chat_get_current_context();
		if (context)
			context->current_option = user_data;
	}
}

/**
 * This function builds the chat interface if it hasn't already been initialized.
 */
struct widget_group *create_chat_dialog()
{
	if (chat_menu)
		return chat_menu;

	chat_menu = widget_group_create();

	// Dark background
	struct widget *dark_background = widget_create();
	widget_set_rect(dark_background, 0, 0, GameConfig.screen_width, GameConfig.screen_height);
	dark_background->display = display_dark_background;
	widget_group_add(chat_menu, dark_background);

	// Entry text selector
	int chat_selector_h = UNIVERSAL_COORD_H(202);
	int chat_selector_w = UNIVERSAL_COORD_W(627);
	int chat_selector_x = (GameConfig.screen_width - chat_selector_w) / 2;
	int chat_selector_y = GameConfig.screen_height - chat_selector_h - 5;

	struct widget_background *chat_selector_bkg = widget_background_create();
	widget_set_rect(WIDGET(chat_selector_bkg), chat_selector_x, chat_selector_y, chat_selector_w, chat_selector_h);
	widget_background_load_3x3_tiles(chat_selector_bkg, "widgets/chat_typo");
	widget_group_add(chat_menu, WIDGET(chat_selector_bkg));

	int left_padding = 35;
	int right_padding = 35;
	int top_padding = 65;
	int bottom_padding = 20;

	chat_selector_inner_rect.x = chat_selector_x + left_padding;
	chat_selector_inner_rect.y = chat_selector_y + top_padding;
	chat_selector_inner_rect.h = chat_selector_h - top_padding - bottom_padding;
	chat_selector_inner_rect.w = chat_selector_w - left_padding - right_padding;

	chat_selector = widget_text_list_create();
	widget_set_rect(WIDGET(chat_selector), chat_selector_inner_rect.x, chat_selector_inner_rect.y, chat_selector_inner_rect.w, chat_selector_inner_rect.h);
	chat_selector->process_entry = dialog_option_selected;
	WIDGET(chat_selector)->update = WIDGET_UPDATE_FLAG_ON_DATA(WIDGET, enabled, !waiting_user_click());
	widget_group_add(chat_menu, WIDGET(chat_selector));

	// Droid portrait
	struct image *img = widget_load_image_resource("widgets/chat_portrait.png", 0);
	int chat_portrait_x = chat_selector_x;
	int chat_portrait_y = chat_selector_y - img->h + 39;

	struct widget_background *chat_portrait_bkg = widget_background_create();
	widget_background_add(chat_portrait_bkg, img, chat_portrait_x, chat_portrait_y, img->w, img->h);
	widget_group_add(chat_menu, WIDGET(chat_portrait_bkg));

	struct widget_button *chat_portrait = widget_button_create();
	widget_set_rect(WIDGET(chat_portrait), chat_portrait_x + 30, chat_portrait_y + 32, 136, 183);
	widget_group_add(chat_menu, WIDGET(chat_portrait));
	droid_portrait = &chat_portrait->image[0][0];

	// Chat log
	int chat_log_x = chat_selector_x + img->w - 30;
	int chat_log_y = 6;
	int chat_log_h = GameConfig.screen_height - chat_selector_h + 10;
	int chat_log_w = chat_selector_x + chat_selector_w - chat_log_x;

	struct widget_background *chat_log_bkg = widget_background_create();
	widget_set_rect(WIDGET(chat_log_bkg), chat_log_x, chat_log_y, chat_log_w, chat_log_h);
	widget_background_load_3x3_tiles(chat_log_bkg, "widgets/chat_log");
	widget_group_add(chat_menu, WIDGET(chat_log_bkg));

	chat_log = widget_text_create();
	widget_set_rect(WIDGET(chat_log), chat_log_x + 50, chat_log_y + 28, chat_log_w - 80, chat_log_h - 66);
	chat_log->font = FPS_Display_BFont;
	chat_log->line_height_factor = LINE_HEIGHT_FACTOR;
	widget_group_add(chat_menu, WIDGET(chat_log));

	struct {
		char *image[3];
		SDL_Rect rect;
		void (*activate_button)(struct widget_button *);
		void (*update)(struct widget *);
	} b[] = {
		// Chat Log scroll up
		{
			{"widgets/scroll_up_off.png", NULL, "widgets/scroll_up.png"},
			{chat_log_x + chat_log_w / 2 - 59, chat_log_y - 1, 118, 17},
			WIDGET_EXECUTE(struct widget_button *wb, widget_text_scroll_up(chat_log)),
			WIDGET_UPDATE_FLAG_ON_DATA(WIDGET_BUTTON, active, widget_text_can_scroll_up(chat_log))
		},
		// Chat Log scroll down
		{
			{"widgets/scroll_down_off.png", NULL, "widgets/scroll_down.png"},
			{chat_log_x + chat_log_w / 2 - 59, chat_log_y + chat_log_h - 26, 118, 17},
			WIDGET_EXECUTE(struct widget_button *wb, widget_text_scroll_down(chat_log)),
			WIDGET_UPDATE_FLAG_ON_DATA(WIDGET_BUTTON, active, widget_text_can_scroll_down(chat_log))
		},
		// Chat Selector Scroll up
		{
			{"widgets/scroll_up_off.png", NULL, "widgets/scroll_up.png"},
			{chat_selector_x + chat_selector_w / 2 - 59, chat_selector_y + 37, 118, 17},
			WIDGET_EXECUTE(struct widget_button *wb, widget_text_list_scroll_up(chat_selector)),
			WIDGET_UPDATE_FLAG_ON_DATA(WIDGET_BUTTON, active, widget_text_list_can_scroll_up(chat_selector))
		},
		// Chat Selector Scroll down
		{
			{"widgets/scroll_down_off.png", NULL, "widgets/scroll_down.png"},
			{chat_selector_x + chat_selector_w / 2 - 59, chat_selector_y + chat_selector_h - 23, 118, 17},
			WIDGET_EXECUTE(struct widget_button *wb, widget_text_list_scroll_down(chat_selector)),
			WIDGET_UPDATE_FLAG_ON_DATA(WIDGET_BUTTON, active, widget_text_list_can_scroll_down(chat_selector))
		}
	};

	int i;
	for (i = 0; i < sizeof(b) / sizeof(b[0]); i++) {
		struct widget_button *button = widget_button_create();

		WIDGET(button)->rect = b[i].rect;
		WIDGET(button)->update = b[i].update;

		button->image[0][DEFAULT] = widget_load_image_resource(b[i].image[0], 0);
		button->image[0][PRESSED] = widget_load_image_resource(b[i].image[1], 0);
		button->image[1][DEFAULT] = widget_load_image_resource(b[i].image[2], 0);
		button->activate_button = b[i].activate_button;

		widget_group_add(chat_menu, WIDGET(button));
	}

	// Wait for user click
	chat_wait = widget_group_create();
	widget_set_rect(WIDGET(chat_wait), 0, 0, GameConfig.screen_width, GameConfig.screen_height);
	WIDGET(chat_wait)->enabled = FALSE;
	WIDGET(chat_wait)->update =  WIDGET_UPDATE_FLAG_ON_DATA(WIDGET, enabled, waiting_user_click());

	struct widget_text* wait_text = widget_text_create();
	widget_set_rect(WIDGET(wait_text), chat_selector_inner_rect.x, chat_selector_inner_rect.y,
	                chat_selector_inner_rect.w, chat_selector_inner_rect.h);
	wait_text->font = FPS_Display_BFont;
	widget_text_init(wait_text, "Click anywhere to continue...");
	widget_group_add(chat_wait, WIDGET(wait_text));

	struct widget_button *wait_but = widget_button_create();
	widget_set_rect(WIDGET(wait_but), 0, 0, GameConfig.screen_width, GameConfig.screen_height);
	WIDGET(wait_but)->display = NULL;
	wait_but->activate_button = stop_wait_user_click;
	widget_group_add(chat_wait, WIDGET(wait_but));

	widget_group_add(chat_menu, WIDGET(chat_wait));

	WIDGET(chat_menu)->enabled = FALSE;

	return chat_menu;
}

/**
 * Wait for a mouse click before continuing.
 */
static void wait_for_click(struct chat_context *context)
{
	SDL_Event event;

	// Discard all pending SDL events
	while (SDL_PollEvent(&event));

	// Loop while we have to wait for a user click

	do {

		show_chat(context);
		blit_mouse_cursor();
		our_SDL_flip_wrapper();

		SDL_WaitEvent(&event);

		// Special events handling

		if (event.type == SDL_QUIT) {
			Terminate(EXIT_SUCCESS, TRUE);
		}

		if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
			case SDLK_SPACE:
			case SDLK_RETURN:
			case SDLK_ESCAPE:
				// Force to end waiting
				stop_wait_user_click(NULL);
				return;
			default:
				break;
			}
		}

		// Push the event to the chat screen
		//
		// Note: we know that the 'chat_wait' widget_group fills the screen, so we do
		// not need to check if the cursor is inside the widget.

		WIDGET(chat_wait)->handle_event(WIDGET(chat_wait), &event);

	} while (waiting_user_click());

	return;
}

void chat_add_response(const char *response)
{
	autostr_append(chat_log->text, "%s", response);
	chat_log->scroll_offset = 0;
}

static int create_and_stack_context(enemy *partner, struct npc *npc, const char *filename, int is_subdialog)
{
	char fpath[2048];
	char finaldir[50];
	struct chat_context *new_context = NULL;

	new_context = chat_create_context(partner, npc, is_subdialog);

	// Find the dialog file and load it inside the new chat context
	sprintf(finaldir, "%s", DIALOG_DIR);
	find_file(filename, finaldir, fpath, 0);
	load_dialog(fpath, new_context);

	// Push the new chat_context
	chat_push_context(new_context);

	return TRUE;
}

/**
 * Create and push a chat context to run a sub dialog.
 *
 * @param filename Subdialog filename.
 *
 * @return TRUE on success, FALSE on error.
 */
int stack_subdialog(const char *filename)
{
	// A subdialog uses the same partner and npc than its parent dialog.
	// The parent dialog is the one on top of the chat_context stack.
	struct chat_context *current_chat_context = current_chat_context = chat_get_current_context();

	return create_and_stack_context(current_chat_context->partner, current_chat_context->npc, filename, TRUE);
}

/**
 * Create and push a chat context to run a dialog.
 *
 * @param partner Enemy we are chatting with.
 *
 * @return TRUE on success, FALSE on error.
 */
int stack_dialog(enemy *partner)
{
	struct npc *npc = npc;
	char dialog_filename[5000];

	npc = npc_get(partner->dialog_section_name);
	if (!npc)
		return FALSE;

	strcpy(dialog_filename, partner->dialog_section_name);
	strcat(dialog_filename, ".dialog");

	return create_and_stack_context(partner, npc, dialog_filename, FALSE);
}

/**
 *
 * This function performs a menu for the player to select from, using the
 * keyboard or mouse wheel.
 *
 * The rest of the menu is made particularly unclear by the fact that there
 * can be some multi-line options too and also there is scrolling up and
 * down possible, when there are more menu options than fit onto one
 * dialog options selection window for the player to click from.
 *
 */
static void chat_do_menu_selection(struct chat_context *context)
{
	SDL_Event event;
	int old_game_status = game_status;
	struct widget *ui = WIDGET(chat_menu);

	game_status = INSIDE_MENU;

	// Discard all pending SDL events
	while (SDL_PollEvent(&event));

	// Loop until a dialog option is selected

	do {

		show_chat(context);
		blit_mouse_cursor();
		our_SDL_flip_wrapper();

		SDL_WaitEvent(&event);

		// Special events handling

		if (event.type == SDL_QUIT)
			Terminate(EXIT_SUCCESS, TRUE);

		if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
			if (GameConfig.enable_cheatkeys) {
				break;
			}
		}

		// Push the event to the chat screen

		WIDGET(ui)->handle_event(WIDGET(ui), &event);

	} while (context->current_option == -1);


	game_status = old_game_status;
}

/**
 * Process the chat option selected by the user.
 *
 * - Echo the option text of the chosen option, unless NO_TEXT is set.
 * - Run the associated LUA code.
 */
static void process_chat_option(struct chat_context *context)
{
	int current_selection = context->current_option;

	// Reset chat control variables for this option, so the user will have to
	// select a new option, unless the lua code ends the dialog or
	// auto-activates an other option.
	context->end_dialog = 0;
	context->current_option = -1;

	// Echo option text
	if (!context->dialog_options[current_selection].no_text) {
		chat_add_response("\1- ");
		chat_add_response(L_(context->dialog_options[current_selection].option_text));
		chat_add_response("\n");
	}

	// Run LUA associated with this selection
	if (context->dialog_options[current_selection].lua_code && strlen(context->dialog_options[current_selection].lua_code))
		context->lua_coroutine = load_lua_coroutine(LUA_DIALOG, context->dialog_options[current_selection].lua_code);
}

/**
 * Run the dialog defined by the chat contest on top of the stack.
 *
 * This is the most important subfunction of the whole chat with friendly
 * droids and characters.  After a chat context has been filled and pushed on
 * the stack, this function is invoked to handle the actual chat interaction
 * and the dialog flow.
 */
static void run_chat()
{
	struct chat_context *current_chat_context = NULL;

	Activate_Conservative_Frame_Computation();

	// Pump all remaining SDL events
	SDL_Event evt;
	while (SDL_PollEvent(&evt));

	// Dialog engine main code.
	//
	// A dialog flow is decomposed into a sequence of several steps:
	// 1 - run the initialization lua code (only once per game)
	// 2 - run the startup lua code (each time a dialog is launched)
	// 3 - interaction step: let the user chose a dialog option, and run the
	//     associated lua script
	// 4 - loop on the next interaction step, unless the dialog is ended.
	//
	// In some cases, a lua script needs a user's interaction (wait for a click
	// in lua npc_say(), for instance). We then have to interrupt the lua script,
	// run an interaction loop, and resume the lua script after the user's interaction.
	//
	// To implement a script interruption/resuming, we use the co-routine
	// yield/resume features of Lua. When a lua script is run, we thus have
	// to loop on its execution until it ends.
	//
	// For calls to a subdialog (lua run_subdialog()) or an other inner dialog
	// (lua start_dialog()), the current lua script also has to be interrupted,
	// the new dialog has to be extracted from the stack and run, and then the
	// previous dialog script has to be resumed.
	// at each loop step.
	//
	// All of these apparently intricate loops are implemented with one single
	// loop, taking care of the actual state of the chat, restarting the loop
	// when needed, and extracting the top chat context at each loop step :
	//
	// while (there is something to run)
	//   get chat context from top of stack
	//   if (a user interaction is waited)
	//     wait user interaction
	//   else if (a lua co-routine is started)
	//     resume the lua co-routine
	//   else
	//     execute dialog FSM step
	//   if (current dialog is ended)
	//     pop the chat context stack

	// Implement the former algorithm, but using 'continue' statements
	// instead of 'else' clauses.
	while ((current_chat_context = chat_get_current_context())) {

		/*
		 * 1- Chat screen rendering, and user's interaction
		 */

		fill_chat_selector(current_chat_context);
		show_chat(current_chat_context);

		if (current_chat_context->wait_user_click) {
			wait_for_click(current_chat_context);
			// restart the loop, to re-render the chat screen
			continue;
		}

		/*
		 * 2- Resume the current lua coroutine, if any, until it ends.
		 */

		if (current_chat_context->lua_coroutine) {
			int rtn = resume_lua_coroutine(current_chat_context->lua_coroutine);

			// If rtn is TRUE, the lua script reached its end, so we mark it
			// as such.
			if (rtn) {
				current_chat_context->lua_coroutine = NULL;
				// end dialog, if asked by the lua script
				if (current_chat_context->end_dialog)
					goto wait_click_and_out;
			}

			// restart the loop, to handle user interaction, if needed, and
			// resume the co-routine, if needed.
			continue;
		}

		/*
		 * 3- Execute current state of the chat FSM
		 */

		switch (current_chat_context->state) {
			case RUN_INIT_SCRIPT:
			{
				if (!current_chat_context->is_subdialog)
					widget_text_init(chat_log, _("\3--- Start of Dialog ---\n"));

				if (current_chat_context->initialization_code && strlen(current_chat_context->initialization_code)) {
					current_chat_context->lua_coroutine = load_lua_coroutine(LUA_DIALOG, current_chat_context->initialization_code);
				}
				current_chat_context->state = RUN_STARTUP_SCRIPT; // next step once the script ends
				break;
			}
			case RUN_STARTUP_SCRIPT:
			{
				if (!current_chat_context->is_subdialog && current_chat_context->npc->chat_character_initialized)
					widget_text_init(chat_log, _("\3--- Start of Dialog ---\n"));

				if (current_chat_context->startup_code && strlen(current_chat_context->startup_code)) {
					current_chat_context->lua_coroutine = load_lua_coroutine(LUA_DIALOG, current_chat_context->startup_code);
				}
				current_chat_context->state = SELECT_NEXT_NODE; // next step once the script ends
				break;
			}
			case SELECT_NEXT_NODE:
			{
				// If no dialog is currently selected, let the user choose one of the
				// active options.
				if (current_chat_context->current_option == -1) {
					chat_do_menu_selection(current_chat_context);
				}

				if ((current_chat_context->current_option >= MAX_DIALOGUE_OPTIONS_IN_ROSTER) || (current_chat_context->current_option < 0)) {
					DebugPrintf(0, "%s: Error: chat_control current_node %i out of range!\n", __FUNCTION__, current_chat_context->current_option);
					goto wait_click_and_out;
				}

				// Output the 'partner' message, and load the node script into
				// a lua coroutine.
				process_chat_option(current_chat_context);

				break;
			}
			default:
				break;
		}

		// restart the loop, to handle user interaction, if needed, resume a
		// lua co-routine, if needed, and stepping the FSM.
		continue;

wait_click_and_out:
		while (EscapePressed() || MouseLeftPressed() || SpacePressed());

		// The dialog has ended. Mark the npc as already initialized, to avoid
		// its initialization script to be run again the next time the dialog
		// is open, and pop the dialog from the chat context stack.
		if (!current_chat_context->npc->chat_character_initialized)
			current_chat_context->npc->chat_character_initialized = TRUE;

		chat_pop_context();
	}
}

/**
 * This is more or less the 'main' function of the chat with friendly 
 * droids and characters.  It is invoked directly from the user interface
 * function as soon as the player requests communication or there is a
 * friendly bot who rushes Tux and opens talk.
 */
void ChatWithFriendlyDroid(enemy * ChatDroid)
{
	if (!stack_dialog(ChatDroid))
		return;

	struct widget *ui = WIDGET(chat_menu);
	ui->enabled = TRUE;
	run_chat();
	ui->enabled = FALSE;
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

	/* Disable sound to speed up validation. */
	int old_sound_on = sound_on;
	sound_on = FALSE;

	find_file("levels.dat", MAP_DIR, fpath, 0);
	LoadShip(fpath, 0);
	PrepareStartOfNewCharacter("NewTuxStartGameSquare");

	/* Temporarily disable screen fadings to speed up validation. */
	GameConfig.do_fadings = FALSE;

	/* _says functions are not run by the validator, as they display
	   text on screen and wait for clicks */
	run_lua(LUA_DIALOG, "function npc_says(a)\nend\n");
	run_lua(LUA_DIALOG, "function tux_says(a)\nend\n");
	run_lua(LUA_DIALOG, "function cli_says(a)\nend\n");

	/* Subdialogs currently call run_chat and we cannot do that when validating dialogs */
	run_lua(LUA_DIALOG, "function run_subdialog(a)\nend\n");
	run_lua(LUA_DIALOG, "function start_chat(a)\nend\n");

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

	list_for_each_entry(n, &npc_head, node) {
		struct chat_context *dummy_context = chat_create_context(dummy_partner, n, FALSE);

		basename = n->dialog_basename;
		printf("Testing dialog \"%s\"...\n", basename);
		sprintf(filename, "%s.dialog", basename);
		find_file(filename, DIALOG_DIR, fpath, 0);
		load_dialog(fpath, dummy_context);

		chat_push_context(dummy_context);

		if (dummy_context->initialization_code && strlen(dummy_context->initialization_code)) {
			printf("\tinit code\n");
			run_lua(LUA_DIALOG, dummy_context->initialization_code);
		}

		if (dummy_context->startup_code && strlen(dummy_context->startup_code)) {
			printf("\tstartup code\n");
			run_lua(LUA_DIALOG, dummy_context->startup_code);
		}

		k = 0;
		for (j = 0; j < MAX_DIALOGUE_OPTIONS_IN_ROSTER; j++) {
			if (dummy_context->dialog_options[j].lua_code && strlen(dummy_context->dialog_options[j].lua_code)) {
				if (!(k%5)) {
					printf("\n");
				}
				printf("|node %2d|\t", j);
				run_lua(LUA_DIALOG, dummy_context->dialog_options[j].lua_code);
				k++;
			}
		}

		printf("\n... dialog OK\n\n");

		chat_pop_context();
	}

	/* Re-enable sound as needed. */
	sound_on = old_sound_on;

	/* Re-enable screen fadings. */
	GameConfig.do_fadings = TRUE;

	return error_caught;
}

#undef _chat_c
