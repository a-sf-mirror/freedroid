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
#include "widgets/widgets.h"

/**
 * \file addon_crafting_ui.c
 *
 * \brief The add-on crafting user interface.
 */

#define RECIPE_LIST_ROWS 4
#define RECIPE_LIST_ROW_HEIGHT 45
#define RECIPE_LIST_IMG_HEIGHT 32
#define RECIPE_LIST_IMG_WIDTH 32

struct crafting_recipe {
	int available; /// TRUE if the player has enough materials to craft the add-on.
	int item_type; /// The type number of the add-on this recipe will craft.
};

struct material {
	int item_type; //item type from the recipe
	int available; //# of items in inventory
	int required; // # if items required by recipe
};

static struct {
	int visible;
	int quit;
	int selection;
	int scroll_offset; /// The scroll offset of the recipe list, in full rows.
	struct dynarray recipes;
	struct widget_text description;
	struct material materials_for_selected[5];
} ui = { .visible = FALSE };

static const struct {
	SDL_Rect main;
	SDL_Rect title_text;
	SDL_Rect details_text;
	SDL_Rect recipe_list;
	SDL_Rect recipe_desc;
	SDL_Rect materials_list;
} rects = {
	{ ADDON_CRAFTING_RECT_X, ADDON_CRAFTING_RECT_Y, ADDON_CRAFTING_RECT_W, ADDON_CRAFTING_RECT_H }, //main
	{ ADDON_CRAFTING_RECT_X + 20, ADDON_CRAFTING_RECT_Y + 12, 280, 38 }, //title_text
	{ ADDON_CRAFTING_RECT_X + 25, ADDON_CRAFTING_RECT_Y + 260, 130, 20 }, //details_text
	{ ADDON_CRAFTING_RECT_X + 20, ADDON_CRAFTING_RECT_Y + 62, 276, 180 }, //recipe_list
	{ ADDON_CRAFTING_RECT_X + 22, ADDON_CRAFTING_RECT_Y + 290, 220, 135 }, //recipe_desc
	{ ADDON_CRAFTING_RECT_X + ADDON_CRAFTING_RECT_W , ADDON_CRAFTING_RECT_Y + 62, 65, 310 } //materials_list
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

	//clean the array
	memset(&ui.materials_for_selected, 0, sizeof(struct material) * 5);
	for (i = 0; i < spec->materials.size; i++) {
		ui.materials_for_selected[i].item_type = get_item_type_by_id(materials[i].name);
		ui.materials_for_selected[i].required = materials[i].value;
		ui.materials_for_selected[i].available = CountItemtypeInInventory(ui.materials_for_selected[i].item_type);
	}

	// Scroll the text widget to the top.
	ui.description.scroll_offset = -get_lines_needed(desc->value,
	            WIDGET(&ui.description)->rect, ui.description.line_height_factor);
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
			int type = get_item_type_by_id(materials[j].name);
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
			int type = get_item_type_by_id(materials[i].name);
			int count = materials[i].value;
			DeleteInventoryItemsOfType(type, count);
		}

		// Create the item and add it to the inventory.
		int type = arr[ui.selection].item_type;
		it = create_item_with_id(ItemMap[type].id, TRUE, 1);
		give_item(&it);

		// The player lost some materials so some of the recipes might have become
		// unaffordable. We need to recheck for the requirements because of this.
		check_recipe_requirements();
		// And we need recheck the materials showed for the selected recipe
		select_recipe(ui.selection);
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
	blit_background("item_upgrade_crafting.png");

	// Draw the title.
	SetCurrentFont(Menu_BFont);
	display_text(_("Craft Addons"), rects.title_text.x, rects.title_text.y, NULL);

	// Draw the details string.
	SetCurrentFont(Blue_BFont);
	display_text(_("Details"), rects.details_text.x, rects.details_text.y, NULL);

	// Draw the parts text
	SetCurrentFont(Blue_BFont);
	display_text(_("Parts"), rects.materials_list.x, rects.materials_list.y, NULL);

	// Draw the apply and close buttons.
	if (arr[ui.selection].available) {
		ShowGenericButtonFromList(ADDON_CRAFTING_APPLY_BUTTON);
	} else {
		ShowGenericButtonFromList(ADDON_CRAFTING_APPLY_BUTTON_DISABLED);
	}
	ShowGenericButtonFromList(ADDON_CRAFTING_CLOSE_BUTTON);

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
		display_text(item_specs_get_name(type), rect.x + rect.h, rect.y + 4, NULL);
		struct image *img = get_item_inventory_image(type);
		if (img) {
			float scale = (float)(RECIPE_LIST_IMG_WIDTH + RECIPE_LIST_IMG_HEIGHT) / (img->w + img->h);

			SDL_Rect icon_rect;
			icon_rect.w = RECIPE_LIST_ROW_HEIGHT;
			icon_rect.h = RECIPE_LIST_ROW_HEIGHT;
			icon_rect.x = rect.x + (icon_rect.w - (img->w * scale)) / 2;
			icon_rect.y = rect.y + (icon_rect.h - (img->h * scale)) / 2;
			display_image_on_screen(img, icon_rect.x, icon_rect.y, IMAGE_SCALE_TRANSFO(scale));
		}
		rect.y += rect.h;
	}

#define MARGIN_SPACE (5)
#define WIDTH_BORDER (5)
	// Draw the parts on the tab
	rect.x = rects.materials_list.x;
	rect.y = rects.materials_list.y + 20 + MARGIN_SPACE;
	rect.w = rects.materials_list.w;

	char text[10];
	for (i = 0; i < 5; i++) {
		if(ui.materials_for_selected[i].required == 0){
			continue;
		}
		// |         |
		// |  @@@@@  |
		// |  @@@@@  | < image
		// |  @@@@@  |
		// | 00 / 11 | < text

		//search for image and display it
		struct image *img = get_item_inventory_image(ui.materials_for_selected[i].item_type);
		int x = rect.x + 30 - (img->h / 2);
		int y = rect.y + MARGIN_SPACE;
		display_image_on_screen(img, x, y, IMAGE_NO_TRANSFO);

		//display the text
		y = rect.y + MARGIN_SPACE + img->h ;

		//first the divisor  (11 part in the diagram)
		sprintf( text ,"%s%02d", font_switchto_neon, ui.materials_for_selected[i].required);
		int w_text = text_width(GetCurrentFont(), text);
		x = rect.x + rect.w  // the right border
			 - WIDTH_BORDER // the blue border
			 - w_text; // right justified
		display_text(text, x, y, NULL);

		//then the '/' center
		w_text = text_width(GetCurrentFont(), "/");
		int half = w_text / 2;
		x = rect.x + (rect.w / 2) // center of the column
			 - half; // center placed
		display_text("/", x, y, NULL);

		//last dividend (00 part in the diagram)
		if (ui.materials_for_selected[i].available < ui.materials_for_selected[i].required) {
			sprintf( text ,"%s%02d", font_switchto_red, ui.materials_for_selected[i].available);
		} else {
			if (ui.materials_for_selected[i].available > 100)
				sprintf( text ,"%s--", font_switchto_neon);
			else
				sprintf( text ,"%s%02d", font_switchto_neon, ui.materials_for_selected[i].available);
		}
		w_text = text_width(GetCurrentFont(), text);
		x = rect.x + (rect.w / 2) // center of the column
			 - half // the part used by /
			 - w_text; // right justified
		display_text(text, x , y , NULL);

		rect.y += MARGIN_SPACE + img->h + 20;
	}

	// Draw the scroll buttons.
	if (can_scroll_up()) {
		ShowGenericButtonFromList(ADDON_CRAFTING_SCROLL_UP_BUTTON);
	}
	if (can_scroll_down()) {
		ShowGenericButtonFromList(ADDON_CRAFTING_SCROLL_DOWN_BUTTON);
	}

	// Draw the description of the selected recipe.
	widget_text_display(WIDGET(&ui.description));
}

int addon_crafting_ui_visible()
{
	return ui.visible;
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
	if (MouseCursorIsOnButton(ADDON_CRAFTING_APPLY_BUTTON, cursor.x, cursor.y)) {
		if (MouseLeftClicked()) {
			craft_item();
		}
		mouse_cursor = MOUSE_CURSOR_NORMAL;
		return;
	}
	if (MouseCursorIsOnButton(ADDON_CRAFTING_CLOSE_BUTTON, cursor.x, cursor.y)) {
		if (MouseLeftClicked()) {
			ui.quit = TRUE;
		}
		mouse_cursor = MOUSE_CURSOR_NORMAL;
		return;
	}

	// Handle events to the description text widget. Since the buttons overlap
	// with the text widget, we only call this when the cursor is not on them.
	// Otherwise, the text widget would handle the events of the buttons.
	widget_text_handle_mouse(&ui.description);
}

/**
 * \brief Shows the add-on crafting user interface.
 */
void addon_crafting_ui()
{
	int old_game_status = game_status;
	game_status = INSIDE_GAME;

	// Clear the struct and build the recipe list.
	memset(&ui, 0, sizeof(ui));
	widget_text_init(&ui.description, "");
	ui.description.font = Messagevar_BFont;
	WIDGET(&ui.description)->rect = rects.recipe_desc;
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

	// Prevent keyboard input.
	input_hold_keyboard();

	// Loop until the player clicks the close button of the UI or presses and
	// releases escape. We need to ensure that the escape key is released so
	// that it doesn't interfere with the dialog that opened the crafting UI.
	while (!ui.quit || EscapePressed()) {

		// Handle input.
		save_mouse_state();
		update_widgets();
		input_handle();
		handle_ui();

		// Draw the UI. AssembleCombatPicture will take care of calling our
		// drawing function in the right place.
		AssembleCombatPicture(DO_SCREEN_UPDATE | SHOW_ITEMS);
	}

	// Release keyboard input.
	input_release_keyboard();

	// Free the description and the recipe list.
	free_autostr(ui.description.text);
	dynarray_free(&ui.recipes);

	ui.visible = FALSE;
	game_status = old_game_status;
}
