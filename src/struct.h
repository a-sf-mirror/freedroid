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
typedef struct iso_image_s
{
    SDL_Surface* surface;
    short offset_x;
    short offset_y;
    SDL_Surface* zoomed_out_surface;
    short texture_width;
    short texture_height;
    short original_image_width;
    short original_image_height;
    int texture_has_been_created;
    void* attached_pixel_data;
#ifdef HAVE_LIBGL
    GLuint texture;  // this is to store an open_gl texture...
#else
    int placeholder_for_texture_value;  // this is to store an open_gl texture...
#endif
    float tx0;
    float tx1;
    float ty0;
    float ty1;
}
iso_image, *Iso_image;
#define UNLOADED_ISO_IMAGE { NULL , 0 , 0 , NULL , 0 , 0 , 0, 0, 0, NULL, 0, 0.0, 0.0, 0.0, 0.0 }

typedef struct mouse_press_button_s
{
    iso_image button_image;
    char *button_image_file_name;
    SDL_Rect button_rect;
    char scale_this_button;
    char use_true_alpha_blending;
}
mouse_press_button, *Mouse_press_button;

typedef struct configuration_for_freedroid_s
{
    float WantedTextVisibleTime;
    int Draw_Framerate;
    int Draw_Position;
    int Enemy_Hit_Text;
    int Enemy_Bump_Text;
    int Enemy_Aim_Text;
    int All_Texts_Switch;
    float Current_BG_Music_Volume;
    float Current_Sound_FX_Volume;
    float current_gamma_correction;
    int StandardEnemyMessages_On_Off;
    int StandardInfluencerMessages_On_Off;
    int Mouse_Input_Permitted;

    //--------------------
    // Now we add all the variables for the current screen/hud
    // configuration:  Is inventory screen or character screen
    // or something like that currently visible or not.
    //
    int Mission_Log_Visible;
    float Mission_Log_Visible_Time;
    float Mission_Log_Visible_Max_Time;
    int Inventory_Visible;
    int CharacterScreen_Visible;
    int SkillScreen_Visible;
    int Automap_Visible;
    int spell_level_visible;

    char freedroid_version_string[500];
    int skip_light_radius;
    int skill_explanation_screen_visible;
    int enemy_energy_bars_visible;
    int hog_CPU;
    int highlighting_mode_full;
    int omit_tux_in_level_editor;
    int omit_obstacles_in_level_editor;
    int omit_enemies_in_level_editor;
    int level_editor_edit_mode;
    int zoom_is_on;
    int show_blood; // this can be used to make the game more friendly for children...
    int show_tooltips;
    int number_of_big_screen_messages ;
    float delay_for_big_screen_messages ;
    int enable_cheatkeys;
    int transparency ;
    int automap_manual_shift_x ;
    int automap_manual_shift_y ;
    int screen_width;
    int screen_height;
    int next_time_width_of_screen;
    int next_time_height_of_screen;
    float automap_display_scale ;
    int skip_shadow_blitting ;
    int language ; // index of language, see in defs.h
    int do_fadings; // do the screen fadings
    int auto_display_to_help; // display the takeover help
    int fullscreen_on;
    int talk_to_bots_after_takeover;
    int xray_vision_for_tux;
}
configuration_for_freedroid , *Configuration_for_freedroid;

typedef struct point_s
{
    int x;
    int y;
}
point, *Point;

typedef struct moderately_finepoint_s
{
    float x;
    float y;
}
moderately_finepoint, *Moderately_finepoint;

typedef struct finepoint_s
{
    double x;
    double y;
}
finepoint, *Finepoint;

typedef struct gps_s
{
    float x;
    float y;
    int z;
} gps, *GPS;

typedef struct location_s
{
    unsigned char level;			
    int x;			
    int y;
}
location, *Location;

typedef struct map_label_s
{
    char* label_name;
    point pos; // how many blocks does this big map insert cover?
}
map_label, *Map_Label;

typedef struct mission_s
{
    char MissionName[500];  // this should be the name of the mission, currently uninitialized
    int MissionWasAssigned; // has be influencer been assigned to this mission? , currently uninitialized
    int MissionIsComplete; // has the mission been completed?
    int MissionWasFailed; // has the mission been failed?
    int MissionExistsAtAll; // is this mission entry used at all, or is it just unused memory?
    int AutomaticallyAssignThisMissionAtGameStart; // well...
    
    int fetch_item;
    int KillOne;
    int must_clear_first_level;
    int must_clear_second_level;
    int   MustReachLevel;
    point MustReachPoint;
    double MustLiveTime;
    int MustBeType;
    int MustBeOne;
    
    int ListOfActionsToBeTriggeredAtAssignment[ MAX_MISSION_TRIGGERED_ACTIONS ];
    int ListOfActionsToBeTriggeredAtCompletition[ MAX_MISSION_TRIGGERED_ACTIONS ];

    int mission_description_visible [ MAX_MISSION_DESCRIPTION_TEXTS ] ;    
    float mission_description_time [ MAX_MISSION_DESCRIPTION_TEXTS ] ;
    int expanded_display_for_this_mission;
}
mission, *Mission;

//--------------------
// This structure can contain things, that might be triggered by a special
// condition, that can be specified in the mission file as well.
//
typedef struct triggered_action_s
{
    char* ActionLabel;  // this is a better reference than a number
    
    // Maybe the triggered action will change some obstacle on some level...
    char* modify_obstacle_with_label;
    char* modify_obstacle_to_type;

    char * modify_event_trigger_with_action_label;
    int modify_event_trigger_value;
    
    // Maybe the triggered event teleports the influencer somewhere
    point TeleportTarget;
    int TeleportTargetLevel;
    
    char * also_execute_action_label; //execute another action (linked action)
    
}
triggered_action , *Triggered_action;

//--------------------
// This structure can contain conditions that must be fulfilled, so that a special
// event is triggered.  Such conditions may be specified in the mission file as well
//
typedef struct event_trigger_s
{
    // Maybe the event is triggerd by the influencer stepping somewhere
    int Influ_Must_Be_At_Level;
    point Influ_Must_Be_At_Point;
    
    // Maybe the event is triggered by time
    float Mission_Time_Must_Have_Passed;
    float Mission_Time_Must_Not_Have_Passed;
    
    int DeleteTriggerAfterExecution;
    // And now of course which event to trigger!!!!
    // Thats propably the most important information at all!!!
    // int EventNumber;
    char* TargetActionLabel;

    int enabled; //is the trigger enabled?
}
event_trigger , *Event_trigger;

typedef struct map_statement_s
{
    int x;
    int y;
    char* Statement_Text;
} map_statement , *Map_statement;

typedef struct obstacle_spec_s
{
    iso_image image;
    iso_image shadow_image;
    SDL_Surface* automap_version;
    //--------------------
    // Some obstacles will block the Tux from walking through them.
    // Currently only rectangles are supported block areas.  The width
    // (i.e. east-west=parm1) and height (i.e. north-south=parm2) of
    // the blocking rectangle can ge specified below.
    //
    float block_area_parm_1;
    float block_area_parm_2;
    float upper_border ;
    float lower_border ;
    float left_border ;
    float right_border ;

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
    //--------------------
    // This is a special property for obstacles, that can be 
    // stepped on, like a rug or floor plate, for proper visibility...
    //
    char* filename;
    char* obstacle_short_name;
    char* obstacle_long_description;
}
obstacle_spec, *Obstacle_spec;

typedef struct item_image_spec_s
{
    point inv_size;
    SDL_Surface* Surface;
    iso_image ingame_iso_image;
    SDL_Surface* scaled_surface_for_shop;
} item_image_spec , *Item_image_spec;

typedef struct item_bonus_s
{
    char* bonus_name;
    short base_bonus_to_dex;
    short modifier_to_bonus_to_dex;
    short base_bonus_to_str;
    short modifier_to_bonus_to_str;
    short base_bonus_to_vit;
    short modifier_to_bonus_to_vit;
    short base_bonus_to_mag;
    short modifier_to_bonus_to_mag;
    short base_bonus_to_all_attributes;
    short modifier_to_bonus_to_all_attributes;
    
    short base_bonus_to_life;
    short modifier_to_bonus_to_life;
    short base_bonus_to_force;
    short modifier_to_bonus_to_force;

    float base_bonus_to_health_recovery;
    float base_bonus_to_cooling_rate;
    
    short base_bonus_to_tohit;
    short modifier_to_bonus_to_tohit;
    short base_bonus_to_ac_or_damage; // this is a percentage
    short modifier_to_bonus_to_ac_or_damage; // this is a percentage

    char base_bonus_to_resist_fire;  // this is a percentage /*XXX bonus to resist whatever ARE NOT READ YET!*/
    char modifier_to_bonus_to_resist_fire;  // this is a percentage
    char base_bonus_to_resist_electricity; // this is a percentage
    char modifier_to_bonus_to_resist_electricity; // this is a percentage
    char base_bonus_to_resist_disruptor; // this is a percentage
    char modifier_to_bonus_to_resist_disruptor; // this is a percentage
    
    char light_bonus_value;

    char level; //"level" of the prefix (is it good or not)
    float price_factor;

} item_bonus , *Item_bonus;

typedef struct itemspec_s
{ 
    char* item_name;
    char* item_rotation_series_prefix;
    char* item_description;
    char* item_drop_sound_file_name;
    char* item_inv_file_name;
    
    char item_can_be_applied_in_combat;
    char item_can_be_installed_in_weapon_slot;
    char item_can_be_installed_in_drive_slot;
    char item_can_be_installed_in_armour_slot;
    char item_can_be_installed_in_shield_slot;
    char item_can_be_installed_in_special_slot;
    
    char item_group_together_in_inventory;
    
    // How good is the item as weapon???
    float item_gun_recharging_time;       // time until the next shot can be made, measures in seconds
    float item_gun_reloading_time;       // time needed to put a new charger
    short    item_gun_bullet_image_type;       // which type of image to use for displaying this bullet
    float item_gun_speed; // how fast should a bullet move straightforward?
    short    base_item_gun_damage; //	damage done by this bullettype 
    short    item_gun_damage_modifier; // modifier to the damage done by this bullettype 
    float item_gun_bullet_lifetime;      // how long does a 'bullet' from this gun type live?
    char    item_gun_bullet_reflect_other_bullets; // can this 'bullet' reflect other bullets
    char    item_gun_bullet_pass_through_explosions; // can this 'bullet' reflect other bullets
    char    item_gun_bullet_pass_through_hit_bodies; // does this bullet go through hit bodies (e.g. like a laser sword)
    char    item_gun_bullet_ignore_wall_collisions; // can this bullet pass through walls and map barriers?
    short    item_gun_ammo_clip_size; //max. number of bullets in the charger
    
    // the following values have only relevance in case of a melee weapon
    char item_weapon_is_melee;	
    float item_gun_start_angle_modifier;	// where to start with a melee weapon swing
    char    item_gun_use_ammunition; // which ammunition does this gun use? - 1 laser 2 plasma 3 exterminator etc.
    char    item_gun_requires_both_hands; // is this a (strictly) 2-handed weapon?
    
    // how good is the item as armour or shield or other protection???
    short base_ac_bonus;
    short ac_bonus_modifier;
    
    // which requirement for strength, dexterity and magic (force) does the item have?
    short int item_require_strength;
    short int item_require_dexterity;
    short int item_require_magic;
    
    // what duration does the item have?
    short int base_item_duration;
    short int item_duration_modifier;
    
    // Which picture to use for this item, when it's lying on the floor?
    // int picture_number;
    item_image_spec inv_image ;
    short int base_list_price;         // the base price of this item at the shop

    short int min_drop_class;
    short int max_drop_class;
    
} itemspec , *Itemspec;

typedef struct item_s
{
    // Here are the rather short-lived properties of the item
    finepoint pos;
    SDL_Rect text_slot_rectangle;
    int type;
    int currently_held_in_hand;      // is the item currently held 'in hand' with the mouse cursor?
    int is_identified;               // is the item identified already?
    int max_duration;                // the maximum item durability reachable for this item
    float current_duration;          // the currently remaining durability for this item
    float throw_time;                // has this item just jumped out from a chest maybe or is it jumping right now?
    
    // Here are the rather long-lived properties of the item
    int prefix_code;
    int suffix_code;
    
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
    int bonus_to_ac_or_damage;       // this is a percentage
    int bonus_to_resist_fire;        // this is a percentage
    int bonus_to_resist_electricity; // this is a percentage
    int bonus_to_resist_disruptor;       // this is a percentage
    
    int ac_bonus;                    // how much is ac increased by this item worn
    int damage;                      // how much damage does this item
    int damage_modifier;             // how much additional damage can add to the base damage
    int multiplicity;
    int ammo_clip; 		// how much bullets in the clip, IN CASE OF WEAPON
    point inventory_position;
} item, *Item;

typedef struct druidspec_s
{
    char *druidname;
    char *droid_portrait_rotation_series_prefix;
    char *droid_death_sound_file_name;
    char *droid_attack_animation_sound_file_name;
    int class;
    float maxenergy;		// the maximum energy the batteries can carry 
    float max_temperature;		// the maximum force this droids mind can carry 

    float lose_health;		// the energy/time the droid gains (heals)
    
    float maxspeed;
    float accel;
    
    char flashimmune;		// is the droid immune to FLASH-bullets 
    short experience_reward;			// experience_reward for the elimination of one droid of this type 
    unsigned char brain;
    unsigned char sensor1;
    unsigned char sensor2;
    unsigned char sensor3;
    
    float range_of_vision;
    float time_spent_eyeing_tux;
    
    short int monster_level;
    short int forced_magic_items;
    
    item weapon_item;
    
    char amount_of_plasma_transistors;
    char amount_of_superconductors;
    char amount_of_antimatter_converters;
    char amount_of_entropy_inverters;
    char amount_of_tachyon_condensators;
    
    short greeting_sound_type;              // which sample to play in order to 'greet' the influencer?
    short got_hit_sound_type;               // which sample to play in order to 'greet' the influencer?
    short to_hit;                           // chance that this droid hits an unarmoured target
    short getting_hit_modifier;             // modifier for this droid to receive a hit from the player
    float recover_time_after_getting_hit;
    char *notes;			        // notes on the druid of this type 
    char is_human;
    short individual_shape_nr;
}
druidspec, *Druidspec;


typedef struct enemy_s
{
    short int id;
    short type;			// the number of the droid specifications in Druidmap 
    gps pos;		        // coordinates of the current position in the level
    gps virt_pos;		// the virtual position (position of the bot if he was on this level, differs from above when it is on a neighboring level)
    finepoint speed;		// current speed  
    float energy;		// current energy of this droid
    
    float animation_phase;        // the current animation frame for this enemy (starting at 0 of course...)
    short animation_type;           // walk-animation, attack-animation, gethit animation, death animation
    
    short nextwaypoint;		// the next waypoint target
    short lastwaypoint;		// the waypoint from whence this robot just came from
    short homewaypoint;		// the waypoint this robot started at
    short max_distance_to_home;	// how far this robot will go before returning to it's home waypoint
    
    int combat_state;             // current state of the bot
    float state_timeout;          // time spent in this state (used by "timeout" states such as STOP_AND_EYE_TARGET only)
    
    float frozen;                 // is this droid currently frozen and for how long will it stay this way?
    float poison_duration_left;   // is this droid currently poisoned and for how long will it stay this way?
    float poison_damage_per_sec;  // is this droid currently poisoned and how much poison is at work?
    float paralysation_duration_left;  // is this droid currently paralyzed and for how long will it stay this way?
    float pure_wait;		// time till the droid will start to move again
    float firewait;		// time this robot still takes until its weapon will be fully reloaded
    short ammo_left; 		  // ammunition left in the charger
    
    char CompletelyFixed;          // set this flag to make the robot entirely immobile
    char follow_tux;               // does this robot try to follow tux via it's random movements?
    char SpecialForce;             // This flag will exclude the droid from initial shuffling of droids
    short on_death_drop_item_code;  // drop a pre-determined item when dying?
    
    int marker;                   // This provides a marker for special mission targets
    
    char is_friendly;              // is this a friendly droid or is it a MS controlled one?
    char has_been_taken_over;      // has the Tux made this a friendly bot via takeover subgame?
    char attack_target_type ;      // attack NOTHING, PLAYER, or BOT
    short int bot_target_n;
    struct enemy_s * bot_target_addr ; 
    char attack_run_only_when_direct_line; // require direct line to target before switching into attach run mode
    char dialog_section_name[ MAX_LENGTH_FOR_DIALOG_SECTION_NAME ]; // This should indicate one of the many sections of the Freedroid.dialogues file
    char short_description_text[ MAX_LENGTH_OF_SHORT_DESCRIPTION_STRING ]; // This should indicate one of the many sections of the Freedroid.dialogues file
    char will_rush_tux;            // will this robot approach the Tux on sight and open communication?
    char has_greeted_influencer;   // has this robot issued his first-time-see-the-Tux message?
    float previous_angle;         // which angle has this robot been facing the frame before?
    float current_angle;          // which angle will the robot be facing now?
    float last_phase_change;      // when did the robot last change his (8-way-)direction of facing
    float previous_phase;         // which (8-way) direction did the robot face before?
    float last_combat_step;       // when did this robot last make a step to move in closer or farther away from Tux in combat?
    
    float TextVisibleTime;
    char* TextToBeDisplayed;
    moderately_finepoint PrivatePathway[ 5 ];

    char bot_stuck_in_wall_at_previous_check;
    float time_since_previous_stuck_in_wall_check;

    list_head_t global_list; // entry of this bot in the global bot lists (alive or dead)
    list_head_t level_list; // entry of this bot in the level bot list (alive only)
}
enemy, *Enemy;

typedef char automap_data_t[100][100];
typedef char * bigscrmsg_t;
typedef unsigned char chatflags_t[MAX_ANSWERS_PER_PERSON];
typedef char cookielist_t[MAX_COOKIE_LENGTH];

typedef struct tux_s
{
    char type;			  
    char status;		  
    
    float current_game_date;      // seconds since game start, will be printed as a different 'date'
                                  // inside the game, like 14:23 is afternoon
    int current_power_bonus;
    float power_bonus_end_date;
    int current_dexterity_bonus;
    float dexterity_bonus_end_date;
    
    finepoint speed;		  // the current speed of the druid 
    gps pos;		          // current position in the whole ship 
    gps teleport_anchor;            // where from have you last teleported home
    gps mouse_move_target;          // where the tux is going automatically by virtue of mouse move

    short int current_enemy_target_n; //which enemy has been targeted 
    enemy * current_enemy_target_addr; // which enemy has been targeted, address
    
    int mouse_move_target_combo_action_type; // what extra action has to be done upon arrival?
    int mouse_move_target_combo_action_parameter; // extra data to use for the combo action
    
    int light_bonus_from_tux ;
    int map_maker_is_present ;
    
    float maxenergy; // current top limit for the influencers energy
    float energy;		  // current energy level 
    float max_temperature;   // current top limit for temperature (highest is better)
    float temperature;                    // current temperature
    float old_temperature;                    // current temperature
    float max_running_power;
    float running_power;
    int running_must_rest;
    int running_power_bonus;

    float health_recovery_rate; //points of health recovered each second
    float cooling_rate; //temperature points recovered each second
    
    int16_t LastMouse_X;             // mostly for other players:  Where was the last mouseclick...
    int16_t LastMouse_Y;             // mostly for other players:  Where was the last mouseclick...
    
    double busy_time;		// time remaining, until the weapon is ready to fire again...
    int busy_type; 		// reason why tux is busy (enum)
    double phase;			// the current phase of animation 
    float angle ;
    float walk_cycle_phase;       // 
    float weapon_swing_time;	// How long is the current weapon swing in progress (in seconds of course) 
    float MissionTimeElapsed;
    float got_hit_time;           // how long stunned now since the last time tux got hit 
    
    char freedroid_version_string[1000]; // a string to identify games from older freedroid versions
    
    int Strength;  // character Strength value = 'power supply max. capacity'
    int Magic;     // character Magic value = 
    int Dexterity; // character Dexterity value = 'power redistribution speed'
    int base_vitality;  // character Vitality value = 'cloaking field maximum strength'
    int base_strength;  // character Strength value = 'power supply max. capacity'
    int base_magic;     // character Magic value = 
    int base_dexterity; // character Dexterity value = 'power redistribution speed'
    int Vitality;  // character Vitality value = 'cloaking field maximum strength'
    int points_to_distribute; // these are the points that are available to distribute upon the character stats
    float base_damage; // the current damage the influencer does
    float damage_modifier; // the modifier to the damage the influencer currently does
    float AC; // the current Armour Class of the influencer
    float to_hit;            // percentage chance, that Tux will hit a random lv 1 bot
    int lv_1_bot_will_hit_percentage; // percentage chance that a random lv 1 bot will hit
    int resist_disruptor;        // percentage to reduce from discruptor aka. "flash" damage
    int resist_fire;         // percentage to reduce from fire damage
    int resist_electricity;  // percentage to reduce from electricity damage
    
    int freezing_melee_targets; // does this Tux freeze melee targets upon hit?
    int double_ranged_damage;   // does this Tux do double ranged weapon damage?
    
    unsigned int Experience; // character Experience = 'spare droid elements found'
    int exp_level;       // which 'experience level' is the influencer currenly at?
    unsigned int ExpRequired;    // how much experience required for the next level?
    unsigned int ExpRequired_previously;    // how was required for the previous level?
    
    unsigned int Gold;
    char character_name[ MAX_CHARACTER_NAME_LENGTH ];
    mission AllMissions[ MAX_MISSIONS_IN_GAME ];         // What must be done to fullfill this mission?
    int marker;                   // In case you've taken over a marked droid, this will contain the marker
    float LastCrysoundTime;
    float LastTransferSoundTime;
    float TextVisibleTime;
    char* TextToBeDisplayed;
    float Current_Victim_Resistance_Factor;
    
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
    item Inventory[ MAX_ITEMS_IN_INVENTORY ];
    item weapon_item;
    item drive_item;
    item armour_item;
    item shield_item;
    item special_item;
    
    //--------------------
    // A record of when and if the tux has been on some maps...
    //
    unsigned char HaveBeenToLevel [ MAX_LEVELS ]; // record of the levels the player has visited yet.
    float time_since_last_visit_or_respawn [ MAX_LEVELS ]; // record of the levels the player has visited yet.
    
    //--------------------
    // Some story-based variables:  which persons has the Tux talked to and
    // what are the dialog options currently open, which 'cookies' have been
    // set by the dialogs for coordination among each other, and also status
    // of the Tux like town guard member or not and the like...
    //
    chatflags_t Chat_Flags[ MAX_PERSONS ];
    cookielist_t cookie_list[ MAX_COOKIES ] ;
    int is_town_guard_member;
    unsigned char chat_character_initialized [ MAX_PERSONS ]; 
    
    //--------------------
    // THE FOLLOWING ARE INFORMATION, THAT ARE HUGE AND THAT ALSO DO NOT NEED
    // TO BE COMMUNICATED FROM THE CLIENT TO THE SERVER OR VICE VERSA
    //
    moderately_finepoint next_intermediate_point [ MAX_INTERMEDIATE_WAYPOINTS_FOR_TUX ] ;  // waypoints for the tux, when target not directly reachable
    unsigned short int	KillRecord[ 200 ];      // how many ( of the first 1000 monster types) have been killed yet?
    automap_data_t Automap[MAX_LEVELS];
    int current_zero_ring_index;
    gps Position_History_Ring_Buffer[ MAX_INFLU_POSITION_HISTORY ];
    
    int BigScreenMessageIndex;
    bigscrmsg_t BigScreenMessage [ MAX_BIG_SCREEN_MESSAGES ]; // bitch
    float BigScreenMessageDuration [ MAX_BIG_SCREEN_MESSAGES ];

    float slowdown_duration;
    float paralyze_duration;
    float invisible_duration;
}
tux_t, *Tux_t;


typedef struct bulletspec_s
{
    int phases;			// how many phases in motion to show 
    double phase_changes_per_second; // how many different phases to display every second
    iso_image image [ BULLET_DIRECTIONS ] [ MAX_PHASES_IN_A_BULLET ] ;
} 
bulletspec, *Bulletspec;

typedef struct bullet_s
{
    short int                  type; 
    unsigned char              phase;                
    signed char                mine;     
    gps                        pos;     
    moderately_finepoint       speed;  
    short int                  time_in_frames; // how long does the bullet exist, measured in number of frames
    short int                  damage;        // damage done by this particular bullet
    float                      time_in_seconds; // how long does the bullet exist in seconds
    float                      bullet_lifetime; // how long can this bullet exist at most
    float                      time_to_hide_still; // countdown to when the bullet will actually appear
    char                       reflect_other_bullets;
    short int                  owner;              
    float                      angle;             

    char                       was_reflected;
    char                       ignore_wall_collisions;
    short int                  to_hit;              
    char                       pass_through_hit_bodies;  // does this bullet go through hit bodies (e.g. like a laser sword stike)
    char                       pass_through_explosions; // does this bullet go through explosions (e.g. laser sword stike though dead droid)
    short int                  freezing_level;       // does this bullet freeze the target?
    float                      poison_duration;     
    float                      poison_damage_per_sec;
    float                      paralysation_duration;

    char is_friendly; //ugly hack, great idea - if the bot has the same value, the bullet just goes through
    char hit_type; //hit bots, humans, both?
}
bullet, *Bullet;

typedef struct melee_shot_s  // this is a melee shot
{
    char attack_target_type; //type of attack
    char mine; //is it mine?
    short int bot_target_n; //which enemy has been targeted 
    enemy * bot_target_addr; // which enemy has been targeted, address
    short int to_hit; //chance to hit, percent
    short int damage; 
    short int owner;
    char level;
} melee_shot; 

typedef struct blastspec_s
{
    int phases;
    float total_animation_time;
    iso_image image [ PHASES_OF_EACH_BLAST ] ;
}
blastspec, *Blastspec;

typedef struct blast_s
{
    gps pos;
    int type;
    float phase;
    int MessageWasDone;
    float damage_per_second;
}
blast, *Blast;

typedef struct spell_active_s
{
    int img_type; // what type of spell is active?
    int damage;
    int poison_duration;
    int poison_dmg;
    int freeze_duration;
    int paralyze_duration;
    moderately_finepoint spell_center;
    float spell_radius;
    float spell_age;
    char active_directions [ RADIAL_SPELL_DIRECTIONS ] ;
    int mine;
    char hit_type;
}
spell_active, *Spell_Active;

typedef struct spell_skill_spec_s
{
    char* name;
    char* icon_name;
    iso_image icon_surface;
    short heat_cost;
    short heat_cost_per_level;
    short damage_base;
    short damage_mod;
    short damage_per_level;
    char hurt_bots;
    char hurt_humans;
    char* description;
    char* effect;
    float effect_duration;
    float effect_duration_per_level;
    char form;
    char present_at_startup;
    char graphics_code;
}
spell_skill_spec, *Spell_Skill_Spec;

typedef struct waypoint_s
{
    int x;			
    int y;
    int num_connections;
    int suppress_random_spawn;
    int connections [ MAX_WP_CONNECTIONS ] ;
}
waypoint, *Waypoint;

typedef struct obstacle_s
{
    int type;
    moderately_finepoint pos;
    int name_index;
    int description_index;
}
obstacle, *Obstacle;

typedef struct map_tile_s
{
    Uint16 floor_value;
    int obstacles_glued_to_here [ MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE ];
}
map_tile, *Map_tile;

typedef struct level_s
{
    short levelnum;
    short xlen;
    short ylen;
    char light_radius_bonus;
    char minimum_light_value;
    char infinite_running_on_this_level;
    char *Levelname;
    char *Background_Song_Name;
    char *Level_Enter_Comment;
    map_statement StatementList [ MAX_STATEMENTS_PER_LEVEL ];
    char *obstacle_name_list [ MAX_OBSTACLE_NAMES_PER_LEVEL ];
    char *obstacle_description_list [ MAX_OBSTACLE_DESCRIPTIONS_PER_LEVEL ];
    int obstacle_statelist_base [ MAX_OBSTACLE_NAMES_PER_LEVEL ];
    int obstacle_statelist_count [ MAX_OBSTACLE_NAMES_PER_LEVEL ];
    char *obstacle_states_names [ MAX_OBSTACLE_STATES_PER_LEVEL ];
    int obstacle_states_values [ MAX_OBSTACLE_STATES_PER_LEVEL ];

    map_tile *map [ MAX_MAP_LINES ];	// this is a vector of pointers
    short jump_threshold_north;
    short jump_threshold_south;
    short jump_threshold_east;
    short jump_threshold_west;
    short jump_target_north;
    short jump_target_south;
    short jump_target_east;
    short jump_target_west;
    char use_underground_lighting;
    
    obstacle obstacle_list[ MAX_OBSTACLES_ON_MAP ];
    
    //--------------------
    // Now the list of indices that need to be known every
    // frame...
    //
    int refresh_obstacle_indices [ MAX_REFRESHES_ON_LEVEL ] ;
    int teleporter_obstacle_indices [ MAX_TELEPORTERS_ON_LEVEL ] ;
    int door_obstacle_indices [ MAX_DOORS_ON_LEVEL ];
    int autogun_obstacle_indices [ MAX_AUTOGUNS_ON_LEVEL ] ;
    
    map_label labels [ MAX_MAP_LABELS_PER_LEVEL ];
    int num_waypoints;
    waypoint AllWaypoints[MAXWAYPOINTS];
    item    ItemList [ MAX_ITEMS_PER_LEVEL ] ;
    item ChestItemList [ MAX_ITEMS_PER_LEVEL ] ;
}
level, *Level;

typedef struct ship_s
{
    int num_levels;
    char* AreaName;
    Level AllLevels[MAX_LEVELS];
}
ship, *Ship;

typedef struct dialogue_option_s
{
    int position_x;
    int position_y;
    char* option_text;
    char* option_sample_file_name;
    
    char* reply_sample_list[ MAX_REPLIES_PER_OPTION ] ;
    char* reply_subtitle_list[ MAX_REPLIES_PER_OPTION ];
    
    char* extra_list[ MAX_EXTRAS_PER_OPTION ];
    
    char* on_goto_condition;
    int on_goto_first_target;
    int on_goto_second_target;
    int link_target;
    int always_execute_this_option_prior_to_dialog_start;
    
    int change_option_nr [ MAX_DIALOGUE_OPTIONS_IN_ROSTER ];
    int change_option_to_value [ MAX_DIALOGUE_OPTIONS_IN_ROSTER ];
}
dialogue_option, *Dialogue_option;

typedef struct supported_languages_s 
{ 
    char * code; 
    char * name;
    char * font_class; 
} supported_languages_t;

#endif
