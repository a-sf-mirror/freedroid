/* 
 *
 *   Copyright (c) 2002, 2003 Johannes Prix
 *   Copyright (c) 2004-2007 Arthur Huillet 
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
 * This function prepares a new set of items, that will be displayed for
 * the Tux to buy.
 *
 * This function is different from the other 'AssemblePointerList..'
 * functions in the sense, that here we really must first CREATE the items.
 *
 * Since the shop will always COPY and DELETE items and not only point
 * some pointer to different directions, recurrent calls of this function
 * should not cause any damage, as long as there is only ONE player in 
 * the game.
 */
void AssembleItemListForTradeCharacter(item * ListToBeFilled, int ShopCharacterCode)
{
	item *ListPointer = ListToBeFilled;
	int i;

	//--------------------
	// At first we clean out the given list.
	//
	ListPointer = ListToBeFilled;
	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		ListPointer->type = (-1);
		ListPointer->prefix_code = (-1);
		ListPointer->suffix_code = (-1);
		ListPointer++;
	}

	//--------------------
	// Depending on the character code given, we'll now refill the list
	// of items to be made available
	//
	ListPointer = ListToBeFilled;
	if (ShopCharacterCode == PERSON_STONE) {
		ListPointer->type = GetItemIndexByName("Big kitchen knife");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Meat cleaver");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Iron pipe");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Big wrench");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Crowbar");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Cap");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Buckler");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Simple Jacket");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Shoes");
		ListPointer++;
		ListPointer->type = GetItemIndexByName(".22 LR Ammunition");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Shotgun shells");
		ListPointer++;
	} else if (ShopCharacterCode == PERSON_DOC_MOORE) {
		ListPointer->type = GetItemIndexByName("Diet supplement");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Antibiotic");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Doc-in-a-can");
		ListPointer++;
	} else if (ShopCharacterCode == PERSON_LUKAS) {
		ListPointer->type = GetItemIndexByName("Laser pistol");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Laser power pack");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Plasma pistol");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Plasma energy container");
		ListPointer++;
//      ListPointer->type = GetItemIndexByName("9x19mm Ammunition"); ListPointer++;
//      ListPointer->type = GetItemIndexByName("7.62x39mm Ammunition"); ListPointer++;
//      ListPointer->type = GetItemIndexByName(".50 BMG (12.7x99mm) Ammunition"); ListPointer++;
		ListPointer->type = GetItemIndexByName("Red Guard's Light Robe");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Red Guard's Heavy Robe");
		ListPointer++;
	} else if (ShopCharacterCode == PERSON_SKIPPY) {
		ListPointer->type = GetItemIndexByName("Map Maker");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Teleporter homing beacon");
		ListPointer++;
	} else if (ShopCharacterCode == PERSON_DUNCAN) {
		ListPointer->type = GetItemIndexByName("VMX Gas Grenade");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("EMP Shockwave Generator");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Plasma Shockwave Emitter");
		ListPointer++;
	} else if (ShopCharacterCode == PERSON_EWALD) {
		ListPointer->type = GetItemIndexByName("Bottled ice");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Industrial coolant");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Liquid nitrogen");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Barf's Energy Drink");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Running Power Capsule");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Fork");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Plate");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Mug");
		ListPointer++;
	} else if (ShopCharacterCode == PERSON_SORENSON) {
		ListPointer->type = GetItemIndexByName("Source Book of Emergency shutdown");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Source Book of Check system integrity");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Source Book of Sanctuary");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Source Book of Analyze item");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Source Book of Malformed packet");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Source Book of Blue Screen");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Source Book of Broadcast Blue Screen");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Source Book of Calculate Pi");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Source Book of Virus");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Source Book of Broadcast virus");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Source Book of Dispel smoke");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Source Book of Killer poke");
		ListPointer++;
//      ListPointer->type = GetItemIndexByName("Source Book of Reverse-engineer"); ListPointer++;
		ListPointer->type = GetItemIndexByName("Source Book of Plasma discharge");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Source Book of Nethack");
		ListPointer++;
		ListPointer->type = GetItemIndexByName("Source Book of Invisibility");
		ListPointer++;
//      ListPointer->type = GetItemIndexByName("Source Book of Ricer CFLAGS"); ListPointer++;
//      ListPointer->type = GetItemIndexByName("Source Book of Light"); ListPointer++;
//      ListPointer->type = GetItemIndexByName("Source Book of Satellite image"); ListPointer++;
	} else {
		ErrorMessage(__FUNCTION__, "\
		The function has received an unexpected character code.  This is not handled\n\
		currently and therefore initiates immediate termination now...", PLEASE_INFORM, IS_FATAL);
	}

	//--------------------
	// Now it's time to fill in the correct item properties and set
	// the right flags, so that we get a 'normal' item.
	//
	ListPointer = ListToBeFilled;
	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		if (ListPointer->type == (-1))
			break;
		FillInItemProperties(ListPointer, TRUE, 1);
		ListPointer->is_identified = TRUE;
		ListPointer++;
	}

};				// void AssembleItemListForTradeCharacter ( .. )

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

	//--------------------
	// First we clean out the new Show_Pointer_List
	//
	CurrentItemPointer = ItemPointerListPointer;
	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		*CurrentItemPointer = NULL;
		CurrentItemPointer++;
	}

	//--------------------
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
	//--------------------
	// Now we start to fill the Show_Pointer_List with the items in the
	// pure unequipped inventory
	//
	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		if (Me.Inventory[i].type == (-1))
			continue;
		else {
			*CurrentItemPointer = &(Me.Inventory[i]);
			CurrentItemPointer++;
			NumberOfItems++;
		}
	}

	return (NumberOfItems);

};				// void AssemblePointerListForItemShow ( .. )

/**
 * Before we enter the chest put/take interface menu, we must assemble
 * the list of pointers to the items currently in this chest.  An item
 * is in the chest at x/y if it's in the chest-item-list of the current
 * players level and if it's coordinates are those of the current players
 * chest.
 */
int AssemblePointerListForChestShow(item ** ItemPointerListPointer, gps chest_pos)
{
	int i;
	item **CurrentItemPointer;
	int NumberOfItems = 0;

	//--------------------
	// First we clean out the new Show_Pointer_List
	//
	CurrentItemPointer = ItemPointerListPointer;
	for (i = 0; i < MAX_CHEST_ITEMS_PER_LEVEL; i++) {
		*CurrentItemPointer = NULL;
		CurrentItemPointer++;
	}

	CurrentItemPointer = ItemPointerListPointer;

	for (i = 0; i < MAX_CHEST_ITEMS_PER_LEVEL; i++) {
		if (curShip.AllLevels[Me.pos.z]->ChestItemList[i].type == (-1))
			continue;

		//--------------------
		// All the items in chests within a range of 1 square around the Tux 
		// will be collected together to be shown in the chest inventory.
		//
		if (sqrt((chest_pos.x - curShip.AllLevels[Me.pos.z]->ChestItemList[i].pos.x) *
			 (chest_pos.x - curShip.AllLevels[Me.pos.z]->ChestItemList[i].pos.x) +
			 (chest_pos.y - curShip.AllLevels[Me.pos.z]->ChestItemList[i].pos.y) *
			 (chest_pos.y - curShip.AllLevels[Me.pos.z]->ChestItemList[i].pos.y)) < 1) {
			*CurrentItemPointer = &(curShip.AllLevels[Me.pos.z]->ChestItemList[i]);
			CurrentItemPointer++;
			NumberOfItems++;
		}
	}

	return (NumberOfItems);

};				// void AssemblePointerListForChestShow ( .. )

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

		//--------------------
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

		//--------------------
		// Now at this point we know, that the click really was in the item
		// overview row.  Therefore we just need to find out the index and
		// can return;
		//
		return ((x - ShopItemRowRect.x) / (INITIAL_BLOCK_WIDTH * GameConfig.screen_width / 640));
	}
};				// int ClickWasOntoItemRowPosition ( int x , int y , int TuxItemRow )

/**
 * The item row in the shop interface (or whereever we're going to use it)
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
	static int first_call = TRUE;

	if (first_call) {
		char fpath[2048];

		find_file("mouse_cursor_0003.png", GRAPHICS_DIR, fpath, 0);
		get_iso_image_from_file_and_path(fpath, &(equipped_icon), FALSE);
		if (equipped_icon.surface == NULL) {
			fprintf(stderr, "\nFull path used: %s.", fpath);
			ErrorMessage(__FUNCTION__, "\
Error loading flag image.", PLEASE_INFORM, IS_FATAL);
		}
		if (use_open_gl) {
			make_texture_out_of_surface(&(equipped_icon));
		}
		first_call = FALSE;
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
	our_SDL_blit_surface_wrapper(ItemMap[ShowItem->type].inv_image.scaled_surface_for_shop, NULL, Screen, &TargetRectangle);
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
static void ShowItemPicture(int PosX, int PosY, int Number)
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

	//--------------------
	// Maybe we have to reload the whole image series
	//
	if (strcmp(LastImageSeriesPrefix, ItemMap[Number].item_rotation_series_prefix)) {
		//--------------------
		// Maybe we have to free the series from an old item display first
		//
		if (ItemRotationSurfaces[0] != NULL) {
			for (i = 0; i < NumberOfImagesInPreviousRotation; i++) {
				SDL_FreeSurface(ItemRotationSurfaces[i]);
			}
		}
		//--------------------
		// Now we can start to load the whole series into memory
		//
		for (i = 0; i < MAX_NUMBER_OF_IMAGES_IN_ITEM_ROTATION; i++) {
			//--------------------
			// At first we will try to find some item rotation models in the
			// new directory structure.
			//
			sprintf(ConstructedFileName, "items/%s/portrait_%04d.jpg", ItemMap[Number].item_rotation_series_prefix, i + 1);
			if (find_file(ConstructedFileName, GRAPHICS_DIR, fpath, 1))
				Whole_Image = NULL;
			else
				Whole_Image = our_IMG_load_wrapper(fpath);	// This is a surface with alpha channel, since the picture is one of this type

			//--------------------
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
			//--------------------
			// But at this point, we should have found the image!!
			// or if not, this maybe indicates that we have reached the
			// last image in the image series...
			//
			if (Whole_Image == NULL) {
				NumberOfImagesInThisRotation = i;
				NumberOfImagesInPreviousRotation = NumberOfImagesInThisRotation;
				DebugPrintf(1, "\nDONE LOADING ITEM IMAGE SERIES.  Loaded %d images into memory.",
					    NumberOfImagesInThisRotation);

				//--------------------
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
			//--------------------
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

	DebugPrintf(2, "\n%s(): Usual end of function reached.", __FUNCTION__);

};				// void ShowItemPicture ( ... )

/* ------------------------------------------------------------
 * This function displays information about one item on a
 * Paradroid-console like display.
 * ------------------------------------------------------------ */
static void ShowItemInfo(item * ShowItem, int Displacement, char ShowArrows, int background_code, int title_text_flag)
{
	static item LastItemShown;
	static char InfoText[10000];
	char TextChunk[2000];
	char *ClassString;
	long int repairPrice = 0;

	if (ShowItem == NULL)
		return;

	SDL_SetClipRect(Screen, NULL);

	blit_special_background(background_code);
	ShowItemPicture(40 * GameConfig.screen_width / 1024 + ((250 * GameConfig.screen_width / 1024) - 132) / 2,
			185 * GameConfig.screen_height / 768 + ((322 * GameConfig.screen_height / 768) - 180) / 2, ShowItem->type);

	//--------------------
	// If that is wanted, we fill out the title header line, announcing the
	// currently browsed items name in full glory.
	//
	if (title_text_flag) {
		SetCurrentFont(Menu_BFont);
		strcpy(TextChunk, D_(ItemMap[ShowItem->type].item_name));
		CutDownStringToMaximalSize(TextChunk, 225);
		PutString(Screen, 330, 38, TextChunk);
	}
	//--------------------
	// Now we can display the rest of the smaller-font item description.
	// If the item has changed, we must first assemble the description
	//
	if (memcmp(ShowItem, &LastItemShown, sizeof(item))) {
		if (ItemMap[ShowItem->type].item_can_be_installed_in_weapon_slot)
			ClassString = _("Weapon");
		else if (ItemMap[ShowItem->type].item_can_be_installed_in_drive_slot)
			ClassString = _("Drive");
		else if (ItemMap[ShowItem->type].item_can_be_installed_in_armour_slot)
			ClassString = _("Armor");
		else if (ItemMap[ShowItem->type].item_can_be_installed_in_shield_slot)
			ClassString = _("Shield");
		else if (ItemMap[ShowItem->type].item_can_be_installed_in_special_slot)
			ClassString = _("Helm");
		else
			ClassString = _("Miscellaneous");

		write_full_item_name_into_string(ShowItem, TextChunk);
		sprintf(InfoText, _("Item: %s \nClass: %s\n"), TextChunk, ClassString);

		if ((ShowItem->suffix_code != (-1)) || (ShowItem->prefix_code != (-1))) {
			if (ShowItem->is_identified == TRUE) {
				GiveItemDescription(TextChunk, ShowItem, TRUE);
				// Now clean up TextChunk to get only information about the special abilities of the object
				char *tmp = strstr(TextChunk, "             +");
				if (tmp)	//In case something would go wrong...
				{
					tmp = strstr(tmp, " +");
					strcpy(TextChunk, tmp);
				}
				strcat(InfoText, _("Specials:"));
				strcat(InfoText, font_switchto_red);
				strcat(InfoText, TextChunk);
				strcat(InfoText, "\n");
				strcat(InfoText, font_switchto_neon);
			}
		}

		if (ItemMap[ShowItem->type].item_group_together_in_inventory) {
			if (!MatchItemWithName(ShowItem->type, "Cyberbucks")) {
				strcat(InfoText, _("Multiplicity: "));
				sprintf(TextChunk, "%d \n", (int)ShowItem->multiplicity);
				strcat(InfoText, TextChunk);
			}
		}

		strcat(InfoText, _("Duration: "));
		if (ShowItem->max_duration >= 0)
			sprintf(TextChunk, "%d / %d\n", (int)ShowItem->current_duration, ShowItem->max_duration);
		else
			sprintf(TextChunk, _("Indestructible\n"));
		strcat(InfoText, TextChunk);

		if (!ItemMap[ShowItem->type].item_can_be_applied_in_combat) {
			strcat(InfoText, _("Attributes required: "));

			if ((ItemMap[ShowItem->type].item_require_strength <= 0) &&
			    (ItemMap[ShowItem->type].item_require_dexterity <= 0) && (ItemMap[ShowItem->type].item_require_magic <= 0)) {
				strcat(InfoText, _("NONE\n"));
			} else {
				if (ItemMap[ShowItem->type].item_require_strength > 0) {
					sprintf(TextChunk, _("Str: %d "), ItemMap[ShowItem->type].item_require_strength);
					strcat(InfoText, TextChunk);
				}
				if (ItemMap[ShowItem->type].item_require_dexterity > 0) {
					sprintf(TextChunk, _("Dex: %d "), ItemMap[ShowItem->type].item_require_dexterity);
					strcat(InfoText, TextChunk);
				}
				if (ItemMap[ShowItem->type].item_require_magic > 0) {
					sprintf(TextChunk, _("Mag: %d "), ItemMap[ShowItem->type].item_require_magic);
					strcat(InfoText, TextChunk);
				}
				strcat(InfoText, "\n");
			}
		} else {
			/*    switch ( ShowItem -> type )
			   {
			   case ITEM_SPELLBOOK_OF_HEALING:
			   case ITEM_SPELLBOOK_OF_EXPLOSION_CIRCLE:
			   case ITEM_SPELLBOOK_OF_EXPLOSION_RAY:
			   case ITEM_SPELLBOOK_OF_TELEPORT_HOME:
			   case ITEM_SPELLBOOK_OF_IDENTIFY:
			   case ITEM_SPELLBOOK_OF_PLASMA_BOLT:
			   case ITEM_SPELLBOOK_OF_ICE_BOLT:
			   case ITEM_SPELLBOOK_OF_POISON_BOLT:
			   case ITEM_SPELLBOOK_OF_PETRIFICATION:
			   case ITEM_SPELLBOOK_OF_RADIAL_EMP_WAVE:
			   case ITEM_SPELLBOOK_OF_RADIAL_VMX_WAVE:
			   case ITEM_SPELLBOOK_OF_RADIAL_PLASMA_WAVE:

			   sprintf( TextChunk , "Spellcasting skill: %s\n " ,
			   _(AllSkillTexts [ required_spellcasting_skill_for_item ( ShowItem -> type ) ]));
			   strcat( InfoText , TextChunk );
			   sprintf( TextChunk , "Magic: %d\n " ,
			   required_magic_stat_for_next_level_and_item ( ShowItem -> type ) );
			   strcat( InfoText , TextChunk );
			   break;
			   default:
			   break;
			   } */
		}
		//--------------------
		// Now we give some pricing information, the base list price for the item,
		// the repair price and the sell value
		if (calculate_item_buy_price(ShowItem)) {
			sprintf(TextChunk, _("Base list price: %ld\n"), calculate_item_buy_price(ShowItem));
			strcat(InfoText, TextChunk);
			sprintf(TextChunk, _("Sell value: %ld\n"), calculate_item_sell_price(ShowItem));
			strcat(InfoText, TextChunk);
			if (ShowItem->current_duration == ShowItem->max_duration || ShowItem->max_duration == (-1))
				repairPrice = 0;
			else
				repairPrice = calculate_item_repair_price(ShowItem);
			sprintf(TextChunk, _("Repair cost: %ld\n"), repairPrice);
			strcat(InfoText, TextChunk);
		} else {
			sprintf(TextChunk, _("Unsellable\n"));
			strcat(InfoText, TextChunk);
		}

		if ((ShowItem->suffix_code != (-1)) || (ShowItem->prefix_code != (-1))) {
			if (ShowItem->is_identified == FALSE)
				strcat(InfoText, _("Identification cost: 100\n"));
		}
		//--------------------
		// If the item is a weapon, then we print out some weapon stats...
		//
		if (ItemMap[ShowItem->type].base_item_gun_damage + ItemMap[ShowItem->type].item_gun_damage_modifier > 0) {
			sprintf(TextChunk, _("Damage: %d - %d\n"),
				ItemMap[ShowItem->type].base_item_gun_damage,
				ItemMap[ShowItem->type].base_item_gun_damage + ItemMap[ShowItem->type].item_gun_damage_modifier);
			strcat(InfoText, TextChunk);
		}

		if (ItemMap[ShowItem->type].item_gun_recharging_time > 0) {
			sprintf(TextChunk, _("Recharge time: %3.2f\n"), ItemMap[ShowItem->type].item_gun_recharging_time);
			strcat(InfoText, TextChunk);
		}

		if (ItemMap[ShowItem->type].item_gun_reloading_time > 0) {
			sprintf(TextChunk, _("Time to reload ammo clip: %3.2f\n"), ItemMap[ShowItem->type].item_gun_reloading_time);
			strcat(InfoText, TextChunk);
		}

		if (ShowItem->ac_bonus > 0) {
			sprintf(TextChunk, _("Defense bonus: %d\n"), ShowItem->ac_bonus);
			strcat(InfoText, TextChunk);
		}

		sprintf(TextChunk, _("Notes: %s"), D_(ItemMap[ShowItem->type].item_description));
		strcat(InfoText, TextChunk);

		switch (ItemMap[ShowItem->type].item_gun_use_ammunition) {
		case 2:
			strcat(InfoText, _(" This weapon requires standard plasma ammunition."));
			break;
		case 1:
			strcat(InfoText, _(" This weapon requires standard laser crystal ammunition."));
			break;
		case 3:
			strcat(InfoText, _(" This weapon requires standard exterminator ammunition spheres."));
			break;
		case 4:
			strcat(InfoText, _(" This weapon requires .22 Long Rifle rounds."));
			break;
		case 5:
			strcat(InfoText, _(" This weapon requires Shotgun shells."));
			break;
		case 6:
			strcat(InfoText, _(" This weapon requires 9x19mm rounds."));
			break;
		case 7:
			strcat(InfoText, _(" This weapon requires 7.62x39mm rounds."));
			break;
		case 8:
			strcat(InfoText, _(" This weapon requires .50 (12.7x99mm) Browning Machine Gun rounds."));
			break;
		}

		//-----------------------
		// We cache the item
		// 
		memcpy(&LastItemShown, ShowItem, sizeof(item));
	}
	// SetCurrentFont( Para_BFont );
	// SetCurrentFont( Menu_BFont );
	SetCurrentFont(FPS_Display_BFont);
	DisplayText(InfoText, Cons_Text_Rect.x, Cons_Text_Rect.y + Displacement, &Cons_Text_Rect, TEXT_STRETCH);

	if (ShowArrows) {
		ShowGenericButtonFromList(UP_BUTTON);
		ShowGenericButtonFromList(DOWN_BUTTON);
	}

};				// void ShowItemInfo ( ... )

/**
 * This function does the item show when the user has selected item
 * show from the console menu.
 */
int GreatShopInterface(int NumberOfItems, item * ShowPointerList[MAX_ITEMS_IN_INVENTORY],
		       int NumberOfItemsInTuxRow, item * TuxItemsList[MAX_ITEMS_IN_INVENTORY], shop_decision * ShopOrder)
{
	int Displacement = 0;
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
	game_status = INSIDE_MENU;

	//--------------------
	// For the shop, we'll also try to use our own mouse cursor
	//
	make_sure_system_mouse_cursor_is_turned_off();

	//--------------------
	// We add some secutiry against indexing beyond the
	// range of items given in the list.
	//
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

	//--------------------
	// We initialize the text rectangle
	//
	Cons_Text_Rect.x = 258 * GameConfig.screen_width / 640;
	Cons_Text_Rect.y = 108 * GameConfig.screen_height / 480;
	Cons_Text_Rect.w = 346 * GameConfig.screen_width / 640;
	Cons_Text_Rect.h = 255 * GameConfig.screen_height / 480;

	Displacement = 0;

	while (1) {

		save_mouse_state();
		input_handle();

		//--------------------
		// We limit the 'displacement', i.e. how far up and down one can
		// scroll the text of the item description up and down a bit, so
		// it cannot be scrolled away ad infinitum...
		//
		if (Displacement < -500)
			Displacement = -500;
		if (Displacement > 50)
			Displacement = 50;

		SDL_Delay(1);
		ShopOrder->shop_command = DO_NOTHING;

		//--------------------
		// We show all the info and the buttons that should be in this
		// interface...
		//
		AssembleCombatPicture(USE_OWN_MOUSE_CURSOR | ONLY_SHOW_MAP);
		if (ItemIndex >= 0)
			ShowItemInfo(ShowPointerList[ItemIndex], Displacement, FALSE, ITEM_BROWSER_SHOP_BACKGROUND_CODE, FALSE);
		else if (TuxItemIndex >= 0)
			ShowItemInfo(TuxItemsList[TuxItemIndex], Displacement, FALSE, ITEM_BROWSER_SHOP_BACKGROUND_CODE, FALSE);
		else
			blit_special_background(ITEM_BROWSER_SHOP_BACKGROUND_CODE);

		for (i = 0; i < RowLength; i++) {
			ShowRescaledItem(i, FALSE, ShowPointerList[i + RowStart]);
		}

		for (i = 0; i < TuxRowLength; i++) {
			ShowRescaledItem(i, TRUE, TuxItemsList[i + TuxRowStart]);
		}

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

			if (!TuxItemsList[TuxItemIndex]->is_identified)
				ShowGenericButtonFromList(IDENTIFY_BUTTON);
		} else {
			BuyButtonActive = FALSE;
			SellButtonActive = FALSE;
		}

		//--------------------
		// We show the current amount of 'gold' or 'cyberbucks' the tux
		// has on him.  However we need to take into account the scaling
		// of the whole screen again for this.
		//
		sprintf(GoldString, "%6d", (int)Me.Gold);
		PutStringFont(Screen, FPS_Display_BFont, 40 * GameConfig.screen_width / 640,
			      370 * GameConfig.screen_height / 480, GoldString);

		blit_our_own_mouse_cursor();
		our_SDL_flip_wrapper();

		if (MouseLeftClicked()) {
			if (MouseCursorIsOnButton(DESCRIPTION_WINDOW_UP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				MoveMenuPositionSound();
				Displacement += FontHeight(GetCurrentFont());
			} else if (MouseCursorIsOnButton(DESCRIPTION_WINDOW_DOWN_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				MoveMenuPositionSound();
				Displacement -= FontHeight(GetCurrentFont());
			} else if (MouseCursorIsOnButton(ITEM_BROWSER_EXIT_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				while (MouseLeftPressed())
					SDL_Delay(1);
				ret = -1;
				goto out;
			} else if (MouseCursorIsOnButton(LEFT_TUX_SHOP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (0 < RowStart) {
					RowStart--;
					if ((ItemIndex != (-1)) && (ItemIndex >= RowStart + RowLength)) {
						Displacement = 0;
						ItemIndex--;
					}
				}
				MoveMenuPositionSound();
			} else if (MouseCursorIsOnButton(RIGHT_TUX_SHOP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (RowStart + RowLength < NumberOfItems) {
					RowStart++;
					if ((ItemIndex != (-1)) && (ItemIndex < RowStart)) {
						Displacement = 0;
						ItemIndex++;
					}
				}
				MoveMenuPositionSound();
			} else if (MouseCursorIsOnButton(LEFT_SHOP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (0 < TuxRowStart) {
					TuxRowStart--;
					if ((TuxItemIndex != (-1)) && (TuxItemIndex >= TuxRowStart + TuxRowLength)) {
						Displacement = 0;
						TuxItemIndex--;
					}
				}
				MoveMenuPositionSound();
			} else if (MouseCursorIsOnButton(RIGHT_SHOP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (TuxRowStart + TuxRowLength < NumberOfItemsInTuxRow) {
					TuxRowStart++;
					if ((TuxItemIndex != (-1)) && (TuxItemIndex < TuxRowStart)) {
						TuxItemIndex++;
						Displacement = 0;
					}
				}
				MoveMenuPositionSound();
			} else if (((ClickTarget = ClickWasOntoItemRowPosition(GetMousePos_x(), GetMousePos_y(), FALSE)) >= 0)) {
				if (ClickTarget < NumberOfItems) {
					ItemIndex = RowStart + ClickTarget;
					TuxItemIndex = (-1);
					Displacement = 0;
				}
			} else if (((ClickTarget = ClickWasOntoItemRowPosition(GetMousePos_x(), GetMousePos_y(), TRUE)) >= 0)) {
				if (ClickTarget < NumberOfItemsInTuxRow) {
					TuxItemIndex = TuxRowStart + ClickTarget;
					ItemIndex = (-1);
					Displacement = 0;
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
				//--------------------
				// Reference to the Tux item list must only be made, when the 'highlight'
				// is really in the tux item row.  Otherwise we just get a segfault...
				//
				if (TuxItemIndex > (-1)) {
					//--------------------
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
			} else if (MouseCursorIsOnButton(IDENTIFY_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				//--------------------
				// Reference to the Tux item list must only be made, when the 'highlight'
				// is really in the tux item row.  Otherwise we just get a segfault...
				//
				if (TuxItemIndex > (-1)) {
					if (!TuxItemsList[TuxItemIndex]->is_identified) {
						ShopOrder->item_selected = TuxItemIndex;
						ShopOrder->shop_command = IDENTIFY_ITEM;
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
			Displacement += FontHeight(GetCurrentFont());
		}
		if (DownPressed() || MouseWheelDownPressed()) {
			MoveMenuPositionSound();
			while (DownPressed()) ;
			Displacement -= FontHeight(GetCurrentFont());
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
 * This function tells us which item in the menu has been clicked upon.
 * It does not check for lower than 4 items in the menu available.
 */
int ClickedMenuItemPosition(void)
{
	int CursorX, CursorY;
	int i;

	CursorX = GetMousePos_x();	// this is already the position corrected for 16 pixels!!
	CursorY = GetMousePos_y();	// this is already the position corrected for 16 pixels!!

#define ITEM_MENU_DISTANCE 80
#define ITEM_FIRST_POS_Y 130
#define NUMBER_OF_ITEMS_ON_ONE_SCREEN 4

	//--------------------
	// When a character is blitted to the screen at x y, then the x and y
	// refer to the top left corner of the coming blit.  Using this information
	// we will define the areas where a click 'on the blitted text' has occured
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

	//--------------------
	// At this point we've already determined and returned to right click-area.
	// if this point is ever reached, a severe error has occured, and Freedroid
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
 * This function tries to identify the item given as parameter.  
 */
void TryToIdentifyItem(item * IdentifyItem)
{
	char *MenuTexts[10];
	MenuTexts[0] = _("Yes");
	MenuTexts[1] = _("No");
	MenuTexts[2] = "";

	while (SpacePressed() || EnterPressed() || MouseLeftPressed()) ;

	if (100 > Me.Gold) {
		MenuTexts[0] = _(" BACK ");
		MenuTexts[1] = "";
		SetCurrentFont(Menu_BFont);
		DoMenuSelection(_("You can't afford to have this item identified! "), MenuTexts, 1, -1, NULL);
		return;
	}

/** As lynx pointed out on irc, it's useless to ask for confirmation, since the user actually requested identification. */
	while (EnterPressed() || SpacePressed()) ;
	Me.Gold -= 100;
	IdentifyItem->is_identified = TRUE;
	PlayOnceNeededSoundSample("effects/Shop_ItemIdentifiedSound_0.ogg", FALSE, FALSE);

	return;
};				// void TryToIdentifyItem( item* IdentifyItem )

/**
 * This function tries to sell the item given as parameter.
 */
void TryToSellItem(item * SellItem, int AmountToSellAtMost)
{
#define ANSWER_YES 1
#define ANSWER_NO 2

	//--------------------
	// We catch the case, that not even one item was selected
	// for buying in the number selector...
	//
	if (AmountToSellAtMost <= 0) {
		DebugPrintf(0, "\nTried to sell 0 items of a kind... doing nothing... ");
		return;
	}
	//--------------------
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

	//--------------------
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
 * This function tries to put an item into the inventory, either by adding
 * this items multiplicity to the multiplicity of an already present item
 * of the very same type or by allocating a new inventory item for this
 * new item and putting it there.
 *
 * In the case that both methods couldn't succeed, a FALSE value is 
 * returned to let the caller know, that this procedure has failed.
 * Otherwise TRUE will indicate that everything is ok and went well.
 */
int TryToIntegrateItemIntoInventory(item * BuyItem, int AmountToBuyAtMost)
{
	int x, y;
	int FreeIndex;
	char linebuf[1000];
	int i;
	char *MenuTexts[10];

	//--------------------
	// At first we try to see if we can just add the multiplicity of the item in question
	// to the existing multiplicity of an item of the same type
	//
	if (ItemMap[BuyItem->type].item_group_together_in_inventory) {
		for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
			if (Me.Inventory[i].type == BuyItem->type) {
				while (1) {
					while (EnterPressed() || SpacePressed()) ;
					Me.Inventory[i].multiplicity += AmountToBuyAtMost;

					//--------------------
					// This is new.  I hope it's not dangerous.
					//
					if (AmountToBuyAtMost >= BuyItem->multiplicity)
						DeleteItem(BuyItem);
					else
						BuyItem->multiplicity -= AmountToBuyAtMost;
					return (TRUE);
				}
			}
		}
	}
	//--------------------
	// Now we must find out if there is an inventory position where we can put the
	// item in question.
	//
	FreeIndex = GetFreeInventoryIndex();

	for (x = 0; x < INVENTORY_GRID_WIDTH; x++) {
		for (y = 0; y < INVENTORY_GRID_HEIGHT; y++) {
			if (ItemCanBeDroppedInInv(BuyItem->type, x, y)) {
				while (1) {
					while (EnterPressed() || SpacePressed()) ;

					CopyItem(BuyItem, &(Me.Inventory[FreeIndex]), FALSE);
					Me.Inventory[FreeIndex].multiplicity = AmountToBuyAtMost;

					Me.Inventory[FreeIndex].currently_held_in_hand = FALSE;
					Me.Inventory[FreeIndex].inventory_position.x = x;
					Me.Inventory[FreeIndex].inventory_position.y = y;

					//--------------------
					// This is new.  I hope it's not dangerous.
					//
					if (BuyItem->multiplicity <= AmountToBuyAtMost)
						DeleteItem(BuyItem);
					else
						BuyItem->multiplicity -= AmountToBuyAtMost;

					return (TRUE);
				}
			}
		}
	}

	MenuTexts[0] = _(" BACK ");
	MenuTexts[1] = "";
	GiveItemDescription(linebuf, BuyItem, TRUE);
	strcat(linebuf, _("\n\n   No room for this item in inventory!"));
	DoMenuSelection(linebuf, MenuTexts, 1, -1, NULL);
	return (FALSE);

};				// void TryToIntegrateItemIntoInventory ( item* BuyItem , int AmountToBuyAtMost )

/**
 * This function tries to buy the item given as parameter.  Currently
 * is just drops the item to the floor under the influencer and will
 * reduce influencers money.
 */
void TryToTakeItem(item * BuyItem, int AmountToBuyAtMost)
{
	int StoredItemType;

	StoredItemType = BuyItem->type;

	//--------------------
	// We catch the case, that not even one item was selected
	// for taking out from the chest in the number selector...
	//
	if (AmountToBuyAtMost <= 0) {
		DebugPrintf(0, _("\nTried to take 0 items of a kind from chest or cointainer... doing nothing... "));
		return;
	}
	//--------------------
	// We prevent some take-put-cheating here.  For buying items this must
	// NOT be done.
	//
	if (AmountToBuyAtMost >= BuyItem->multiplicity)
		AmountToBuyAtMost = BuyItem->multiplicity;

	if (TryToIntegrateItemIntoInventory(BuyItem, AmountToBuyAtMost)) {
		// PlayItemSound( ItemMap[ StoredItemType ].sound_number );
		play_item_sound(StoredItemType);
	}
};				// void TryToTakeItem( item* BuyItem , int AmountToBuyAtMost )

/**
 * This function tries to buy the item given as parameter.  Currently
 * is just drops the item to the floor under the influencer and will
 * reduce influencers money.
 */
void TryToBuyItem(item * BuyItem, int WithBacktalk, int AmountToBuyAtMost)
{
	int FreeIndex;
	char linebuf[1000];
	float PotentialPrice;

#define ANSWER_YES 1
#define ANSWER_NO 2

	char *MenuTexts[10];
	MenuTexts[0] = _("Yes");
	MenuTexts[1] = _("No");
	MenuTexts[2] = "";

	DebugPrintf(0, "\nTryToBuyItem (...):  function called.");

	//--------------------
	// We catch the case, that not even one item was selected
	// for buying in the number selector...
	//
	if (AmountToBuyAtMost <= 0) {
		DebugPrintf(0, "\nTried to buy 0 items of a kind... doing nothing... ");
		return;
	}

	BuyItem->multiplicity = AmountToBuyAtMost;

	FreeIndex = GetFreeInventoryIndex();

	while (SpacePressed() || EnterPressed() || MouseLeftPressed()) ;

	if (calculate_item_buy_price(BuyItem) > Me.Gold) {
		if (WithBacktalk) {
			MenuTexts[0] = _(" BACK ");
			MenuTexts[1] = "";
			GiveItemDescription(linebuf, BuyItem, TRUE);
			strcat(linebuf, _("\n\n    You can't afford to purchase this item!"));
			SetCurrentFont(Menu_BFont);
			DoMenuSelection(linebuf, MenuTexts, 1, -1, NULL);
		}
		return;
	}
	//--------------------
	// In the case that the item could be afforded in theory, we need to
	// calculate the price, then have the item integrated into the inventory
	// if that's possible, and if so, subtract the items price from the
	// current gold.
	//
	PotentialPrice = calculate_item_buy_price(BuyItem);

	if (TryToIntegrateItemIntoInventory(BuyItem, AmountToBuyAtMost)) {
		Me.Gold -= PotentialPrice;
		PlayOnceNeededSoundSample("effects/Shop_ItemBoughtSound_0.ogg", FALSE, FALSE);
	} else {
		// bad luck.  couldn't store item in inventory, so no price paid...
	}

};				// void TryToBuyItem( item* BuyItem )

/**
 * This is some preparation for the shop interface.  We assemble some
 * pointer list with the stuff Tux has to sell and the stuff the shop
 * has to offer.
 *
 *
 */
void InitTradeWithCharacter(int CharacterCode)
{
#define FIXED_SHOP_INVENTORY TRUE
#define NUMBER_OF_ITEMS_IN_SHOP 17
	// #define NUMBER_OF_ITEMS_IN_SHOP 4

	item SalesList[MAX_ITEMS_IN_INVENTORY];
	item *BuyPointerList[MAX_ITEMS_IN_INVENTORY];
	item *TuxItemsList[MAX_ITEMS_IN_INVENTORY];
	int i;
	int ItemSelected = 0;
	shop_decision ShopOrder;
	int NumberOfItemsInTuxRow = 0;
	int NumberOfItemsInShop = 0;

	AssembleItemListForTradeCharacter(&(SalesList[0]), CharacterCode);
	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		if (SalesList[i].type == (-1))
			BuyPointerList[i] = NULL;
		else
			BuyPointerList[i] = &(SalesList[i]);
	}

	//--------------------
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
			TryToBuyItem(BuyPointerList[ShopOrder.item_selected], TRUE, ShopOrder.number_selected);
			break;
		case SELL_1_ITEM:
			TryToSellItem(TuxItemsList[ShopOrder.item_selected], ShopOrder.number_selected);
			break;
		case REPAIR_ITEM:
			TryToRepairItem(TuxItemsList[ShopOrder.item_selected]);
			break;
		case IDENTIFY_ITEM:
			TryToIdentifyItem(TuxItemsList[ShopOrder.item_selected]);
			break;
		default:

			break;
		};

		AssembleItemListForTradeCharacter(&(SalesList[0]), CharacterCode);
		for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
			if (SalesList[i].type == (-1))
				BuyPointerList[i] = NULL;
			else
				BuyPointerList[i] = &(SalesList[i]);
		}

	}

};				// void InitTradeWithCharacter( void )

#undef _shop_c
