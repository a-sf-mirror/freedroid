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

extern char *freedroid_version;

#undef EXTERN
#ifdef _main_c
#define EXTERN
#else
#define EXTERN extern

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
EXTERN droidspec *Droidmap;
EXTERN bulletspec *Bulletmap;
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
EXTERN int quest_browser_activated;

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
EXTERN int show_all_droids;	// display enemies regardless of IsVisible() 
EXTERN int draw_collision_rectangles;	// to better debug collision rectangles
EXTERN int start_editor;
EXTERN int load_saved;
EXTERN char *saved_game_name;

EXTERN FPSmanager SDL_FPSmanager;

#undef EXTERN
#ifdef _misc_c
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN float timeout_from_item_drop;
EXTERN int use_open_gl;
#ifdef DEBUG_QUAD_BORDER
EXTERN unsigned int debug_quad_border_seed;
#endif

EXTERN mouse_press_button AllMousePressButtons[MAX_MOUSE_PRESS_BUTTONS];
EXTERN item *item_held_in_hand;

EXTERN struct image MouseCursorImageList[NUMBER_OF_MOUSE_CURSOR_PICTURES];

EXTERN int Number_Of_Droids_On_Ship;
EXTERN configuration_for_freedroid GameConfig;
EXTERN BFont_Info *Menu_BFont;
EXTERN BFont_Info *Messagevar_BFont;
EXTERN BFont_Info *Messagestat_BFont;
EXTERN BFont_Info *Para_BFont;
EXTERN BFont_Info *FPS_Display_BFont;
EXTERN BFont_Info *Blue_BFont;
EXTERN BFont_Info *Red_BFont;
EXTERN float Overall_Average;
EXTERN int SkipAFewFrames;

#undef EXTERN
#ifdef _view_c
#define EXTERN
#else
#define EXTERN extern
#endif
EXTERN struct image light_radius_chunk[NUMBER_OF_SHADOW_IMAGES];
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
#define NEIGHBOR_ID(lvl, x, y) (level_neighbors_map[lvl][y][x] ? level_neighbors_map[lvl][y][x]->lvl_idx : -1)
#define NEIGHBOR_ID_NW(lvl) NEIGHBOR_ID(lvl, 0, 0)
#define NEIGHBOR_ID_N(lvl)  NEIGHBOR_ID(lvl, 1, 0)
#define NEIGHBOR_ID_NE(lvl) NEIGHBOR_ID(lvl, 2, 0)
#define NEIGHBOR_ID_W(lvl)  NEIGHBOR_ID(lvl, 0, 1)
#define NEIGHBOR_ID_E(lvl)  NEIGHBOR_ID(lvl, 2, 1)
#define NEIGHBOR_ID_SW(lvl) NEIGHBOR_ID(lvl, 0, 2)
#define NEIGHBOR_ID_S(lvl)  NEIGHBOR_ID(lvl, 1, 2)
#define NEIGHBOR_ID_SE(lvl) NEIGHBOR_ID(lvl, 2, 2)

EXTERN struct tux_rendering tux_rendering;
EXTERN int clickable_obstacle_under_cursor;

#undef EXTERN
#ifdef _light_c
#define EXTERN
#else
#define EXTERN extern
#endif
EXTERN light_radius_config LightRadiusConfig;

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
EXTERN int mouse_cursor;
#ifdef HAVE_LIBGL
EXTERN GLuint StoredMenuBackgroundTex[2];
#endif

//--------------------
// Now the iso-image pointers for the new individually shaped
// isometric enemy images
//
EXTERN struct image enemy_images[ENEMY_ROTATION_MODELS_AVAILABLE][ROTATION_ANGLES_PER_ROTATION_MODEL][MAX_ENEMY_MOVEMENT_PHASES];
EXTERN struct image chat_portrait_of_droid[ENEMY_ROTATION_MODELS_AVAILABLE];

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
EXTERN int droid_walk_animation_speed_factor[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int droid_attack_animation_speed_factor[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int droid_gethit_animation_speed_factor[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int droid_death_animation_speed_factor[ENEMY_ROTATION_MODELS_AVAILABLE];
EXTERN int droid_stand_animation_speed_factor[ENEMY_ROTATION_MODELS_AVAILABLE];

EXTERN struct tux_motion_class_images *tux_images;
EXTERN struct dynarray obstacle_images;

EXTERN float iso_floor_tile_width;
EXTERN float iso_floor_tile_height;
EXTERN float iso_floor_tile_width_over_two;
EXTERN float iso_floor_tile_height_over_two;
EXTERN int vid_bpp;		// bits per pixel 

#undef EXTERN
#ifdef _hud_c
#define EXTERN
#else
#define EXTERN extern
#endif
EXTERN struct widget_text *message_log;

#undef EXTERN
#ifdef _blocks_c
#define EXTERN
#else
#define EXTERN extern
#endif
EXTERN struct dynarray obstacle_map;
EXTERN struct dynarray underlay_floor_tiles;
EXTERN struct dynarray overlay_floor_tiles;
EXTERN char *PrefixToFilename[ENEMY_ROTATION_MODELS_AVAILABLE];

#undef EXTERN
#ifdef _text_c
#define EXTERN
#else
#define EXTERN extern
#endif
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

EXTERN int game_root_mode;
enum {
	ROOT_IS_UNKNOWN = 0,
	ROOT_IS_GAME,
	ROOT_IS_LVLEDIT,
};

#undef EXTERN
#ifdef _obstacle_c
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN struct dynarray obstacle_groups;

#undef EXTERN
#ifdef _saveloadgame_c
#define EXTERN
#else
#define EXTERN extern
#endif
EXTERN struct auto_string *savestruct_autostr;
EXTERN jmp_buf saveload_jmpbuf;

#undef EXTERN
#ifdef _benchmark_c
#define EXTERN
#else
#define EXTERN extern
#endif
EXTERN char *do_benchmark;

#undef EXTERN
#ifdef _npc_c
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN list_head_t npc_head;

#undef EXTERN
#ifdef _game_ui_c
#define EXTERN
#else
#define EXTERN extern
#endif
EXTERN struct widget_button *game_map;

#undef EXTERN
#ifdef _influ_c
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN struct {
	int standing_keyframe;
	struct timebased_animation attack;
	struct distancebased_animation walk;
	struct distancebased_animation run;
} tux_anim;

#define IMAGE_SCALE_RGB_TRANSFO(SCALE, R, G, B) set_image_transformation(SCALE, SCALE, R, G, B, 1.0, 0)
#define IMAGE_SCALE_TRANSFO(SCALE) IMAGE_SCALE_RGB_TRANSFO(SCALE, 1.0, 1.0, 1.0)
#define IMAGE_NO_TRANSFO IMAGE_SCALE_TRANSFO(1.0)

#endif				// _global_h

