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

#define RECIPE_LIST_ROWS 4
#define RECIPE_LIST_ROW_HEIGHT 45

struct crafting_recipe {
	int available; /// TRUE if the player has enough materials to craft the add-on.
	int item_type; /// The type number of the add-on this recipe will craft.
};

static struct {
	int visible;
	int quit;
	int selection;
	int scroll_offset; /// The scroll offset of the recipe list, in full rows.
	struct dynarray recipes;
	struct text_widget description;
} ui = { .visible = FALSE };

static const struct {
	SDL_Rect main;
	SDL_Rect title_text;
	SDL_Rect details_text;
	SDL_Rect recipe_list;
	SDL_Rect recipe_desc;
} rects = {
	{ ADDON_CRAFTING_RECT_X, ADDON_CRAFTING_RECT_Y, ADDON_CRAFTING_RECT_W, ADDON_CRAFTING_RECT_H },
	{ ADDON_CRAFTING_RECT_X + 20, ADDON_CRAFTING_RECT_Y + 12, 280, 38 },
	{ ADDON_CRAFTING_RECT_X + 25, ADDON_CRAFTING_RECT_Y + 260, 130, 20 },
	{ ADDON_CRAFTING_RECT_X + 20, ADDON_CRAFTING_RECT_Y + 62, 276, 180 },
	{ ADDON_CRAFTING_RECT_X + 22, ADDON_CRAFTING_RECT_Y + 290, 220, 140 }
};

/**
 * \brief Selects a recipe from the list.
 *
 * Selects the recipe with the requested index from the recipe list and updates
 * the description text to show information on the recipe.
 * \param index Index in the recipe list.
 */
static void select_recipe(int index)
{
	int i;
	int type;
	struct crafting_recipe *arr = ui.recipes.arr;
	struct addon_spec *spec = get_addon_spec(arr[index].item_type);
	struct addon_material *materials = spec->materials.arr;

	ui.selection = index;

	// Copy the item description to the text widget.
	struct auto_string *desc = ui.description.text;
	type = arr[index].item_type;
	autostr_printf(desc, "%s", ItemMap[type].item_description);

	// Append the add-on tooltip to the text widget.
	autostr_append(desc, "\n\n%s%s%s\n", font_switchto_msgstat, _("Features:"), font_switchto_msgvar);
	print_addon_description(spec, desc);

	// Append material requirements to the text widget.
	autostr_append(desc, "\n%s%s%s\n", font_switchto_msgstat, _("Materials:"), font_switchto_msgvar);
	for (i = 0; i < spec->materials.size; i++) {
		autostr_append(desc, _("%s: %d\n"), materials[i].name, materials[i].value);
	}

	// Scroll the text widget to the top.
	ui.description.scroll_offset = -get_lines_needed(desc->value,
	            ui.description.rect, ui.description.text_stretch);
}

/**
 * \brief Checks which recipes the player can afford.
 */
static void check_recipe_requirements()
{
	int i;
	int j;
	struct crafting_recipe *recipes = ui.recipes.arr;

	// Disable recipes the player can't afford.
	for (i = 0; i < ui.recipes.size; i++) {
		struct addon_spec *spec = get_addon_spec(recipes[i].item_type);
		struct addon_material *materials = spec->materials.arr;
		recipes[i].available = TRUE;

		// Check if the player has enough materials.
		for (j = 0; j < spec->materials.size; j++) {
			int type = GetItemIndexByName(materials[j].name);
			int count = CountItemtypeInInventory(type);
			if (count < materials[j].value) {
				recipes[i].available = FALSE;
				break;
			}
		}
	}
}

static void craft_item()
{
	item it;
	struct crafting_recipe *arr = ui.recipes.arr;

	// Craft the selected item if the player can afford it.
	if (arr[ui.selection].available) {
		int i;
		struct addon_spec *spec = get_addon_spec(arr[ui.selection].item_type);
		struct addon_material *materials = spec->materials.arr;

		// Subtract materials.
		for (i = 0; i < spec->materials.size; i++) {
			int type = GetItemIndexByName(materials[i].name);
			int count = materials[i].value;
			DeleteInventoryItemsOfType(type, count);
		}

		// Create the item and add it to the inventory.
		int type = arr[ui.selection].item_type;
		it = create_item_with_name(ItemMap[type].item_name, TRUE, 1);
		give_item(&it);

		// The player lost some materials so some of the recipes might have become
		// unaffordable. We need to recheck for the requirements because of this.
		check_recipe_requirements();
	}
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
	int i;
	struct dynarray *specs = get_addon_specs();
	struct addon_spec *arr = specs->arr;

	// Read recipes from the add-on spec array.
	dynarray_init(&ui.recipes, 16, sizeof(struct crafting_recipe));
	for (i = 0; i < specs->size; i++) {
		struct crafting_recipe recipe;
		recipe.available = TRUE;
		recipe.item_type = arr[i].type;
		dynarray_add(&ui.recipes, &recipe, sizeof(struct crafting_recipe));
	}

	// Check which recipes the player can afford.
	check_recipe_requirements();
}

/**
 * \brief Returns TRUE if the crafting UI is visible and the cursor is inside its rectangle.
 * \param cursor Cursor position.
 * \return TRUE if the cursor is on the UI, FALSE if not.
 */
int cursor_is_on_addon_crafting_ui(const point *cursor)
{
	if (ui.visible && MouseCursorIsInRect(&rects.main, cursor->x, cursor->y)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

static int can_scroll_up()
{
	return ui.scroll_offset > 0;
}

static int can_scroll_down()
{
	return ui.scroll_offset < ui.recipes.size - RECIPE_LIST_ROWS;
}

static void draw_scroll_desc_up_button()
{
	ShowGenericButtonFromList(ADDON_CRAFTING_SCROLL_DESC_UP_BUTTON);
}

static void draw_scroll_desc_down_button()
{
	ShowGenericButtonFromList(ADDON_CRAFTING_SCROLL_DESC_DOWN_BUTTON);
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
	if (arr[ui.selection].available) {
		ShowGenericButtonFromList(ITEM_UPGRADE_APPLY_BUTTON);
	} else {
		ShowGenericButtonFromList(ITEM_UPGRADE_APPLY_BUTTON_DISABLED);
	}
	ShowGenericButtonFromList(ITEM_UPGRADE_CLOSE_BUTTON);

	// Draw the icons and names of the recipes.
	rect.x = rects.recipe_list.x;
	rect.y = rects.recipe_list.y;
	rect.w = rects.recipe_list.w;
	rect.h = RECIPE_LIST_ROW_HEIGHT;
	int max_row = min(ui.recipes.size, ui.scroll_offset + RECIPE_LIST_ROWS);
	for (i = ui.scroll_offset; i < max_row; i++) {
		if (i == ui.selection) {
			HighlightRectangle(Screen, rect);
		}
		if (arr[i].available) {
			SetCurrentFont(Blue_BFont);
		} else {
			SetCurrentFont(Red_BFont);
		}
		int type = arr[i].item_type;
		DisplayText(ItemMap[type].item_name, rect.x + rect.h, rect.y + 4, NULL, TEXT_STRETCH);
		iso_image *img = get_item_inventory_image(type);
		if (img) {
			SDL_Rect icon_rect;
			icon_rect.w = RECIPE_LIST_ROW_HEIGHT;
			icon_rect.h = RECIPE_LIST_ROW_HEIGHT;
			icon_rect.x = rect.x + (icon_rect.w - img->original_image_width) / 2;
			icon_rect.y = rect.y + (icon_rect.h - img->original_image_height) / 2;
			blit_iso_image_to_screen_position(img, icon_rect.x, icon_rect.y);
		}
		rect.y += rect.h;
	}

	// Draw the scroll buttons.
	if (can_scroll_up()) {
		ShowGenericButtonFromList(ADDON_CRAFTING_SCROLL_UP_BUTTON);
	}
	if (can_scroll_down()) {
		ShowGenericButtonFromList(ADDON_CRAFTING_SCROLL_DOWN_BUTTON);
	}

	// Draw the description of the selected recipe.
	show_text_widget(&ui.description);
}

static void handle_ui()
{
	point cursor;

	// Check for quit with the escape key.
	if (EscapePressed()) {
		ui.quit = TRUE;
		return;
	}

	// Get the position of the cursor.
	cursor.x = GetMousePos_x();
	cursor.y = GetMousePos_y();

	// Handle scrolling and selections of the recipe list. The list can be scrolled
	// with both the mouse scroll wheel and the up and down scroll buttons of the UI.
	if (MouseCursorIsInRect(&rects.recipe_list, cursor.x, cursor.y)) {
		if (MouseWheelUpPressed() && can_scroll_up()) {
			ui.scroll_offset--;
		}
		if (MouseWheelDownPressed() && can_scroll_down()) {
			ui.scroll_offset++;
		}
		if (MouseLeftClicked()) {
			if (MouseCursorIsOnButton(ADDON_CRAFTING_SCROLL_UP_BUTTON, cursor.x, cursor.y) &&
			           can_scroll_up()) {
				ui.scroll_offset--;
			} else if (MouseCursorIsOnButton(ADDON_CRAFTING_SCROLL_DOWN_BUTTON, cursor.x, cursor.y) &&
			           can_scroll_down()) {
				ui.scroll_offset++;
			} else {
				int clicked_row = (cursor.y - rects.recipe_list.y) / RECIPE_LIST_ROW_HEIGHT;
				if (clicked_row + ui.scroll_offset < ui.recipes.size) {
					select_recipe(clicked_row + ui.scroll_offset);
				}
			}
		}
	}

	// Handle scrolling of the description using the buttons in the UI.
	if (MouseLeftClicked()) {
		if (MouseCursorIsOnButton(ADDON_CRAFTING_SCROLL_DESC_UP_BUTTON, cursor.x, cursor.y)) {
			ui.description.scroll_offset--;
		} else if (MouseCursorIsOnButton(ADDON_CRAFTING_SCROLL_DESC_DOWN_BUTTON, cursor.x, cursor.y)) {
			ui.description.scroll_offset++;
		}
	}

	// Handle hovering and clicks of the apply and close buttons. We need to reset
	// the cursor to normal if it's on a button since the text widget might have
	// changed it to a scrolling cursor in the previous call.
	if (MouseCursorIsOnButton(ITEM_UPGRADE_APPLY_BUTTON, cursor.x, cursor.y)) {
		if (MouseLeftClicked()) {
			craft_item();
		}
		mouse_cursor = MOUSE_CURSOR_NORMAL;
		return;
	}
	if (MouseCursorIsOnButton(ITEM_UPGRADE_CLOSE_BUTTON, cursor.x, cursor.y)) {
		if (MouseLeftClicked()) {
			ui.quit = TRUE;
		}
		mouse_cursor = MOUSE_CURSOR_NORMAL;
		return;
	}

	// Handle events to the description text widget. Since the buttons overlap
	// with the text widget, we only call this when the cursor is not on them.
	// Otherwise, the text widget would handle the events of the buttons.
	widget_handle_mouse(&ui.description);
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
	init_text_widget(&ui.description, "");
	ui.description.font = Messagevar_BFont;
	ui.description.rect = rects.recipe_desc;
	ui.description.content_above_func = draw_scroll_desc_up_button;
	ui.description.content_below_func = draw_scroll_desc_down_button;
	build_recipe_list();
	select_recipe(0);

	// Setup the correct screen state.
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
		AssembleCombatPicture(DO_SCREEN_UPDATE | SHOW_ITEMS);

		limit_fps();
		ComputeFPSForThisFrame();
	}

	// Free the description and the recipe list.
	free_autostr(ui.description.text);
	dynarray_free(&ui.recipes);

	ui.visible = FALSE;
	game_status = old_game_status;
}
