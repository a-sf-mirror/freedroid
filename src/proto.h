/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2007 Arthur Huillet 
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
#ifndef _proto_h
#define _proto_h

#include "struct.h"

// main.c 
void Game(void);

// automap.c
void display_automap ( void );
void toggle_automap(void);

// init.c
void ResetGameConfigToDefaultValues ( void );
void ShowStartupPercentage ( int Percentage ) ;
void ParseCommandLine ( int argc , char *const argv[] );
void ClearAutomapData( void );
void InitFreedroid ( int, char ** );
void PrepareStartOfNewCharacter ( char * startpos ) ;
void ThouArtDefeated ( void );
void ThouHastWon ( void );
void PlayATitleFile ( char* Filename );
void Get_Item_Data ( char* DataPointer );

// event.c
void GetEventTriggers ( const char* EventsAndEventTriggersFilename );
void CheckForTriggeredEventsAndStatements (void);
int teleporter_square_below_mouse_cursor ( char* ItemDescText );

// lua.c
void init_lua(void);
void run_lua(const char *code);

// influ.c 
float calc_euklid_distance ( float pos1_x , float pos1_y , float pos2_x , float pos2_y ); 
float vect_len ( moderately_finepoint our_vector );
enemy * GetLivingDroidBelowMouseCursor ( void );
int GetObstacleBelowMouseCursor ( void );
int find_free_floor_items_index ( int levelnum ) ;
int closed_chest_below_mouse_cursor ( void ) ;
int smashable_barrel_below_mouse_cursor ( void ) ;
void tux_wants_to_attack_now ( int use_mouse_cursor_for_targeting ) ;
int PerformTuxAttackRaw ( int use_mouse_cursor_for_targeting ) ;
void TuxReloadWeapon ( void ) ;
void correct_tux_position_according_to_jump_thresholds ( void );
void InitInfluPositionHistory( void );
float GetInfluPositionHistoryX( int Index );
float GetInfluPositionHistoryY( int Index );
float GetInfluPositionHistoryZ( int Index );
void FillInDefaultBulletStruct ( bullet * CurBullet, int bullet_image_type, short int weapon_item_type );
void FireTuxRangedWeaponRaw ( short int weapon_item_type , int bullet_image_type , bullet *, moderately_finepoint target_location ) ;
void move_tux ( void ) ;
void animate_tux ( void ) ;
void check_tux_enemy_collision (void);
void start_tux_death_explosions (void);
void skew_and_blit_rect( float x1, float y1, float x2, float y2, Uint32 color);
moderately_finepoint translate_point_to_map_location ( float axis_x , float axis_y , int zoom_is_on );
void blit_zoomed_iso_image_to_map_position ( iso_image* our_iso_image , float pos_x , float pos_y );
int tux_can_walk_this_line ( float x1, float y1 , float x2 , float y2 );
void clear_out_intermediate_points ( gps *, moderately_finepoint *, int);
int set_up_intermediate_course_between_positions ( enemy * droid, int check_if_free, gps * curpos, moderately_finepoint * move_target, moderately_finepoint * waypoints, int maxwp );
void adapt_position_for_jump_thresholds ( gps* old_position, gps* new_position );

// bullet.c 
void RotateVectorByAngle ( moderately_finepoint* vector , float rot_angle );
void AnalyzePlayersMouseClick ( void ) ;
void MoveBullets (void);
void DoMeleeDamage(void);
void DeleteBullet (int num , int StartBlast );
void StartBlast ( float x , float y , int level , int type, int dmg );
void animate_blasts (void);
void DeleteBlast (int num);
void MoveActiveSpells (void);
void DeleteSpell (int num);
void clear_active_spells ( void );
void CheckBulletCollisions (int num);
void CheckBlastCollisions (int num);
int find_free_bullet_index ( void ) ;
int find_free_melee_shot_index ( void ) ;
void delete_melee_shot ( melee_shot * );

// view.c 
void update_virtual_position ( gps* target_pos , gps* source_pos , int level_num );
void FdFillRect (SDL_Rect rect, SDL_Color color);
void ShowPosition (void);
void ShowCombatScreenTexts ( int mask );
void isometric_show_floor_around_tux_without_doublebuffering (int mask);
void set_up_ordered_blitting_list ( int mask );
void blit_preput_objects_according_to_blitting_list ( int mask );
void blit_nonpreput_objects_according_to_blitting_list ( int mask );
void show_obstacle_labels ( int mask );
void draw_grid_on_the_floor (int mask);
void blit_leveleditor_point ( int x, int y );
void update_item_text_slot_positions ( void );
void blit_all_item_slots ( void );
void AssembleCombatPicture (int );
void blit_tux ( int x , int y );
void PutBullet ( int Bullet_number , int mask );
void PutItem ( int ItemNumber , int mask , int put_thrown_items_flag , int highlight_item );
void PutBlast (int);
void PutEnemy (enemy * e, int x , int y , int mask , int highlight );
void PutMouseMoveCursor ( void ) ;
void ShowInventoryScreen ( void );
void clear_all_loaded_tux_images ( int with_free );
int set_rotation_index_for_this_robot ( enemy* ThisRobot );
int set_rotation_model_for_this_robot ( enemy* ThisRobot );
void grab_enemy_images_from_archive ( int enemy_model_nr );
int level_is_visible ( int level_num );

// light.c 
void LightRadiusInit(void);
void LightRadiusClean(void);
int get_light_strength_screen ( int x, int y );
int get_light_strength_cell ( uint32_t x, uint32_t y );
int get_light_strength ( moderately_finepoint target_pos );
void update_light_list ( void );
void blit_light_radius ( void );

// open_gl.c 

enum {
		Z_DIR,
		X_DIR,
		Y_DIR
};

int our_SDL_flip_wrapper (void ) ;
int our_SDL_blit_surface_wrapper(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
void our_SDL_update_rect_wrapper ( SDL_Surface *screen, Sint32 x, Sint32 y, Sint32 w, Sint32 h ) ;
int our_SDL_fill_rect_wrapper (SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color);
int blit_quad ( int x1 , int y1 , int x2, int y2, int x3, int y3, int x4 , int y4 , Uint32 color );
void drawIsoEnergyBar(int dir, int x, int y, int z, int w, int d, int length, float fill, myColor *c1, myColor *c2  ) ;

SDL_Surface* our_SDL_display_format_wrapperAlpha ( SDL_Surface *surface );
SDL_Surface* our_IMG_load_wrapper( const char *file );
void flip_image_vertically ( SDL_Surface* tmp1 ) ;
void make_texture_out_of_surface ( iso_image* our_image ) ;
void make_texture_out_of_prepadded_image ( iso_image* our_image ) ;
SDL_Surface* pad_image_for_texture ( SDL_Surface* our_surface ) ;
void clear_screen (void) ;
void blit_open_gl_stretched_texture_light_radius ( void ) ;
void PutPixel_open_gl ( int x, int y, Uint32 pixel);
void GL_HighlightRectangle ( SDL_Surface* Surface , SDL_Rect * Area , unsigned char r , unsigned char g , unsigned char b , unsigned char alpha );
void show_character_screen_background ( void );
int safely_initialize_our_default_open_gl_parameters ( void ) ;
void draw_gl_textured_quad_at_map_position ( iso_image * our_floor_iso_image , float our_col , float our_line , float r , float g , float b , int highlight_texture, int blend, float zoom_factor ) ;
void draw_gl_textured_quad_at_screen_position ( iso_image * our_floor_iso_image , int x , int y ) ;
void draw_gl_scaled_textured_quad_at_screen_position ( iso_image * our_floor_iso_image , int x , int y , float scale_factor ) ;
void blit_semitransparent_open_gl_texture_to_screen_position ( iso_image * our_floor_iso_image , int x , int y , float scale_factor ) ;
void blit_special_background ( int background_code );
void open_gl_check_error_status ( const char* name_of_calling_function );
void draw_gl_bg_textured_quad_at_screen_position ( iso_image * our_floor_iso_image , int x , int y ) ;

// blocks.c 
int wall_orientation(int wall);
void try_to_load_ingame_item_surface ( int item_type );
void iso_load_bullet_surfaces ( void );
void get_iso_image_from_file_and_path ( char* fpath , iso_image* our_iso_image , int use_offset_file ) ;
void make_sure_zoomed_surface_is_there ( iso_image* our_iso_image );
void make_sure_automap_surface_is_there ( obstacle_spec* our_obstacle_spec );
void load_item_surfaces_for_item_type ( int item_type );
void Load_Mouse_Move_Cursor_Surfaces(void);
void Load_Skill_Level_Button_Surfaces( void );
void LoadOneSkillSurfaceIfNotYetLoaded ( int SkillSpellNr );
void LoadAndPrepareEnemyRotationModelNr ( int RotationModel );
void LoadAndPrepareRedEnemyRotationModelNr ( int RotationModel );
void LoadAndPrepareGreenEnemyRotationModelNr ( int RotationModel );
void LoadAndPrepareBlueEnemyRotationModelNr ( int RotationModel );
void Load_Enemy_Surfaces (void);
void Load_Tux_Surfaces( void );
void Load_Bullet_Surfaces (void);
void Load_Blast_Surfaces (void);
void load_floor_tiles ( void );
void load_all_obstacles ( void );
void blit_iso_image_to_map_position ( iso_image * our_iso_image , float pos_x , float pos_y );
void blit_iso_image_to_screen_position ( iso_image * our_iso_image , float pos_x , float pos_y );
void blit_outline_of_iso_image_to_map_position ( iso_image * our_iso_image , float pos_x , float pos_y );

// graphics.c 
void set_mouse_cursor_to_shape ( int given_shape );
void make_sure_system_mouse_cursor_is_turned_off ( void );
void make_sure_system_mouse_cursor_is_turned_on ( void );
void blit_our_own_mouse_cursor ( void );
void blit_mouse_cursor_corona ( void );
SDL_Surface* rip_rectangle_from_alpha_image ( SDL_Surface* our_surface , SDL_Rect our_rect ) ;
SDL_Surface* CreateColorFilteredSurface ( SDL_Surface* FirstSurface , int FilterType );
void fade_out_using_gamma_ramp ( void );
void fade_in_using_gamma_ramp ( void );
void MakeGridOnScreen( SDL_Rect* Grid_Rectangle );
void InitPictures (void);
void InitTimer (void);
void InitVideo (void);
void InitOurBFonts (void);
void FreeOurBFonts (void);
void ClearGraphMem ( void );
void SDL_HighlightRectangle ( SDL_Surface* Surface , SDL_Rect Area );
void HighlightRectangle ( SDL_Surface* Surface , SDL_Rect Area );
void ShadowingRectangle ( SDL_Surface* Surface , SDL_Rect Area );
int do_graphical_number_selection_in_range ( int lower_range , int upper_range, int default_value );
Uint8 GetAlphaComponent ( SDL_Surface* surface , int x , int y );

// saveloadgame.c 
void ShowSaveLoadGameProgressMeter( int Percentage , int IsSavegame ) ;
void LoadAndShowThumbnail ( char* CoreFilename );
int SaveGame( void );
int LoadBackupGame( void );
int LoadGame( void );
int DeleteGame( void );
void LoadAndShowStats ( char* CoreFilename );

/* Primitive types */

/* Saving is done via macros */
#define save_pritype(Z,X,Y) fprintf(SaveGameFile, Z, X, *(Y))
#define save_char(X,Y) save_pritype("%s: %hhd\n", X, Y)
#define save_uchar(X,Y) save_pritype("%s: %hhu\n", X, Y)
#define save_uint16_t(X,Y) save_pritype("%s: %hu\n", X, Y)
#define save_int16_t(X,Y) save_pritype("%s: %hd\n", X, Y)
#define save_int32_t(X,Y) save_pritype("%s: %d\n", X, Y)
#define save_uint32_t(X,Y) save_pritype("%s: %u\n", X, Y)
#define save_float(X,Y) save_pritype("%s: %f\n", X, Y)
#define save_double(X,Y) save_pritype("%s: %lf\n", X, Y)
#define save_string(X,Y) save_pritype("%s: %s\n", X, Y)

/* Reading is slightly more difficult so we do it with functions */
void read_int32_t(const char *, const char *, int32_t *);
void read_int16_t(const char *, const char *, int16_t *);
void read_char(const char *, const char *, char *);
void read_uint32_t(const char *, const char *, uint32_t *);
void read_uint16_t(const char *, const char *, uint16_t *);
void read_uchar(const char *, const char *, unsigned char *);
void read_double(const char *, const char *, double *);
void read_float(const char *, const char *, float *);
void read_string(const char *, const char *, char *);

/* Array writing/reading */
void save_moderately_finepoint_array(const char *, moderately_finepoint *, int);
void read_moderately_finepoint_array(const char *, const char *, moderately_finepoint *, int);
void save_mission_array(const char *, mission *, int);
void read_mission_array(const char *, const char *, mission *, int);
void save_int32_t_array(const char *, int *, int);
void read_int32_t_array(const char *, const char *, int *, int);
void save_item_array(const char *, item *, int);
void read_item_array(const char *, const char *, item *, int);
void save_uchar_array(const char *, unsigned char *, int);
void read_uchar_array(const char *, const char *, unsigned char *, int);
void save_uint16_t_array(const char *, uint16_t *, int);
void read_uint16_t_array(const char *, const char *, uint16_t *, int);
void save_gps_array(const char *, gps *, int);
void read_gps_array(const char *, const char *, gps *, int);
void save_float_array(const char *, float *, int);
void read_float_array(const char *, const char *, float *, int);

/* Hacks */
void save_keybind_t_array(const char *, keybind_t * , int);
void read_keybind_t_array(const char *, const char *, keybind_t *, int);
void save_chatflags_t_array(const char *, chatflags_t * , int);
void read_chatflags_t_array(const char *, const char *, chatflags_t *, int);
void save_cookielist_t_array(const char *, cookielist_t *, int);
void read_cookielist_t_array(const char *, const char *, cookielist_t *, int);
#define save_automap_data_t_array save_automap_data
void save_automap_data(const char*, automap_data_t *, int);
void read_automap_data_t_array(char *, char *, automap_data_t *, int);
void save_bigscrmsg_t_array(const char *, bigscrmsg_t *, int);
void read_bigscrmsg_t_array(const char *, const char *, bigscrmsg_t *, int);
void save_sdl_rect(const char *, SDL_Rect *);
int read_sdl_rect(const char *, const char *, SDL_Rect *);
#define save_list_head_t(X,Y) 
#define read_list_head_t(X,Y,Z)
void save_luacode(const char *, luacode *);
void read_luacode(const char *, const char *, luacode *);

// mission.c 
void quest_browser_interface ( void );
void CompleteMission(const char *);
void AssignMission(const char *);
void GetQuestList ( char* QuestListFilename ) ;
void clear_tux_mission_info ( void ) ;
void CheckIfMissionIsComplete ( void );
void quest_browser_enable_new_diary_entry (const char *, int mis_diary_entry_num );
int GetMissionIndexByName(const char *);

// map.c 
void respawn_level ( int level_num );
void glue_obstacles_to_floor_tiles_for_level ( int level_num );
void ResolveMapLabelOnShip (const char* MapLabel , location* PositionPointer );
void CollectAutomapData ( void ) ;
int smash_obstacle ( float x , float y );
Uint16 GetMapBrick (Level deck, float x, float y);

void CountNumberOfDroidsOnShip ( void );
int LoadShip (char *filename);
int SaveShip(const char *filename);
void GetAnimatedMapTiles ();
int GetCrew (char *shipname);

void AnimateCyclingMapTiles (void);
void MoveLevelDoors ( void ) ;
void WorkLevelGuns ( void ) ;
int IsVisible ( GPS objpos ) ;
void DeleteWaypoint (level *Lev, int num);
void CreateWaypoint (level *Lev, int x, int y);
#define translate_map_point_to_screen_pixel translate_map_point_to_screen_pixel_func
#define translate_map_point_to_screen_pixel_x(X,Y)  ( UserCenter_x + ceilf((X)*iso_floor_tile_width_over_two) - ceilf((Y)*iso_floor_tile_width_over_two) \
		                                                           + ceilf(Me.pos.y*iso_floor_tile_width_over_two) - ceilf(Me.pos.x*iso_floor_tile_width_over_two) )
#define translate_map_point_to_screen_pixel_y(X,Y)  ( UserCenter_y + ceilf((X)*iso_floor_tile_height_over_two) + ceilf((Y)*iso_floor_tile_height_over_two) \
		                                                           - ceilf(Me.pos.x*iso_floor_tile_height_over_two) - ceilf(Me.pos.y*iso_floor_tile_height_over_two) )
void translate_map_point_to_screen_pixel_func( float x_map_pos, float y_map_pos, int * x_res, int * y_res, float zoom_factor);
float translate_pixel_to_map_location ( float axis_x , float axis_y , int give_x ) ;
float translate_pixel_to_zoomed_map_location ( float axis_x , float axis_y , int give_x ) ;

//colldet.c
int WalkablePassFilterCallback(colldet_filter* this, obstacle* obs, int obs_idx);
int FlyablePassFilterCallback(colldet_filter* this, obstacle* obs, int obs_idx);
int VisiblePassFilterCallback(colldet_filter* this, obstacle* obs, int obs_idx);
int ObstacleByIdPassFilterCallback(colldet_filter* this, obstacle* obs, int obs_idx);
colldet_filter WalkablePassFilter;
colldet_filter FlyablePassFilter;
colldet_filter VisiblePassFilter;
colldet_filter ObstacleByIdPassFilter;
int CheckIfWayIsFreeOfDroids (char test_tux, float x1 , float y1 , float x2 , float y2 , int OurLevel , enemy * ExceptedRobot ) ;
int EscapeFromObstacle( float* posX, float* posY, int posZ, colldet_filter* filter );
int SinglePointColldet ( float x , float y , int z, colldet_filter* filter ) ;
int DirectLineColldet( float x1 , float y1 , float x2 , float y2 , int z, colldet_filter* filter );
int normalize_vect ( float, float, float *, float *);

// sound.c  OR nosound.c 
void PlayOnceNeededSoundSample( const char* SoundSampleFileName , const int With_Waiting , const int no_double_catching );
void InitAudio(void);
void SetBGMusicVolume(float);
void SetSoundFXVolume(float);
void SwitchBackgroundMusicTo ( char * filename_raw );
void GotHitSound (void);
void tux_scream_sound (void);
void No_Ammo_Sound ( void );
void Not_Enough_Power_Sound( void );
void Not_Enough_Dist_Sound( void );
void Not_Enough_Mana_Sound( void );
void CrySound (void);
void CantCarrySound (void);
void TransferSound (void);
void MenuItemSelectedSound (void);
void MenuItemDeselectedSound (void);
void MoveMenuPositionSound (void);
void teleport_arrival_sound (void);
void healing_spell_sound ( void );
void application_requirements_not_met_sound ( void );
void Fire_Bullet_Sound (int);
void Mission_Status_Change_Sound (void);
void BounceSound (void);
void DruidBlastSound (void);
void ExterminatorBlastSound (void);
void ThouArtDefeatedSound (void);
void Takeover_Set_Capsule_Sound (void);
void Takeover_Game_Won_Sound (void);
void Takeover_Game_Deadlock_Sound (void);
void Takeover_Game_Lost_Sound (void);
void PlayGreetingSound ( int SoundCode );
void play_enter_attack_run_state_sound ( int SoundCode );
void play_death_sound_for_bot ( enemy* ThisRobot );
void play_item_sound ( int item_type ) ;
void PlayLevelCommentSound ( int levelnum );
void PlayEnemyGotHitSound ( int enemytype );
void BulletReflectedSound (void);
void Play_Spell_ForceToEnergy_Sound( void );
void Play_Spell_DetectItems_Sound( void );
void play_melee_weapon_hit_something_sound ( void );
void play_melee_weapon_missed_sound ( void );
void play_open_chest_sound ( void );
void play_sample_using_WAV_cache( char* SoundSampleFileName , int With_Waiting , int no_double_catching ) ;
void play_sample_using_WAV_cache_v( char* SoundSampleFileName , int With_Waiting , int no_double_catching ,double volume) ;
// items.c
void handle_player_identification_command ( void );
void MoveItem( item* SourceItem , item* DestItem );
void silently_unhold_all_items ( void );
int required_magic_stat_for_next_level_and_item ( int item_type );
int CountItemtypeInInventory( int Itemtype );
void DeleteInventoryItemsOfType( int Itemtype , int amount );
void DeleteOneInventoryItemsOfType( int Itemtype );
void DamageItem( item* CurItem );
void DamageWeapon( item* CurItem );
int GetFreeInventoryIndex( void );
int ItemCanBeDroppedInInv ( int ItemType , int InvPos_x , int InvPos_y );
unsigned long calculate_item_buy_price ( item* BuyItem );
unsigned long calculate_item_repair_price ( item* repair_item );
unsigned long calculate_item_sell_price ( item* BuyItem );
void FillInItemProperties( item* ThisItem , int FullDuration , int multiplicity);
void DamageProtectiveEquipment( void ) ;
void write_full_item_name_into_string ( item* ShowItem , char* full_item_name ); 
void DropItemAt( int ItemType , int level_num , float x , float y , int prefix , int suffix , int multiplicity );
void Quick_ApplyItem( int ItemKey );
int MatchItemWithName ( int type, const char * name );
int GetItemIndexByName ( const char * name );
void ApplyItem( item* CurItem );
int Inv_Pos_Is_Free( int x , int y );
int GetInventoryItemAt ( int x , int y );
item* GetHeldItemPointer( void );
Item FindPointerToPositionCode ( int PositionCode ) ;
int ItemUsageRequirementsMet( item* UseItem , int MakeSound );
int MouseCursorIsInInventoryGrid( int x , int y );
int MouseCursorIsInUserRect( int x , int y );
int MouseCursorIsInInvRect( int x , int y );
int MouseCursorIsInChaRect( int x , int y );
int MouseCursorIsInSkiRect( int x , int y );
int GetHeldItemCode ( void );
int GetInventorySquare_x( int x );
int GetInventorySquare_y( int x );
void DropHeldItemToInventory( void );
int DropHeldItemToTheFloor ( void );
void DropItemToTheFloor ( Item DropItemPointer , float x , float y , int levelnum ) ;
void ShowQuickInventory ( void );
void HandleInventoryScreen ( void );
int AddFloorItemDirectlyToInventory( item* ItemPointer );
void CopyItem( item* SourceItem , item* DestItem , int MakeSound );
void DeleteItem( item* Item );
void DropRandomItem( int level_num , float x , float y , int class , int ForceMagical );
int get_floor_item_index_under_mouse_cursor ( void );
int item_is_currently_equipped( item* Item );
int Get_Prefixes_Data ( char * DataPointer );

// character.c
void DisplayButtons( void );
void UpdateAllCharacterStats ( void );
void ShowCharacterScreen ( void );
void HandleCharacterScreen ( void );

// leveleditor.c
void LevelEditor(void);

// skills.c
void RadialVMXWave ( gps ExpCenter , int SpellCostsMana );
void RadialEMPWave ( gps ExpCenter , int SpellCostsMana );
void RadialFireWave ( gps ExpCenter , int SpellCostsMana );
void ShowSkillsScreen ( void );
void HandleCurrentlyActivatedSkill( void );
int DoSkill( int skill_index, int SpellCost);
void activate_nth_aquired_skill ( int skill_num );
void ImproveSkill(int *skill);
int get_program_index_with_name(const char *);
int calculate_program_heat_cost ( int program_id );

// input.c 
Uint8 * key_state_array;
Uint8 mouse_state_last_frame;
int input_handle( void );
void save_mouse_state(void);
void init_keyboard_input_array(void);

int MouseWheelUpPressed(void);
int MouseWheelDownPressed(void);
int GetMousePos_x(void);
int GetMousePos_y(void);
int MouseRightPressed(void); 
int MouseLeftPressed(void);
int MouseLeftClicked(void);
int MouseRightClicked(void);
int MouseLeftUnclicked(void);
int MouseRightUnclicked(void);
int LeftPressed (void) ;
int RightPressed (void) ;
int UpPressed (void) ;
int DownPressed (void) ;
int SpacePressed (void) ;
int EnterPressed (void) ;
int EscapePressed (void) ;
int LeftCtrlPressed (void) ;
int CtrlPressed (void) ;
int ShiftPressed (void) ;
int AltPressed (void) ;
int APressed (void) ;
int HPressed (void) ;
int MPressed (void) ;
int QPressed (void) ;
int SPressed (void) ;
int WPressed (void) ;
int XPressed (void) ;

int input_key_press(SDLKey, SDLMod);
void keychart(void);
void input_set_default(void);
void input_keyboard_init(void);
int getchar_raw (int *);
void input_get_keybind(char *cmdname, SDLKey *key, SDLMod *mod);
void input_set_keybind( char *keybind, SDLKey key, SDLMod mod);

// menu.c 
void clear_player_inventory_and_stats ( void );
void StoreMenuBackground ( int backup_slot );
void RestoreMenuBackground ( int backup_slot );
int DoMenuSelection( char* InitialText , char *MenuTexts[], int FirstItem , int background_code , void* MenuFont );
int ChatDoMenuSelectionFlagged( char* InitialText , char** MenuTexts, 
				       unsigned char * Chat_Flags, int FirstItem , 
				       int background_code , void* MenuFont , enemy* ChatDroid );
int ChatDoMenuSelection( char **MenuTexts, int FirstItem , void* MenuFont , enemy* ChatDroid );
void StartupMenu (void);
void InitiateMenu( int background_code );
void Cheatmenu (void);
void EscapeMenu (void);
int GetNumberOfTextLinesNeeded ( char* GivenText, SDL_Rect GivenRectangle , float text_stretch );

// misc.c 
#define CURLEVEL() (curShip . AllLevels [ Me . pos . z ])
void print_trace ( int signum );
void implant_backtrace_into_signal_handlers ( void ) ;
void adapt_button_positions_to_screen_resolution( void );
void ErrorMessage ( const char* FunctionName , const char* ProblemDescription, int InformDevelopers , int IsFatal, ... );
void UpdateScreenOverButtonFromList ( int ButtonIndex );
void ShowGenericButtonFromList ( int ButtonIndex );
int MouseCursorIsInRect ( SDL_Rect* our_rect , int x , int y );
int MouseCursorIsOnButton( int ButtonIndex , int x , int y );
void *MyMemmem ( char *haystack, size_t haystacklen, char *needle, size_t needlelen);
char* ReadAndMallocStringFromData ( char* SearchString , const char* StartIndicationString , const char* EndIndicationString );
char* ReadAndMallocStringFromDataOptional ( char* SearchString , const char* StartIndicationString , const char* EndIndicationString, char Terminator );
int CountStringOccurences ( char* SearchString , const char* TargetString ) ;
void ReadValueFromStringWithDefault( char* SearchBeginPointer , const char* ValuePreceedText , const char* FormatString , const char * DefaultValueString, void* TargetValue , char* EndOfSearchSectionPointer );
void ReadValueFromString( char* SearchBeginPointer , const char* ValuePreceedText , const char* FormatString , void* TargetValue , char* EndOfSearchSectionPointer );
char* ReadAndMallocAndTerminateFile( char* filename , const char* File_End_String ) ;
char* LocateStringInData ( char* SearchBeginPointer, const char* SearchTextPointer ) ;
int find_file (const char *fname, const char *datadir, char * File_Path, int silent);
void Pause (void);
void ComputeFPSForThisFrame(void);
void StartTakingTimeForFPSCalculation(void);
int Get_Average_FPS ( void );
float Frame_Time (void);
void Activate_Conservative_Frame_Computation(void);
int MyRandom (int);
void Teleport ( int LNum , float X , float Y , int WithSound ) ;
int SaveGameConfig (void);
int LoadGameConfig (void);
void InsertNewMessage (void);
void Terminate (int);
void ShowDebugInfos (void);
Sint16 ReadSint16 (void * memory);
void endian_swap(char * pdata, size_t dsize, size_t nelements);
uint32_t pot_gte( uint32_t v );
obstacle * give_pointer_to_obstacle_with_label (const char* obstacle_label ); 
int give_level_of_obstacle_with_label (const char* obstacle_label );

// enemy.c 
void robot_group_turn_hostile ( enemy * );
void SetRestOfGroupToState ( Enemy ThisRobot , short NewState );
int MakeSureEnemyIsInsideHisLevel ( Enemy ThisRobot );
void ShuffleEnemys ( int LevelNum );
int CheckEnemyEnemyCollision (enemy *);
void MoveEnemys (void);
void ClearEnemys (void);
void InitEnemy (enemy *);
void animate_enemy ( enemy * our_bot ) ;
void hit_enemy ( enemy * target, float hit, char givexp, short int killertype, char mine);
enemy * enemy_resolve_address ( short int enemy_number, enemy ** enemy_addr );
void enemy_set_reference ( short int * enemy_number, enemy ** enemy_addr, enemy * addr);
void enemy_generate_level_lists (void);
int TeleportToRandomWaypoint(enemy *, level *, char *);

#define BROWSE_ALIVE_BOTS_SAFE(X,Y) list_for_each_entry_safe(X, Y, &alive_bots_head, global_list)
#define BROWSE_ALIVE_BOTS(X) list_for_each_entry(X, &alive_bots_head, global_list)
#define BROWSE_DEAD_BOTS_SAFE(X,Y) list_for_each_entry_safe(X, Y, &dead_bots_head, global_list)
#define BROWSE_DEAD_BOTS(X) list_for_each_entry(X, &dead_bots_head, global_list)
#define BROWSE_LEVEL_BOTS_SAFE(X,Y,L) list_for_each_entry_safe(X,Y, &level_bots_head[(L)], level_list)
#define BROWSE_LEVEL_BOTS(T,L) list_for_each_entry(T, &level_bots_head[(L)], level_list)

// text.c 
void show_backgrounded_label_at_map_position ( char* LabelText , float fill_status , float pos_x , float pos_y , int zoom_is_on );
void show_backgrounded_text_rectangle (const char* text , int x , int y , int w , int h );
char * GetEditableStringInPopupWindow ( int MaxLen , char* PopupWindowTitle , char* DefaultString );
void GiveMouseAlertWindow ( const char* WindowText ) ;
int CutDownStringToMaximalSize ( char* StringToCut , int LengthInPixels );
void SetNewBigScreenMessage(const char* ScreenMessageText );
void DisplayBigScreenMessage( void );
void ChatWithFriendlyDroid( Enemy ChatDroid );
void EnemyHitByBulletText( enemy * );
void EnemyInfluCollisionText ( enemy * );

void SetTextCursor (int x, int y);
void SetLineLength (int);

int DisplayText ( const char *text, int startx, int starty, const SDL_Rect *clip , float text_stretch );
void display_current_chat_protocol ( int background_picture_code , enemy* ChatDroid , int with_update );

void DisplayChar (unsigned char c);
int ScrollText (char *text, int background_code );

int ImprovedCheckLineBreak(char *text, const SDL_Rect *clip , float text_stretch );
char *PreviousLine (char *textstart, char *text);
char *NextLine (char *text);
char *GetString ( int max_len , int background_code , const char* text_for_overhead_promt ) ;
void printf_SDL (SDL_Surface *screen, int x, int y, const char *fmt, ...);

// text_public.c 

void DebugPrintf (int db_level, const char *fmt, ...);
void *MyMalloc (long);
int FS_filelength (FILE *f);
void inflate_stream(FILE *, unsigned char **, int *);

// hud.c 
void GiveItemDescription ( char* ItemDescText , item* CurItem , int ForShop );
void DisplayBanner ( void );
int get_days_of_game_duration ( float current_game_date );
int get_hours_of_game_duration ( float current_game_date );
int get_minutes_of_game_duration ( float current_game_date );
void append_new_game_message (const char* game_message_text );
void display_current_game_message_window ( void );
void toggle_game_config_screen_visibility ( int screen_visible );

// shop.c 
void ShowRescaledItem ( int position , int TuxItemRow , item* ShowItem );
int TryToIntegrateItemIntoInventory ( item* BuyItem , int AmountToBuyAtMost );
int AssemblePointerListForChestShow ( item** ItemPointerListPointer , moderately_finepoint chest_pos );
int AssemblePointerListForItemShow ( item** ItemPointerListPointer , int IncludeWornItems );
void InitTradeWithCharacter( int CharacterCode ) ;

// takeover.c 

int Takeover (enemy *);
void ChooseColor (void);
void PlayGame (void);
void EnemyMovements (void);

int GetTakeoverGraphics (void);
void ShowPlayground (void);
void InventPlayground (void);

void ProcessPlayground (void);
void ProcessDisplayColumn (void);
void ProcessCapsules (void);
void AnimateCurrents (void);

void ClearPlayground (void);
int IsActive (int color, int row);

// BFont.c
Uint32 FdGetPixel32 (SDL_Surface * Surface, Sint32 X, Sint32 Y);
Uint32 FdGetPixel24 (SDL_Surface * Surface, Sint32 X, Sint32 Y);
Uint16 FdGetPixel16 (SDL_Surface * Surface, Sint32 X, Sint32 Y);
Uint8 FdGetPixel8 (SDL_Surface * Surface, Sint32 X, Sint32 Y);
void PutPixel32 (SDL_Surface * surface, int x, int y, Uint32 pixel);
void PutPixel24 (SDL_Surface * surface, int x, int y, Uint32 pixel);
void PutPixel16 (SDL_Surface * surface, int x, int y, Uint32 pixel);
void PutPixel8 (SDL_Surface * surface, int x, int y, Uint32 pixel);
Uint32 FdGetPixel (SDL_Surface * Surface, Sint32 X, Sint32 Y);
void PutPixel (SDL_Surface * surface, int x, int y, Uint32 pixel);

void list_add(list_head_t *new, list_head_t * head);
void list_add_tail(list_head_t *new, list_head_t * head);
void list_del(list_head_t *entry);
void list_del_init(list_head_t *entry);
void list_move(list_head_t *list, list_head_t *head);
void list_move_tail(list_head_t *list, list_head_t *head);
int list_empty(const list_head_t *head);
void list_splice(list_head_t *list, list_head_t *head);
void list_splice_init(list_head_t *list, list_head_t *head);

int load_texture_atlas ( const char *, const char *, char *filenames[], iso_image *, int ); 

// chat.c
void PlantCookie(const char *);
void DeleteCookie (const char *);
int ResolveDialogSectionToChatFlagsIndex (const char* SectionName );
void run_subdialog(const char *);
void GiveSubtitleNSample(const char*, const char*, enemy* , int);

#endif
