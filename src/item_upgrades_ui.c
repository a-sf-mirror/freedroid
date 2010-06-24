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

#define ADDON_ITEMS_MAX 4
#define ELECTRIC_SOCKET_COST 1
#define MECHANICAL_SOCKET_COST 1
#define UNIVERSAL_SOCKET_COST 1
#define SOCKET_UPGRADE_COST 1

enum {
	IMAGE_SOCKET_NONE,
	IMAGE_SOCKET_MECHANICAL,
	IMAGE_SOCKET_ELECTRIC,
	IMAGE_SOCKET_UNIVERSAL,
	IMAGE_SOCKET_ADD,
	IMAGE_SOCKET_INSTALLED,
	IMAGE_MAX
};

struct upgrade_ui {
	int quit;
	int cost;
	int apply_button_active;
	int create_socket_active;
	int create_socket_index;
	item custom_item;
	item dragged_item;
	item addon_items[ADDON_ITEMS_MAX];
	struct auto_string *bonus_text;
};

struct upgrade_ui_rects {
	SDL_Rect main;
	SDL_Rect title_text;
	SDL_Rect cost_text;
	SDL_Rect money_text;
	SDL_Rect bonus_text;
	SDL_Rect custom_slot;
	SDL_Rect bonus_list;
	SDL_Rect prompt;
	SDL_Rect table[2][5];
	SDL_Rect socket_slots[ADDON_ITEMS_MAX];
};

// Global variables.
static iso_image images[IMAGE_MAX];
static int ui_visible = FALSE;
static int images_loaded = FALSE;
static struct upgrade_ui ui;
static const struct upgrade_ui_rects rects = {
	{ ITEM_UPGRADE_RECT_X, ITEM_UPGRADE_RECT_Y, ITEM_UPGRADE_RECT_W, ITEM_UPGRADE_RECT_H },
	{ ITEM_UPGRADE_RECT_X + 20, ITEM_UPGRADE_RECT_Y + 12, 280, 38 },
	{ ITEM_UPGRADE_RECT_X + 178, ITEM_UPGRADE_RECT_Y + 69, 100, 16 },
	{ ITEM_UPGRADE_RECT_X + 178, ITEM_UPGRADE_RECT_Y + 85, 100, 20 },
	{ ITEM_UPGRADE_RECT_X + 25, ITEM_UPGRADE_RECT_Y + 260, 130, 20 },
	{ ITEM_UPGRADE_RECT_X + 18, ITEM_UPGRADE_RECT_Y + 62, 132, 180 },
	{ ITEM_UPGRADE_RECT_X + 30, ITEM_UPGRADE_RECT_Y + 290, 280, 150 },
	{ ITEM_UPGRADE_RECT_X + 177, ITEM_UPGRADE_RECT_Y + 119, 127, 247 },
	{ { { ITEM_UPGRADE_RECT_X + 177, ITEM_UPGRADE_RECT_Y + 149, 127, 15 },
	    { ITEM_UPGRADE_RECT_X + 177, ITEM_UPGRADE_RECT_Y + 164, 127, 15 },
	    { ITEM_UPGRADE_RECT_X + 177, ITEM_UPGRADE_RECT_Y + 179, 127, 15 },
	    { ITEM_UPGRADE_RECT_X + 177, ITEM_UPGRADE_RECT_Y + 194, 127, 15 },
	    { ITEM_UPGRADE_RECT_X + 177, ITEM_UPGRADE_RECT_Y + 224, 127, 15 } },
	  { { ITEM_UPGRADE_RECT_X + 267, ITEM_UPGRADE_RECT_Y + 149, 37, 15 },
	    { ITEM_UPGRADE_RECT_X + 267, ITEM_UPGRADE_RECT_Y + 164, 37, 15 },
	    { ITEM_UPGRADE_RECT_X + 267, ITEM_UPGRADE_RECT_Y + 179, 37, 15 },
	    { ITEM_UPGRADE_RECT_X + 267, ITEM_UPGRADE_RECT_Y + 194, 37, 15 },
	    { ITEM_UPGRADE_RECT_X + 267, ITEM_UPGRADE_RECT_Y + 224, 37, 15 } } },
	{ { ITEM_UPGRADE_RECT_X + 177, ITEM_UPGRADE_RECT_Y + 117, 60, 60 },
	  { ITEM_UPGRADE_RECT_X + 241, ITEM_UPGRADE_RECT_Y + 117, 60, 60 },
	  { ITEM_UPGRADE_RECT_X + 177, ITEM_UPGRADE_RECT_Y + 181, 60, 60 },
	  { ITEM_UPGRADE_RECT_X + 241, ITEM_UPGRADE_RECT_Y + 181, 60, 60 } }
};

static void load_images()
{
	int i;
	char fpath[2048];
	const char *fnames[IMAGE_MAX] = {
		"item_upgrade/socket_none.png",
		"item_upgrade/socket_mechanical.png",
		"item_upgrade/socket_electric.png",
		"item_upgrade/socket_universal.png",
		"item_upgrade/socket_add.png",
		"item_upgrade/socket_installed.png"
	};

	for (i = 0; i < IMAGE_MAX; i++) {
		find_file(fnames[i], GRAPHICS_DIR, fpath, FALSE);
		memset(&images[i], 0, sizeof(iso_image));
		get_iso_image_from_file_and_path(fpath, &images[i], FALSE);
	}
}

/**
 * \brief Gets the item grabbed from the item upgrade UI by the player.
 * \return Pointer to an item or NULL if no item was grabbed from the upgrade UI.
 */
item *get_item_grabbed_from_item_upgrade_ui()
{
	// Make sure the UI is actually visible.
	if (!ui_visible) {
		return NULL;
	}

	// Return the dragged item or NULL.
	if (ui.dragged_item.type != -1)
		return &ui.dragged_item;
	return NULL;
}

/**
 * \brief Gets the tooltip text the item upgrade UI would like to display.
 * \param cursor Cursor position.
 * \param result Return location for the tooltip text.
 * \return TRUE if the tooltip was set, FALSE if the cursor isn't on the upgrade UI.
 */
int get_item_upgrade_ui_tooltip(const point *cursor, char *result)
{
	int i;

	// Make sure the UI is actually visible.
	if (!ui_visible) {
		return FALSE;
	}

	// Check if a tooltip for an add-on item needs to be displayed.
	if (!ui.create_socket_active) {
		for (i = 0; i < ADDON_ITEMS_MAX; i++) {
			if (MouseCursorIsInRect(&rects.socket_slots[i], cursor->x, cursor->y)) {
				if (ui.addon_items[i].type != -1) {
					give_item_description(result, &ui.addon_items[i]);
					return TRUE;
				}
			}
		}
	}

	// Check if a tooltip for the customized item needs to be displayed.
	if (MouseCursorIsInRect(&rects.custom_slot, cursor->x, cursor->y)) {
		if (ui.custom_item.type != -1) {
			give_item_description(result, &ui.custom_item);
			return TRUE;
		}
	}

	// If the cursor is inside the upgrade UI but not on any item, tell the
	// caller that no tooltip should be shown. The tooltips of NPCs and other
	// such things behind the upgrade UI would be displayed without this.
	if (MouseCursorIsInRect(&rects.main, cursor->x, cursor->y)) {
		strcpy(result, "");
		return TRUE;
	}

	return FALSE;
}

/**
 * \brief Draws the item upgrade user interface.
 *
 * The UI consists of an area where the customized item is placed, an area to
 * the right from it that contains the sockets and the items in them, and a text
 * area below the two that lists the bonuses the item will have. The UI also
 * has text labels for the title, a money string, and a cost string.
 *
 * When no item is placed for customization, a small text label asking the user
 * to drag an item to the slot is displayed. When the user drags an item to the
 * slot, the text label is hidden and the rotation animation of the item is displayed.
 *
 * When the user is adding a new socket or upgrading an existing socket, the socket
 * images aren't drawn. Instead, a two column table that contains the available buy
 * or upgrade options and their prices is drawn to the socket area.
 */
void show_item_upgrade_ui()
{
	int i;
	char buffer[512];
	iso_image *image;
	SDL_Rect rect;
	SDL_Surface *surface;
	struct upgrade_socket_dynarray *sockets = &ui.custom_item.upgrade_sockets;

	// We're being called every time the UI of the game is drawn so we need to
	// make sure the upgrade UI is actually active before proceeding.
	if (!ui_visible) {
		return;
	}

	// Draw the background image.
	blit_special_background(ITEM_UPGRADE_BACKGROUND_CODE);

	// Draw the apply and close buttons.
	if (ui.apply_button_active) {
		ShowGenericButtonFromList(ITEM_UPGRADE_APPLY_BUTTON);
	} else {
		ShowGenericButtonFromList(ITEM_UPGRADE_APPLY_BUTTON_DISABLED);
	}
	ShowGenericButtonFromList(ITEM_UPGRADE_CLOSE_BUTTON);

	// Draw the customized item.
	if (ui.custom_item.type != -1) {
		ShowItemPicture(rects.custom_slot.x, rects.custom_slot.y, ui.custom_item.type);
	} else {
		SetCurrentFont(Messagestat_BFont);
		DisplayText(_("Drag an item here"), rects.custom_slot.x,
		            rects.custom_slot.y + rects.custom_slot.h / 2, NULL, TEXT_STRETCH);
	}

	// Draw the title.
	SetCurrentFont(Menu_BFont);
	DisplayText(_("Upgrade Items"), rects.title_text.x, rects.title_text.y, NULL, TEXT_STRETCH);

	// Draw the text labels.
	SetCurrentFont(Messagevar_BFont);
	sprintf(buffer, _("Money: %d"), Me.Gold);
	DisplayText(buffer, rects.money_text.x, rects.money_text.y, NULL, TEXT_STRETCH);
	if (ui.cost > Me.Gold) {
		SetCurrentFont(Messagestat_BFont);
	} else {
		SetCurrentFont(Messagevar_BFont);
	}
	sprintf(buffer, _("Cost: %d"), ui.cost);
	DisplayText(buffer, rects.cost_text.x, rects.cost_text.y, NULL, TEXT_STRETCH);
	SetCurrentFont(Blue_BFont);
	DisplayText(_("Bonuses"), rects.bonus_text.x, rects.bonus_text.y, NULL, TEXT_STRETCH);

	// Draw the item bonus string.
	SetCurrentFont(Messagevar_BFont);
	DisplayText(ui.bonus_text->value, rects.bonus_list.x, rects.bonus_list.y,
	            &rects.bonus_list, TEXT_STRETCH);

	if (ui.create_socket_active) {

		// Show the socket creation prompt if the player is creating a new socket.
		// It's just a simple two column table with names and costs in it.
		SetCurrentFont(Blue_BFont);
		DisplayText(_("Add socket"), rects.prompt.x, rects.prompt.y, NULL, TEXT_STRETCH);
		SetCurrentFont(Messagestat_BFont);
		DisplayText(_("Type"), rects.table[0][0].x, rects.table[0][0].y, NULL, TEXT_STRETCH);
		DisplayText(_("Cost"), rects.table[1][0].x, rects.table[1][0].y, NULL, TEXT_STRETCH);
		SetCurrentFont(Messagevar_BFont);
		if (ui.create_socket_index == sockets->size) {
			DisplayText(_("Electric"), rects.table[0][1].x, rects.table[0][1].y, NULL, TEXT_STRETCH);
			DisplayText(_("Mechanical"), rects.table[0][2].x, rects.table[0][2].y, NULL, TEXT_STRETCH);
			DisplayText(_("Universal"), rects.table[0][3].x, rects.table[0][3].y, NULL, TEXT_STRETCH);
			sprintf(buffer, "%d", ELECTRIC_SOCKET_COST);
			DisplayText(buffer, rects.table[1][1].x, rects.table[1][1].y, NULL, TEXT_STRETCH);
			sprintf(buffer, "%d", MECHANICAL_SOCKET_COST);
			DisplayText(buffer, rects.table[1][2].x, rects.table[1][2].y, NULL, TEXT_STRETCH);
			sprintf(buffer, "%d", UNIVERSAL_SOCKET_COST);
			DisplayText(buffer, rects.table[1][3].x, rects.table[1][3].y, NULL, TEXT_STRETCH);
		} else {
			DisplayText(_("Universal"), rects.table[0][1].x, rects.table[0][1].y, NULL, TEXT_STRETCH);
			sprintf(buffer, "%d", SOCKET_UPGRADE_COST);
			DisplayText(buffer, rects.table[1][1].x, rects.table[1][1].y, NULL, TEXT_STRETCH);
		}
		DisplayText(_("Cancel"), rects.table[0][4].x, rects.table[0][4].y, NULL, TEXT_STRETCH);

	} else {

		// Draw the base socket images.
		for (i = 0; i < ADDON_ITEMS_MAX; i++) {
			if (i < sockets->size) {
				switch (sockets->arr[i].type) {
				case UPGRADE_SOCKET_TYPE_MECHANICAL:
					image = &images[IMAGE_SOCKET_MECHANICAL];
					break;
				case UPGRADE_SOCKET_TYPE_ELECTRIC:
					image = &images[IMAGE_SOCKET_ELECTRIC];
					break;
				case UPGRADE_SOCKET_TYPE_UNIVERSAL:
					image = &images[IMAGE_SOCKET_UNIVERSAL];
					break;
				default:
					image = NULL;
					break;
				}
			} else if (ui.custom_item.type != -1 && i == sockets->size) {
				image = &images[IMAGE_SOCKET_ADD];
			} else {
				image = &images[IMAGE_SOCKET_NONE];
			}
			rect = rects.socket_slots[i];
			rect.x += (rect.w - image->original_image_width) / 2;
			rect.y += (rect.h - image->original_image_height) / 2;
			blit_iso_image_to_screen_position(image, rect.x, rect.y);
			if (i < sockets->size && ui.custom_item.upgrade_sockets.arr[i].addon) {
				image = &images[IMAGE_SOCKET_INSTALLED];
				rect = rects.socket_slots[i];
				rect.x += (rect.w - image->original_image_width) / 2;
				rect.y += (rect.h - image->original_image_height) / 2;
				blit_iso_image_to_screen_position(image, rect.x, rect.y);
			}
		}

		// Draw the items in the sockets.
		for (i = 0; i < ADDON_ITEMS_MAX; i++) {
			if (ui.addon_items[i].type != -1) {
				surface = ItemMap[ui.addon_items[i].type].inv_image.Surface;
				if (surface) {
					rect = rects.socket_slots[i];
					rect.x += (rect.w - surface->w) / 2;
					rect.y += (rect.h - surface->h) / 2;
					our_SDL_blit_surface_wrapper(surface, NULL, Screen, &rect);
				}
			}
		}

	}
}

static void clear_addon_items()
{
	int i;
	struct upgrade_socket *socket;

	// Remove items from the socket slots. Items that are not present in
	// the upgrade socket array are considered not installed and will be
	// dropped to the inventory. Items already installed to the sockets
	// are deleted directly in order not to duplicate them.
	for (i = 0; i < ADDON_ITEMS_MAX; i++) {
		if (ui.addon_items[i].type != -1) {
			socket = &ui.custom_item.upgrade_sockets.arr[i];
			if (socket->addon) {
				DeleteItem(&ui.addon_items[i]);
			} else if (AddFloorItemDirectlyToInventory(&ui.addon_items[i])) {
				DropItemToTheFloor(&ui.addon_items[i], Me.pos.x, Me.pos.y, Me.pos.z);
			}
		}
	}
}

static void calculate_cost_and_bonuses()
{
	int i;
	int changed = FALSE;
	item temp;
	item *addon;
	struct upgrade_socket *socket;
	struct addon_spec *spec;

	// Sum the upgrade costs of all add-ons that are present
	// in the UI slots but not in the upgrade socket array.
	ui.cost = 0;
	if (ui.custom_item.type != -1) {
		for (i = 0; i < ADDON_ITEMS_MAX && i < ui.custom_item.upgrade_sockets.size; i++) {
			addon = &ui.addon_items[i];
			socket = &ui.custom_item.upgrade_sockets.arr[i];
			if (!socket->addon && addon->type != -1) {
				spec = get_addon_spec(addon->type);
				ui.cost += spec->upgrade_cost;
				changed = TRUE;
			}
		}
	}

	// The apply button is enabled if the player has enough money
	// to fully perform the upgrade operation.
	if (Me.Gold >= ui.cost && changed) {
		ui.apply_button_active = TRUE;
	} else {
		ui.apply_button_active = FALSE;
	}

	// Rebuild the item bonuses using a temporary item that has all the
	// yet to be installed add-ons installed.
	CopyItem(&ui.custom_item, &temp, FALSE);
	for (i = 0; i < ADDON_ITEMS_MAX; i++) {
		if (i < temp.upgrade_sockets.size) {
			addon = &ui.addon_items[i];
			if (addon->type != -1) {
				socket = &temp.upgrade_sockets.arr[i];
				free(socket->addon);
				socket->addon = strdup(ItemMap[addon->type].item_name);
			}
		}
	}
	calculate_item_bonuses(&temp);
	autostr_printf(ui.bonus_text, "");
	get_item_bonus_string(&temp, "\n", ui.bonus_text);
	DeleteItem(&temp);
}

static int grab_customized_item()
{
	// If the slot is empty, do nothing and return FALSE to inform
	// the caller that no drag could be initiated.
	if (ui.custom_item.type == -1) {
		return FALSE;
	}

	// If the player dragged some add-ons to the sockets but didn't
	// press the apply button, return the add-on items to the
	// inventory so that the player doesn't lose them.
	clear_addon_items(ui);

	// Mark the item as being dragged. We move it to a temporary
	// field so that it doesn't get cleared when we alter the UI. 
	MoveItem(&ui.custom_item, &ui.dragged_item);
	Item_Held_In_Hand = &ui.dragged_item;
	Item_Held_In_Hand->currently_held_in_hand = TRUE;
	play_item_sound(Item_Held_In_Hand->type);

	// Reset the cost field to zero and disable the apply button
	// since there's no item to customize.
	calculate_cost_and_bonuses(ui);

	// Close the socket creation prompt if it was open. We don't
	// want it to be visible when there's no item to customize.
	ui.create_socket_active = FALSE;

	return TRUE;
}

static int grab_addon_item(int index)
{
	struct upgrade_socket *socket;

	// If the socket is empty, do nothing and return FALSE to inform
	// the caller that no drag could be initiated.
	if (ui.addon_items[index].type == -1) {
		return FALSE;
	}

	// Mark the item as being dragged. We move it to a temporary
	// field so that it doesn't get cleared when we alter the UI. 
	MoveItem(&ui.addon_items[index], &ui.dragged_item);
	Item_Held_In_Hand = &ui.dragged_item;
	Item_Held_In_Hand->currently_held_in_hand = TRUE;
	play_item_sound(Item_Held_In_Hand->type);

	// Remove the corresponding string from the upgrade socket array. We
	// use the NULL pointer to identify sockets that have been modified
	// and need to be updated when the player pressed the apply button.
	socket = &ui.custom_item.upgrade_sockets.arr[index];
	free (socket->addon);
	socket->addon = NULL;

	// Recalculate the bonuses of the customized item since those might
	// have changed if the add-on was already installed.
	calculate_item_bonuses(&ui.custom_item);

	// Removing a yet to be installed item reduces the cost and modifies
	// bonuses so we need to recalculate them.
	calculate_cost_and_bonuses(ui);

	return TRUE;
}

static void clear_customized_item()
{
	// If an item was being customized, drop it and any add-ons not yet
	// installed to it to the inventory so that the player doesn't lose them.
	if (ui.custom_item.type != -1) {
		clear_addon_items(ui);
		if (AddFloorItemDirectlyToInventory(&ui.custom_item)) {
			DropItemToTheFloor(&ui.custom_item, Me.pos.x, Me.pos.y, Me.pos.z);
		}
	}

	// Reset the cost to zero and disable the apply button since
	// there's no item to customize present in the interface anymore.
	calculate_cost_and_bonuses(ui);
}

static int set_customized_item(item *it)
{
	int i;
	struct upgrade_socket *socket;

	// Make sure the item is of a customizable type.
	if (!item_can_be_customized(it)) {
		return FALSE;
	}

	// If there are any items in the upgrade UI, drop them
	// to the inventory so that they aren't overwritten and lost.
	clear_customized_item(ui);

	// Move the item to the customization slot.
	play_item_sound(it->type);
	it->currently_held_in_hand = FALSE;
	MoveItem(it, &ui.custom_item);

	// Create add-on items from the strings in the upgrade socket array
	// so that they player can drag the existing add-ons out of the sockets.
	for (i = 0; i < ADDON_ITEMS_MAX && i < ui.custom_item.upgrade_sockets.size; i++) {
		socket = &ui.custom_item.upgrade_sockets.arr[i];
		if (socket->addon) {
			ui.addon_items[i] = create_item_with_name(socket->addon, TRUE, 1);
		}
	}

	// Update the bonus string since the item might have existing bonuses.
	calculate_cost_and_bonuses(ui);

	return TRUE;
}

static int set_addon_item(item *it, int index)
{
	// Make sure the socket exists and is empty and that the socket and the
	// dragged item are compatible with each other.
	if (ui.custom_item.upgrade_sockets.size <= index ||
	       ui.addon_items[index].type != -1 ||
	       !item_can_be_installed_to_socket(&ui.custom_item, it, index)) {
		return FALSE;
	}

	// Move the item to the socket slot. The upgrade socket array of the 
	// customized item remains unchanged; the player needs to press the
	// apply button before any modifications to the item take place.
	play_item_sound(it->type);
	it->currently_held_in_hand = FALSE;
	MoveItem(it, &ui.addon_items[index]);

	// Recalculate the cost and the bonus string and enable the apply button
	// if the player has enough gold to pay for the upgrade.
	calculate_cost_and_bonuses(ui);

	return TRUE;
}

static void apply_customization()
{
	int i;
	item *addon;
	struct upgrade_socket *socket;

	// Subtract money from the player. The player is guaranteed to have enough
	// if he gets here since the apply button would be disabled otherwise.
	Me.Gold -= ui.cost;

	// Add each item not already present in the upgrade socket array to it.
	// The upgrade socket array only contains the names of the add-on items
	// so that we don't need to store items recursively inside each other.
	for (i = 0; i < ADDON_ITEMS_MAX && i < ui.custom_item.upgrade_sockets.size; i++) {
		addon = &ui.addon_items[i];
		socket = &ui.custom_item.upgrade_sockets.arr[i];
		if (!socket->addon && addon->type != -1) {
			free(socket->addon);
			socket->addon = strdup(ItemMap[addon->type].item_name);
		}
	}

	// Calculate the new item bonuses that will modify the attributes
	// of Tux when he equips the item.
	calculate_item_bonuses(&ui.custom_item);

	// Reset the cost to zero and disable the apply button since we
	// just did all the changes the player has requested.
	calculate_cost_and_bonuses(ui);
}

static void handle_buy_socket(point *cursor, int row, int type, int cost)
{
	struct upgrade_socket_dynarray *sockets = &ui.custom_item.upgrade_sockets;

	if (MouseCursorIsInRect(&rects.table[0][row], cursor->x, cursor->y)) {
		if (Me.Gold >= cost) {
			if (ui.create_socket_index == sockets->size) {
				// Create a new socket.
				struct upgrade_socket socket = { type, NULL };
				dynarray_add((struct dynarray *) sockets, &socket, sizeof(socket));
			} else {
				// Modify an existing socket.
				sockets->arr[ui.create_socket_index].type = type;
			}
			ui.create_socket_active = FALSE;
			Me.Gold -= cost;
		}
	}
}

static void handle_socket(int index)
{
	struct upgrade_socket_dynarray *sockets = &ui.custom_item.upgrade_sockets;

	// Nothing to do if there's no customized item.
	if (ui.custom_item.type == -1) {
		return;
	}

	// If the player is dragging an item, try to drop a it to the socket.
	// If the item doesn't fit into the socket, silently give up.
	if (Item_Held_In_Hand) {
		if (set_addon_item(Item_Held_In_Hand, index)) {
			Item_Held_In_Hand = NULL;
		}

	// The player isn't dragging an item so check if an item can be dragged
	// out of the socket. If so, removed the item from the socket and mark
	// it as being dragged.
	} else if (ui.addon_items[index].type != -1) {
		grab_addon_item(index);

	// Allow the user to create a new socket if this is the first socket not yet created.
	// Opens the socket creation prompt in creation mode.
	} else if (index == sockets->size && index < ADDON_ITEMS_MAX) {
		ui.create_socket_active = TRUE;
		ui.create_socket_index = index;

	// Try to upgrade the socket if this is an existing socket.
	// Opens the socket creation prompt in upgrade mode.
	} else if (index < sockets->size && sockets->arr[index].type != UPGRADE_SOCKET_TYPE_UNIVERSAL) {
		ui.create_socket_active = TRUE;
		ui.create_socket_index = index;
	}
}

static int handle_ui()
{
	int i;
	point cursor;

	// Check for quit with the escape key.
	if (EscapePressed()) {
		ui.quit = TRUE;
		return TRUE;
	}

	// Check if we need to do anything.
	if (!MouseLeftClicked() || global_ingame_mode == GLOBAL_INGAME_MODE_IDENTIFY) {
		return FALSE;
	}

	// Get the position of the cursor.
	cursor.x = GetMousePos_x();
	cursor.y = GetMousePos_y();

	// Handle clicks to the socket creation prompt if it's visible.
	// Clicks there may either create a new socket and/or close the prompt.
	if (ui.create_socket_active) {
		if (ui.create_socket_index == ui.custom_item.upgrade_sockets.size) {
			// Create a new socket.
			handle_buy_socket(&cursor, 1, UPGRADE_SOCKET_TYPE_ELECTRIC, ELECTRIC_SOCKET_COST);
			handle_buy_socket(&cursor, 2, UPGRADE_SOCKET_TYPE_MECHANICAL, MECHANICAL_SOCKET_COST);
			handle_buy_socket(&cursor, 3, UPGRADE_SOCKET_TYPE_UNIVERSAL, UNIVERSAL_SOCKET_COST);
		} else {
			// Upgrade an existing socket.
			handle_buy_socket(&cursor, 1, UPGRADE_SOCKET_TYPE_UNIVERSAL, SOCKET_UPGRADE_COST);
		}
		if (MouseCursorIsInRect(&rects.table[0][4], cursor.x, cursor.y)) {
			ui.create_socket_active = FALSE;
			return TRUE;
		}

	// If no socket creation prompt is open, handle clicks to sockets.
	} else {
		for (i = 0; i < ADDON_ITEMS_MAX; i++) {
			if (MouseCursorIsInRect(&rects.socket_slots[i], cursor.x, cursor.y)) {
				handle_socket(i);
				return TRUE;
			}
		}
	}

	// If the user clicked the slot of the customized item, try to either
	// drop a dragged item into it or grab an existing item out of it.
	if (MouseCursorIsInRect(&rects.custom_slot, cursor.x, cursor.y))
	{
		if (!Item_Held_In_Hand) {
			grab_customized_item(ui);
		} else if (set_customized_item(Item_Held_In_Hand)) {
			Item_Held_In_Hand = NULL;
		}
		return TRUE;
	}

	// Handle clicks to the apply button.
	if (MouseCursorIsOnButton(ITEM_UPGRADE_APPLY_BUTTON, cursor.x, cursor.y))
	{
		if (ui.apply_button_active) {
			apply_customization(ui);
		}
		return TRUE;
	}

	// Handle clicks to the close button.
	if (MouseCursorIsOnButton(ITEM_UPGRADE_CLOSE_BUTTON, cursor.x, cursor.y))
	{
		ui.quit = TRUE;
		return TRUE;
	}

	// Don't pass the event to the inventory if the click was still inside
	// the upgrade UI. Otherwise, missing a valid slot would result to the
	// item being dropped to the floor.
	if (MouseCursorIsInRect(&rects.main, cursor.x, cursor.y))
		return TRUE;

	return FALSE;
}

/**
 * \brief Shows the item upgrade user interface.
 */
void item_upgrade_ui()
{
	int i;
	int old_game_status = game_status;
	game_status = INSIDE_MENU;

	// Load all static images if not already loaded.
	if (!images_loaded) {
		load_images();
		images_loaded = TRUE;
	}

	// Clear the struct and initialize the items as empty.
	memset(&ui, 0, sizeof(struct upgrade_ui));
	ui.bonus_text = alloc_autostr(64);
	init_item(&ui.dragged_item);
	init_item(&ui.custom_item);
	for (i = 0; i < ADDON_ITEMS_MAX; i++) {
		init_item(&ui.addon_items[i]);
	}

	// Setup the correct screen state.
	make_sure_system_mouse_cursor_is_turned_off();
	GameConfig.Inventory_Visible = TRUE;
	GameConfig.CharacterScreen_Visible = FALSE;
	GameConfig.SkillScreen_Visible = FALSE;
	GameConfig.skill_explanation_screen_visible = FALSE;
	ui_visible = TRUE;

	// Loop until the player clicks the close button of the UI or presses and
	// releases escape. We need to ensure that the escape key is released so
	// that it doesn't interfere with the dialog that opened the upgrade UI.
	while (!ui.quit || EscapePressed()) {
		StartTakingTimeForFPSCalculation();

		// Handle input. Since the UI makes use of the inventory, we need to let
		// the inventory handle events that fall outside of the upgrade UI.
		save_mouse_state();
		input_handle();
		if (!handle_ui())
			HandleInventoryScreen();

		// Draw the UI. AssembleCombatPicture will take care of calling our
		// drawing function in the right place.
		AssembleCombatPicture(DO_SCREEN_UPDATE | SHOW_ITEMS | USE_OWN_MOUSE_CURSOR);

		// Limit framerate if configured to do so.
		limit_fps();
		ComputeFPSForThisFrame();
	}

	// If the player left any items to the upgrade interface,
	// drop them to the inventory or to the ground.
	clear_customized_item(ui);

	free_autostr(ui.bonus_text);

	ui_visible = FALSE;
	GameConfig.Inventory_Visible = FALSE;
	game_status = old_game_status;
}
