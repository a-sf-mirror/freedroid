/* 
 *
 *  Copyright (c) 2010 Ari Mustonen
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

static struct dynarray *addon_specs = NULL;

/**
 * \brief Adds an upgrade socket to the item.
 * \param it Item.
 * \param type Upgrade socket type.
 * \param addon Upgrade item type or NULL.
 */
void create_upgrade_socket(item *it, int type, const char *addon)
{
	struct upgrade_socket socket; 

	// Initialize upgrade socket data.
	socket.type = type;
	socket.addon = NULL;
	if (addon) {
		socket.addon = strdup(addon);
	}

	// Append an upgrade socket to the socket array.
	dynarray_add((struct dynarray *) &it->upgrade_sockets, &socket, sizeof(struct upgrade_socket));
}

/**
 * \brief Deletes all upgrade sockets of an item.
 * \param it Item.
 */
void delete_upgrade_sockets(item *it)
{
	int i;
	for (i = 0 ; i < it->upgrade_sockets.size ; i++) {
		free(it->upgrade_sockets.arr[i].addon);
	}
	dynarray_free((struct dynarray *) &it->upgrade_sockets);
}

/**
 * \brief Copies upgrade sockets from an item to another.
 *
 * The destination item must have its sockets deleted prior to calling this.
 * Otherwise, the old sockets will be leaked.
 * \param srcitem Source item.
 * \param dstitem Destination item.
 */
void copy_upgrade_sockets(item *srcitem, item *dstitem)
{
	int i;

	// Allocate new upgrade sockets.
	int size = srcitem->upgrade_sockets.size;
	dynarray_init((struct dynarray *) &dstitem->upgrade_sockets, size, sizeof(struct upgrade_socket));

	// Duplicate socket data.
	for (i = 0 ; i < size ; i++) {
		dynarray_add((struct dynarray *) &dstitem->upgrade_sockets, &srcitem->upgrade_sockets.arr[i],
		              sizeof(struct upgrade_socket));
		if (srcitem->upgrade_sockets.arr[i].addon) {
			dstitem->upgrade_sockets.arr[i].addon = strdup(srcitem->upgrade_sockets.arr[i].addon);
		}
	}
}

/**
 * \brief Returns TRUE if the item is of a customizable type.
 * \param it Item.
 * \return TRUE if customizable, FALSE otherwise.
 */
int item_can_be_customized(item *it)
{
	itemspec *spec = &ItemMap[it->type];

	return spec->slot != NO_SLOT;
}

/**
 * \brief Checks if the spec of an add-on is compatible with an item.
 * \param addonspec Spec of the add-on whose compatiblity to test.
 * \param it Item to which the add-on would be installed.
 * \return TRUE if compatible, FALSE if incompatible.
 */
static int addon_is_compatible_with_item(struct addon_spec *addonspec, item *it)
{
	int ret = TRUE;
	const char *str = addonspec->requires_item;

	if (str) {
		itemspec* spec = &ItemMap[it->type];
		if (!strcmp(str, "melee weapon")) {
			ret = (spec->slot == WEAPON_SLOT) &&
			      spec->item_weapon_is_melee;
		} else if (!strcmp(str, "ranged weapon")) {
			ret = (spec->slot == WEAPON_SLOT) &&
			      !spec->item_weapon_is_melee;
		} else if (!strcmp(str, "armor")) {
			ret = (spec->slot & (SHIELD_SLOT | HELM_SLOT | ARMOR_SLOT | BOOT_SLOT));
		} else if (!strcmp(str, "boots")) {
			ret = (spec->slot == BOOT_SLOT);
		} else if (!strcmp(str, "jacket")) {
			ret = (spec->slot == ARMOR_SLOT);
		} else if (!strcmp(str, "shield")) {
			ret = (spec->slot == SHIELD_SLOT);
		} else if (!strcmp(str, "helmet")) {
			ret = (spec->slot == HELM_SLOT);
		} else {
			ret = FALSE;
		}
	}

	return ret;
}

/**
 * \brief Returns TRUE if the item is compatible with the socket.
 * \param dstitem Destionation item.
 * \param addon Add-on item.
 * \param socketid Socket index.
 * \return TRUE if compatible, FALSE otherwise.
 */
int item_can_be_installed_to_socket(item *dstitem, item *addon, int socketid)
{
	const char *str;
	struct addon_spec *spec;
	enum upgrade_socket_types addon_type;
	enum upgrade_socket_types socket_type;

	// Make sure that the socket and the add-on exist.
	if (socketid >= dstitem->upgrade_sockets.size) {
		ErrorMessage(__FUNCTION__, "Socket index out of bounds", PLEASE_INFORM, IS_WARNING_ONLY);
		return FALSE;
	}
	if (addon->type == -1) {
		ErrorMessage(__FUNCTION__, "Add-on type was -1", PLEASE_INFORM, IS_WARNING_ONLY);
		return FALSE;
	}

	// Make sure the item is an add-on.
	spec = get_addon_spec(addon->type);
	if (spec == NULL) {
		return FALSE;
	}
	str = spec->requires_socket;
	if (strcmp(str, "mechanical") == 0) {
		addon_type = UPGRADE_SOCKET_TYPE_MECHANICAL;
	} else if (strcmp(str, "electric") == 0) {
		addon_type = UPGRADE_SOCKET_TYPE_ELECTRIC;
	} else if (strcmp(str, "universal") == 0) {
		addon_type = UPGRADE_SOCKET_TYPE_UNIVERSAL;
	} else {
		return FALSE;
	}

	// Check if the socket type is correct. The socket must be either of the
	// type specified in itemspec for the add-on or universal.
	socket_type = dstitem->upgrade_sockets.arr[socketid].type;
	if (socket_type != addon_type && socket_type != UPGRADE_SOCKET_TYPE_UNIVERSAL) {
		return FALSE;
	}

	// Make sure the add-on is compatible with the type of the customized item.
	return addon_is_compatible_with_item(spec, dstitem);
}

/**
 * \brief Gets the add-on specification for an item type.
 * \param item_type Item type number.
 * \return Add-on specification or NULL if the item isn't an add-on.
 */
struct addon_spec *get_addon_spec(int item_type)
{
	int i;
	struct addon_spec *spec;

	for (i = 0; i < addon_specs->size; i++) {
		spec = &((struct addon_spec *) addon_specs->arr)[i];
		if (spec->type == item_type) {
			return spec;
		}
	}

	return NULL;
}

/**
 * \brief Gets the list of all add-on specs.
 * \return A dynarray containing items of type addon_spec.
 */
struct dynarray *get_addon_specs()
{
	return addon_specs;
}

/**
 * \brief Registers an add-on specification.
 *
 * Only the contents of the passed spec are stored to the add-on spec array.
 * The spec pointer itself isn't used and needs to be freed by the caller.
 * \param spec Add-on specification to register.
 */
void add_addon_spec(struct addon_spec *spec)
{
	if (addon_specs == NULL) {
		addon_specs = dynarray_alloc(32, sizeof(struct addon_spec));
	}
	dynarray_add(addon_specs, spec, sizeof(struct addon_spec));
}

/**
 * \brief Writes the item bonuses of an item to a string suitable for displaying in the UI.
 * \param it Item.
 * \param separator Separator string to use between bonuses.
 * \param desc An allocated auto string to which to append the string.
 */
void get_item_bonus_string(item *it, const char *separator, struct auto_string *desc)
{
	// Append the bonuses to the string.
	if (it->bonus_to_str) {
		autostr_append(desc, _("%+d to strength%s"), it->bonus_to_str, separator);
	}
	if (it->bonus_to_dex) {
		autostr_append(desc, _("%+d to dexterity%s"), it->bonus_to_dex, separator);
	}
	if (it->bonus_to_cooling) {
		autostr_append(desc, _("%+d to cooling%s"), it->bonus_to_cooling, separator);
	}
	if (it->bonus_to_physique) {
		autostr_append(desc, _("%+d to physique%s"), it->bonus_to_physique, separator);
	}
	if (it->bonus_to_health_points) {
		autostr_append(desc, _("%+d health points%s"), it->bonus_to_health_points, separator);
	}
	if (it->bonus_to_health_recovery) {
		autostr_append(desc, _("%+0.1f health points per second%s"), it->bonus_to_health_recovery, separator);
	}
	if (it->bonus_to_cooling_rate) {
		if (it->bonus_to_cooling_rate > 0) {
			autostr_append(desc, _("%0.1f cooling per second%s"), it->bonus_to_cooling_rate, separator);
		} else {
			autostr_append(desc, _("%0.1f heating per second%s"), -it->bonus_to_cooling_rate, separator);
		}
	}
	if (it->bonus_to_attack) {
		autostr_append(desc, _("%+d%% to attack%s"), it->bonus_to_attack, separator);
	}
	if (it->bonus_to_all_attributes) {
		autostr_append(desc, _("%+d to all attributes%s"), it->bonus_to_all_attributes, separator);
	}
	if (it->bonus_to_damage) {
		autostr_append(desc, _("%+d to damage%s"), it->bonus_to_damage, separator);
	}
	if (it->bonus_to_armor_class) {
		autostr_append(desc, _("%+d to armor%s"), it->bonus_to_armor_class, separator);
	}
	if (it->bonus_to_paralyze_enemy) {
		autostr_append(desc, _("%+d to paralyze enemy%s"), it->bonus_to_paralyze_enemy, separator);
	}
	if (it->bonus_to_slow_enemy) {
		autostr_append(desc, _("%+d to slow enemy%s"), it->bonus_to_slow_enemy, separator);
	}
	if (it->bonus_to_light_radius) {
		autostr_append(desc, _("%+d to light radius%s"), it->bonus_to_light_radius, separator);
	}
	if (it->bonus_to_experience_gain) {
		autostr_append(desc, _("%+d%% to experience gain%s"), it->bonus_to_experience_gain, separator);
	}
}

/**
 * \brief Appends information about the add-on to the autostring.
 * \param spec Add-on specification.
 * \param desc An auto string to which to append the description.
 */
void print_addon_description(struct addon_spec *spec, struct auto_string *desc)
{
	item temp_item;
	const char *str;

	// Append socket type requirements.
	str = spec->requires_socket;
	if (str) {
		if (!strcmp(str, "mechanical")) {
			autostr_append(desc, _("Install to a mechanical socket\n"));
		} else if (!strcmp(str, "electric")) {
			autostr_append(desc, _("Install to an electric socket\n"));
		} else if (!strcmp(str, "universal")) {
			autostr_append(desc, _("Install to a universal socket\n"));
		}
	}

	// Append item type requirements.
	str = spec->requires_item;
	if (str) {
		if (!strcmp(str, "melee weapon")) {
			autostr_append(desc, _("Install to a melee weapon\n"));
		} else if (!strcmp(str, "ranged weapon")) {
			autostr_append(desc, _("Install to a ranged weapon\n"));
		} else if (!strcmp(str, "armor")) {
			autostr_append(desc, _("Install to any armor\n"));
		} else if (!strcmp(str, "boots")) {
			autostr_append(desc, _("Install to boots\n"));
		} else if (!strcmp(str, "jacket")) {
			autostr_append(desc, _("Install to a jacket\n"));
		} else if (!strcmp(str, "shield")) {
			autostr_append(desc, _("Install to a shield\n"));
		} else if (!strcmp(str, "helmet")) {
			autostr_append(desc, _("Install to a helmet\n"));
		}
	}

	// Append bonuses provided by the add-on. We use a temporary item of
	// an arbitrary type to calculate the effects of the add-on.
	init_item(&temp_item);
	temp_item.type = 1;
	create_upgrade_socket(&temp_item, UPGRADE_SOCKET_TYPE_UNIVERSAL, ItemMap[spec->type].item_name);
	calculate_item_bonuses(&temp_item);
	get_item_bonus_string(&temp_item, "\n", desc);
	DeleteItem(&temp_item);
}

/**
 * \brief Applies an add-on bonus to the item.
 * \param it Item.
 * \param bonus Bonus to apply.
 */
static void apply_addon_bonus(item *it, struct addon_bonus *bonus)
{
	if (!strcmp(bonus->name, "all_attributes")) {
		it->bonus_to_all_attributes += bonus->value;
	} else if (!strcmp(bonus->name, "attack")) {
		it->bonus_to_attack += bonus->value;
	} else if (!strcmp(bonus->name, "armor")) {
		it->bonus_to_armor_class += bonus->value;
	} else if (!strcmp(bonus->name, "cooling")) {
		it->bonus_to_cooling += bonus->value;
	} else if (!strcmp(bonus->name, "cooling_rate")) {
		it->bonus_to_cooling_rate += bonus->value;
	} else if (!strcmp(bonus->name, "damage")) {
		it->damage += bonus->value;
		it->bonus_to_damage += bonus->value;
	} else if (!strcmp(bonus->name, "dexterity")) {
		it->bonus_to_dex += bonus->value;
	} else if (!strcmp(bonus->name, "experience_gain")) {
		it->bonus_to_experience_gain += bonus->value;
	} else if (!strcmp(bonus->name, "health")) {
		it->bonus_to_health_points += bonus->value;
	} else if (!strcmp(bonus->name, "health_recovery")) {
		it->bonus_to_health_recovery += bonus->value;
	} else if (!strcmp(bonus->name, "light_radius")) {
		it->bonus_to_light_radius += bonus->value;
	} else if (!strcmp(bonus->name, "paralyze_enemy")) {
		it->bonus_to_paralyze_enemy += bonus->value;
	} else if (!strcmp(bonus->name, "physique")) {
		it->bonus_to_physique += bonus->value;
	} else if (!strcmp(bonus->name, "slow_enemy")) {
		it->bonus_to_slow_enemy += bonus->value;
	} else if (!strcmp(bonus->name, "strength")) {
		it->bonus_to_str += bonus->value;
	}
}

/**
 * \brief Calculates the bonuses of the item.
 *
 * Can be used to initialize the bonuses of an item or to recalculate them
 * when, for example, adding add-ons to it.
 * \param it Item.
 */
void calculate_item_bonuses(item *it)
{
	int i;
	int j;
	const char *addon;
	struct addon_spec *spec;

	// Reset all the bonuses to defaults.
	it->bonus_to_dex = 0;
	it->bonus_to_str = 0;
	it->bonus_to_physique = 0;
	it->bonus_to_cooling = 0;
	it->bonus_to_health_points = 0;
	it->bonus_to_health_recovery = 0.0f;
	it->bonus_to_cooling_rate = 0.0f;
	it->bonus_to_attack = 0;
	it->bonus_to_all_attributes = 0;
	it->bonus_to_armor_class = 0;
	it->bonus_to_damage = 0;
	it->bonus_to_paralyze_enemy = 0;
	it->bonus_to_slow_enemy = 0;
	it->bonus_to_light_radius = 0;
	it->bonus_to_experience_gain = 0;
	it->damage = ItemMap[it->type].base_item_gun_damage;
	it->damage_modifier = ItemMap[it->type].item_gun_damage_modifier;
	if (it->type < 0) {
		return;
	}

	// Apply bonuses from add-ons.
	for (i = 0; i < it->upgrade_sockets.size; i++) {
		addon = it->upgrade_sockets.arr[i].addon;
		if (addon) {
			spec = get_addon_spec(GetItemIndexByName(addon));
			for (j = 0; j < spec->bonuses.size; j++) {
				apply_addon_bonus(it, &((struct addon_bonus*) spec->bonuses.arr)[j]);
			}
		}
	}
}

/**
 * \brief Calculates the number of used sockets.
 *
 * Can be used to count and return the number of used sockets.
 * \param it Item.
 * \return Number of used sockets
 */
int count_used_sockets(item *it)
{
	int i;
	int count = 0;
	const char *addon;

	for (i = 0; i < it->upgrade_sockets.size; i++) {
		addon = it->upgrade_sockets.arr[i].addon;
		if (addon)	
			count++;
	}

	return count;
}
