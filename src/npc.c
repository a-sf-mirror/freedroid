/* 
 *
 *   Copyright (c) 2009 Arthur Huillet
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
		"614",
		"614_cryo",
		"Bender",
		"Benjamin",
		//	"Boris",
		"Bruce",
		"Butch",
		"Chandra",
		"Darwin",
		"Dixon",
		"DocMoore",
		"Duncan",
		"Ewald",
		"FirmwareUpdateServer",
		"Francis",
		"Jasmine",
		"Kevin",
		"KevinGuard",
		"Koan",
		//	"Lina",
		"Lukas",
		"MER",
		//	"MSCD",
		"MSFacilityGateGuardLeader",
		"Melfis",
		"Michelangelo",
		"OldTownGateGuardLeader",
		"Pendragon",
		"RMS",
		"SACD",
		"SADD",
		//	"Serge",
		"Skippy",
		"Sorenson",
		"Spencer",
		"StandardBotAfterTakeover",
		"StandardMSFacilityGateGuard",
		"StandardOldTownGateGuard",
		"Stone",
		"Tania",
		"TestDroid",
		"TutorialTom",
		"Tybalt",
	};

	int i;
	for (i = 0; i < sizeof(npcs)/sizeof(npcs[0]); i++) {
		npc_add(npcs[i]);
	}

	/* Create NPC default shoplists */
	const char *stone_shop[] = {
		"Big kitchen knife",
		"Meat cleaver",
		"Small Axe",
		"Large Axe",
		"Hunting knife",
		"Iron pipe",
		"Big wrench",
		"Crowbar",
		"Power hammer",
		"Baseball bat",
		"Normal Jacket",
		"Reinforced Jacket",
		"Protective Jacket",
		"Standard Shield",
		"Large Shield",
		"Worker Helmet",
		"Miner Helmet",
		"Shoes",
		"Worker Shoes",
		".22 LR Ammunition",
		"Shotgun shells",
	};

	const char *moore_shop[] = {
		"Diet supplement",
		"Antibiotic",
		"Doc-in-a-can",
	};

	const char *lukas_shop[] = {
		"Laser pistol",
		"Laser power pack",
		"Plasma pistol",
		"Plasma energy container",
		"Riot Shield",
		"Light Battle Helmet",
		"Battle Helmet",
		"Battle Shoes",
		"Red Guard's Light Robe",
		"Red Guard's Heavy Robe",
	};

	const char *skippy_shop[] = {
		"Map Maker",
		"Teleporter homing beacon",
	};

	const char *duncan_shop[] = {
		"VMX Gas Grenade",
		"EMP Shockwave Generator",
		"Plasma Shockwave Emitter",
	};

	const char *ewald_shop[] = {
		"Bottled ice",
		"Industrial coolant",
		"Liquid nitrogen",
		"Barf's Energy Drink",
		"Running Power Capsule",
		"Fork",
		"Plate",
		"Mug",
	};

	const char *sorenson_shop[] = {
		"Source Book of Emergency shutdown",
		"Source Book of Check system integrity",
		"Source Book of Sanctuary",
		"Source Book of Analyze item",
		"Source Book of Malformed packet",
		"Source Book of Blue Screen",
		"Source Book of Broadcast Blue Screen",
		"Source Book of Calculate Pi",
		"Source Book of Virus",
		"Source Book of Broadcast virus",
		"Source Book of Dispel smoke",
		"Source Book of Killer poke",
		"Source Book of Reverse-engineer",
		"Source Book of Plasma discharge",
		"Source Book of Nethack",
		"Source Book of Invisibility",
	};

#define ADD_AR(a, name) \
	for (i=0; i < sizeof(a)/sizeof(a[0]); i++) { \
		npc_add_shoplist(name, a[i]); \
	}

	ADD_AR(stone_shop, "Stone");
	ADD_AR(moore_shop, "DocMoore");
	ADD_AR(lukas_shop, "Lukas");
	ADD_AR(skippy_shop, "Skippy");
	ADD_AR(duncan_shop, "Duncan");
	ADD_AR(ewald_shop, "Ewald");
	ADD_AR(sorenson_shop, "Sorenson");
}

void clear_npcs()
{
	struct npc *n, *next;
	int i;
	list_for_each_entry_safe(n, next, &npc_head, node) {
		list_del(&n->node);

		free(n->dialog_basename);

		for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
			if (!n->shoplist[i])
				break;
			free(n->shoplist[i]);
		}

		free(n);
	}

	INIT_LIST_HEAD(&npc_head);
}

int npc_add_shoplist(const char *dialog_basename, const char *item_name)
{
	int i;
	struct npc *n;

	n = npc_get(dialog_basename); 
	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		if (n->shoplist[i] == NULL)
			break;
	}

	if (i == MAX_ITEMS_IN_INVENTORY) {
		ErrorMessage(__FUNCTION__, "Shop list for character \"%s\" is full. Cannot add item \"%s\".\n", PLEASE_INFORM, IS_WARNING_ONLY, n->dialog_basename, item_name);
		return 1;
	}

	n->shoplist[i] = strdup(item_name);
	return 0;
}

static void npc_clear_inventory(struct npc *n)
{
	int i;
	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		n->npc_inventory[i].type = -1;
		n->npc_inventory[i].prefix_code = -1;
		n->npc_inventory[i].suffix_code = -1;
	}
}

static int npc_inventory_size(struct npc *n)
{
	int i;
	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		if (n->npc_inventory[i].type == -1)
			break;
	}

	return i;
}

static int npc_shoplist_size(struct npc *n)
{
	int i;
	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		if (n->shoplist[i] == NULL)
			break;
	}

	return i;
}

/**
 * Remove an item from the NPC inventory, preserving others.
 */
void npc_inventory_delete_item(struct npc *n, int index)
{
	if (index == MAX_ITEMS_IN_INVENTORY - 1) {
		// If we are removing the last item of the list, it is easy
		n->npc_inventory[index].type = -1;
		n->npc_inventory[index].prefix_code = -1;
		n->npc_inventory[index].suffix_code = -1;
		return;
	}

	// Otherwise, erase this item with the next ones
	memmove(&n->npc_inventory[index], &n->npc_inventory[index+1], sizeof(item)*(MAX_ITEMS_IN_INVENTORY - index - 1));
}

/**
 * Add an item in the NPC inventory, given its name
 * Returns 0 on success.
 */
static int add_item(struct npc *n, const char *item_name)
{
	int i;

	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		if (n->npc_inventory[i].type == -1)
			break;
	}

	if (i == MAX_ITEMS_IN_INVENTORY) {
		ErrorMessage(__FUNCTION__, "Unable to add item \"%s\" inside NPC \"%s\" inventory, because the inventory is full.\n", PLEASE_INFORM, IS_WARNING_ONLY, item_name, n->dialog_basename);
		return 1;
	}

	printf("adding item %s\n", item_name);
	n->npc_inventory[i].type = GetItemIndexByName(item_name);
	FillInItemProperties(&n->npc_inventory[i], TRUE, 1);
	n->npc_inventory[i].is_identified = TRUE;

	return 0;
}

/**
 * Refresh the inventory of an NPC so as to introduce a bit 
 * of variation in what NPCs sell.
 */
static void npc_refresh_inventory(struct npc *n)
{
	int i;
	int target_size;
	int shoplist_size = npc_shoplist_size(n);

	// Remove each item with a given probability
	for (i = npc_inventory_size(n) - 1; i >= 0; i--) {
		printf("refresh: removing item %d\n", i);
		// The loop is backwards so repeated remove_item calls
		// do as little memory traffic as possible
		if (MyRandom(100) < 50) {
			npc_inventory_delete_item(n, i);
		}
	}

	// Compute the target size and add items to match it
	target_size = (3*npc_shoplist_size(n)) / 4;
	printf("refresh: target size is %d, inventory size %d, shoplist size %d\n", target_size, npc_inventory_size(n), shoplist_size);

	if (npc_inventory_size(n) >= target_size) {
		// We do not need to add any items, we have too many already
		return;
	}

	// Add the required number of items
	i = target_size - npc_inventory_size(n);
	while (i--) {
		add_item(n, n->shoplist[MyRandom(shoplist_size - 1)]);
	}
}

/**
 * This function is used by shops to get a list of the items
 * a NPC will sell.
 * It takes care of refreshing the list when necessary.
 */
item *npc_get_inventory(struct npc *n)
{
	// Time based refresh
	if ((Me.current_game_date - n->last_trading_date) > 360) {
		printf("time based  refresh\n");
		// Refresh every 360 secondes
		npc_refresh_inventory(n);
	}

	// Low-stock based refresh
	if (npc_inventory_size(n) < (1 + npc_shoplist_size(n) / 2)) {
		printf("stock based  refresh\n");
		// If stock < 50% of catalog
		npc_refresh_inventory(n);
	}

	return n->npc_inventory;
}

#undef _npc_c
