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

/* ----------------------------------------------------------------------
 * This file contains all functions for the heart of the level editor.
 * ---------------------------------------------------------------------- */

#ifndef _leveleditor_h_
#define _leveleditor_h_

#undef EXTERN
#ifndef _leveleditor_c
#define EXTERN extern
#else
#define EXTERN
#endif

struct quickbar_entry {
    struct list_head node;
    int id;
    int obstacle_type;
    int used;
};

typedef struct line_element {
    moderately_finepoint position;
    obstacle* address;

    list_head_t list;
} line_element, *Line_element;

enum ActionType
    {
	ACT_CREATE_OBSTACLE,
	ACT_REMOVE_OBSTACLE,
	ACT_WAYPOINT_TOGGLE,
	ACT_WAYPOINT_TOGGLE_CONNECT,
	ACT_TILE_FLOOR_SET,
	ACT_MULTIPLE_FLOOR_SETS,
	ACT_SET_OBSTACLE_LABEL,
	ACT_SET_MAP_LABEL,
	ACT_JUMP_TO_LEVEL
    };

typedef struct {
    struct list_head node;
    enum ActionType type;

    union {
	struct {
	    double x, y;
	    int new_obstacle_type;
	}create_obstacle;

	obstacle *delete_obstacle;

	struct {
	    int x, y;
	    int spawn_toggle;
	}waypoint_toggle; /* ToogleWaypoint */

	struct {
	    int x, y;
	    int type;
	}change_floor; /* 5663 */

	int number_fill_set; /* RecFill */
	
	struct {
	    obstacle *obstacle;
	    char *new_name;
	}change_obstacle_name; /* give_new_name_to_obstacle */

	struct {
	    int id;
	    char *new_name;
	}change_label_name; /* EditMapLabelData */

	struct {
		int target_level;
		double x, y;
	}jump_to_level;
    }d;
}action;

enum
  {
      REDO = -1, /* pop in to_redo and push in to_undo */
      NORMAL = 0, /* push only in to_undo */
      UNDO = 1 /* pop in to_undo and push in to_redo  */
  };

enum
  {
      NORMAL_MODE,
      DRAG_DROP_MODE,
      LINE_MODE,
      RECTANGLE_MODE,
      CLICK_DRAG_MODE,
  };


EXTERN void LevelEditor(void);
EXTERN void cycle_marked_obstacle( Level EditLevel );
EXTERN void CreateNewMapLevel( int level_num );
EXTERN void duplicate_all_obstacles_in_area ( Level source_level ,
				       float source_start_x , float source_start_y , 
				       float source_area_width , float source_area_height ,
				       Level target_level ,
				       float target_start_x , float target_start_y );
enum leveleditor_object_type {
    OBJECT_FLOOR,
    OBJECT_OBSTACLE,
    OBJECT_NPC, //just kidding, not implemented :)
    OBJECT_ANY,
};


EXTERN iso_image *leveleditor_get_object_image(enum leveleditor_object_type type, int * array, int idx);

#ifdef _leveleditor_c
int floor_tiles_list[] =    {
    ISO_FLOOR_ERROR_TILE ,
    ISO_FLOOR_STONE_FLOOR ,
    ISO_FLOOR_STONE_FLOOR_WITH_DOT , 
    ISO_FLOOR_STONE_FLOOR_WITH_GRATE ,
    ISO_FLOOR_SAND ,
    ISO_FLOOR_HOUSE_FLOOR ,
    ISO_SAND_FLOOR_1 ,
    ISO_SAND_FLOOR_2 ,
    ISO_SAND_FLOOR_3 ,
    ISO_SAND_FLOOR_4 ,
    ISO_SAND_FLOOR_5 ,
    ISO_SAND_FLOOR_6 ,

    ISO_WATER ,
    ISO_COMPLETELY_DARK ,
    ISO_RED_WAREHOUSE_FLOOR ,

    ISO_MISCELLANEOUS_FLOOR_9 ,
    ISO_MISCELLANEOUS_FLOOR_10 ,
    ISO_MISCELLANEOUS_FLOOR_11 ,
    ISO_MISCELLANEOUS_FLOOR_12 ,
    ISO_MISCELLANEOUS_FLOOR_13 ,
    ISO_MISCELLANEOUS_FLOOR_14 ,
    ISO_MISCELLANEOUS_FLOOR_15 ,
    ISO_MISCELLANEOUS_FLOOR_16 ,
    ISO_MISCELLANEOUS_FLOOR_17 ,
    ISO_MISCELLANEOUS_FLOOR_18 ,
    ISO_MISCELLANEOUS_FLOOR_19 ,
    ISO_MISCELLANEOUS_FLOOR_20 ,
    ISO_MISCELLANEOUS_FLOOR_21 ,
    ISO_MISCELLANEOUS_FLOOR_22 ,
    ISO_MISCELLANEOUS_FLOOR_23 , 

    ISO_SIDEWALK_1 ,
    ISO_SIDEWALK_2 ,
    ISO_SIDEWALK_3 ,
    ISO_SIDEWALK_4 ,
    ISO_SIDEWALK_5 ,
    ISO_SIDEWALK_6 ,
    ISO_SIDEWALK_7 ,
    ISO_SIDEWALK_8 ,
    ISO_SIDEWALK_9 ,
    ISO_SIDEWALK_10 ,
    ISO_SIDEWALK_11 ,
    ISO_SIDEWALK_12 ,
    ISO_SIDEWALK_13 ,
    ISO_SIDEWALK_14 ,
    ISO_SIDEWALK_15 ,
    ISO_SIDEWALK_16 ,
    ISO_SIDEWALK_17 ,
    ISO_SIDEWALK_18 ,
    ISO_SIDEWALK_19 ,
    ISO_SIDEWALK_20 ,

    ISO_FLOOR_SAND_WITH_GRASS_1 ,
    ISO_FLOOR_SAND_WITH_GRASS_2 ,
    ISO_FLOOR_SAND_WITH_GRASS_3 ,
    ISO_FLOOR_SAND_WITH_GRASS_4 ,
    ISO_FLOOR_SAND_WITH_GRASS_5 ,
    ISO_FLOOR_SAND_WITH_GRASS_6 ,
    ISO_FLOOR_SAND_WITH_GRASS_7 ,
    ISO_FLOOR_SAND_WITH_GRASS_8 ,
    ISO_FLOOR_SAND_WITH_GRASS_9 ,
    ISO_FLOOR_SAND_WITH_GRASS_10 ,

    ISO_FLOOR_SAND_WITH_GRASS_11 ,
    ISO_FLOOR_SAND_WITH_GRASS_12 ,
    ISO_FLOOR_SAND_WITH_GRASS_13 ,

    ISO_FLOOR_SAND_WITH_GRASS_14 ,
    ISO_FLOOR_SAND_WITH_GRASS_15 ,
    ISO_FLOOR_SAND_WITH_GRASS_16 ,
    ISO_FLOOR_SAND_WITH_GRASS_17 ,

    ISO_FLOOR_SAND_WITH_GRASS_18 ,
    ISO_FLOOR_SAND_WITH_GRASS_19 ,
    ISO_FLOOR_SAND_WITH_GRASS_20 ,
    ISO_FLOOR_SAND_WITH_GRASS_21 ,

    ISO_FLOOR_SAND_WITH_GRASS_22 ,
    ISO_FLOOR_SAND_WITH_GRASS_23 ,
    ISO_FLOOR_SAND_WITH_GRASS_24 ,
    ISO_FLOOR_SAND_WITH_GRASS_25 ,
    ISO_FLOOR_SAND_WITH_GRASS_26 ,
    ISO_FLOOR_SAND_WITH_GRASS_27 ,
    ISO_FLOOR_SAND_WITH_GRASS_28 ,
    ISO_FLOOR_SAND_WITH_GRASS_29 ,

    -1
};

int wall_tiles_list[] = { 
    ISO_TRANSP_FOR_WATER,

    ISO_OUTER_DOOR_V_00 ,
    ISO_OUTER_DOOR_H_00 ,
    ISO_OUTER_DOOR_V_LOCKED ,
    ISO_OUTER_DOOR_H_LOCKED ,
    ISO_OUTER_DOOR_V_OFFLINE ,
    ISO_OUTER_DOOR_H_OFFLINE ,
    ISO_OUTER_WALL_W1,
    ISO_OUTER_WALL_N1,
    ISO_OUTER_WALL_W2,
    ISO_OUTER_WALL_N2,
    ISO_OUTER_WALL_W3,
    ISO_OUTER_WALL_N3,
    ISO_OUTER_WALL_E1,
    ISO_OUTER_WALL_S1,
    ISO_OUTER_WALL_E2,
    ISO_OUTER_WALL_S2,
    ISO_OUTER_WALL_E3,
    ISO_OUTER_WALL_S3,
    ISO_OUTER_WALL_CORNER_1 ,
    ISO_OUTER_WALL_CORNER_4 ,
    ISO_OUTER_WALL_CORNER_3 ,
    ISO_OUTER_WALL_CORNER_2 ,
    ISO_OUTER_WALL_SMALL_CORNER_1 ,
    ISO_OUTER_WALL_SMALL_CORNER_4 ,
    ISO_OUTER_WALL_SMALL_CORNER_3 ,
    ISO_OUTER_WALL_SMALL_CORNER_2 ,
    ISO_V_DOOR_000_OPEN ,
    ISO_H_DOOR_000_OPEN ,
    ISO_DV_DOOR_000_OPEN ,
    ISO_DH_DOOR_000_OPEN ,
    ISO_V_DOOR_LOCKED,
    ISO_H_DOOR_LOCKED,
    ISO_DV_DOOR_LOCKED,
    ISO_DH_DOOR_LOCKED,
    ISO_V_WALL ,
    ISO_H_WALL ,
    ISO_V_WALL_WITH_DOT ,
    ISO_H_WALL_WITH_DOT ,
    ISO_GREY_WALL_END_W ,
    ISO_GREY_WALL_END_N ,
    ISO_GREY_WALL_END_E ,
    ISO_GREY_WALL_END_S ,
    ISO_GREY_WALL_CORNER_1 ,
    ISO_GREY_WALL_CORNER_2 ,
    ISO_GREY_WALL_CORNER_3 ,
    ISO_GREY_WALL_CORNER_4 ,
    ISO_ROOM_WALL_V_RED ,
    ISO_ROOM_WALL_H_RED ,
    ISO_RED_WALL_WINDOW_1,
    ISO_RED_WALL_WINDOW_2,
    ISO_ROOM_WALL_V_GREEN ,
    ISO_ROOM_WALL_H_GREEN ,
    ISO_CYAN_WALL_WINDOW_1,
    ISO_CYAN_WALL_WINDOW_2,
    ISO_LIGHT_GREEN_WALL_1,
    ISO_LIGHT_GREEN_WALL_2,
    ISO_FLOWER_WALL_WINDOW_1,
    ISO_FLOWER_WALL_WINDOW_2,
    ISO_FUNKY_WALL_1,
    ISO_FUNKY_WALL_2,
    ISO_FUNKY_WALL_3,
    ISO_FUNKY_WALL_4,
    ISO_FUNKY_WALL_WINDOW_1,
    ISO_FUNKY_WALL_WINDOW_2,
    ISO_CAVE_WALL_V ,
    ISO_CAVE_WALL_H ,
    ISO_CAVE_CORNER_NW ,
    ISO_CAVE_CORNER_NE ,
    ISO_CAVE_CORNER_SE ,
    ISO_CAVE_CORNER_SW ,
    ISO_CAVE_WALL_END_E ,
    ISO_CAVE_WALL_END_S ,
    ISO_CAVE_WALL_END_W ,
    ISO_CAVE_WALL_END_N ,
    ISO_V_WOOD_FENCE ,
    ISO_H_WOOD_FENCE , 
    ISO_V_DENSE_FENCE ,
    ISO_H_DENSE_FENCE ,
    ISO_V_MESH_FENCE ,
    ISO_H_MESH_FENCE , 
    ISO_V_WIRE_FENCE ,
    ISO_H_WIRE_FENCE ,
    ISO_GLASS_WALL_1,
    ISO_GLASS_WALL_2,
    ISO_THICK_WALL_V ,
    ISO_THICK_WALL_H ,
    ISO_THICK_WALL_CORNER_NW ,
    ISO_THICK_WALL_CORNER_NE ,
    ISO_THICK_WALL_CORNER_SE ,
    ISO_THICK_WALL_CORNER_SW ,
    ISO_THICK_WALL_T_W ,
    ISO_THICK_WALL_T_N ,
    ISO_THICK_WALL_T_E ,
    ISO_THICK_WALL_T_S ,
    ISO_BRICK_WALL_H ,
    ISO_BRICK_WALL_V ,
    ISO_BRICK_WALL_EH ,
    ISO_BRICK_WALL_EV ,
    ISO_BRICK_WALL_END ,
    ISO_BRICK_WALL_CORNER_3 , 
    ISO_BRICK_WALL_CORNER_1 , 
    ISO_BRICK_WALL_CORNER_4 ,
    ISO_BRICK_WALL_CORNER_2 , 
    ISO_BRICK_WALL_JUNCTION_4 ,
    ISO_BRICK_WALL_JUNCTION_1 ,
    ISO_BRICK_WALL_JUNCTION_2 ,
    ISO_BRICK_WALL_JUNCTION_3 ,
    ISO_BRICK_WALL_CRACKED_1 ,
    ISO_BRICK_WALL_CRACKED_2 ,
    ISO_BRICK_WALL_RUBBLE_1 ,
    ISO_BRICK_WALL_RUBBLE_2 ,
    ISO_BRICK_WALL_CABLES_V ,
    ISO_BRICK_WALL_CABLES_H ,
    ISO_BRICK_WALL_CABLES_CORNER_3 , 
    ISO_BRICK_WALL_CABLES_CORNER_1 , 
    ISO_BRICK_WALL_CABLES_CORNER_4 ,
    ISO_BRICK_WALL_CABLES_CORNER_2 , 
    ISO_RED_FENCE_V,
    ISO_RED_FENCE_H,
    ISO_BLOCK_1 ,
    ISO_BLOCK_2 ,
    ISO_V_CURTAIN ,
    ISO_H_CURTAIN ,



    -1
};


int machinery_tiles_list[] = {
    ISO_ENHANCER_RU ,
    ISO_ENHANCER_LU ,
    ISO_ENHANCER_RD ,
    ISO_ENHANCER_LD ,

    ISO_TELEPORTER_3 ,

    ISO_REFRESH_3 ,

    ISO_AUTOGUN_W ,
    ISO_AUTOGUN_N ,
    ISO_AUTOGUN_E ,
    ISO_AUTOGUN_S ,

    ISO_DIS_AUTOGUN_W ,
    ISO_DIS_AUTOGUN_N ,
    ISO_DIS_AUTOGUN_E ,
    ISO_DIS_AUTOGUN_S ,

    ISO_COOKING_POT ,

    ISO_CONSOLE_S ,
    ISO_CONSOLE_E ,
    ISO_CONSOLE_N ,
    ISO_CONSOLE_W ,

    ISO_TV_PILLAR_W ,
    ISO_TV_PILLAR_N ,
    ISO_TV_PILLAR_E ,
    ISO_TV_PILLAR_S ,

    ISO_PROJECTOR_N ,
    ISO_PROJECTOR_E ,
    ISO_PROJECTOR_S ,
    ISO_PROJECTOR_W ,

    ISO_SIGN_1 ,
    ISO_SIGN_2 ,
    ISO_SIGN_3 ,

    ISO_EXIT_2 ,
    ISO_EXIT_1 ,
    ISO_EXIT_4 ,
    ISO_EXIT_3 ,

    -1,
};

int furniture_tiles_list[] = {
    ISO_LAMP_N ,  
    ISO_LAMP_E , 
    ISO_LAMP_S ,
    ISO_LAMP_W ,
    ISO_N_TOILET_SMALL ,
    ISO_E_TOILET_SMALL ,
    ISO_S_TOILET_SMALL ,
    ISO_W_TOILET_SMALL ,
    ISO_N_TOILET_BIG ,
    ISO_E_TOILET_BIG ,
    ISO_S_TOILET_BIG ,
    ISO_W_TOILET_BIG ,
    ISO_N_CHAIR ,
    ISO_E_CHAIR ,
    ISO_S_CHAIR ,
    ISO_W_CHAIR ,
    ISO_N_DESK ,
    ISO_E_DESK ,
    ISO_S_DESK ,
    ISO_W_DESK ,
    ISO_N_SCHOOL_CHAIR ,
    ISO_E_SCHOOL_CHAIR ,
    ISO_S_SCHOOL_CHAIR ,
    ISO_W_SCHOOL_CHAIR ,
    ISO_DESKCHAIR_1,
    ISO_DESKCHAIR_2,
    ISO_DESKCHAIR_3,

    ISO_N_BED ,
    ISO_E_BED ,
    ISO_S_BED ,
    ISO_W_BED ,
    ISO_N_FULL_PARK_BENCH ,
    ISO_E_FULL_PARK_BENCH ,
    ISO_S_FULL_PARK_BENCH ,
    ISO_W_FULL_PARK_BENCH ,

    ISO_H_BATHTUB , 
    ISO_V_BATHTUB ,
    ISO_3_BATHTUB ,
    ISO_4_BATHTUB ,
    ISO_H_WASHTUB , 
    ISO_V_WASHTUB ,

    ISO_BASIN_1,
    ISO_BASIN_2,
    ISO_BASIN_3,
    ISO_BASIN_4,
    ISO_E_SOFA , 
    ISO_S_SOFA , 
    ISO_W_SOFA , 
    ISO_N_SOFA ,

    ISO_TABLE_OVAL_1,
    ISO_TABLE_OVAL_2,
    ISO_TABLE_GLASS_1,
    ISO_TABLE_GLASS_2,

    ISO_SHOP_FURNITURE_1,
    ISO_SHOP_FURNITURE_2,
    ISO_SHOP_FURNITURE_3,
    ISO_SHOP_FURNITURE_4,
    ISO_SHOP_FURNITURE_5,
    ISO_SHOP_FURNITURE_6,

    ISO_LIBRARY_FURNITURE_1,
    ISO_LIBRARY_FURNITURE_2,

    ISO_YELLOW_CHAIR_N ,
    ISO_YELLOW_CHAIR_E ,
    ISO_YELLOW_CHAIR_S ,
    ISO_YELLOW_CHAIR_W ,
    ISO_RED_CHAIR_N ,
    ISO_RED_CHAIR_E ,
    ISO_RED_CHAIR_S ,
    ISO_RED_CHAIR_W ,

    ISO_SOFFA_1,
    ISO_SOFFA_2,
    ISO_SOFFA_3,
    ISO_SOFFA_4,
    ISO_SOFFA_CORNER_1,
    ISO_SOFFA_CORNER_2,
    ISO_SOFFA_CORNER_3,
    ISO_SOFFA_CORNER_4,
    ISO_SOFFA_CORNER_PLANT_1,
    ISO_SOFFA_CORNER_PLANT_2,
    ISO_SOFFA_CORNER_PLANT_3,
    ISO_SOFFA_CORNER_PLANT_4,

    ISO_CONFERENCE_TABLE_N,
    ISO_CONFERENCE_TABLE_E,
    ISO_CONFERENCE_TABLE_S,
    ISO_CONFERENCE_TABLE_W,

    ISO_PROJECTOR_SCREEN_N ,
    ISO_PROJECTOR_SCREEN_E ,
    ISO_PROJECTOR_SCREEN_S ,
    ISO_PROJECTOR_SCREEN_W ,

    ISO_BED_1,
    ISO_BED_2,
    ISO_BED_3,
    ISO_BED_4,
    ISO_BED_5,
    ISO_BED_6,
    ISO_BED_7,
    ISO_BED_8,

    ISO_SHELF_FULL_V,
    ISO_SHELF_FULL_H,
    ISO_SHELF_EMPTY_V,
    ISO_SHELF_EMPTY_H,
    ISO_SHELF_SMALL_FULL_V,
    ISO_SHELF_SMALL_FULL_H,
    ISO_SHELF_SMALL_EMPTY_V,
    ISO_SHELF_SMALL_EMPTY_H,

    ISO_RESTAURANT_SHELVES_1,
    ISO_RESTAURANT_SHELVES_2,
    ISO_RESTAURANT_SHELVES_3,
    ISO_RESTAURANT_SHELVES_4,
    ISO_RESTAURANT_SHELVES_5,
    ISO_RESTAURANT_SHELVES_6,
    ISO_RESTAURANT_SHELVES_7,
    ISO_RESTAURANT_SHELVES_8,
    ISO_RESTAURANT_SHELVES_9,
    ISO_RESTAURANT_SHELVES_10,



    ISO_COUNTER_MIDDLE_1,
    ISO_COUNTER_MIDDLE_2,
    ISO_COUNTER_MIDDLE_3,
    ISO_COUNTER_MIDDLE_4,
    ISO_COUNTER_CORNER_ROUND_1,
    ISO_COUNTER_CORNER_ROUND_2,
    ISO_COUNTER_CORNER_ROUND_3,
    ISO_COUNTER_CORNER_ROUND_4,
    ISO_COUNTER_CORNER_SHARP_1,
    ISO_COUNTER_CORNER_SHARP_2,
    ISO_COUNTER_CORNER_SHARP_3,
    ISO_COUNTER_CORNER_SHARP_4,

    ISO_BAR_TABLE,
    ISO_RESTAURANT_DESK_1,
    ISO_RESTAURANT_DESK_2,
    ISO_RESTAURANT_BIGSHELF_1,
    ISO_RESTAURANT_BIGSHELF_2,

    -1,
};

int container_tiles_list[] = {
    ISO_H_CHEST_CLOSED ,
    ISO_V_CHEST_CLOSED ,
    ISO_H_CHEST_OPEN ,
    ISO_V_CHEST_OPEN ,

    ISO_BARREL_1 ,
    ISO_BARREL_2 ,
    ISO_BARREL_3 ,
    ISO_BARREL_4 ,

    //--------------------
    // We repeat the same obstacles once more, cause we should have at
    // least 10 things in each groups for technical reasons...
    //
    ISO_BARREL_1 ,
    ISO_BARREL_2 ,
    ISO_BARREL_3 ,
    ISO_BARREL_4 ,

    -1,
};

int plant_tiles_list[] = {
    ISO_TREE_1 ,
    ISO_TREE_2 ,
    ISO_TREE_3 ,
    ISO_TREE_4 ,
    ISO_TREE_5 ,

    ISO_ROCKS_N_PLANTS_1 ,
    ISO_ROCKS_N_PLANTS_2 ,
    ISO_ROCKS_N_PLANTS_3 ,
    ISO_ROCKS_N_PLANTS_4 ,
    ISO_ROCKS_N_PLANTS_5 ,
    ISO_ROCKS_N_PLANTS_6 ,
    ISO_ROCKS_N_PLANTS_7 ,
    ISO_ROCKS_N_PLANTS_8 ,

    ISO_CRYSTALS_1 ,
    ISO_CRYSTALS_2 ,
    ISO_CRYSTALS_3 ,
    ISO_CRYSTALS_4 ,
    ISO_CRYSTALS_5 ,
    ISO_CRYSTALS_6 ,

    -1,
};

#else
EXTERN int floor_tiles_list[];
EXTERN int wall_tiles_list[];
EXTERN int furniture_tiles_list[];
EXTERN int plant_tiles_list[];
EXTERN int machinery_tiles_list[];
EXTERN int container_tiles_list[];
#endif

EXTERN int EditX();
EXTERN int EditY();
EXTERN level *EditLevel();

EXTERN char VanishingMessage[10000];
EXTERN float VanishingMessageEndDate;

EXTERN int OriginWaypoint;
EXTERN int number_of_walls [ NUMBER_OF_LEVEL_EDITOR_GROUPS ] ;

EXTERN iso_image * quickbar_getimage(int, int *);
EXTERN int marked_obstacle_is_glued_to_here (level *, float, float);

EXTERN int level_editor_done;

EXTERN int object_type;
EXTERN int *object_list;
#endif
