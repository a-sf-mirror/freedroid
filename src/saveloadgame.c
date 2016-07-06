/* 
 *
 *   Copyright (c) 2002, 2003 Johannes Prix
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

/**
 * All functions that have to do with loading and saving of games.
 */

#define _saveloadgame_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include <sys/stat.h>
#include "widgets/widgets.h"
#include "savestruct.h"
#include "savegame/savegame.h"

#include "scandir.h"

#ifdef HAVE_LANGINFO_H
#include <langinfo.h>
#endif

#ifndef HAVE_NL_LANGINFO
#if __WIN32__
#define nl_langinfo(X) "%a %b %#d %H:%M:%S %Y"
#else
#define nl_langinfo(X) "%a %b %e %H:%M:%S %Y"
#endif
#endif

#define SAVEDGAME_EXT ".sav.gz"
#define SAVE_GAME_THUMBNAIL_EXT ".thumbnail.png"

/**
 * Filter function for scandir calls.
 * This function keeps files with the SAVEDGAME_EXT extension.
 */
static int filename_filter_func(const struct dirent *file)
{
	char *pos = strstr(file->d_name, SAVEDGAME_EXT);

	if (pos != NULL) {	// SAVEDGAME_EXT found
		if (strlen(pos) == strlen(SAVEDGAME_EXT)) {
			// d_name *ENDS* with SAVEDGAME_EXT

			if (strstr(file->d_name, ".bkp"SAVEDGAME_EXT) + 4 == pos) {
				//then we have .bkp+SAVEDGAME_EXT = filter it out
				return 0;
			}
			return 1;
		}
	}
	return 0;
}

/**
 * Find all currently saved games.
 * @param namelist the found files -- must bee freed just like scandir(3)
 * @return the number of found save games
 */
int find_saved_games(struct dirent ***namelist)
{
	int n = scandir(data_dirs[CONFIG_DIR].path, namelist, filename_filter_func, alphasort);

	if (n == -1)
	{
		error_message(__FUNCTION__, "Error occurred while reading save game directory.",
					 NO_REPORT);
		return 0;
	}

	// For each element in list, remove the suffix SAVEDGAME_EXT
	int i;
	for (i = 0; i < n; i++) {
		*strstr((*namelist)[i]->d_name, SAVEDGAME_EXT) = 0;
		if (!strlen((*namelist)[i]->d_name))
			strcpy((*namelist)[i]->d_name, "INVALID");
	}

	return n;
}

void load_and_show_thumbnail(char *core_filename)
{
	char filename[PATH_MAX];
	char filepath[PATH_MAX];
	struct image thumbnail = EMPTY_IMAGE;
	SDL_Rect target_rectangle;

	if (!strlen(data_dirs[CONFIG_DIR].path))
		return;

	sprintf(filename, "%s%s", core_filename, SAVE_GAME_THUMBNAIL_EXT);
	if (!find_file(filepath, CONFIG_DIR, filename, NULL, NO_REPORT))
		return;

	/* Load the image */
	thumbnail.surface = our_IMG_load_wrapper(filepath);
	if (!thumbnail.surface)
		return;

	thumbnail.w = thumbnail.surface->w;
	thumbnail.h = thumbnail.surface->h;
	if (use_open_gl)
		make_texture_out_of_surface(&thumbnail);

	target_rectangle.x = 10;
	target_rectangle.y = GameConfig.screen_height - thumbnail.h - 10;

	display_image_on_screen(&thumbnail, target_rectangle.x, target_rectangle.y, IMAGE_NO_TRANSFO);

	delete_image(&thumbnail);
}

/**
 * 
 *
 */
void load_and_show_stats(char *core_filename)
{
	char filename[PATH_MAX];
	char filepath[PATH_MAX];
	struct stat file_info_buffer;
	char info_string[5000];
	long int file_size;

	if (!strlen(data_dirs[CONFIG_DIR].path))
		return;

	// First we get the information of the .sav file

	sprintf(filename, "%s%s", core_filename, SAVEDGAME_EXT);
	if (!find_file(filepath, CONFIG_DIR, filename, NULL, NO_REPORT)) {
		error_once_message(ONCE_PER_RUN, __FUNCTION__,
		                   "FreedroidRPG was unable to access your saved game file (%s).\n"
		                   "This is either a bug in FreedroidRPG or an indication, that the directory\n"
		                   "or file permissions of ~/.freedroid_rpg are somehow not right.",
		                   PLEASE_INFORM, filename);
		return;
	}

	if (stat(filepath, &file_info_buffer)) {
		error_once_message(ONCE_PER_RUN, __FUNCTION__,
		                   "FreedroidRPG was unable to get the stats of your saved game file (%s).\n"
		                   "This is either a bug in FreedroidRPG or an indication, that the directory\n"
		                   "or file permissions of ~/.freedroid_rpg are somehow not right.",
		                   PLEASE_INFORM, filename);
		return;
	};
	file_size = file_info_buffer.st_size;
	strftime(info_string, sizeof(info_string), nl_langinfo(D_T_FMT), localtime(&(file_info_buffer.st_mtime)));

	put_string(get_current_font(), UNIVERSAL_COORD_W(240), GameConfig.screen_height - 3 * get_font_height(get_current_font()), _("Last Modified:"));
	put_string(get_current_font(), UNIVERSAL_COORD_W(240), GameConfig.screen_height - 2 * get_font_height(get_current_font()), info_string);

	// Now we compute the overall disk space of all files in question.
	// The saved .shp must exist.  On not, it's a sever error!

	sprintf(filename, "%s%s", core_filename, ".shp");
	if (!find_file(filepath, CONFIG_DIR, filename, NULL, NO_REPORT)) {
		error_once_message(ONCE_PER_RUN, __FUNCTION__,
		                   "FreedroidRPG was unable to access your saved game file (%s).\n"
		                   "This is either a bug in FreedroidRPG or an indication, that the directory\n"
		                   "or file permissions of ~/.freedroid_rpg are somehow not right.",
		                   PLEASE_INFORM, filename);
		return;
	}

	if (!stat(filepath, &file_info_buffer)) {
		file_size += file_info_buffer.st_size;
	}

	// A thumbnail may not yet exist.  We won't make much fuss if it doesn't.

	sprintf(filename, "%s%s", core_filename, SAVE_GAME_THUMBNAIL_EXT);
	if (find_file(filepath, CONFIG_DIR, filename, NULL, SILENT)) {
		if (!stat(filename, &(file_info_buffer))) {
			file_size += file_info_buffer.st_size;
		}
	}

	sprintf(info_string, _("File Size: %2.3f MB"), ((float)file_size) / (1024.0 * 1024.0));
	put_string(get_current_font(), UNIVERSAL_COORD_W(240), GameConfig.screen_height - 1 * get_font_height(get_current_font()), info_string);
}

/**
 * This function stores a thumbnail of the currently running game, so that
 * these thumbnails can be browsed when choosing which game to load.
 */
void save_thumbnail(void)
{
	char filepath[PATH_MAX];

	if (!strlen(data_dirs[CONFIG_DIR].path))
		return;

	find_file(filepath, CONFIG_DIR, Me.character_name, SAVE_GAME_THUMBNAIL_EXT, SILENT);

	AssembleCombatPicture(SHOW_ITEMS | NO_CURSOR);

	save_screenshot(filepath, 210);
}

/**
 * This function saves the current game of FreedroidRPG to a file.
 */

int save_game(void)
{
	char filepath[PATH_MAX];
	char backup_filepath[PATH_MAX];
	int ret;
	FILE *savegame_file;

	if (Me.energy <= 0) {
		alert_window(_("You are dead. Savegame not modified."));
		return (ERR);
	}

	if (!strlen(data_dirs[CONFIG_DIR].path))
		return (OK);

	/* Start with a 1MB string */
	savestruct_autostr = alloc_autostr(1048576);

	Activate_Conservative_Frame_Computation();

	find_file(filepath, CONFIG_DIR, Me.character_name, ".shp", SILENT);
	if (find_file(backup_filepath, CONFIG_DIR, Me.character_name, ".bkp.shp", SILENT))
		unlink(backup_filepath);
	ret = rename(filepath, backup_filepath);

	if (ret && errno != ENOENT) {
		error_message(__FUNCTION__, "Unable to create the shipfile backup. Errno: %d", PLEASE_INFORM, errno);
	}

	put_string_centered(Menu_Font, 10, _("Saving"));
	our_SDL_flip_wrapper();

	if (SaveShip(filepath, FALSE, 1) != OK) {
		error_message(__FUNCTION__,
		              "The SAVING OF THE SHIP DATA FOR THE SAVED GAME FAILED!\n"
		              "This is either a bug in FreedroidRPG or an indication, that the directory\n"
		              "or file permissions of ~/.freedroid_rpg are somehow not right.",
					  PLEASE_INFORM | IS_FATAL);
	}

	find_file(filepath, CONFIG_DIR, Me.character_name, SAVEDGAME_EXT, SILENT);
	if (find_file(backup_filepath, CONFIG_DIR, Me.character_name, ".bkp"SAVEDGAME_EXT, SILENT))
		unlink(backup_filepath);
	ret = rename(filepath, backup_filepath);

	if (ret && errno != ENOENT) {
		error_message(__FUNCTION__, "Unable to create the savegame backup. Errno: %d", PLEASE_INFORM, errno);
	}

	if ((savegame_file = fopen(filepath, "wb")) == NULL) {
		error_message(__FUNCTION__, "Error opening save game file for writing...\n\nTerminating...", IS_FATAL);
	}

	save_game_data(savestruct_autostr);

	deflate_to_stream((unsigned char *)savestruct_autostr->value, savestruct_autostr->length+1, savegame_file);
	fclose(savegame_file);

	save_thumbnail();

	append_new_game_message(_("Game saved."));

	free_autostr(savestruct_autostr);
	savestruct_autostr = NULL;

	return OK;
}

/**
 * This function delete a saved game and its backup (if there's one)
 */
int delete_game(void)
{
	char file_path[PATH_MAX];

	if (find_file(file_path, CONFIG_DIR, Me.character_name, ".shp", SILENT))
		remove(file_path);
	if (find_file(file_path, CONFIG_DIR, Me.character_name, SAVEDGAME_EXT, SILENT))
		remove(file_path);

	if (find_file(file_path, CONFIG_DIR, Me.character_name, SAVE_GAME_THUMBNAIL_EXT, SILENT))
		remove(file_path);

	// We do not check if the backup files exists before to remove it, but since
	// do not check the returned value of remove(), this is not an issue
	if (find_file(file_path, CONFIG_DIR, Me.character_name, ".bkp.shp", SILENT))
		remove(file_path);
	if (find_file(file_path, CONFIG_DIR, Me.character_name, ".bkp"SAVEDGAME_EXT, SILENT))
		remove(file_path);

	return (OK);
}

static int load_saved_game(int use_backup)
{
	char filepath[PATH_MAX];

	if (!strlen(data_dirs[CONFIG_DIR].path)) {
		return OK;
	}

	put_string_centered(Menu_Font, 10, _("Loading"));
	StoreMenuBackground(1);
	our_SDL_flip_wrapper();
	RestoreMenuBackground(1);

	Activate_Conservative_Frame_Computation();
	clean_error_msg_store();

	/*
	 * Load maps (still using the legacy format)
	 */

	find_file(filepath, CONFIG_DIR, Me.character_name, (use_backup) ? ".bkp.shp" : ".shp", SILENT);
	LoadShip(filepath, 1);

	/*
	 * Load data
	 */

	int loaded_size = 0;
	char *game_data = NULL;

	// Read savegame into a buffer

	find_file(filepath, CONFIG_DIR, Me.character_name, (use_backup) ? ".bkp"SAVEDGAME_EXT : SAVEDGAME_EXT, SILENT);
	FILE *data_file = fopen(filepath, "rb");

	if (inflate_stream(data_file, (unsigned char **)&game_data, &loaded_size)) {
		fclose(data_file);
		alert_window(_("Unable to decompress saved game - this is probably a very old, incompatible game. Sorry."));
		return ERR;
	}
	fclose(data_file);

	// Try to apply savegame converters

	int rtc = convert_old_savegame(&game_data, &loaded_size);
	if (rtc == FILTER_APPLIED) {
		// A fix/conversion was needed. Save the new data.
		data_file = fopen(filepath, "wb");
		deflate_to_stream((unsigned char *)game_data, loaded_size, data_file);
		fclose(data_file);
	} else if (rtc == FILTER_ABORT) {
		// Conversion was aborted.
		// However game_data was possibly changed by a filter function.
		// We reload the savegame and use it as is.
		free(game_data);
		data_file = fopen(filepath, "rb");
		int loaded_size = 0;
		inflate_stream(data_file, (unsigned char **)&game_data, &loaded_size);
		fclose(data_file);
	}

	// Load the savegame (lua format)

	clear_out_arrays_for_fresh_game();
	reset_lua_state();

	if (setjmp(saveload_jmpbuf)) {
		// We longjump here if load_game_data() detects an error
		if (game_data != NULL)
			free((char *)game_data);
		game_data = NULL;
		return (ERR);
	}

	load_game_data(game_data);
	free(game_data);
	game_data = NULL;

	/*
	 * Post-loading: Some transient states have to be adapted
	**/

	reset_visible_levels();
	get_visible_levels();
	animation_timeline_reset();
	switch_background_music(curShip.AllLevels[Me.pos.z]->Background_Song_Name);

	// Reset animation of dead bots

	enemy *erot;
	BROWSE_DEAD_BOTS(erot) {
		 // It is sufficient to set ->animation_type, since ->animation_phase will be set to
		 // last_death_animation_image[erot->type] inside animate_enemy();
		 //
		 // Also, we might not know the correct value of last_death_animation_image[] yet,
		 // since it's initialized by grab_enemy_images_from_archive(erot->type), and that
		 // function may not have been called for the current bot type.
		if (!level_is_visible(erot->pos.z))
			erot->animation_type = DEAD_ANIMATION;
	}

	// Count and initialize the number of droids used in this ship.
	// Otherwise we might ignore some robots.

	CountNumberOfDroidsOnShip();

	// Maybe someone just lost in the game and has then pressed the load
	// button.  Then a new game is loaded and the game-over status has
	// to be restored as well of course.

	GameOver = FALSE;

	// Now we know that right after loading an old saved game, the Tux might have
	// to 'change clothes' i.e. a lot of tux images need to be updated which can
	// take a little time.  Therefore we print some message so the user will not
	// panic and push the reset button :)

	put_string(FPS_Display_Font, 75, 150, _("Updating Tux images (this may take a little while...)"));
	our_SDL_flip_wrapper();

	/*
	 * We're now ready to start playing !
	**/
	widget_text_init(message_log, _("--- Message Log ---"));
	append_new_game_message(_("Game loaded."));

	return OK;
}

/**
 * This function loads an old saved game of FreedroidRPG from a file.
 */
int load_game(void)
{
	return load_saved_game(0);
}

/**
 * This loads the backup for the current player name
 */
int load_backup_game()
{
	return load_saved_game(1);
}

#undef _saveloadgame_c
