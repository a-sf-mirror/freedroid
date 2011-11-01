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

static int current_enemy_nr;

static int __enemy_animation(const char *filename, int *rotation, int *phase, int *first_image, int **last_image)
{
	int i;
	int rotation_index;
	int phase_index;
	int first_animation_image;
	int* last_animation_image;

	struct {
		const char *format_string;
		int first_image;
		int *last_image;
	} animations[] = {
		{ "enemy_rot_%02d_walk-%02d.png", first_walk_animation_image[current_enemy_nr], &last_walk_animation_image[current_enemy_nr] },
		{ "enemy_rot_%02d_attack-%02d.png", first_attack_animation_image[current_enemy_nr], &last_attack_animation_image[current_enemy_nr] },
		{ "enemy_rot_%02d_gethit-%02d.png", first_gethit_animation_image[current_enemy_nr], &last_gethit_animation_image[current_enemy_nr] },
		{ "enemy_rot_%02d_death-%02d.png", first_death_animation_image[current_enemy_nr], &last_death_animation_image[current_enemy_nr] },
		{ "enemy_rot_%02d_stand-%02d.png", first_stand_animation_image[current_enemy_nr], &last_stand_animation_image[current_enemy_nr] }
	};

	for (i = 0; i < sizeof(animations) / sizeof(animations[0]); i++) {
		if (sscanf(filename, animations[i].format_string, &rotation_index, &phase_index) == 2) {
			first_animation_image = animations[i].first_image;
			last_animation_image = animations[i].last_image;
			break;
		}
	}

	if (i == sizeof(animations) / sizeof(animations[0])) {
		ErrorMessage(__FUNCTION__, "Unexpected image filename '%s' for enemy '%s'.", PLEASE_INFORM, IS_WARNING_ONLY,
			filename, PrefixToFilename[current_enemy_nr]);
		return -1;
	}

	if (rotation)
		*rotation = rotation_index;
	if (phase)
		*phase = phase_index;
	if (first_image)
		*first_image = first_animation_image;
	if (last_image)
		*last_image = last_animation_image;

	return 0;
}

static struct image *get_storage_for_enemy_image(const char *filename)
{
	int rotation_index;
	int phase_index;
	int first_animation_image;

	if (__enemy_animation(filename, &rotation_index, &phase_index, &first_animation_image, NULL))
		return NULL;

	return &enemy_images[current_enemy_nr][rotation_index][first_animation_image + phase_index - 1];
}

static struct image *compute_number_of_phases_for_enemy(const char *filename)
{
	int phase_index;
	int *last_animation_image;

	if (!__enemy_animation(filename, NULL, &phase_index, NULL, &last_animation_image)) {
		if (*last_animation_image < phase_index + 1)
			*last_animation_image = phase_index + 1;
	}

	return NULL;
}

/**
 * Enemy animation images are stored in texture atlases. This functions loads all
 * the images for the given enemy model.
 * It's typically called once whenever the enemy type is first encountered
 * in one run of the engine.
 */
static void load_enemy_graphics(int enemy_model_nr)
{
	char atlas_filename[4096];
	char atlas_directory[4096];

	sprintf(atlas_filename, "droids/%s/atlas.txt", PrefixToFilename[enemy_model_nr]);
	sprintf(atlas_directory, "droids/%s/", PrefixToFilename[enemy_model_nr]);

	// The information about cycle length needs to be entered into the
	// corresponding arrays (usually initialized in blocks.c, for those
	// series, that don't have an image archive yet...)
	//
	last_walk_animation_image[enemy_model_nr] = 0;
	last_attack_animation_image[enemy_model_nr] = 0;
	last_gethit_animation_image[enemy_model_nr] = 0;
	last_death_animation_image[enemy_model_nr] = 0;
	last_stand_animation_image[enemy_model_nr] = 0;

	current_enemy_nr = enemy_model_nr;
	if (load_texture_atlas(atlas_filename, atlas_directory, compute_number_of_phases_for_enemy)) {
		ErrorMessage(__FUNCTION__, "Unable to access the texture atlas for enemy '%s' at '%s'.",
			PLEASE_INFORM, IS_FATAL, PrefixToFilename[enemy_model_nr], atlas_filename);
	}

	first_walk_animation_image[enemy_model_nr] = 1;
	first_attack_animation_image[enemy_model_nr] = last_walk_animation_image[enemy_model_nr] + 1;
	last_attack_animation_image[enemy_model_nr] += last_walk_animation_image[enemy_model_nr];
	first_gethit_animation_image[enemy_model_nr] = last_attack_animation_image[enemy_model_nr] + 1;
	last_gethit_animation_image[enemy_model_nr] += last_attack_animation_image[enemy_model_nr];
	first_death_animation_image[enemy_model_nr] = last_gethit_animation_image[enemy_model_nr] + 1;
	last_death_animation_image[enemy_model_nr] += last_gethit_animation_image[enemy_model_nr];
	first_stand_animation_image[enemy_model_nr] = last_death_animation_image[enemy_model_nr] + 1;
	last_stand_animation_image[enemy_model_nr] += last_death_animation_image[enemy_model_nr];

	// Now some error checking against more phases in this enemy animation than
	// currently allowed from the array size...
	//
	if (last_stand_animation_image[enemy_model_nr] >= MAX_ENEMY_MOVEMENT_PHASES) {
		ErrorMessage(__FUNCTION__,
			"The number of images found in the image collection for enemy model %d is bigger than currently allowed (found %d images, max. %d).",
			PLEASE_INFORM, IS_FATAL, enemy_model_nr, last_stand_animation_image[enemy_model_nr], MAX_ENEMY_MOVEMENT_PHASES);
	}

	if (load_texture_atlas(atlas_filename, atlas_directory, get_storage_for_enemy_image)) {
		ErrorMessage(__FUNCTION__, "Unable to load texture atlas for enemy '%s' at %s.",
			PLEASE_INFORM, IS_FATAL, PrefixToFilename[enemy_model_nr], atlas_filename);
	}
}


/**
 * This function loads the Blast image and decodes it into the multiple
 * small Blast surfaces.
 */
void Load_Blast_Surfaces(void)
{
	int i, j;
	char fpath[2048];

	for (i = 0; i < sizeof(Blastmap) / sizeof(Blastmap[0]); i++) {
		for (j = 0; j < Blastmap[i].phases; j++) {
			sprintf(fpath, "blasts/%s_%04d.png", Blastmap[i].name, j + 1);
			load_image(&Blastmap[i].images[j], fpath, TRUE);
		}
	}
}

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
		spec->inventory_image.surface = SDL_DisplayFormatAlpha(tmp_surf2);
		SDL_FreeSurface(tmp_surf2);
	} else
		spec->inventory_image.surface = SDL_DisplayFormatAlpha(original_img);

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
	spec->shop_image.surface = SDL_DisplayFormatAlpha(tmp_surf2);
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
 * Free all images associated with items.
 */
void free_item_graphics(void)
{
	int i;
	struct image empty_image = EMPTY_IMAGE;

	for (i = 0; i < Number_Of_Item_Types; i++) {
		if (image_loaded(&ItemMap[i].inventory_image)) {
			delete_image(&ItemMap[i].inventory_image);

			// If the ingame image is not available for an item, then it is just a copy
			// of the inventory image. In this case, the ingame image should not be
			// deleted. The resources associated with this image will be freed when
			// the inventory image is deleted.
			if (strcmp(ItemMap[i].item_rotation_series_prefix, "NONE_AVAILABLE_YET"))
				delete_image(&ItemMap[i].ingame_image);
			else
				memcpy(&ItemMap[i].ingame_image, &empty_image, sizeof(struct image));

			delete_image(&ItemMap[i].shop_image);
		}
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
This can lead to minor positioning perturbations\n\
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

static int EnemyFullyPrepared[ENEMY_ROTATION_MODELS_AVAILABLE];

/**
 *
 *
 */
void LoadAndPrepareEnemyRotationModelNr(int ModelNr)
{
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

	load_enemy_graphics(ModelNr);
}

void free_enemy_graphics(void)
{
	int i;
	int rotation_index, phase_index;

	for (i = 0; i < ENEMY_ROTATION_MODELS_AVAILABLE; i++) {
		if (!EnemyFullyPrepared[i])
			continue;

		for (rotation_index = 0; rotation_index < ROTATION_ANGLES_PER_ROTATION_MODEL; rotation_index++) {
			for (phase_index = 0; phase_index < MAX_ENEMY_MOVEMENT_PHASES; phase_index++)
				delete_image(&enemy_images[i][rotation_index][phase_index]);
		}
		EnemyFullyPrepared[i] = FALSE;
	}
}

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
			ErrorMessage(__FUNCTION__, "enemy_surfaces.dat specifies more surfaces than ENEMY_ROTATION_MODELS_AVAILABLE (%d) allows.", PLEASE_INFORM, IS_FATAL, ENEMY_ROTATION_MODELS_AVAILABLE);
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

	for (i = 0; i < ENEMY_ROTATION_MODELS_AVAILABLE; i++) {
		struct image empty = EMPTY_IMAGE;
		chat_portrait_of_droid[i] = empty;
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
	}


	char fpath[2048];
	char *Data;

	find_file("enemy_surfaces.dat", MAP_DIR, fpath, 0);
	Data = ReadAndMallocAndTerminateFile(fpath, "*** End of this Freedroid data File ***");
	get_enemy_surfaces_data(Data);
	free(Data);
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
 * Return a pointer towards the struct image
 * associated to the given obstacle type.
 */
struct image *get_obstacle_image(int type, int frame_index)
{
	struct obstacle_graphics *obs_graphics = &((struct obstacle_graphics *)obstacle_images.arr)[type];
	return &obs_graphics->images[frame_index];
}

/**
 * Return a pointer towards the shadow image
 * associated to the given obstacle type.
 */
struct image *get_obstacle_shadow_image(int type, int frame_index)
{
	struct obstacle_graphics *obs_graphics= &((struct obstacle_graphics *)obstacle_images.arr)[type];
	return &obs_graphics->shadows[frame_index];
}

/**
 * Return a pointer towards the map label image.
 */
struct image *get_map_label_image()
{
	static struct image img = EMPTY_IMAGE;

	if (!image_loaded(&img))
		load_image(&img, "level_editor_map_label_indicator.png", TRUE);

	return &img;
}

/**
 * Free all images associated with obstacles.
 */
void free_obstacle_graphics(void)
{
	int i, j;
	for (i = 0; i < obstacle_map.size; i++) {
		struct obstacle_graphics *graphics = &((struct obstacle_graphics *)obstacle_images.arr)[i];
		for (j = 0; j < graphics->count; j++) {
			delete_image(&graphics->images[j]);
			delete_image(&graphics->shadows[j]);
		}
	}
}

/**
 * Check if all images for obstacles were loaded correctly. Issue warnings for
 * missing images.
 */
static void validate_obstacle_graphics(void)
{
	int i, j;
	for (i = 0; i < obstacle_map.size; i++) {
		struct obstacle_graphics *graphics = &((struct obstacle_graphics *)obstacle_images.arr)[i];
		for (j = 0; j < graphics->count; j++) {
			const char *filename = ((char **)get_obstacle_spec(i)->filenames.arr)[j];
			if (!image_loaded(&graphics->images[j])) {
				if (strcmp(filename, "DUMMY OBSTACLE")) {
					ErrorMessage(__FUNCTION__, "Could not load the image '%s' for obstacle %d.",
						PLEASE_INFORM, IS_WARNING_ONLY, filename, i);
				}
			}
		}
	}
}

static struct image *get_storage_for_obstacle_image(const char *filename)
{
	int i, j;

	for (i = 0; i < obstacle_map.size; i++) {
		obstacle_spec *spec = get_obstacle_spec(i);
		struct obstacle_graphics *graphics = &((struct obstacle_graphics *)obstacle_images.arr)[i];
		for (j = 0; j < spec->filenames.size; j++) {
			const char *fname = ((char **)spec->filenames.arr)[j];
			if (!strcmp(fname, filename))
				return &graphics->images[j];
		}
	}

	ErrorMessage(__FUNCTION__, "Obstacles texture atlas specifies element %s which is not expected.",
		PLEASE_INFORM, IS_WARNING_ONLY, filename);
	return NULL;
}

static struct image *get_storage_for_obstacle_shadow_image(const char *filename)
{
	int i, j;
	const char *obstacle_filename = filename + strlen("shadow_");

	for (i = 0; i < obstacle_map.size; i++) {
		obstacle_spec *spec = get_obstacle_spec(i);
		struct obstacle_graphics *graphics = &((struct obstacle_graphics *)obstacle_images.arr)[i];
		for (j = 0; j < spec->filenames.size; j++) {
			const char *fname = ((char **)spec->filenames.arr)[j];
			if (!strcmp(fname, obstacle_filename))
				return &graphics->shadows[j];
		}
	}

	ErrorMessage(__FUNCTION__, "Obstacle shadows texture atlas specifies element %s which is not expected.",
		PLEASE_INFORM, IS_WARNING_ONLY, filename);
	return NULL;
}

void load_all_obstacles(int with_startup_bar)
{
	int i, j;
	struct image empty_image = EMPTY_IMAGE;

	if (load_texture_atlas("obstacles/atlas.txt", "obstacles/", get_storage_for_obstacle_image)) {
		ErrorMessage(__FUNCTION__, "Unable to load texture atlas for obstacles at obstacles/atlas.txt.", PLEASE_INFORM, IS_FATAL);
	}

	if (with_startup_bar)
		next_startup_percentage(62);

	// Clear obstacle shadow images
	// It's required because only subset of obstacles has got shadow
	for (i = 0; i < obstacle_map.size; i++) {
		obstacle_spec *spec = get_obstacle_spec(i);
		struct obstacle_graphics *graphics = &((struct obstacle_graphics *)obstacle_images.arr)[i];
		for (j = 0; j < spec->filenames.size; j++)
			memcpy(&graphics->shadows[j], &empty_image, sizeof(empty_image));
	}

	if (load_texture_atlas("obstacles/shadow_atlas.txt", "obstacles/", get_storage_for_obstacle_shadow_image))
		ErrorMessage(__FUNCTION__, "Unable to load texture atlas for obstacle shadows at obstacle/shadow_atlas.txt.", PLEASE_INFORM, IS_FATAL);

	if (with_startup_bar)
		next_startup_percentage(8);

	validate_obstacle_graphics();
}

static struct image *get_storage_for_floor_tile(const char *filename)
{
	int i;

	for (i = 0; i < underlay_floor_tile_filenames.size; i++) {
		char **current_filename = dynarray_member(&underlay_floor_tile_filenames, i, sizeof(char *));
		if (!strcmp(*current_filename, filename))
			return dynarray_member(&underlay_floor_images, i, sizeof(struct image));
	}

	for (i = 0; i < overlay_floor_tile_filenames.size; i++) {
		char **current_filename = dynarray_member(&overlay_floor_tile_filenames, i, sizeof(char *));
		if (!strcmp(*current_filename, filename))
			return dynarray_member(&overlay_floor_images, i, sizeof(struct image));
	}

	ErrorMessage(__FUNCTION__, "Floor tiles texture atlas specifies element %s which is not expected.",
		PLEASE_INFORM, IS_WARNING_ONLY, filename);
	return NULL;
}

/**
 * This function loads isometric floor tiles, and in OpenGL mode, generates
 * a texture atlas.
 *
 */
void load_floor_tiles(void)
{
	// Try to load the atlas
	if (load_texture_atlas("floor_tiles/atlas.txt", "floor_tiles/", get_storage_for_floor_tile)) {
		ErrorMessage(__FUNCTION__, "Unable to load floor tiles atlas at floor_tiles/atlas.txt.", PLEASE_INFORM, IS_FATAL);
	}
}

void free_floor_tiles(void)
{
	int i;
	for (i = 0; i < underlay_floor_images.size; i++) {
		delete_image(dynarray_member(&underlay_floor_images, i, sizeof(struct image)));
	}

	for (i = 0; i < overlay_floor_images.size; i++) {
		delete_image(dynarray_member(&overlay_floor_images, i, sizeof(struct image)));
	}
}

static int current_tux_motion_class;
static int current_tux_part_group;

static struct image *get_storage_for_tux_image(const char *filename)
{
	int rotation;
	int phase;

	if (sscanf(filename, "tux_rot_%02d_phase_%02d.png", &rotation, &phase) != 2) {
		ErrorMessage(__FUNCTION__, "Invalid filename '%s' in tux texture atlas.",
			PLEASE_INFORM, IS_WARNING_ONLY, filename);
		return NULL;
	}

	if (rotation >= MAX_TUX_DIRECTIONS) {
		ErrorMessage(__FUNCTION__, "Invalid rotation index %d in tux texture atlas.\n"
			"Maximum allowed value for the rotation index is %d.", PLEASE_INFORM, IS_WARNING_ONLY,
			rotation, MAX_TUX_DIRECTIONS - 1);
		return NULL;
	}

	if (phase >= TUX_TOTAL_PHASES) {
		ErrorMessage(__FUNCTION__, "Invalid phase index %d in tux texture atlas.\n"
			"Maximum allowed value for the phase index is %d.", PLEASE_INFORM, IS_WARNING_ONLY,
			phase, TUX_TOTAL_PHASES - 1);
		return NULL;
	}

	return &tux_images[current_tux_motion_class].part_images[current_tux_part_group][phase][rotation];
}

/**
 * Load the tux image part group for the given motion class.
 */
void load_tux_graphics(int motion_class, int tux_part_group, const char *part_string)
{
	char atlas_filename[4096];
	char atlas_directory[4096];
	static char *part_group_strings[ALL_PART_GROUPS] = {
		"head/",
		"shield/",
		"torso/",
		"feet/",
		"weapon/",
		"weaponarm/"
	};

	sprintf(atlas_directory, "tux_motion_parts/%s/%s%s/",
		get_motion_class_name_by_id(motion_class), part_group_strings[tux_part_group], part_string);
	sprintf(atlas_filename, "%s/atlas.txt", atlas_directory);

	current_tux_motion_class = motion_class;
	current_tux_part_group = tux_part_group;
	if (load_texture_atlas(atlas_filename, atlas_directory, get_storage_for_tux_image)) {
		ErrorMessage(__FUNCTION__, "Unable to load tux texture atlas at %s.",
			PLEASE_INFORM, IS_FATAL, atlas_filename);
	}
}

/**
 * Force tux graphics to reload.
 */
void reload_tux_graphics(void)
{
	int i, j;

	// Clear tux part strings. It will force tux graphics to reload.
	for (i = 0; i < tux_rendering.motion_class_names.size; i++) {
		struct tux_motion_class_images *motion_class = &tux_images[i];
		for (j = 0; j < ALL_PART_GROUPS; j++)
			motion_class->part_names[j][0] = '\0';
	}
}

#undef _blocks_c
