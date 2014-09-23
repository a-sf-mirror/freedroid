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

#define _saveloadgame_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include <sys/stat.h>
#include "widgets/widgets.h"
#include "savestruct.h"

#include "scandir.h"

#ifdef HAVE_LANGINFO_H
#include <langinfo.h>
#endif

#ifndef HAVE_NL_LANGINFO
#define nl_langinfo(X) "%a %b %e %H:%M:%S %Y"
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
	int n = scandir(our_config_dir, namelist, filename_filter_func, alphasort);

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

void LoadAndShowThumbnail(char *CoreFilename)
{
	char filename[1000];
	struct image thumbnail = EMPTY_IMAGE;
	SDL_Rect TargetRectangle;

	if (!our_config_dir)
		return;

	sprintf(filename, "%s/%s%s", our_config_dir, CoreFilename, SAVE_GAME_THUMBNAIL_EXT);

	/* Load the image */
	thumbnail.surface = our_IMG_load_wrapper(filename);
	if (!thumbnail.surface)
		return;

	thumbnail.w = thumbnail.surface->w;
	thumbnail.h = thumbnail.surface->h;
	if (use_open_gl)
		make_texture_out_of_surface(&thumbnail);

	TargetRectangle.x = 10;
	TargetRectangle.y = GameConfig.screen_height - thumbnail.h - 10;

	display_image_on_screen(&thumbnail, TargetRectangle.x, TargetRectangle.y, IMAGE_NO_TRANSFO);

	delete_image(&thumbnail);
}

/**
 * 
 *
 */
void LoadAndShowStats(char *CoreFilename)
{
	char filename[1000];
	struct stat FileInfoBuffer;
	char InfoString[5000];
	struct tm *LocalTimeSplitup;
	long int FileSize;

	if (!our_config_dir)
		return;

	DebugPrintf(2, "\nTrying to get file stats for character '%s'. ", CoreFilename);

	// First we save the full ship information, same as with the level editor
	//

	sprintf(filename, "%s/%s%s", our_config_dir, CoreFilename, SAVEDGAME_EXT);

	if (stat(filename, &(FileInfoBuffer))) {
		fprintf(stderr, "\n\nfilename: %s. \n", filename);
		error_message(__FUNCTION__, "\
FreedroidRPG was unable to determine the time of the last modification on\n\
your saved game file.\n\
This is either a bug in FreedroidRPG or an indication, that the directory\n\
or file permissions of ~/.freedroid_rpg are somehow not right.", NO_REPORT);
		return;
	};

	LocalTimeSplitup = localtime(&(FileInfoBuffer.st_mtime));
	strftime(InfoString, sizeof(InfoString), nl_langinfo(D_T_FMT), LocalTimeSplitup);

	put_string(GetCurrentFont(), UNIVERSAL_COORD_W(240), GameConfig.screen_height - 3 * FontHeight(GetCurrentFont()), _("Last Modified:"));
	put_string(GetCurrentFont(), UNIVERSAL_COORD_W(240), GameConfig.screen_height - 2 * FontHeight(GetCurrentFont()), InfoString);

	// Now that the modification time has been set up, we can start to compute
	// the overall disk space of all files in question.
	//
	FileSize = FileInfoBuffer.st_size;

	// The saved ship must exist.  On not, it's a sever error!
	//    
	sprintf(filename, "%s/%s%s", our_config_dir, CoreFilename, ".shp");
	if (stat(filename, &(FileInfoBuffer))) {
		fprintf(stderr, "\n\nfilename: %s. \n", filename);
		error_message(__FUNCTION__, "\
FreedroidRPG was unable to determine the time of the last modification on\n\
your saved game file.\n\
This is either a bug in FreedroidRPG or an indication, that the directory\n\
or file permissions of ~/.freedroid_rpg are somehow not right.", IS_FATAL);
	}
	FileSize += FileInfoBuffer.st_size;

	// A thumbnail may not yet exist.  We won't make much fuss if it doesn't.
	//
	sprintf(filename, "%s/%s%s", our_config_dir, CoreFilename, SAVE_GAME_THUMBNAIL_EXT);
	if (!stat(filename, &(FileInfoBuffer))) {
		FileSize += FileInfoBuffer.st_size;
	}

	sprintf(InfoString, _("File Size: %2.3f MB"), ((float)FileSize) / (1024.0 * 1024.0));

	put_string(GetCurrentFont(), UNIVERSAL_COORD_W(240), GameConfig.screen_height - 1 * FontHeight(GetCurrentFont()), InfoString);

};				// void LoadAndShowStats ( char* filename );

/**
 * This function stores a thumbnail of the currently running game, so that
 * these thumbnails can be browsed when choosing which game to load.
 */
void SaveThumbnailOfGame(void)
{
	char filename[1000];
	if (!our_config_dir)
		return;

	sprintf(filename, "%s/%s%s", our_config_dir, Me.character_name, SAVE_GAME_THUMBNAIL_EXT);

	AssembleCombatPicture(SHOW_ITEMS | NO_CURSOR);

	save_screenshot(filename, 210);
};				// void SaveThumbnailOfGame ( void )

/**
 * This function saves the current game of FreedroidRPG to a file.
 */

int SaveGame(void)
{
	char filename[1000];
	char filename2[1000];
	struct auto_string *version_string;
	int ret;
	FILE *SaveGameFile;

	if (Me.energy <= 0) {
		alert_window(_("You are dead. Savegame not modified."));
		return (ERR);
	}

	if (!our_config_dir)
		return (OK);

	/* Start with a 1MB string */
	savestruct_autostr = alloc_autostr(1048576);

	Activate_Conservative_Frame_Computation();

	version_string = alloc_autostr(256);
	autostr_printf(version_string,
		"%s;sizeof(tux_t)=%d;sizeof(enemy)=%d;sizeof(bullet)=%d;MAXBULLETS=%d",
		VERSION, (int)sizeof(tux_t), (int)sizeof(enemy), (int)sizeof(bullet), (int)MAXBULLETS);
	free(Me.savegame_version_string);
	Me.savegame_version_string = strdup(version_string->value);

	sprintf(filename, "%s/%s%s", our_config_dir, Me.character_name, ".shp");
	sprintf(filename2, "%s/%s%s", our_config_dir, Me.character_name, ".bkp.shp");

	unlink(filename2);
	ret = rename(filename, filename2);

	if (ret && errno != ENOENT) {
		error_message(__FUNCTION__, "Unable to create the shipfile backup", PLEASE_INFORM);
	}

	put_string_centered(Menu_BFont, 10, _("Saving"));
	our_SDL_flip_wrapper();

	if (SaveShip(filename, FALSE, 1) != OK) {
		error_message(__FUNCTION__, "\
The SAVING OF THE SHIP DATA FOR THE SAVED GAME FAILED!\n\
This is either a bug in FreedroidRPG or an indication, that the directory\n\
or file permissions of ~/.freedroid_rpg are somehow not right.", PLEASE_INFORM | IS_FATAL);
	} else {
		DebugPrintf(SAVE_LOAD_GAME_DEBUG, "\nShip data for saved game seems to have been saved correctly.\n");
	}

	sprintf(filename, "%s/%s%s", our_config_dir, Me.character_name, SAVEDGAME_EXT);
	sprintf(filename2, "%s/%s%s", our_config_dir, Me.character_name, ".bkp"SAVEDGAME_EXT);

	unlink(filename2);
	ret = rename(filename, filename2);

	if (ret && errno != ENOENT) {
		error_message(__FUNCTION__, "Unable to create the savegame backup", PLEASE_INFORM);
	}

	if ((SaveGameFile = fopen(filename, "wb")) == NULL) {
		error_message(__FUNCTION__, "Error opening save game file for writing...\n\nTerminating...", IS_FATAL);
	}

	save_game_data(savestruct_autostr);

	deflate_to_stream((unsigned char *)savestruct_autostr->value, savestruct_autostr->length+1, SaveGameFile);
	fclose(SaveGameFile);

	SaveThumbnailOfGame();

	append_new_game_message(_("Game saved."));

	DebugPrintf(SAVE_LOAD_GAME_DEBUG, "\nint SaveGame( void ): end of function reached.");

	free_autostr(version_string);
	free_autostr(savestruct_autostr);
	savestruct_autostr = NULL;
	return OK;
};				// int SaveGame( void )

/**
 * This function delete a saved game and its backup (if there's one)
 */
int DeleteGame(void)
{
	char filename[FILENAME_MAX];

	if (!our_config_dir)
		return (OK);

	snprintf(filename, sizeof(filename), "%s/%s%s", our_config_dir, Me.character_name, ".shp");
	remove(filename);
	snprintf(filename, sizeof(filename), "%s/%s%s", our_config_dir, Me.character_name, SAVEDGAME_EXT);
	remove(filename);

	snprintf(filename, sizeof(filename), "%s/%s%s", our_config_dir, Me.character_name, SAVE_GAME_THUMBNAIL_EXT);
	remove(filename);

	// We do not check if the backup files exists before to remove it, but since
	// do not check the returned value of remove(), this is not an issue
	snprintf(filename, sizeof(filename), "%s/%s%s", our_config_dir, Me.character_name, ".bkp.shp");
	remove(filename);
	snprintf(filename, sizeof(filename), "%s/%s%s", our_config_dir, Me.character_name, ".bkp"SAVEDGAME_EXT);
	remove(filename);

	return (OK);
}

static int load_saved_game(int use_backup)
{
	char version_check_string[1000];
	char *LoadGameData = NULL;
	char filename[1000], prefix[1000];
	FILE *DataFile;

	if (!our_config_dir) {
		DebugPrintf(0, "No Configuration-directory, cannot load any games\n");
		return (OK);
	}

	clean_error_msg_store();

	put_string_centered(Menu_BFont, 10, _("Loading"));
	our_SDL_flip_wrapper();

	DebugPrintf(SAVE_LOAD_GAME_DEBUG, "\n%s(): function call confirmed....", __FUNCTION__);

	Activate_Conservative_Frame_Computation();
	// Initialize Lua state correctly
	reset_lua_state();

	sprintf(prefix, "%s/%s", our_config_dir, Me.character_name);
	if (use_backup) {
		strcat(prefix, ".bkp");
	}

	sprintf(filename, "%s%s", prefix, ".shp");

	if ((DataFile = fopen(filename, "rb")) == NULL) {
		alert_window(_("W A R N I N G !\n\nFreedroidRPG was unable to locate the saved game file you requested to load.\nThis might mean that it really isn't there cause you tried to load a game without ever having saved the game before.\nThe other explanation of this error might be a severe error in FreedroidRPG. If you think this is the case, please report this to the developers."));
		append_new_game_message(_("Failed to load old game."));
		return (ERR);
	} else {
		fclose(DataFile);
		DebugPrintf(1, "\nThe saved game file (.shp file) seems to be there at least.....");
	}
	if (setjmp(saveload_jmpbuf)) {
		if (LoadGameData != NULL)
			free((char *)LoadGameData);
		LoadGameData = NULL;
		return (ERR);
	}

	LoadShip(filename, 1);

	sprintf(filename, "%s%s", prefix, SAVEDGAME_EXT);
	DataFile = fopen(filename, "rb");
	if (inflate_stream(DataFile, (unsigned char **)&LoadGameData, NULL)) {
		fclose(DataFile);
		alert_window("Unable to decompress saved game - this is probably an old, incompatible game. Sorry.");
		return ERR;
	}
	fclose(DataFile);

	clear_out_arrays_for_fresh_game();
	load_game_data(LoadGameData);

	free(LoadGameData);
	LoadGameData = NULL;

	sprintf(version_check_string, "%s;sizeof(tux_t)=%d;sizeof(enemy)=%d;sizeof(bullet)=%d;MAXBULLETS=%d",
		VERSION, (int)sizeof(tux_t), (int)sizeof(enemy), (int)sizeof(bullet), (int)MAXBULLETS);

	if (strcmp(Me.savegame_version_string, version_check_string) != 0) {
		alert_once_window(ONCE_PER_GAME, _("Version or structsize mismatch! The savegame is not from the same version of FreedroidRPG... possible breakage."));
	}

	/* post-loading: Some transient states have to be adapted */
	reset_visible_levels();
	get_visible_levels();

	enemy *erot;
	BROWSE_DEAD_BOTS(erot) {
		/*
		 * It is sufficient to set ->animation_type, since ->animation_phase will be set to
		 * last_death_animation_image[erot->type] inside animate_enemy();
		 *
		 * Also, we might not know the correct value of last_death_animation_image[] yet,
		 * since it's initialized by grab_enemy_images_from_archive(erot->type), and that
		 * function may not have been called for the current bot type.
		 */
		if (!level_is_visible(erot->pos.z))
			erot->animation_type = DEAD_ANIMATION;
	}

	// Now that we have loaded the game, we must count and initialize the number
	// of droids used in this ship.  Otherwise we might ignore some robots.
	//
	CountNumberOfDroidsOnShip();

	SwitchBackgroundMusicTo(curShip.AllLevels[Me.pos.z]->Background_Song_Name);

	// Maybe someone just lost in the game and has then pressed the load
	// button.  Then a new game is loaded and the game-over status has
	// to be restored as well of course.
	//
	GameOver = FALSE;

	DebugPrintf(SAVE_LOAD_GAME_DEBUG, "\n%s(): end of function reached.", __FUNCTION__);

	// Now we know that right after loading an old saved game, the Tux might have
	// to 'change clothes' i.e. a lot of tux images need to be updated which can
	// take a little time.  Therefore we print some message so the user will not
	// panic and push the reset button :)
	//
	put_string(FPS_Display_BFont, 75, 150, _("Updating Tux images (this may take a little while...)"));
	our_SDL_flip_wrapper();

	animation_timeline_reset();
	widget_text_init(message_log, _("--- Message Log ---"));
	append_new_game_message(_("Game loaded."));
	return OK;
}

/**
 * This function loads an old saved game of FreedroidRPG from a file.
 */
int LoadGame(void)
{
	return load_saved_game(0);
}

/**
 * This loads the backup for the current player name
 */
int LoadBackupGame()
{
	return load_saved_game(1);
}

#undef _saveloadgame_c
