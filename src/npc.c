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

	ErrorMessage(__FUNCTION__, "Could not find NPC with name \"%s\".", PLEASE_INFORM, IS_WARNING_ONLY, dialog_basename);
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
	n->chat_character_initialized = FALSE;
	n->last_trading_date = 0.0f;

#ifdef WITH_NEW_DIALOG
	dynarray_free(&n->enabled_nodes);
#endif
	npc_clear_inventory(n);
	npc_insert(n);	
}

void init_npcs()
{
	char fpath[2048];

	find_file("npc_specs.lua", MAP_DIR, fpath, 0);
	run_lua_file(LUA_CONFIG, fpath);
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
