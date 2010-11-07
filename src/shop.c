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
#include "SDL_rotozoom.h"

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
int ClickWasOntoItemRowPosition(int x, int y, int TuxItemRow)
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
	static iso_image equipped_icon;

	if (!iso_image_loaded(&equipped_icon)) {
		load_iso_image(&equipped_icon, "mouse_cursor_0003.png", FALSE);
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
	our_SDL_blit_surface_wrapper(ItemMap[ShowItem->type].inv_image.shop_iso_image.surface, NULL, Screen, &TargetRectangle);
	if (item_is_currently_equipped(ShowItem)) {
		if (use_open_gl) {
			draw_gl_textured_quad_at_screen_position(&equipped_icon, TargetRectangle.x + TargetRectangle.w - 24,
								 TargetRectangle.y);
		} else {
			blit_iso_image_to_screen_position(&equipped_icon, TargetRectangle.x + TargetRectangle.w - 24, TargetRectangle.y);
		}
	}
};				// void ShowRescaledItem ( int position , item* ShowItem )

/**
 * This function displays an item picture. 
 */
void ShowItemPicture(int PosX, int PosY, int Number)
{
	SDL_Surface *tmp;
	SDL_Rect target;
	char ConstructedFileName[2048];
	char fpath[2048];
	static char LastImageSeriesPrefix[1000] = "NONE_AT_ALL";
	static int NumberOfImagesInPreviousRotation = 0;
	static int NumberOfImagesInThisRotation = 0;
#define MAX_NUMBER_OF_IMAGES_IN_ITEM_ROTATION 64
	static SDL_Surface *ItemRotationSurfaces[MAX_NUMBER_OF_IMAGES_IN_ITEM_ROTATION] = { NULL };
	SDL_Surface *Whole_Image;
	int i;
	int RotationIndex;

	// DebugPrintf (2, "\nvoid ShowItemPicture(...): Function call confirmed.");

	// if ( !strcmp ( ItemMap[ Number ] . item_rotation_series_prefix , "NONE_AVAILABLE_YET" ) )
	// return; // later this should be a default-correction instead

	// Maybe we have to reload the whole image series
	//
	if (strcmp(LastImageSeriesPrefix, ItemMap[Number].item_rotation_series_prefix)) {
		// Maybe we have to free the series from an old item display first
		//
		if (ItemRotationSurfaces[0] != NULL) {
			for (i = 0; i < NumberOfImagesInPreviousRotation; i++) {
				SDL_FreeSurface(ItemRotationSurfaces[i]);
			}
		}
		// Now we can start to load the whole series into memory
		//
		for (i = 0; i < MAX_NUMBER_OF_IMAGES_IN_ITEM_ROTATION; i++) {
			// At first we will try to find some item rotation models in the
			// new directory structure.
			//
			sprintf(ConstructedFileName, "items/%s/portrait_%04d.jpg", ItemMap[Number].item_rotation_series_prefix, i + 1);
			if (find_file(ConstructedFileName, GRAPHICS_DIR, fpath, 1))
				Whole_Image = NULL;
			else
				Whole_Image = our_IMG_load_wrapper(fpath);	// This is a surface with alpha channel, since the picture is one of this type

			// If that didn't work, then it's time to try the same directory with 'png' ending...
			// Maybe there's still some (old) rotation image of this kind.
			//
			if (Whole_Image == NULL) {
				DebugPrintf(1, "\nNo luck trying to load .jpg item image series from the 'bastian' dir... trying png...");
				sprintf(ConstructedFileName, "items/%s/portrait_%04d.png", ItemMap[Number].item_rotation_series_prefix,
					i + 1);
				if (find_file(ConstructedFileName, GRAPHICS_DIR, fpath, 1))
					Whole_Image = NULL;
				else
					Whole_Image = our_IMG_load_wrapper(fpath);	// This is a surface with alpha channel, since the picture is one of this type
			}
			// But at this point, we should have found the image!!
			// or if not, this maybe indicates that we have reached the
			// last image in the image series...
			//
			if (Whole_Image == NULL) {
				NumberOfImagesInThisRotation = i;
				NumberOfImagesInPreviousRotation = NumberOfImagesInThisRotation;
				DebugPrintf(1, "\nDONE LOADING ITEM IMAGE SERIES.  Loaded %d images into memory.",
					    NumberOfImagesInThisRotation);

				// Maybe we've received the nothing loaded case even on the first attempt
				// to load something.  This of course would mean a severe error in Freedroid!
				//
				if (NumberOfImagesInThisRotation <= 0) {
					fprintf(stderr, "\n\nfpath: %s. \n", fpath);
					ErrorMessage(__FUNCTION__, "\
Freedroid was unable to load even one image of a rotated item image series into memory.\n\
This error indicates some installation problem with freedroid.", PLEASE_INFORM, IS_FATAL);
				}

				break;
			}
			// Also we must check for our upper bound of the list of 
			// item images.  This will most likely never be exceeded, but it
			// can hurt to just be on the safe side.
			//
			if (i >= MAX_NUMBER_OF_IMAGES_IN_ITEM_ROTATION - 2) {
				fprintf(stderr, "\n\nfpath: %s. \n", fpath);
				ErrorMessage(__FUNCTION__, "\
Freedroid was encountered more item images in an item rotation image series\n\
than it is able to handle.  This is a very strange error.  Someone has been\n\
trying to make the ultra-fine item rotation series.  Strange.", PLEASE_INFORM, IS_FATAL);
			}

			SDL_SetAlpha(Whole_Image, 0, SDL_ALPHA_OPAQUE);
			ItemRotationSurfaces[i] = SDL_CreateRGBSurface(0, Whole_Image->w, Whole_Image->h, 32, rmask, gmask, bmask, amask);
			SDL_BlitSurface(Whole_Image, NULL, ItemRotationSurfaces[i], NULL);
			SDL_FreeSurface(Whole_Image);

			// We must remember, that his is already loaded of course
			strcpy(LastImageSeriesPrefix, ItemMap[Number].item_rotation_series_prefix);

		}

	}

	RotationIndex = (SDL_GetTicks() / 70);

	RotationIndex = RotationIndex - (RotationIndex / NumberOfImagesInThisRotation) * NumberOfImagesInThisRotation;

	tmp = ItemRotationSurfaces[RotationIndex];

	SDL_SetClipRect(Screen, NULL);
	Set_Rect(target, PosX, PosY, GameConfig.screen_width, GameConfig.screen_height);
	our_SDL_blit_surface_wrapper(tmp, NULL, Screen, &target);
}

/**
 * Assemble item description.
 */
static void fill_item_description(text_widget *desc, item *show_item)
{
	char TextChunk[2000];
	char *class_string;
	long int repair_price = 0;
	itemspec *info;

	if (show_item == NULL)
		return;

	if (MatchItemWithName(show_item->type, "Valuable Circuits"))
		ErrorMessage(__FUNCTION__, "Valuable Circuits in shop.", PLEASE_INFORM, IS_WARNING_ONLY);

	info = &ItemMap[show_item->type];

	init_text_widget(desc, "");

	if (info->item_can_be_installed_in_weapon_slot)
		class_string = _("Weapon");
	else if (info->item_can_be_installed_in_drive_slot)
		class_string = _("Drive");
	else if (info->item_can_be_installed_in_armour_slot)
		class_string = _("Armor");
	else if (info->item_can_be_installed_in_shield_slot)
		class_string = _("Shield");
	else if (info->item_can_be_installed_in_special_slot)
		class_string = _("Helm");
	else
		class_string = _("Miscellaneous");

	write_full_item_name_into_string(show_item, TextChunk);
	autostr_printf(desc->text, _("Item: %s \nClass: %s\n"), TextChunk, class_string);

	// Append item bonuses.
	struct auto_string *bonuses = alloc_autostr(128);
	get_item_bonus_string(show_item, ", ", bonuses);
	if (bonuses->length)
		autostr_append(desc->text, "%s%s%s%s\n", _("Specials: "), font_switchto_red,
					   bonuses->value, font_switchto_neon);
	free_autostr(bonuses);

	if (info->item_group_together_in_inventory)
		autostr_append(desc->text, _("Multiplicity: %d\n"), (int)show_item->multiplicity);

	autostr_append(desc->text, _("Durability: "));
	if (show_item->max_duration >= 0)
		autostr_append(desc->text, "%d / %d\n", (int)show_item->current_duration, show_item->max_duration);
	else
		autostr_append(desc->text, _("Indestructible\n"));

	if (!info->item_can_be_applied_in_combat) {
		autostr_append(desc->text, _("Attributes required: "));

		if ((info->item_require_strength <= 0) &&
			(info->item_require_dexterity <= 0) && (info->item_require_magic <= 0)) {
			autostr_append(desc->text, _("NONE\n"));
		} else {
			if (info->item_require_strength > 0)
				autostr_append(desc->text, _("Str: %d "), info->item_require_strength);
			if (info->item_require_dexterity > 0)
				autostr_append(desc->text, _("Dex: %d "), info->item_require_dexterity);
			if (info->item_require_magic > 0)
				autostr_append(desc->text, _("Mag: %d "), info->item_require_magic);
			autostr_append(desc->text, "\n");
		}
	}
	// Now we give some pricing information, the base list price for the item,
	// the repair price and the sell value
	if (calculate_item_buy_price(show_item)) {
		autostr_append(desc->text, _("Base list price: %ld\n"), calculate_item_buy_price(show_item));
		autostr_append(desc->text, _("Sell value: %ld\n"), calculate_item_sell_price(show_item));
		if (show_item->current_duration == show_item->max_duration || show_item->max_duration == (-1))
			repair_price = 0;
		else
			repair_price = calculate_item_repair_price(show_item);
		if (show_item->max_duration == (-1))
			autostr_append(desc->text, _("Indestructible\n"));
		else
			autostr_append(desc->text, _("Repair cost: %ld\n"), repair_price);
	} else {
		autostr_append(desc->text, _("Unsellable\n"));
	}

	/* If the item is a weapon, then we print out some weapon stats. */
	if (info->base_item_gun_damage + info->item_gun_damage_modifier > 0) {
		autostr_append(desc->text, _("Damage: %d - %d\n"),
					   info->base_item_gun_damage,
					   info->base_item_gun_damage + info->item_gun_damage_modifier);
	}

	if (info->item_gun_recharging_time > 0)
		autostr_append(desc->text, _("Recharge time: %3.2f\n"),
					   info->item_gun_recharging_time);

	if (info->item_gun_reloading_time > 0)
		autostr_append(desc->text, _("Time to reload ammo clip: %3.2f\n"),
					   info->item_gun_reloading_time);

	if (show_item->armor_class + show_item->bonus_to_armor_class > 0)
		autostr_append(desc->text, _("Armor class: %d\n"), show_item->armor_class + show_item->bonus_to_armor_class);

	autostr_append(desc->text, _("Notes: %s"), D_(info->item_description));

	if (info->item_gun_use_ammunition)
		autostr_append(desc->text, _(" This weapon requires %s."),
					   ammo_desc_for_weapon(show_item->type));
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
	static int RowStart = 0;
	static int TuxRowStart = 0;
	static int ItemIndex = 0;
	static int TuxItemIndex = -1;
	int RowLength = SHOP_ROW_LENGTH;
	int TuxRowLength = SHOP_ROW_LENGTH;
	char GoldString[1000];
	SDL_Rect HighlightRect;
	int BuyButtonActive = FALSE;
	int SellButtonActive = FALSE;
	int ret = 0;
	int old_game_status = game_status;
	static text_widget item_description;
	game_status = INSIDE_MENU;

	// For the shop, we'll also try to use our own mouse cursor
	//
	make_sure_system_mouse_cursor_is_turned_off();

	// We add some security against indexing beyond the
	// range of items given in the list.
	if (RowLength > NumberOfItems)
		RowLength = NumberOfItems;
	while (ItemIndex >= NumberOfItems)
		ItemIndex--;
	while (RowStart + RowLength > NumberOfItems)
		RowStart--;
	if (RowStart < 0)
		RowStart = 0;

	if (TuxRowLength > NumberOfItemsInTuxRow)
		TuxRowLength = NumberOfItemsInTuxRow;
	while (TuxItemIndex >= NumberOfItemsInTuxRow)
		TuxItemIndex--;
	while (TuxRowStart + TuxRowLength > NumberOfItemsInTuxRow)
		TuxRowStart--;
	if (TuxRowStart < 0)
		TuxRowStart = 0;

	if (NumberOfItemsInTuxRow <= 0)
		TuxItemIndex = (-1);
	if (NumberOfItems <= 0)
		ItemIndex = (-1);

	/* Initialize the text widget. */
	init_text_widget(&item_description, "");
	item_description.rect.x = UNIVERSAL_COORD_W(258);
	item_description.rect.y = UNIVERSAL_COORD_H(108);
	item_description.rect.w = UNIVERSAL_COORD_W(346);
	item_description.rect.h = UNIVERSAL_COORD_H(255);
	item_description.font = FPS_Display_BFont;
	item_description.text_stretch = TEXT_STRETCH;

	while (1) {
		StartTakingTimeForFPSCalculation();
		save_mouse_state();
		input_handle();

		SDL_Delay(1);
		ShopOrder->shop_command = DO_NOTHING;

		// We show all the info and the buttons that should be in this
		// interface...
		AssembleCombatPicture(USE_OWN_MOUSE_CURSOR | ONLY_SHOW_MAP);
		SDL_SetClipRect(Screen, NULL);
		blit_special_background(ITEM_BROWSER_SHOP_BACKGROUND_CODE);

		/* This is a magic formula to place the item picture. */
		int x = 40 * GameConfig.screen_width / 1024 + ((250 * GameConfig.screen_width / 1024) - 132) / 2;
		int y = 185 * GameConfig.screen_height / 768 + ((322 * GameConfig.screen_height / 768) - 180) / 2;

		if (ItemIndex >= 0) {
			ShowItemPicture(x, y, ShowPointerList[ItemIndex]->type);
			fill_item_description(&item_description, ShowPointerList[ItemIndex]);
		} else if (TuxItemIndex >= 0) {
			ShowItemPicture(x, y, TuxItemsList[TuxItemIndex]->type);
			fill_item_description(&item_description, TuxItemsList[TuxItemIndex]);
		}
		show_text_widget(&item_description);

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

			if ((ItemMap[TuxItemsList[TuxItemIndex]->type].base_item_duration >= 0) &&
			    (TuxItemsList[TuxItemIndex]->max_duration > TuxItemsList[TuxItemIndex]->current_duration))
				ShowGenericButtonFromList(REPAIR_BUTTON);
		} else {
			BuyButtonActive = FALSE;
			SellButtonActive = FALSE;
		}

		/* Show the amount of 'Valuable Circuits' Tux has. */
		sprintf(GoldString, "%6d", (int)Me.Gold);
		PutStringFont(Screen, FPS_Display_BFont, 40 * GameConfig.screen_width / 640 - 15,
			      370 * GameConfig.screen_height / 480, GoldString);

		blit_our_own_mouse_cursor();
		our_SDL_flip_wrapper();

		const int scroll_to_top = -1000;

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
					if (ItemIndex != -1 && ItemIndex >= RowStart + RowLength)
						ItemIndex--;
				}
				item_description.scroll_offset = scroll_to_top;
				MoveMenuPositionSound();
			} else if (MouseCursorIsOnButton(RIGHT_TUX_SHOP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (RowStart + RowLength < NumberOfItems) {
					RowStart++;
					if (ItemIndex != -1 && ItemIndex < RowStart)
						ItemIndex++;
				}
				item_description.scroll_offset = scroll_to_top;
				MoveMenuPositionSound();
			} else if (MouseCursorIsOnButton(LEFT_SHOP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (0 < TuxRowStart) {
					TuxRowStart--;
					if (TuxItemIndex != -1 && TuxItemIndex >= TuxRowStart + TuxRowLength)
						TuxItemIndex--;
				}
				item_description.scroll_offset = scroll_to_top;
				MoveMenuPositionSound();
			} else if (MouseCursorIsOnButton(RIGHT_SHOP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (TuxRowStart + TuxRowLength < NumberOfItemsInTuxRow) {
					TuxRowStart++;
					if (TuxItemIndex != -1 && TuxItemIndex < TuxRowStart)
						TuxItemIndex++;
				}
				item_description.scroll_offset = scroll_to_top;
				MoveMenuPositionSound();
			} else if (((ClickTarget = ClickWasOntoItemRowPosition(GetMousePos_x(), GetMousePos_y(), FALSE)) >= 0)) {
				if (RowStart + ClickTarget < NumberOfItems) {
					ItemIndex = RowStart + ClickTarget;
					TuxItemIndex = (-1);
					item_description.scroll_offset = scroll_to_top;
				}
			} else if (((ClickTarget = ClickWasOntoItemRowPosition(GetMousePos_x(), GetMousePos_y(), TRUE)) >= 0)) {
				if (TuxRowStart + ClickTarget < NumberOfItemsInTuxRow) {
					TuxItemIndex = TuxRowStart + ClickTarget;
					ItemIndex = (-1);
					item_description.scroll_offset = scroll_to_top;
				}
			} else if (MouseCursorIsOnButton(BUY_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (BuyButtonActive) {
					ShopOrder->item_selected = ItemIndex;
					ShopOrder->shop_command = BUY_1_ITEM;
					if ((ItemMap[ShowPointerList[ItemIndex]->type].item_group_together_in_inventory) &&
					    (Me.Gold / ItemMap[ShowPointerList[ItemIndex]->type].base_list_price >= 1)) {
						ShopOrder->number_selected =
						    do_graphical_number_selection_in_range(0,
											   Me.Gold /
											   ItemMap[ShowPointerList[ItemIndex]->type].
											   base_list_price, 1);
					} else {
						ShopOrder->number_selected = 1;
					}
					ret = 0;
					goto out;
				}
			} else if (MouseCursorIsOnButton(SELL_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (SellButtonActive) {
					ShopOrder->item_selected = TuxItemIndex;
					ShopOrder->shop_command = SELL_1_ITEM;

					if ((ItemMap[TuxItemsList[TuxItemIndex]->type].item_group_together_in_inventory) &&
					    (TuxItemsList[TuxItemIndex]->multiplicity > 1)) {
						ShopOrder->number_selected =
						    do_graphical_number_selection_in_range(0, TuxItemsList[TuxItemIndex]->multiplicity,
											   TuxItemsList[TuxItemIndex]->multiplicity);
					} else
						ShopOrder->number_selected = 1;
					ret = 0;
					goto out;
				}
			} else if (MouseCursorIsOnButton(REPAIR_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				// Reference to the Tux item list must only be made, when the 'highlight'
				// is really in the tux item row.  Otherwise we just get a segfault...
				//
				if (TuxItemIndex > (-1)) {
					// Of course the repair button should only have effect, if there is
					// really something to repair (and therefore the button is shown at
					// all further above.
					//
					if ((ItemMap[TuxItemsList[TuxItemIndex]->type].base_item_duration >= 0) &&
					    (TuxItemsList[TuxItemIndex]->max_duration > TuxItemsList[TuxItemIndex]->current_duration)) {
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

		// Limit framerate if configured to do so.
		limit_fps();
		ComputeFPSForThisFrame();

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
 * This function tells us which item in the menu has been clicked upon.
 * It does not check for lower than 4 items in the menu available.
 */
int ClickedMenuItemPosition(void)
{
	int CursorY;
	int i;

	CursorY = GetMousePos_y();	// this is already the position corrected for 16 pixels!!

#define ITEM_MENU_DISTANCE 80
#define ITEM_FIRST_POS_Y 130
#define NUMBER_OF_ITEMS_ON_ONE_SCREEN 4

	// When a character is blitted to the screen at x y, then the x and y
	// refer to the top left corner of the coming blit.  Using this information
	// we will define the areas where a click 'on the blitted text' has occurred
	// or not.
	//
	if (CursorY < ITEM_FIRST_POS_Y)
		return (-1);
	if (CursorY > ITEM_FIRST_POS_Y + NUMBER_OF_ITEMS_ON_ONE_SCREEN * ITEM_MENU_DISTANCE)
		return (-1);

	for (i = 0; i < NUMBER_OF_ITEMS_ON_ONE_SCREEN; i++) {
		if (CursorY < ITEM_FIRST_POS_Y + (i + 1) * ITEM_MENU_DISTANCE)
			return i;
	}

	// At this point we've already determined and returned to right click-area.
	// if this point is ever reached, a severe error has occurred, and Freedroid
	// should therefore also say so.
	//
	ErrorMessage(__FUNCTION__, "\
The MENU CODE was unable to properly resolve a mouse button press.", PLEASE_INFORM, IS_FATAL);

	return (3);		// to make compilers happy :)

};				// int ClickedMenuItemPosition( void )

/**
 * This function repairs the item given as parameter.
 */
void TryToRepairItem(item * RepairItem)
{
	char *MenuTexts[10];
	MenuTexts[0] = _("Yes");
	MenuTexts[1] = _("No");
	MenuTexts[2] = "";

	while (SpacePressed() || EnterPressed() || MouseLeftPressed())
		SDL_Delay(1);

	if (calculate_item_repair_price(RepairItem) > Me.Gold) {
		MenuTexts[0] = _(" BACK ");
		MenuTexts[1] = "";
		SetCurrentFont(Menu_BFont);
		DoMenuSelection(_("\n\nYou can't afford to have this item repaired! "), MenuTexts, 1, -1, NULL);
		return;
	}

	Me.Gold -= calculate_item_repair_price(RepairItem);
	RepairItem->current_duration = RepairItem->max_duration;
	PlayOnceNeededSoundSample("effects/Shop_ItemRepairedSound_0.ogg", FALSE, FALSE);
};				// void TryToRepairItem( item* RepairItem )

/**
 * This function tries to sell the item given as parameter.
 */
void TryToSellItem(item * SellItem, int AmountToSellAtMost)
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
		fatal errors in the engine OR it might be due to some items droped on the\n\
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
	Me.Gold += calculate_item_sell_price(SellItem) * ((float)AmountToSellAtMost) / ((float)SellItem->multiplicity);
	if (AmountToSellAtMost < SellItem->multiplicity)
		SellItem->multiplicity -= AmountToSellAtMost;
	else
		DeleteItem(SellItem);

	PlayOnceNeededSoundSample("effects/Shop_ItemSoldSound_0.ogg", FALSE, TRUE);
};				// void TryToSellItem( item* SellItem )

/**
 * This function tries to buy the item given as parameter.
 * Returns 0 if buying was possible, 1 otherwise.
 */
static int buy_item(item *BuyItem, int amount)
{
	float item_price;

	if (amount <= 0) {
		return 1;
	}

	BuyItem->multiplicity = amount;

	item_price = calculate_item_buy_price(BuyItem);

	// If the item is too expensive, bail out
	if (item_price > Me.Gold) {
		char linebuf[1000];
		sprintf(linebuf, _("%s\n\nYou can't afford to purchase this item!"),
		        ItemMap[BuyItem->type].item_name);
		alert_window(linebuf);
		return 1;
	}

	// Give the item to the player and subtract money.
	give_item(BuyItem);
	Me.Gold -= item_price;
	PlayOnceNeededSoundSample("effects/Shop_ItemBoughtSound_0.ogg", FALSE, FALSE);

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

	item *SalesList;
	item *BuyPointerList[MAX_ITEMS_IN_INVENTORY];
	item *TuxItemsList[MAX_ITEMS_IN_INVENTORY];
	int i;
	int ItemSelected = 0;
	shop_decision ShopOrder;
	int NumberOfItemsInTuxRow = 0;
	int NumberOfItemsInShop = 0;

	SalesList = npc_get_inventory(npc);

	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		if (SalesList[i].type == (-1))
			BuyPointerList[i] = NULL;
		else
			BuyPointerList[i] = &(SalesList[i]);
	}
	// Now here comes the new thing:  This will be a loop from now
	// on.  The buy and buy and buy until at one point we say 'BACK'
	//
	while (ItemSelected != (-1)) {

		NumberOfItemsInTuxRow = AssemblePointerListForItemShow(&(TuxItemsList[0]), TRUE);

		for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
			if (BuyPointerList[i] == NULL) {
				NumberOfItemsInShop = i;
				break;
			}
		}

		ItemSelected = GreatShopInterface(NumberOfItemsInShop, BuyPointerList, NumberOfItemsInTuxRow, TuxItemsList, &(ShopOrder));
		switch (ShopOrder.shop_command) {
		case BUY_1_ITEM:
			if (!buy_item(BuyPointerList[ShopOrder.item_selected], ShopOrder.number_selected)) {
				// destroy our copy of the item
				npc_inventory_delete_item(npc, ShopOrder.item_selected);
			}
			break;
		case SELL_1_ITEM:
			TryToSellItem(TuxItemsList[ShopOrder.item_selected], ShopOrder.number_selected);
			break;
		case REPAIR_ITEM:
			TryToRepairItem(TuxItemsList[ShopOrder.item_selected]);
			break;
		default:

			break;
		};

		for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
			if (SalesList[i].type == (-1))
				BuyPointerList[i] = NULL;
			else
				BuyPointerList[i] = &(SalesList[i]);
		}
	}
}

#undef _shop_c
