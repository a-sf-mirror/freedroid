/* 
 *
 *   Copyright (c) 2002, 2003 Johannes Prix
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
 * This file contains all menu functions and their subfunctions
 */

#define _shop_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "widgets/widgets.h"

#define SHOP_ROW_LENGTH 8

SDL_Rect ShopItemRowRect;
SDL_Rect TuxItemRowRect;

/**
 * At some points in the game, like when at the shop interface or at the
 * items browser at the console, we wish to show a list of the items 
 * currently in inventory.  This function assembles this list.  It lets
 * the caller decide on whether to include worn items in the list or not
 * and it will return the number of items finally filled into that list.
 */
int AssemblePointerListForItemShow(item ** ItemPointerListPointer, int IncludeWornItems)
{
	int i;
	item **CurrentItemPointer;
	int NumberOfItems = 0;

	// First we clean out the new Show_Pointer_List
	//
	CurrentItemPointer = ItemPointerListPointer;
	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		*CurrentItemPointer = NULL;
		CurrentItemPointer++;
	}

	// Now we start to fill the Show_Pointer_List with the items
	// currently equipped, if that is what is desired by parameters...
	//
	CurrentItemPointer = ItemPointerListPointer;
	if (IncludeWornItems) {
		if (Me.weapon_item.type != (-1)) {
			*CurrentItemPointer = &(Me.weapon_item);
			CurrentItemPointer++;
			NumberOfItems++;
		}
		if (Me.drive_item.type != (-1)) {
			*CurrentItemPointer = &(Me.drive_item);
			CurrentItemPointer++;
			NumberOfItems++;
		}
		if (Me.armour_item.type != (-1)) {
			*CurrentItemPointer = &(Me.armour_item);
			CurrentItemPointer++;
			NumberOfItems++;
		}
		if (Me.shield_item.type != (-1)) {
			*CurrentItemPointer = &(Me.shield_item);
			CurrentItemPointer++;
			NumberOfItems++;
		}
		if (Me.special_item.type != (-1)) {
			*CurrentItemPointer = &(Me.special_item);
			CurrentItemPointer++;
			NumberOfItems++;
		}
	}
	// Now we start to fill the Show_Pointer_List with the items in the
	// pure unequipped inventory
	//
	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		if (Me.Inventory[i].type == (-1) || Me.Inventory[i].type < 0)
			continue;
		else {
			*CurrentItemPointer = &(Me.Inventory[i]);
			CurrentItemPointer++;
			NumberOfItems++;
		}
	}

	return (NumberOfItems);

}

/**
 * Maybe the user has clicked right onto the item overview row.  Then of
 * course we must find out and return the index of the item clicked on.
 * If no item was clicked on, then a -1 will be returned as index.
 */
static int ClickWasOntoItemRowPosition(int x, int y, int TuxItemRow)
{
	if (TuxItemRow) {
		if (y < TuxItemRowRect.y)
			return (-1);
		if (y > TuxItemRowRect.y + TuxItemRowRect.h)
			return (-1);
		if (x < TuxItemRowRect.x)
			return (-1);
		if (x > TuxItemRowRect.x + TuxItemRowRect.w)
			return (-1);

		// Now at this point we know, that the click really was in the item
		// overview row.  Therefore we just need to find out the index and
		// can return;
		//
		return ((x - TuxItemRowRect.x) / (INITIAL_BLOCK_WIDTH * GameConfig.screen_width / 640));
	} else {
		if (y < ShopItemRowRect.y)
			return (-1);
		if (y > ShopItemRowRect.y + ShopItemRowRect.h)
			return (-1);
		if (x < ShopItemRowRect.x)
			return (-1);
		if (x > ShopItemRowRect.x + ShopItemRowRect.w)
			return (-1);

		// Now at this point we know, that the click really was in the item
		// overview row.  Therefore we just need to find out the index and
		// can return;
		//
		return ((x - ShopItemRowRect.x) / (INITIAL_BLOCK_WIDTH * GameConfig.screen_width / 640));
	}
};				// int ClickWasOntoItemRowPosition ( int x , int y , int TuxItemRow )

/**
 * The item row in the shop interface (or wherever we're going to use it)
 * should display not only the rotating item display but also a row or a
 * column of the current equipment, so that some better overview is given
 * as well and the item can be better associated with it's in-game inventory
 * representation.  This function displays one such representation with 
 * the correct size to fit perfectly into the overview item row.
 */
void ShowRescaledItem(int position, int TuxItemRow, item * ShowItem)
{
	SDL_Rect TargetRectangle =
	    { 0, 0, INITIAL_BLOCK_WIDTH * GameConfig.screen_width / 640, INITIAL_BLOCK_HEIGHT * GameConfig.screen_height / 480 };
	static struct image equipped_icon;

	if (!image_loaded(&equipped_icon)) {
		load_image(&equipped_icon, "cursors/mouse_cursor_0003.png", FALSE);
	}

	TuxItemRowRect.x = 55 * GameConfig.screen_width / 640;
	TuxItemRowRect.y = 410 * GameConfig.screen_height / 480;
	TuxItemRowRect.h = INITIAL_BLOCK_HEIGHT * GameConfig.screen_height / 480;
//    TuxItemRowRect . h = 64 ;
	TuxItemRowRect.w = INITIAL_BLOCK_WIDTH * SHOP_ROW_LENGTH * GameConfig.screen_width / 640;
//    TuxItemRowRect . w = 64 ;

	ShopItemRowRect.x = 55 * GameConfig.screen_width / 640;
	ShopItemRowRect.y = 10 * GameConfig.screen_height / 480;
	ShopItemRowRect.h = INITIAL_BLOCK_HEIGHT * GameConfig.screen_height / 480;
	ShopItemRowRect.w = INITIAL_BLOCK_WIDTH * SHOP_ROW_LENGTH * GameConfig.screen_width / 640;
//    ShopItemRowRect . h = 64 ;
//    ShopItemRowRect . w = 64 ;

	if (TuxItemRow == 1) {
		TargetRectangle.x = TuxItemRowRect.x + position * INITIAL_BLOCK_WIDTH * GameConfig.screen_width / 640;
		TargetRectangle.y = TuxItemRowRect.y;
	} else if (TuxItemRow == 0) {
		TargetRectangle.x = ShopItemRowRect.x + position * INITIAL_BLOCK_WIDTH * GameConfig.screen_width / 640;
		TargetRectangle.y = ShopItemRowRect.y;
	} else {
		TargetRectangle.x = ShopItemRowRect.x + position * INITIAL_BLOCK_WIDTH * GameConfig.screen_width / 640;
		TargetRectangle.y = TuxItemRow;
	}
	
	struct image *img = get_item_shop_image(ShowItem->type);
	if (img) {
		display_image_on_screen(img, TargetRectangle.x, TargetRectangle.y, IMAGE_NO_TRANSFO);
	}

	if (item_is_currently_equipped(ShowItem)) {
		display_image_on_screen(&equipped_icon, TargetRectangle.x + TargetRectangle.w - 24, TargetRectangle.y, IMAGE_NO_TRANSFO);
	}
};				// void ShowRescaledItem ( int position , item* ShowItem )

/**
 * This function displays an item picture. 
 */
void ShowItemPicture(int PosX, int PosY, int Number)
{
	static char LastImageSeriesPrefix[1000] = "NONE_AT_ALL";
	static int NumberOfImagesInThisRotation = 1;
#define MAX_NUMBER_OF_IMAGES_IN_ITEM_ROTATION 64
	static struct image item_rotation_img[MAX_NUMBER_OF_IMAGES_IN_ITEM_ROTATION] = { EMPTY_IMAGE };
	int i;
	int RotationIndex;

	if (!image_loaded(&item_rotation_img[0])) {
		// Initialize image structures
		struct image empty = EMPTY_IMAGE;
		for (i = 0; i < sizeof(item_rotation_img)/sizeof(item_rotation_img[0]); i++) {
			memcpy(&item_rotation_img[i], &empty, sizeof(struct image));
		}
	}

	if (strcmp(LastImageSeriesPrefix, ItemMap[Number].item_rotation_series_prefix)) {

		// Free previous images
		for (i = 0; i < MAX_NUMBER_OF_IMAGES_IN_ITEM_ROTATION; i++) {
			delete_image(&item_rotation_img[i]);
		}

		// Load new images
		for (i = 0; i < MAX_NUMBER_OF_IMAGES_IN_ITEM_ROTATION; i++) {
			char ConstructedFileName[PATH_MAX];
			char fpath[PATH_MAX];
			sprintf(ConstructedFileName, "items/%s/portrait_%04d.jpg", ItemMap[Number].item_rotation_series_prefix, i + 1);

			// Look for the next file
			if (!find_file(ConstructedFileName, GRAPHICS_DIR, fpath)) {
				load_image(&item_rotation_img[i], ConstructedFileName, FALSE);
			} else {
				NumberOfImagesInThisRotation = i;

				if (!NumberOfImagesInThisRotation)
					ErrorMessage(__FUNCTION__, "Unable to load any item rotation image for item \"%s\". File \"%s\" was not found.", PLEASE_INFORM, IS_WARNING_ONLY, ItemMap[Number].id, ConstructedFileName);

				break;
			}

		}

		// Remember what series we have just loaded
		strcpy(LastImageSeriesPrefix, ItemMap[Number].item_rotation_series_prefix);
	}

	RotationIndex = (SDL_GetTicks() / 45);

	RotationIndex = RotationIndex - (RotationIndex / NumberOfImagesInThisRotation) * NumberOfImagesInThisRotation;

	if (image_loaded(&item_rotation_img[RotationIndex]))
			display_image_on_screen(&item_rotation_img[RotationIndex], PosX, PosY, IMAGE_NO_TRANSFO);
}

/**
 * Assemble item description.
 */
static void fill_item_description(struct widget_text *desc, item *show_item, int buy)
{
	long int repair_price = 0;
	itemspec *info;
	int price = calculate_item_buy_price(show_item);
	const char *action = _("Buy");
	const char *buyone_highlight = font_switchto_neon;
	const char *buyall_highlight = font_switchto_neon;

	if (show_item == NULL)
		return;

	info = &ItemMap[show_item->type];

	widget_text_init(desc, "");

	append_item_description(desc->text, show_item);
	
	// Now we give some pricing information, the base list price for the item,
	// the repair price and the sell value
	if (price) {
		if (!buy) {
 			price = calculate_item_sell_price(show_item);
			action = _("Sell");
		} else if (price > Me.Gold) {
			buyone_highlight = font_switchto_red;
			buyall_highlight = font_switchto_red;
		} else if ((price * show_item->multiplicity) > Me.Gold) {
			buyall_highlight = font_switchto_red;
		}

		if (ItemMap[show_item->type].item_group_together_in_inventory) {
			autostr_append(desc->text, _("Price per unit: %s%d%s\n%s all: %s%d%s\n"),
			               buyone_highlight, price, font_switchto_neon,
			               action, buyall_highlight, price * show_item->multiplicity, font_switchto_neon);
		} else {
			autostr_append(desc->text, _("%s Price: %s%d%s\n"), action, buyone_highlight, price, font_switchto_neon);
		}


		if (show_item->current_durability == show_item->max_durability || show_item->max_durability == (-1))
			repair_price = 0;
		else
			repair_price = calculate_item_repair_price(show_item);

		if (ItemMap[show_item->type].base_item_durability != (-1)) {
			if (show_item->max_durability == (-1))
				autostr_append(desc->text, _("Indestructible\n"));
			else if (!buy)
				autostr_append(desc->text, _("Repair cost: %ld\n"), repair_price);
		}
	} else {
		autostr_append(desc->text, _("Unsellable\n"));
	}

	/* If the item is a weapon, then we print out some weapon stats. */
	if (info->item_gun_recharging_time > 0)
		autostr_append(desc->text, _("Recharge time: %3.2f\n"),
					   info->item_gun_recharging_time);

	if (info->item_gun_reloading_time > 0)
		autostr_append(desc->text, _("Time to reload ammo clip: %3.2f\n"),
					   info->item_gun_reloading_time);

	autostr_append(desc->text, _("Notes: %s"), D_(info->item_description));

	if (info->item_gun_use_ammunition)
		autostr_append(desc->text, _("\nThis weapon requires %s."),
					   _(ammo_desc_for_weapon(show_item->type)));
}

/**
 * This function does the item show when the user has selected item
 * show from the console menu.
 */
int GreatShopInterface(int NumberOfItems, item * ShowPointerList[MAX_ITEMS_IN_INVENTORY],
		       int NumberOfItemsInTuxRow, item * TuxItemsList[MAX_ITEMS_IN_INVENTORY], shop_decision * ShopOrder)
{
	int i;
	int ClickTarget;

	int RowLength = SHOP_ROW_LENGTH;
	static int RowStart = 0;
	static int ItemIndex = 0;

	int TuxRowLength = SHOP_ROW_LENGTH;
	static int TuxRowStart = 0;
	static int TuxItemIndex = -1;

	char GoldString[1000];
	SDL_Rect HighlightRect;
	int BuyButtonActive = FALSE;
	int SellButtonActive = FALSE;
	int ret = 0;
	int old_game_status = game_status;
	static struct widget_text item_description;
	const int scroll_to_top = -1000;

	game_status = INSIDE_MENU;

	// We add some security against indexing beyond the
	// range of items given in the list.
	if (NumberOfItems <= 0) {
		NumberOfItems = 0;
		RowLength = 0;
		RowStart = 0;
		ItemIndex = -1;
	} else {
		if (RowLength > NumberOfItems)
			RowLength = NumberOfItems;
		if (RowStart + RowLength > NumberOfItems)
			RowStart = NumberOfItems - RowLength;
		if (RowStart < 0)
			RowStart = 0;
		if (ItemIndex >= NumberOfItems)
			ItemIndex = NumberOfItems - 1;
	}

	if (NumberOfItemsInTuxRow <= 0) {
		NumberOfItemsInTuxRow = 0;
		TuxRowLength = 0;
		TuxRowStart = 0;
		TuxItemIndex = -1;
	} else {
		if (TuxRowLength > NumberOfItemsInTuxRow)
			TuxRowLength = NumberOfItemsInTuxRow;
		if (TuxRowStart + TuxRowLength > NumberOfItemsInTuxRow)
			TuxRowStart = NumberOfItemsInTuxRow - TuxRowLength;
		if (TuxRowStart < 0)
			TuxRowStart = 0;
		if (TuxItemIndex >= NumberOfItemsInTuxRow)
			TuxItemIndex = NumberOfItemsInTuxRow - 1;
	}

	/* Initialize the text widget. */
	widget_text_init(&item_description, "");
	widget_set_rect(WIDGET(&item_description), UNIVERSAL_COORD_W(258), UNIVERSAL_COORD_H(108), UNIVERSAL_COORD_W(346), UNIVERSAL_COORD_H(255));
	item_description.font = FPS_Display_BFont;
	item_description.line_height_factor = LINE_HEIGHT_FACTOR;
	item_description.scroll_offset = scroll_to_top;

	if (ItemIndex >= 0) {
		fill_item_description(&item_description, ShowPointerList[ItemIndex], 1);
		item_description.scroll_offset = scroll_to_top;
	} else if (TuxItemIndex >= 0) {
		fill_item_description(&item_description, TuxItemsList[TuxItemIndex], 0);
		item_description.scroll_offset = scroll_to_top;
	}

	while (1) {
		save_mouse_state();
		input_handle();

		SDL_Delay(1);
		ShopOrder->shop_command = DO_NOTHING;

		// We show all the info and the buttons that should be in this
		// interface...
		AssembleCombatPicture(ONLY_SHOW_MAP);
		SDL_SetClipRect(Screen, NULL);
		blit_background("item_browser_shop.png");

		/* This is a magic formula to place the item picture. */
		int x = 40 * GameConfig.screen_width / 1024 + ((250 * GameConfig.screen_width / 1024) - 132) / 2;
		int y = 185 * GameConfig.screen_height / 768 + ((322 * GameConfig.screen_height / 768) - 180) / 2;

		if (ItemIndex >= 0) {
			ShowItemPicture(x, y, ShowPointerList[ItemIndex]->type);
		} else if (TuxItemIndex >= 0) {
			ShowItemPicture(x, y, TuxItemsList[TuxItemIndex]->type);
		}

		widget_text_display(WIDGET(&item_description));

		for (i = 0; i < RowLength; i++) {
			ShowRescaledItem(i, FALSE, ShowPointerList[i + RowStart]);
		}

		for (i = 0; i < TuxRowLength; i++) {
			ShowRescaledItem(i, TRUE, TuxItemsList[i + TuxRowStart]);
		}

		/* Highlight the currently selected item. */
		if (ItemIndex >= 0) {
			HighlightRect.x =
			    (ShopItemRowRect.x + (ItemIndex - RowStart) * INITIAL_BLOCK_WIDTH * GameConfig.screen_width / 640);
			HighlightRect.y = ShopItemRowRect.y;
			HighlightRect.w = INITIAL_BLOCK_WIDTH * GameConfig.screen_width / 640;
			HighlightRect.h = INITIAL_BLOCK_HEIGHT * GameConfig.screen_height / 480;
			HighlightRectangle(Screen, HighlightRect);
		}
		if (TuxItemIndex >= 0) {
			HighlightRect.x =
			    (TuxItemRowRect.x + (TuxItemIndex - TuxRowStart) * INITIAL_BLOCK_WIDTH * GameConfig.screen_width / 640);
			HighlightRect.y = TuxItemRowRect.y;
			HighlightRect.w = INITIAL_BLOCK_WIDTH * GameConfig.screen_width / 640;
			HighlightRect.h = INITIAL_BLOCK_HEIGHT * GameConfig.screen_height / 480;
			HighlightRectangle(Screen, HighlightRect);
		}

		if (ItemIndex >= 0) {
			ShowGenericButtonFromList(BUY_BUTTON);
			BuyButtonActive = TRUE;
			SellButtonActive = FALSE;
		} else if (TuxItemIndex >= 0) {
			SellButtonActive = FALSE;
			if (calculate_item_sell_price(TuxItemsList[TuxItemIndex])) {
				ShowGenericButtonFromList(SELL_BUTTON);
				SellButtonActive = TRUE;
			}
			BuyButtonActive = FALSE;

			if ((ItemMap[TuxItemsList[TuxItemIndex]->type].base_item_durability >= 0) &&
			    (TuxItemsList[TuxItemIndex]->max_durability > TuxItemsList[TuxItemIndex]->current_durability))
				ShowGenericButtonFromList(REPAIR_BUTTON);
		} else {
			BuyButtonActive = FALSE;
			SellButtonActive = FALSE;
		}

		/* Show the amount of 'Valuable Circuits' Tux has. */
		sprintf(GoldString, "%6d", (int)Me.Gold);
		put_string(FPS_Display_BFont, 40 * GameConfig.screen_width / 640 - 15,
			      370 * GameConfig.screen_height / 480, GoldString);

		blit_mouse_cursor();
		our_SDL_flip_wrapper();


		if (MouseLeftClicked()) {
			if (MouseCursorIsOnButton(DESCRIPTION_WINDOW_UP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				MoveMenuPositionSound();
				item_description.scroll_offset--;
			} else if (MouseCursorIsOnButton(DESCRIPTION_WINDOW_DOWN_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				MoveMenuPositionSound();
				item_description.scroll_offset++;
			} else if (MouseCursorIsOnButton(ITEM_BROWSER_EXIT_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				while (MouseLeftPressed())
					SDL_Delay(1);
				ret = -1;
				goto out;
			} else if (MouseCursorIsOnButton(LEFT_TUX_SHOP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (0 < RowStart) {
					RowStart--;
					if (ItemIndex != -1) {
						if (ItemIndex >= RowStart + RowLength)
							ItemIndex--;
						fill_item_description(&item_description, ShowPointerList[ItemIndex], 1);
						item_description.scroll_offset = scroll_to_top;
					}
				}
				MoveMenuPositionSound();
			} else if (MouseCursorIsOnButton(RIGHT_TUX_SHOP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (RowStart + RowLength < NumberOfItems) {
					RowStart++;
					if (ItemIndex != -1) {
						if (ItemIndex < RowStart)
							ItemIndex++;
						fill_item_description(&item_description, ShowPointerList[ItemIndex], 1);
						item_description.scroll_offset = scroll_to_top;
					}
				}
				MoveMenuPositionSound();
			} else if (MouseCursorIsOnButton(LEFT_SHOP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (0 < TuxRowStart) {
					TuxRowStart--;
					if (TuxItemIndex != -1) {
						if (TuxItemIndex >= TuxRowStart + TuxRowLength)
							TuxItemIndex--;
						fill_item_description(&item_description, TuxItemsList[TuxItemIndex], 0);
						item_description.scroll_offset = scroll_to_top;
					}
				}
				MoveMenuPositionSound();
			} else if (MouseCursorIsOnButton(RIGHT_SHOP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (TuxRowStart + TuxRowLength < NumberOfItemsInTuxRow) {
					TuxRowStart++;
					if (TuxItemIndex != -1) {
						if (TuxItemIndex < TuxRowStart)
							TuxItemIndex++;
						fill_item_description(&item_description, TuxItemsList[TuxItemIndex], 0);
						item_description.scroll_offset = scroll_to_top;
					}
				}
				MoveMenuPositionSound();
			} else if (((ClickTarget = ClickWasOntoItemRowPosition(GetMousePos_x(), GetMousePos_y(), FALSE)) >= 0)) {
				if (ClickTarget < RowLength) {
					ItemIndex = RowStart + ClickTarget;
					TuxItemIndex = (-1);
					fill_item_description(&item_description, ShowPointerList[ItemIndex], 1);
					item_description.scroll_offset = scroll_to_top;
				}
			} else if (((ClickTarget = ClickWasOntoItemRowPosition(GetMousePos_x(), GetMousePos_y(), TRUE)) >= 0)) {
				if (ClickTarget < TuxRowLength) {
					TuxItemIndex = TuxRowStart + ClickTarget;
					ItemIndex = (-1);
					fill_item_description(&item_description, TuxItemsList[TuxItemIndex], 0);
					item_description.scroll_offset = scroll_to_top;
				}
			} else if (MouseCursorIsOnButton(BUY_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (BuyButtonActive && ItemIndex != -1) {
					ShopOrder->item_selected = ItemIndex;
					ShopOrder->shop_command = BUY_1_ITEM;
					int afford = Me.Gold / ItemMap[ShowPointerList[ItemIndex]->type].base_list_price;
					if ((ItemMap[ShowPointerList[ItemIndex]->type].item_group_together_in_inventory) && (afford >= 1)) {
						ShopOrder->number_selected =
							do_graphical_number_selection_in_range(0,
									(ShowPointerList[ItemIndex]->multiplicity <= afford) ?
										ShowPointerList[ItemIndex]->multiplicity : afford,
									1, calculate_item_buy_price(ShowPointerList[ItemIndex]));
					} else {
						ShopOrder->number_selected = 1;
					}
					ret = 0;
					goto out;
				}
			} else if (MouseCursorIsOnButton(SELL_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (SellButtonActive && TuxItemIndex != -1) {
					ShopOrder->item_selected = TuxItemIndex;
					ShopOrder->shop_command = SELL_1_ITEM;

					if ((ItemMap[TuxItemsList[TuxItemIndex]->type].item_group_together_in_inventory) &&
					    (TuxItemsList[TuxItemIndex]->multiplicity > 1)) {
						ShopOrder->number_selected =
						    do_graphical_number_selection_in_range(0, TuxItemsList[TuxItemIndex]->multiplicity,
											   TuxItemsList[TuxItemIndex]->multiplicity,
											calculate_item_sell_price(TuxItemsList[TuxItemIndex]));
					} else
						ShopOrder->number_selected = 1;
					ret = 0;
					goto out;
				}
			} else if (MouseCursorIsOnButton(REPAIR_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				// Reference to the Tux item list must only be made, when the 'highlight'
				// is really in the tux item row.  Otherwise we just get a segfault...
				//
				if (TuxItemIndex != -1) {
					// Of course the repair button should only have effect, if there is
					// really something to repair (and therefore the button is shown at
					// all further above.
					//
					if ((ItemMap[TuxItemsList[TuxItemIndex]->type].base_item_durability >= 0) &&
					    (TuxItemsList[TuxItemIndex]->max_durability > TuxItemsList[TuxItemIndex]->current_durability)) {
						ShopOrder->item_selected = TuxItemIndex;
						ShopOrder->shop_command = REPAIR_ITEM;
						ShopOrder->number_selected = 1;
						ret = 0;
						goto out;
					}
				}
			}
		}

		if (UpPressed() || MouseWheelUpPressed()) {
			MoveMenuPositionSound();
			while (UpPressed()) ;
			item_description.scroll_offset--;
		}
		if (DownPressed() || MouseWheelDownPressed()) {
			MoveMenuPositionSound();
			while (DownPressed()) ;
			item_description.scroll_offset++;
		}

		if (EscapePressed()) {
			while (EscapePressed()) ;
			ret = -1;
			goto out;
		}

	}

 out:
	game_status = old_game_status;
	return ret;
}

/**
 * This function repairs the item given as parameter.
 */
static void repair_item(item * RepairItem)
{
	while (SpacePressed() || EnterPressed() || MouseLeftPressed())
		SDL_Delay(1);

	if (calculate_item_repair_price(RepairItem) > Me.Gold) {
		alert_window("%s\n\n%s", D_(item_specs_get_name(RepairItem->type)), _("You can not afford to have this item repaired."));
		return;
	}

	Me.Gold -= calculate_item_repair_price(RepairItem);
	RepairItem->current_durability = RepairItem->max_durability;
	play_sound("effects/Shop_ItemRepairedSound_0.ogg");
}

/**
 * This function tries to sell the item given as parameter.
 */
static void TryToSellItem(item * SellItem, int AmountToSellAtMost)
{
	// We catch the case, that not even one item was selected
	// for buying in the number selector...
	//
	if (AmountToSellAtMost <= 0) {
		DebugPrintf(0, "\nTried to sell 0 items of a kind... doing nothing... ");
		return;
	}
	// First some error-checking against illegal values.  This should not normally
	// occur, but some items on the map are from very old times and therefore the
	// engine might have made some mistakes back then or also changes that broke these
	// items, so some extra care will be taken here...
	//
	if (SellItem->multiplicity < 1) {
		ErrorMessage(__FUNCTION__, "\
		An item sold seemed to have multiplicity < 1.  This might be due to some\n\
		fatal errors in the engine OR it might be due to some items dropped on the\n\
		maps somewhere long ago still had multiplicity=0 setting, which should not\n\
		normally occur with 'freshly' generated items.  Well, that's some dust from\n\
		the past, but now it should be fixed and not occur in future releases (0.9.10\n\
		    or later) of the game.  If you encounter this message after release 0.9.10,\n\
		please inform the developers...", PLEASE_INFORM, IS_WARNING_ONLY);
	}

	if (AmountToSellAtMost > SellItem->multiplicity)
		AmountToSellAtMost = SellItem->multiplicity;

	while (SpacePressed() || EnterPressed()) ;

	// Ok.  Here we silently sell the item.
	//
	Me.Gold += calculate_item_sell_price(SellItem) * AmountToSellAtMost;
	if (AmountToSellAtMost < SellItem->multiplicity)
		SellItem->multiplicity -= AmountToSellAtMost;
	else
		DeleteItem(SellItem);

	play_sound("effects/Shop_ItemSoldSound_0.ogg");
}

/**
 * This function tries to buy the item given as parameter.
 * Returns -1 if buying failed, 0 if buying was possible, and 1 if all items were sold.
 */
static int buy_item(item *BuyItem, int amount)
{
	float item_price;
	item new_item;

	if (amount <= 0) {
		return -1;
	}

	CopyItem(BuyItem, &new_item);
	
	if (BuyItem->multiplicity < amount)
		amount = BuyItem->multiplicity;
		
	new_item.multiplicity = amount;
	item_price = calculate_item_buy_price(&new_item) * new_item.multiplicity;

	// If the item is too expensive, bail out
	if (item_price > Me.Gold) {
		alert_window("%s\n\n%s", D_(item_specs_get_name(BuyItem->type)), _("You can not afford this item."));
		return -1;
	}

	// Subtract money, give item, play sound.
	Me.Gold -= item_price;
	give_item(&new_item);
	play_sound("effects/Shop_ItemBoughtSound_0.ogg");
	
	// Did player want all of the items?
	if(BuyItem->multiplicity == amount) {
		return 1;
	} else {
		BuyItem->multiplicity -= amount;
	}
	
	return 0;

}

/**
 * This is some preparation for the shop interface.  We assemble some
 * pointer list with the stuff Tux has to sell and the stuff the shop
 * has to offer.
 *
 *
 */
void InitTradeWithCharacter(struct npc *npc)
{
#define NUMBER_OF_ITEMS_IN_SHOP 17

	item *BuyPointerList[MAX_ITEMS_IN_INVENTORY];
	item *TuxItemsList[MAX_ITEMS_IN_INVENTORY];
	int i;
	int ItemSelected = 0;
	shop_decision ShopOrder;
	int NumberOfItemsInTuxRow = 0;
	struct dynarray *sold_items;

	sold_items = npc_get_inventory(npc);

	for (i = 0; i < sold_items->size && i < sizeof(BuyPointerList)/sizeof(BuyPointerList[0]); i++) {
			BuyPointerList[i] = &((item *)(sold_items->arr))[i];
	}

	// Now here comes the new thing:  This will be a loop from now
	// on.  The buy and buy and buy until at one point we say 'BACK'
	//
	while (ItemSelected != (-1)) {

		NumberOfItemsInTuxRow = AssemblePointerListForItemShow(&(TuxItemsList[0]), TRUE);

		ItemSelected = GreatShopInterface(sold_items->size, BuyPointerList, NumberOfItemsInTuxRow, TuxItemsList, &(ShopOrder));
		switch (ShopOrder.shop_command) {
		case BUY_1_ITEM:
			if (buy_item(BuyPointerList[ShopOrder.item_selected], ShopOrder.number_selected) == 1) {
				// destroy our copy of the item
				npc_inventory_delete_item(npc, ShopOrder.item_selected);
			}
			break;
		case SELL_1_ITEM:
			TryToSellItem(TuxItemsList[ShopOrder.item_selected], ShopOrder.number_selected);
			break;
		case REPAIR_ITEM:
			repair_item(TuxItemsList[ShopOrder.item_selected]);
			break;
		default:

			break;
		};

		for (i = 0; i < sold_items->size && i < sizeof(BuyPointerList)/sizeof(BuyPointerList[0]); i++) {
			BuyPointerList[i] = &((item *)(sold_items->arr))[i];
		}
	}
}

#undef _shop_c
