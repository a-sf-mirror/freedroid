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

#define _blocks_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit_display.h"

static struct droidspec *current_droid_spec;

static int __enemy_animation(const char *filename, int *rotation, int *phase, int *first_image, int **last_image)
{
	int i;
	int rotation_index = 0;
	int phase_index = 0;
	int first_animation_image = 0;
	int* last_animation_image = NULL;

	struct {
		const char *format_string;
		int first_image;
		int *last_image;
	} animations[] = {
		{ "enemy_rot_%02d_walk-%02d.png",   current_droid_spec->walk_animation_first_image,   &current_droid_spec->walk_animation_last_image   },
		{ "enemy_rot_%02d_attack-%02d.png", current_droid_spec->attack_animation_first_image, &current_droid_spec->attack_animation_last_image },
		{ "enemy_rot_%02d_gethit-%02d.png", current_droid_spec->gethit_animation_first_image, &current_droid_spec->gethit_animation_last_image },
		{ "enemy_rot_%02d_death-%02d.png",  current_droid_spec->death_animation_first_image,  &current_droid_spec->death_animation_last_image  },
		{ "enemy_rot_%02d_stand-%02d.png",  current_droid_spec->stand_animation_first_image,  &current_droid_spec->stand_animation_last_image  }
	};

	for (i = 0; i < sizeof(animations) / sizeof(animations[0]); i++) {
		if (sscanf(filename, animations[i].format_string, &rotation_index, &phase_index) == 2) {
			first_animation_image = animations[i].first_image;
			last_animation_image = animations[i].last_image;
			break;
		}
	}

	if (i == sizeof(animations) / sizeof(animations[0])) {
		error_message(__FUNCTION__, "Unexpected image filename '%s' for enemy '%s'.", PLEASE_INFORM,
			filename, current_droid_spec->gfx_prefix);
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

	return &current_droid_spec->droid_images[rotation_index][first_animation_image + phase_index - 1];
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
static void load_enemy_graphics(struct droidspec *droid_spec)
{
	char atlas_filename[4096];
	char atlas_directory[4096];

	sprintf(atlas_filename, "%s/atlas.txt", droid_spec->gfx_prefix);
	sprintf(atlas_directory, "%s/", droid_spec->gfx_prefix);

	current_droid_spec = droid_spec;

	// The information about cycle length needs to be entered into the
	// corresponding arrays (usually initialized in blocks.c, for those
	// series, that don't have an image archive yet...)
	//
	droid_spec->walk_animation_last_image = 0;
	droid_spec->attack_animation_last_image = 0;
	droid_spec->gethit_animation_last_image = 0;
	droid_spec->death_animation_last_image = 0;
	droid_spec->stand_animation_last_image = 0;

	if (load_texture_atlas(atlas_filename, atlas_directory, compute_number_of_phases_for_enemy)) {
		error_message(__FUNCTION__, "Unable to access the texture atlas for enemy '%s' at '%s'.",
			PLEASE_INFORM | IS_FATAL, droid_spec->gfx_prefix, atlas_filename);
	}

	droid_spec->walk_animation_first_image   = 1;
	droid_spec->attack_animation_first_image = droid_spec->walk_animation_last_image + 1;
	droid_spec->attack_animation_last_image += droid_spec->walk_animation_last_image;
	droid_spec->gethit_animation_first_image = droid_spec->attack_animation_last_image + 1;
	droid_spec->gethit_animation_last_image += droid_spec->attack_animation_last_image;
	droid_spec->death_animation_first_image  = droid_spec->gethit_animation_last_image + 1;
	droid_spec->death_animation_last_image  += droid_spec->gethit_animation_last_image;
	droid_spec->stand_animation_first_image  = droid_spec->death_animation_last_image + 1;
	droid_spec->stand_animation_last_image  += droid_spec->death_animation_last_image;

	// Now some error checking against more phases in this enemy animation than
	// currently allowed from the array size...
	//
	if (droid_spec->stand_animation_last_image >= MAX_ENEMY_MOVEMENT_PHASES) {
		error_message(__FUNCTION__,
			"The number of images found in the image collection for enemy model %s is bigger than currently allowed (found %d images, max. %d).",
			PLEASE_INFORM | IS_FATAL, droid_spec->droidname, droid_spec->stand_animation_last_image, MAX_ENEMY_MOVEMENT_PHASES);
	}

	if (load_texture_atlas(atlas_filename, atlas_directory, get_storage_for_enemy_image)) {
		error_message(__FUNCTION__, "Unable to load texture atlas for enemy '%s' at %s.",
			PLEASE_INFORM | IS_FATAL, droid_spec->gfx_prefix, atlas_filename);
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
			load_image(&Blastmap[i].images[j], GRAPHICS_DIR, fpath, USE_OFFSET);
		}
	}
}

static void load_item_graphics(int item_type)
{
    char our_filename[PATH_MAX];
    itemspec *spec = &ItemMap[item_type];
    int target_x = spec->inv_size.x * 32;
    int target_y = spec->inv_size.y * 32;
    float factor_x, factor_y;

    sprintf(our_filename, "items/%s", spec->item_inv_file_name);

    if (use_open_gl) {
        load_image(&spec->inventory_image, GRAPHICS_DIR, our_filename, 0);
        spec->shop_image = spec->inventory_image;

        // Scale inventory image
        spec->inventory_image.w = target_x;
        spec->inventory_image.h = target_y;
        
        // Scale shop image to keep aspect ratio and fill 64 pixels (at -r0)
        float shop_square_size = 64.0 * GameConfig.screen_width / 640.0;
        short int *small = &(spec->shop_image.w);
        short int *big = &(spec->shop_image.h);

        if (*small > *big) {
            big = &(spec->shop_image.w);
            small = &(spec->shop_image.h);
        }

        float ratio = (float)*small/(float)*big;
        *big = shop_square_size;
        *small = shop_square_size * ratio;

    } else {
        SDL_Surface *original_img;
        SDL_Surface *tmp_surf2 = NULL;
        char fpath[PATH_MAX];

        // Load the inventory image	
        find_file(fpath, GRAPHICS_DIR, our_filename, NULL, PLEASE_INFORM | IS_FATAL);

        original_img = IMG_Load(fpath);
        if (original_img == NULL) {
            error_message(__FUNCTION__, "Inventory image for item type %d, at path %s was not found",
                    PLEASE_INFORM | IS_FATAL, item_type, fpath);
        }

        if ((target_x != original_img->w) || (target_y != original_img->h)) {
            factor_x = (float)target_x / (float)original_img->w;
            factor_y = (float)target_y / (float)original_img->h;
            tmp_surf2 = zoomSurface(original_img, factor_x, factor_y, FALSE);
            spec->inventory_image.surface = SDL_DisplayFormatAlpha(tmp_surf2);
            SDL_FreeSurface(tmp_surf2);
        } else {
            spec->inventory_image.surface = SDL_DisplayFormatAlpha(original_img);
        }

        spec->inventory_image.w = spec->inventory_image.surface->w;
        spec->inventory_image.h = spec->inventory_image.surface->h;

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

        spec->shop_image.w = spec->shop_image.surface->w;
        spec->shop_image.h = spec->shop_image.surface->h;
    }

	// Load ingame image
	if (strcmp(spec->item_rotation_series_prefix, "NONE_AVAILABLE_YET")) {
		sprintf(our_filename, "items/%s/ingame.png", spec->item_rotation_series_prefix);
		load_image(&spec->ingame_image, GRAPHICS_DIR, our_filename, USE_OFFSET);
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
		load_image(&MouseCursorImageList[j], GUI_DIR, our_filename, NO_MOD);
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

	for (i = 0; i < bullet_specs.size; i++) {
		struct bulletspec *bullet_spec = dynarray_member(&bullet_specs, i, sizeof(struct bulletspec));

		if (strlen(bullet_spec->name) && strstr(bullet_spec->name, "NO BULLET IMAGE"))
			continue;

		for (j = 0; j < bullet_spec->phases; j++) {
			for (k = 0; k < BULLET_DIRECTIONS; k++) {
				sprintf(constructed_filename, "bullets/iso_bullet_%s_%02d_%04d.png", bullet_spec->name, k, j + 1);

				load_image(&bullet_spec->image[k][j], GRAPHICS_DIR, constructed_filename, USE_OFFSET);
			}
		}
	}

};				// void iso_load_bullet_surfaces ( void )

/**
 *
 *
 */
void get_offset_for_iso_image_from_file_and_path(const char *fpath, struct image * our_iso_image)
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
		error_message(__FUNCTION__, "\
FreedroidRPG was unable to open offset file %s for an isometric image.\n\
Since the offset could not be obtained from the offset file, 0 will be used instead.\n\
This can lead to minor positioning perturbations\n\
in graphics displayed, but FreedroidRPG will continue to work.", NO_REPORT, offset_file_name);
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
	offset_data = read_and_malloc_and_terminate_file(offset_file_name, END_OF_OFFSET_FILE_STRING);

	ReadValueFromString(offset_data, OFFSET_FILE_OFFSETX_STRING, "%hd", &(our_iso_image->offset_x), offset_data + strlen(offset_data));

	ReadValueFromString(offset_data, OFFSET_FILE_OFFSETY_STRING, "%hd", &(our_iso_image->offset_y), offset_data + strlen(offset_data));
	free(offset_data);

};				// void get_offset_for_iso_image_from_file_and_path ( fpath , our_iso_image )

/**
 *
 *
 */
void load_droid_animation_images(struct droidspec *this_droid_spec)
{
	if (!this_droid_spec->is_a_living || this_droid_spec->gfx_prepared)
		return;

	Activate_Conservative_Frame_Computation();

	load_enemy_graphics(this_droid_spec);
	this_droid_spec->gfx_prepared = TRUE;
}

void free_enemy_graphics(void)
{
	int i;
	int rotation_index, phase_index;

	for (i = 0; i < Number_Of_Droid_Types; i++) {
		if (Droidmap[i].gfx_prepared) {
			for (rotation_index = 0; rotation_index < ROTATION_ANGLES_PER_ROTATION_MODEL; rotation_index++) {
				for (phase_index = 0; phase_index < MAX_ENEMY_MOVEMENT_PHASES; phase_index++)
					delete_image(&Droidmap[i].droid_images[rotation_index][phase_index]);
			}
			Droidmap[i].gfx_prepared = FALSE;
		}
		if (image_loaded(&Droidmap[i].portrait))
			delete_image(&Droidmap[i].portrait);
	}
}

static void load_droid_portrait(int type)
{
	char fpath[1024];

	strcpy(fpath, Droidmap[type].gfx_prefix);
	strcat(fpath, "/portrait.png");

	load_image(&Droidmap[type].portrait, GRAPHICS_DIR, fpath, NO_MOD);
}

struct image *get_droid_portrait_image(int type)
{
	if (!image_loaded(&Droidmap[type].portrait)) {
		load_droid_portrait(type);
	}

	return &Droidmap[type].portrait;
}

/**
 * Return a pointer towards the struct image
 * associated to the given obstacle type.
 */
struct image *get_obstacle_image(int type, int frame_index)
{
	struct obstacle_graphics *obs_graphics = &((struct obstacle_graphics *)obstacle_images.arr)[type];
	return &obs_graphics->images[frame_index % obs_graphics->count];
}

/**
 * Return a pointer towards the shadow image
 * associated to the given obstacle type.
 */
struct image *get_obstacle_shadow_image(int type, int frame_index)
{
	struct obstacle_graphics *obs_graphics= &((struct obstacle_graphics *)obstacle_images.arr)[type];
	return &obs_graphics->shadows[frame_index % obs_graphics->count];
}

/**
 * Return a pointer towards the map label image.
 */
struct image *get_map_label_image()
{
	static struct image img = EMPTY_IMAGE;

	if (!image_loaded(&img))
		load_image(&img, GUI_DIR, "level_editor/map_label_indicator.png", USE_OFFSET);

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
					error_message(__FUNCTION__, "Could not load the image '%s' for obstacle %d.",
						PLEASE_INFORM, filename, i);
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

	error_message(__FUNCTION__, "Obstacles texture atlas specifies element %s which is not expected.",
		PLEASE_INFORM, filename);
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

	error_message(__FUNCTION__, "Obstacle shadows texture atlas specifies element %s which is not expected.",
		PLEASE_INFORM, filename);
	return NULL;
}

void load_all_obstacles(int with_startup_bar)
{
	int i, j;
	struct image empty_image = EMPTY_IMAGE;

	if (load_texture_atlas("obstacles/atlas.txt", "obstacles/", get_storage_for_obstacle_image)) {
		error_message(__FUNCTION__, "Unable to load texture atlas for obstacles at obstacles/atlas.txt.", PLEASE_INFORM | IS_FATAL);
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
		error_message(__FUNCTION__, "Unable to load texture atlas for obstacle shadows at obstacle/shadow_atlas.txt.", PLEASE_INFORM | IS_FATAL);

	if (with_startup_bar)
		next_startup_percentage(8);

	validate_obstacle_graphics();
}

static struct image *get_storage_for_floor_tile(const char *filename)
{
	int i, j;

	for (i = 0; i < underlay_floor_tiles.size; i++) {
		struct floor_tile_spec *floor_tile = dynarray_member(&underlay_floor_tiles, i, sizeof(struct floor_tile_spec));
		for (j = 0; j < floor_tile->frames; j++) {
			if (!strcmp(((char **)floor_tile->filenames.arr)[j], filename))
				return &floor_tile->images[j];
		}
	}

	for (i = 0; i < overlay_floor_tiles.size; i++) {
		struct floor_tile_spec *floor_tile = dynarray_member(&overlay_floor_tiles, i, sizeof(struct floor_tile_spec));
		for (j = 0; j < floor_tile->frames; j++) {
			if (!strcmp(((char **)floor_tile->filenames.arr)[j], filename))
				return &floor_tile->images[j];
		}
	}

	error_message(__FUNCTION__, "Floor tiles texture atlas specifies element %s which is not expected.",
		PLEASE_INFORM, filename);
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
		error_message(__FUNCTION__, "Unable to load floor tiles atlas at floor_tiles/atlas.txt.", PLEASE_INFORM | IS_FATAL);
	}
}

void free_floor_tiles(void)
{
	int i, j;

	for (i = 0; i < underlay_floor_tiles.size; i++) {
		struct floor_tile_spec *floor_tile = dynarray_member(&underlay_floor_tiles, i, sizeof(struct floor_tile_spec));
		for (j = 0; j < floor_tile->frames; j++) {
			delete_image(&floor_tile->images[j]);
		}
	}

	for (i = 0; i < overlay_floor_tiles.size; i++) {
		struct floor_tile_spec *floor_tile = dynarray_member(&overlay_floor_tiles, i, sizeof(struct floor_tile_spec));
		for (j = 0; j < floor_tile->frames; j++) {
			delete_image(&floor_tile->images[j]);
		}
	}
}

static int current_tux_motion_class;
static int current_tux_part_group;

static struct image *get_storage_for_tux_image(const char *filename)
{
	int rotation;
	int phase;

	if (sscanf(filename, "tux_rot_%02d_phase_%02d.png", &rotation, &phase) != 2) {
		error_message(__FUNCTION__, "Invalid filename '%s' in tux texture atlas.",
			PLEASE_INFORM, filename);
		return NULL;
	}

	if (rotation >= MAX_TUX_DIRECTIONS) {
		error_message(__FUNCTION__, "Invalid rotation index %d in tux texture atlas.\n"
			"Maximum allowed value for the rotation index is %d.", PLEASE_INFORM,
			rotation, MAX_TUX_DIRECTIONS - 1);
		return NULL;
	}

	if (phase >= TUX_TOTAL_PHASES) {
		error_message(__FUNCTION__, "Invalid phase index %d in tux texture atlas.\n"
			"Maximum allowed value for the phase index is %d.", PLEASE_INFORM,
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
		error_message(__FUNCTION__, "Unable to load tux texture atlas at %s.",
			PLEASE_INFORM | IS_FATAL, atlas_filename);
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
