/* 
 *
 *   Copyright (c) 2009-2010 Arthur Huillet
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
 * This file contains functions related to non playing characters, notably
 * chat flags and shop lists.
 */

#define _npc_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"
#include "SDL_rotozoom.h"

#define DEBUG_SHOP 0

static void npc_clear_inventory(struct npc *);

// List of NPCs in the game
LIST_HEAD(npc_head);

struct npc *npc_get(const char *dialog_basename)
{
	struct npc *n;
	list_for_each_entry(n, &npc_head, node) {
		if (!strcmp(n->dialog_basename, dialog_basename))
			return n;
	}

	ErrorMessage(__FUNCTION__, "Could not find NPC with name \"%s\".", PLEASE_INFORM, IS_FATAL, dialog_basename);
	return NULL;
}

void npc_insert(struct npc *n)
{
	list_add(&n->node, &npc_head);
}

void npc_add(const char *dialog_basename) 
{
	struct npc *n = MyMalloc(sizeof(struct npc));

	n->dialog_basename = strdup(dialog_basename);
	n->chat_character_initialized = 0;
	n->last_trading_date = 0.0f;

	npc_clear_inventory(n);
	npc_insert(n);	
}

void init_npcs()
{
	/* Create NPCs*/
	const char *npcs[] = {
		"614_cryo",
		"AfterTakeover",
		"Bender",
		"Benjamin",
		//	"Boris",
		"Bruce",
		"Butch",
		"Chandra",
		"c-net",
		"Cryo-Terminal",
		"Dixon",
		"DocMoore",
		"DSB-MachineDeckControl",
		"DSB-PowerControl",
		"DSB-PowerControlGate1",
		"Duncan",
		"Engel",
		"Ewald",
		"Francis",
		"Geist",
		"HF-FirmwareUpdateServer",
		"HF-GateGuard",
		"HF-GateGuardLeader",
		"Iris",
		"Jasmine",
		"Kevin",
		"KevinGuard",
		"Kevin-Lawnmower",
		"Koan",
		//	"Lina",
		"Lukas",
		"Michelangelo",
		"Maintenance-Terminal",
		"Pendragon",
		"Peter",
		"Richard",
		"SACD",
		"SADD",
		//	"Serge",
		"Singularity",
		"Singularity-Drone",
		"Skippy",
		"Sorenson",
		"Spencer",
		"Stone",
		"Town-614",
		"Town-GuardhouseGuard",
		"Town-NorthGateGuard",
		"Town-TeleporterGuard",
		"Town-TuxGuard",
		"Tamara",
		"Tania",
		"TestDroid",
		"Terminal",
		"TutorialTerminal",
		"TutorialTom",
		"Tybalt",
		"WillGapes",
	};

	int i;
	for (i = 0; i < sizeof(npcs)/sizeof(npcs[0]); i++) {
		npc_add(npcs[i]);
	}

	struct shop {
		const char *item_name;
		int weight;
	};

	/* Create NPC default shoplists */
	struct shop stone_shop[] = {
			{ "Big kitchen knife", 1 },
			{ "Meat cleaver", 1 },
			{ "Small Axe", 1 },
			{ "Large Axe", 1 },
			{ "Hunting knife", 1 },
			{ "Iron pipe", 1 },
			{ "Big wrench", 1 },
			{ "Crowbar", 1 },
			{ "Power hammer", 1 },
			{ "Baseball bat", 1 },
			{ "Normal Jacket", 1 },
			{ "Normal Jacket", 1 },
			{ "Reinforced Jacket", 1 },
			{ "Protective Jacket", 1 },
			{ "Standard Shield", 1 },
			{ "Heavy Shield", 1 },
			{ "Worker Helmet", 1 },
			{ "Worker Helmet", 1 },
			{ "Miner Helmet", 1 },
			{ "Shoes", 1 },
			{ "Shoes", 1 },
			{ "Worker Shoes", 1 },
			{ ".22 LR Ammunition", 1 },
			{ "Shotgun shells", 1 },
	};

	struct shop moore_shop[] = { 
			{ "Diet supplement", 3 },
			{ "Antibiotic", 1 },
			{ "Doc-in-a-can", 1 },
	};

	struct shop lukas_shop[] = { 
			{ "9x19mm Ammunition", 1 },
			{ "7.62x39mm Ammunition", 1 },
			{ ".50 BMG (12.7x99mm) Ammunition", 1 },
			{ "2 mm Exterminator Ammunition", 1 },
			{ "Laser power pack", 1 },
			{ "Laser power pack", 1 },
			{ "Laser power pack", 1 },
			{ "Laser pistol", 1 },
			{ "Laser pistol", 1 },
			// too ugly and missing rotationimages		"Laser Rifle",
			{ "Plasma energy container", 1 },
			{ "Plasma energy container", 1 },
			{ "Plasma energy container", 1 },
			{ "Plasma pistol", 1 },
			{ "Plasma pistol", 1 },
			{ "Plasma gun", 1 },
			{ "Riot Shield", 1 },
			{ "Light Battle Helmet", 1 },
			{ "Battle Helmet", 1 },
			{ "Battle Shoes", 1 },
			{ "Red Guard's Light Robe", 1 },
			{ "Red Guard's Heavy Robe", 1 },
	};

	struct shop skippy_shop[] = { 
		//		"Map Maker",  
		//		Sniper wristband
		//		Hacker wristband
		//		Script?
			{ "Teleporter homing beacon", 1 },
			{ "Teleporter homing beacon", 1 },
			{ "Teleporter homing beacon", 1 },
	};

	struct shop duncan_shop[] = { 
			{ "VMX Gas Grenade", 1 },
			{ "VMX Gas Grenade", 1 },
			{ "Small EMP Shockwave Generator", 1 },
			{ "Small EMP Shockwave Generator", 1 },
			{ "Small Plasma Shockwave Emitter", 1 },
			{ "Small Plasma Shockwave Emitter", 1 },
	};

	struct shop ewald_shop[] = { 
			{ "Bottled ice", 1 },
			{ "Industrial coolant", 1 },
			{ "Liquid nitrogen", 1 },
			{ "Barf's Energy Drink", 1 },
			{ "Running Power Capsule", 1 },
			{ "Fork", 1 },
			{ "Plate", 1 },
			{ "Mug", 1 },
	};

	struct shop sorenson_shop[] = { 
			{ "Source Book of Emergency shutdown", 1 },
			{ "Source Book of Check system integrity", 1 },
			{ "Source Book of Sanctuary", 1 },
			{ "Source Book of Malformed packet", 1 },
			{ "Source Book of Blue Screen", 1 },
			{ "Source Book of Broadcast Blue Screen", 1 },
			{ "Source Book of Calculate Pi", 1 },
			{ "Source Book of Virus", 1 },
			{ "Source Book of Broadcast virus", 1 },
			{ "Source Book of Dispel smoke", 1 },
			{ "Source Book of Killer poke", 1 },
			{ "Source Book of Invisibility", 1 },
			//{ "Source Book of Analyze item", 1 },
			//		"Source Book of Plasma discharge",
			//		"Source Book of Reverse-engineer",
			//		"Source Book of Nethack",
	};

	struct shop tamara_shop[] = { 
			{ "Source Book of Emergency shutdown", 1 },
			{ "Source Book of Check system integrity", 1 },
			{ "Source Book of Sanctuary", 1 },
			{ "Source Book of Malformed packet", 1 },
			{ "Source Book of Blue Screen", 1 },
			{ "Source Book of Broadcast Blue Screen", 1 },
			{ "Source Book of Calculate Pi", 1 },
			{ "Source Book of Virus", 1 },
			{ "Source Book of Broadcast virus", 1 },
			{ "Source Book of Dispel smoke", 1 },
			{ "Source Book of Killer poke", 1 },
			{ "Source Book of Invisibility" , 1 },
			//		"Source Book of Plasma discharge",
			//		"Source Book of Reverse-engineer",
			//		"Source Book of Nethack",
	};


#define ADD_AR(a, name) \
	for (i=0; i < sizeof(a)/sizeof(a[0]); i++) { \
		npc_add_shoplist(name, a[i].item_name, a[i].weight); \
	}

	ADD_AR(stone_shop, "Stone");
	ADD_AR(moore_shop, "DocMoore");
	ADD_AR(lukas_shop, "Lukas");
	ADD_AR(skippy_shop, "Skippy");
	ADD_AR(duncan_shop, "Duncan");
	ADD_AR(ewald_shop, "Ewald");
	ADD_AR(sorenson_shop, "Sorenson");
	ADD_AR(tamara_shop, "Tamara");
}

void clear_npcs()
{
	struct npc *n, *next;
	int i;
	list_for_each_entry_safe(n, next, &npc_head, node) {
		list_del(&n->node);

		free(n->dialog_basename);

		for (i = 0; i < MAX_ITEMS_IN_NPC_SHOPLIST; i++) {
			if (!n->shoplist[i])
				break;
			free(n->shoplist[i]);
			n->shoplistweight[i] = 0;
		}

		free(n);
	}

	INIT_LIST_HEAD(&npc_head);
}

int npc_add_shoplist(const char *dialog_basename, const char *item_name, int weight)
{
	int i;
	struct npc *n;

	n = npc_get(dialog_basename); 
	for (i = 0; i < MAX_ITEMS_IN_NPC_SHOPLIST; i++) {
		if (n->shoplist[i] == NULL)
			break;
	}

	if (i == MAX_ITEMS_IN_NPC_SHOPLIST) {
		ErrorMessage(__FUNCTION__, "Shop list for character \"%s\" is full. Cannot add item \"%s\".\n", PLEASE_INFORM, IS_WARNING_ONLY, n->dialog_basename, item_name);
		return 1;
	}

	n->shoplist[i] = strdup(item_name);
	n->shoplistweight[i] = weight;
	return 0;
}

static void npc_clear_inventory(struct npc *n)
{
	dynarray_free(&n->npc_inventory);
}

static int npc_inventory_size(struct npc *n)
{
	return n->npc_inventory.size;
}

static int npc_shoplist_weight(struct npc *n)
{
	int total_weight = 0;
	int i;

	for (i = 0; i < MAX_ITEMS_IN_NPC_SHOPLIST; i++) {
		if (n->shoplistweight[i] == 0)
			break;

		total_weight += n->shoplistweight[i];
	}

	return total_weight;
}

/**
 * Remove an item from the NPC inventory, preserving others.
 */
void npc_inventory_delete_item(struct npc *n, int index)
{
	dynarray_del(&n->npc_inventory, index, sizeof(item));
}

/**
 * Add an item in the NPC inventory, given its name
 * Returns 0 on success.
 */
static int add_item(struct npc *n, const char *item_name)
{
	int i;
	int stack_item = -1;
	int item_type = GetItemIndexByName(item_name);
	int amount = 1;

	// Stackable items are added in quantities larger than one. We use 50 as the value.
	if (ItemMap[item_type].item_group_together_in_inventory) {
		amount = 90 + MyRandom(10);
	}

	// If the item is stackable, look for the item index
	// to stack at.
	for (i = 0; i < n->npc_inventory.size; i++) {
		if (ItemMap[item_type].item_group_together_in_inventory && ((item *)(n->npc_inventory.arr))[i].type == item_type) {
			stack_item = i;
			break;
		}
	}

	DebugPrintf(DEBUG_SHOP, "adding item %s\n", item_name);
	if (stack_item != -1) {
		((item *)(n->npc_inventory.arr))[i].multiplicity += amount;
	} else {
		item it;
		init_item(&it);
		it.type = GetItemIndexByName(item_name);
		FillInItemProperties(&it, TRUE, 1);
		it.multiplicity = amount;
		dynarray_add(&n->npc_inventory, &it, sizeof(it));
	}

	return 0;
}

/** 
 * Pick an item at random from the NPC shoplist
 */
static const char *npc_pick_item(struct npc *n)
{
	int total_weight;
	int i;
	int pick;

	total_weight = npc_shoplist_weight(n);
	pick = MyRandom(total_weight);

	for (i = 0; i < MAX_ITEMS_IN_NPC_SHOPLIST; i++) {
		pick -= n->shoplistweight[i];
		if (pick <= 0)
			break;
	}

	return n->shoplist[i];
}

/**
 * Refresh the inventory of an NPC so as to introduce a bit 
 * of variation in what NPCs sell.
 */
static void npc_refresh_inventory(struct npc *n)
{
	int i;
	int target_size;
	int shoplist_weight = npc_shoplist_weight(n);

	// Remove each item with a given probability
	for (i = npc_inventory_size(n) - 1; i >= 0; i--) {
		DebugPrintf(DEBUG_SHOP, "refresh: removing item %d\n", i);
		// The loop is backwards so repeated remove_item calls
		// do as little memory traffic as possible
		if (MyRandom(100) < 50) {
			npc_inventory_delete_item(n, i);
		}
	}

	// Compute the target size and add items to match it
	target_size = npc_shoplist_weight(n);
	if (target_size > 12)
		target_size = 12;

	DebugPrintf(DEBUG_SHOP, "refresh: target size is %d, inventory size %d, shoplist weight %d\n", target_size, npc_inventory_size(n), shoplist_weight);

	if (npc_inventory_size(n) >= target_size) {
		// We do not need to add any items, we have too many already
		return;
	}

	// Add the required number of items
	i = target_size - npc_inventory_size(n);
	while (i--) {
		add_item(n, npc_pick_item(n));
	}
}

/**
 * This function is used by shops to get a list of the items
 * a NPC will sell.
 * It takes care of refreshing the list when necessary.
 */
item_dynarray *npc_get_inventory(struct npc *n)
{
	// Time based refresh
	if ((Me.current_game_date - n->last_trading_date) > 360) {
		DebugPrintf(DEBUG_SHOP, "time based  refresh\n");
		// Refresh every 360 secondes
		npc_refresh_inventory(n);
	}

	// Low-stock based refresh
	if (npc_inventory_size(n) < 6) {
		DebugPrintf(DEBUG_SHOP, "stock based  refresh\n");
		// Less than 6 items in stock? Buy more
		npc_refresh_inventory(n);
	}

	return &n->npc_inventory;
}

#undef _npc_c
