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

//===================================================================
#define INTERN_FOR _main_c
#include "extint_macros.h"

EXTERN float FPSover1;
EXTERN char *AllSkillTexts[];
EXTERN char font_switchto_red[];
EXTERN char font_switchto_blue[];
EXTERN char font_switchto_neon[];
EXTERN char font_switchto_msgstat[];
EXTERN char font_switchto_msgvar[];

EXTERN float MeleeDamageMultiplierTable[];
EXTERN float MeleeRechargeMultiplierTable[];
EXTERN float RangedDamageMultiplierTable[];
EXTERN float RangedRechargeMultiplierTable[];
EXTERN spell_skill_spec *SpellSkillMap;
EXTERN tux_t Me;		/* the influence data */
EXTERN droidspec *Droidmap;
EXTERN blastspec Blastmap[ALLBLASTTYPES];
EXTERN int skip_initial_menus;
EXTERN int number_of_skills;
EXTERN int last_bot_number;
EXTERN enum _game_status game_status;

EXTERN SDL_Rect User_Rect;
EXTERN SDL_Rect Full_User_Rect;
EXTERN SDL_Rect Cons_Text_Rect;

extern int term_has_color_cap;
extern int run_from_term;

EXTERN int Number_Of_Droid_Types;
EXTERN int QuitProgram;
EXTERN int GameOver;
EXTERN int quest_browser_activated;

extern list_head_t alive_bots_head;
extern list_head_t dead_bots_head;
extern list_head_t level_bots_head[MAX_LEVELS];

EXTERN spell_active AllActiveSpells[MAX_ACTIVE_SPELLS];
EXTERN ship curShip;		/* the current ship-data */

EXTERN bullet_sparsedynarray all_bullets INITIALIZER(SPARSE_DYNARRAY);
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

//===================================================================
#define INTERN_FOR _misc_c
#include "extint_macros.h"

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
EXTERN float Overall_Average;
EXTERN int SkipAFewFrames;
EXTERN struct data_dir data_dirs[];

//===================================================================
#define INTERN_FOR _view_c
#include "extint_macros.h"

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

//===================================================================
#define INTERN_FOR _light_c
#include "extint_macros.h"

EXTERN light_radius_config LightRadiusConfig;

//===================================================================
#define INTERN_FOR _graphics_c
#include "extint_macros.h"

EXTERN SDL_Surface *Screen;
EXTERN SDL_Surface *StoredMenuBackground[2];
EXTERN int mouse_cursor;
#ifdef HAVE_LIBGL
EXTERN GLuint StoredMenuBackgroundTex[2];
#endif

EXTERN struct tux_motion_class_images *tux_images;
EXTERN struct dynarray obstacle_images;

EXTERN float iso_floor_tile_width;
EXTERN float iso_floor_tile_height;
EXTERN float iso_floor_tile_width_over_two;
EXTERN float iso_floor_tile_height_over_two;
EXTERN int vid_bpp;		// bits per pixel 

//===================================================================
#define INTERN_FOR _hud_c
#include "extint_macros.h"

EXTERN struct widget_text *message_log;

//===================================================================
#define INTERN_FOR _blocks_c
#include "extint_macros.h"

EXTERN struct dynarray obstacle_map;
EXTERN struct dynarray underlay_floor_tiles;
EXTERN struct dynarray overlay_floor_tiles;

//===================================================================
#define INTERN_FOR _text_c
#include "extint_macros.h"

EXTERN SDL_Rect Droid_Image_Window;

//===================================================================
#define INTERN_FOR _input_c
#include "extint_macros.h"

EXTERN point input_axis;	/* joystick (and mouse) axis values */

//===================================================================
#define INTERN_FOR _skills_c
#include "extint_macros.h"

EXTERN SDL_Rect SkillScreenRect;

//===================================================================
#define INTERN_FOR _items_c
#include "extint_macros.h"

EXTERN int Number_Of_Item_Types;
EXTERN itemspec *ItemMap;
EXTERN short int item_count_per_class[MAX_DROP_CLASS + 1];

EXTERN int game_root_mode;
enum {
	ROOT_IS_UNKNOWN = 0,
	ROOT_IS_GAME,
	ROOT_IS_LVLEDIT,
};

//===================================================================
#define INTERN_FOR _obstacle_c
#include "extint_macros.h"

EXTERN struct dynarray obstacle_groups;

//===================================================================
#define INTERN_FOR _saveloadgame_c
#include "extint_macros.h"

EXTERN struct auto_string *savestruct_autostr;
EXTERN jmp_buf saveload_jmpbuf;

//===================================================================
#define INTERN_FOR _benchmark_c
#include "extint_macros.h"

EXTERN char *do_benchmark;

//===================================================================
#define INTERN_FOR _npc_c
#include "extint_macros.h"

EXTERN list_head_t npc_head;

//===================================================================
#define INTERN_FOR _game_ui_c
#include "extint_macros.h"

EXTERN struct widget_button *game_map;

//===================================================================
#define INTERN_FOR _influ_c
#include "extint_macros.h"

EXTERN struct {
	int standing_keyframe;
	struct timebased_animation attack;
	struct distancebased_animation walk;
	struct distancebased_animation run;
} tux_anim;

#define IMAGE_SCALE_RGB_TRANSFO(SCALE, R, G, B) set_image_transformation(SCALE, SCALE, R, G, B, 1.0, 0)
#define IMAGE_SCALE_TRANSFO(SCALE) IMAGE_SCALE_RGB_TRANSFO(SCALE, 1.0, 1.0, 1.0)
#define IMAGE_NO_TRANSFO IMAGE_SCALE_TRANSFO(1.0)

//===================================================================
#define INTERN_FOR _bullet_c
#include "extint_macros.h"

EXTERN struct dynarray bullet_specs;

//===================================================================
#define INTERN_FOR _lang_c
#include "extint_macros.h"

EXTERN struct dynarray lang_specs;
EXTERN struct dynarray lang_codesets;

//===================================================================
#define INTERN_FOR _init_c
#include "extint_macros.h"

EXTERN struct dynarray difficulties;

//===================================================================
#define INTERN_FOR _font_c
#include "extint_macros.h"

EXTERN struct font *Menu_Font;
EXTERN struct font *Messagevar_Font;
EXTERN struct font *Messagestat_Font;
EXTERN struct font *Para_Font;
EXTERN struct font *FPS_Display_Font;
EXTERN struct font *Blue_Font;
EXTERN struct font *Red_Font;
EXTERN struct font *Messagered_Font;

//===================================================================
#define INTERN_FOR _colldet_c
#include "extint_macros.h"

EXTERN colldet_filter WalkablePassFilter;
EXTERN colldet_filter WalkableWithMarginPassFilter;
EXTERN colldet_filter FlyablePassFilter;
EXTERN colldet_filter VisiblePassFilter;
EXTERN colldet_filter ObstacleByIdPassFilter;
EXTERN colldet_filter WalkableExceptIdPassFilter;
EXTERN colldet_filter FlyableExceptIdPassFilter;

//===================================================================
// Final include to undef all macros
#include "extint_macros.h"

#endif				// _global_h
