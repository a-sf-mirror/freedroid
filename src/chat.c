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
#include "lua.h"

#include "widgets/widgets.h"

/* Stack of chat contexts. The current context is at the top of the stack */
static struct list_head chat_context_stack = LIST_HEAD_INIT(chat_context_stack);
static int chat_context_stack_size = 0;

static struct widget_group *chat_menu = NULL;
struct widget_text *chat_log = NULL;
struct widget_text_list *chat_selector = NULL;
static struct widget_group *chat_wait = NULL;

static struct image **droid_portrait = NULL;
static SDL_Rect chat_selector_inner_rect;

/**
 * Print an error message and stop the game if too many dialogs are stacked.
 */
static void check_chat_context_stack_size()
{
	struct auto_string *chat_stack_str = alloc_autostr(64);
	struct chat_context *ctx;
	int first = TRUE;

	if (chat_context_stack_size < CHAT_CONTEXT_STACK_SIZE)
		return;

	list_for_each_entry_reverse(ctx, &chat_context_stack, stack_node) {
		autostr_append(chat_stack_str, "%s%s", (first) ? "" : " -> ", ctx->npc->dialog_basename);
		first = FALSE;
	}

	ErrorMessage(__FUNCTION__, "The chat context stack reached its maximum (%d).\n"
			                   "It could mean that we are under an infinite recursion attack, so our last defense is to stop the game.\n"
			                   "Current dialog stack (from first to last stacked dialog):\n%s\n",
			                   PLEASE_INFORM, IS_FATAL, CHAT_CONTEXT_STACK_SIZE, chat_stack_str->value);
}

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
 *
 * @return TRUE if on success, else return FALSE
 */
int chat_push_context(struct chat_context *chat_context)
{
	check_chat_context_stack_size();
	list_add(&chat_context->stack_node, &chat_context_stack);
	chat_context_stack_size++;

	if (!call_lua_func(LUA_DIALOG, "FDdialog", "push_dialog", "sS", NULL, chat_context->npc->dialog_basename, &chat_context->npc->enabled_nodes)) {
		list_del(chat_context_stack.next);
		chat_context_stack_size--;
		return FALSE;
	}

	return TRUE;
}

/**
 * Pop from the chat contexts stack.
 *
 * This function deletes the context at the top of the list.
 * The new top context becomes the current one.
 */
static void chat_pop_context()
{
	struct chat_context *top_chat_context = chat_get_current_context();
	if (top_chat_context) {
		// clear the enabled_nodes dynarray, before to fill it with new values
		int i;
		for (i = 0; i < top_chat_context->npc->enabled_nodes.size; i++) {
			char **ptr = dynarray_member(&top_chat_context->npc->enabled_nodes, i, sizeof(char *));
			free(*ptr);
			*ptr = NULL;
		}
		dynarray_free(&top_chat_context->npc->enabled_nodes);

		// Tell lua dialog engine to pop the current dialog, and get the currently enabled
		// nodes (so that they will be saved along with the npc data)
		call_lua_func(LUA_DIALOG, "FDdialog", "pop_dialog", NULL, "S", &top_chat_context->npc->enabled_nodes);

		// remove the dialog from the chat context stack
		list_del(chat_context_stack.next);
		chat_delete_context(top_chat_context);
		chat_context_stack_size--;
	}
}

/**
 * Create a chat context and initialize it.
 *
 * @param partner     Pointer to the enemy to talk with
 * @param npc         Pointer to the npc containing the dialogue definition.
 * @param dialog_name Name of the dialog
 * @return Pointer to the allocated chat context.
 */
struct chat_context *chat_create_context(enemy *partner, struct npc *npc, const char *dialog_name)
{
	struct chat_context *new_chat_context = (struct chat_context *)MyMalloc(sizeof(struct chat_context));

	// Init chat control variables.
	// MyMalloc() already zeroed the struct, so we only initialize 'non zero' fields.

	new_chat_context->partner = partner;
	new_chat_context->npc = npc;
	new_chat_context->partner_started = partner->will_rush_tux;

	// The first time a dialog is open, the init script has to be run first.
	// The next times the dialog is open, directly run the startup script.
	new_chat_context->state = LOAD_INIT_SCRIPT;
	if (npc->chat_character_initialized) {
		new_chat_context->state = LOAD_STARTUP_SCRIPT;
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
void chat_delete_context(struct chat_context *chat_context)
{
	free(chat_context);
}

/**
 * Fill the chat selector widget with the currently selected options
 */
static void fill_chat_selector(struct chat_context *context)
{
	char *empty_entries[] = { NULL };
	int i;

	widget_text_list_init(chat_selector, empty_entries, NULL);

	// Call FDdialog.get_options() to get the list of option texts to display.
	// Note: call_lua_func() can not be used here, because extracting data from a
	// returned table is not yet implemented.

	lua_State *L = get_lua_state(LUA_DIALOG);

	lua_getglobal(L, "FDdialog");
	lua_getfield(L, -1, "get_options");
	lua_remove(L, -2);
	if (lua_pcall(L, 0, 1, 0)) {
		DebugPrintf(-1, "error while calling FDdialog.get_options(): %s", lua_tostring(L, -1));
		ErrorMessage(__FUNCTION__, "Aborting chat selector filling.\n", NO_NEED_TO_INFORM, IS_WARNING_ONLY);
		return;
	}

	if (!lua_istable(L, -1)) {
		DebugPrintf(-1, "function FDdialog.get_options() must return a table");
		ErrorMessage(__FUNCTION__, "Aborting chat selector filling.\n", NO_NEED_TO_INFORM, IS_WARNING_ONLY);
		return;
	}

	for (i = 1; i <= lua_rawlen(L, -1);) {
		lua_rawgeti(L, -1, i);
		int index = lua_tointeger(L, -1);
		lua_pop(L, 1);
		i++;
		lua_rawgeti(L, -1, i);
		const char *text = lua_tostring(L, -1);
		widget_text_list_dupadd(chat_selector, text, index);
		lua_pop(L, 1);
		i++;
	}

	lua_pop(L, 1);
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
static int waiting_user_interaction(struct chat_context *context)
{
	int waiting = FALSE;

	waiting = context->wait_user_click ||
	          ((context->state == SELECT_NEXT_NODE) && (context->current_option == -1));

	return waiting;
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
	int left_padding, right_padding, top_padding, bottom_padding;
	int font_height;
	int nb_lines;
	int inner_height, actual_inner_height;

	if (chat_menu)
		return chat_menu;

	chat_menu = widget_group_create();

	//
	// Dark background
	//

	struct widget *dark_background = widget_create();
	widget_set_rect(dark_background, 0, 0, GameConfig.screen_width, GameConfig.screen_height);
	dark_background->display = display_dark_background;
	widget_group_add(chat_menu, dark_background);

	//
	// Entry text selector
	//

	int chat_selector_h = UNIVERSAL_COORD_H(170);
	int chat_selector_w = UNIVERSAL_COORD_W(640);
	int chat_selector_x = (GameConfig.screen_width - chat_selector_w) / 2;
	int chat_selector_y = GameConfig.screen_height - chat_selector_h;

	/* Default padding for text area */
	left_padding = 35;
	right_padding = 35;
	top_padding = 24;
	bottom_padding = 24;
	inner_height = chat_selector_h - top_padding - bottom_padding;

	struct widget_background *chat_selector_bkg = widget_background_create();
	widget_set_rect(WIDGET(chat_selector_bkg), chat_selector_x, chat_selector_y, chat_selector_w, chat_selector_h);
	widget_background_load_3x3_tiles(chat_selector_bkg, "widgets/chat_typo");
	widget_group_add(chat_menu, WIDGET(chat_selector_bkg));

	chat_selector = widget_text_list_create();

	/* Adjust padding of the text area, to adapt the vertically padding to the available height */
	font_height = FontHeight(chat_selector->font);
	nb_lines = floor((float)inner_height / (float)font_height);
	actual_inner_height = nb_lines * font_height;
	top_padding += (inner_height - actual_inner_height) / 2;
	bottom_padding = chat_selector_h - actual_inner_height - top_padding;

	chat_selector_inner_rect.x = chat_selector_x + left_padding;
	chat_selector_inner_rect.y = chat_selector_y + top_padding;
	chat_selector_inner_rect.h = chat_selector_h - top_padding - bottom_padding;
	chat_selector_inner_rect.w = chat_selector_w - left_padding - right_padding;

	widget_set_rect(WIDGET(chat_selector), chat_selector_inner_rect.x, chat_selector_inner_rect.y, chat_selector_inner_rect.w, chat_selector_inner_rect.h);
	chat_selector->process_entry = dialog_option_selected;
	WIDGET(chat_selector)->update = WIDGET_UPDATE_FLAG_ON_DATA(WIDGET, enabled, !chat_get_current_context()->wait_user_click);
	widget_group_add(chat_menu, WIDGET(chat_selector));

	//
	// Droid portrait
	//

	struct image *img_frame = widget_load_image_resource("widgets/chat_portrait_frame.png", 0);
	struct image *img_connector = widget_load_image_resource("widgets/chat_portrait_connector.png", 0);
	struct image *img_tube = widget_load_image_resource("widgets/chat_portrait_tube.png", 0);
	int chat_portrait_frame_x = 13;
	int chat_portrait_frame_y = 4;
	int chat_portrait_connector_x = chat_selector_x;
	int chat_portrait_connector_y = chat_selector_y - img_connector->h;
	int chat_portrait_tube_x = chat_portrait_connector_x + 86;
	int chat_portrait_tube_y = chat_portrait_frame_y + img_frame->h;
	int chat_portrait_tube_h = (chat_portrait_connector_y + 39) - chat_portrait_tube_y;

	struct widget_background *chat_portrait_connector = widget_background_create();
	widget_background_add(chat_portrait_connector, img_connector, chat_portrait_connector_x, chat_portrait_connector_y, img_connector->w, img_connector->h, 0);
	widget_group_add(chat_menu, WIDGET(chat_portrait_connector));

	if (chat_portrait_tube_h > 0) {
		int tube_repeat = ceil((float)chat_portrait_tube_h / (float)img_tube->h);
		chat_portrait_tube_h = img_tube->h * tube_repeat;
		chat_portrait_tube_y = (chat_portrait_connector_y + 39) - chat_portrait_tube_h;
		struct widget_background *chat_portrait_tube = widget_background_create();
		widget_background_add(chat_portrait_tube, img_tube, chat_portrait_tube_x, chat_portrait_tube_y, img_tube->w, chat_portrait_tube_h, REPEATED);
		widget_group_add(chat_menu, WIDGET(chat_portrait_tube));
	}

	struct widget_background *chat_portrait_frame = widget_background_create();
	widget_background_add(chat_portrait_frame, img_frame, chat_portrait_frame_x, chat_portrait_frame_y, img_frame->w, img_frame->h, 0);
	widget_group_add(chat_menu, WIDGET(chat_portrait_frame));

	struct widget_button *chat_portrait = widget_button_create();
	widget_set_rect(WIDGET(chat_portrait), chat_portrait_frame_x + 30, chat_portrait_frame_y + 32, 136, 183);
	widget_group_add(chat_menu, WIDGET(chat_portrait));
	droid_portrait = &chat_portrait->image[0][0];

	//
	// Chat log
	//

	int chat_log_x = chat_portrait_connector_x + img_connector->w;
	int chat_log_y = 0;
	int chat_log_h = chat_selector_y - chat_log_y;
	int chat_log_w = chat_selector_x + chat_selector_w - chat_log_x;

	/* Default padding of the text area */
	left_padding = 50;
	right_padding = 30;
	top_padding = 24;
	bottom_padding = 48;
	inner_height = chat_log_h - top_padding - bottom_padding;

	struct widget_background *chat_log_bkg = widget_background_create();
	widget_set_rect(WIDGET(chat_log_bkg), chat_log_x, chat_log_y, chat_log_w, chat_log_h);
	widget_background_load_3x3_tiles(chat_log_bkg, "widgets/chat_log");
	widget_group_add(chat_menu, WIDGET(chat_log_bkg));

	chat_log = widget_text_create();
	chat_log->font = FPS_Display_BFont;
	chat_log->line_height_factor = LINE_HEIGHT_FACTOR;

	/* Adjust padding of the text area, to adapt the vertically padding to the available height */
	font_height = FontHeight(chat_log->font);
	nb_lines = floor((float)inner_height / (float)font_height);
	actual_inner_height = nb_lines * font_height;
	top_padding += (inner_height - actual_inner_height) / 2;
	bottom_padding = chat_log_h - actual_inner_height - top_padding;

	int chat_log_inner_rect_x = chat_log_x + left_padding;
	int chat_log_inner_rect_y = chat_log_y + top_padding;
	int chat_log_inner_rect_h = chat_log_h - top_padding - bottom_padding;
	int chat_log_inner_rect_w = chat_log_w - left_padding - right_padding;

	widget_set_rect(WIDGET(chat_log), chat_log_inner_rect_x, chat_log_inner_rect_y, chat_log_inner_rect_w, chat_log_inner_rect_h);
	widget_group_add(chat_menu, WIDGET(chat_log));

	struct {
		char *image[3];
		SDL_Rect rect;
		void (*activate_button)(struct widget_button *);
		void (*update)(struct widget *);
	} b[] = {
		// Chat Log scroll up
		{
			{"widgets/chat_scroll_up_off.png", NULL, "widgets/chat_scroll_up.png"},
			{chat_log_x + (chat_log_w - 158) / 2, chat_log_y + 2, 158, 22},
			WIDGET_EXECUTE(struct widget_button *wb, widget_text_scroll_up(chat_log)),
			WIDGET_UPDATE_FLAG_ON_DATA(WIDGET_BUTTON, active, widget_text_can_scroll_up(chat_log))
		},
		// Chat Log scroll down
		{
			{"widgets/chat_scroll_down_off.png", NULL, "widgets/chat_scroll_down.png"},
			{chat_log_x + (chat_log_w - 158) / 2, chat_log_y + chat_log_h - 41, 158, 22},
			WIDGET_EXECUTE(struct widget_button *wb, widget_text_scroll_down(chat_log)),
			WIDGET_UPDATE_FLAG_ON_DATA(WIDGET_BUTTON, active, widget_text_can_scroll_down(chat_log))
		},
		// Chat Selector Scroll up
		{
			{"widgets/chat_scroll_up_off.png", NULL, "widgets/chat_scroll_up.png"},
			{chat_selector_x + (chat_selector_w - 158) / 2, chat_selector_y - 2, 158, 22},
			WIDGET_EXECUTE(struct widget_button *wb, widget_text_list_scroll_up(chat_selector)),
			WIDGET_UPDATE_FLAG_ON_DATA(WIDGET_BUTTON, active, widget_text_list_can_scroll_up(chat_selector))
		},
		// Chat Selector Scroll down
		{
			{"widgets/chat_scroll_down_off.png", NULL, "widgets/chat_scroll_down.png"},
			{chat_selector_x + (chat_selector_w - 158) / 2, chat_selector_y + chat_selector_h - 23, 158, 22},
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

	//
	// Widget used when waiting for user click
	//

	chat_wait = widget_group_create();
	widget_set_rect(WIDGET(chat_wait), 0, 0, GameConfig.screen_width, GameConfig.screen_height);
	WIDGET(chat_wait)->enabled = FALSE;
	WIDGET(chat_wait)->update =  WIDGET_UPDATE_FLAG_ON_DATA(WIDGET, enabled, chat_get_current_context()->wait_user_click);

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

void chat_add_response(const char *response)
{
	autostr_append(chat_log->text, "%s", response);
	chat_log->scroll_offset = 0;
}

/**
 * Run the dialog defined by the chat contest on top of the stack.
 *
 * This is the most important subfunction of the whole chat with friendly
 * droids and characters.  After a chat context has been filled and pushed on
 * the stack, this function is invoked to handle the actual chat interaction
 * and the dialog flow.
 */
void chat_run()
{
	 char *empty_entries[] = { NULL };

	struct chat_context *current_chat_context = NULL;
	struct widget *chat_ui = WIDGET(chat_menu);

	Activate_Conservative_Frame_Computation();

	// Pump all remaining SDL events
	SDL_Event event;
	while (SDL_PollEvent(&event));

	// Activate chat screen
	chat_ui->enabled = TRUE;

	// Init some widgets
	widget_text_init(chat_log, "");
	widget_text_list_init(chat_selector, empty_entries, NULL);

	// Dialog engine main code.
	//
	// A dialog flow is decomposed into a sequence of several steps:
	// 1 - run the initialization lua code (only once per game)
	// 2 - run the startup lua code (each time a dialog is launched)
	// 3 - interaction step: let the user chose a dialog option (i.e. a dialog
	//     node) and run the associated lua script
	// 4 - loop on the next interaction step, unless the dialog is ended.
	//
	// In some cases, a lua script needs a user's interaction (wait for a click
	// in lua npc_say(), for instance). We then have to interrupt that lua
	// script, run an interaction loop, and resume the lua script after the
	// user's interaction.
	//
	// To implement a script interruption/resuming, we use the co-routine
	// yield/resume features of Lua. When a lua script is run, we thus have to
	// loop on its execution (i.e. resume it) until it ends.
	//
	// For calls to open a dialog inside a dialog (lua start_dialog()), the
	// current lua script also has to be interrupted, the context of the new
	// dialog has to be extracted from the stack to run the new dialog flow, and
	// once the new dialog is ended the previous dialog script has to be resumed.
	//
	// All of these apparently intricate loops are implemented with one single
	// loop, taking care of the actual state of the chat, restarting the loop
	// when needed, and extracting the top chat context at each loop step :
	//
	// while (there is something to run)
	//   get chat context from top of stack
	//   if (a user interaction is waited)
	//     check user interaction
	//   if (a user interaction is no more waited) /* do not use an 'else' here ! */
	//     execute dialog FSM step
	//   if (current dialog is ended)
	//     pop the chat context stack
	//
	// TODO: document the relation between the C part and the Lua part of the
	// dialog engine.

	while ((current_chat_context = chat_get_current_context())) {

		/*
		 * 1- Chat screen rendering
		 */

		*droid_portrait = get_droid_portrait_image(current_chat_context->partner->type);
		clear_screen();
		chat_ui->update_tree(chat_ui);

		chat_ui->display(chat_ui);
		blit_mouse_cursor();
		our_SDL_flip_wrapper();

		/*
		 * 2- User's interaction
		 */

		if (waiting_user_interaction(current_chat_context)) {

			SDL_WaitEvent(&event);

			// Specific events handling
			if (event.type == SDL_QUIT) {
				Terminate(EXIT_SUCCESS, TRUE);
			}
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
				if (GameConfig.enable_cheatkeys)
					goto end_current_dialog;
			}
			if (current_chat_context->wait_user_click && event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_SPACE:
				case SDLK_RETURN:
					// Force to end waiting
					stop_wait_user_click(NULL);
					break;
				default:
					break;
				}
			}

			// Push the event to the chat screen.
			// Note: we know that the 'chat_menu' widget_group fills the
			// screen, so we do not need to check if the cursor is inside the widget.
			chat_ui->handle_event(chat_ui, &event);
		}

		/*
		 * 3- Execute current state of the chat FSM
		 * (Note: do not replace the test with an 'else')
		 */
		if (!waiting_user_interaction(current_chat_context)) {

			switch (current_chat_context->state) {
				case LOAD_INIT_SCRIPT:
				{
					widget_text_init(chat_log, _("[n]--- Start of Dialog ---\n"));
					current_chat_context->script_coroutine = prepare_lua_coroutine(LUA_DIALOG, "FDdialog", "run_init", NULL);
					// next step
					current_chat_context->state = RUN_INIT_SCRIPT;
					break;
				}
				case LOAD_STARTUP_SCRIPT:
				{
					if (current_chat_context->npc->chat_character_initialized)
						widget_text_init(chat_log, _("[n]--- Start of Dialog ---\n"));
					current_chat_context->script_coroutine = prepare_lua_coroutine(LUA_DIALOG, "FDdialog", "run_startup", NULL);
					// next step
					current_chat_context->state = RUN_STARTUP_SCRIPT;
					break;
				}
				case SELECT_NEXT_NODE:
				{
					// Output the 'partner' message, and load the node script into
					// a lua coroutine.
					// Also, reset chat control variables, so the user will have to
					// select a new option, unless the lua code ends the dialog
					// or auto-activates an other option.
					current_chat_context->script_coroutine = prepare_lua_coroutine(LUA_DIALOG, "FDdialog", "run_node", "d", current_chat_context->current_option);
					current_chat_context->end_dialog = 0;
					current_chat_context->current_option = -1;
					// next step
					current_chat_context->state = RUN_NODE_SCRIPT;
					break;
				}
				case RUN_INIT_SCRIPT:
				case RUN_STARTUP_SCRIPT:
				case RUN_NODE_SCRIPT:
				{
					if (current_chat_context->script_coroutine) {
						if (resume_lua_coroutine(current_chat_context->script_coroutine)) {
							// the lua script reached its end, remove its reference
							lua_State *L = get_lua_state(LUA_DIALOG);
							lua_pop(L, 1);
							free(current_chat_context->script_coroutine);
							current_chat_context->script_coroutine = NULL;
							// end dialog, if asked by the lua script
							if (current_chat_context->end_dialog)
								goto end_current_dialog;
						}
					}

					// next step to run once the script has ended
					// (note: do not replace the test with an 'else')
					if (!current_chat_context->script_coroutine) {
						if (current_chat_context->state == RUN_INIT_SCRIPT)
							current_chat_context->state = LOAD_STARTUP_SCRIPT;
						else
							current_chat_context->state = SELECT_NEXT_NODE;
					}

					// The script can have changed the list of active dialog options,
					// so we have to refresh the chat_selector content.
					fill_chat_selector(current_chat_context);
					break;
				}
				default:
					break;
			}

		}

		// restart the loop, to handle user interaction or stepping the FSM.
		continue;

end_current_dialog:
		// Pump all remaining SDL events
		while (SDL_PollEvent(&event));

		// The dialog has ended. Mark the npc as already initialized, to avoid
		// its initialization script to be run again the next time the dialog
		// is open, and pop the dialog from the chat context stack.
		current_chat_context->npc->chat_character_initialized = TRUE;
		chat_pop_context();
		// Tux's option entries have been freed by chat_pop_context()
		// so we have to clean-up the widget using it
		widget_text_list_init(chat_selector, empty_entries, NULL);
	}

	// Close chat screen
	chat_ui->enabled = FALSE;
}

/**
 * This is more or less the 'main' function of the chat with friendly
 * droids and characters.  It is invoked directly from the user interface
 * function as soon as the player requests communication or there is a
 * friendly bot who rushes Tux and opens talk.
 */
int chat_with_droid(struct enemy *partner)
{
	struct npc *npc;
	struct chat_context *chat_context;
	char *dialog_name;

	// Create a chat context.
	// Use the partner's dialog name attribute to get the related npc
	// and the dialog to load.
	npc = npc_get(partner->dialog_section_name);
	if (!npc)
		return FALSE;
	dialog_name = partner->dialog_section_name;

	chat_context = chat_create_context(partner, npc, dialog_name);

	// Push the chat context on the stack.
	// The dialog will be run on the next loop of the chat engine.
	if (!chat_push_context(chat_context)) {
		chat_delete_context(chat_context);
		return FALSE;
	}

	// Open the chat screen and run the chat engine.
	chat_run();

	return TRUE;
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
	char fpath[PATH_MAX];
	enemy *dummy_partner;
	struct npc *n;
	int error_caught = FALSE;

	skip_initial_menus = 1;

	/* Disable sound to speed up validation. */
	int old_sound_on = sound_on;
	sound_on = FALSE;

	find_file("levels.dat", MAP_DIR, fpath);
	LoadShip(fpath, 0);
	PrepareStartOfNewCharacter("NewTuxStartGameSquare");

	/* Temporarily disable screen fadings to speed up validation. */
	GameConfig.do_fadings = FALSE;

	/* _says functions are not run by the validator, as they display
	   text on screen and wait for clicks */
	run_lua(LUA_DIALOG, "function chat_says(a)\nend\n");
	run_lua(LUA_DIALOG, "function npc_says(a)\nend\n");
	run_lua(LUA_DIALOG, "function tux_says(a)\nend\n");
	run_lua(LUA_DIALOG, "function cli_says(a)\nend\n");

	/* Subdialogs currently call run_chat and we cannot do that when validating dialogs */
	run_lua(LUA_DIALOG, "function call_subdialog(a)\nend\n");
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

	/* set_mouse_move_target() breaks validator */
	run_lua(LUA_DIALOG, "function set_mouse_move_target(a)\nend\n");

	/* win_game() causes silly animations and delays the process. */
	run_lua(LUA_DIALOG, "function win_game(a)\nend\n");

	/* This dummy is needed for the Lua functions that communicates with a npc */
	BROWSE_ALIVE_BOTS(dummy_partner) {
		break;
	}

	list_for_each_entry(n, &npc_head, node) {
		printf("Testing dialog \"%s\"...\n", n->dialog_basename);

		struct chat_context *dummy_context = chat_create_context(dummy_partner, n, n->dialog_basename);

		// We want the dialog validator to catch all errors. It thus has to call
		// push_dialog() itself. As a consequence, so we can not use chat_push_context().
		check_chat_context_stack_size();
		list_add(&dummy_context->stack_node, &chat_context_stack);
		chat_context_stack_size++;

		int rtn;
		if (call_lua_func(LUA_DIALOG, "FDdialog", "validate_dialog", "s", "d", n->dialog_basename, &rtn)) {
			if (!rtn) {
				error_caught = TRUE;
			}
			if (term_has_color_cap)
				printf("Result: %s\n", rtn ? "\033[32msuccess\033[0m" : "\033[31mfailed\033[0m");
			else
				printf("Result: %s\n", rtn ? "success" : "failed");
		} else {
			error_caught = TRUE;
		}

		// Remove the dialog from the chat context stack
		list_del(chat_context_stack.next);
		chat_delete_context(dummy_context);
		chat_context_stack_size--;

		printf("\n");
	}

	/* Re-enable sound as needed. */
	sound_on = old_sound_on;

	/* Re-enable screen fadings. */
	GameConfig.do_fadings = TRUE;

	return error_caught;
}

#undef _chat_c
