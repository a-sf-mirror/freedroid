/* 
 *
 *   Copyright (c) 2010 Arthur Huillet
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
#define _armor_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

/**
 * Compute the armor class from the classes of all items.
 * Armor class is the sum of armor classes of all items, with no ponderation.
 */
void update_player_armor_class()
{
	Me.armor_class = 0;

	item *protective_equipment[] = {
		&Me.armour_item,
		&Me.shield_item,
		&Me.special_item,
		&Me.drive_item,
	};

	int i;

	for (i = 0; i < sizeof(protective_equipment)/sizeof(protective_equipment[0]); i++) {
		if (protective_equipment[i]->type != -1) {
			Me.armor_class += protective_equipment[i]->armor_class;
			Me.armor_class += protective_equipment[i]->bonus_to_armor_class;
		}
	}
}

/**
 * How much damage does the armor absorb.
 *
 * The rule is: 
 * 0AC -> 100% damage
 * 120AC -> 20% damage
 *
 * as an exponential law of the form a * exp(-b * x),
 * this means a = 1 and b = 0.0134
 */
float get_player_damage_factor()
{
	float damage_reduction = exp(-0.0134 * Me.armor_class);
	return damage_reduction;
}
