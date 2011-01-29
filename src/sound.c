/*
 *
 *   Copyright (c) 2006 Arvid Picciani
 *   Copyright (c) 2004-2010 Arthur Huillet 
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
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

/**
 * This file contains sound related functions
 */

#ifndef _sound_c
#define _sound_c
#endif

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#define MAX_SOUNDS_IN_DYNAMIC_WAV_CACHE 100
#define ALL_MOD_MUSICS 1
#define MAX_SOUND_CHANNELS 5000

static void remove_all_samples_from_WAV_cache(void);
static void LoadAndFadeInBackgroundMusic(void);

/** ============================================  DUMMYS  ============================================  */
/*
these functions are defined if there is no sound. they do nothing
*/

#ifndef HAVE_LIBSDL_MIXER
	/**system*/
void InitAudio(void)
{
};

void SetBGMusicVolume(float NewVolume)
{
}

void SetSoundFXVolume(float NewVolume)
{
}

	/**functions for threading*/
void channel_done(int channel)
{
}

void OldMusicHasFinishedFadingOut(void)
{
}

	/**music*/
static void LoadAndFadeInBackgroundMusic(void)
{
}

void SwitchBackgroundMusicTo(char *filename_raw_parameter)
{
}

	/**sample functions*/
void play_sound_cached(const char *filename)
{
}

void play_sound_cached_v(const char *filename, double volume)
{
}

static void remove_all_samples_from_WAV_cache(void)
{
}

void play_sound(const char *SoundSampleFileName)
{
}

void play_sound_directly(const char *filename)
{
}
#endif

#ifdef HAVE_LIBSDL_MIXER

/** ============================================ GLOBALS ============================================  */

static Mix_Chunk *dynamic_WAV_cache[MAX_SOUNDS_IN_DYNAMIC_WAV_CACHE];
static char *sound_names_in_dynamic_wav_chache[MAX_SOUNDS_IN_DYNAMIC_WAV_CACHE];
static char channel_must_be_freed[MAX_SOUND_CHANNELS];

static Mix_Chunk *wav_files_to_free[MAX_SOUND_CHANNELS];
static Mix_Music *Loaded_MOD_Files[ALL_MOD_MUSICS] = {
	NULL
};

enum {
	NOTHING_PLAYING_AT_ALL = 3,
	FADING_IN = 4,
	FADING_OUT = 5
};

static int BackgroundMusicStateMachineState = NOTHING_PLAYING_AT_ALL;
static char NewMusicTargetFileName[5000];

// This variable refers to the next free position inside the WAV file cache.
static int next_free_position_in_cache = 0;

/** ============================================ SYSTEM ============================================ */
// ----------------------------------------------------------------------
// This function shall initialize the SDL Audio subsystem.  It is called
// as soon as Freedroid is started.  It does ONLY work with SDL and no
// longer with any form of sound engine like the YIFF.
// ----------------------------------------------------------------------
void InitAudio(void)
{
	int audio_rate = 44100;
	Uint16 audio_format = AUDIO_S16;
	int audio_channels = 2;
	int audio_buffers = 4096;

	DebugPrintf(1, "\nInitializing SDL Audio Systems....\n");

	if (!sound_on)
		return;

	// Now SDL_AUDIO is initialized here:

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
		fprintf(stderr, "\n\nSDL just reported a problem.\n\
				 The error string from SDL_GetError\nwas: %s \n\
				 The error string from the SDL mixer subsystem was: %s \n", SDL_GetError(), Mix_GetError());
		ErrorMessage(__FUNCTION__, "\
								   The SDL AUDIO SUBSYSTEM COULD NOT BE INITIALIZED.\n\
								   \n\
								   Please check that your sound card is properly configured,\n\
								   i.e. if other applications are able to play sounds.\n\
								   \n\
								   If you for some reason cannot get your sound card ready, \n\
								   you can choose to play without sound.\n\
								   \n\
								   If you want this, use the appropriate command line option and Freedroid will \n\
								   not complain any more.", NO_NEED_TO_INFORM, IS_WARNING_ONLY);
		sound_on = FALSE;
		return;
	} else {
		DebugPrintf(1, "\nSDL Audio initialisation successful.\n");
	}

	// Now that we have initialized the audio SubSystem, we must open
	// an audio channel.  This will be done here (see code from Mixer-Tutorial):
	//
	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
		fprintf(stderr, "\n\nSDL just reported a problem.\n\
				 The error string from SDL_GetError\nwas: %s \n\
				 The error string from the SDL mixer subsystem was: %s \n", SDL_GetError(), Mix_GetError());
		ErrorMessage(__FUNCTION__, "\
								   The SDL AUDIO CHANNEL COULD NOT BE OPEND.\n\
								   \n\
								   Please check that your sound card is properly configured,\n\
								   i.e. if other applications are able to play sounds.\n\
								   \n\
								   If you for some reason cannot get your sound card ready, \n\
								   you can choose to play without sound.\n\
								   \n\
								   If you want this, use the appropriate command line option and Freedroid will \n\
								   not complain any more.", NO_NEED_TO_INFORM, IS_WARNING_ONLY);
		sound_on = FALSE;
		return;
	} else {
		DebugPrintf(1, "\nSuccessfully opened SDL audio channel.");
	}

	// Since we don't want some sounds to be omitted due to lack of mixing
	// channels, we select to have some at our disposal.  The SDL will do this
	// for a small increase in memory appetite as the price.  Whether this will
	// really resolve the problem however is unsure.
	//
	DebugPrintf(1, "\nChannels allocated: %d. ", Mix_AllocateChannels(200));
};				// void InitAudio(void)

void SetBGMusicVolume(float NewVolume)
{
	if (!sound_on)
		return;
	Mix_VolumeMusic((int)rintf(NewVolume * MIX_MAX_VOLUME));
	// SwitchBackgroundMusicTo ( COMBAT_BACKGROUND_MUSIC_SOUND );
}

void SetSoundFXVolume(float NewVolume)
{
	if (!sound_on)
		return;
	remove_all_samples_from_WAV_cache();
}

/** ================================= FUNCTIONS FOR THREADING ======================================== */

/**
 * When play_sound() has been called, we want to free the memory once it is done
 * playing.  This is done by setting this as the SDL callback function with
 * Mix_ChannelFinished().
 *
 * When this function is called, it frees the sound as needed.
 */
void channel_done(int channel)
{
	if (channel_must_be_freed[channel])
		Mix_FreeChunk(wav_files_to_free[channel]);

	// Now we can safely mark the channel as unused again
	channel_must_be_freed[channel] = 0;
};

/**
 * This function will be invoked by the callback of the SDL mixer when
 * the fading out effect with the background music is completed.
 */
void OldMusicHasFinishedFadingOut(void)
{

	DebugPrintf(1, "\nOld music has now completely faded out and stopped... Can proceed to fade in new music... \n");
	LoadAndFadeInBackgroundMusic();

}

;				// void OldMusicHasFinishedFadingOut ( void )

/** ============================================ MUSIC ============================================ */
/**
 * This function is intended to provide a convenient way of switching
 * between different background sounds in freedroid.
 * If no background sound was yet running, the function should start playing
 * the given background music.
 * If some background sound was already playing, the function should shut down
 * the old background music and start playing the new one.
 *
 */
static void LoadAndFadeInBackgroundMusic(void)
{
#ifndef HAVE_LIBSDL_MIXER
	return;
#else

	static int MOD_Music_Channel = -1;
	char fpath[2048];
	char filename_raw[2048];

	if (!sound_on)
		return;

	if (filename_raw == SILENCE)	// SILENCE is defined as -1 I think
	{
		//printf("\nOld Background music channel has been halted.");
		// fflush(stdout);
		Mix_HaltMusic();	// this REALLY is a VOID-argument function!!
		MOD_Music_Channel = -1;
		return;
	}
	// Now we LOAD the music file from disk into memory!!
	// But before we free the old music.  This is not a danger, cause the music
	// is first initialized in InitAudio with some dummy mod files, so that there
	// is always something allocated, that we can free here.
	//
	// The loading of music and sound files is
	// something that was previously done only in the initialization function
	// of the audio thing.  But now we want to allow for dynamic specification of
	// music files via the mission files and that.  So we load the music now.
	//

	if (Loaded_MOD_Files[0] != NULL) {
		Mix_FreeMusic(Loaded_MOD_Files[0]);
		Loaded_MOD_Files[0] = NULL;
	}

	strcpy(filename_raw, "music/");
	strcat(filename_raw, NewMusicTargetFileName);
	find_file(filename_raw, SOUND_DIR, fpath, 0);
	Loaded_MOD_Files[0] = Mix_LoadMUS(fpath);
	if (Loaded_MOD_Files[0] == NULL) {
		DebugPrintf(0, "The music file %s could not be loaded!\n", NewMusicTargetFileName);
		return;
	} else
		DebugPrintf(1, "\nSuccessfully loaded file %s.", fpath);

	// MOD_Music_Channel = Mix_PlayMusic ( Loaded_MOD_Files[ Tune ] , -1 );
	MOD_Music_Channel = Mix_PlayMusic(Loaded_MOD_Files[0], -1);
	//    MOD_Music_Channel = Mix_FadeInMusic ( Loaded_MOD_Files[ 0 ] , -1 ,  );

	Mix_VolumeMusic((int)rintf(GameConfig.Current_BG_Music_Volume * MIX_MAX_VOLUME));

#endif				// HAVE_LIBSDL_MIXER
}

void SwitchBackgroundMusicTo(char *filename_raw_parameter)
{
	static char PreviousFileParameter[5000] = "NONE_AT_ALL";
	if (!sound_on)
		return;

	// Maybe the background music switch command given instructs us to initiate
	// the same background music that has been playing all the while anyway in
	// an endless loop.  So in this case, we need not touch anything at all and
	// just return.
	//
	if (!strcmp(PreviousFileParameter, filename_raw_parameter)) {
		DebugPrintf(0, "\nSwitch BG music instruction just repeats running music... nothing done.\n");
		return;
	} else {
		strcpy(PreviousFileParameter, filename_raw_parameter);
	}

	strcpy(NewMusicTargetFileName, filename_raw_parameter);

	// Now we can start to get some new music going, either directly or
	// by issuing a fade out instruction, that will then launch a callback
	// to get the new music going...
	//

	if (BackgroundMusicStateMachineState == NOTHING_PLAYING_AT_ALL) {
		LoadAndFadeInBackgroundMusic();
		BackgroundMusicStateMachineState = FADING_IN;
	} else {
		Mix_FadeOutMusic(200);
		BackgroundMusicStateMachineState = FADING_OUT;

		// We set up a function to be invoked by the SDL automatically as
		// soon as the fading out effect is completed (and the new music
		// can start to get going that is...)
		//
		Mix_HookMusicFinished(OldMusicHasFinishedFadingOut);

	}

}				// void SwitchBackgroundMusicTo ( char* filename_raw_parameter )

/** ============================================ SAMPLE FUNCTIONS ========================================= */

/**
 * This function plays a sound sample, that is NOT needed within the action part of the game but
 * only in menus or dialogs and can therefore be loaded and dumped on demand while the other sound
 * samples for the action parts of the game will be kept in memory all the time.
 **/
void play_sound_directly(const char *SoundSampleFileName)
{
	// This code searches for different kinds of
	int i;
	int pathlen;
	char *extension;
	char *extensions[] = { ".spx", ".ogg", ".wav", NULL };	// Extensions to try for audio

	// These variables will only be needed when compiling with sound!
	//

	int Newest_Sound_Channel = 0;
	Mix_Chunk *One_Shot_WAV_File;
	char Temp_Filename[5000];
	char fpath[2048] = "no_fpath_has_been_set";

	// Return immediately if sound is disabled
	if (!sound_on)
		return;

	// Now we set a callback function, that should be called by SDL
	// as soon as ANY other sound channel finishes playing...
	//
	Mix_ChannelFinished(channel_done);

	// Now we try to load the requested sound file into memory...
	//
	One_Shot_WAV_File = NULL;

	strcpy(Temp_Filename, SoundSampleFileName);

	// Only if the file name wasn't 'no_voice_sample', we really
	// try to load anything...
	//
	if (strcmp(SoundSampleFileName, "Sorry_No_Voice_Sample_Yet_0.wav")) {
		pathlen = strlen(Temp_Filename);

		if (strcmp(Temp_Filename + pathlen - 4, ".ogg") == 0) {
			extension = Temp_Filename + pathlen - 4;
		} else {
			extension = Temp_Filename + pathlen;
		}

		i = 0;
		while (extensions[i] != NULL) {
			strcpy(extension, extensions[i]);
			// find_file_silent may return a NULL pointer, in case the file name
			// composed hasn't been found.  We need to catch that case of course.
			//
			if (find_file(Temp_Filename, SOUND_DIR, fpath, 1) == 0) {
				One_Shot_WAV_File = Mix_LoadWAV(fpath);
				if (One_Shot_WAV_File != NULL) {
					break;
				} else {
					ErrorMessage(__FUNCTION__, "\
			    Corrupt sound file encountered!  The file is there, \n\
			    but the SDL MIXER was unable to LOAD it.", NO_NEED_TO_INFORM, IS_WARNING_ONLY);
				}
			}

			i++;
		}
	} else
		One_Shot_WAV_File = NULL;

	// Now some error checking against failed/missing sound samples...
	//
	if (One_Shot_WAV_File == NULL) {
		// A warning message about a missing speech file should only be issued,
		// if it wasn't the 'no_voice_sample' dummy entry anyway...
		//
		if (strcmp(SoundSampleFileName, "Sorry_No_Voice_Sample_Yet_0.wav")) {
			fprintf(stderr, "\n\nSoundSampleFileName: '%s'", SoundSampleFileName);
			ErrorMessage(__FUNCTION__, "\
		    There seems to be a sound file missing.", NO_NEED_TO_INFORM, FALSE);
		}
		// Now we must return, since we do not want to 'free' the sound sample, that
		// hasn't been loaded successfully and produce a segfault, do we?
		//
		return;
	}

	// Hoping, that this will not take up too much processor speed, we'll
	// now change the volume of the sound sample in question to what is normal
	// for sound effects right now...
	//
	// And of course we may only do this, if the sound file has been loaded
	// successfully!  Otherwise the SDL_mixer lib will produce a segfault.
	//
	if (One_Shot_WAV_File != NULL)
		Mix_VolumeChunk(One_Shot_WAV_File, (int)rintf(GameConfig.Current_Sound_FX_Volume * MIX_MAX_VOLUME));

	// Now we try to play the sound file that has just been successfully
	// loaded into memory...
	//
	// In case of an error, we will of course print an error message
	// and quit...
	//
	Newest_Sound_Channel = Mix_PlayChannel(-1, One_Shot_WAV_File, 0);
	if (Newest_Sound_Channel <= -1) {
		fprintf(stderr, "\n\nSoundSampleFileName: '%s' Mix_GetError(): %s \n", SoundSampleFileName, Mix_GetError());
		ErrorMessage(__FUNCTION__, "\
		The SDL MIXER WAS UNABLE TO PLAY A CERTAIN FILE LOADED INTO MEMORY FOR PLAYING ONCE.\n", PLEASE_INFORM, IS_WARNING_ONLY);

		// If we receive an error playing a sound file here, this is very inconvenient.
		// We must see to it that the callback code and allocation there and all that doesn't
		// get touched.  I hope that the following fix does already what we want here...
		// But it should :->
		//
		Mix_FreeChunk(One_Shot_WAV_File);
		return;
	}

	// We do nothing here, cause we can't halt the channel and
	// we also can't free the channel, that is still playing.
	channel_must_be_freed[Newest_Sound_Channel] = 1;
	wav_files_to_free[Newest_Sound_Channel] = One_Shot_WAV_File;
}

/**
 * Wrap play_sound_directly() such that we make sure that the same sound is not played with too
 * short of a delay.  Playing the sound several times in a row could lead to non-loadability errors
 * with the sound files.
 */
void play_sound(const char *filename)
{
	static char previous_filename[1000] = "";
	static Uint32 previous_ticks = 0;
	Uint32 ticks_now;

	// Return immediately if sound is disabled
	if (!sound_on)
		return;

	// Cancel unless we have had enough delay
	ticks_now = SDL_GetTicks();
	if (strcmp(previous_filename, filename) == 0
			&& ((ticks_now - previous_ticks) < 2.5 * 1000))
		return;

	// Save values for next run
	previous_ticks = ticks_now;
	strncpy(previous_filename, filename, 1000);

	// OK, play the sound.
	play_sound_directly(filename);
}

//aep: wrapper for the play_sound_cached_v
void play_sound_cached(const char *filename)
{
	play_sound_cached_v(filename, 1.0);
}

//----------------------------------------------------------------------
// This function should play a sound sample, that is NOT needed within
// the action part of the game but only in menus or dialogs and can
// therefore be loaded and dumped on demand while the other sound samples
// for the action parts of the game will be kept in memory all the time.
// ----------------------------------------------------------------------

//aep: implements volume now volume must be a double from 0 to 1. thats not my idea! go to JP, kill him!
void play_sound_cached_v(const char *SoundSampleFileName, double volume)
{
	int Newest_Sound_Channel = 0;
	char fpath[2048];
	int index_of_sample_to_be_played = 0;
	int sound_must_be_loaded = TRUE;
	int i;

	// In case sound has been disabled, we don't do anything here...
	//
	if (!sound_on)
		return;

	// First we go take a look if maybe the sound sample file name in question
	// has been given to this function (at least) once before.  Then we can
	// assume, that the corresponding sound sample is already loaded and still
	// in the cache and just need to play it and then we can safely return
	// immediately, without setting up any callbacks or the like, like the
	// play_sound() function has to.
	//
	for (i = 0; i < MAX_SOUNDS_IN_DYNAMIC_WAV_CACHE; i++) {
		if (i >= next_free_position_in_cache) {
			sound_must_be_loaded = TRUE;
			break;
		}

		if (!strcmp(sound_names_in_dynamic_wav_chache[i], SoundSampleFileName)) {
			sound_must_be_loaded = FALSE;
			index_of_sample_to_be_played = i;
			DebugPrintf(0, "\nFound the sound sample '%s' already in the cache!  Good.", SoundSampleFileName);
			break;
		}
	}

	// So if the sound sample isn't in the cache and therefore must still be loaded,
	// we do so here before any playing takes place...
	//
	if (sound_must_be_loaded) {
		// So now we know, that the sound sample in question has not yet ever been
		// used before.  We must load it, play it and keep it in cache memory.

		// Now we try to load the requested sound file into memory...
		//
		dynamic_WAV_cache[next_free_position_in_cache] = NULL;
		find_file(SoundSampleFileName, SOUND_DIR, fpath, 0);
		dynamic_WAV_cache[next_free_position_in_cache] = Mix_LoadWAV(fpath);
		if (dynamic_WAV_cache[next_free_position_in_cache] == NULL) {
			fprintf(stderr, "\n\nfpath: '%s'\n", fpath);
			ErrorMessage(__FUNCTION__, "Could not load sound file \"%s\": %s", NO_NEED_TO_INFORM, IS_WARNING_ONLY, fpath, Mix_GetError());
			// If the sample couldn't be loaded, we just quit, not marking anything
			// as loaded and inside the cache and also not trying to play anything...
			//
			return;
		}
		// Hoping, that this will not take up too much processor speed, we'll
		// now change the volume of the sound sample in question to what is normal
		// for sound effects right now...
		//
		Mix_VolumeChunk(dynamic_WAV_cache[next_free_position_in_cache], (int)rintf(MIX_MAX_VOLUME * volume * GameConfig.Current_Sound_FX_Volume));	//aep

		// We note the position of the sound file to be played
		//
		index_of_sample_to_be_played = next_free_position_in_cache;

		// Now we store the corresponding file name as well.
		//
		sound_names_in_dynamic_wav_chache[next_free_position_in_cache] = MyMalloc(strlen(SoundSampleFileName) + 1);
		strcpy(sound_names_in_dynamic_wav_chache[next_free_position_in_cache], SoundSampleFileName);
		DebugPrintf(1, "\nSuccessfully added sample '%s' to sound cache at new position %d.",
			    sound_names_in_dynamic_wav_chache[next_free_position_in_cache], next_free_position_in_cache);

		// Now we increase the 'next_sample' index and are done.
		//
		next_free_position_in_cache++;
		if (next_free_position_in_cache >= MAX_SOUNDS_IN_DYNAMIC_WAV_CACHE) {
			fprintf(stderr, "\n\nnext_free_position_in_cache: %d,\n", next_free_position_in_cache);
			ErrorMessage(__FUNCTION__, "\
			                           ALERT!  Ran out of space in the dynamic wav sample cache!  Cache size too small?", PLEASE_INFORM, IS_FATAL);
		}
	}
	// Now we try to play the sound file that has just been successfully
	// loaded into memory or has resided in memory already for some time...
	//
	// In case of an error, we will of course print an error message
	// and quit...
	//
	Newest_Sound_Channel = Mix_PlayChannel(-1, dynamic_WAV_cache[index_of_sample_to_be_played], 0);
	if (Newest_Sound_Channel <= -1) {
		fprintf(stderr, "\n\nSoundSampleFileName: '%s' Mix_GetError(): %s \n", SoundSampleFileName, Mix_GetError());
		ErrorMessage(__FUNCTION__, "\
		                           The SDL mixer was unable to play a certain sound sample file, that was supposed to be cached for later.\n", NO_NEED_TO_INFORM, IS_WARNING_ONLY);
	}
}

/**
 * When the sound sample volume is changed via the in-game controls, we 
 * need to re-sample everything, which is done only upon loading of the
 * sound samples.  Instead of RE-MIXING SOUND SAMPLES AGAIN AND AGAIN to
 * adapt the sound sample volume to the users needs, we will just clear
 * out the current wav file cache, so that when needed again, the sound
 * files will be loaded again and then automatically re-sampled to proper
 * volume from the play/load function itself.  Perfect solution.
 */
static void remove_all_samples_from_WAV_cache(void)
{
	int i;
	for (i = 0; i < next_free_position_in_cache; i++) {
		// We free the allocated memory for the file name.  This is overly 'clean'
		// code.  The amount of memory in question would be neglectable...
		//
		free(sound_names_in_dynamic_wav_chache[i]);

		// We free the allocated music chunk.  This is more important since the
		// music chunks in question can consume more memory...
		//
		Mix_FreeChunk(dynamic_WAV_cache[i]);
	}

	// Now that the cache has been emptied, it must be marked as such, or
	// the next play function will search it and produce a segfault.
	//
	next_free_position_in_cache = 0;
}

#endif				// HAVE_LIBSDL_MIXER

#undef _sound_c
