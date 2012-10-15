/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2010 Arthur Huillet 
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
#include "lua.h"

// main.c 
void Game(void);

// automap.c
void display_automap(void);
void toggle_automap(void);
void CollectAutomapData(void);
void update_obstacle_automap(int z, obstacle *our_obstacle);

// init.c
void ResetGameConfigToDefaultValues(void);
void clear_out_arrays_for_fresh_game(void);
void next_startup_percentage(int Percentage);
void ParseCommandLine(int argc, char *const argv[]);
void ClearAutomapData(void);
void InitFreedroid(int, char **);
void PrepareStartOfNewCharacter(char *startpos);
void prepare_level_editor(void);
void ThouArtDefeated(void);
void ThouHastWon(void);
void PlayATitleFile(char *Filename);

// event.c
void GetEventTriggers(const char *EventsAndEventTriggersFilename);
void trigger_position_events(void);
void event_level_changed(int past_lvl, int cur_lvl);
void event_enemy_died(enemy *dead);
const char *teleporter_square_below_mouse_cursor(void);
void event_modify_trigger_state(const char *name, int state);
struct event_trigger * visible_event_at_location(int x, int y, int z);

// lua.c
void init_lua(void);
lua_State *get_lua_state(enum lua_target target);
lua_State *load_lua_coroutine(enum lua_target, const char *);
int resume_lua_coroutine(lua_State*);
void run_lua(enum lua_target, const char *code);
void run_lua_file(enum lua_target, const char *);
void reset_lua_state(void);
void write_lua_variables(struct auto_string *savestruct_autostr);

// luaconfig.c
void init_luaconfig(void);

// influ.c 
float calc_distance(float pos1_x, float pos1_y, float pos2_x, float pos2_y);
float vect_len(moderately_finepoint our_vector);
enemy *GetLivingDroidBelowMouseCursor(void);
void tux_wants_to_attack_now(int use_mouse_cursor_for_targeting);
int PerformTuxAttackRaw(int use_mouse_cursor_for_targeting);
void TuxReloadWeapon(void);
void correct_tux_position_according_to_jump(void);
void InitInfluPositionHistory(void);
float GetInfluPositionHistoryX(int Index);
float GetInfluPositionHistoryY(int Index);
float GetInfluPositionHistoryZ(int Index);
void FillInDefaultBulletStruct(bullet * CurBullet, int bullet_image_type, short int weapon_item_type);
void FireTuxRangedWeaponRaw(short int weapon_item_type, int bullet_image_type, bullet *bullet_parameters, moderately_finepoint target_location);
void move_tux(void);
void hit_tux(float);
void animate_tux(void);
void check_tux_enemy_collision(void);
void start_tux_death_explosions(void);
void init_tux(void);
void set_movement_with_keys(int move_x, int move_y);
void do_death_menu(void);

// action.c
void chest_open_action(level *chest_lvl, int chest_index);
void barrel_action(level *barrel_lvl, int barrel_index);
void terminal_connect_action(level *, int);
void sign_read_action(level *sign_lvl, int sign_index);
int clickable_obstacle_below_mouse_cursor(level **obst_lvl);
int check_for_items_to_pickup(level *item_lvl, int item_index);
action_fptr get_action_by_name(const char *action_name);

// pathfinder.c
int set_up_intermediate_course_between_positions(gps * curpos, moderately_finepoint * move_target, moderately_finepoint * waypoints,
						 int maxwp, pathfinder_context * ctx);
void clear_out_intermediate_points(gps *, moderately_finepoint *, int);

// bullet.c 
void RotateVectorByAngle(moderately_finepoint * vector, float rot_angle);
void MoveBullets(void);
void DoMeleeDamage(void);
void DeleteBullet(int num, int StartBlast);
void StartBlast(float x, float y, int level, int type, int dmg, int faction, char *sound_name);
void animate_blasts(void);
void DeleteBlast(int num);
int get_blast_type_by_name(const char *name);
void MoveActiveSpells(void);
void DeleteSpell(int num);
void clear_active_spells(void);
void clear_active_bullets(void);
void CheckBulletCollisions(int num);
void CheckBlastCollisions(int num);
int find_free_bullet_index(void);
int find_free_melee_shot_index(void);
void delete_melee_shot(melee_shot *);
int GetBulletByName(const char *bullet_name);

// view.c 
void get_floor_boundaries(int, int *, int *, int *, int *);
void gps_transform_map_init(void);
void update_virtual_position(gps * target_pos, gps * source_pos, int level_num);
int resolve_virtual_position(gps * actual_pos, gps * virtual_pos);
int pos_inside_level(float x, float y, level * lvl);
int pos_near_level(float x, float y, level * lvl, float dist);
void ShowPosition(void);
void set_up_ordered_blitting_list(int mask);
void blit_preput_objects_according_to_blitting_list(int mask);
void blit_nonpreput_objects_according_to_blitting_list(int mask);
void draw_grid_on_the_floor(int mask);
void blit_leveleditor_point(int x, int y);
void update_item_text_slot_positions(void);
void AssembleCombatPicture(int);
void blit_tux(int x, int y);
struct tux_part_render_data *tux_get_part_render_data(char *part_name);
void tux_rendering_load_specs(const char *config_filename);
void PutBullet(int Bullet_number, int mask);
void PutItem(item *CurItem, int ItemNumber, int mask, int put_thrown_items_flag, int highlight_item);
void PutBlast(int);
void PutEnemy(enemy * e, int x, int y, int mask, int highlight);
void PutMouseMoveCursor(void);
int set_rotation_index_for_this_robot(enemy * ThisRobot);
int set_rotation_model_for_this_robot(enemy * ThisRobot);
int level_is_visible(int level_num);
void get_visible_levels(void);
void reset_visible_levels(void);
void PutIndividuallyShapedDroidBody(enemy *, SDL_Rect, int, int);
void object_vtx_color(void *, float *, float *, float *);
int get_motion_class_id_by_name(char *);
char *get_motion_class_name_by_id(int);
int get_motion_class_id(void);
void show_inventory_screen(void);

#define next_valid_visible_level(pos, start) ({ \
	pos = start; \
	while (&pos->node != &visible_level_list && !pos->valid) \
		pos = list_entry(pos->node.next, typeof(*pos), node); \
	})

#define next_valid_nearby_visible_level(pos, start, d) ({ \
	pos = start; \
	while (&pos->node != &visible_level_list && (!pos->valid || pos->boundary_squared_dist>=d)) \
		pos = list_entry(pos->node.next, typeof(*pos), node); \
	})

// This macro will loop on each valid visible levels.
// It is based on the list_for_each_entry_safe() macro.
// However, some entries in the visible_level_list are marked as not "valid",
// and such levels have to be ignored during the call to the macro.
// So, in the 'update part' of the 'for' statement we have to advance the ptr 
// *until* a valid entry is found. Thus, a loop (that is a compound-statement) is 
// needed.
// However, the 'update part' of a 'for' statement has to be an 'expression'.
// Thanks to gcc (this is not part of the C99 standard), it is possible to 
// transform a compound statement into an expression, with the 
// "({compound-statement;})" construct.
// To ease readiness, the loop is defined in a next_valid_visible_level() macro.
// This loop is also used in the 'initialization part' of the 'for' statement,
// to reach the first valid entry.
#define BROWSE_VISIBLE_LEVELS(pos, n) \
	for (next_valid_visible_level(pos, list_entry(visible_level_list.next, typeof(*pos), node)), \
		n = list_entry(pos->node.next, typeof(*pos), node) ; \
		&pos->node != (&visible_level_list) ; \
		next_valid_visible_level(pos, n), n = list_entry(pos->node.next, typeof(*pos), node))

// This macro will loop on each valid visible levels, if one of their boundary is
// at a distance less than 'd'.
// It uses the same trick than the BROWSE_VISIBLE_LEVELS() macro.
#define BROWSE_NEARBY_VISIBLE_LEVELS(pos, n, d) \
	for (next_valid_nearby_visible_level(pos, list_entry(visible_level_list.next, typeof(*pos), node), d), \
		n = list_entry(pos->node.next, typeof(*pos), node) ; \
		&pos->node != (&visible_level_list) ; \
		next_valid_nearby_visible_level(pos, n, d), n = list_entry(pos->node.next, typeof(*pos), node))

// light.c 
void LightRadiusInit(void);
void LightRadiusClean(void);
int get_light_strength_screen(int x, int y);
int get_light_strength_cell(uint32_t x, uint32_t y);
void update_light_list(void);
void blit_light_radius(void);

// open_gl.c 
int our_SDL_flip_wrapper(void);
int our_SDL_blit_surface_wrapper(SDL_Surface * src, SDL_Rect * srcrect, SDL_Surface * dst, SDL_Rect * dstrect);
void our_SDL_update_rect_wrapper(SDL_Surface * screen, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
int blit_quad(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, Uint32 color);
void drawIsoEnergyBar(int x, int y, int z, int w, int d, int length, float fill, myColor * c1, myColor * c2);

SDL_Surface *our_IMG_load_wrapper(const char *file);
void flip_image_vertically(SDL_Surface * tmp1);
void make_texture_out_of_surface(struct image *our_image);
void make_texture_out_of_prepadded_image(struct image *our_image);
void blit_open_gl_stretched_texture_light_radius(int decay_x, int decay_y);
void gl_draw_rectangle(SDL_Rect *, int, int, int, int);
void show_character_screen_background(void);
int safely_initialize_our_default_open_gl_parameters(void);
void blit_background(const char *background);
void open_gl_check_error_status(const char *name_of_calling_function);
void set_gl_clip_rect(const SDL_Rect *clip);
void unset_gl_clip_rect(void);

// blocks.c 
int wall_orientation(int wall);
void iso_load_bullet_surfaces(void);
void Load_Mouse_Move_Cursor_Surfaces(void);
void LoadAndPrepareEnemyRotationModelNr(int RotationModel);
void free_enemy_graphics(void);
void Load_Enemy_Surfaces(void);
void Load_Tux_Surfaces(void);
void Load_Bullet_Surfaces(void);
void Load_Blast_Surfaces(void);
void load_floor_tiles(void);
void free_floor_tiles(void);
struct image *get_obstacle_image(int, int);
struct image *get_obstacle_shadow_image(int, int);
struct image *get_map_label_image(void);
struct image *get_droid_portrait_image(int);
void load_all_obstacles(int with_startup_bar);
void free_obstacle_graphics(void);
struct image *get_item_shop_image(int type);
struct image *get_item_ingame_image(int type);
struct image *get_item_inventory_image(int type);
void load_all_items(void);
void free_item_graphics(void);
void load_tux_graphics(int motion_class, int tux_part_group, const char *part_string);
void reload_tux_graphics(void);
void get_offset_for_iso_image_from_file_and_path(char *fpath, struct image * our_iso_image);

// graphics.c 
void blit_mouse_cursor(void);
void fade_out_screen(void);
void fade_in_screen(void);
void InitPictures(void);
void init_timer(void);
void InitVideo(void);
void InitOurBFonts(void);
void HighlightRectangle(SDL_Surface * Surface, SDL_Rect Area);
void ShadowingRectangle(SDL_Surface * Surface, SDL_Rect Area);
int do_graphical_number_selection_in_range(int lower_range, int upper_range, int default_value, int unit_price);
void draw_line(float x1, float y1, float x2, float y2, uint8_t r, uint8_t g, uint8_t b, int glwidth);
void draw_line_on_map(float x1, float y1, float x2, float y2, uint8_t r, uint8_t g, uint8_t b, int width);
void sdl_draw_rectangle(SDL_Rect *rect, int r, int g, int b, int a);
SDL_Surface *sdl_create_colored_surface(SDL_Surface *surf, float r, float g, float b, float a, int highlight);
void draw_rectangle(SDL_Rect *rect, int r, int g, int b, int a);
void draw_quad(const int16_t vx[4], const int16_t vy[4], int r, int g, int b, int a);
uint32_t sdl_get_pixel(SDL_Surface *surf, int x, int y);
void sdl_put_pixel(SDL_Surface *surf, int x, int y, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
void save_screenshot(const char *filename, int width);
void reload_graphics(void);
void clear_screen(void);

// saveloadgame.c 
int find_saved_games(struct dirent ***);
void LoadAndShowThumbnail(char *CoreFilename);
int SaveGame(void);
int LoadBackupGame(void);
int LoadGame(void);
int DeleteGame(void);
void LoadAndShowStats(char *CoreFilename);

// mission.c 
void CompleteMission(const char *);
void AssignMission(const char *);
void GetQuestList(char *QuestListFilename);
void clear_tux_mission_info(void);
void CheckIfMissionIsComplete(void);
void mission_diary_add(const char *, const char *);
int GetMissionIndexByName(const char *);

// quest_browser_ui.c
void toggle_quest_browser(void);
struct widget_group *create_quest_browser(void);

// map.c 
void respawn_level(int level_num);
void ResolveMapLabelOnShip(const char *MapLabel, location * PositionPointer);
int smash_obstacle(float x, float y, int level);
Uint16 get_map_brick(level *, float, float, int);
void CountNumberOfDroidsOnShip(void);
int LoadShip(char *filename, int);
int SaveShip(const char *filename, int reset_random_levels, int);
int save_special_forces(const char *filename);
void get_animated_obstacle_lists(struct visible_level *vis_lvl);
void dirty_animated_obstacle_lists(int lvl_num);
void clear_animated_obstacle_lists(struct visible_level *vis_lvl);
int GetCrew(char *shipname);
moderately_finepoint translate_point_to_map_location(float axis_x, float axis_y, int zoom_is_on);

void animate_obstacles(void);
int IsVisible(gps *objpos);
#define translate_map_point_to_screen_pixel translate_map_point_to_screen_pixel_func
#define translate_map_point_to_screen_pixel_x(X,Y)  ( UserCenter_x + ceilf((X)*iso_floor_tile_width_over_two) - ceilf((Y)*iso_floor_tile_width_over_two) \
		                                                           + ceilf(Me.pos.y*iso_floor_tile_width_over_two) - ceilf(Me.pos.x*iso_floor_tile_width_over_two) )
#define translate_map_point_to_screen_pixel_y(X,Y)  ( UserCenter_y + ceilf((X)*iso_floor_tile_height_over_two) + ceilf((Y)*iso_floor_tile_height_over_two) \
		                                                           - ceilf(Me.pos.x*iso_floor_tile_height_over_two) - ceilf(Me.pos.y*iso_floor_tile_height_over_two) )
void translate_map_point_to_screen_pixel_func(float x_map_pos, float y_map_pos, int *x_res, int *y_res);
float translate_pixel_to_map_location(float axis_x, float axis_y, int give_x);
float translate_pixel_to_zoomed_map_location(float axis_x, float axis_y, int give_x);

//floor_tiles.c
int next_glue_timestamp(void);
void free_glued_obstacles(level *lvl);
struct image *get_floor_tile_image(int floor_value);

//colldet.c
int WalkablePassFilterCallback(colldet_filter * this, obstacle * obs, int obs_idx);
int FlyablePassFilterCallback(colldet_filter * this, obstacle * obs, int obs_idx);
int VisiblePassFilterCallback(colldet_filter * this, obstacle * obs, int obs_idx);
int ObstacleByIdPassFilterCallback(colldet_filter * this, obstacle * obs, int obs_idx);
colldet_filter WalkablePassFilter;
colldet_filter WalkableWithMarginPassFilter;
colldet_filter FlyablePassFilter;
colldet_filter VisiblePassFilter;
colldet_filter ObstacleByIdPassFilter;
colldet_filter WalkableExceptIdPassFilter;
colldet_filter FlyableExceptIdPassFilter;
int location_free_of_droids(float x, float y, int levelnum, freeway_context *ctx);
int way_free_of_droids(float x1, float y1, float x2, float y2, int OurLevel, freeway_context * ctx);
int EscapeFromObstacle(float *posX, float *posY, int posZ, colldet_filter * filter);
int SinglePointColldet(float x, float y, int z, colldet_filter * filter);
int DirectLineColldet(float x1, float y1, float x2, float y2, int z, colldet_filter * filter);
int normalize_vect(float, float, float *, float *);

// sound.c
void play_sound(const char *);
void InitAudio(void);
void SetBGMusicVolume(float);
void SetSoundFXVolume(float);
void SwitchBackgroundMusicTo(char *filename_raw);

// sound_effects.c
void tux_scream_sound(void);
void No_Ammo_Sound(void);
void Not_Enough_Power_Sound(void);
void Not_Enough_Dist_Sound(void);
void CantCarrySound(void);
void MenuItemSelectedSound(void);
void MenuItemDeselectedSound(void);
void MoveMenuPositionSound(void);
void teleport_arrival_sound(void);
void fire_bullet_sound(int BulletType, struct gps *shooter_pos);
void play_blast_sound(char *blast_sound, struct gps *blast_pos);
void Mission_Status_Change_Sound(void);
void droid_blast_sound(struct gps *blast_pos);
void exterminator_blast_sound(struct gps *blast_pos);
void ThouArtDefeatedSound(void);
void Takeover_Set_Capsule_Sound(void);
void Takeover_Game_Won_Sound(void);
void Takeover_Game_Deadlock_Sound(void);
void Takeover_Game_Lost_Sound(void);
void play_greeting_sound(enemy *ThisRobot);
void play_enter_attack_run_state_sound(enemy *ThisRobot);
void play_death_sound_for_bot(enemy * ThisRobot);
void play_item_sound(int item_type, struct gps *item_pos);
void BulletReflectedSound(void);
void Play_Spell_ForceToEnergy_Sound(void);
void play_melee_weapon_hit_something_sound(void);
void play_melee_weapon_missed_sound(struct gps *attacker_pos);
void play_open_chest_sound(void);
void play_sound_cached(const char *SoundSampleFileName);
void play_sound_cached_v(const char *SoundSampleFileName, double volume);
void play_sound_at_position(const char *filename, struct gps *listener, struct gps *emitter);

// items.c
void init_item(item *);
item create_item_with_name(const char *item_name, int full_durability, int multiplicity);
void equip_item(item *new_item);
item *get_equipped_item_in_slot_for(int item_type);
void MoveItem(item * SourceItem, item * DestItem);
void silently_unhold_all_items(void);
int CountItemtypeInInventory(int Itemtype);
void DeleteInventoryItemsOfType(int Itemtype, int amount);
void DeleteOneInventoryItemsOfType(int Itemtype);
void DamageItem(item * CurItem);
void DamageWeapon(item * CurItem);
int GetFreeInventoryIndex(void);
int ItemCanBeDroppedInInv(int ItemType, int InvPos_x, int InvPos_y);
unsigned long calculate_item_buy_price(item * BuyItem);
unsigned long calculate_item_repair_price(item * repair_item);
unsigned long calculate_item_sell_price(item * BuyItem);
void FillInItemProperties(item *it, int full_durability, int multiplicity);
void DamageProtectiveEquipment(void);
void append_item_name(item *ShowItem, struct auto_string *str);
item *DropItemAt(int, int, float, float, int);
void Quick_ApplyItem(int ItemKey);
int MatchItemWithName(int type, const char *name);
int GetItemIndexByName(const char *name);
void ApplyItem(item * CurItem);
int Inv_Pos_Is_Free(int x, int y);
int GetInventoryItemAt(int x, int y);
item *get_held_item(void);
int ItemUsageRequirementsMet(item * UseItem, int MakeSound);
int MouseCursorIsInInventoryGrid(int x, int y);
int MouseCursorIsInUserRect(int x, int y);
int MouseCursorIsInInvRect(int x, int y);
int MouseCursorIsInChaRect(int x, int y);
int MouseCursorIsInSkiRect(int x, int y);
int GetInventorySquare_x(int x);
int GetInventorySquare_y(int x);
void DropHeldItemToInventory(void);
item *drop_item(item *, float, float, int);
void HandleInventoryScreen(void);
int try_give_item(item *it);
int give_item(item *);
void CopyItem(item * SourceItem, item * DestItem);
void DeleteItem(item * Item);
void DropRandomItem(int level_num, float x, float y, int class, int ForceMagical);
int get_floor_item_index_under_mouse_cursor(level **item_lvl);
int item_is_currently_equipped(item * Item);
const char *ammo_desc_for_weapon(int);
enum slot_type get_slot_type_by_name(char *name);

// item_upgrades.c
void create_upgrade_socket(item *, int, const char *);
void delete_upgrade_sockets(item *);
void copy_upgrade_sockets(item *, item *);
int item_can_be_customized(item *);
int item_can_be_installed_to_socket(item *, item *, int);
struct addon_spec *get_addon_spec(int);
struct dynarray *get_addon_specs(void);
void add_addon_spec(struct addon_spec *);
void get_item_bonus_string(item *, const char *, struct auto_string *);
void print_addon_description(struct addon_spec *, struct auto_string *);
void calculate_item_bonuses(item *);
int count_used_sockets(item *);
int item_upgrade_ui_visible(void);

// character.c
void display_buttons(void);
int get_experience_required(int);
void UpdateAllCharacterStats(void);
void ShowCharacterScreen(void);
void HandleCharacterScreen(void);
void update_all_primary_stats(void);

// armor.c
void update_player_armor_class(void);
float get_player_damage_factor(void);

// leveleditor.c
void LevelEditor(void);

// skills.c
void RadialVMXWave(gps ExpCenter, int SpellCostsMana);
void RadialEMPWave(gps ExpCenter, int SpellCostsMana);
void RadialFireWave(gps ExpCenter, int SpellCostsMana);
void ShowSkillsScreen(void);
void HandleCurrentlyActivatedSkill(void);
int DoSkill(int skill_index, int SpellCost);
void do_radial_skill(int skill_index, int pos_x, int pos_y, int from_tux);
void activate_nth_skill(int skill_num);
void set_nth_quick_skill(int quick_skill);
void ImproveSkill(int *skill);
int improve_program(int);
void downgrade_program(int program_id);
int get_program_index_with_name(const char *);
int calculate_program_heat_cost(int program_id);
int CursorIsOnWhichSkillButton(int x, int y);
void load_skill_icon_if_needed(spell_skill_spec *spec);

// input.c 
Uint8 *key_state_array;
Uint8 mouse_state_last_frame;
int input_handle(void);
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
int LeftPressed(void);
int RightPressed(void);
int UpPressed(void);
int DownPressed(void);
int SpacePressed(void);
int EnterPressed(void);
int EscapePressed(void);
int LeftCtrlPressed(void);
int CtrlPressed(void);
int ShiftPressed(void);
int APressed(void);
int QPressed(void);
int XPressed(void);

int input_key_press(SDL_Event *);
void keychart(void);
void input_set_default(void);
void input_keyboard_init(void);
void input_hold_keyboard(void);
void input_release_keyboard(void);
int getchar_raw(int *);
void input_get_keybind(const char *cmdname, SDLKey * key, SDLMod * mod);
void input_get_keybind_string(const char *cmd, char *out);
void input_set_keybind(char *keybind, SDLKey key, SDLMod mod);

// menu.c 
void clear_player_inventory_and_stats(void);
void StoreMenuBackground(int backup_slot);
void RestoreMenuBackground(int backup_slot);
int DoMenuSelection(char *InitialText, char *MenuTexts[], int FirstItem, const char *background_name, void *MenuFont);
int chat_do_menu_selection(char **MenuTexts, enemy *chat_droid);
void StartupMenu(void);
void InitiateMenu(const char *background_name);
void Cheatmenu(void);
void EscapeMenu(void);
int load_named_game(const char *name);

// misc.c 
#define CURLEVEL() (curShip.AllLevels[Me.pos.z])
void print_trace(int signum);
void adapt_button_positions_to_screen_resolution(void);
#ifdef __GNUC__
#define PRINTF_FMT_ATTRIBUTE(fmt,firstarg) __attribute__ ((format(printf,fmt,firstarg)));
#else
#define PRINTF_FMT_ATTRIBUTE(fmt,firstarg)
#endif
void ErrorMessage(const char *FunctionName, const char *ProblemDescription, int InformDevelopers, int IsFatal, ...) PRINTF_FMT_ATTRIBUTE(2,5);
void ShowGenericButtonFromList(int ButtonIndex);
int mouse_cursor_is_on_that_image(float pos_x, float pos_y, struct image *our_iso_image);
int MouseCursorIsInRect(const SDL_Rect *, int, int);
int MouseCursorIsOnButton(int ButtonIndex, int x, int y);
void *MyMemmem(char *haystack, size_t haystacklen, char *needle, size_t needlelen);
int find_file(const char *fname, const char *datadir, char *File_Path, int silent);
void Pause(void);
void ComputeFPSForThisFrame(void);
void StartTakingTimeForFPSCalculation(void);
int Get_Average_FPS(void);
float Frame_Time(void);
void Activate_Conservative_Frame_Computation(void);
void update_frames_displayed(void);
int MyRandom(int);
void Teleport(int LNum, float X, float Y, int WithSound, int with_animation_reset);
void teleport_to_level_center(int);
int SaveGameConfig(void);
int LoadGameConfig(void);
void InsertNewMessage(void);
void Terminate(int, int);
void ShowDebugInfos(void);
uint32_t pot_gte(uint32_t v);
obstacle *give_pointer_to_obstacle_with_label(const char *, int *);
int level_exists(int);
void freeze_world(void);
void unfreeze_world(void);
int world_frozen(void);

// enemy.c 
void SetRestOfGroupToState(Enemy ThisRobot, short NewState);
int MakeSureEnemyIsInsideHisLevel(Enemy ThisRobot);
int CheckEnemyEnemyCollision(enemy *);
void move_enemies(void);
void clear_enemies(void);
void enemy_reset(enemy *this_bot);
void enemy_reset_fabric(void);
enemy *enemy_new(int type);
void enemy_free(enemy *);
void enemy_insert_into_lists(enemy *this_enemy, int is_living);
void animate_enemy(enemy * our_bot);
void hit_enemy(enemy * target, float hit, char givexp, short int killertype, char mine);
enemy *enemy_resolve_address(short int enemy_number, enemy ** enemy_addr);
void enemy_set_reference(short int *enemy_number, enemy ** enemy_addr, enemy * addr);
void enemy_generate_level_lists(void);
int teleport_to_closest_waypoint(enemy *ThisRobot);
int teleport_to_random_waypoint(enemy *, level *, char *);
void teleport_enemy(enemy *, int, float, float);
int get_droid_type(const char *);
enemy *get_enemy_with_dialog(const char *dialog);

#define BROWSE_ALIVE_BOTS_SAFE(X,Y) list_for_each_entry_safe(X, Y, &alive_bots_head, global_list)
#define BROWSE_ALIVE_BOTS(X) list_for_each_entry(X, &alive_bots_head, global_list)
#define BROWSE_DEAD_BOTS_SAFE(X,Y) list_for_each_entry_safe(X, Y, &dead_bots_head, global_list)
#define BROWSE_DEAD_BOTS(X) list_for_each_entry(X, &dead_bots_head, global_list)
#define BROWSE_LEVEL_BOTS_SAFE(X,Y,L) list_for_each_entry_safe(X,Y, &level_bots_head[(L)], level_list)
#define BROWSE_LEVEL_BOTS(T,L) list_for_each_entry(T, &level_bots_head[(L)], level_list)

// text.c
int get_lines_needed(const char *text, SDL_Rect t_rect, float line_height_factor);
void show_backgrounded_label_at_map_position(char *LabelText, float fill_status, float pos_x, float pos_y, int zoom_is_on);
char *GetEditableStringInPopupWindow(int MaxLen, const char *PopupWindowTitle, const char *DefaultString);
int show_backgrounded_text_rectangle(const char *, struct BFont_Info *, int, int, int, int);
void alert_window(const char *text, ...) PRINTF_FMT_ATTRIBUTE(1,2);
int CutDownStringToMaximalSize(char *StringToCut, int LengthInPixels);
void SetNewBigScreenMessage(const char *ScreenMessageText);
void DisplayBigScreenMessage(void);
void ChatWithFriendlyDroid(Enemy ChatDroid);
void EnemyInfluCollisionText(enemy *);

int display_text_using_line_height(const char *, int, int, const SDL_Rect*, float);
int display_text(const char *, int, int, const SDL_Rect*);
void show_chat_log(enemy *);

int ScrollText(char *text, const char *background_name);

int ImprovedCheckLineBreak(char *, const SDL_Rect*, float);
char *PreviousLine(char *textstart, char *text);
char *NextLine(char *text);
char *get_string(int max_len, const char *background_name, const char *text_for_overhead_promt);
void printf_SDL(SDL_Surface * screen, int x, int y, const char *fmt, ...) PRINTF_FMT_ATTRIBUTE(4,5);
int longest_line_width(char *text);

// text_public.c 
char *ReadAndMallocStringFromData(char *SearchString, const char *StartIndicationString, const char *EndIndicationString);
char *ReadAndMallocStringFromDataOptional(char *SearchString, const char *StartIndicationString, const char *EndIndicationString);
int CountStringOccurences(char *SearchString, const char *TargetString);
void ReadValueFromStringWithDefault(char *SearchBeginPointer, const char *ValuePreceedText, const char *FormatString,
				    const char *DefaultValueString, void *TargetValue, char *EndOfSearchSectionPointer);
void ReadValueFromString(char *SearchBeginPointer, const char *ValuePreceedText, const char *FormatString, void *TargetValue,
			 char *EndOfSearchSectionPointer);
int ReadRangeFromString(char *SearchString, const char *StartIndicationString, const char *EndIndicationString, int *min, int *max, int default_val);
char *ReadAndMallocAndTerminateFile(const char *filename, const char *File_End_String);
char *LocateStringInData(char *SearchBeginPointer, const char *SearchTextPointer);
void DebugPrintf(int db_level, const char *fmt, ...) PRINTF_FMT_ATTRIBUTE(2,3);
void *MyMalloc(long);
int FS_filelength(FILE * f);
int inflate_stream(FILE *, unsigned char **, int *);
int deflate_to_stream(unsigned char *, int, FILE *);

// hud.c 
void append_item_description(struct auto_string *str, item *);
void ShowCurrentSkill(void);
void ShowCurrentWeapon(void);
void ShowCurrentHealthAndForceLevel(void);
void show_texts_and_banner(void);
int get_days_of_game_duration(float current_game_date);
int get_hours_of_game_duration(float current_game_date);
int get_minutes_of_game_duration(float current_game_date);
void append_new_game_message(const char *fmt, ...) PRINTF_FMT_ATTRIBUTE(1,2);
void init_message_log(void);
void toggle_game_config_screen_visibility(int screen_visible);
int get_current_fps(void);
void display_tooltip(const char *, int, SDL_Rect);
void blit_vertical_status_bar(float, float, Uint32, Uint32, int, int, int, int);

// game_ui.c
struct widget_group *get_game_ui(void);

// item_upgrades_ui.c
int append_item_upgrade_ui_tooltip(const point *, struct auto_string *str);
void show_item_upgrade_ui(void);
void item_upgrade_ui(void);

// addon_crafting_ui.c
int cursor_is_on_addon_crafting_ui(const point *);
void show_addon_crafting_ui(void);
void addon_crafting_ui(void);
int addon_crafting_ui_visible(void);

// shop.c 
void ShowItemPicture(int, int, int);
void ShowRescaledItem(int position, int TuxItemRow, item * ShowItem);
int AssemblePointerListForItemShow(item ** ItemPointerListPointer, int IncludeWornItems);
void InitTradeWithCharacter(struct npc *);
int GreatShopInterface(int, item * ShowPointerList[MAX_ITEMS_IN_INVENTORY], int, item * TuxItemsList[MAX_ITEMS_IN_INVENTORY],
		       shop_decision *);

// takeover.c 

int droid_takeover(enemy *, float *);
int do_takeover(int, int, int, enemy *);

void InventPlayground(void);

void ProcessPlayground(void);
void ProcessDisplayColumn(void);
void ProcessCapsules(void);
void AnimateCurrents(void);

void ClearPlayground(void);
int IsActive(int color, int row);

void list_add(list_head_t * new, list_head_t * head);
void list_add_tail(list_head_t * new, list_head_t * head);
void list_del(list_head_t * entry);
void list_del_init(list_head_t * entry);
void list_move(list_head_t * list, list_head_t * head);
void list_move_tail(list_head_t * list, list_head_t * head);
int list_empty(const list_head_t * head);
void list_splice(list_head_t * list, list_head_t * head);
void list_splice_init(list_head_t * list, list_head_t * head);

int load_texture_atlas(const char *, const char *, struct image *(*get_storage_for_key)(const char *key));

// chat.c
void chat_push_topic(const char *topic);
void chat_pop_topic(void);
int stack_dialog(enemy *);
int stack_subdialog(const char *);
void chat_add_response(const char *);
int validate_dialogs(void);
struct chat_context *chat_get_current_context();

// leveleditor_input.c
void leveleditor_process_input(void);
void leveleditor_input_mouse_motion(SDL_Event *);
void leveleditor_input_mouse_button(SDL_Event *);
void leveleditor_input_keybevent(SDL_Event *);

// mapgen/mapgen.c
int generate_dungeon(int, int, int, int);

// mapgen/fd_hooks.c
void set_dungeon_output(level *);

// string.c
struct auto_string *alloc_autostr(int);
void free_autostr(struct auto_string *);
int autostr_printf(struct auto_string *, const char *, ...) PRINTF_FMT_ATTRIBUTE(2,3);
int autostr_vappend(struct auto_string *str, const char *fmt, va_list args);
int autostr_append(struct auto_string *, const char *, ...) PRINTF_FMT_ATTRIBUTE(2,3);

// dynarray.c
struct dynarray *dynarray_alloc(int, size_t);
void dynarray_init(struct dynarray *, int, size_t);
void dynarray_resize(struct dynarray *, int, size_t);
void dynarray_free(struct dynarray *);
void dynarray_add(struct dynarray *, void *, size_t);
void dynarray_del(struct dynarray *, int, size_t);
void *dynarray_member(struct dynarray *, int, size_t);

// animate.c
void animation_timeline_reset(void);
void animation_timeline_advance(void);
int animate_door(level* obstacle_lvl, int obstacle_idx);
int animate_autogun(level* obstacle_lvl, int obstacle_idx);
int animate_obstacle(level *obstacle_lvl, int obstacle_idx);
animation_fptr get_animation_by_name(const char *animation_name);

// benchmark.c
int benchmark(void);

// npc.c
struct npc *npc_get(const char *);
void npc_insert(struct npc *);
void npc_add(const char *);
int npc_add_shoplist(const char *, const char *, int);
void init_npcs(void);
void clear_npcs(void);
item_dynarray *npc_get_inventory(struct npc *);
void npc_inventory_delete_item(struct npc *, int);

// faction.c
enum faction_id get_faction_id(const char *);
const char *get_faction_from_id(int);
void set_faction_state(enum faction_id, enum faction_id, enum faction_state);
int is_friendly(enum faction_id, enum faction_id);
void init_factions(void);

// obstacle_extension.c
void *get_obstacle_extension(level *, obstacle *, enum obstacle_extension_type);
int get_obstacle_index(level *, obstacle *);
void add_obstacle_extension(level *, obstacle *, enum obstacle_extension_type, void *);
void del_obstacle_extension(level *, obstacle *, enum obstacle_extension_type);
void del_obstacle_extensions(level *, obstacle *);
void defrag_obstacle_array(level *);
void free_obstacle_extensions(level *lvl);

// map_label.c
void add_map_label(level *, int, int, char *);
void del_map_label(level *, const char *);
void free_map_labels(level *lvl);
struct map_label *get_map_label(level *, const char *);
struct map_label *get_map_label_from_coords(level *, float, float);
struct map_label *map_label_exists(const char *);

// lvledit_display.c
float lvledit_zoomfact_inv(void);

// lvledit_widgets.c
struct widget_group *get_lvledit_ui(void);

// waypoint.c
int add_waypoint(level *, int, int, int);
void del_waypoint(level *, int, int);
int get_waypoint(level *, int, int);
void move_waypoint(level *, waypoint *, int, int);

// image.c
void start_image_batch(void);
void end_image_batch(void);
void display_image_on_screen(struct image *img, int x, int y, struct image_transformation t);
void display_image_on_map(struct image *img, float X, float Y, struct image_transformation t);
void create_subimage(struct image *source, struct image *new_img, SDL_Rect *rect);
void load_image(struct image *, const char *, int);
void load_image_surface(struct image *img, const char *filename, int use_offset_file);
void free_image_surface(struct image *img);
void delete_image(struct image *img);
int image_loaded(struct image *);
struct image_transformation set_image_transformation(float scale_x, float scale_y, float r, float g, float b, float a, int highlight);

// obstacle.c
obstacle *add_obstacle(level *lvl, float x, float y, int type);
void del_obstacle(obstacle *o);
obstacle_spec *get_obstacle_spec(int index);
void glue_obstacle(level *lvl, obstacle *o);
void unglue_obstacle(level *lvl, obstacle *o);
void move_obstacle(obstacle *o, float x, float y);
struct obstacle_group *get_obstacle_group_by_name(const char *group_name);
void add_obstacle_to_group(const char *group_name, int type);
struct obstacle_group *find_obstacle_group(int type);

#endif
