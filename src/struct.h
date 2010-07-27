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

typedef struct tColorRGBA {
	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;
} tColorRGBA, myColor;

typedef struct tColorY {
	Uint8 y;
} tColorY;

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

typedef struct upgrade_socket_dynarray {
	struct upgrade_socket *arr;
	int size;
	int capacity;
} upgrade_socket_dynarray;

/* prototype for BFont_Info from BFont.h */
struct BFont_Info;

/* ----------------------------------------------------------------------
 * Here comes the struct for an iso image.  It also contains some 
 * placeholder for a possible 'zoomed-out' version of the iso image and
 * also some placeholder for a possible OpenGL texture as well.
 *
 * In order to have a convenient means to initialize variables of this
 * type in several places, even in case this struct changes shape in the
 * future, the 'UNLOADED_ISO_IMAGE' definition below is made.  This
 * definition should be used always when initializing a variable of this
 * so that later changes to the struct can be made with minimal effort 
 * and mistakes.
 * ---------------------------------------------------------------------- */
typedef struct iso_image_s {
	SDL_Surface *surface;
	short offset_x;
	short offset_y;
	SDL_Surface *zoomed_out_surface;
	short texture_width;
	short texture_height;
	short original_image_width;
	short original_image_height;
	int texture_has_been_created;
#ifdef HAVE_LIBGL
	GLuint texture;		// this is to store an open_gl texture...
#else
	int placeholder_for_texture_value;	// this is to store an open_gl texture...
#endif
	float tx0;
	float tx1;
	float ty0;
	float ty1;
} iso_image, *Iso_image;
#define UNLOADED_ISO_IMAGE { NULL , 0 , 0 , NULL , 0 , 0 , 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0 }

typedef struct mouse_press_button_s {
	iso_image button_image;
	char *button_image_file_name;
	SDL_Rect button_rect;
	char scale_this_button;
} mouse_press_button, *Mouse_press_button;

typedef struct keybind_s {
	char *name;
		/**< keybinding name, taken from keybindNames */
	int key;
	     /**< key/axis/button event number */
	int mod;
	     /**< Key modifiers */
} keybind_t;

typedef char *string;

typedef struct configuration_for_freedroid_s {
	float WantedTextVisibleTime;
	int Draw_Framerate;
	int Draw_Position;
	int Enemy_Hit_Text;
	int All_Texts_Switch;
	float Current_BG_Music_Volume;
	float Current_Sound_FX_Volume;
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
	int highlighting_mode_full;
	int omit_obstacles_in_level_editor;
	int omit_enemies_in_level_editor;
	int zoom_is_on;
	int show_blood;		// this can be used to make the game more friendly for children...
	int show_tooltips;
	int number_of_big_screen_messages;
	float delay_for_big_screen_messages;
	int enable_cheatkeys;
	int transparency;
	int screen_width;
	int screen_height;
	int next_time_width_of_screen;
	int next_time_height_of_screen;
	float automap_display_scale;
	int skip_shadow_blitting;
	int do_fadings;		// do the screen fadings
	int fullscreen_on;
	int talk_to_bots_after_takeover;
	int xray_vision_for_tux;
	int cheat_running_stamina;
	int lazyload;
	int show_item_labels;
	int last_edited_level;

	int difficulty_level;

	// This must be the last element of the structure, because the
	// input keybind parsing code uses strtok which messes with the
	// string.
	keybind_t input_keybinds[100];
} configuration_for_freedroid;

typedef struct point_s {
	int x;
	int y;
} point;

typedef struct moderately_finepoint_s {
	float x;
	float y;
} moderately_finepoint;

typedef struct finepoint_s {
	double x;
	double y;
} finepoint;

typedef struct gps_s {
	float x;
	float y;
	int z;
} gps;

typedef struct location_s {
	unsigned char level;
	int x;
	int y;
} location;

typedef struct map_label_s {
	char *label_name;
	point pos;		// how many blocks does this big map insert cover?
} map_label;

typedef char *luacode;
typedef struct mission_s {
	string mission_name;
	int MissionWasAssigned;	// has be influencer been assigned to this mission? , currently uninitialized
	int MissionIsComplete;	// has the mission been completed?
	int MissionWasFailed;	// has the mission been failed?
	int MissionExistsAtAll;	// is this mission entry used at all, or is it just unused memory?

	int fetch_item;
	int KillOne;
	int must_clear_first_level;
	int must_clear_second_level;
	int MustReachLevel;
	point MustReachPoint;
	double MustLiveTime;
	int MustBeOne;

	luacode completion_lua_code;
	luacode assignment_lua_code;

	string mission_diary_texts[MAX_MISSION_DESCRIPTION_TEXTS];
	float mission_description_time[MAX_MISSION_DESCRIPTION_TEXTS];
	int expanded_display_for_this_mission;
} mission;

//--------------------
// This structure can contain conditions that must be fulfilled, so that a special
// event is triggered.  Such conditions may be specified in the mission file as well
//
typedef struct event_trigger_s {
	// Maybe the event is triggerd by the influencer stepping somewhere
	int Influ_Must_Be_At_Level;
	point Influ_Must_Be_At_Point;

	int DeleteTriggerAfterExecution;

	luacode lua_code;

	char *name;
	int enabled;		//is the trigger enabled?
	int silent;		//do we have to advertise this trigger to the user? (teleporters..)
} event_trigger;

typedef struct item_image_spec_s {
	point inv_size;
	SDL_Surface *Surface;
	iso_image ingame_iso_image;
	iso_image shop_iso_image;
} item_image_spec;

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

typedef struct itemspec_s {
	char *item_name;
	char *item_rotation_series_prefix;
	char *item_description;
	char *item_drop_sound_file_name;
	char *item_inv_file_name;

	char item_can_be_applied_in_combat;
	char item_can_be_installed_in_weapon_slot;
	char item_can_be_installed_in_drive_slot;
	char item_can_be_installed_in_armour_slot;
	char item_can_be_installed_in_shield_slot;
	char item_can_be_installed_in_special_slot;

	char item_group_together_in_inventory;

	// How good is the item as weapon???
	float item_gun_recharging_time;	// time until the next shot can be made, measures in seconds
	float item_gun_reloading_time;	// time needed to put a new charger
	short item_gun_bullet_image_type;	// which type of image to use for displaying this bullet
	float item_gun_speed;	// how fast should a bullet move straightforward?
	short base_item_gun_damage;	//   damage done by this bullettype 
	short item_gun_damage_modifier;	// modifier to the damage done by this bullettype 
	float item_gun_bullet_lifetime;	// how long does a 'bullet' from this gun type live?
	char item_gun_bullet_reflect_other_bullets;	// can this 'bullet' reflect other bullets
	char item_gun_bullet_pass_through_explosions;	// can this 'bullet' reflect other bullets
	char item_gun_bullet_pass_through_hit_bodies;	// does this bullet go through hit bodies (e.g. like a laser sword)
	char item_gun_bullet_ignore_wall_collisions;	// can this bullet pass through walls and map barriers?
	short item_gun_ammo_clip_size;	//max. number of bullets in the charger

	// the following values have only relevance in case of a melee weapon
	short int item_weapon_is_melee;
	float item_gun_start_angle_modifier;	// where to start with a melee weapon swing
	char item_gun_use_ammunition;	// which ammunition does this gun use? - 1 laser 2 plasma 3 exterminator etc.
	char item_gun_requires_both_hands;	// is this a (strictly) 2-handed weapon?

	// how good is the item as armour or shield or other protection???
	short base_damred_bonus;
	short damred_bonus_modifier;

	// which requirement for strength, dexterity and magic (force) does the item have?
	short int item_require_strength;
	short int item_require_dexterity;
	short int item_require_magic;

	// what duration does the item have?
	short int base_item_duration;
	short int item_duration_modifier;

	// Which picture to use for this item, when it's lying on the floor?
	// int picture_number;
	item_image_spec inv_image;
	short int base_list_price;	// the base price of this item at the shop

	short int min_drop_class;
	short int max_drop_class;

} itemspec;

typedef struct upgrade_socket {
	int type;
	string addon;
} upgrade_socket;

typedef struct item_s {
	// Here are the rather short-lived properties of the item
	gps pos;
	gps virt_pos;
	SDL_Rect text_slot_rectangle;
	int type;
	int currently_held_in_hand;	// is the item currently held 'in hand' with the mouse cursor?
	int max_duration;	// the maximum item durability reachable for this item
	float current_duration;	// the currently remaining durability for this item
	float throw_time;	// has this item just jumped out from a chest maybe or is it jumping right now?

	int bonus_to_dex;
	int bonus_to_str;
	int bonus_to_vit;
	int bonus_to_mag;
	int bonus_to_life;
	int bonus_to_force;
	float bonus_to_health_recovery;
	float bonus_to_cooling_rate;
	int bonus_to_tohit;
	int bonus_to_all_attributes;
	int bonus_to_damred_or_damage;	// this is a percentage
	int bonus_to_resist_fire;	// this is a percentage
	int bonus_to_resist_electricity;	// this is a percentage
	int bonus_to_paralyze_enemy;
	int bonus_to_slow_enemy;
	int bonus_to_light_radius;
	int bonus_to_experience_gain;	// this is a percentage

	int damred_base;		// the base damred given to the item at creation time
	int damred_bonus;		// how much is damred increased by this item worn
	int damage;		// how much damage does this item
	int damage_modifier;	// how much additional damage can add to the base damage
	int multiplicity;
	int ammo_clip;		// how much bullets in the clip, IN CASE OF WEAPON
	point inventory_position;

	struct upgrade_socket_dynarray upgrade_sockets;
} item;

typedef struct druidspec_s {
	char *druidname;
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

	short int monster_level;
	short int forced_magic_items;

	item weapon_item;

	short int amount_of_plasma_transistors;
	short int amount_of_superconductors;
	short int amount_of_antimatter_converters;
	short int amount_of_entropy_inverters;
	short int amount_of_tachyon_condensators;

	short greeting_sound_type;	// which sample to play in order to 'greet' the influencer?
	short got_hit_sound_type;	// which sample to play in order to 'greet' the influencer?
	short to_hit;		// chance that this droid hits an unarmoured target
	float recover_time_after_getting_hit;
	char *notes;		// notes on the druid of this type 
	short int is_human;
	short individual_shape_nr;
} druidspec;

typedef struct enemy_s {
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
	short int type;                     // the number of the droid specifications in Druidmap
	char SpecialForce;                  // this flag will exclude the droid from initial shuffling of droids
	int marker;                         // this provides a marker for special mission targets
	short int max_distance_to_home;     // how far this robot will go before returning to it's home waypoint
	string dialog_section_name;
	string short_description_text;
	short int on_death_drop_item_code;  // drop a pre-determined item when dying

	//--------------------
	// 2nd set ('global state')
	//
	int faction;
	char will_rush_tux;           // will this robot approach the Tux on sight and open communication?
	int combat_state;             // current state of the bot
	float state_timeout;          // time spent in this state (used by "timeout" states such as STOP_AND_EYE_TARGET only)
	short int CompletelyFixed;    // set this flag to make the robot entirely immobile
	char has_been_taken_over;     // has the Tux made this a friendly bot via takeover subgame?
	char follow_tux;              // does this robot try to follow tux via it's random movements?
	char has_greeted_influencer;  // has this robot issued his first-time-see-the-Tux message?
	short int nextwaypoint;       // the next waypoint target
	short int lastwaypoint;       // the waypoint from whence this robot just came from
	short int homewaypoint;       // the waypoint this robot started at
	gps pos;                      // coordinates of the current position in the level

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
	char attack_target_type;           // attack NOTHING, PLAYER, or BOT
	short int bot_target_n;
	struct enemy_s *bot_target_addr;
	float previous_angle;              // which angle has this robot been facing the frame before?
	float current_angle;               // which angle will the robot be facing now?
	float previous_phase;              // which (8-way) direction did the robot face before?
	float last_phase_change;           // when did the robot last change his (8-way-)direction of facing
	float last_combat_step;            // when did this robot last make a step to move in closer or farther away from Tux in combat?
	float TextVisibleTime;
	char *TextToBeDisplayed;           // WARNING!!! Only use static texts
	moderately_finepoint PrivatePathway[5];
	char bot_stuck_in_wall_at_previous_check;
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
	unsigned char chat_character_initialized;
	unsigned char chat_flags[MAX_ANSWERS_PER_PERSON];

	string shoplist[MAX_ITEMS_IN_INVENTORY]; //list of items that can be put in the inventory of the NPC
	int shoplistweight[MAX_ITEMS_IN_INVENTORY]; //weight of each item: relative probability of appearance in inventory
	item npc_inventory[MAX_ITEMS_IN_NPC_INVENTORY]; // current NPC stock (what can be readily bought)

	float last_trading_date; // when did we trade with this NPC the last time?

	list_head_t node; 
} npc;

typedef char automap_data_t[100][100];
typedef struct tux_s {
	float current_game_date;	// seconds since game start, will be printed as a different 'date'
	// inside the game, like 14:23 is afternoon
	int current_power_bonus;
	float power_bonus_end_date;
	int current_dexterity_bonus;
	float dexterity_bonus_end_date;

	finepoint speed;	// the current speed of the druid 
	gps pos;		// current position in the whole ship 
	gps teleport_anchor;	// where from have you last teleported home
	gps mouse_move_target;	// where the tux is going automatically by virtue of mouse move

	short int current_enemy_target_n;	//which enemy has been targeted 
	char god_mode;
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

	char savegame_version_string[1000];	// a string to identify games from older freedroid versions

	int Strength;		// character Strength value = 'power supply max. capacity'
	int Magic;		// character Magic value = 
	int Dexterity;		// character Dexterity value = 'power redistribution speed'
	int base_vitality;	// character Vitality value = 'cloaking field maximum strength'
	int base_strength;	// character Strength value = 'power supply max. capacity'
	int base_magic;		// character Magic value = 
	int base_dexterity;	// character Dexterity value = 'power redistribution speed'
	int Vitality;		// character Vitality value = 'cloaking field maximum strength'
	int points_to_distribute;	// these are the points that are available to distribute upon the character stats
	float base_damage;	// the current damage the influencer does
	float damage_modifier;	// the modifier to the damage the influencer currently does
	float DAMRED;		// the current Armour Damage Reduction of the influencer
	float to_hit;		// percentage chance, that Tux will hit a random lv 1 bot
	int lv_1_bot_will_hit_percentage;	// percentage chance that a random lv 1 bot will hit
	int resist_fire;	// percentage to reduce from fire damage
	int resist_electricity;	// percentage to reduce from electricity damage

	int slowing_melee_targets;	// duration for how long hit enemies are slowed down
	int paralyzing_melee_targets;	// duration for how long hit enemies are paralyzed
	int double_ranged_damage;	// does this Tux do double ranged weapon damage?
	float experience_factor; // multiplier for the experience gained from bots

	unsigned int Experience;	// character Experience = 'spare droid elements found'
	int exp_level;		// which 'experience level' is the influencer currenly at?
	unsigned int ExpRequired;	// how much experience required for the next level?
	unsigned int ExpRequired_previously;	// how was required for the previous level?

	unsigned int Gold;
	char character_name[MAX_CHARACTER_NAME_LENGTH];
	mission AllMissions[MAX_MISSIONS_IN_GAME];	// What must be done to fullfill this mission?
	int marker;		// In case you've taken over a marked droid, this will contain the marker
	float LastCrysoundTime;
	float TextVisibleTime;
	char *TextToBeDisplayed;                    // WARNING!!! Only use static texts

	//--------------------
	// Here we note all the 'skill levels' of the Tux and also which skill is
	// currently readied and that...
	//
	int readied_skill;
	int SkillLevel[MAX_NUMBER_OF_PROGRAMS];
	int base_skill_level[MAX_NUMBER_OF_PROGRAMS];
	int melee_weapon_skill;
	int ranged_weapon_skill;
	int spellcasting_skill;

	//--------------------
	// The inventory slots.  Some items are residing in general inventory,
	// other items might be equiped in some of the corresponding slots of
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
	unsigned char HaveBeenToLevel[MAX_LEVELS];	// record of the levels the player has visited yet.
	float time_since_last_visit_or_respawn[MAX_LEVELS];	// record of the levels the player has visited yet.

	string cookie_list[MAX_COOKIES];

	//--------------------
	// THE FOLLOWING ARE INFORMATION, THAT ARE HUGE AND THAT ALSO DO NOT NEED
	// TO BE COMMUNICATED FROM THE CLIENT TO THE SERVER OR VICE VERSA
	//
	moderately_finepoint next_intermediate_point[MAX_INTERMEDIATE_WAYPOINTS_FOR_TUX];	// waypoints for the tux, when target not directly reachable
	unsigned short int TakeoverSuccesses[200]; // how many (of each type) did Tux takeover and make friendly?
	unsigned short int TakeoverFailures[200];  // how many times for (each type) did Tux fail at a takeover attempt?
	automap_data_t Automap[MAX_LEVELS];
	int current_zero_ring_index;
	gps Position_History_Ring_Buffer[MAX_INFLU_POSITION_HISTORY];

	int BigScreenMessageIndex;
	string BigScreenMessage[MAX_BIG_SCREEN_MESSAGES];
	float BigScreenMessageDuration[MAX_BIG_SCREEN_MESSAGES];

	float slowdown_duration;
	float paralyze_duration;
	float invisible_duration;

	int quest_browser_changed;

	int program_shortcuts[10];
} tux_t;

typedef struct bulletspec_s {
	int phases;		// how many phases in motion to show 
	double phase_changes_per_second;	// how many different phases to display every second
	iso_image image[BULLET_DIRECTIONS][MAX_PHASES_IN_A_BULLET];
} bulletspec;

typedef struct bullet_s {
	short int type;
	unsigned char phase;
	signed char mine;
	gps pos;
	moderately_finepoint speed;
	short int time_in_frames;	// how long does the bullet exist, measured in number of frames
	short int damage;	// damage done by this particular bullet
	float time_in_seconds;	// how long does the bullet exist in seconds
	float bullet_lifetime;	// how long can this bullet exist at most
	float time_to_hide_still;	// countdown to when the bullet will actually appear
	char reflect_other_bullets;
	short int owner;
	float angle;

	char was_reflected;
	char ignore_wall_collisions;
	short int to_hit;
	char pass_through_hit_bodies;	// does this bullet go through hit bodies (e.g. like a laser sword stike)
	char pass_through_explosions;	// does this bullet go through explosions (e.g. laser sword stike though dead droid)
	short int freezing_level;	// does this bullet freeze the target?
	float poison_duration;
	float poison_damage_per_sec;
	float paralysation_duration;

	int faction;
	char hit_type;		//hit bots, humans, both?
} bullet, *Bullet;

typedef struct melee_shot_s	// this is a melee shot
{
	char attack_target_type;	//type of attack
	char mine;		//is it mine?
	short int bot_target_n;	//which enemy has been targeted 
	enemy *bot_target_addr;	// which enemy has been targeted, address
	short int to_hit;	//chance to hit, percent
	short int damage;
	short int owner;
	char level;
} melee_shot;

typedef struct blastspec_s {
	int phases;
	float total_animation_time;
	iso_image image[PHASES_OF_EACH_BLAST];
} blastspec;

typedef struct blast_s {
	gps pos;
	int type;
	float phase;
	int MessageWasDone;
	float damage_per_second;
} blast, *Blast;

typedef struct spell_active_s {
	int img_type;		// what type of spell is active?
	int damage;
	int poison_duration;
	int poison_dmg;
	int freeze_duration;
	int paralyze_duration;
	moderately_finepoint spell_center;
	float spell_radius;
	float spell_age;
	char active_directions[RADIAL_SPELL_DIRECTIONS];
	int mine;
	char hit_type;
} spell_active;

typedef struct spell_skill_spec_s {
	char *name;
	char *icon_name;
	iso_image icon_surface;
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

typedef struct waypoint_s {
	int x;
	int y;
	int suppress_random_spawn;
	struct dynarray connections;
} waypoint;

typedef struct obstacle_s {
	int type;
	gps pos;
} obstacle;

typedef struct map_tile_s {
	Uint16 floor_value;
	int obstacles_glued_to_here[MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE];
} map_tile;

struct obstacle_extension {
	obstacle *obs;
	enum obstacle_extension_type type;
	void *data;
}; /** This contains "extension data" for obstacles - labels, item lists, ... */

typedef struct level_s {
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

	map_tile *map[MAX_MAP_LINES];	// this is a vector of pointers
	int jump_target_north;
	int jump_target_south;
	int jump_target_east;
	int jump_target_west;
	int use_underground_lighting;

	obstacle obstacle_list[MAX_OBSTACLES_ON_MAP];
	item ItemList[MAX_ITEMS_PER_LEVEL];

	struct dynarray obstacle_extensions;
	struct dynarray map_labels;
	struct dynarray waypoints;

	int teleport_pair;
} level, *Level;

typedef struct obstacle_spec_s {
	iso_image image;
	iso_image shadow_image;
	SDL_Surface *automap_version;

	char image_loaded;
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
	// Some obstacles will emitt light.  Specify light strength here.
	// A value of 0 light will be sufficient in most cases...
	//
	short emitted_light_strength;
	char transparent;
	
	char *filename;
    void (*action) (level *obst_lvl, int obstacle_id);
	
	//--------------------
	// Some obstacles have an associated animation.
	// This property defines the function to call to animate them
	int (*animate_fn) (level *obstacle_lvl, int obstacle_idx);
} obstacle_spec;

struct visible_level {
	int valid;
	level *lvl_pointer;
	float boundary_squared_dist;
	struct list_head animated_obstacles_list;
	int animated_obstacles_dirty_flag;
	struct list_head node;
};

typedef struct ship_s {
	int num_levels;
	level *AllLevels[MAX_LEVELS];
} ship;

typedef struct dialogue_option_s {
	char *option_text;
	char *option_sample_file_name;

	char *reply_sample_list[MAX_REPLIES_PER_OPTION];
	char *reply_subtitle_list[MAX_REPLIES_PER_OPTION];

	luacode lua_code;

	char exists;
} dialogue_option;

typedef struct supported_languages_s {
	char *code;
	char *name;
	char *font_class;
	char *encoding;
} supported_languages_t;

typedef struct colldet_filter_s {
	int (*callback) (struct colldet_filter_s * filter, obstacle * obs, int obs_idx);
	void *data;
	float extra_margin;
	struct colldet_filter_s *next;
} colldet_filter;

typedef struct light_radius_config_s {
	uint32_t cells_w;
	uint32_t cells_h;
	uint32_t texture_w;
	uint32_t texture_h;
	int translate_y;
	float scale_factor;
} light_radius_config;

typedef struct screen_resolution_s {
	int xres;
	int yres;
	char *comment;
	int supported;
} screen_resolution;

/*
 * CheckIfWayIsFreeOfDroids's execution context
 * 
 * Note : '2' excepted bots are needed when the pathfinder is called
 *         to let a bot reach an other one (used during attack, for example).
 */
typedef struct freeway_context_s {
	int check_tux;		// Check if Tux is along the way
	Enemy except_bots[2];	// Do not check if those bots are along the way (see note below)
} freeway_context;

/*
 * Pathfinder's execution context
 */
typedef struct pathfinder_context_s {
	colldet_filter *dlc_filter;	// DLC filter to use
	freeway_context *frw_ctx;	// CheckIfWayIsFreeOfDroids's execution context to use
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

/* text_widget.c */
typedef struct text_widget {
	SDL_Rect rect;             /* The area in which the text should be displayed */
	struct BFont_Info *font;
	struct auto_string *text;
	float text_stretch;

	int scroll_offset;         /* 0 means bottom, negative means above bottem. */
	int mouse_already_handled;

	void (*content_below_func)(void);
	void (*content_above_func)(void);
} text_widget;

#endif
