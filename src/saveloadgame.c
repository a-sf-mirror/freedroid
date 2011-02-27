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
#include "sys/stat.h"
#include "savestruct.h"

#include "scandir.h"

#ifdef HAVE_LANGINFO_H
#include <langinfo.h>
#endif

#ifndef HAVE_NL_LANGINFO
#define nl_langinfo(X) "%a %b %e %H:%M:%S %Y"
#endif

#define SAVEDGAME_EXT ".savegame"
#define SAVE_GAME_THUMBNAIL_EXT ".thumbnail.bmp"

jmp_buf saveload_jmpbuf;

#define WrapErrorMessage(a, b, c, d, ...) do { \
	ErrorMessage(a,b,c,IS_WARNING_ONLY, ##__VA_ARGS__);\
    	alert_window(_("An error occurred when trying to load the savegame.\nA common reason for this is that FreedroidRPG has been updated to a newer version since the save was made, in which case the savegame is very likely not compatible.\nIf you see this message and you have not updated the game, make sure to report this to the developers.\nThanks!"));\
        if (d==IS_FATAL)\
    	    longjmp(saveload_jmpbuf, 1);\
} while (0)

/**
 * Filter function for scandir calls.
 * This function keeps files with the ".savegame" extension.
 */
static int filename_filter_func(const struct dirent *file)
{
	char *pos = strstr(file->d_name, ".savegame");

	if (pos != NULL) {	// ".savegame" found
		if (strlen(pos) == 9) {	// since strlen(".savegame") is 9, then
			// d_name *ENDS* with ".savegame"

			if (strstr(file->d_name, ".bkp.savegame") + 4 == pos) {
				//then we have .bkp.savegame = filter it out
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
	char save_game_dir[1000];
	sprintf(save_game_dir, "%s/.freedroid_rpg", our_homedir);

	int n = scandir(save_game_dir, namelist, filename_filter_func, alphasort);

	if (n == -1)
	{
		ErrorMessage(__FUNCTION__, "Error occured while reading save game directory.",
					 NO_NEED_TO_INFORM, IS_WARNING_ONLY);
		return 0;
	}

	// For each element in list, remove the suffix ".savegame"
	int i;
	for (i = 0; i < n; i++) {
		*strstr((*namelist)[i]->d_name, ".savegame") = 0;
		if (!strlen((*namelist)[i]->d_name))
			strcpy((*namelist)[i]->d_name, "INVALID");
	}

	return n;
}

void LoadAndShowThumbnail(char *CoreFilename)
{
	char filename[1000];
	SDL_Surface *NewThumbnail;
	SDL_Rect TargetRectangle;

	if (!our_config_dir)
		return;

	sprintf(filename, "%s/%s%s", our_config_dir, CoreFilename, SAVE_GAME_THUMBNAIL_EXT);

	/* Load the image */
	NewThumbnail = our_IMG_load_wrapper(filename);
	if (NewThumbnail == NULL)
		return;

	/* Create a surface to hold the - correctly formatted - image */
	SDL_Surface *tmp = SDL_CreateRGBSurface(0, NewThumbnail->w, NewThumbnail->h, 32, rmask, gmask, bmask, amask);
	our_SDL_blit_surface_wrapper(NewThumbnail, NULL, tmp, NULL);
	SDL_FreeSurface(NewThumbnail);

	TargetRectangle.x = 10;
	TargetRectangle.y = GameConfig.screen_height - tmp->h - 10;

	if (use_open_gl) {
		flip_image_vertically(tmp);
	}

	our_SDL_blit_surface_wrapper(tmp, NULL, Screen, &TargetRectangle);

	SDL_FreeSurface(tmp);

};				// void LoadAndShowThumbnail ( char* CoreFilename )

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
		ErrorMessage(__FUNCTION__, "\
Freedroid was unable to determine the time of the last modification on\n\
your saved game file.\n\
This is either a bug in Freedroid or an indication, that the directory\n\
or file permissions of ~/.freedroid_rpg are somehow not right.", NO_NEED_TO_INFORM, IS_WARNING_ONLY);
		return;
	};

	LocalTimeSplitup = localtime(&(FileInfoBuffer.st_mtime));
	strftime(InfoString, sizeof(InfoString), nl_langinfo(D_T_FMT), LocalTimeSplitup);

	PutString(Screen, UNIVERSAL_COORD_W(240), GameConfig.screen_height - 3 * FontHeight(GetCurrentFont()), _("Last Modified:"));
	PutString(Screen, UNIVERSAL_COORD_W(240), GameConfig.screen_height - 2 * FontHeight(GetCurrentFont()), InfoString);

	// Now that the modification time has been set up, we can start to compute
	// the overall disk space of all files in question.
	//
	FileSize = FileInfoBuffer.st_size;

	// The saved ship must exist.  On not, it's a sever error!
	//    
	sprintf(filename, "%s/%s%s", our_config_dir, CoreFilename, ".shp");
	if (stat(filename, &(FileInfoBuffer))) {
		fprintf(stderr, "\n\nfilename: %s. \n", filename);
		ErrorMessage(__FUNCTION__, "\
Freedroid was unable to determine the time of the last modification on\n\
your saved game file.\n\
This is either a bug in Freedroid or an indication, that the directory\n\
or file permissions of ~/.freedroid_rpg are somehow not right.", NO_NEED_TO_INFORM, IS_FATAL);
	}
	FileSize += FileInfoBuffer.st_size;

	// A thumbnail may not yet exist.  We won't make much fuss if it doesn't.
	//
	sprintf(filename, "%s/%s%s", our_config_dir, CoreFilename, SAVE_GAME_THUMBNAIL_EXT);
	if (!stat(filename, &(FileInfoBuffer))) {
		FileSize += FileInfoBuffer.st_size;
	}

	sprintf(InfoString, _("File Size: %2.3f MB"), ((float)FileSize) / (1024.0 * 1024.0));

	PutString(Screen, UNIVERSAL_COORD_W(240), GameConfig.screen_height - 1 * FontHeight(GetCurrentFont()), InfoString);

};				// void LoadAndShowStats ( char* filename );

/**
 * This function stores a thumbnail of the currently running game, so that
 * these thumbnails can be browsed when choosing which game to load.
 */
void SaveThumbnailOfGame(void)
{
	char filename[1000];
	SDL_Surface *NewThumbnail = NULL;

	if (!our_config_dir)
		return;

	sprintf(filename, "%s/%s%s", our_config_dir, Me.character_name, SAVE_GAME_THUMBNAIL_EXT);

	AssembleCombatPicture(SHOW_ITEMS | NO_CURSOR);

	if (use_open_gl) {
#ifdef HAVE_LIBGL
		SDL_Surface *FullView;
		// We need to make a copy in processor memory. 
		GLvoid *imgdata = malloc((GameConfig.screen_width + 2) * (GameConfig.screen_height + 2) * 4);
		glReadPixels(0, 1, GameConfig.screen_width, GameConfig.screen_height - 1, GL_RGB, GL_UNSIGNED_BYTE, imgdata);

		// Now we need to make a real SDL surface from the raw image data we
		// have just extracted.
		//
		FullView =
		    SDL_CreateRGBSurfaceFrom(imgdata, GameConfig.screen_width, GameConfig.screen_height, 24, 3 * GameConfig.screen_width,
					     bmask, gmask, rmask, 0);

		NewThumbnail = zoomSurface(FullView, 0.32 * 640.0f / GameConfig.screen_width, 0.32 * 640.0f / GameConfig.screen_width, 0);

		if (NewThumbnail == NULL)
			return;

		flip_image_vertically(NewThumbnail);

		SDL_FreeSurface(FullView);
		free(imgdata);
#endif
	} else {
		NewThumbnail = zoomSurface(Screen, 0.32, 0.32, 0);
	}

	SDL_SaveBMP(NewThumbnail, filename);

	SDL_FreeSurface(NewThumbnail);

};				// void SaveThumbnailOfGame ( void )

/**
 * This function saves the current game of Freedroid to a file.
 */

int SaveGame(void)
{
	char filename[1000];
	char filename2[1000];
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

	sprintf(Me.savegame_version_string,
		"%s;sizeof(tux_t)=%d;sizeof(enemy)=%d;sizeof(bullet)=%d;MAXBULLETS=%d",
		VERSION, (int)sizeof(tux_t), (int)sizeof(enemy), (int)sizeof(bullet), (int)MAXBULLETS);

	sprintf(filename, "%s/%s%s", our_config_dir, Me.character_name, ".shp");
	sprintf(filename2, "%s/%s%s", our_config_dir, Me.character_name, ".bkp.shp");

	unlink(filename2);
	ret = rename(filename, filename2);

	if (ret && errno != ENOENT) {
		ErrorMessage(__FUNCTION__, "Unable to create the shipfile backup\n", PLEASE_INFORM, IS_WARNING_ONLY);
	}

	CenteredPutStringFont(Screen, Menu_BFont, 10, _("Saving"));
	our_SDL_flip_wrapper();

	if (SaveShip(filename, FALSE, 1) != OK) {
		ErrorMessage(__FUNCTION__, "\
The SAVING OF THE SHIP DATA FOR THE SAVED GAME FAILED!\n\
This is either a bug in Freedroid or an indication, that the directory\n\
or file permissions of ~/.freedroid_rpg are somehow not right.", PLEASE_INFORM, IS_FATAL);
	} else {
		DebugPrintf(SAVE_LOAD_GAME_DEBUG, "\nShip data for saved game seems to have been saved correctly.\n");
	}

	sprintf(filename, "%s/%s%s", our_config_dir, Me.character_name, ".savegame");
	sprintf(filename2, "%s/%s%s", our_config_dir, Me.character_name, ".bkp.savegame");

	unlink(filename2);
	ret = rename(filename, filename2);

	if (ret && errno != ENOENT) {
		ErrorMessage(__FUNCTION__, "Unable to create the savegame backup\n", PLEASE_INFORM, IS_WARNING_ONLY);
	}

	if ((SaveGameFile = fopen(filename, "wb")) == NULL) {
		printf(_("\n\nError opening save game file for writing...\n\nTerminating...\n\n"));
		Terminate(EXIT_FAILURE, TRUE);
	}

	/* Save tux */
	save_tux_t("player", &Me);

	/* Save all enemies */
	int i;
	int a = 0;
	for (i = 0; i < 2; i++) {
		enemy *erot;
		list_for_each_entry(erot, i ? &dead_bots_head : &alive_bots_head, global_list) {
			char str[20];
			sprintf(str, "%s enemy %d", i ? "dead" : "alive", a);
			save_enemy(str, erot);
			a++;
		}
	}

	/* Save NPCs */
	a = 0;
	struct npc *n;
	list_for_each_entry(n, &npc_head, node) {
		char str[20];
		sprintf(str, "npc %d", a);
		save_npc(str, n);
		a++;
	}

	/* Save all bullets */
	for (i = 0; i < MAXBULLETS; i++) {
		char str[20];
		sprintf(str, "blt%d", i);
		save_bullet(str, &AllBullets[i]);
	}

	/* Save melee shots */
	for (i = 0; i < MAX_MELEE_SHOTS; i++) {
		char str[20];
		sprintf(str, "ml%d", i);
		save_melee_shot(str, &AllMeleeShots[i]);
	}

	save_factions(savestruct_autostr);
	autostr_append(savestruct_autostr, "End of freedroidRPG savefile\n");
	deflate_to_stream((unsigned char *)savestruct_autostr->value, savestruct_autostr->length+1, SaveGameFile);
	fclose(SaveGameFile);

	SaveThumbnailOfGame();

	append_new_game_message(_("Game saved."));

	DebugPrintf(SAVE_LOAD_GAME_DEBUG, "\nint SaveGame( void ): end of function reached.");

	free_autostr(savestruct_autostr);
	savestruct_autostr = NULL;
	return OK;
};				// int SaveGame( void )

/**
 * This function loads an old saved game of Freedroid from a file.
 */
int DeleteGame(void)
{
	char filename[1000];

	if (!our_config_dir)
		return (OK);

	// First we save the full ship information, same as with the level editor
	//
	sprintf(filename, "%s/%s%s", our_config_dir, Me.character_name, ".shp");

	remove(filename);

	// First, we must determine the savedgame data file name
	//
	sprintf(filename, "%s/%s%s", our_config_dir, Me.character_name, SAVEDGAME_EXT);

	remove(filename);

	sprintf(filename, "%s/%s%s", our_config_dir, Me.character_name, SAVE_GAME_THUMBNAIL_EXT);

	remove(filename);

	return (OK);

};				// int DeleteGame( void )

/**
 * This loads the backup for the current player name
 */
int LoadBackupGame()
{
	strcat(Me.character_name, ".bkp");
	int ret = LoadGame();

	// Make sure the character name does not end in ".bkp". LoadGame() will
	// reset the name to the correct one, except when it failed.
	char *ptr = Me.character_name + strlen(Me.character_name) - 4;
	if (!strcmp(ptr, ".bkp"))
		*ptr = '\0';

	return ret;
}

/**
 * Load all enemies from a savegame
 */
static void load_enemies(char *game_data)
{
	enemy one_enemy;
	int done;
	int a = 0, i;

	clear_enemies();

	for (i = 0; i < 2; i++) {
		char *cpos = game_data;
		done = 0;
		while (!done) {
			char str[25];
			sprintf(str, "%s enemy %d", i ? "dead" : "alive", a);
			cpos = strstr(cpos, str);
			if (!cpos) {
				break;
			}
			cpos -= 5;
			memset(&one_enemy, 0, sizeof(enemy)); // all pointers have to be zeroed before to call read_xxx() functions
			if (read_enemy(cpos, str, &one_enemy)) {
				done = 1;
			} else {
				enemy *newen = enemy_new(one_enemy.type);
				free(newen->short_description_text);
				memcpy(newen, &one_enemy, sizeof(enemy));
				enemy_insert_into_lists(newen, (i==0));
			}
			a++;
		}
	}
}

static void load_npcs(char *LoadGameData)
{
	clear_npcs();

	struct npc *npc;
	char *cpos = LoadGameData;
	int a = 0, done = 0;
	while (!done) {
		npc = calloc(1, sizeof(struct npc));
		char str[25];
		sprintf(str, "npc %d", a);
		cpos = strstr(cpos, str);
		if (!cpos) {
			done = 1;
			free(npc);
			break;
		}
		cpos -= 5;
		if (read_npc(cpos, str, npc)) {
			done = 1;
			free(npc);
		} else {
			npc_insert(npc);
		}
		a++;
	}
}

static void load_bullets(char *LoadGameData)
{
	char *cpos = LoadGameData;
	int i;
	for (i = 0; i < MAXBULLETS; i++) {
		char str[20];
		sprintf(str, "blt%d", i);
		cpos = strstr(cpos, str);
		if (!cpos)
			break;
		cpos -= 5;
		read_bullet(cpos, str, &AllBullets[i]);
	}
}

/**
 * This function loads an old saved game of Freedroid from a file.
 */
int LoadGame(void)
{
	char version_check_string[1000];
	char *LoadGameData = NULL;
	char filename[1000];
	int i;
	FILE *DataFile;

	if (!our_config_dir) {
		DebugPrintf(0, "No Config-directory, cannot load any games\n");
		return (OK);
	}

	CenteredPutStringFont(Screen, Menu_BFont, 10, _("Loading"));
	our_SDL_flip_wrapper();

	DebugPrintf(SAVE_LOAD_GAME_DEBUG, "\n%s(): function call confirmed....", __FUNCTION__);

	Activate_Conservative_Frame_Computation();

	sprintf(filename, "%s/%s%s", our_config_dir, Me.character_name, ".shp");

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

	sprintf(filename, "%s/%s%s", our_config_dir, Me.character_name, SAVEDGAME_EXT);

	DataFile = fopen(filename, "rb");
	if (inflate_stream(DataFile, (unsigned char **)&LoadGameData, NULL)) {
		fclose(DataFile);
		alert_window("Unable to decompress saved game - this is probably an old, incompatible game. Sorry.");
		return ERR;
	}

	fclose(DataFile);

	init_tux();
	read_tux_t(LoadGameData, "player", &Me);

	reset_visible_levels();
	get_visible_levels();

	load_enemies(LoadGameData);
	load_npcs(LoadGameData);
	load_bullets(LoadGameData);
	load_factions(LoadGameData);

	/* post-loading: Some transient states have to be adapted */
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

	for (i = 0; i < MAXBULLETS; i++) {
		DeleteBullet(i, FALSE);
	}
	sprintf(version_check_string, "%s;sizeof(tux_t)=%d;sizeof(enemy)=%d;sizeof(bullet)=%d;MAXBULLETS=%d",
		VERSION, (int)sizeof(tux_t), (int)sizeof(enemy), (int)sizeof(bullet), (int)MAXBULLETS);

	if (strcmp(Me.savegame_version_string, version_check_string) != 0) {
		alert_window(_("Version or structsize mismatch! The savegame is not from the same version of FreedroidRPG... possible breakage."));
		our_SDL_flip_wrapper();
		while (!MouseLeftPressed())
			SDL_Delay(1);
	}
	// To prevent cheating, we remove all active spells, that might still be there
	// from other games just played before.
	//
	clear_active_spells();

	// Now that we have loaded the game, we must count and initialize the number
	// of droids used in this ship.  Otherwise we might ignore some robots.
	//
	CountNumberOfDroidsOnShip();

	SwitchBackgroundMusicTo(curShip.AllLevels[Me.pos.z]->Background_Song_Name);

	free((char *)LoadGameData);

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
	PutStringFont(Screen, FPS_Display_BFont, 75, 150, _("Updating Tux images (this may take a little while...)"));
	our_SDL_flip_wrapper();

	animation_timeline_reset();

	append_new_game_message(_("Game loaded."));
	return OK;
}

void read_enemy_ptr(const char *buffer, const char *tag, enemy ** val)
{
}

void read_int32_t(const char *buffer, const char *tag, int32_t * val)
{
	char search[strlen(tag) + 3];
	sprintf(search, "\n%s:", tag);
	char *pos = strstr(buffer, search);
	if (!pos)
		return;
	pos += 3 + strlen(tag);
	char *epos = pos;
	while (*epos != '\n')
		epos++;
	*epos = '\0';
	while (!isdigit(*pos) && *pos != '-')
		pos++;
	*val = strtol(pos, NULL, 10);
	*epos = '\n';
}

void read_int16_t(const char *buffer, const char *tag, int16_t * val)
{
	int32_t valt;
	read_int32_t(buffer, tag, &valt);
	*val = valt;
}

void read_char(const char *buffer, const char *tag, char *val)
{
	int32_t valt;
	read_int32_t(buffer, tag, &valt);
	*val = valt;
}

void read_uint32_t(const char *buffer, const char *tag, uint32_t * val)
{
	int32_t valt;
	read_int32_t(buffer, tag, &valt);
	*val = valt;
}

void read_uint16_t(const char *buffer, const char *tag, uint16_t * val)
{
	uint32_t valt;
	read_uint32_t(buffer, tag, &valt);
	*val = valt;
}

void read_uchar(const char *buffer, const char *tag, unsigned char *val)
{
	uint32_t valt;
	read_uint32_t(buffer, tag, &valt);
	*val = valt;
}

void read_double(const char *buffer, const char *tag, double *val)
{
	char search[strlen(tag) + 3];
	sprintf(search, "\n%s:", tag);
	char *pos = strstr(buffer, search);
	if (!pos)
		return;
	pos += 3 + strlen(tag);
	char *epos = pos;
	while (*epos != '\n')
		epos++;
	*epos = '\0';
	while (!isdigit(*pos) && *pos != '-')
		pos++;
	*val = strtod(pos, NULL);
	*epos = '\n';
}

void read_float(const char *buffer, const char *tag, float *val)
{
	double valt;
	read_double(buffer, tag, &valt);
	*val = valt;
}

void read_string(const char *buffer, const char *tag, string *val)
{
	char search[strlen(tag) + 3];
	char *pos, *epos;
	sprintf(search, "\n%s:", tag);
	pos = strstr(buffer, search);
	if (!pos)
		return;
	pos += 3 + strlen(tag);
	epos = pos;
	while (*epos != '\n')
		epos++;
	*epos = '\0';

	int size = strlen(pos);
    if (*val) {
		ErrorMessage(__FUNCTION__, "The target char * passed was not NULL - this means we are probably trying to overwrite an existing string which will lead to a memory leak.", PLEASE_INFORM, IS_WARNING_ONLY);
	}

	*val = MyMalloc(size + 1);

	strcpy(*val, pos);
	*epos = '\n';
}

void save_luacode(const char *tag, luacode * val)
{
	if (*val)
		autostr_append(savestruct_autostr, "%s:LuaCode={%s}\n", tag, *val);
}

void read_luacode(const char *buffer, const char *tag, luacode * val)
{
	if (*val) {
		free(*val);
		*val = NULL;
	}

	char search[strlen(tag) + 3];
	sprintf(search, "\n%s:LuaCode={", tag);
	char *pos = strstr(buffer, search);
	if (!pos)
		return;
	while (*pos != '{')
		pos++;
	pos++;
	char *epos = pos;
	while (*epos != '}' || *(epos + 1) != '\n')
		epos++;
	*epos = '\0';
	*val = malloc(strlen(pos) + 1);
	strcpy(*val, pos);
	*epos = '\n';
}

/* Save arrays of simple types */
#define define_save_xxx_array(X) void save_##X##_array(const char *tag, X *val_ar, int size)\
{\
	autostr_append(savestruct_autostr, "<%s array n=%d>\n", tag, size);\
	int i;\
	for (i = 0; i < size; i++) {\
		char str[strlen(tag) + 20];\
		sprintf(str, "%s elem %d", tag, i);\
		save_##X(str, &val_ar[i]);\
	}\
	autostr_append(savestruct_autostr, "</%s>\n", tag);\
}

/* Save dynamic arrays of simple types.
 * Requires you to first use define_save_xxx_array for the type.
 */
#define define_save_xxx_dynarray(X) void save_##X##_dynarray(const char *tag, X##_dynarray *array)\
{\
	save_##X##_array(tag, array->arr, array->size);\
}

define_save_xxx_array(int32_t);
define_save_xxx_array(uint32_t);
define_save_xxx_array(int16_t);
define_save_xxx_array(uint16_t);
typedef unsigned char uchar;
define_save_xxx_array(float);
define_save_xxx_array(uchar);
define_save_xxx_array(char);
define_save_xxx_array(string);
define_save_xxx_array(mission);
define_save_xxx_array(upgrade_socket);
define_save_xxx_dynarray(upgrade_socket);
define_save_xxx_dynarray(item);
define_save_xxx_array(item);
define_save_xxx_array(gps);
define_save_xxx_array(moderately_finepoint);

/**
 * \brief Reads the number of elements stored to a generic savestruct array.
 * \param buffer String from which to read.
 * \param tag Tag name of the array.
 * \return The size of the array.
 */
int read_array_size(const char *buffer, const char *tag)
{
	// Find the beginning element.
	char search[strlen(tag) + 20];
	sprintf(search, "<%s array", tag);
	char *pos = strstr(buffer, search);
	if (!pos) WrapErrorMessage(__FUNCTION__, "Unable to find array %s\n", PLEASE_INFORM, IS_FATAL, tag);

	// Find the size string.
	char *runp = pos + 1;
	runp += strlen(tag) + 1 + strlen("array") + 1;
	while (!isdigit(*runp)) runp++;

	// Convert it to an integer.
	return atoi(runp);
}

/* Read arrays of simple types */
#define define_read_xxx_array(X) void read_##X##_array(char *buffer, const char *tag, X *val_ar, int size)\
{\
	int i;\
	/* Find the beginning element. */\
	char search[strlen(tag) + 20];\
	sprintf(search, "<%s array", tag);\
	char * pos = strstr(buffer, search);\
	if (!pos) WrapErrorMessage(__FUNCTION__, "Unable to find array %s\n", PLEASE_INFORM, IS_FATAL, tag);\
	sprintf(search, "</%s>", tag);\
	/* Find the end element. */\
	char * epos = strstr(pos, search);\
	if ( ! epos ) WrapErrorMessage(__FUNCTION__, "Unable to find array end %s\n", PLEASE_INFORM, IS_FATAL, tag);\
	epos += strlen(search);\
	/* Check for valid size. */\
	int nb = read_array_size(pos, tag);\
	if ( nb != size ) WrapErrorMessage(__FUNCTION__, "Size mismatch for array %s, %d in file, %d in game\n", NO_NEED_TO_INFORM, IS_WARNING_ONLY, tag, nb, size);\
	/* Overwrite the last character of the end element with NUL. */\
	char savechar = *(epos + 1);\
	*(epos+1) = '\0';\
	/* Read array elements. */\
	for (i = 0; i < (nb <= size ? nb : size); i++) {\
		char str[strlen(tag) + 20];\
		sprintf(str, "%s elem %d", tag, i);\
		pos = strstr(pos, str);\
		if (!pos) break;\
		pos -= 5;\
		read_##X(pos, str, &val_ar[i]);\
	}\
	/* Restore the last character of the end element. */\
	*(epos+1) = savechar;\
}

/* Read dynamic arrays of simple types
 * Requires you to first use define_read_xxx_array for the type.
 */
#define define_read_xxx_dynarray(X) void read_##X##_dynarray(char *buffer, const char *tag, X##_dynarray *array)\
{\
	/* Read the size. */\
	int size = read_array_size(buffer, tag);\
	/* Allocate space. */\
	dynarray_init((struct dynarray *) array, size, sizeof(X));\
	array->size = size;\
	/* Read the array elements. */\
	read_##X##_array(buffer, tag, array->arr, array->size);\
}

define_read_xxx_array(int32_t);
define_read_xxx_array(uint32_t);
define_read_xxx_array(int16_t);
define_read_xxx_array(uint16_t);
define_read_xxx_array(float);
define_read_xxx_array(uchar);
define_read_xxx_array(char);
define_read_xxx_array(string);
define_read_xxx_array(mission);
define_read_xxx_array(upgrade_socket);
define_read_xxx_dynarray(upgrade_socket);
define_read_xxx_array(item);
define_read_xxx_dynarray(item);
define_read_xxx_array(gps);
define_read_xxx_array(moderately_finepoint);

void save_sdl_rect(const char *tag, SDL_Rect * target)
{
	autostr_append(savestruct_autostr, "<%s>\n", tag);
	save_int16_t("x", &(target->x));
	save_int16_t("y", &(target->y));
	save_uint16_t("w", &(target->w));
	save_uint16_t("h", &(target->h));
	autostr_append(savestruct_autostr, "</%s>\n", tag);
}

int read_sdl_rect(const char *buffer, const char *tag, SDL_Rect * target)
{
	char search[strlen(tag) + 5];
	sprintf(search, "<%s>", tag);
	char *pos = strstr(buffer, search);
	if (!pos)
		return 1;
	pos += 1 + strlen(tag);
	sprintf(search, "</%s>", tag);
	char *epos = strstr(buffer, search);
	if (!epos)
		return 2;
	*epos = 0;
	read_int16_t(pos, "x", &(target->x));
	read_int16_t(pos, "y", &(target->y));
	read_uint16_t(pos, "w", &(target->w));
	read_uint16_t(pos, "h", &(target->h));
	*epos = '>';
	return 0;
}

void save_keybind_t_array(const char *tag, keybind_t * keybinds, int size)
{
	autostr_append(savestruct_autostr, "<keybinds cmd=%d>\n", size);
	int i;
	for (i = 0; i < size; i++) {
		if (!strcmp(keybinds[i].name, "end"))
			break;

		autostr_append(savestruct_autostr, "%s %d %d\n", keybinds[i].name, keybinds[i].key, keybinds[i].mod);
	}
	autostr_append(savestruct_autostr, "</keybinds>\n");
}

void read_keybind_t_array(const char *buffer, const char *tag, keybind_t * kbs, int size)
{
	int tknb;
	char *pos = strstr(buffer, "<keybinds cmd=");
	if (!pos) {
		WrapErrorMessage(__FUNCTION__, "Unable to find keybinds.\n", NO_NEED_TO_INFORM, IS_WARNING_ONLY);
		return;
	}
	char *epos = strstr(pos, "</keybinds>\n");
	if (!epos) {
		WrapErrorMessage(__FUNCTION__, "Unable to find keybinds end.\n", PLEASE_INFORM, IS_WARNING_ONLY);
		return;
	}
	char savechar = *epos;
	*epos = '\0';
	int mc = 0;
	char tmpname[100];
	int tmpcode = 0;
	int tmpmod;

	pos += strlen("<keybinds cmd=");
	while (!isdigit(*pos))
		pos++;
	char *erunp = pos;
	while ((*erunp) != '>')
		erunp++;
	*erunp = '\0';
	mc = atoi(pos);
	*erunp = '>';
	pos = erunp;

	if (mc < size)
		WrapErrorMessage(__FUNCTION__, "Config file defines %d keybindings, game supports only %d.\n", PLEASE_INFORM, IS_FATAL, mc,
				 size);

	while (*pos != '\n')
		pos++;
	pos++;

	strtok(pos, " \n");
	tknb = 0;
	do {
		switch (tknb) {
		case 0:	/*key name */
			strncpy(tmpname, pos, 99);
			tmpname[99] = 0;
			tknb++;
			break;
		case 1:	/*key code */
			tmpcode = atoi(pos);
			tknb++;
			break;
		case 2:	/*key mod */
			tmpmod = atoi(pos);

			input_set_keybind(tmpname, tmpcode, tmpmod);
			tknb = 0;
			break;
		}
	} while ((pos = strtok(NULL, " \n")));

	*epos = savechar;
}

void save_automap_data(const char *tag, automap_data_t * automapdata, int size)
{
	autostr_append(savestruct_autostr, "<automap nl=%d sx=%d sy=%d>\n", MAX_LEVELS, 100, 100);
	int i, j, k;
	for (i = 0; i < MAX_LEVELS; i++) {
		autostr_append(savestruct_autostr, "%d\n", i);
		for (j = 0; j < 100; j++) {
			for (k = 0; k < 100; k += 5)
				autostr_append(savestruct_autostr, "%hhd %hhd %hhd %hhd %hhd ", automapdata[i][j][k], automapdata[i][j][k + 1],
					automapdata[i][j][k + 2], automapdata[i][j][k + 3], automapdata[i][j][k + 4]);
			autostr_append(savestruct_autostr, "\n");
		}
	}
	autostr_append(savestruct_autostr, "</automap>\n");
}

void read_automap_data_t_array(char *buffer, char *tag, automap_data_t * automapdata, int size)
{
	char *pos = strstr(buffer, "<automap nl=");
	if (!pos)
		WrapErrorMessage(__FUNCTION__, "Unable to find automap data\n", PLEASE_INFORM, IS_FATAL);
	char *epos = strstr(pos, "</automap>\n");
	if (!epos)
		WrapErrorMessage(__FUNCTION__, "Unable to find automap data end\n", PLEASE_INFORM, IS_FATAL);
	epos += strlen("</automap>\n");
	char savechar = *(epos + 1);
	*(epos + 1) = '\0';
	char *runp = pos + 1;
	runp += strlen("<automap");

	ClearAutomapData();

	while (*runp != '\n')
		runp++;
	*runp = '\0';
	int nl, sx, sy;
	sscanf(pos, "<automap nl=%d sx=%d sy=%d>", &nl, &sx, &sy);

	if (nl != MAX_LEVELS)
		WrapErrorMessage(__FUNCTION__, "Number of levels mismatch when reading automap data : file %d, game %d\n",
				 NO_NEED_TO_INFORM, IS_WARNING_ONLY, nl, MAX_LEVELS);
	if (sx != 100 || sy != 100)
		WrapErrorMessage(__FUNCTION__, "Size mismatch when reading automap data.\n", PLEASE_INFORM, IS_FATAL);

	*runp = '\n';
	runp++;
	char *erunp;
	int i = 0, j = 0, k = 0;
	while (i < nl) {
		/* we're on level number */
		while (*runp != '\n')
			runp++;
		runp++;
		/*now on the actual data */
		j = 0;
		while (j < 100) {
			k = 0;
			while (k < 100) {
				while (!isdigit(*runp))
					runp++;
				erunp = runp + 1;
				while (*erunp != ' ')
					erunp++;
				*erunp = '\0';

				(automapdata)[i][j][k] = atoi(runp);
				*erunp = ' ';
				runp = erunp;
				k++;
			}
			while (*runp != '\n')
				runp++;
			runp++;
			j++;
		}

		while (*runp != '\n')
			runp++;
		i++;
	}
	*(epos + 1) = savechar;

}

#undef _saveloadgame_c
