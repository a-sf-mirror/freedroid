/*
 *
 *   Copyright (c) 2004-2010 Arthur Huillet
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
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

#define _blocks_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit_display.h"

/**
 * This function loads the Blast image and decodes it into the multiple
 * small Blast surfaces.
 */
void Load_Blast_Surfaces(void)
{
	int j;
	char constructed_filename[5000];

	for (j = 0; j < PHASES_OF_EACH_BLAST; j++) {
		sprintf(constructed_filename, "blasts/iso_blast_bullet_%04d.png", j + 1);
		load_image(&Blastmap[0].image[j], constructed_filename, TRUE);
	}

	for (j = 0; j < PHASES_OF_EACH_BLAST; j++) {
		sprintf(constructed_filename, "blasts/iso_blast_droid_%04d.png", j + 1);
		load_image(&Blastmap[1].image[j], constructed_filename, TRUE);
	}

	for (j = 0; j < PHASES_OF_EACH_BLAST; j++) {
		sprintf(constructed_filename, "blasts/iso_blast_exterminator_%04d.png", j + 1);
		load_image(&Blastmap[2].image[j], constructed_filename, TRUE);
	}

/*Now also set up values for blasts*/
	Blastmap[0].phases = 6;
	Blastmap[1].phases = 9;
	Blastmap[2].phases = 9;
	Blastmap[0].total_animation_time = 0.6;
	Blastmap[1].total_animation_time = 1.0;
	Blastmap[2].total_animation_time = 1.0;

};				// void Load_Blast_Surfaces( void )

static void load_item_graphics(int item_type)
{
	SDL_Surface *original_img;
	SDL_Surface *tmp_surf2 = NULL;
	char fpath[2048];
	char our_filename[2000];
	itemspec *spec = &ItemMap[item_type];

	sprintf(our_filename, "items/%s", spec->item_inv_file_name);

	// Load the inventory image	
	find_file(our_filename, GRAPHICS_DIR, fpath, 0);

	original_img = IMG_Load(fpath);
	if (original_img == NULL) {
		ErrorMessage(__FUNCTION__, "\
Inventory image for item type %d, at path %s was not found", PLEASE_INFORM, IS_FATAL, item_type, fpath);
	}

	int target_x = spec->inv_size.x * 32;
	int target_y = spec->inv_size.y * 32;
	float factor_x, factor_y;
	if ((target_x != original_img->w) || (target_y != original_img->h)) {
		factor_x = (float)target_x / (float)original_img->w;
		factor_y = (float)target_y / (float)original_img->h;
		tmp_surf2 = zoomSurface(original_img, factor_x, factor_y, FALSE);
		spec->inventory_image.surface = our_SDL_display_format_wrapperAlpha(tmp_surf2);
		SDL_FreeSurface(tmp_surf2);
	} else
		spec->inventory_image.surface = our_SDL_display_format_wrapperAlpha(original_img);

	if (use_open_gl) {
		make_texture_out_of_surface(&spec->inventory_image);
	} else {
		spec->inventory_image.w = spec->inventory_image.surface->w;
		spec->inventory_image.h = spec->inventory_image.surface->h;
	}

	// For the shop, we need versions of each image, where the image is scaled so
	// that it takes up a whole 64x64 shop display square.  So we prepare scaled
	// versions here and now...
	
	// Scale shop image
	if (original_img->w >= original_img->h) {
		target_x = 64;
		target_y = original_img->h * 64.0 / (float)original_img->w;	//keep the scaling ratio !
	}

	if (original_img->h > original_img->w) {
		target_y = 64;
		target_x = original_img->w * 64.0 / (float)original_img->h;
	}

	factor_x = ((float)GameConfig.screen_width / 640.0) * ((float)target_x / (float)original_img->w);
	factor_y = ((float)GameConfig.screen_height / 480.0) * ((float)target_y / (float)original_img->h);
	tmp_surf2 = zoomSurface(original_img, factor_x, factor_y, FALSE);
	spec->shop_image.surface = our_SDL_display_format_wrapperAlpha(tmp_surf2);
	SDL_FreeSurface(original_img);
	SDL_FreeSurface(tmp_surf2);

	if (use_open_gl) {
		make_texture_out_of_surface(&spec->shop_image);
	} else {
		spec->shop_image.w = spec->shop_image.surface->w;
		spec->shop_image.h = spec->shop_image.surface->h;
	}

	// Load ingame image
	if (strcmp(spec->item_rotation_series_prefix, "NONE_AVAILABLE_YET")) {
		sprintf(our_filename, "items/%s/ingame.png", spec->item_rotation_series_prefix);
		load_image(&spec->ingame_image, our_filename, TRUE);
	} else {
		memcpy(&spec->ingame_image, &spec->inventory_image, sizeof(struct image));
	}
}

static void load_if_needed(int type)
{
	itemspec *spec = &ItemMap[type];

	if (!image_loaded(&spec->inventory_image)) {
		load_item_graphics(type);
	}
}

struct image *get_item_inventory_image(int type)
{
	load_if_needed(type);	
	return &ItemMap[type].inventory_image;
}

struct image *get_item_shop_image(int type)
{
	load_if_needed(type);	
	return &ItemMap[type].shop_image;
}

struct image *get_item_ingame_image(int type)
{
	load_if_needed(type);	
	return &ItemMap[type].ingame_image;
}

void load_all_items(void)
{
	int i;

	for (i = 0; i < Number_Of_Item_Types; i++) {
		load_item_graphics(i);
	}
}
	
/**
 * This function loads the items image and decodes it into the multiple
 * small item surfaces.
 */
void Load_Mouse_Move_Cursor_Surfaces(void)
{
	int j;
	char our_filename[2000] = "";

	for (j = 0; j < NUMBER_OF_MOUSE_CURSOR_PICTURES; j++) {
		sprintf(our_filename, "cursors/mouse_move_cursor_%d.png", j);
		load_image(&MouseCursorImageList[j], our_filename, FALSE);
	}

};				// void Load_Mouse_Move_Cursor_Surfaces( void )

/**
 * This function loads all the bullet images into memory.
 *
 */
void iso_load_bullet_surfaces(void)
{
	int i, j, k;
	char constructed_filename[5000];

	DebugPrintf(1, "Number_Of_Bullet_Types: %d.", Number_Of_Bullet_Types);

	for (i = 0; i < Number_Of_Bullet_Types; i++) {
		if (strlen(Bulletmap[i].name) && strstr(Bulletmap[i].name, "NO BULLET IMAGE"))
			continue;

		for (j = 0; j < Bulletmap[i].phases; j++) {
			for (k = 0; k < BULLET_DIRECTIONS; k++) {
				sprintf(constructed_filename, "bullets/iso_bullet_%s_%02d_%04d.png", Bulletmap[i].name, k, j + 1);

				load_image(&Bulletmap[i].image[k][j], constructed_filename, TRUE);
			}
		}
	}

};				// void iso_load_bullet_surfaces ( void )

/**
 *
 *
 */
void get_offset_for_iso_image_from_file_and_path(char *fpath, struct image * our_iso_image)
{
	char offset_file_name[10000];
	FILE *OffsetFile;
	char *offset_data;
	// Now we try to load the associated offset file, that we'll be needing
	// in order to properly fine-position the image later when blitting is to
	// a map location.
	//
	strcpy(offset_file_name, fpath);
	offset_file_name[strlen(offset_file_name) - 4] = 0;
	strcat(offset_file_name, ".offset");

	// Let's see if we can find an offset file...
	//
	if ((OffsetFile = fopen(offset_file_name, "rb")) == NULL) {
		ErrorMessage(__FUNCTION__, "\
Freedroid was unable to open offset file %s for an isometric image.\n\
Since the offset could not be obtained from the offset file, 0 will be used instead.\n\
This can lead to minor positioning pertubations\n\
in graphics displayed, but FreedroidRPG will continue to work.", NO_NEED_TO_INFORM, IS_WARNING_ONLY, offset_file_name);
		our_iso_image->offset_x = 0;
		our_iso_image->offset_y = 0;
		return;
	} else {
		fclose(OffsetFile);
	}

	// So at this point we can be certain, that the offset file is there.
	// That means, that we can now use the (otherwise terminating) read-and-malloc-...
	// functions.
	//
	offset_data = ReadAndMallocAndTerminateFile(offset_file_name, END_OF_OFFSET_FILE_STRING);

	ReadValueFromString(offset_data, OFFSET_FILE_OFFSETX_STRING, "%hd", &(our_iso_image->offset_x), offset_data + strlen(offset_data));

	ReadValueFromString(offset_data, OFFSET_FILE_OFFSETY_STRING, "%hd", &(our_iso_image->offset_y), offset_data + strlen(offset_data));
	free(offset_data);

};				// void get_offset_for_iso_image_from_file_and_path ( fpath , our_iso_image )

/**
 *
 *
 */
void LoadAndPrepareEnemyRotationModelNr(int ModelNr)
{
	int i;
	static int FirstCallEver = TRUE;
	static int EnemyFullyPrepared[ENEMY_ROTATION_MODELS_AVAILABLE];

	// Maybe this function has just been called for the first time ever.
	// Then of course we need to initialize the array, that is used for
	// keeping track of the currently loaded enemy rotation surfaces.
	// This we do here.
	//
	if (FirstCallEver) {
		for (i = 0; i < ENEMY_ROTATION_MODELS_AVAILABLE; i++) {
			EnemyFullyPrepared[i] = FALSE;
		}
		FirstCallEver = FALSE;
	}
	// Now a sanity check against using rotation types, that don't exist
	// in Freedroid RPG at all!
	//
	if ((ModelNr < 0) || (ModelNr >= ENEMY_ROTATION_MODELS_AVAILABLE)) {
		ErrorMessage(__FUNCTION__, "\
Freedroid received a rotation model number that does not exist: %d\n", PLEASE_INFORM, IS_FATAL, ModelNr);
	}
	// Now we can check if the given rotation model type was perhaps already
	// allocated and loaded and fully prepared.  Then of course we need not 
	// do anything here...  Otherwise we can have trust and mark it as loaded
	// already...
	//
	if (EnemyFullyPrepared[ModelNr])
		return;
	EnemyFullyPrepared[ModelNr] = TRUE;
	Activate_Conservative_Frame_Computation();

	grab_enemy_images_from_archive(ModelNr);
	return;

};				// void LoadAndPrepareEnemyRotationModelNr ( int j )

/**
 * Read the Enemy Surfaces details from the data stream.
 */
void get_enemy_surfaces_data(char *DataPointer)
{
	char *SurfacePointer;
	char *EndOfSurfaceData;
	int SurfaceIndex = 0;

#define ENEMY_SURFACES_SECTION_BEGIN_STRING "*** Start of Enemy Surfaces Section: ***"
#define ENEMY_SURFACES_SECTION_END_STRING "*** End of Enemy Surfaces Section: ***"
#define NEW_SURFACE_BEGIN_STRING "** Start of new surface specification subsection **"

#define SURFACES_FILE_NAME_BEGIN_STRING "PrefixToFilename=\""
#define SURFACES_WALK_ANI_SPEED_BEGIN_STRING "droid_walk_animation_speed_factor="
#define SURFACES_ATTACK_ANI_SPEED_BEGIN_STRING "droid_attack_animation_speed_factor="
#define SURFACES_GETHIT_ANI_SPEED_BEGIN_STRING "droid_gethit_animation_speed_factor="
#define SURFACES_DEATH_ANI_SPEED_BEGIN_STRING "droid_death_animation_speed_factor="
#define SURFACES_STAND_ANI_SPEED_BEGIN_STRING "droid_stand_animation_speed_factor="


	SurfacePointer = LocateStringInData(DataPointer, ENEMY_SURFACES_SECTION_BEGIN_STRING);
	EndOfSurfaceData = LocateStringInData(DataPointer, ENEMY_SURFACES_SECTION_END_STRING);

 	DebugPrintf(1, "\n\nStarting to read surfaces data...\n\n");

	SurfacePointer = DataPointer;

	while ((SurfacePointer = strstr(SurfacePointer, NEW_SURFACE_BEGIN_STRING)) != NULL) {
		if (SurfaceIndex >= ENEMY_ROTATION_MODELS_AVAILABLE) {
			ErrorMessage(__FUNCTION__, "freedroid.enemy_surfaces specifies more surfaces than ENEMY_ROTATION_MODELS_AVAILABLE (%d) allows.", PLEASE_INFORM, IS_FATAL, ENEMY_ROTATION_MODELS_AVAILABLE);
		}

 		DebugPrintf(1, "\n\nFound another surface specification entry!  Lets add that to the others!");
		SurfacePointer++;

		PrefixToFilename[SurfaceIndex] = ReadAndMallocStringFromData(SurfacePointer, SURFACES_FILE_NAME_BEGIN_STRING, "\"");

		ReadValueFromStringWithDefault(SurfacePointer, SURFACES_WALK_ANI_SPEED_BEGIN_STRING,
			"%d", "0", &(droid_walk_animation_speed_factor[SurfaceIndex]), EndOfSurfaceData);
		ReadValueFromStringWithDefault(SurfacePointer, SURFACES_ATTACK_ANI_SPEED_BEGIN_STRING,
			"%d", "0", &(droid_attack_animation_speed_factor[SurfaceIndex]), EndOfSurfaceData);
		ReadValueFromStringWithDefault(SurfacePointer, SURFACES_GETHIT_ANI_SPEED_BEGIN_STRING,
			"%d", "0", &(droid_gethit_animation_speed_factor[SurfaceIndex]), EndOfSurfaceData);
		ReadValueFromStringWithDefault(SurfacePointer, SURFACES_DEATH_ANI_SPEED_BEGIN_STRING,
			"%d", "0", &(droid_death_animation_speed_factor[SurfaceIndex]), EndOfSurfaceData);
		ReadValueFromStringWithDefault(SurfacePointer, SURFACES_STAND_ANI_SPEED_BEGIN_STRING,
			"%d", "0", &(droid_stand_animation_speed_factor[SurfaceIndex]), EndOfSurfaceData);

		SurfaceIndex++;
	}

 	DebugPrintf(1, "\nEnd of get_enemy_surfaces_data ( char* DataPointer ) reached.");
}

/**
 * This function creates all the surfaces, that are necessary to blit the
 * 'head' and 'shoes' of an enemy.  The numbers are not dealt with here.
 */
void Load_Enemy_Surfaces(void)
{
	int i;
	int j;

	// We clean out the rotated enemy surface pointers, so that later we
	// can judge securely which of them have been initialized (non-Null)
	// and which of them have not.
	//
	for (j = 0; j < ENEMY_ROTATION_MODELS_AVAILABLE; j++) {
		chat_portrait_of_droid[j].surface = NULL;
		chat_portrait_of_droid[j].texture_has_been_created = FALSE;
		for (i = 0; i < ROTATION_ANGLES_PER_ROTATION_MODEL; i++) {
			enemy_images[j][i][0].surface = NULL;
		}
	}

	// When using the new tux image collection files, the animation cycle
	// lengths for droids will be taken from the image collection file itself.
	// That is good, because it's so dynamic.  However, it also means, that
	// the real animation phase lengths and that will in general not be known
	// until the graphics for that bot has been loaded.  But on the other hand
	// it might happen that some phase computation is done before the first
	// blit already.  In that case, uninitialized data structs might cause 
	// severe harm.  Therefore we initialize some sane default values, that should
	// protect against certain cases of wrong phase counts.
	//
	for (i = 0; i < ENEMY_ROTATION_MODELS_AVAILABLE; i++) {
		first_walk_animation_image[i] = 1;
		last_walk_animation_image[i] = 1;
		first_attack_animation_image[i] = 1;
		last_attack_animation_image[i] = 1;
		first_gethit_animation_image[i] = 1;
		last_gethit_animation_image[i] = 1;
		first_death_animation_image[i] = 1;
		last_death_animation_image[i] = 1;
		first_stand_animation_image[i] = 1;
		last_stand_animation_image[i] = 1;
		use_default_attack_image[i] = TRUE;
		use_default_gethit_image[i] = TRUE;
		use_default_death_image[i] = TRUE;
		use_default_stand_image[i] = TRUE;
	}


	char fpath[2048];
	char *Data;

	find_file("freedroid.enemy_surfaces", MAP_DIR, fpath, 0);
	Data = ReadAndMallocAndTerminateFile(fpath, "*** End of this Freedroid data File ***");
	get_enemy_surfaces_data(Data);
	free(Data);
}

/**
 * Return a pointer towards the struct image
 * associated to the given obstacle type.
 * Used for lazy loading.
 */
struct image *get_obstacle_image(int type)
{
	if (!image_loaded(&obstacle_map[type].image)) {
		//printf("Just in time loading for obstacle %d\n", type);
		load_obstacle(type);
	}

	return &obstacle_map[type].image;
}

static void load_droid_portrait(int type)
{
	char fpath[1024];

	strcpy(fpath, "droids/");
	strcat(fpath, PrefixToFilename[Druidmap[type].individual_shape_nr]);
	strcat(fpath, "/portrait.png");

	load_image(&chat_portrait_of_droid[type], fpath, FALSE);
}

struct image *get_droid_portrait_image(int type)
{
	if (type >= ENEMY_ROTATION_MODELS_AVAILABLE) {
		ErrorMessage(__FUNCTION__, "Tried to load a portrait image of a bot those type is #%d, but the maximum configured value is %d.\n"
				                   "ENEMY_ROTATION_MODELS_AVAILABLE should be raised.", 
				                   PLEASE_INFORM, IS_FATAL,
				                   type, ENEMY_ROTATION_MODELS_AVAILABLE - 1);
		return NULL;
	}
	
	if (!image_loaded(&chat_portrait_of_droid[type])) {
		load_droid_portrait(type);
	}

	return &chat_portrait_of_droid[type];
}

/**
 * Load the images associated to the given
 * obstacle type.
 */
void load_obstacle(int i)
{
	char fpath[1024];
	char shadow_file_name[2000];

	if (image_loaded(&obstacle_map[i].image)) {
		ErrorMessage(__FUNCTION__, "Tried to load image for obstacle type %d that was already loaded.\n", PLEASE_INFORM,
			     IS_WARNING_ONLY, i);
		return;
	}

	if (!obstacle_map[i].filename) {
		ErrorMessage(__FUNCTION__, "Obstacle type %d has no filename specified for its graphics.\n", PLEASE_INFORM, IS_FATAL, i);
	}

	// At first we construct the file name of the single tile file we are about to load...
	sprintf(fpath, "obstacles/%s", obstacle_map[i].filename);
	load_image(&obstacle_map[i].image, fpath, TRUE);

	// Maybe the obstacle in question also has a shadow image?  In that
	// case we should load the shadow image now. 
	if (strlen(obstacle_map[i].filename) >= 8) {
		strcpy(shadow_file_name, fpath);
		shadow_file_name[strlen(shadow_file_name) - 8] = 0;
		strcat(shadow_file_name, "shadow_");
		strcat(shadow_file_name, &(fpath[strlen(fpath) - 8]));
		if (find_file(shadow_file_name, GRAPHICS_DIR, fpath, 1)) {
			obstacle_map[i].shadow_image.surface = NULL;
			obstacle_map[i].shadow_image.texture_has_been_created = FALSE;
			return;
		} else {
			load_image(&obstacle_map[i].shadow_image, shadow_file_name, TRUE);
		}
	}
}

void load_all_obstacles(void)
{
	int i;

	for (i = 0; i < NUMBER_OF_OBSTACLE_TYPES; i++) {
		load_obstacle(i);
	}

};				// void load_all_obstacles ( void )

/**
 * This function loads isometric floor tiles, and in OpenGL mode, generates
 * a texture atlas.
 *
 */
void load_floor_tiles(void)
{
	// Try to load the atlas
	if (load_texture_atlas("floor_tiles/atlas.txt", "floor_tiles/", floor_tile_filenames, floor_images, ALL_ISOMETRIC_FLOOR_TILES)) {
		ErrorMessage(__FUNCTION__, "Unable to load floor tiles atlas at floor_tiles/atlas.txt.", PLEASE_INFORM, IS_FATAL);
	}
}

#undef _blocks_c
