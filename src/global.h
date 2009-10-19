/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
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

#ifndef _global_h
#define _global_h

#include "BFont.h"
#include "lists.h"

#undef EXTERN
#ifdef _main_c
#define EXTERN
#else
#define EXTERN extern

EXTERN char *floor_tile_filenames[ALL_ISOMETRIC_FLOOR_TILES];
EXTERN obstacle_spec obstacle_map[NUMBER_OF_OBSTACLE_TYPES];
EXTERN float FPSover1;
EXTERN float FPSover10;
EXTERN float FPSover100;
EXTERN char *AllSkillTexts[];
EXTERN char font_switchto_red[2];
EXTERN char font_switchto_blue[2];
EXTERN char font_switchto_neon[2];
EXTERN char font_switchto_msgstat[2];
EXTERN char font_switchto_msgvar[2];

EXTERN int SpellHitPercentageTable[];
EXTERN float MeleeDamageMultiplierTable[];
EXTERN float MeleeRechargeMultiplierTable[];
EXTERN float RangedDamageMultiplierTable[];
EXTERN float RangedRechargeMultiplierTable[];
EXTERN spell_skill_spec *SpellSkillMap;
EXTERN tux_t Me;		/* the influence data */
EXTERN Druidspec Druidmap;
EXTERN Bulletspec Bulletmap;
EXTERN blastspec Blastmap[ALLBLASTTYPES];
EXTERN int skip_initial_menus;
EXTERN int number_of_skills;
EXTERN int last_bot_number;
EXTERN enum { INSIDE_MENU = 0, INSIDE_GAME, INSIDE_LVLEDITOR } game_status;
#endif

EXTERN SDL_Rect User_Rect;
EXTERN SDL_Rect Full_User_Rect;
EXTERN SDL_Rect Cons_Text_Rect;

extern char *our_homedir;
extern char *our_config_dir;

EXTERN int Number_Of_Droid_Types;
EXTERN int QuitProgram;
EXTERN int GameOver;

extern list_head_t alive_bots_head;
extern list_head_t dead_bots_head;
extern list_head_t level_bots_head[MAX_LEVELS];

EXTERN spell_active AllActiveSpells[MAX_ACTIVE_SPELLS];
EXTERN ship curShip;		/* the current ship-data */

EXTERN bullet AllBullets[MAXBULLETS + 10];
EXTERN melee_shot AllMeleeShots[MAX_MELEE_SHOTS];
EXTERN blast AllBlasts[MAXBLASTS + 10];

EXTERN int sound_on;		// Toggle TRUE/FALSE for turning sounds on/off 
EXTERN int debug_level;		// 0=no debug 1=some debug messages 2=...etc 
				// (currently only 0 or !=0 is implemented) 
EXTERN int show_all_droids;	// display enemys regardless of IsVisible() 
EXTERN int draw_collision_rectangles;	// to better debug collision rectangles
EXTERN int draw_grid;		// grid to see where objects will be positioned
#undef EXTERN
#ifdef _misc_c
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN float timeout_from_item_drop;
EXTERN int use_open_gl;
EXTERN int command_line_override_for_screen_resolution;
EXTERN char *character_descriptions[MAX_PERSONS];

EXTERN char previous_part_strings[ALL_PART_GROUPS][50];
EXTERN mouse_press_button AllMousePressButtons[MAX_MOUSE_PRESS_BUTTONS];
EXTERN int Item_Held_In_Hand;
EXTERN point InventorySize;

EXTERN iso_image MouseCursorImageList[NUMBER_OF_MOUSE_CURSOR_PICTURES];
EXTERN iso_image SpellLevelButtonImageList[NUMBER_OF_SKILL_PAGES];

EXTERN int Number_Of_Droids_On_Ship;
EXTERN configuration_for_freedroid GameConfig;
EXTERN BFont_Info *Menu_BFont;
EXTERN BFont_Info *Messagevar_BFont;
EXTERN BFont_Info *Messagestat_BFont;
EXTERN BFont_Info *Para_BFont;
EXTERN BFont_Info *FPS_Display_BFont;
EXTERN BFont_Info *Blue_BFont;
EXTERN BFont_Info *Red_BFont;
EXTERN BFont_Info *Highscore_BFont;
EXTERN float Overall_Average;
EXTERN int SkipAFewFrames;

EXTERN int global_ingame_mode;

#undef EXTERN
#ifdef _view_c
#define EXTERN
#else
#define EXTERN extern
#endif
EXTERN iso_image light_radius_chunk[NUMBER_OF_SHADOW_IMAGES];
EXTERN SDL_Rect InventoryRect;
EXTERN SDL_Rect CharacterRect;
#ifdef HAVE_LIBGL
EXTERN GLuint automap_texture;	// this is to store an open_gl texture...
EXTERN GLuint light_radius_stretch_texture;	// this is to store an open_gl texture...
EXTERN SDL_Surface *light_radius_stretch_surface;
#endif
EXTERN int amask;
EXTERN int gmask;
EXTERN int bmask;
EXTERN int rmask;

struct neighbor_data_cell {
	int delta_x;
	int delta_y;
	int lvl_idx;
	int valid;
};
EXTERN struct neighbor_data_cell gps_transform_matrix[MAX_LEVELS][MAX_LEVELS];
EXTERN struct neighbor_data_cell *(level_neighbors_map[MAX_LEVELS][3][3]);
EXTERN list_head_t visible_level_list;
EXTERN int gps_transform_map_dirty_flag;

#define NEIGHBOR_IDX(x,len) ( ( (x) < 0) ? (0) : ( ( (x) < (len) ) ? (1) : (2) ) )
#define NEIGHBOR_ID_NW(lvl) ( level_neighbors_map[lvl][0][0] ? level_neighbors_map[lvl][0][0]->lvl_idx : -1 )
#define NEIGHBOR_ID_N(lvl)  ( level_neighbors_map[lvl][0][1] ? level_neighbors_map[lvl][0][1]->lvl_idx : -1 )
#define NEIGHBOR_ID_NE(lvl) ( level_neighbors_map[lvl][0][2] ? level_neighbors_map[lvl][0][2]->lvl_idx : -1 )
#define NEIGHBOR_ID_W(lvl)  ( level_neighbors_map[lvl][1][0] ? level_neighbors_map[lvl][1][0]->lvl_idx : -1 )
#define NEIGHBOR_ID_E(lvl)  ( level_neighbors_map[lvl][1][2] ? level_neighbors_map[lvl][1][2]->lvl_idx : -1 )
#define NEIGHBOR_ID_SW(lvl) ( level_neighbors_map[lvl][2][0] ? level_neighbors_map[lvl][2][0]->lvl_idx : -1 )
#define NEIGHBOR_ID_S(lvl)  ( level_neighbors_map[lvl][2][1] ? level_neighbors_map[lvl][2][1]->lvl_idx : -1 )
#define NEIGHBOR_ID_SE(lvl) ( level_neighbors_map[lvl][2][2] ? level_neighbors_map[lvl][2][2]->lvl_idx : -1 )

#undef EXTERN
#ifdef _light_c
#define EXTERN
#else
#define EXTERN extern
#endif
EXTERN light_radius_config LightRadiusConfig;

#undef EXTERN
#ifdef _event_c
#define EXTERN
#else
#define EXTERN extern
#endif
EXTERN event_trigger AllEventTriggers[MAX_EVENT_TRIGGERS];

#undef EXTERN
#ifdef _chat_c
#define EXTERN
#else
#define EXTERN extern
#endif
int chat_control_next_node;	//what is the next node to use?
int chat_control_end_dialog;	//end current dialog?
int chat_control_partner_code;	//our partner code to access chat flags from Lua
int chat_control_partner_started;	//the dialog partner is the one who started the talk
enemy *chat_control_chat_droid;	//droid we are chatting with
dialogue_option ChatRoster[MAX_DIALOGUE_OPTIONS_IN_ROSTER];

#undef EXTERN
#ifdef _sound_c
#define EXTERN
#else
#define EXTERN extern
#endif

#undef EXTERN
#ifdef _enemy_c
#define EXTERN
#else
#define EXTERN extern
#endif

#undef EXTERN
#ifdef _graphics_c
#define EXTERN
#else
#define EXTERN extern
#endif
EXTERN int Number_Of_Bullet_Types;
EXTERN SDL_Surface *Screen;
EXTERN SDL_Surface *StoredMenuBackground[2];
#ifdef HAVE_LIBGL
EXTERN GLuint StoredMenuBackgroundTex[2];
#endif
EXTERN int current_mouse_cursor_shape;

//--------------------
// Now the iso-image pointers for the new individually shaped
// isometric enemy images
//
// EXTERN SDL_Surface *EnemyRotationSurfacePointer[ ENEMY_ROTATION_MODELS_AVAILABLE ] [ ROTATION_ANGLES_PER_ROTATION_MODEL ];
EXTERN iso_image enemy_iso_images[ENEMY_ROTATION_MODELS_AVAILABLE][ROTATION_ANGLES_PER_ROTATION_MODEL][MAX_ENEMY_MOVEMENT_PHASES];
EXTERN iso_image
    BlueEnemyRotationSurfacePointer[ENEMY_ROTATION_MODELS_AVAILABLE][ROTATION_ANGLES_PER_ROTATION_MODEL][MAX_ENEMY_MOVEMENT_PHASES];
EXTERN iso_image
    RedEnemyRotationSurfacePointer[ENEMY_ROTATION_MODELS_AVAILABLE][ROTATION_ANGLES_PER_ROTATION_MODEL][MAX_ENEMY_MOVEMENT_PHASES];
EXTERN iso_image
    GreenEnemyRotationSurfacePointer[ENEMY_ROTATION_MODELS_AVAILABLE][ROTATION_ANGLES_PER_ROTATION_MODEL][MAX_ENEMY_MOVEMENT_PHASES];
EXTERN iso_image chat_portrait_of_droid[ENEMY_ROTATION_MODELS_AVAILABLE];

// EXTERN int phases_in_enemy_animation [ ENEMY_ROTATION_MODELS_AVAILABLE ];
EXTERN int first_walk_animation_image[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int last_walk_animation_image[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int first_attack_animation_image[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int last_attack_animation_image[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int first_gethit_animation_image[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int last_gethit_animation_image[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int first_death_animation_image[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int last_death_animation_image[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int first_stand_animation_image[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int last_stand_animation_image[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int use_default_attack_image[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int use_default_gethit_image[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int use_default_death_image[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int use_default_stand_image[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int droid_walk_animation_speed_factor[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int droid_attack_animation_speed_factor[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int droid_gethit_animation_speed_factor[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int droid_death_animation_speed_factor[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int droid_stand_animation_speed_factor[ENEMY_ROTATION_MODELS_AVAILABLE];

EXTERN iso_image floor_iso_images[ALL_ISOMETRIC_FLOOR_TILES];

EXTERN float iso_floor_tile_width;
EXTERN float iso_floor_tile_height;
EXTERN float iso_floor_tile_width_over_two;
EXTERN float iso_floor_tile_height_over_two;
EXTERN int vid_bpp;		// bits per pixel 

#undef EXTERN
#ifdef _blocks_c
#define EXTERN
#else
#define EXTERN extern
#endif
EXTERN obstacle_spec obstacle_map[NUMBER_OF_OBSTACLE_TYPES];

#undef EXTERN
#ifdef _text_c
#define EXTERN
#else
#define EXTERN extern
#endif
EXTERN int display_char_disabled;
EXTERN int chat_protocol_scroll_override_from_user;
EXTERN int game_message_protocol_scroll_override_from_user;
EXTERN SDL_Rect Droid_Image_Window;

#undef EXTERN
#ifdef _text_public_c
#define EXTERN
#else
#define EXTERN extern
#endif

#undef EXTERN
#ifdef _leveleditor_c
#define EXTERN
#else
#define EXTERN extern
#endif

#undef EXTERN
#ifdef _input_c
#define EXTERN
#else
#define EXTERN extern
#endif
EXTERN point input_axis;	/* joystick (and mouse) axis values */
EXTERN int always_show_items_text;

#undef EXTERN
#ifdef _takeover_c
#define EXTERN
#else
#define EXTERN extern
#endif

#undef EXTERN
#ifdef _skills_c
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN SDL_Rect SkillScreenRect;

#undef EXTERN
#ifdef _items_c
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN int Number_Of_Item_Types;
EXTERN itemspec *ItemMap;
EXTERN short int item_count_per_class[10];
EXTERN item_bonus *PrefixList;
EXTERN item_bonus *SuffixList;

EXTERN int game_root_mode;
enum {
	ROOT_IS_UNKNOWN = 0,
	ROOT_IS_GAME,
	ROOT_IS_LVLEDIT,
};

#undef EXTERN
#ifdef _saveloadgame_c
#define EXTERN
#else
#define EXTERN extern
#endif
EXTERN struct auto_string *savestruct_autostr;

#endif				// _global_h
