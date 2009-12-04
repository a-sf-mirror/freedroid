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
		"Iron pipe",
		"Big wrench",
		"Crowbar",
		"Cap",
		"Buckler",
		"Simple Jacket",
		"Shoes",
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

#undef _npc_c
