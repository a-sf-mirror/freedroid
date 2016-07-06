/* 
 *
 *   Copyright (c) 2009-2011 Arthur Huillet
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

#define _leveleditor_tile_lists_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_object_lists.h"

static int floor_tiles_array[] = {

	ISO_SIDEWALK_0,

	ISO_SIDEWALK_7,
	ISO_SIDEWALK_5,
	ISO_SIDEWALK_6,
	ISO_SIDEWALK_8,

	ISO_SIDEWALK_11,
	ISO_SIDEWALK_9,
	ISO_SIDEWALK_10,
	ISO_SIDEWALK_12,

	ISO_SIDEWALK_3,
	ISO_SIDEWALK_1,
	ISO_SIDEWALK_2,
	ISO_SIDEWALK_4,

	ISO_SIDEWALK_18,
	ISO_SIDEWALK_19,
	ISO_SIDEWALK_20,
	ISO_SIDEWALK_17,

	ISO_SIDEWALK_15,
	ISO_SIDEWALK_16,
	ISO_SIDEWALK_13,
	ISO_SIDEWALK_14,

	ISO_SIDEWALK_21,
	ISO_SIDEWALK_22,
	ISO_SIDEWALK_23,
	ISO_SIDEWALK_24,
	ISO_WATER_SIDEWALK_01,
	ISO_WATER_SIDEWALK_02,
	ISO_WATER_SIDEWALK_03,
	ISO_WATER_SIDEWALK_04,

	ISO_FLOOR_STONE_FLOOR,
	ISO_FLOOR_STONE_FLOOR_WITH_DOT,
	ISO_FLOOR_STONE_FLOOR_WITH_GRATE,

	ISO_MISCELLANEOUS_FLOOR_14,
	ISO_MISCELLANEOUS_FLOOR_10,
	ISO_MISCELLANEOUS_FLOOR_11,
	ISO_MISCELLANEOUS_FLOOR_12,
	ISO_MISCELLANEOUS_FLOOR_13,

	ISO_MISCELLANEOUS_FLOOR_23,
	ISO_MISCELLANEOUS_FLOOR_17,
	ISO_MISCELLANEOUS_FLOOR_18,
	ISO_MISCELLANEOUS_FLOOR_15,
	ISO_MISCELLANEOUS_FLOOR_16,

	ISO_WATER_ANIMATED_DARK,
	ISO_WATER_ANIMATED_SHALLOW,
	ISO_WATER,
	ISO_WATER_EDGE_1,
	ISO_WATER_EDGE_2,
	ISO_WATER_EDGE_3,
	ISO_WATER_EDGE_4,
	ISO_WATER_EDGE_5,
	ISO_WATER_EDGE_6,
	ISO_WATER_EDGE_7,
	ISO_WATER_EDGE_8,
	ISO_WATER_EDGE_9,
	ISO_WATER_EDGE_10,
	ISO_WATER_EDGE_11,
	ISO_WATER_EDGE_12,
	ISO_WATER_EDGE_13,
	ISO_WATER_EDGE_14,

	ISO_MISCELLANEOUS_FLOOR_9,
	ISO_FLOOR_EMPTY,
	ISO_FLOOR_ERROR_TILE,
	ISO_RED_WAREHOUSE_FLOOR,
	ISO_MISCELLANEOUS_FLOOR_21,
	ISO_MISCELLANEOUS_FLOOR_22,
	ISO_FLOOR_HOUSE_FLOOR,
	ISO_MISCELLANEOUS_FLOOR_19,
	ISO_MISCELLANEOUS_FLOOR_20,

	ISO_SAND_FLOOR_4,
	ISO_SAND_FLOOR_5,
	ISO_SAND_FLOOR_6,
	ISO_FLOOR_SAND,
	ISO_SAND_FLOOR_1,
	ISO_SAND_FLOOR_2,
	ISO_SAND_FLOOR_3,

	ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_1,
	ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_2,
	ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_3,
	ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_4,
	ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_5,
	ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_25,
	ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_26,
	ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_27,

	ISO_OVERLAY_GRASS_01,
	ISO_OVERLAY_GRASS_02,
	ISO_OVERLAY_GRASS_03,
	ISO_OVERLAY_GRASS_04,
	ISO_OVERLAY_GRASS_05,
	ISO_OVERLAY_GRASS_06,
	ISO_OVERLAY_GRASS_07,
	ISO_OVERLAY_GRASS_08,
	ISO_OVERLAY_GRASS_09,
	ISO_OVERLAY_GRASS_10,
	ISO_OVERLAY_GRASS_11,
	ISO_OVERLAY_GRASS_12,
	ISO_OVERLAY_GRASS_13,
	ISO_OVERLAY_GRASS_14,
	ISO_OVERLAY_GRASS_15,
	ISO_OVERLAY_GRASS_16,
	ISO_OVERLAY_GRASS_17,
	ISO_OVERLAY_GRASS_18,
	ISO_OVERLAY_GRASS_19,
	ISO_OVERLAY_GRASS_20,
	ISO_OVERLAY_GRASS_21,

	ISO_CARPET_TILE_0001,
	ISO_CARPET_TILE_0002,
	ISO_CARPET_TILE_0003,
	ISO_CARPET_TILE_0004,
	ISO_LARGE_SQUARE_BBB,
	ISO_LARGE_SQUARE_BRB,
	ISO_LARGE_SQUARE_BRR,
	ISO_LARGE_SQUARE_GBB,
	ISO_LARGE_SQUARE_GRB,
	ISO_LARGE_SQUARE_GRR,
	ISO_LARGE_SQUARE_RBB,
	ISO_LARGE_SQUARE_RRB,
	ISO_LARGE_SQUARE_RRR,
	ISO_MINI_SQUARE_0001,
	ISO_MINI_SQUARE_0002,
	ISO_MINI_SQUARE_0003,
	ISO_MINI_SQUARE_0004,
	ISO_MINI_SQUARE_0005,
	ISO_MINI_SQUARE_0006,
	ISO_MINI_SQUARE_0007,
	ISO_MINI_SQUARE_0008,
	ISO_SQUARE_TILE_AAB,
	ISO_SQUARE_TILE_ACB2,
	ISO_SQUARE_TILE_ACB,
	ISO_SQUARE_TILE_ADB2,
	ISO_SQUARE_TILE_ADB,
	ISO_SQUARE_TILE_CAB2,
	ISO_SQUARE_TILE_CAB,
	ISO_SQUARE_TILE_CCB,
	ISO_SQUARE_TILE_DAB2,
	ISO_SQUARE_TILE_DAB,
	ISO_SQUARE_TILE_DDB,

	ISO_COMPLICATED_CMM,
	ISO_COMPLICATED_CMM2,
	ISO_COMPLICATED_P4,
	ISO_COMPLICATED_PMG,
	ISO_COMPLICATED_PMG2,
	ISO_COMPLICATED_PMM,

	ISO_DISCO,

	ISO_TWOSQUARE_0001,
	ISO_TWOSQUARE_0002,
	ISO_TWOSQUARE_0003,
	ISO_GROUNDMARKER_BLUE_1,
	ISO_GROUNDMARKER_BLUE_2,
	ISO_GROUNDMARKER_BLUE_3,
	ISO_GROUNDMARKER_BLUE_4,
	ISO_GROUNDMARKER_BLUE_5,
	ISO_GROUNDMARKER_BLUE_6,
	ISO_GROUNDMARKER_BLUE_7,
	ISO_GROUNDMARKER_BLUE_8,
	ISO_GROUNDMARKER_RED_1,
	ISO_GROUNDMARKER_RED_2,
	ISO_GROUNDMARKER_RED_3,
	ISO_GROUNDMARKER_RED_4,
	ISO_GROUNDMARKER_RED_5,
	ISO_GROUNDMARKER_RED_6,
	ISO_GROUNDMARKER_RED_7,
	ISO_GROUNDMARKER_RED_8,
	ISO_GROUNDMARKER_BLUE_Y_1,
	ISO_GROUNDMARKER_BLUE_Y_2,
	ISO_GROUNDMARKER_BLUE_Y_3,
	ISO_GROUNDMARKER_BLUE_Y_4,
	ISO_GROUNDMARKER_BLUE_X_1,
	ISO_GROUNDMARKER_RED_Y_1,
	ISO_GROUNDMARKER_RED_Y_2,
	ISO_GROUNDMARKER_RED_Y_3,
	ISO_GROUNDMARKER_RED_Y_4,
	ISO_GROUNDMARKER_RED_X_1,
	ISO_SAND_EDGE_0001,
	ISO_SAND_EDGE_0002,
	ISO_SAND_EDGE_0003,
	ISO_SAND_EDGE_0004,
	ISO_SAND_EDGE_0005,
	ISO_SAND_EDGE_0006,
	ISO_SAND_EDGE_0007,
	ISO_SAND_EDGE_0008,
	ISO_SAND_EDGE_0009,
	ISO_SAND_EDGE_0010,
	ISO_SAND_EDGE_0011,
	ISO_SAND_EDGE_0012,
	ISO_SAND_EDGE_0013,
	ISO_SAND_EDGE_0014,
	-1
};

static int waypoint_array[] = {
	0,			//random spawn
	1,			//no random spawn 
	-1
};

static int map_label_array[] = {
	0,
	-1
};

int *floor_tiles_list = floor_tiles_array;
int *wall_tiles_list = NULL;
int *machinery_tiles_list = NULL;
int *furniture_tiles_list = NULL;
int *container_tiles_list = NULL;
int *nature_tiles_list = NULL;
int *misc_tiles_list = NULL;
int *waypoint_list = waypoint_array;
int *map_label_list = map_label_array;

int *sidewalk_floor_list;
int *water_floor_list;
int *grass_floor_list;
int *square_floor_list;
int *sand_floor_list;
int *other_floor_list;

int *melee_items_list;
int *gun_items_list;
int *defense_items_list;
int *spell_items_list;
int *other_items_list;
int *all_items_list;

int *droid_enemies_list = NULL;
int *human_enemies_list = NULL;
int *all_enemies_list = NULL;

void lvledit_set_obstacle_list_for_category(const char *category_name, struct dynarray *obstacles_filenames)
{
	const struct {
		const char *name;
		int **ptr;
	} obstacle_lists[] = {
		{ "WALL", &wall_tiles_list },
		{ "MACHINERY", &machinery_tiles_list },
		{ "FURNITURE", &furniture_tiles_list },
		{ "CONTAINER", &container_tiles_list },
		{ "NATURE", &nature_tiles_list },
		{ "OTHER", &misc_tiles_list }
	};

	int i, j;
	int *idx_list = MyMalloc(sizeof(int) * (obstacles_filenames->size + 1));

	// Translate obstacle filenames to obstacle indices
	int idx = 0;
	for (i = 0; i < obstacles_filenames->size; i++) {
		const char *filename = ((const char **)obstacles_filenames->arr)[i];

		for (j = 0; j < obstacle_map.size; j++) {
			const char *current_filename = ((char **)get_obstacle_spec(j)->filenames.arr)[0];
			if (!strcmp(filename, current_filename)) {
				// If the image isn't loaded the obstacle isn't inserted in the obstacle list
				if (!image_loaded(get_obstacle_image(j, 0))) {
					error_message(__FUNCTION__, "The image '%s' for obstacle %d isn't loaded. The obstacle won't be included in obstacle list in the leveleditor.",
						PLEASE_INFORM, current_filename, j);
					break;
				}

				idx_list[idx++] = j;
				break;
			}
		}

		if (j == obstacle_map.size)
			error_message(__FUNCTION__, "Could not find obstacle with the given filename: %s.", PLEASE_INFORM, filename);
	}
	idx_list[idx] = -1;

	for (i = 0; i < sizeof(obstacle_lists) / sizeof(obstacle_lists[0]); i++) {
		if (!strcmp(category_name, obstacle_lists[i].name)) {
			*obstacle_lists[i].ptr = idx_list;
			return;
		}
	}

	error_message(__FUNCTION__, "Unknown obstacle category: %s.", PLEASE_INFORM, category_name);
}

static void sort_floor_tiles_by_categories(struct dynarray *floor_tiles, int base, int *sidewalk, int *water, int *grass, int *square, int *sand, int *other)
{
	int i;

	for (i = 0; i < floor_tiles->size; i++) {
		struct floor_tile_spec *floor_tile = dynarray_member(floor_tiles, i, sizeof(struct floor_tile_spec));
		const char *current_filename = ((char **)floor_tile->filenames.arr)[0];

		if (strstr(current_filename, "sidewalk")) {
			sidewalk_floor_list[*sidewalk] = base + i;
			(*sidewalk)++;
			if (strstr(current_filename, "water")) { //Water-Sidewalk tiles should be in both
				water_floor_list[*water] = base + i;
				(*water)++;
			}
		} else if (strstr(current_filename, "water")) {
			water_floor_list[*water] = base + i;
			(*water)++;
		} else if (strstr(current_filename, "grass")) {
			grass_floor_list[*grass] = base + i;
			(*grass)++;
		} else if (strstr(current_filename, "square")) {
			square_floor_list[*square] = base + i;
			(*square)++;
		} else if (strstr(current_filename, "sand")) {
			sand_floor_list[*sand] = base + i;
			(*sand)++;
		} else if (strcmp(current_filename, "DUMMY_FLOOR_TILE")) {
			other_floor_list[*other] = base + i;
			(*other)++;
		}
	}
}

static void build_floor_tile_lists(void)
{
	int sidewalk = 0;
	int water    = 0;
	int grass    = 0;
	int square   = 0;
	int sand     = 0;
	int other    = 0;

	free(sidewalk_floor_list); //Sidewalk Tiles
	free(water_floor_list);    //Water Tiles
	free(grass_floor_list);    //Grass Tiles
	free(square_floor_list);   //Square Tiles - Geometric Patterned
	free(sand_floor_list);     //Sand Tiles
	free(other_floor_list);    //OTHER: Dirt, Sand, Carpet, Misc, etc.

	int num_floor_tiles = underlay_floor_tiles.size + overlay_floor_tiles.size;
	sidewalk_floor_list = MyMalloc(num_floor_tiles * sizeof(int));
	water_floor_list    = MyMalloc(num_floor_tiles * sizeof(int));
	grass_floor_list    = MyMalloc(num_floor_tiles * sizeof(int));
	square_floor_list   = MyMalloc(num_floor_tiles * sizeof(int));
	sand_floor_list     = MyMalloc(num_floor_tiles * sizeof(int));
	other_floor_list    = MyMalloc(num_floor_tiles * sizeof(int));

	sort_floor_tiles_by_categories(&underlay_floor_tiles, 0, &sidewalk, &water, &grass, &square, &sand, &other);
	sort_floor_tiles_by_categories(&overlay_floor_tiles, MAX_UNDERLAY_FLOOR_TILES, &sidewalk, &water, &grass, &square, &sand, &other);

	sidewalk_floor_list[sidewalk] = -1;
	water_floor_list[water]       = -1;
	grass_floor_list[grass]       = -1;
	square_floor_list[square]     = -1;
	sand_floor_list[sand]         = -1;
	other_floor_list[other]       = -1;
}

static void build_item_lists(void)
{
	int i;
	int melee   = 0;
	int guns    = 0;
	int defense = 0;
	int spell   = 0;
	int other   = 0;
	int all     = 0;

	free(melee_items_list);  //MELEE WEAPONS
	free(gun_items_list);    //GUNS
	free(defense_items_list);//ARMOR, SHIELDS, & SHOES
	free(spell_items_list);  //SPELL-LIKE: Grenades, Spell Books, Pills, & Potions
	free(other_items_list);  //OTHER: Ammo, Circuts, Plot Items, etc.
	free(all_items_list);    //EVERYTHING

	melee_items_list   = MyMalloc((Number_Of_Item_Types + 1) * sizeof(int));
	gun_items_list     = MyMalloc((Number_Of_Item_Types + 1) * sizeof(int));
	defense_items_list = MyMalloc((Number_Of_Item_Types + 1) * sizeof(int));
	spell_items_list   = MyMalloc((Number_Of_Item_Types + 1) * sizeof(int));
	other_items_list   = MyMalloc((Number_Of_Item_Types + 1) * sizeof(int));
	all_items_list     = MyMalloc((Number_Of_Item_Types + 1) * sizeof(int));

	for (i = 1; i < Number_Of_Item_Types; i++) { //Start at 1 to skip the CTD item
		if (ItemMap[i].weapon_is_melee) {
			melee_items_list[melee] = i;
			melee++;
		} else if (ItemMap[i].slot == WEAPON_SLOT) {
			gun_items_list[guns] = i;
			guns++;
		} else if (ItemMap[i].slot & (SHIELD_SLOT | HELM_SLOT | ARMOR_SLOT | BOOT_SLOT)) {
			defense_items_list[defense] = i;
			defense++;
		} else if (ItemMap[i].right_use.tooltip) {
			spell_items_list[spell] = i;
			spell++;
		} else {
			other_items_list[other] = i;
			other++;
		}
		all_items_list[all] = i;
		all++;
	}
	melee_items_list[melee]     = -1;
	gun_items_list[guns]        = -1;
	defense_items_list[defense] = -1;
	spell_items_list[spell]     = -1;
	other_items_list[other]     = -1;
	all_items_list[all]           = -1;

}

static void build_enemies_lists(void)
{
	int i;
	int humans = 0;
	int droids = 0;

	droid_enemies_list = MyMalloc((Number_Of_Droid_Types + 1) * sizeof(int));
	human_enemies_list = MyMalloc((Number_Of_Droid_Types + 1) * sizeof(int));
	all_enemies_list = MyMalloc((Number_Of_Droid_Types + 1) * sizeof(int));

	for (i = 0; i < Number_Of_Droid_Types; i++) {
		// XXX: Skip the "TRM" type because it doesn't have an atlas file
		if (!strcmp(Droidmap[i].droidname, "TRM"))
			continue;

		if (Droidmap[i].is_human)
			human_enemies_list[humans++] = i;
		else
			droid_enemies_list[droids++] = i;

		all_enemies_list[i] = i;
	}

	all_enemies_list[i] = droid_enemies_list[droids] = human_enemies_list[humans] = -1;
}

/**
 * This function builds all the lists of objects belonging to the various categories.
 */
void lvledit_build_tile_lists(void)
{
	build_floor_tile_lists();
	build_item_lists();
	build_enemies_lists();
}
