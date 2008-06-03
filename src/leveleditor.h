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

enum
  {
    JUMP_THRESHOLD_NORTH = 1,
    JUMP_THRESHOLD_SOUTH ,
    JUMP_THRESHOLD_EAST ,
    JUMP_THRESHOLD_WEST ,
    JUMP_TARGET_NORTH ,
    JUMP_TARGET_SOUTH ,
    JUMP_TARGET_EAST ,
    JUMP_TARGET_WEST ,
    EXPORT_THIS_LEVEL , 
    REPORT_INTERFACE_INCONSISTENCIES , 
    QUIT_THRESHOLD_EDITOR_POSITION
  };

enum
  {
    INSERTREMOVE_COLUMN_VERY_WEST = 1,
    INSERTREMOVE_COLUMN_WESTERN_INTERFACE,
    INSERTREMOVE_COLUMN_EASTERN_INTERFACE,
    INSERTREMOVE_COLUMN_VERY_EAST,
    INSERTREMOVE_LINE_VERY_NORTH,
    INSERTREMOVE_LINE_NORTHERN_INTERFACE,
    INSERTREMOVE_LINE_SOUTHERN_INTERFACE,
    INSERTREMOVE_LINE_VERY_SOUTH,
    DUMMY_NO_REACTION1,
    DUMMY_NO_REACTION2,
    BACK_TO_LE_MAIN_MENU
  };

enum ActionType
    {
	ACT_CREATE_OBSTACLE,
	ACT_REMOVE_OBSTACLE,
	ACT_WAYPOINT_TOGGLE,
	ACT_WAYPOINT_TOGGLE_CONNECT,
	ACT_TILE_FLOOR_SET,
	ACT_MULTIPLE_FLOOR_SETS,
	ACT_SET_OBSTACLE_LABEL,
	ACT_SET_MAP_LABEL
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


typedef struct leveleditor_state_s {
    /* Current postion */
    moderately_finepoint TargetSquare;

    int mode;

    /* drag&drop */
    obstacle *d_selected_obstacle;

    /* Line mode */
    int l_direction;
    int l_selected_mode;
    int l_id;
    line_element l_elements;

    /* Rectangle mode */
    point r_start;
    int r_len_x, r_len_y;
    int r_step_x, r_step_y;
    int r_tile_used;

    /* click&drag */
    point c_last_right_click;
    moderately_finepoint c_corresponding_position;
} leveleditor_state;


void ShowWaypoints( int PrintConnectionList , int maks );
void LevelEditor(void);
void cycle_marked_obstacle( Level EditLevel );
void CreateNewMapLevel( void );
void SetLevelInterfaces ( void );
void duplicate_all_obstacles_in_area ( Level source_level ,
				       float source_start_x , float source_start_y , 
				       float source_area_width , float source_area_height ,
				       Level target_level ,
				       float target_start_x , float target_start_y );
void give_new_description_to_obstacle ( Level EditLevel , obstacle* our_obstacle , char* predefined_description );

/* Line mode */
void start_line_mode(leveleditor_state *cur_state, int already_defined);

/* Rectangle mode */
void start_rectangle_mode(leveleditor_state *cur_state , int already_defined);


/* Undoable actions*/
obstacle *
action_create_obstacle (Level EditLevel, double x, double y, int new_obstacle_type);
obstacle *
action_create_obstacle_user (Level EditLevel, double x, double y, int new_obstacle_type);
void action_remove_obstacle_user ( Level EditLevel, obstacle *our_obstacle);
void action_remove_obstacle ( Level EditLevel, obstacle *our_obstacle);
void action_toggle_waypoint ( Level EditLevel , int BlockX , int BlockY , int toggle_random_spawn );
int action_toggle_waypoint_connection ( Level EditLevel, int id_origin, int id_target);
void action_toggle_waypoint_connection_user ( Level EditLevel , int BlockX , int BlockY );
void action_set_floor ( Level EditLevel, int x, int y, int type);
void action_fill_user_recursive ( Level EditLevel, int x, int y, int type, int *changed);
void action_fill_user ( Level EditLevel, int BlockX, int BlockY, int SpecialMapValue);
void action_change_obstacle_label ( Level EditLevel, obstacle *obstacle, char *name);
void action_change_obstacle_label_user ( Level EditLevel, obstacle *our_obstacle, char *predefined_name);
void action_change_map_label ( Level EditLevel, int i, char *name );
void action_change_map_label_user ( Level EditLevel );



int wall_indices [ NUMBER_OF_LEVEL_EDITOR_GROUPS ] [ NUMBER_OF_OBSTACLE_TYPES ] = 
{
    //--------------------
    // First the floor group.  (this is a pure dummy right now...)
    //
    {
	-1,
	-1,
	-1
    },
    //--------------------
    // Now the 'walls' group.
    //
    { 
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
	ISO_H_DOOR_000_OPEN ,
	ISO_V_DOOR_000_OPEN ,
	ISO_H_DOOR_LOCKED,
	ISO_V_DOOR_LOCKED,
	ISO_CAVE_WALL_H ,
	ISO_CAVE_WALL_V ,
	ISO_CAVE_CORNER_NE ,
	ISO_CAVE_CORNER_SE ,
	ISO_CAVE_CORNER_NW ,
	ISO_CAVE_CORNER_SW ,
	ISO_CAVE_WALL_END_W ,
	ISO_CAVE_WALL_END_N ,
	ISO_CAVE_WALL_END_E ,
	ISO_CAVE_WALL_END_S ,
	ISO_V_WOOD_FENCE ,
	ISO_H_WOOD_FENCE , 
	ISO_V_DENSE_FENCE ,
	ISO_H_DENSE_FENCE ,
	ISO_V_MESH_FENCE ,
	ISO_H_MESH_FENCE , 
	ISO_V_WIRE_FENCE ,
	ISO_H_WIRE_FENCE ,
	ISO_V_CURTAIN ,
	ISO_H_CURTAIN ,
	ISO_GLASS_WALL_1,
	ISO_GLASS_WALL_2,
	ISO_THICK_WALL_H ,
	ISO_THICK_WALL_V ,
	ISO_THICK_WALL_CORNER_NE ,
	ISO_THICK_WALL_CORNER_SE ,
	ISO_THICK_WALL_CORNER_NW ,
	ISO_THICK_WALL_CORNER_SW ,
	ISO_THICK_WALL_T_N ,
	ISO_THICK_WALL_T_E ,
	ISO_THICK_WALL_T_S ,
	ISO_THICK_WALL_T_W ,
	ISO_BRICK_WALL_H ,
	ISO_BRICK_WALL_V ,
	ISO_BRICK_WALL_EH ,
	ISO_BRICK_WALL_EV ,
	ISO_BRICK_WALL_END ,
	ISO_BRICK_WALL_CORNER_1 , 
	ISO_BRICK_WALL_CORNER_2 , 
	ISO_BRICK_WALL_CORNER_3 , 
	ISO_BRICK_WALL_CORNER_4 ,
	ISO_BRICK_WALL_JUNCTION_1 ,
	ISO_BRICK_WALL_JUNCTION_2 ,
	ISO_BRICK_WALL_JUNCTION_3 ,
	ISO_BRICK_WALL_JUNCTION_4 ,
        ISO_BRICK_WALL_CRACKED_1 ,
	ISO_BRICK_WALL_CRACKED_2 ,
	ISO_BRICK_WALL_RUBBLE_1 ,
	ISO_BRICK_WALL_RUBBLE_2 ,

	ISO_BRICK_WALL_CABLES_H ,
	ISO_BRICK_WALL_CABLES_V ,
	ISO_BRICK_WALL_CABLES_CORNER_1 , 
	ISO_BRICK_WALL_CABLES_CORNER_2 , 
	ISO_BRICK_WALL_CABLES_CORNER_3 , 
	ISO_BRICK_WALL_CABLES_CORNER_4 ,

	ISO_OUTER_WALL_N1,
	ISO_OUTER_WALL_N2,
	ISO_OUTER_WALL_N3,
	ISO_OUTER_WALL_S1,
	ISO_OUTER_WALL_S2,
	ISO_OUTER_WALL_S3,
	
	ISO_OUTER_WALL_E1,
	ISO_OUTER_WALL_E2,
	ISO_OUTER_WALL_E3,
	ISO_OUTER_WALL_W1,
	ISO_OUTER_WALL_W2,
	ISO_OUTER_WALL_W3,
	
	ISO_OUTER_WALL_CORNER_1 ,
	ISO_OUTER_WALL_CORNER_2 ,
	ISO_OUTER_WALL_CORNER_3 ,
	ISO_OUTER_WALL_CORNER_4 ,
	
	ISO_OUTER_WALL_SMALL_CORNER_1 ,
	ISO_OUTER_WALL_SMALL_CORNER_2 ,
	ISO_OUTER_WALL_SMALL_CORNER_3 ,
	ISO_OUTER_WALL_SMALL_CORNER_4 ,

	ISO_RED_FENCE_V,
	ISO_RED_FENCE_H,

	ISO_TRANSP_FOR_WATER,
	-1
    } ,
    //--------------------
    // Now the 'machinery' group.
    //
    {
	ISO_ENHANCER_RU ,
	ISO_ENHANCER_LU ,
	ISO_ENHANCER_RD ,
	ISO_ENHANCER_LD ,

	ISO_TELEPORTER_1 ,
	ISO_TELEPORTER_2 ,
	ISO_TELEPORTER_3 ,
	ISO_TELEPORTER_4 ,
	ISO_TELEPORTER_5 ,
	
	ISO_REFRESH_1 ,
	ISO_REFRESH_2 ,
	ISO_REFRESH_3 ,
	ISO_REFRESH_4 ,
	ISO_REFRESH_5 ,

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

	-1,
	-1,
	-1,
	-1
    } ,
    //--------------------
    // Now the 'furniture' group.
    //
    {
	ISO_BLOCK_1 ,
	ISO_BLOCK_2 ,

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
	ISO_V_CURTAIN ,
	ISO_H_CURTAIN ,
	ISO_E_SOFA , 
	ISO_S_SOFA , 
	ISO_W_SOFA , 
	ISO_N_SOFA ,
	
	ISO_TABLE_OVAL_1,
	ISO_TABLE_OVAL_2,
	ISO_TABLE_GLASS_1,
	ISO_TABLE_GLASS_2,

	ISO_EXIT_1 ,
	ISO_EXIT_2 ,
	ISO_EXIT_3 ,
	ISO_EXIT_4 ,

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
	-1,
	-1,
	-1
    } ,
    //--------------------
    // Now the 'containers' group.
    //
    {
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
	-1,
	-1,
	-1
    } ,
    //--------------------
    // Now the 'plants' group.
    //
    {
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
	-1,
	-1,
	-1
    } ,
    //--------------------
    // Now the 'all' group.  (this is a pure dummy...)
    //
    {
	-1,
	-1,
	-1,
	-1
    }
}; // end of definition of selection groups



#endif
