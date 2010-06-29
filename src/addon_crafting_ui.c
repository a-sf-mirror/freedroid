/* 
 *
 *   Copyright (c) 2010 Ari Mustonen
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
#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

/**
 * \file addon_crafting_ui.c
 *
 * \brief The add-on crafting user interface.
 */

struct crafting_recipe {
	int available; /// TRUE if the player has enough materials to craft the add-on.
	char *name; /// The name of the add-on this recipe will craft.
};

static struct {
	int visible;
	int quit;
	struct dynarray recipes;
} ui = { .visible = FALSE };

static const struct {
	SDL_Rect main;
	SDL_Rect title_text;
	SDL_Rect details_text;
	SDL_Rect recipe_list;
} rects = {
	{ ADDON_CRAFTING_RECT_X, ADDON_CRAFTING_RECT_Y, ADDON_CRAFTING_RECT_W, ADDON_CRAFTING_RECT_H },
	{ ADDON_CRAFTING_RECT_X + 20, ADDON_CRAFTING_RECT_Y + 12, 280, 38 },
	{ ADDON_CRAFTING_RECT_X + 25, ADDON_CRAFTING_RECT_Y + 260, 130, 20 },
	{ ADDON_CRAFTING_RECT_X + 20, ADDON_CRAFTING_RECT_Y + 64, 276, 240 },
};

static void clear_recipe_list()
{
	int i;
	struct crafting_recipe *arr = ui.recipes.arr;

	for (i = 0; i < ui.recipes.size; i++) {
		free(arr[i].name);
	}
	dynarray_free(&ui.recipes);
}

/**
 * \brief Builds the list of add-on recipes.
 *
 * Finds add-ons that can be crafted using the UI and adds them to the recipe
 * list. For each added recipe, a test is also performed to see if the player
 * has enough materials to craft the add-on.
 */
static void build_recipe_list()
{
	dynarray_init(&ui.recipes, 16, sizeof(struct crafting_recipe));

	// TODO: Get recipes from the add-on spec array.
	struct crafting_recipe recipe;
	recipe.available = TRUE;
	recipe.name = strdup("Linarian power crank");
	dynarray_add(&ui.recipes, &recipe, sizeof(struct crafting_recipe));
}

/**
 * \brief Draws the add-on crafting user interface.
 *
 * The upper half of the UI consists of a simple list of craftable items, from
 * which the user can select the add-on to be crafted. The lower half contains a
 * text field that shows the description, bonuses, and other relevant information
 * about the selected add-on.
 */
void show_addon_crafting_ui()
{
	int i;
	SDL_Rect rect;
	SDL_Surface *surface;
	struct crafting_recipe *arr = ui.recipes.arr;

	// We're being called every time the UI of the game is drawn so we need to
	// make sure the crafting UI is actually active before proceeding.
	if (!ui.visible) {
		return;
	}

	// Draw the background image.
	blit_special_background(ADDON_CRAFTING_BACKGROUND_CODE);

	// Draw the title.
	SetCurrentFont(Menu_BFont);
	DisplayText(_("Craft Addons"), rects.title_text.x, rects.title_text.y, NULL, TEXT_STRETCH);

	// Draw the details string.
	SetCurrentFont(Blue_BFont);
	DisplayText(_("Details"), rects.details_text.x, rects.details_text.y, NULL, TEXT_STRETCH);

	// Draw the apply and close buttons.
	ShowGenericButtonFromList(ITEM_UPGRADE_APPLY_BUTTON_DISABLED);
	ShowGenericButtonFromList(ITEM_UPGRADE_CLOSE_BUTTON);

	// Draw the icons and names of the recipes.
	rect.x = rects.recipe_list.x;
	rect.y = rects.recipe_list.y;
	rect.w = rects.recipe_list.w;
	rect.h = 32;
	for (i = 0; i < ui.recipes.size; i++) {
		if (arr[i].available) {
			SetCurrentFont(Blue_BFont);
		} else {
			SetCurrentFont(Red_BFont);
		}
		DisplayText(arr[i].name, rect.x + rect.h, rect.y + 4, NULL, TEXT_STRETCH);
		surface = ItemMap[GetItemIndexByName(arr[i].name)].inv_image.Surface;
		if (surface) {
			our_SDL_blit_surface_wrapper(surface, NULL, Screen, &rect);
		}
		rect.y += rect.h;
	}
}

static void handle_ui()
{
	point cursor;

	// Check for quit with the escape key.
	if (EscapePressed()) {
		ui.quit = TRUE;
		return;
	}

	// Check if we need to do anything.
	if (!MouseLeftClicked()) {
		return;
	}

	// Get the position of the cursor.
	cursor.x = GetMousePos_x();
	cursor.y = GetMousePos_y();

	// Handle clicks to the close button.
	if (MouseCursorIsOnButton(ITEM_UPGRADE_CLOSE_BUTTON, cursor.x, cursor.y)) {
		ui.quit = TRUE;
		return;
	}
}

/**
 * \brief Shows the add-on crafting user interface.
 */
void addon_crafting_ui()
{
	int old_game_status = game_status;
	game_status = INSIDE_MENU;

	// Clear the struct and build the recipe list.
	memset(&ui, 0, sizeof(ui));
	build_recipe_list();

	// Setup the correct screen state.
	make_sure_system_mouse_cursor_is_turned_off();
	GameConfig.Inventory_Visible = FALSE;
	GameConfig.CharacterScreen_Visible = FALSE;
	GameConfig.SkillScreen_Visible = FALSE;
	GameConfig.skill_explanation_screen_visible = FALSE;
	ui.visible = TRUE;

	// Loop until the player clicks the close button of the UI or presses and
	// releases escape. We need to ensure that the escape key is released so
	// that it doesn't interfere with the dialog that opened the crafting UI.
	while (!ui.quit || EscapePressed()) {
		StartTakingTimeForFPSCalculation();

		// Handle input.
		save_mouse_state();
		input_handle();
		handle_ui();

		// Draw the UI. AssembleCombatPicture will take care of calling our
		// drawing function in the right place.
		AssembleCombatPicture(DO_SCREEN_UPDATE | SHOW_ITEMS | USE_OWN_MOUSE_CURSOR);

		limit_fps();
		ComputeFPSForThisFrame();
	}

	// Free the description and the recipe list.
	clear_recipe_list();

	ui.visible = FALSE;
	game_status = old_game_status;
}
