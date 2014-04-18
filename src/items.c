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
 * This file contains all the functions managing the items in the game.
 */

#define _items_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

/**
 * \brief Initializes an empty item.
 * \param item_ Item.
 */
void init_item(item *it)
{
	memset(it, 0, sizeof(item));
	it->type = -1;
	it->pos.x = -1;
	it->pos.y = -1;
	it->pos.z = -1;
	it->inventory_position.x = -1;
	it->inventory_position.y = -1;
}

item create_item_with_id(const char *item_id, int full_durability, int multiplicity)
{
	item new_item;

	init_item(&new_item);
	new_item.type = get_item_type_by_id(item_id);
	if (new_item.type < 0 || new_item.type >= Number_Of_Item_Types) {
		error_message(__FUNCTION__, "No items with the name \"%s\" exist in the game. Cannot create item.\nChanging item type to bug item to avoid crash.", PLEASE_INFORM, item_id);
		new_item.type = 0;
	}

	FillInItemProperties(&new_item, full_durability, multiplicity);

	return new_item;
}

/**
 * This function checks if the item can be equipped
 * \param it item to check
 * \return TRUE if the item can be installed in one slot, FALSE otherwise
 */
static int equippable_item(item *it)
{
	return ItemMap[it->type].slot != NO_SLOT;
}

void equip_item(item *new_item)
{
	item *old_item;
	itemspec *new_itemspec;
	
	new_itemspec = &ItemMap[new_item->type];

	// If the item can't be equiped, stop now and throw a warning.
	if (!equippable_item(new_item)) {
		error_message(__FUNCTION__, "Tried to equip the item \"%s\" which can't be equipped.",
				PLEASE_INFORM, new_itemspec->id);
		return;
 	}

	// If there's an existing item in the equipment slot, drop it to the
	// inventory or to the floor.
	old_item = get_equipped_item_in_slot_for(new_item->type);
	if (old_item->type != -1) {
		give_item(old_item);
	}

	// If we're equipping a two-handed weapon, we need to unequip the shield
	// as well. We drop the shield to the inventory or to the floor.
	if (new_itemspec->item_gun_requires_both_hands && Me.shield_item.type != -1) {
		give_item(&Me.shield_item);
	}

	// Before equipping a shield, if a two-handed weapon is equipped, you need to drop it.
	if (new_itemspec->slot == SHIELD_SLOT && Me.weapon_item.type != -1 
			&& ItemMap[Me.weapon_item.type].item_gun_requires_both_hands) {
		give_item(&Me.weapon_item);
	}

	// Move the new item to the now empty equipment slot.
	MoveItem(new_item, old_item);
}


/**
 * Gets a pointer to the currently equipped item of the
 * specified type
 */
item *get_equipped_item_in_slot_for(int item_type)
{
	item *equipped_item;
	itemspec *spec;

	spec = &ItemMap[item_type];
	
	if (spec->slot == WEAPON_SLOT) {
		equipped_item = &Me.weapon_item;
	}
	else if (spec->slot == BOOT_SLOT) {
		equipped_item = &Me.drive_item;
	}
	else if (spec->slot == ARMOR_SLOT) {
		equipped_item = &Me.armour_item;
	}
	else if (spec->slot == SHIELD_SLOT) {
		equipped_item = &Me.shield_item;
	}
	else if (spec->slot == HELM_SLOT) {
		equipped_item = &Me.special_item;
	}
	else
	{
		equipped_item = NULL;
	}
	
	return equipped_item;
}

/**
 * This function does the home made item repair, i.e. the item repair via
 * the repair skill in contrast to the item repair via the shop, which of
 * course works much better.
 */
static void self_repair_item(item *it)
{
	int wear = 0, used = 0;
	int my_skill_level = 0;
	int percent_chance = MyRandom(100);

	if (it->max_durability == -1) {
		play_sound("effects/tux_ingame_comments/Tux_Item_Cant_Be_0.ogg");
		return;
	} 

	my_skill_level = Me.skill_level[get_program_index_with_name("Repair equipment")];

/* if Tux's repair ability is at the full skill level 
 * make it so that most of the time the item is fully repaired. Give it a 5 % chance 
 * of improving the item and a 3% chance of making it worse.
 * otherwise give him a skill level *10 percent chance of repairing the item completely
 */
	if(my_skill_level >= NUMBER_OF_SKILL_LEVELS-1) {
		if(percent_chance > 94)
		{
			it->max_durability++;
		}
		else if (percent_chance < 3)
		{
			it->max_durability--;
		}
	}
	else if(my_skill_level*10 < percent_chance) {		
		used = it->max_durability - it->current_durability;
		/* Self repair formula: decrease max_durability between 1 and 11-skill_level*/
		wear = 1 + MyRandom(10 - my_skill_level);
		//never decrease more than current missing durability
		it->max_durability -= min(wear, used);
		if (it->max_durability < 1) {
			it->max_durability = 1;
		}
	}
	//when you wear off all extra durability, the item become normal again
	if (it->quality == GOOD_QUALITY && it->max_durability < ItemMap[it->type].base_item_durability) {
		it->quality = NORMAL_QUALITY;
	}
	//if you add extra durability, the item becomes good
	if (it->quality == NORMAL_QUALITY && it->max_durability > ItemMap[it->type].base_item_durability) {
		it->quality = GOOD_QUALITY;
	}

	it->current_durability = it->max_durability;
	play_sound("effects/tux_ingame_comments/Tux_This_Quick_Fix_0.ogg");
}

/**
 * This function calculates the price of a single given item, taking into account
 * (*) the base list price of the item
 * (*) the base list prices of the installed add-ons
 */
unsigned long calculate_item_buy_price(item * BuyItem)
{
	int i;
	int price = ItemMap[BuyItem->type].base_list_price;

	// Add the prices of the add-ons to the total price.
	for (i = 0; i < BuyItem->upgrade_sockets.size; i++) {
		const char *addon = BuyItem->upgrade_sockets.arr[i].addon;
		if (addon) {
			int type = get_item_type_by_id(addon);
			price += ItemMap[type].base_list_price;
		}
	}

	return price;
}

/**
 * This function calculates the sell price of a single given item, taking into account
 * the markdown (currently 0.3 for all NPCs)
 */
unsigned long calculate_item_sell_price(item * BuyItem)
{
	int price;
	// Some items cannot be sold
	if (!(price = calculate_item_buy_price(BuyItem)))
		return 0;

	// Items sell for less than the full price of the item.
	price = floor(0.3 * price);

	// Prices have to be non-zero so the item can be sold
	return price ? price : 1;
}

/**
 * This function calculates the price of a given item, taking into account
 * (*) the base list price of the item
 * (*) the base list prices of the installed add-ons
 * (*) AND THE CURRENT DURABILITY of the item in relation to its max durability.
 */
unsigned long calculate_item_repair_price(item * repair_item)
{
	// For repair, it's not the full 'buy' cost...
	//
#define REPAIR_PRICE_FACTOR (0.5)

	// This is the price of the DAMAGE in the item, haha
	// This can only be requested for repair items
	//
	if (repair_item->max_durability != (-1)) {
		unsigned long price = (calculate_item_buy_price(repair_item) *
			REPAIR_PRICE_FACTOR * (repair_item->max_durability - repair_item->current_durability) / repair_item->max_durability);

		// Never repair for free, minimum price is 1
		return price ? price : 1;
	}
	return 0;
};				// long calculate_item_repair_price ( item* repair_item )

/**
 * \brief Returns a random quality multiplier.
 * \return a quality indicator.
 */
static enum item_quality random_item_quality()
{
	// In order to make normal quality items more common than others, we first
	// choose a quality level by indexing a probability distribution array.
	const enum item_quality quality_distribution[] = {
		BAD_QUALITY, 
		BAD_QUALITY, 	//20%
		NORMAL_QUALITY,
		NORMAL_QUALITY,
		NORMAL_QUALITY,
		NORMAL_QUALITY,
		NORMAL_QUALITY,
		NORMAL_QUALITY,
		NORMAL_QUALITY, //70%
		GOOD_QUALITY};	//10%
	int max_index = (sizeof(quality_distribution) / sizeof(quality_distribution[0])) - 1;
	int quality_index = MyRandom(max_index);
	return quality_distribution[quality_index];
}

/**
 * \brief Initializes the properties of the item.
 *
 * Sets the durability, armor rating, and other such properties based on the
 * type of the item. The caller must ensure that the type field has been set.
 *
 * \param it Item whose properties to initialize.
 * \param full_durability TRUE to make the item fully repaired.
 * \param multiplicity Multiplicity of the item.
 */
void FillInItemProperties(item *it, int full_durability, int multiplicity)
{
	// Some basic error checking of item type.
	if ((it->type < 0) || (it->type >= Number_Of_Item_Types)) {
		error_message(__FUNCTION__, "Cannot fill in information for item with invalid type.", PLEASE_INFORM | IS_FATAL);
		return;
	}	

	itemspec *spec = &ItemMap[it->type];

	it->multiplicity = multiplicity;
	it->ammo_clip = 0;
	it->throw_time = 0;
	it->quality = NORMAL_QUALITY;
	it->max_durability = -1;
	
	if (!equippable_item(it)) {
		return;
	}


	it->quality = random_item_quality();

	// Add random bullets to the clip if the item is a gun.
	if (spec->item_gun_ammo_clip_size) {
		it->ammo_clip = MyRandom(spec->item_gun_ammo_clip_size);
	}

	// Set the base damage reduction by using the item spec and a random multiplier.
	it->armor_class = spec->base_armor_class + MyRandom(spec->armor_class_modifier);


	// Set the maximum and current durabilities of the item.
	if (spec->base_item_durability != -1) {
		// The maximum durability is within the range specified by the item spec.
		it->max_durability = spec->base_item_durability + MyRandom(spec->item_durability_modifier);

		int half = it->max_durability / 2;
		if (it->quality == BAD_QUALITY) {
			//between 50% and 100%
			it->max_durability = max(1, half + MyRandom(half));
		} else if (it->quality == GOOD_QUALITY) {
			//between 100% and 150%
			it->max_durability +=  MyRandom(half);
		}

		if (full_durability) {
			it->current_durability = it->max_durability;
		} else {
			int quarter = it->max_durability / 4;
			//between 25% and 100%
			it->current_durability = max(1, quarter + MyRandom(quarter*3));
		}

	} else {
		it->max_durability = -1;
		it->current_durability = 1;
	}

	// Calculate the item bonuses affected by add-ons.
	// This will set the final armor rating and weapon damage, among other things.
	calculate_item_bonuses(it);
}

void append_item_name(item * ShowItem, struct auto_string *str)
{
	// If the item has upgrade sockets, use a different color for the name.
	if (ShowItem->upgrade_sockets.size) {
		autostr_append(str, "%s", font_switchto_blue);
	} else {
		autostr_append(str, "%s", font_switchto_neon);
	}

	if (item_spec_eq_id(ShowItem->type, "Valuable Circuits"))
		autostr_append(str, "%d ", ShowItem->multiplicity);

	autostr_append(str, "%s", D_(item_specs_get_name(ShowItem->type)));

	if (ShowItem->quality == GOOD_QUALITY) {
		autostr_append(str, "\n%s", font_switchto_blue);
		autostr_append(str, _("Good quality"));
	} else if (ShowItem->quality == BAD_QUALITY) {
		autostr_append(str, "\n%s", font_switchto_red);
		autostr_append(str, _("Bad quality"));
	}

	// Now that the item name is out, we can switch back to the standard font color...
	autostr_append(str, "%s", font_switchto_neon);
}

/**
 * This function drops an item at a given place.
 */
item *DropItemAt(int ItemType, int level_num, float x, float y, int multiplicity)
{
	gps item_pos;

	if (ItemType < 0 || ItemType >= Number_Of_Item_Types)
		error_message(__FUNCTION__, "\
Received item type %d that is outside the range of allowed item types.",
			     PLEASE_INFORM | IS_FATAL, ItemType);

	// Fix virtual position (e.g. from a dying robot)
	item_pos.x = x;
	item_pos.y = y;
	item_pos.z = level_num;
	if (!resolve_virtual_position(&item_pos, &item_pos))
		return NULL;

	// Construct the new item
	item tmp_item;
	init_item(&tmp_item);
	tmp_item.type = ItemType;
	FillInItemProperties(&tmp_item, FALSE, multiplicity);

	play_item_sound(ItemType, &item_pos);

	return drop_item(&tmp_item, item_pos.x, item_pos.y, item_pos.z);
}

static int get_random_item_type(int class)
{

	if (class > 9) {
		error_message(__FUNCTION__, "Random item class %d exceeds 9.", class, PLEASE_INFORM | IS_FATAL);
	}

	int a = MyRandom(item_count_per_class[class] - 1) + 1;

	//printf("Choosing in class %d among %d items, taking item %d\n", class, item_count_per_class[class], a);

	int i;
	for (i = 0; i < Number_Of_Item_Types; i++) {
		if (ItemMap[i].min_drop_class != -1) {
			if (ItemMap[i].min_drop_class <= class && ItemMap[i].max_drop_class >= class)
				a--;
			if (!a)
				break;
		}
	}

	if (a) {
		error_message(__FUNCTION__, "Looking for random item with class %d, a = %d after the loop.", PLEASE_INFORM | IS_FATAL, class,
			     a);
	}
	//printf("Dropping item %s (%d <= class <= %d), class %d\n", ItemMap[i].item_name, ItemMap[i].min_drop_class, ItemMap[i].max_drop_class, class);
	return i;
}

/**
 * This function drops a random item to the floor of the current level
 * at position ( x , y ).
 *
 * The strategy in dropping the item is that one can easily set up and
 * modify the table of items to be dropped.
 *
 */
void DropRandomItem(int level_num, float x, float y, int class, int force_magical)
{
	int DropDecision;
	int drop_item_type = 1;
	int drop_item_multiplicity = 1;

	// First we determine if there is something dropped at all or not,
	// cause in the later case, we can return immediately.  If a drop is
	// forced, we need not check for not do drop.
	//
	DropDecision = MyRandom(100);

	// We decide whether we drop something at all or not
	//
	if ((DropDecision < 100 - GOLD_DROP_PERCENTAGE) && (DropDecision > ITEM_DROP_PERCENTAGE))
		return;

	// Perhaps it's some gold that will be dropped.  That's rather
	// simple, so we do this first.
	//
	if ((DropDecision > 100 - GOLD_DROP_PERCENTAGE)) {
		// If class == 0, we want to avoid to drop 0-1 valuable circuits
		int how_many = (class == 0) ? 2 : 0;
		how_many += MONEY_PER_BOT_CLASS * class + MyRandom(MONEY_PER_BOT_CLASS - 1);
		DropItemAt(get_item_type_by_id("Valuable Circuits"), level_num, x, y, how_many);
	}

	if ((DropDecision < ITEM_DROP_PERCENTAGE)) {
		drop_item_type = get_random_item_type(class);

		// Determine the multiplicity for the item
		drop_item_multiplicity = ItemMap[drop_item_type].drop_amount + MyRandom(ItemMap[drop_item_type].drop_amount_max - ItemMap[drop_item_type].drop_amount);

		// Create the item and place it to the map. This can fail under certain
		// conditions so we need to check for errors and give up if one occurred.
		item *it = DropItemAt(drop_item_type, level_num, x, y, drop_item_multiplicity);
		if (!it) {
			return;
		}

		// Create sockets occasionally if the item is of a customizable type.
		int socket_drop_decision = MyRandom(100);
		if (item_can_be_customized(it) && (force_magical ||
		            socket_drop_decision < SOCKET_DROP_PERCENTAGE)) {
			// Decide how many sockets to create. We randomly index an array of
			// socket counts to implement a non-uniform probability distribution.
			const int create_count_array[] = { 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 4 };
			int random_index = MyRandom(10);
			int create_count = create_count_array[random_index];

			// Create the desired number of sockets of random types.
			while (create_count--) {
				int socket_type = MyRandom(UPGRADE_SOCKET_TYPE_UNIVERSAL);
				create_upgrade_socket(it, socket_type, NULL);
			}
		}
	}
}

/**
 * When the influencer gets hit, all of his equipment suffers some damage.
 * This is exactly what this function does:  apply the damage.
 */
void DamageItem(item * CurItem)
{

	// If the item mentioned as parameter exists and if it is of 
	// a destructable sort, then we apply the usual damage to it
	if ((CurItem->type != (-1)) && (CurItem->max_durability != (-1))) {
		CurItem->current_durability -= (MyRandom(100) < ARMOUR_DURABILITYLOSS_PERCENTAGE_WHEN_HIT) ? 1 : 0;
		
		// Make sound denoting some protective item was damaged
		BulletReflectedSound();

		// If the item has gone over its threshold of durability, it finally
		// breaks and vaporizes
		//
		if (rintf(CurItem->current_durability) <= 0) {
			DeleteItem(CurItem);
		}
	}

};				// void DamageItem( item* CurItem )

/* Now we do the same for a weapon that has been fired */
void DamageWeapon(item * CurItem)
{

	if ((CurItem->type != (-1)) && (CurItem->max_durability != (-1))) {
		CurItem->current_durability -= (MyRandom(100) < WEAPON_DURABILITYLOSS_PERCENTAGE_WHEN_USED) ? 1 : 0;
		if (rintf(CurItem->current_durability) <= 0) {
			DeleteItem(CurItem);
		}
	}

};				// void DamageWeapon( item* CurItem )

/**
 * When the influencer gets hit, some of his equipment might suffer some damage.
 * This is exactly what this function does:  apply the damage.
 */
void DamageProtectiveEquipment()
{
	int ItemHit = MyRandom(6);

	if (ItemHit < 2)
		DamageItem(&(Me.armour_item));
	else if (ItemHit < 4)
		DamageItem(&(Me.shield_item));
	else if (ItemHit < 5)
		DamageItem(&(Me.drive_item));
	else		
                DamageItem(&(Me.special_item));

};				// void DamageProtectiveEquipment( void )

/**
 * This function is used when an equipment EXCHANGE is performed, i.e.
 * one weapon equipped is replaced by a new item using the mouse.  This 
 * leads to an exchange in the items.  Yet, if the new item came from 
 * inventory, the old item can't be just put in the same place where the
 * new item was, cause it might be bigger.  So, attempting to solve the
 * problem, the old item from the slot can just be made into an item on
 * the floor, but not visible yet of course, cause it still gets the 
 * held in hand attribute.
 */
static void MakeHeldFloorItemOutOf(item * SourceItem)
{
	int i;

	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		if (CURLEVEL()->ItemList[i].type == (-1))
			break;
	}
	if (i >= MAX_ITEMS_PER_LEVEL) {
		DebugPrintf(0, "\n No free position to drop item!!! ");
		i = 0;
		Terminate(EXIT_FAILURE);
	}
	// Now we enter the item into the item list of this level
	//
	CopyItem(SourceItem, &(CURLEVEL()->ItemList[i]));

	CURLEVEL()->ItemList[i].pos.x = Me.pos.x;
	CURLEVEL()->ItemList[i].pos.y = Me.pos.y;
	CURLEVEL()->ItemList[i].pos.z = Me.pos.z;

	item_held_in_hand = &(CURLEVEL()->ItemList[i]);

	DeleteItem(SourceItem);
};				// void MakeHeldFloorItemOutOf( item* SourceItem )

/**
 * This function DELETES an item from the source location.
 */
void DeleteItem(item *it)
{
	delete_upgrade_sockets(it);
	init_item(it);
}

/**
 * This function COPIES an item from the source location to the destination
 * location.
 */
void CopyItem(item * SourceItem, item * DestItem)
{

	memcpy(DestItem, SourceItem, sizeof(item));

	// Create a soft copy of the upgrade sockets. Memcpy just copied the
	// pointer but we want the actual data to be duplicated.
	copy_upgrade_sockets(SourceItem, DestItem);
}

/**
 * This function MOVES an item from the source location to the destination
 * location.  The source location is then marked as unused inventory 
 * entry.
 */
void MoveItem(item *source_item, item *dest_item)
{
	if (source_item != dest_item) {
		memcpy(dest_item, source_item, sizeof(item));
		init_item(source_item);
	}
}

/**
 * This function applies a given item (to the influencer) and maybe 
 * eliminates the item after that, if it's an item that gets used up.
 */
void Quick_ApplyItem(int ItemKey)
{
	int FoundItemNr;

	if (ItemKey == 0) {
		//quick_inventory0 is slot 10
		ItemKey = 10;
	}

	FoundItemNr = GetInventoryItemAt(ItemKey - 1, INVENTORY_GRID_HEIGHT - 1);
	if (FoundItemNr == (-1))
		return;

	ApplyItem(&(Me.Inventory[FoundItemNr]));

};				// void Quick_ApplyItem( item* CurItem )

/**
 * This function checks whether a given item has the name specified. This is
 * used to match an item which its type in a flexible way (match by name instead
 * of matching by index value)
 */
int get_item_type_by_id(const char *id)
{
	int cidx = 0;
	for (; cidx < Number_Of_Item_Types; cidx++) {
		if (!strcmp(ItemMap[cidx].id, id))
			return cidx;
	}

	error_message(__FUNCTION__, "Unable to find item id %s", PLEASE_INFORM, id);
	return -1;
}

/**
 * This function checks whether a given item has the name specified. This is
 * used to match an item which its type in a flexible way (match by name instead
 * of matching by index value)
 */
int item_spec_eq_id(int type, const char *id)
{
	if (type < 0 || type >= Number_Of_Item_Types)
		return FALSE;

	if (!strcmp(ItemMap[type].id, id))
		return TRUE;
	else
		return FALSE;
}

/**
 * This function applies a given item (to the influencer) and maybe 
 * eliminates the item after that, if it's an item that gets used up.
 */
void ApplyItem(item * CurItem)
{
	int failed_usage = 0; // if an item cannot be applied, to not remove it from inventory

	// If the inventory slot is not at all filled, we need not do anything more...
	if (CurItem->type < 0)
		return;

	if (!ItemMap[CurItem->type].right_use.tooltip) {
		Me.TextVisibleTime = 0;
		Me.TextToBeDisplayed = _("I can't use this item here.");
		return;
	}

	// Forbid using items while paralyzed
	if (Me.paralyze_duration) {
		append_new_game_message(_("You can not use any items while paralyzed."));
		return;
	}

	if (Me.busy_time > 0) {
		char *msg;
		switch (Me.busy_type) {
		case DRINKING_POTION:
			msg = _("You are drinking a potion!");
			break;
		case WEAPON_FIREWAIT:
			msg = _("Your are waiting for your weapon to fire again!");
			break;
		case WEAPON_RELOAD:
			msg = _("You are reloading your weapon!");
			break;
		case THROWING_GRENADE:
			msg = _("You are throwing a grenade!");
			break;
		case RUNNING_PROGRAM:
			msg = _("You are running a program!");
			break;
		case TAKING_PILL:
			msg = _("You are taking a pill!");
			break;
		default:
			msg = _("You are doing something so weird the game does not understand what it is");
		}
		// TRANSLATORS: the trailing %s is what the player is currently already doing (drinking, running a program, ...)
		append_new_game_message(_("How do you expect to do two things at a time? %s"), msg);
		return;		//if the player is busy reloading or anything
	}
	// At this point we know that the item is applicable in combat situation
	// and therefore all we need to do from here on is execute the item effect
	// upon the influencer or his environment.
	//
	if (item_spec_eq_id(CurItem->type, "Barf's Energy Drink")) {
		Me.energy += 15;
		Me.temperature -= 15;
		Me.running_power += 15;
	} else if (item_spec_eq_id(CurItem->type, "Diet supplement")) {
		Me.energy += 25;
		play_sound_cached("effects/new_healing_sound.ogg");
	} else if (item_spec_eq_id(CurItem->type, "Antibiotic")) {
		Me.energy += 50;
		play_sound_cached("effects/new_healing_sound.ogg");
	} else if (item_spec_eq_id(CurItem->type, "Doc-in-a-can")) {
		Me.energy += Me.maxenergy;
		play_sound_cached("effects/new_healing_sound.ogg");
	} else if (item_spec_eq_id(CurItem->type, "Bottled ice")) {
		Me.temperature -= 50;
	} else if (item_spec_eq_id(CurItem->type, "Industrial coolant")) {
		Me.temperature -= 100;
	} else if (item_spec_eq_id(CurItem->type, "Liquid nitrogen")) {
		Me.temperature = 0;
	} else if (item_spec_eq_id(CurItem->type, "Running Power Capsule")) {
		Me.running_power = Me.max_running_power;
		Me.running_must_rest = FALSE;
	} else if (item_spec_eq_id(CurItem->type, "Strength Capsule")) {
		Me.current_power_bonus = 30;
		Me.power_bonus_end_date = Me.current_game_date + 2.0 * 60;
	} else if (item_spec_eq_id(CurItem->type, "Dexterity Capsule")) {
		Me.current_dexterity_bonus = 30;
		Me.dexterity_bonus_end_date = Me.current_game_date + 2.0 * 60;
	} else if (item_spec_eq_id(CurItem->type, "Map Maker")) {
		Me.map_maker_is_present = TRUE;
		GameConfig.Automap_Visible = TRUE;
		Play_Spell_ForceToEnergy_Sound();
	} else if (item_spec_eq_id(CurItem->type, "Strength Pill")) {
		Me.base_strength++;
	} else if (item_spec_eq_id(CurItem->type, "Dexterity Pill")) {
		Me.base_dexterity++;
	} else if (item_spec_eq_id(CurItem->type, "Code Pill")) {
		Me.base_cooling++;
	} else if (item_spec_eq_id(CurItem->type, "Brain Enlargement Pill")) {
		Me.base_cooling = 5;
		Me.base_strength = 5;
		Me.base_dexterity = 5;
		Me.base_physique = 5;
		Takeover_Game_Lost_Sound();
		append_new_game_message(_("The doctor warned you. You are now weak and sickly."));
	}

	// Do the skill
	if (ItemMap[CurItem->type].right_use.skill) {
		failed_usage = !DoSkill(get_program_index_with_name(ItemMap[CurItem->type].right_use.skill), 0);
	// Improve the skill
	} else if (ItemMap[CurItem->type].right_use.add_skill) {
		failed_usage = improve_program(get_program_index_with_name(ItemMap[CurItem->type].right_use.add_skill));

		if(failed_usage == 0) {
			Play_Spell_ForceToEnergy_Sound();
		} else {
			// TRANSLATORS: the trailing %s is a program name
			append_new_game_message(_("You have reached the maximum skill level for %s"), 
									D_(ItemMap[CurItem->type].right_use.add_skill));
			Takeover_Game_Deadlock_Sound();
		}
	}

	if (!failed_usage) {

		play_item_sound(CurItem->type, &Me.pos);

		// Apply busy time and busy type
		Me.busy_time = ItemMap[CurItem->type].right_use.busy_time;
		Me.busy_type = ItemMap[CurItem->type].right_use.busy_type;

		// In some cases the item concerned is a one-shot-device like a health potion, which should
		// evaporize after the first application.  Therefore we delete the item from the inventory list.
		//
		if (CurItem->multiplicity > 1)
			CurItem->multiplicity--;
		else
			DeleteItem(CurItem);
	}

	while (MouseRightPressed())
		SDL_Delay(1);
}

/**
 * This function checks if a given coordinate within the influencers
 * inventory grid can be considered as free or as occupied by some item.
 */
int Inv_Pos_Is_Free(int x, int y)
{
	int i;
	int item_width;
	int item_height;

	for (i = 0; i < MAX_ITEMS_IN_INVENTORY - 1; i++) {
		if (Me.Inventory[i].type == (-1))
			continue;

		if (item_held_in_hand == &Me.Inventory[i])
			continue;

		// for ( item_height = 0 ; item_height < ItemSizeTable[ Me.Inventory[ i ].type ].y ; item_height ++ )
		for (item_height = 0; item_height < ItemMap[Me.Inventory[i].type].inv_size.y; item_height++) {
			for (item_width = 0; item_width < ItemMap[Me.Inventory[i].type].inv_size.x; item_width++) {
				if (((Me.Inventory[i].inventory_position.x + item_width) == x) &&
				    ((Me.Inventory[i].inventory_position.y + item_height) == y))
					return (FALSE);
			}
		}
	}
	return TRUE;

};				// int Inv_Pos_Is_Free( Inv_Loc.x , Inv_Loc.y )

/**
 * This function returns the index in the inventory list of the object
 * at the inventory position x y.  If no object is found to occupy that
 * square, an index of (-1) is returned.
 * 
 * NOTE: The mentioned coordinates refer to the squares of the inventory grid!!
 *
 */
int GetInventoryItemAt(int x, int y)
{
	int i;
	int item_width;
	int item_height;

	for (i = 0; i < MAX_ITEMS_IN_INVENTORY - 1; i++) {
		if (Me.Inventory[i].type == (-1))
			continue;

		for (item_height = 0; item_height < ItemMap[Me.Inventory[i].type].inv_size.y; item_height++) {
			for (item_width = 0; item_width < ItemMap[Me.Inventory[i].type].inv_size.x; item_width++) {
				if (((Me.Inventory[i].inventory_position.x + item_width) == x) &&
				    ((Me.Inventory[i].inventory_position.y + item_height) == y)) {
					return (i);
				}
			}
		}
	}
	return (-1);		// Nothing found at this grabbing location!!

};				// int GetInventoryItemAt ( int x , int y )

/**
 *
 * Often, especially in dialogs and in order to determine if some answer
 * should be allowed for the Tux or not, it is important to know if the
 * Tux has some special item of a given type in inventory or not and also
 * how many of those items the Tux really has.  
 *
 * This function is now intended to count the number of items of a given
 * type in the inventory of the Me.
 *
 */
int CountItemtypeInInventory(int Itemtype)
{
	int i;
	int NumberOfItemsFound = 0;

	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		if (Me.Inventory[i].type == Itemtype)
			NumberOfItemsFound += Me.Inventory[i].multiplicity;
	}
	return NumberOfItemsFound;

};				// int CountItemtypeInInventory( int Itemtype )

/**
 *
 *
 */
static int FindFirstInventoryIndexWithItemType(int Itemtype)
{
	int i;

	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		if (Me.Inventory[i].type == Itemtype)
			return (i);
	}

	// Severe error:  Item type NOT found in inventory!!!
	//
	fprintf(stderr, "\n\nItemType: '%d'.\n", Itemtype);
	error_message(__FUNCTION__, "\
There was an item code for an item to locate in inventory, but inventory\n\
did not contain this item type at all!  This indicates a severe bug in FreedroidRPG.", PLEASE_INFORM | IS_FATAL);

	return (-1);

};				// int FindFirstInventoryIndexWithItemType ( ItemPointer->type , PLAYER_NR_0 )

/**
 * At some point the Tux will hand over all his items of a given type
 * to a dialog partner.  This function is intended to do exactly this:
 * To remove all items of a given type from the inventory of a given 
 * player.
 */
void DeleteInventoryItemsOfType(int Itemtype, int amount)
{
	int i;
	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		if (Me.Inventory[i].type == Itemtype) {
			if (Me.Inventory[i].multiplicity > amount)
				Me.Inventory[i].multiplicity -= amount;
			else
				DeleteItem(&(Me.Inventory[i]));
			return;
		}
	}
};				// void DeleteInventoryItemsOfType( int Itemtype )

/**
 * This deletes ONE item of the given type, like one bullet that has 
 * just been expended.
 */
void DeleteOneInventoryItemsOfType(int Itemtype)
{
	int i;
	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		if (Me.Inventory[i].type == Itemtype) {
			if (Me.Inventory[i].multiplicity > 1)
				Me.Inventory[i].multiplicity--;
			else
				DeleteItem(&(Me.Inventory[i]));
			return;
		}
	}

	// This point must never be reached or a severe error has occurred...
	//
	fprintf(stderr, "\n\nItemType: '%d'.\n", Itemtype);
	error_message(__FUNCTION__, "\
One single item of all the items of a given type in the Tux inventory\n\
should be removed, but there was not even one such item ever found in\n\
Tux inventory.  Something must have gone awry...", PLEASE_INFORM | IS_FATAL);

};				// void DeleteOneInventoryItemsOfType( int Itemtype  )

static int MouseCursorIsInSkiERect(int x, int y)
{
	if (x > 320 || x < 0)
		return FALSE;
	if (y > 480 || y < 0)
		return FALSE;

	return TRUE;
}

/**
 * This function checks if a given screen position lies within the inventory
 * rectangle or not.
 */
int MouseCursorIsInInvRect(int x, int y)
{
	if (!GameConfig.Inventory_Visible)
		return FALSE;
	if (x > InventoryRect.x + InventoryRect.w)
		return (FALSE);
	if (x < InventoryRect.x)
		return (FALSE);
	if (y > InventoryRect.y + InventoryRect.h)
		return (FALSE);
	if (y < InventoryRect.y)
		return (FALSE);
	return TRUE;
};				// int MouseCursorIsInInvRect( int x , int y )

/**
 * This function checks if a given screen position lies within the character
 * rectangle or not.
 */
int MouseCursorIsInChaRect(int x, int y)
{
	if (!GameConfig.CharacterScreen_Visible)
		return FALSE;
	if (x > CharacterRect.x + CharacterRect.w)
		return (FALSE);
	if (x < CharacterRect.x)
		return (FALSE);
	if (y > CharacterRect.y + CharacterRect.h)
		return (FALSE);
	if (y < CharacterRect.y)
		return (FALSE);
	return TRUE;
};				// int MouseCursorIsInChaRect( int x , int y )

/**
 * This function checks if a given screen position lies within the skill
 * rectangle or not.
 */
int MouseCursorIsInSkiRect(int x, int y)
{
	if (!GameConfig.SkillScreen_Visible)
		return FALSE;
	if (x > SkillScreenRect.x + SkillScreenRect.w)
		return (FALSE);
	if (x < SkillScreenRect.x)
		return (FALSE);
	if (y > SkillScreenRect.y + SkillScreenRect.h)
		return (FALSE);
	if (y < SkillScreenRect.y)
		return (FALSE);
	return TRUE;
};				// int MouseCursorIsInSkiRect( int x , int y )

/**
 * This function checks if a given screen position lies within the grid
 * where the inventory of the player is usually located or not.
 */
int MouseCursorIsInInventoryGrid(int x, int y)
{
	if (!GameConfig.Inventory_Visible)
		return FALSE;
	if ((x >= INVENTORY_RECT_X) && (x <= INVENTORY_RECT_X + INVENTORY_GRID_WIDTH * INV_SUBSQUARE_WIDTH)) {
		if ((y >= User_Rect.y + INVENTORY_RECT_Y) &&
		    (y <= User_Rect.y + INVENTORY_RECT_Y + INV_SUBSQUARE_HEIGHT * INVENTORY_GRID_HEIGHT)) {
			return TRUE;
		}
	}
	return (FALSE);
};				// int MouseCursorIsInInventoryGrid( int x , int y )

/**
 * This function checks if a given screen position lies within the user
 * i.e. combat rectangle or not.
 */
int MouseCursorIsInUserRect(int x, int y)
{
	// no interaction with the game when the world is frozen
	if (world_frozen())
		return FALSE;

	if (y < User_Rect.y)
		return (FALSE);
	if (y > User_Rect.y + User_Rect.h)
		return (FALSE);

	if ((!GameConfig.Inventory_Visible) && (!GameConfig.CharacterScreen_Visible) && (!GameConfig.SkillScreen_Visible)) {
		if (x > User_Rect.x + User_Rect.w)
			return (FALSE);
		if (x < User_Rect.x)
			return (FALSE);
		return TRUE;
	}
	if ((GameConfig.Inventory_Visible && MouseCursorIsInInvRect(x, y))
	    || (GameConfig.CharacterScreen_Visible && MouseCursorIsInChaRect(x, y)) || (GameConfig.SkillScreen_Visible
											&& MouseCursorIsInSkiRect(x, y))
	    || (GameConfig.skill_explanation_screen_visible && MouseCursorIsInSkiERect(x, y)))
		return FALSE;
	return TRUE;
};				// int MouseCursorIsInUserRect( int x , int y )

/**
 * This function gives the x coordinate of the inventory square that 
 * corresponds to the mouse cursor location given to the function.
 */
int GetInventorySquare_x(int x)
{
	return ((x - INVENTORY_RECT_X) / INV_SUBSQUARE_WIDTH);
};				// int GetInventorySquare_x( x )

/**
 * This function gives the y coordinate of the inventory square that 
 * corresponds to the mouse cursor location given to the function.
 */
int GetInventorySquare_y(int y)
{
	return ((y - (User_Rect.y + INVENTORY_RECT_Y)) / INV_SUBSQUARE_HEIGHT);
};				// int GetInventorySquare_y( y )

/**
 * This function checks if a given item type could be dropped into the 
 * inventory grid at location x y.  Only the space is taken into account
 * and if other items block the way or not.
 */
int ItemCanBeDroppedInInv(int ItemType, int InvPos_x, int InvPos_y)
{
	int item_height;
	int item_width;

	// Perhaps the item reaches even outside the inventory grid.  Then of course
	// it does not fit and we need/should not even test the details...
	//
	if (InvPos_x < 0 || InvPos_y < 0)
		return FALSE;
	if (ItemMap[ItemType].inv_size.x - 1 + InvPos_x >= INVENTORY_GRID_WIDTH)
		return (FALSE);
	if (ItemMap[ItemType].inv_size.y - 1 + InvPos_y >= INVENTORY_GRID_HEIGHT)
		return (FALSE);

	// Now that we know, that the desired position is at least inside the inventory
	// grid, we can start to test for the details of the available inventory space
	//
	for (item_height = 0; item_height < ItemMap[ItemType].inv_size.y; item_height++) {
		for (item_width = 0; item_width < ItemMap[ItemType].inv_size.x; item_width++) {
			if (!Inv_Pos_Is_Free(InvPos_x + item_width, InvPos_y + item_height))
				return (FALSE);
		}
	}
	return TRUE;

};				// int ItemCanBeDroppedInInv ( int ItemType , int InvPos_x , int InvPos_y )

/**
 * Find a free index in the item array of the drop level.
 */
static int find_free_floor_index(level* drop_level)
{
	int i;
	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		if (drop_level->ItemList[i].type == -1) {
			return i;
		}
	}

	// We did not find a free index.
	error_message(__FUNCTION__, "The item array for level %d was full.",
	            PLEASE_INFORM, drop_level->levelnum);
	return -1;
}

/**
 * Drop an item to the floor in the given location.  No checks are done to
 * verify this location is unobstructed or otherwise reasonable.
 */
item *drop_item(item *item_pointer, float x, float y, int level_num)
{
	level *drop_level = curShip.AllLevels[level_num];

	int index = find_free_floor_index(drop_level);

	// Cancel the drop if we did not find an empty index to use.
	if (index == -1) {
		return NULL;
	}

	// Create the item
	init_item(&(drop_level->ItemList[index]));
	MoveItem(item_pointer, &(drop_level->ItemList[index]));

	// Place item on level
	drop_level->ItemList[index].inventory_position.x = -1;
	drop_level->ItemList[index].inventory_position.y = -1;
	drop_level->ItemList[index].pos.x = x;
	drop_level->ItemList[index].pos.y = y;
	drop_level->ItemList[index].pos.z = level_num;
	drop_level->ItemList[index].throw_time = 0.01;  // something > 0

	timeout_from_item_drop = 0.4;

	if (item_pointer == item_held_in_hand)
		item_held_in_hand = NULL;

	return &(drop_level->ItemList[index]);
}

/**
 * Drop held item to the floor.
 *
 * Before calling this function, make sure item_held_in_hand != NULL
 */
static void drop_held_item(void)
{
	float x = translate_pixel_to_map_location(input_axis.x, input_axis.y, TRUE);
	float y = translate_pixel_to_map_location(input_axis.x, input_axis.y, FALSE);

	gps pos = { x, y, Me.pos.z };
	gps rpos;

	/* The vector from Tux to the map position where the player clicked */
	float ax = x - Me.pos.x;
	float ay = y - Me.pos.y;
	/* The length of mentioned vector */
	float length = sqrt(ax * ax + ay * ay);
	/* The vector with the same direction but length ITEM_TAKE_DIST */
	float ux = ITEM_TAKE_DIST * (ax / length);
	float uy = ITEM_TAKE_DIST * (ay / length);

	/* Don't let the player drop the item farther than ITEM_TAKE_DIST */
	if (length > ITEM_TAKE_DIST) {
		pos.x = Me.pos.x + ux;
		pos.y = Me.pos.y + uy;
	}
	
	/* If we have an invalid drop position, attempt positions closer to Tux
	 * until we have a good position. */
	colldet_filter margin = WalkableWithMarginPassFilter;
	margin.extra_margin = 0.2;
	int collision;
	while ((collision = !(resolve_virtual_position(&rpos, &pos) &&
						  DirectLineColldet(Me.pos.x, Me.pos.y, pos.x, pos.y, Me.pos.z, &margin)))
		   && sqrt(ux * ux + uy * uy) > 0.1) // length > 0.1
	{
		ux *= 0.9;
		uy *= 0.9;
		pos.x = Me.pos.x + ux;
		pos.y = Me.pos.y + uy;
	}

	// Fall back to Tux's feet if the position is still invalid
	if (collision) {
		rpos.x = Me.pos.x;
		rpos.y = Me.pos.y;
		rpos.z = Me.pos.z;
	}

	// Finally, drop the item
	drop_item(item_held_in_hand, rpos.x, rpos.y, rpos.z);
	item_held_in_hand = NULL;
	timeout_from_item_drop = 0.4;
}

/**
 * This function checks if the usage requirements for a given item are
 * met by the influencer or not.
 */
int ItemUsageRequirementsMet(item * UseItem, int MakeSound)
{
	if (Me.strength < ItemMap[UseItem->type].item_require_strength && ItemMap[UseItem->type].item_require_strength > 0) {
		if (MakeSound)
			Not_Enough_Power_Sound();
		return (FALSE);
	}
	if (Me.dexterity < ItemMap[UseItem->type].item_require_dexterity && ItemMap[UseItem->type].item_require_dexterity > 0) {
		if (MakeSound)
			Not_Enough_Dist_Sound();
		return (FALSE);
	}
	if (Me.cooling < ItemMap[UseItem->type].item_require_cooling && ItemMap[UseItem->type].item_require_cooling > 0) {
		return (FALSE);
	}
	return TRUE;
};				// int ItemUsageRequirementsMet( item* UseItem )

/**
 * This function checks, if the influencer mets the requirements of the
 * item currently held in hand by the player/influencer.  Which item this
 * is will be found out by the function.
 */
static int HeldItemUsageRequirementsMet(void)
{
	// Check validity of HeldItem
	if (item_held_in_hand == NULL) {
		DebugPrintf(0, "\nvoid HeldItemUsageRequirementsMet ( void ) : No item in inventory seems to be currently held in hand...");
		return (FALSE);		
	}
	
	return (ItemUsageRequirementsMet(item_held_in_hand, TRUE));
};				// int HeldItemUsageRequirementsMet( void )

/**
 * This function installs an item into a slot.  The given parameter is 
 * only the slot where this item should be installed.  The source item
 * will be found out from inside this function.  Very convenient.
 */
static void DropHeldItemToSlot(item * SlotItem)
{
	item *DropItemPointer; // temporary storage

	// Chech validity of held item
	if (item_held_in_hand == NULL) {
		DebugPrintf(0, "\nvoid DropHeldItemToSlot ( void ) : No item in inventory seems to be currently held in hand...");
		return;		
	}

	// If there is an old item in the slot, we make a held item on the
	// floor out of it and also set the HeldItemType accordingly, so that
	// after the new item was placed successfully, the old item will
	// be out of all inventory slots, but still in the hand of the 
	// player and ready to be put somewhere else
	//
	// But this may only be done of course, if the 'old item' is not
	// the item we want to put there itself!!!!  HAHAHAHA!!!!
	//
	DropItemPointer = item_held_in_hand;
	if ((SlotItem->type != (-1)) && (item_held_in_hand != SlotItem))
		MakeHeldFloorItemOutOf(SlotItem);
	else
		item_held_in_hand = NULL;
	
	// Move the item to the slot and mark it as no longer grabbed.
	MoveItem(DropItemPointer, SlotItem);
	play_item_sound(SlotItem->type, &Me.pos);
}

/**
 * This function looks for a free inventory index.  Since there are more
 * inventory indices than squares in the inventory grid, the function 
 * should always be able to find a free inventory index.  If not, this is
 * considered a severe program error, which will cause immediate 
 * termination of FreedroidRPG.
 */
int GetFreeInventoryIndex(void)
{
	int InvPos;

	// We find out the first free inventory index:
	//
	for (InvPos = 0; InvPos < MAX_ITEMS_IN_INVENTORY - 1; InvPos++) {
		if (Me.Inventory[InvPos].type == (-1)) {
			return (InvPos);
		}
	}

	// If this point is reached, the severe error mentioned above has
	// occurred, an error message must be printed out and the program
	// must be terminated.
	//
	error_message(__FUNCTION__, "\
A FREE INVENTORY INDEX POSITION COULD NOT BE FOUND.\n\
This is an internal error, that must never happen unless there are\n\
severe bugs in the inventory system.", PLEASE_INFORM | IS_FATAL);
	return (-1);		// just to make compilers happy.
};				// int GetFreeInventoryIndex( void )

/**
 * If an item is held and then clicked again in the inventory field, this
 * item should be dropped into the inventory field, provided there is room
 * enough in it at that location.  If that is the case, then the item is
 * dropped onto this inventory location, else nothing is done.
 */
void DropHeldItemToInventory(void)
{
	point CurPos;
	int FreeInvIndex;
	int i;
	FreeInvIndex = GetFreeInventoryIndex();

	// First we check validity of held item
	//
	if (item_held_in_hand == NULL) {
		DebugPrintf(0, "\nvoid DropHeldItemToInventory ( void ) : No item in inventory seems to be currently held in hand...");
		return;
	}

	// Now we want to drop the item to the right location again.
	// Therefore we need to find out the right position, which of course
	// depends as well on current mouse cursor location as well as the
	// size of the dropped item.
	//
	CurPos.x = GetMousePos_x() - (16 * (ItemMap[item_held_in_hand->type].inv_size.x - 1));
	CurPos.y = GetMousePos_y() - (16 * (ItemMap[item_held_in_hand->type].inv_size.y - 1));

	if (ItemCanBeDroppedInInv(item_held_in_hand->type, GetInventorySquare_x(CurPos.x), GetInventorySquare_y(CurPos.y))) {
		CopyItem(item_held_in_hand, &(Me.Inventory[FreeInvIndex]));
		play_item_sound(item_held_in_hand->type, &Me.pos);
		Me.Inventory[FreeInvIndex].inventory_position.x = GetInventorySquare_x(CurPos.x);
		Me.Inventory[FreeInvIndex].inventory_position.y = GetInventorySquare_y(CurPos.y);

		// Now that we know that the item could be dropped directly to inventory 
		// without swapping any spaces, we can as well make the item
		// 'not held in hand' immediately and return
		//
		DeleteItem(item_held_in_hand);
		item_held_in_hand = NULL;
		return;
	} else {
		// So the item could not be placed into inventory directly, but maybe
		// it can be placed there if we swap our dropitem with some other item.
		// Let's test this opportunity here.
		//
		for (i = 0; i < MAX_ITEMS_IN_INVENTORY - 1; i++) {
			// FIRST: Security check against segfaults:  It might happen that we 
			// delete the Dropitem itself while trying several items as candidates
			// for removal.  This would cause testing dropability with a -1 item
			// type and a SEGFAULT would result...
			//
			if (&(Me.Inventory[i]) == item_held_in_hand)
				continue;

			// So we make a copy of each of the items we remove in order to 
			// try to create new space for the drop item.  After that, we can
			// remove it.
			//
			CopyItem(&(Me.Inventory[i]), &(Me.Inventory[MAX_ITEMS_IN_INVENTORY - 1]));
			Me.Inventory[i].type = (-1);

			if (ItemCanBeDroppedInInv(item_held_in_hand->type, GetInventorySquare_x(CurPos.x), GetInventorySquare_y(CurPos.y))) {
				
				// Copy the HelItem to the now free position
				CopyItem(item_held_in_hand, &(Me.Inventory[FreeInvIndex]));
				play_item_sound(item_held_in_hand->type, &Me.pos);
				Me.Inventory[FreeInvIndex].inventory_position.x = GetInventorySquare_x(CurPos.x);
				Me.Inventory[FreeInvIndex].inventory_position.y = GetInventorySquare_y(CurPos.y);
				DeleteItem(item_held_in_hand);

				// The removed item Nr. i is put in hand in replacement of the
				// prior HeldItem.
				MakeHeldFloorItemOutOf(&(Me.Inventory[MAX_ITEMS_IN_INVENTORY - 1]));

				return;
			}

			// But if even the removal of one item was not enough, so that the new
			// item would fit into the inventory, then of course we should re-add the
			// removed item to the inventory, so that no other items get lost.
			//
			CopyItem(&(Me.Inventory[MAX_ITEMS_IN_INVENTORY - 1]), &(Me.Inventory[i]));

		}		// for: try all items if removal is the solution
	}			// if not immediately place findable
};				// void DropHeldItemToInventory( void )

/**
 *
 *
 */
int get_floor_item_index_under_mouse_cursor(level **item_lvl)
{
	gps mouse_pos;
	int i;

	// no interaction with the game when the world is frozen
	if (world_frozen())
		return -1;

	// In the case that X was pressed, we don't use the item positions but rather
	// we use the item slot rectangles from the item texts.
	//
	if (XPressed() || GameConfig.show_item_labels) {
		struct visible_level *vis_lvl, *n;
		
		BROWSE_VISIBLE_LEVELS(vis_lvl, n) {	
			level *lvl = vis_lvl->lvl_pointer;

			for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
				if (lvl->ItemList[i].type == (-1))
					continue;
	
				if (MouseCursorIsInRect(&(lvl->ItemList[i].text_slot_rectangle), GetMousePos_x(), GetMousePos_y())) {
					*item_lvl = lvl;
					return (i);
				}
			}
		}
	}
	// If no X was pressed, we only use the floor position the mouse
	// has pointed to and see if we can find an item that has geographically
	// that very same (or a similar enough) position.
	//
	else {
		mouse_pos.x = translate_pixel_to_map_location(input_axis.x, input_axis.y, TRUE);
		mouse_pos.y = translate_pixel_to_map_location(input_axis.x, input_axis.y, FALSE);
		mouse_pos.z = Me.pos.z;
		
		gps virt_mouse_pos;
		struct visible_level *vis_lvl, *n;
		
		BROWSE_VISIBLE_LEVELS(vis_lvl, n) {
			
			level *lvl = vis_lvl->lvl_pointer;
			update_virtual_position(&virt_mouse_pos, &mouse_pos, lvl->levelnum);
			
			for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
				if (lvl->ItemList[i].type == (-1))
					continue;
	
				if ((fabsf(virt_mouse_pos.x - lvl->ItemList[i].pos.x) < 0.5) &&
					(fabsf(virt_mouse_pos.y - lvl->ItemList[i].pos.y) < 0.5)) {
					*item_lvl = lvl;
					return (i);
				}
			}
		}
	}

	return (-1);
}

/**
 * Handle inventory screen and related things: interact with items in inventory
 * grid, in inventory slots and in hand. Also handle dropping items in hand and
 * apply (right click) items.
 */
void HandleInventoryScreen(void)
{
	point CurPos;
	
	struct {
		int buttonidx;
		item *slot;
	} allslots[] = {		/*list of all slots and their associated item */
			{ WEAPON_RECT_BUTTON, &(Me.weapon_item) }, 
			{ DRIVE_RECT_BUTTON, &(Me.drive_item) },
		   	{ SHIELD_RECT_BUTTON, &(Me.shield_item) }, 
			{ ARMOUR_RECT_BUTTON, &(Me.armour_item) }, 
			{ HELMET_RECT_BUTTON, &(Me.special_item) },
	};

	if (Me.energy <= 0) {
		return;
	}
	
	// If the inventory is not visible there is nothing to do 
	if (GameConfig.Inventory_Visible == FALSE) {
		item_held_in_hand = NULL;
		return;
	}

	// We will need the current mouse position on several spots...
	CurPos.x = GetMousePos_x();
	CurPos.y = GetMousePos_y();

	// Case 1: The user left-clicks while not holding an item
	if (MouseLeftClicked() && item_held_in_hand == NULL) {

		// Forbid using the inventory while paralyzed
		if (Me.paralyze_duration) {
			append_new_game_message(_("You can not use the inventory while paralyzed."));
			return;
		}

		// Case 1.1: The user left-clicks on the inventory grid
		if (MouseCursorIsInInventoryGrid(CurPos.x, CurPos.y)) {
			point Inv_GrabLoc;
			int Grabbed_InvPos;

			Inv_GrabLoc.x = GetInventorySquare_x(CurPos.x);
			Inv_GrabLoc.y = GetInventorySquare_y(CurPos.y);
			Grabbed_InvPos = GetInventoryItemAt(Inv_GrabLoc.x, Inv_GrabLoc.y);
			
			if (Grabbed_InvPos == (-1)) {
				/* No item under the cursor */
				return;
			}
			
			// At this point we know, that we have just grabbed something from the inventory
			// So we set, that something should be displayed in the 'hand', and it should of
			// course be the image of the item grabbed from inventory.
			item_held_in_hand = &(Me.Inventory[Grabbed_InvPos]);
			play_item_sound(item_held_in_hand->type, &Me.pos);

			return;
		}
		
		// Case 1.2: The user left-clicks on one of the equipment slots
		unsigned int i;

		for (i = 0; i < sizeof(allslots) / sizeof(allslots[0]); i++) {
			if (MouseCursorIsOnButton(allslots[i].buttonidx, CurPos.x, CurPos.y)) {
				if (allslots[i].slot->type > 0) {
					MakeHeldFloorItemOutOf(allslots[i].slot);
					return;
				}
			}
		}

		// Case 1.3: The user left-clicks on an item on the floor
		int item_idx;
		level *item_lvl = NULL;

		if (MouseCursorIsInUserRect(GetMousePos_x(), GetMousePos_y()) 
		   && (item_idx = get_floor_item_index_under_mouse_cursor(&item_lvl)) != -1) {
			// Try to auto-put or auto-equip the item. If it's not possible,
			// the item will be 'put in hand'.
			if (check_for_items_to_pickup(item_lvl, item_idx)) {
				item *it = &item_lvl->ItemList[item_idx];
				if (!try_give_item(it)) {
					item_held_in_hand = it;
				}
			}
			return;
		}
		
		// No item was picked
		return;
	}
	
	// Case 2: The user left-clicks somewhere to drop a held item
	//
	if (MouseLeftClicked() && (item_held_in_hand != NULL)) {
		
		// The left-click is on the inventory grid -> we must see if 
		// the item was dropped onto a correct inventory location and 
		// should from then on not only no longer be in the players
		// hand but also remain at the newly assigned position.
		if (MouseCursorIsInInventoryGrid(CurPos.x, CurPos.y)) {
			DropHeldItemToInventory();
			return;
		}

		// The user left-clicks in the weapon's equipment slot or the bottom left corner
		// HUD weapon display.
		if (MouseCursorIsOnButton(WEAPON_RECT_BUTTON, CurPos.x, CurPos.y)
			|| MouseCursorIsOnButton(WEAPON_MODE_BUTTON, CurPos.x, CurPos.y)) {
			
			// Check if the item can be installed in the weapon slot
			if (ItemMap[item_held_in_hand->type].slot != WEAPON_SLOT) {
				append_new_game_message(_("You cannot fight with this!"));
				return;
			}
			
			// Check if the user has enough skill to use the weapon
			if (!HeldItemUsageRequirementsMet()) {
				append_new_game_message(_("You cannot yet fight with this!"));
				return;
			}
	
			// Now a weapon is about to be dropped to the weapons rectangle and obviously
			// the stat requirements for usage are met.  But maybe this is a 2-handed weapon.
			// In this case we need to do some extra check.  If it isn't a 2-handed weapon,
			// then we can just go ahead and equip the item
			if (!ItemMap[item_held_in_hand->type].item_gun_requires_both_hands) {
				DropHeldItemToSlot(&(Me.weapon_item));
				return;
			}
			
			// So, this is a 2-handed weapon. If the shield slot is just empty, 
			// that makes matters a lot simpler, because then we can just drop 
			// this 2-handed weapon to the weapon slot and all is fine, because 
			//no conflicts will result...
			if (Me.shield_item.type == (-1)) {
				DropHeldItemToSlot(&(Me.weapon_item));
				return;
			}
			
			// But if there is something in the shield slot too, then we need to be
			// a bit more sophisticated and either swap the 2-handed item in for just
			// the shield alone, which then will be held OR we need to refuse completely
			// because there might be a weapon AND a shield equipped already.
			if (Me.weapon_item.type == (-1)) {
				// first of all check requirements again but without the shield :
				// virtually remove the shield, compute requirements, if 
				// everything is okay, proceed otherwise we inform the player
				int shield_item_type = Me.shield_item.type;
				Me.shield_item.type = (-1);
				update_all_primary_stats();
				if (HeldItemUsageRequirementsMet()) {
					DropHeldItemToSlot(&(Me.weapon_item));
					Me.shield_item.type = shield_item_type;
					MakeHeldFloorItemOutOf(&(Me.shield_item));
				} else {
					append_new_game_message(
							_("Two-handed weapon requirements not met: shield bonus doesn't count."));
					Me.shield_item.type = shield_item_type;
				}
			} else {
				play_sound("effects/tux_ingame_comments/ThisItemRequiresBothHands.ogg");
			}
		}

		// The user left-clicks in the shield's equipment slot
		if (MouseCursorIsOnButton(SHIELD_RECT_BUTTON, CurPos.x, CurPos.y)) {
			
			// Check if the item can be installed in the shield slot
			if (ItemMap[item_held_in_hand->type].slot != SHIELD_SLOT) {
				append_new_game_message(_("You cannot equip this!"));
				return;
			}
			
			// Check if the user has enough skill to use the shield
			if (!HeldItemUsageRequirementsMet()) {
				append_new_game_message(_("You cannot equip this yet!"));
				return;
			}
			
			// Now if there isn't any weapon equipped right now, the matter
			// is rather simple and we just need to do the normal drop-to-slot-thing.
			if (Me.weapon_item.type == (-1)) {
				DropHeldItemToSlot(&(Me.shield_item));
				return;
			}

			// A shield, when equipped, will push out any 2-handed weapon currently
			// equipped from it's weapon slot.... So first check if a 2-handed
			// weapon is equipped.
			if (!ItemMap[Me.weapon_item.type].item_gun_requires_both_hands) {
				DropHeldItemToSlot(&(Me.shield_item));
				return;
			}
			
			// There is a 2-handed weapon equipped, so first of all check 
			// requirements again but without the weapon :
			// virtually remove the weapon, compute requirements, if 
			// everything is okay, proceed otherwise we inform the player
			int weapon_item_type = Me.weapon_item.type;
			Me.weapon_item.type = (-1);
			update_all_primary_stats();
			if (HeldItemUsageRequirementsMet()) {
				DropHeldItemToSlot(&(Me.shield_item));
				Me.weapon_item.type = weapon_item_type;
				MakeHeldFloorItemOutOf(&(Me.weapon_item));
			} else {
				append_new_game_message(_
							("Shield requirements not met: two-handed weapon bonus doesn't count."));
				Me.weapon_item.type = weapon_item_type;
			}
		}

		// The user left-clicks in another equipment slot
		itemspec *tocheck = &ItemMap[item_held_in_hand->type];
		struct {
			int btnidx;
			char propcheck;
			item *slot;
		} dropslots[] = { {
		DRIVE_RECT_BUTTON, (tocheck->slot == BOOT_SLOT), &(Me.drive_item)}, {
		ARMOUR_RECT_BUTTON, (tocheck->slot == ARMOR_SLOT), &(Me.armour_item)}, {
		HELMET_RECT_BUTTON, (tocheck->slot == HELM_SLOT), &(Me.special_item)},};
		int i;
		for (i = 0; i < sizeof(dropslots) / sizeof(dropslots[0]); i++) {
			if (MouseCursorIsOnButton(dropslots[i].btnidx, CurPos.x, CurPos.y) && dropslots[i].propcheck
			    && HeldItemUsageRequirementsMet()) {
				DropHeldItemToSlot(dropslots[i].slot);
				return;
			}
		}

		// The user left-clicks in the "UserRect" -> the item should 
		// be dropped to the floor
		if (MouseCursorIsInUserRect(CurPos.x, CurPos.y)) {
			drop_held_item();
			return;
		}

		// The left-click did not lead to anything useful
		return;
	}

	// Case 3: The user is right-clicking inside the inventory rectangle which 
	// would mean for us that he is applying the item under the mouse button
	//
	if (MouseRightClicked()) {

		if (Me.readied_skill == get_program_index_with_name("Repair equipment")) {
			// Here we know, that the repair skill is selected, therefore we try to 
			// repair the item currently under the mouse cursor.
			//
			if (MouseCursorIsInInventoryGrid(CurPos.x, CurPos.y)) {
				point Inv_GrabLoc;
				int Grabbed_InvPos;
				
				Inv_GrabLoc.x = GetInventorySquare_x(CurPos.x);
				Inv_GrabLoc.y = GetInventorySquare_y(CurPos.y);

				DebugPrintf(0, "\nTrying to repair item at inv-pos: %d %d.", Inv_GrabLoc.x, Inv_GrabLoc.y);

				Grabbed_InvPos = GetInventoryItemAt(Inv_GrabLoc.x, Inv_GrabLoc.y);
				DebugPrintf(0, "\nTrying to repair inventory entry no.: %d.", Grabbed_InvPos);

				if (Grabbed_InvPos == (-1)) {
					// Nothing grabbed, so we need not do anything more here..
					DebugPrintf(0, "\nRepairing in INVENTORY grid FAILED:  NO ITEM AT THIS POSITION FOUND!");
				} else {
					if (Me.Inventory[Grabbed_InvPos].max_durability != -1)
						self_repair_item(&(Me.Inventory[Grabbed_InvPos]));
					else
						ApplyItem(&(Me.Inventory[Grabbed_InvPos]));
				}
			} else {
				int i;
				for (i = 0; i < sizeof(allslots) / sizeof(allslots[0]); i++) {
					if (MouseCursorIsOnButton(allslots[i].buttonidx, CurPos.x, CurPos.y)
					    && allslots[i].slot->type != -1) {
						self_repair_item(allslots[i].slot);
						break;
					}
				}
			}
		} else {
			if (MouseCursorIsInInventoryGrid(CurPos.x, CurPos.y)) {
				point Inv_GrabLoc;
				int Grabbed_InvPos;

				Inv_GrabLoc.x = GetInventorySquare_x(CurPos.x);
				Inv_GrabLoc.y = GetInventorySquare_y(CurPos.y);

				Grabbed_InvPos = GetInventoryItemAt(Inv_GrabLoc.x, Inv_GrabLoc.y);

				if (Grabbed_InvPos != -1) {
					// At this point we know, that we have just applied something from the inventory
					//
					ApplyItem(&(Me.Inventory[Grabbed_InvPos]));
				}
			}
		}
	}
}

/**
 *
 *
 */
static void raw_move_picked_up_item_to_entry(item * ItemPointer, item * TargetPointer, point Inv_Loc)
{
	// We add the new item to the inventory
	CopyItem(ItemPointer, TargetPointer);
	TargetPointer->inventory_position.x = Inv_Loc.x;
	TargetPointer->inventory_position.y = Inv_Loc.y;

	// We make the sound of an item being taken
	// PlayItemSound( ItemMap[ ItemPointer->type ].sound_number );
	play_item_sound(ItemPointer->type, &Me.pos);

	DeleteItem(ItemPointer);
};				// void move_picked_up_item_to_entry ( ItemPointer , TargetPointer )

/**
 *
 */
static int place_item_on_this_position_if_you_can(item * ItemPointer, point Inv_Loc, int InvPos)
{
	int item_height;
	int item_width;

	for (item_height = 0; item_height < ItemMap[ItemPointer->type].inv_size.y; item_height++) {
		for (item_width = 0; item_width < ItemMap[ItemPointer->type].inv_size.x; item_width++) {
			DebugPrintf(1, "\nAddFloorItemDirectlyToInventory:  Checking pos: %d %d ", Inv_Loc.x + item_width,
				    Inv_Loc.y + item_height);
			if (!Inv_Pos_Is_Free(Inv_Loc.x + item_width, Inv_Loc.y + item_height)) {
				Me.Inventory[InvPos].inventory_position.x = -1;
				Me.Inventory[InvPos].inventory_position.y = -1;
				// goto This_Is_No_Possible_Location;
				return (FALSE);
			}
		}
	}
	// if ( !Inv_Pos_Is_Free( Inv_Loc.x , Inv_Loc.y ) ) continue;

	// At this point we know we have reached a position where we can plant this item.
	Me.Inventory[InvPos].inventory_position.x = Inv_Loc.x;
	Me.Inventory[InvPos].inventory_position.y = Inv_Loc.y;
	DebugPrintf(1, "\nAddFloorItemDirectlyToInventory:  FINE INVENTORY POSITION FOUND!!");

	if ((InvPos >= MAX_ITEMS_IN_INVENTORY - 1) || (Me.Inventory[InvPos].inventory_position.x == (-1))) {
		Me.TextVisibleTime = 0;
		Me.TextToBeDisplayed = _("I can't carry any more.");
		CantCarrySound();
		// can't take any more items,
	} else {
		raw_move_picked_up_item_to_entry(ItemPointer, &(Me.Inventory[InvPos]), Inv_Loc);
	}
	return TRUE;
};				// int place_item_on_this_position_if_you_can ( ... )

/**
 * This function deals with the case, that WHILE THERE IS NO INVENTORY
 * SCREEN OPEN, the Tux still clicks some items on the floor to pick them
 * up.  So no big visible operation is required, but instead the items
 * picked up should be either auto-equipped, if possible, or they should
 * be put into the inventory items pool.
 *
 * \return -1 on error, 1 if the item was placed somewhere, 0 if there is no room
 *         for the item in the inventory.
 */
int try_give_item(item *ItemPointer)
{
	int InvPos;
	point Inv_Loc = { -1, -1 };
	int TargetItemIndex;

	if (ItemPointer == NULL)
		return -1;

	// In the special case of money, we add the amount of money to our
	// money counter and eliminate the item on the floor.

	if (item_spec_eq_id(ItemPointer->type, "Valuable Circuits")) {
		play_item_sound(ItemPointer->type, &Me.pos);
		Me.Gold += ItemPointer->multiplicity;
		DeleteItem(ItemPointer);
		return 1;
	}

	// In the special case, that this is an item, that groups together with others
	// of the same type AND we also have as item of this type already in inventory,
	// then we just need to manipulate multiplicity a bit and we're done.  Very easy.

	if (ItemMap[ItemPointer->type].item_group_together_in_inventory) {
		if (CountItemtypeInInventory(ItemPointer->type)) {
			TargetItemIndex = FindFirstInventoryIndexWithItemType(ItemPointer->type);
			Me.Inventory[TargetItemIndex].multiplicity += ItemPointer->multiplicity;
			play_item_sound(ItemPointer->type, &Me.pos);
			DeleteItem(ItemPointer);
			return 1;
		}
	}

	// Maybe the item is of a kind that can be equipped right now.  Then
	// we decide to directly drop it to the corresponding slot.

	if ((Me.weapon_item.type == (-1)) && (ItemMap[ItemPointer->type].slot == WEAPON_SLOT)) {
		if (ItemUsageRequirementsMet(ItemPointer, TRUE)) {
			// Now we're picking up a weapon while no weapon is equipped.  But still
			// it might be a 2-handed weapon while there is some shield equipped.  Well,
			// when that is the case, we refuse to put it directly to the proper slot, 
			// otherwise we do it.
			//
			if (Me.shield_item.type == (-1)) {
				raw_move_picked_up_item_to_entry(ItemPointer, &(Me.weapon_item), Inv_Loc);
				return 1;
			}
			// So now we know that some shield item is equipped.  Let's be careful:  2-handed
			// weapons will be rejected from direct addition to the slot.
			//
			if (!ItemMap[ItemPointer->type].item_gun_requires_both_hands) {
				raw_move_picked_up_item_to_entry(ItemPointer, &(Me.weapon_item), Inv_Loc);
				return 1;
			}
		}
	}

	if ((Me.shield_item.type == (-1)) && (ItemMap[ItemPointer->type].slot == SHIELD_SLOT)) {
		if (ItemUsageRequirementsMet(ItemPointer, TRUE)) {
			// Auto-equipping shields can be done.  But only if there isn't a 2-handed
			// weapon equipped already.  Well, in case of no weapon present it's easy:
			//
			if (Me.weapon_item.type == (-1)) {
				raw_move_picked_up_item_to_entry(ItemPointer, &(Me.shield_item), Inv_Loc);
				return 1;
			}
			// But now we know, that there is some weapon present.  We need to be careful:
			// it might be a 2-handed weapon.
			// 
			if (!ItemMap[Me.weapon_item.type].item_gun_requires_both_hands) {
				raw_move_picked_up_item_to_entry(ItemPointer, &(Me.shield_item), Inv_Loc);
				return 1;
			}
		}
	}

	if ((Me.armour_item.type == (-1)) && (ItemMap[ItemPointer->type].slot == ARMOR_SLOT)) {
		if (ItemUsageRequirementsMet(ItemPointer, TRUE)) {
			raw_move_picked_up_item_to_entry(ItemPointer, &(Me.armour_item), Inv_Loc);
			return 1;
		}
	}

	if ((Me.drive_item.type == (-1)) && (ItemMap[ItemPointer->type].slot == BOOT_SLOT)) {
		if (ItemUsageRequirementsMet(ItemPointer, TRUE)) {
			raw_move_picked_up_item_to_entry(ItemPointer, &(Me.drive_item), Inv_Loc);
			return 1;
		}
	}

	if ((Me.special_item.type == (-1)) && (ItemMap[ItemPointer->type].slot == HELM_SLOT)) {
		if (ItemUsageRequirementsMet(ItemPointer, TRUE)) {
			raw_move_picked_up_item_to_entry(ItemPointer, &(Me.special_item), Inv_Loc);
			return 1;
		}
	}

	// find a free position in the inventory list
	for (InvPos = 0; InvPos < MAX_ITEMS_IN_INVENTORY - 1; InvPos++) {
		if (Me.Inventory[InvPos].type == (-1))
			break;
	}

	// Maybe the item in question is something, that would best be placed inside
	// the quick inventory.  If that is so, we try to put it there first.  If that
	// isn't possible, it can still be placed somewhere outside of the quick 
	// inventory later.

	if ((ItemMap[ItemPointer->type].inv_size.x == 1) &&
	    (ItemMap[ItemPointer->type].inv_size.y == 1) && (ItemMap[ItemPointer->type].right_use.tooltip)) {
		DebugPrintf(2, "\n\nTrying to place this item inside of the quick inventory first...");
		Inv_Loc.y = INVENTORY_GRID_HEIGHT - 1;
		for (Inv_Loc.x = 0; Inv_Loc.x < INVENTORY_GRID_WIDTH - ItemMap[ItemPointer->type].inv_size.x + 1; Inv_Loc.x++) {
			if (place_item_on_this_position_if_you_can(ItemPointer, Inv_Loc, InvPos))
				return 1;
		}
	}

	// Find enough free squares in the inventory to fit
	for (Inv_Loc.y = 0; Inv_Loc.y < INVENTORY_GRID_HEIGHT - ItemMap[ItemPointer->type].inv_size.y + 1; Inv_Loc.y++) {
		for (Inv_Loc.x = 0; Inv_Loc.x < INVENTORY_GRID_WIDTH - ItemMap[ItemPointer->type].inv_size.x + 1; Inv_Loc.x++) {
			if (place_item_on_this_position_if_you_can(ItemPointer, Inv_Loc, InvPos))
				return 1;
		}
	}

	// No enough free place in the inventory
	if (Me.Inventory[InvPos].inventory_position.x == (-1)) {
		return 0;
	}

	// Place the item in the inventory
	raw_move_picked_up_item_to_entry(ItemPointer, &(Me.Inventory[InvPos]), Inv_Loc);

	return 1;
}

const char *ammo_desc_for_weapon(int type) {
	const char *ammo_desc[] = {
		"", // no ammunition
		N_("Laser power pack"),
		N_("Plasma energy container"),
		N_("2 mm Exterminator Ammunition"),
		N_(".22 LR Ammunition"),
		N_("Shotgun shells"),
		N_("9x19mm Ammunition"),
		N_("7.62x39mm Ammunition"),
		N_(".50 BMG (12.7x99mm) Ammunition"),
	};

	itemspec *weapon = &ItemMap[type];

	if (weapon->item_gun_use_ammunition < 0 || weapon->item_gun_use_ammunition >= sizeof(ammo_desc)/sizeof(ammo_desc[0])) {
		error_message(__FUNCTION__, "Unknown ammunition type %d for weapon %s.",
					 PLEASE_INFORM | IS_FATAL, weapon->item_gun_use_ammunition, weapon->id);
	}

	return ammo_desc[weapon->item_gun_use_ammunition];
}

/**
 * \brief Places the item to the inventory of the player or to the floor.
 *
 * If the item is of an equippable type whose requirements are met and the
 * matching equipment slot is empty, the item is placed to the equipment slot.
 * Otherwise, if there's enough room in the inventory, the item is placed there.
 * Otherwise, the item is dropped to the floor at the feet of the player.
 *
 * \param it The item to give to the player.
 *
 * \return TRUE if equipped or placed into the inventory, FALSE if dropped on the floor
 */
int give_item(item *it)
{
	if (it == NULL)
		return FALSE;

	if (try_give_item(it)) {
		return TRUE;
	}

	Me.TextVisibleTime = 0;
	Me.TextToBeDisplayed = _("I can't carry any more.");
	CantCarrySound();
	drop_item(it, Me.pos.x, Me.pos.y, Me.pos.z);

	return FALSE;
}

int item_is_currently_equipped(item * Item)
{
	if ((&(Me.weapon_item) == Item) || (&(Me.drive_item) == Item) || (&(Me.armour_item) == Item)
	    || (&(Me.shield_item) == Item) || (&(Me.special_item) == Item))
		return 1;

	return 0;
}

enum slot_type get_slot_type_by_name(char *name)
{
	struct { 
		const char *name;
		enum slot_type slot;
	} slots[] = {	{ "weapon", WEAPON_SLOT }, 
					{"drive", 	BOOT_SLOT 	}, 
					{"shield", 	SHIELD_SLOT }, 
					{"armour", 	ARMOR_SLOT 	}, 
					{"special", HELM_SLOT 	}};

	int i;
	for (i = 0; i < sizeof(slots)/sizeof(slots[0]); i++) {
		if (!strcmp(name, slots[i].name))
			return slots[i].slot;
	}
	return NO_SLOT;
}

/**
 * \brief Get the name of a item specs by its type.
 * If the item spec haven't a title, the name is used, and otherwise "UNNAMED ITEM".
 * \param type A valid type of itemspec (different of -1).
 * \return The title of the item.
 */
const char *item_specs_get_name(int type)
{
	if (ItemMap[type].name)
		return ItemMap[type].name;
	else if (ItemMap[type].id)
		return ItemMap[type].id;
	return "BUG - UNNAMED ITEM";
}

enum _busytype get_busy_type_by_name(char *name)
{
	if (!strcmp(name, "drinking")) {
		return DRINKING_POTION;
	} else if (!strcmp(name, "throwing")) {
		return THROWING_GRENADE;
	} else if (!strcmp(name, "pill")) {
		return TAKING_PILL;
	}
	return NONE;
}

#undef _items_c
