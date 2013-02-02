/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2010 Arthur Huillet 
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

#ifndef _struct_h
#define _struct_h

#include "system.h"
#include "defs.h"
#include "lua.h"

typedef struct {
	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;
} myColor;

/**
 * Simple doubly linked list implementation.
 */
typedef struct list_head {
	struct list_head *next;
	struct list_head *prev;
} list_head_t;

/**
 * Dynamic arrays
 */
struct dynarray {
	void *arr;
	int size;
	int capacity;
};

typedef struct dynarray item_dynarray;

typedef struct upgrade_socket_dynarray {
	struct upgrade_socket *arr;
	int size;
	int capacity;
} upgrade_socket_dynarray;

/* prototype for BFont_Info from BFont.h */
struct BFont_Info;

enum image_transformation_mode {
	HIGHLIGHTED = 1 << 1,
	REPEATED    = 1 << 2
};

struct image_transformation {
	SDL_Surface *surface;
	float scale_x; /**< zoom factor or repeat factor, depending on transformation mode */
	float scale_y; /**< zoom factor or repeat factor, depending on transformation mode */
	float c[4];    /**< color transformation, r g b a in that order */
	enum image_transformation_mode mode;
};

/**
 * This structure defines an image in FreedroidRPG. It contains information
 * that enables rendering using SDL or OpenGL.
 */
struct image {
	SDL_Surface *surface;
	short offset_x;
	short offset_y;
	short w;
	short h;
	int texture_has_been_created;
#ifdef HAVE_LIBGL
	GLuint texture;
#else
	int texture;
#endif
	float tex_x0;
	float tex_x1;
	float tex_y0;
	float tex_y1;
	short tex_w;
	short tex_h;

	struct image_transformation cached_transformation;
};
#define EMPTY_IMAGE { .surface = NULL , .offset_x = 0 , .offset_y = 0 , .texture_has_been_created = 0 , .cached_transformation = { NULL, 0.0, 0.0, { 0.0, 0.0, 0.0, 0.0}, 0 } }

typedef struct mouse_press_button {
	struct image button_image;
	char *button_image_file_name;
	SDL_Rect button_rect;
	char scale_this_button;
} mouse_press_button, *Mouse_press_button;

typedef char *string;

typedef struct keybind {
	string name; /**< keybinding name, taken from keybindNames */
	int key;      /**< key/axis/button event number */
	int mod;      /**< Key modifiers */
} keybind_t;

typedef struct configuration_for_freedroid {
	float WantedTextVisibleTime;
	int Draw_Framerate;
	int Draw_Position;
	int All_Texts_Switch;
	float Current_BG_Music_Volume;
	float Current_Sound_FX_Volume;
	int Current_Sound_Output_Fmt;
	float current_gamma_correction;

	int Inventory_Visible;
	int CharacterScreen_Visible;
	int SkillScreen_Visible;
	int Automap_Visible;
	int spell_level_visible;

	int autorun_activated;

	string freedroid_version_string;
	int skip_light_radius;
	int skill_explanation_screen_visible;
	int enemy_energy_bars_visible;
	int limit_framerate;
	int omit_obstacles_in_level_editor;
	int omit_map_labels_in_level_editor;
	int omit_enemies_in_level_editor;
	int zoom_is_on;
	int show_blood;		// this can be used to make the game more friendly for children...
	int show_lvledit_tooltips;
	int show_grid;
	int show_wp_connections;
	int grid_mode;
	int number_of_big_screen_messages;
	float delay_for_big_screen_messages;
	int enable_cheatkeys;
	int transparency;
	int screen_width;
	int screen_height;
	int next_time_width_of_screen;
	int next_time_height_of_screen;
	int skip_shadow_blitting;
	int do_fadings;		// do the screen fadings
	int fullscreen_on;
	int talk_to_bots_after_takeover;
	int xray_vision_for_tux;
	int cheat_running_stamina;
	int cheat_double_speed;
	int lazyload;
	int show_item_labels;
	int last_edited_level;
	int show_all_floor_layers;

	int difficulty_level;

	// This must be the last element of the structure, because the
	// input keybind parsing code uses strtok which messes with the
	// string.
	keybind_t input_keybinds[100];
} configuration_for_freedroid;

typedef struct point {
	int x;
	int y;
} point;

typedef struct moderately_finepoint {
	float x;
	float y;
} moderately_finepoint;

typedef struct finepoint {
	double x;
	double y;
} finepoint;

typedef struct gps {
	float x;
	float y;
	int z;
} gps;

typedef struct location {
	unsigned char level;
	int x;
	int y;
} location;

typedef struct map_label {
	char *label_name;
	point pos;		// how many blocks does this big map insert cover?
} map_label;

typedef char *luacode;
typedef struct mission {
	string mission_name;
	int MissionWasAssigned;	// has be influencer been assigned to this mission? , currently uninitialized
	int MissionIsComplete;	// has the mission been completed?
	int MissionWasFailed;	// has the mission been failed?
	int MissionExistsAtAll;	// is this mission entry used at all, or is it just unused memory?

	int KillOne;
	int must_clear_first_level;
	int must_clear_second_level;

	luacode completion_lua_code;
	luacode assignment_lua_code;

	string mission_diary_texts[MAX_MISSION_DESCRIPTION_TEXTS];
	float mission_description_time[MAX_MISSION_DESCRIPTION_TEXTS];
	int expanded_display_for_this_mission;
} mission;

struct addon_bonus {
	char *name;
	int value;
};

struct addon_material {
	char *name;
	int value;
};

struct addon_spec {
	int type;
	int upgrade_cost;
	char *requires_socket;
	char *requires_item;
	struct dynarray bonuses;
	struct dynarray materials;
};

typedef struct itemspec {
	char *item_name;
	char *item_rotation_series_prefix;
	char *item_description;
	char *item_drop_sound_file_name;
	char *item_inv_file_name;

	char *item_combat_use_description;
	
	enum slot_type slot;
	
	char *tux_part_instance;

	char item_group_together_in_inventory;

	// How good is the item as weapon???
	float item_gun_recharging_time;	// time until the next shot can be made, measures in seconds
	float item_gun_reloading_time;	// time needed to put a new charger
	short item_gun_bullet_image_type;	// which type of image to use for displaying this bullet
	float item_gun_speed;	// how fast should a bullet move straightforward?
	short base_item_gun_damage;	//   damage done by this bullettype 
	short item_gun_damage_modifier;	// modifier to the damage done by this bullettype 
	float item_gun_bullet_lifetime;	// how long does a 'bullet' from this gun type live?
	char item_gun_bullet_pass_through_hit_bodies;	// does this bullet go through hit bodies (e.g. like a laser sword)
	short item_gun_ammo_clip_size;	//max. number of bullets in the charger

	// the following values have only relevance in case of a melee weapon
	short int item_weapon_is_melee;
	float item_gun_start_angle_modifier;	// where to start with a melee weapon swing
	int item_gun_use_ammunition;        // which ammunition does this gun use? see ammo_desc_for_weapon()
	char item_gun_requires_both_hands;	// is this a (strictly) 2-handed weapon?
	short int motion_class;				// Tux's motion class to use

	// how good is the item as armour or shield or other protection???
	short base_armor_class;
	short armor_class_modifier;

	// which requirement for strength, dexterity and cooling does the item have?
	short int item_require_strength;
	short int item_require_dexterity;
	short int item_require_cooling;

	// what durability does the item have?
	short int base_item_durability;
	short int item_durability_modifier;

	point inv_size;
	struct image inventory_image;
	struct image ingame_image;
	struct image shop_image;

	short int base_list_price;	// the base price of this item at the shop

	short int min_drop_class;
	short int max_drop_class;
	int drop_amount;		// minimum number of items to drop at once
	int drop_amount_max;	// maximum of items to drop at once

} itemspec;

typedef struct upgrade_socket {
	int type;
	string addon;
} upgrade_socket;

typedef struct item {
	// Here are the rather short-lived properties of the item
	gps pos;
	gps virt_pos;
	SDL_Rect text_slot_rectangle;
	int type;
	int max_durability;	// the maximum item durability reachable for this item
	float current_durability;	// the currently remaining durability for this item
	float throw_time;	// has this item just jumped out from a chest maybe or is it jumping right now?

	int bonus_to_dex;
	int bonus_to_str;
	int bonus_to_physique;
	int bonus_to_cooling;
	int bonus_to_health_points;
	float bonus_to_health_recovery;
	float bonus_to_cooling_rate;
	int bonus_to_attack;
	int bonus_to_all_attributes;
	int bonus_to_armor_class;
	int bonus_to_damage;
	int bonus_to_paralyze_enemy;
	int bonus_to_slow_enemy;
	int bonus_to_light_radius;
	int bonus_to_experience_gain;	// this is a percentage

	int armor_class;
	int damage;		// how much damage does this item
	int damage_modifier;	// how much additional damage can add to the base damage
	int multiplicity;
	int ammo_clip;		// how much bullets in the clip, IN CASE OF WEAPON
	point inventory_position;

	struct upgrade_socket_dynarray upgrade_sockets;
	int quality;
} item;

typedef struct droidspec {
	char *droidname;
	char *default_short_description;
	char *droid_portrait_rotation_series_prefix;
	char *droid_death_sound_file_name;
	char *droid_attack_animation_sound_file_name;
	int class;
	float maxenergy;	// the maximum energy the batteries can carry 

	float healing_friendly; // the energy/second the droid heals as a friendly towards Tux
	float healing_hostile;  // the energy/second the droid heals as a hostile towards Tux

	float maxspeed;

	short experience_reward;	// experience_reward for the elimination of one droid of this type 

	float aggression_distance;
	float time_spent_eyeing_tux;

	short int drop_class;

	item weapon_item;
	int gun_muzzle_height;

	short int amount_of_plasma_transistors;
	short int amount_of_superconductors;
	short int amount_of_antimatter_converters;
	short int amount_of_entropy_inverters;
	short int amount_of_tachyon_condensators;

	short greeting_sound_type;	// which sample to play in order to 'greet' the influencer?
	short to_hit;		// chance that this droid hits an unarmoured target
	float recover_time_after_getting_hit;
	char *notes;		// notes on the droid of this type 
	short int is_human;
	short individual_shape_nr;
} droidspec;

typedef char s_char; // Used for pointer to static string which are not to be saved

typedef struct enemy {
	// There are three sets of attributes, which are initialized and
	// possibly re-initialized by 3 different codes:
	//
	// 1) 'identity' attributes.
	//    The identity attributes define the basic information about a droid.
	//    They should not change during the game, apart in some very
	//    specific cases.
	// 2) global state' attributes.
	//    Those attributes define the global behavior of the enemy.
	//    The global state of a droid can possibly change during the game.
	// 3) 'transient state' attributes
	//    Their values are used by the animation and AI code.
	//    They change very frequently during the game.
	//
	// 1st and 2nd sets are initialized to default values in enemy_new().
	// The 3rd set is unconditionally reseted in enemy_reset().
	// The 2nd set contains attributes whose values depend on the
	// type of the droid. They are first positioned in GetThisLevelsDroids()
	// and GetThisLevelsSpecialForces(), and are conditionally reseted in
	// respawn_level().

	//--------------------
	// 1st set (identity)
	//
	short int id;                       // unique id of the droid. start at 1.
	short int type;                     // the number of the droid specifications in Droidmap
	uint8_t SpecialForce;               // this flag will exclude the droid from initial shuffling of droids
	int marker;                         // this provides a marker for special mission targets
	short int max_distance_to_home;     // how far this robot will go before returning to it's home waypoint
	string dialog_section_name;
	string short_description_text;
	short int on_death_drop_item_code;  // drop a pre-determined item when dying

	//--------------------
	// 2nd set ('global state')
	//
	int faction;
	uint8_t will_rush_tux;          // will this robot approach the Tux on sight and open communication?
	int combat_state;               // current state of the bot
	float state_timeout;            // time spent in this state (used by "timeout" states such as STOP_AND_EYE_TARGET only)
	short int CompletelyFixed;      // set this flag to make the robot entirely immobile
	uint8_t has_been_taken_over;    // has the Tux made this a friendly bot via takeover subgame?
	uint8_t follow_tux;             // does this robot try to follow tux via it's random movements?
	uint8_t has_greeted_influencer; // has this robot issued his first-time-see-the-Tux message?
	short int nextwaypoint;         // the next waypoint target
	short int lastwaypoint;         // the waypoint from whence this robot just came from
	short int homewaypoint;         // the waypoint this robot started at
	gps pos;                        // coordinates of the current position in the level

	//--------------------
	// 3rd set ('transient state')
	//
	finepoint speed;                   // current speed
	float energy;                      // current energy of this droid
	float animation_phase;             // the current animation frame for this enemy (starting at 0 of course...)
	short int animation_type;          // walk-animation, attack-animation, gethit animation, death animation
	float frozen;                      // is this droid currently frozen and for how long will it stay this way?
	float poison_duration_left;        // is this droid currently poisoned and for how long will it stay this way?
	float poison_damage_per_sec;       // is this droid currently poisoned and how much poison is at work?
	float paralysation_duration_left;  // is this droid currently paralyzed and for how long will it stay this way?
	float pure_wait;                   // time till the droid will start to move again
	float firewait;                    // time this robot still takes until its weapon will be fully reloaded
	short int ammo_left;               // ammunition left in the charger
	uint8_t attack_target_type;        // attack NOTHING, PLAYER, or BOT
	short int bot_target_n;
	struct enemy *bot_target_addr;
	float previous_angle;              // which angle has this robot been facing the frame before?
	float current_angle;               // which angle will the robot be facing now?
	float previous_phase;              // which (8-way) direction did the robot face before?
	float last_phase_change;           // when did the robot last change his (8-way-)direction of facing
	float last_combat_step;            // when did this robot last make a step to move in closer or farther away from Tux in combat?
	float TextVisibleTime;
	s_char *TextToBeDisplayed;         // WARNING!!! Only use static texts
	moderately_finepoint PrivatePathway[5];
	uint8_t bot_stuck_in_wall_at_previous_check;
	float time_since_previous_stuck_in_wall_check;

	//--------------------
	// Misc attributes
	//
	gps virt_pos;             // the virtual position (position of the bot if he was on this level, differs from above when it is on a neighboring level)
	list_head_t global_list;  // entry of this bot in the global bot lists (alive or dead)
	list_head_t level_list;   // entry of this bot in the level bot list (alive only)
} enemy, *Enemy;

typedef struct npc {
	string dialog_basename;
	uint8_t chat_character_initialized;
	uint8_t chat_flags[MAX_DIALOGUE_OPTIONS_IN_ROSTER];

	string shoplist[MAX_ITEMS_IN_INVENTORY]; //list of items that can be put in the inventory of the NPC
	int shoplistweight[MAX_ITEMS_IN_INVENTORY]; //weight of each item: relative probability of appearance in inventory
	item_dynarray npc_inventory;

	float last_trading_date; // when did we trade with this NPC the last time?

	list_head_t node; 
} npc;

typedef char automap_data_t[100][100];
typedef struct tux {
	float current_game_date;	// seconds since game start, will be printed as a different 'date'
	// inside the game, like 14:23 is afternoon
	int current_power_bonus;
	float power_bonus_end_date;
	int current_dexterity_bonus;
	float dexterity_bonus_end_date;
	float light_bonus_end_date;

	finepoint speed;	// the current speed of the droid 
	gps pos;		// current position in the whole ship 
	gps teleport_anchor;	// where from have you last teleported home
	gps mouse_move_target;	// where the tux is going automatically by virtue of mouse move

	short int current_enemy_target_n;	//which enemy has been targeted 
	uint8_t god_mode;
	enemy *current_enemy_target_addr;	// which enemy has been targeted, address

	int mouse_move_target_combo_action_type;	// what extra action has to be done upon arrival?
	int mouse_move_target_combo_action_parameter;	// extra data to use for the combo action

	int light_bonus_from_tux;
	int map_maker_is_present;

	float maxenergy;	// current top limit for the influencers energy
	float energy;		// current energy level 
	float max_temperature;	// current top limit for temperature (highest is better)
	float temperature;	// current temperature
	float old_temperature;	// current temperature
	float max_running_power;
	float running_power;
	int running_must_rest;
	int running_power_bonus;

	float health_recovery_rate;	//points of health recovered each second
	float cooling_rate;	//temperature points recovered each second

	double busy_time;	// time remaining, until the weapon is ready to fire again...
	int busy_type;		// reason why tux is busy (enum)
	double phase;		// the current phase of animation 
	float angle;
	float walk_cycle_phase;	// 
	float weapon_swing_time;	// How long is the current weapon swing in progress (in seconds of course) 
	float MissionTimeElapsed;
	float got_hit_time;	// how long stunned now since the last time tux got hit 

	string savegame_version_string;	// a string to identify games from older FreedroidRPG versions

	int base_cooling;
	int base_dexterity;
	int base_physique;
	int base_strength;
	int cooling;
	int dexterity;
	int physique;
	int strength;
	int points_to_distribute;	// these are the points that are available to distribute upon the character stats
	float base_damage;	// the current damage the influencer does
	float damage_modifier;	// the modifier to the damage the influencer currently does
	float armor_class;
	float to_hit;		// percentage chance, that Tux will hit a random lv 1 bot

	int slowing_melee_targets;	// duration for how long hit enemies are slowed down
	int paralyzing_melee_targets;	// duration for how long hit enemies are paralyzed
	float experience_factor; // multiplier for the experience gained from bots

	unsigned int Experience;	
	int exp_level;			

	unsigned int Gold;
	string character_name;
	mission AllMissions[MAX_MISSIONS_IN_GAME];	// What must be done to fullfill this mission?
	int marker;		// In case you've taken over a marked droid, this will contain the marker
	float LastCrysoundTime;
	float TextVisibleTime;
	s_char *TextToBeDisplayed;                    // WARNING!!! Only use static texts

	//--------------------
	// Here we note all the 'skill levels' of the Tux and also which skill is
	// currently readied and that...
	//
	int readied_skill;
	int skill_level[MAX_NUMBER_OF_PROGRAMS];
	int melee_weapon_skill;
	int ranged_weapon_skill;
	int spellcasting_skill;
	//--------------------
	// The inventory slots.  Some items are residing in general inventory,
	// other items might be equipped in some of the corresponding slots of
	// the inventory screen.
	//
	item Inventory[MAX_ITEMS_IN_INVENTORY];
	item weapon_item;
	item drive_item;
	item armour_item;
	item shield_item;
	item special_item;

	//--------------------
	// A record of when and if the tux has been on some maps...
	//
	uint8_t HaveBeenToLevel[MAX_LEVELS];	            // record of the levels the player has visited yet.
	float time_since_last_visit_or_respawn[MAX_LEVELS];	// record of the levels the player has visited yet.

	//--------------------
	// THE FOLLOWING ARE INFORMATION, THAT ARE HUGE AND THAT ALSO DO NOT NEED
	// TO BE COMMUNICATED FROM THE CLIENT TO THE SERVER OR VICE VERSA
	//
	moderately_finepoint next_intermediate_point[MAX_INTERMEDIATE_WAYPOINTS_FOR_TUX];	// waypoints for the tux, when target not directly reachable
	automap_data_t Automap[MAX_LEVELS];
	int current_zero_ring_index;
	gps Position_History_Ring_Buffer[MAX_INFLU_POSITION_HISTORY];

	int BigScreenMessageIndex;
	string BigScreenMessage[MAX_BIG_SCREEN_MESSAGES];
	float BigScreenMessageDuration[MAX_BIG_SCREEN_MESSAGES];

	float slowdown_duration;
	float paralyze_duration;
	float invisible_duration;
	float nmap_duration;

	int quest_browser_changed;

	int program_shortcuts[10];

	// STATISTICS ABOUT TUX
	// Should warn if less than Number_Of_Droid_Types + 2
#define NB_DROID_TYPES 50
	int TakeoverSuccesses[NB_DROID_TYPES]; // how many did Tux takeover and make friendly?
	int TakeoverFailures[NB_DROID_TYPES];  // how many did Tux fail at a takeover attempt?
	int destroyed_bots[NB_DROID_TYPES];    // how many bots have been destroyed?
	int damage_dealt[NB_DROID_TYPES];      // how much damage dealt?
	float meters_traveled;     // how many meters has Tux traveled?
} tux_t;

typedef struct bulletspec {
	char *name;		// what is the name of this bullet type
	char *sound;		// what sound to play
	int phases;		// how many phases in motion to show 
	double phase_changes_per_second;	// how many different phases to display every second
	int blast_type;
	struct image image[BULLET_DIRECTIONS][MAX_PHASES_IN_A_BULLET];
} bulletspec;

typedef struct bullet {
	short int type;
	int phase;
	uint8_t mine;
	gps pos;
	int height;
	moderately_finepoint speed;
	short int damage;	// damage done by this particular bullet
	float time_in_seconds;	// how long does the bullet exist in seconds
	float bullet_lifetime;	// how long can this bullet exist at most
	float time_to_hide_still;	// countdown to when the bullet will actually appear
	short int owner;
	float angle;

	uint8_t pass_through_hit_bodies; // does this bullet go through hit bodies (e.g. like a laser sword strike)
	short int freezing_level;	// does this bullet freeze the target?
	float poison_duration;
	float poison_damage_per_sec;
	float paralysation_duration;

	int faction;
	uint8_t hit_type;		//hit bots, humans, both?
} bullet, *Bullet;

typedef struct melee_shot	// this is a melee shot
{
	uint8_t attack_target_type;	//type of attack
	uint8_t mine;		//is it mine?
	short int bot_target_n;	//which enemy has been targeted 
	enemy *bot_target_addr;	// which enemy has been targeted, address
	short int to_hit;	//chance to hit, percent
	short int damage;
	short int owner;
	float time_to_hit;	//Time until the 'shot' makes contact
} melee_shot;

typedef struct blastspec {
	int phases;
	float total_animation_time;
	int do_damage;
	struct image *images;
	char *name;
	string sound_file;
} blastspec;

typedef struct blast {
	gps pos;
	int type;
	float phase;
	float damage_per_second;
	int faction;
} blast;

typedef struct spell_active {
	int img_type;		// what type of spell is active?
	int damage;
	int poison_duration;
	int poison_dmg;
	int freeze_duration;
	int paralyze_duration;
	moderately_finepoint spell_center;
	float spell_radius;
	float spell_age;
	uint8_t active_directions[RADIAL_SPELL_DIRECTIONS];
	int mine;
	uint8_t hit_type;
} spell_active;

typedef struct spell_skill_spec {
	char *name;
	char *icon_name;
	struct image icon_surface;
	short heat_cost;
	short heat_cost_per_level;
	short damage_base;
	short damage_mod;
	short damage_per_level;
	short hurt_bots;
	short hurt_humans;
	short form;
	short present_at_startup;
	char *description;
	char *effect;
	float effect_duration;
	float effect_duration_per_level;
	int graphics_code;
} spell_skill_spec;

typedef struct waypoint {
	int x;
	int y;
	int suppress_random_spawn;
	struct dynarray connections;
} waypoint;

typedef struct obstacle {
	int type;
	gps pos;
	int timestamp;
	int frame_index;
} obstacle;

typedef struct map_tile {
	Uint16 floor_values[MAX_FLOOR_LAYERS];
	struct dynarray glued_obstacles;
} map_tile;

struct obstacle_extension {
	obstacle *obs;
	enum obstacle_extension_type type;
	void *data;
}; /** This contains "extension data" for obstacles - labels, item lists, ... */

typedef struct level {
	int levelnum;
	int xlen;
	int ylen;
	int light_bonus;
	int minimum_light_value;
	int infinite_running_on_this_level;
	int random_dungeon;
	int dungeon_generated;
	char *Levelname;
	char *Background_Song_Name;

	int floor_layers;
	map_tile *map[MAX_MAP_LINES];	// this is a vector of pointers
	int jump_target_north;
	int jump_target_south;
	int jump_target_east;
	int jump_target_west;

	obstacle obstacle_list[MAX_OBSTACLES_ON_MAP];
	item ItemList[MAX_ITEMS_PER_LEVEL];

	struct dynarray obstacle_extensions;
	struct dynarray map_labels;
	struct dynarray waypoints;
	struct {
		int nr;
		int types[10];
		int types_size;
	} random_droids;

	int teleport_pair;
	int flags;
} level, *Level;

typedef void (*action_fptr) (level *obst_lvl, int obstacle_idx);
typedef int (*animation_fptr) (level *obst_lvl, void *);

struct obstacle_graphics {
	int count;
	struct image *images;
	struct image *shadows;
};

typedef struct obstacle_spec {
	char *label; 

	//--------------------
	// Some obstacles will block the Tux from walking through them.
	// Currently only rectangles are supported block areas.  The width
	// (i.e. east-west=parm1) and height (i.e. north-south=parm2) of
	// the blocking rectangle can ge specified below.
	//
	float block_area_parm_1;
	float block_area_parm_2;
	float left_border;
	float right_border;
	float upper_border;
	float lower_border;

	float diaglength;

	char block_area_type;
	int result_type_after_smashing_once;

	unsigned int flags;

	//--------------------
	// Some obstacles will emit light.  Specify light strength here.
	// A value of 0 light will be sufficient in most cases...
	//
	struct dynarray emitted_light_strength;
	char transparent;

	struct dynarray filenames;
	action_fptr action_fn;

	//--------------------
	// Some obstacles have an associated animation.
	// This property defines the function to call to animate them
	animation_fptr animation_fn;
	float animation_fps;

	//-----------------------
	// Some obstacles have a different sounds / blast animations
	// these properties allow us to define a specific sound or blast for
	// a given obstacle
	unsigned int blast_type;
	char *smashed_sound;
} obstacle_spec;

struct obstacle_group {
	const char *name;
	struct dynarray members;
};

struct floor_tile_spec {
	int frames;	// More than 1 for animated floor tiles
	struct dynarray filenames;
	struct image *images;
	struct image *current_image;

	// Properties for animated floor tiles
	animation_fptr animation_fn;
	float animation_fps;
};

struct visible_level {
	int valid;
	level *lvl_pointer;
	float boundary_squared_dist;
	struct list_head animated_obstacles_list;
	int animated_obstacles_dirty_flag;
	struct list_head node;
};

typedef struct ship {
	int num_levels;
	level *AllLevels[MAX_LEVELS];
} ship;

typedef struct dialogue_option {
	char *topic;
	char *option_text;
	int no_text;
	luacode lua_code;
	int exists;
} dialogue_option;

typedef struct colldet_filter {
	int (*callback) (struct colldet_filter *filter, obstacle *obs, int obs_idx);
	void *data;
	float extra_margin;
	struct colldet_filter *next;
} colldet_filter;

typedef struct light_radius_config {
	uint32_t cells_w;
	uint32_t cells_h;
	uint32_t texture_w;
	uint32_t texture_h;
	int translate_y;
	float scale_factor;
} light_radius_config;

typedef struct screen_resolution {
	int xres;
	int yres;
	char *comment;
	int supported;
} screen_resolution;

/*
 * [way|location]_free_of_droids's execution context
 * 
 * Note : '2' excepted bots are needed when the pathfinder is called
 *         to let a bot reach an other one (used during attack, for example).
 */
typedef struct freeway_context {
	int check_tux;		// Check if Tux is along the way
	Enemy except_bots[2];	// Do not check if those bots are along the way (see note below)
} freeway_context;

/*
 * Pathfinder's execution context
 */
typedef struct pathfinder_context {
	colldet_filter *dlc_filter;	// DLC filter to use
	freeway_context *frw_ctx;	// [way|location]_free_of_droids's execution context to use
} pathfinder_context;

typedef struct {
	int shop_command;
	int item_selected;
	int number_selected;
} shop_decision;

struct auto_string {
	char *value;
	unsigned long length;
	unsigned long capacity;
};

/*
 * Specification of Animations
 */

struct timebased_animation {
	float duration;			// Duration of the animation
	int first_keyframe;
	int last_keyframe;
	int nb_keyframes;		// Set to (last - first). Avoid to compute it each time this information is needed
};

struct distancebased_animation {
	float distance;			// Distance covered during the whole animation
	int first_keyframe;
	int last_keyframe;
	int nb_keyframes;		// Set to (last - first). Avoid to compute it each time this information is needed
};

/**
 * Specification of Tux's parts rendering data
 * Pointers to data needed to render one given Tux' part
 */
struct tux_part_render_data {
	char *name;                      // Name of the part (used as a key of the struct)
	char **default_part_instance;    // Pointer to one of the tuxrendering.default_instances
	item *wearable_item;             // Pointer to one of the Me.XXXX_item
	int part_group;                  // One of the PART_GROUP_XXXX values
};

/**
 * Specification of Tux's parts rendering order
 * Defines how to order Tux's parts for a 'set' of animation phases.
 * A linked list of sets is associated to a (motion_class, rotation) pair
 * (see the definition of struct tux_rendering_s in global.h).
 */
struct tux_part_render_set {
	int phase_start;	// First animation phase of the set
	int phase_end;		// Last animation phase of the set
	struct tux_part_render_data *part_render_data[ALL_PART_GROUPS];	// Ordered array of the data needed to render Tux parts
	struct tux_part_render_set *next;
};
typedef struct tux_part_render_set *tux_part_render_motionclass[MAX_TUX_DIRECTIONS];

/**
 * Contains the prefix of the animation archive files needed to render
 * each Tux's part.
 */
struct tux_part_instances {
	char *head;
	char *torso;
	char *weaponarm;
	char *weapon;
	char *shieldarm;
	char *feet;
};

/**
 * Contains all the informations needed to render Tux
 */
struct tux_rendering {
	struct dynarray motion_class_names;             // All motion classes
	struct tux_part_instances default_instances;    // Default part instances
	tux_part_render_motionclass *render_order;      // The render_sets of each motion class
	int gun_muzzle_height;							// Vertical offset to apply to bullets
} tux_rendering;

/**
 * Contains a set of Tux's parts images for a motion class.
 */
struct tux_motion_class_images {
	struct image part_images[ALL_PART_GROUPS][TUX_TOTAL_PHASES][MAX_TUX_DIRECTIONS];
	char part_names[ALL_PART_GROUPS][64];
};

/**
 * Holds all variables needed to run a chat
 */
struct chat_context {
       enum chat_context_state state;  // current state of the chat engine.
       int wait_user_click;            // TRUE if the chat engine is waiting for a user click.

       int is_subdialog;               // TRUE if the structure references a sub-dialog.
       enemy *partner;                 // The bot we are talking with.
       struct npc *npc;                // The NPC containing the specifications of the dialog to run.
       int partner_started;            // TRUE if the dialog was started by the partner.
       int end_dialog;                 // TRUE if a dialog lua script asked to end the dialog.

       char *initialization_code;      // lua initialization code (run on the first activation)
       char *startup_code;             // lua startup code (run on every activation)

       struct dialogue_option dialog_options[MAX_DIALOGUE_OPTIONS_IN_ROSTER];  // The dialog nodes
       uint8_t *dialog_flags;          // One flag per dialog node: TRUE if the corresponding dialog node is active
       int current_option;             // Current dialog node to run (-1 if none is selected)
       lua_State *lua_coroutine;       // Handle to the lua co-routine running the current node script

       char *topic_stack[CHAT_TOPIC_STACK_SIZE];  // Stack of topics. A topic is a dialog node selector.
       unsigned int topic_stack_slot;             // Index of the top of the stack

       struct list_head stack_node;    // Used to create a stack of chat_context.
};

#endif
