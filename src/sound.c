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

static int allocated_sound_channels;

static void play_sound_cached_pos(const char *SoundSampleFileName, unsigned short angle, unsigned char distance);
static int allocate_sample_cache_entry(void);
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

static void play_sound_cached_pos(const char *SoundSampleFileName, unsigned short angle, unsigned char distance)
{
}

void play_sound_at_position(const char *filename, struct gps *listener, struct gps *emitter)
{
}

static void remove_all_samples_from_WAV_cache(void)
{
}

void play_sound(const char *filename)
{
}

#endif

#ifdef HAVE_LIBSDL_MIXER

/** ============================================ GLOBALS ============================================  */

static Mix_Chunk *dynamic_WAV_cache[MAX_SOUNDS_IN_DYNAMIC_WAV_CACHE];
static Uint32 dynamic_WAV_cache_last_use[MAX_SOUNDS_IN_DYNAMIC_WAV_CACHE];
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
// as soon as FreedroidRPG is started.  It does ONLY work with SDL and no
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
	
	if (GameConfig.Current_Sound_Output_Fmt == SOUND_OUTPUT_FMT_SURROUND40) {
		audio_channels = 4;
	} else if (GameConfig.Current_Sound_Output_Fmt == SOUND_OUTPUT_FMT_SURROUND51) {
		audio_channels = 6;
	} 

	// Now SDL_AUDIO is initialized here:

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
		fprintf(stderr, " \n\nSDL just reported a problem.\n"
			"The error string from SDL_GetError\nwas: %s \n"
			"The error string from the SDL mixer subsystem was: %s \n", SDL_GetError(), Mix_GetError());
		error_message(__FUNCTION__, "\n"
					"The SDL AUDIO SUBSYSTEM COULD NOT BE INITIALIZED.\n\n"
					"Please check that your sound card is properly configured,\n"
					"i.e. if other applications are able to play sounds.\n\n"
					"If you for some reason cannot get your sound card ready, \n"
					"you can choose to play without sound.\n\n"
					"If you want this, use the appropriate command line option and FreedroidRPG will\n"
					"not complain any more.", NO_REPORT);
		sound_on = FALSE;
		return;
	} else {
		DebugPrintf(1, "\nSDL Audio initialisation successful.\n");
	}

	// Now that we have initialized the audio SubSystem, we must open
	// an audio channel.  This will be done here (see code from Mixer-Tutorial):
	//
	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0) {
		fprintf(stderr, "\n\nSDL just reported a problem.\n"
			"The error string from SDL_GetError\nwas: %s \n"
			"The error string from the SDL mixer subsystem was: %s \n" 
			"using audio output format: %d \n", 
			SDL_GetError(), Mix_GetError(), GameConfig.Current_Sound_Output_Fmt);

		error_message(__FUNCTION__, "\n"
			"\nA Problem occured while trying to open the audio device.\n"
			"Reverting to default settings.", NO_REPORT);

		// revert to stereo if we fail to open the audio device
		GameConfig.Current_Sound_Output_Fmt = SOUND_OUTPUT_FMT_STEREO;

		if (Mix_OpenAudio(audio_rate, audio_format, 2, audio_buffers) < 0) {
			fprintf(stderr, "\n\nSDL just reported a problem.\n"
			"The error string from SDL_GetError\nwas: %s \n"
			"The error string from the SDL mixer subsystem was: %s \n", SDL_GetError(), Mix_GetError());
			error_message(__FUNCTION__, "\n"
						"The SDL AUDIO CHANNEL COULD NOT BE OPENED.\n\n"
						"Please check that your sound card is properly configured,\n"
						"i.e. if other applications are able to play sounds.\n\n"
						"If you for some reason cannot get your sound card ready,\n"
						"you can choose to play without sound.\n\n"
						"If you want this, use the appropriate command line option and FreedroidRPG will\n"
						"not complain any more.", NO_REPORT);
			sound_on = FALSE;
			return;
		}
	} else {
		DebugPrintf(1, "\nSuccessfully opened SDL audio channel.");
	}

	// Since we don't want some sounds to be omitted due to lack of mixing
	// channels, we select to have some at our disposal.  The SDL will do this
	// for a small increase in memory appetite as the price.  Whether this will
	// really resolve the problem however is unsure.
	//
	allocated_sound_channels = Mix_AllocateChannels(200);
}

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


/**
 * Finds a free channel to play a sample on
 *
 * Returns channel number if successful otherwise returns -1 if free channel isn't found
 **/
static int find_free_channel()
{
	int i = 0;

	// iterate through all of the channels
	for (i = 0; i < allocated_sound_channels; i++) {

		// if the channel is no longer playing, we can use it.
		if (!Mix_Playing(i))
			return i; 
	}

	// allocate more sound channels
	int requested_sound_channels = allocated_sound_channels + 100;
	if (requested_sound_channels <= MAX_SOUND_CHANNELS) {
		int old_allocated_sound_channels = allocated_sound_channels;
		allocated_sound_channels = Mix_AllocateChannels(allocated_sound_channels + 100);
		if (allocated_sound_channels > old_allocated_sound_channels)
			return old_allocated_sound_channels + 1;
	}

	// we couldn't find a free channel. 
	return -1;
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
}

/** ============================================ MUSIC ============================================ */
/**
 * This function is intended to provide a convenient way of switching
 * between different background sounds in FreedroidRPG.
 * If no background sound was yet running, the function should start playing
 * the given background music.
 * If some background sound was already playing, the function should shut down
 * the old background music and start playing the new one.
 *
 */
static void LoadAndFadeInBackgroundMusic(void)
{
	char fpath[PATH_MAX];
	char filename_raw[PATH_MAX];

	if (!sound_on)
		return;

	if (filename_raw == SILENCE)	// SILENCE is defined as -1 I think
	{
		//printf("\nOld Background music channel has been halted.");
		// fflush(stdout);
		Mix_HaltMusic();	// this REALLY is a VOID-argument function!!
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
	if (find_file(filename_raw, SOUND_DIR, fpath, PLEASE_INFORM)) {
		Loaded_MOD_Files[0] = Mix_LoadMUS(fpath);
		if (Loaded_MOD_Files[0] == NULL) {
			error_message(__FUNCTION__, "The music file %s could not be loaded!", PLEASE_INFORM, NewMusicTargetFileName);
			return;
		}
		Mix_PlayMusic(Loaded_MOD_Files[0], -1);
	}

	Mix_VolumeMusic((int)rintf(GameConfig.Current_BG_Music_Volume * MIX_MAX_VOLUME));
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
		Mix_FadeOutMusic(2000);
		BackgroundMusicStateMachineState = FADING_OUT;

		// We set up a function to be invoked by the SDL automatically as
		// soon as the fading out effect is completed (and the new music
		// can start to get going that is...)
		//
		Mix_HookMusicFinished(LoadAndFadeInBackgroundMusic);
	}
}

/** ============================================ SAMPLE FUNCTIONS ========================================= */

/**
 * This function plays a sound sample, that is NOT needed within the action part of the game but
 * only in menus or dialogs and can therefore be loaded and dumped on demand while the other sound
 * samples for the action parts of the game will be kept in memory all the time.
 **/
void play_sound(const char *filename)
{
	int Newest_Sound_Channel = 0;
	Mix_Chunk *One_Shot_WAV_File = NULL;
	char fpath[2048] = "no_fpath_has_been_set";

	// Return immediately if sound is disabled
	if (!sound_on)
		return;

	// Now we set a callback function, that should be called by SDL
	// as soon as ANY other sound channel finishes playing...
	Mix_ChannelFinished(channel_done);

	// Try to load the requested sound file into memory.
	if (!find_file(filename, SOUND_DIR, fpath, PLEASE_INFORM)) {
		return;
	}
	One_Shot_WAV_File = Mix_LoadWAV(fpath);
	if (One_Shot_WAV_File == NULL) {
		error_message(__FUNCTION__, "Corrupt sound file encountered: %s.",
					 PLEASE_INFORM, fpath);
		return;
	}

	// Hoping, that this will not take up too much processor speed, we'll
	// now change the volume of the sound sample in question to what is normal
	// for sound effects right now...
	Mix_VolumeChunk(One_Shot_WAV_File, (int)rintf(GameConfig.Current_Sound_FX_Volume * MIX_MAX_VOLUME));

	// Now we try to play the sound file that has just been successfully
	// loaded into memory...
	Newest_Sound_Channel = Mix_PlayChannel(-1, One_Shot_WAV_File, 0);
	if (Newest_Sound_Channel <= -1) {
		fprintf(stderr, "\n\nfilename: '%s' Mix_GetError(): %s \n", filename, Mix_GetError());
		error_message(__FUNCTION__, "\n"
					"The SDL MIXER WAS UNABLE TO PLAY A CERTAIN FILE LOADED INTO MEMORY FOR PLAYING ONCE.", PLEASE_INFORM);

		// If we receive an error playing a sound file here, we must see to it that the callback
		// code and allocation there and all that doesn't get touched.
		Mix_FreeChunk(One_Shot_WAV_File);
		return;
	}

	// We do nothing here, cause we can't halt the channel and
	// we also can't free the channel, that is still playing.
	channel_must_be_freed[Newest_Sound_Channel] = 1;
	wav_files_to_free[Newest_Sound_Channel] = One_Shot_WAV_File;
}

//aep: wrapper for the play_sound_cached_v
void play_sound_cached(const char *filename)
{
	play_sound_cached_v(filename, 1.0);
}

#define MAX_HEARING_DISTANCE 620
#define DEGREE_PER_RADIAN (180/M_PI)

/**
 * Plays a sound relative to the listener where listener is the position of the listener, and emitter is the position
 * of the emitter of the sound.
 **/
void play_sound_at_position(const char *filename, struct gps *listener, struct gps *emitter)
{
	float angle;
	float distance;
	int emitter_x;
	int emitter_y;
	int listener_x;
	int listener_y;
	int difference_x;
	int difference_y;
	gps emitter_virt_coords;
	unsigned int distance_value;
	unsigned short angle_value;

	// need to get the emitters coordinates as if it were on the same level as the listener.
	if (emitter->z == -1 || listener->z == -1) {
		play_sound(filename);
		return;
	} else {
		update_virtual_position(&emitter_virt_coords, emitter, listener->z);
	}

	// translate the emitter and listener coordinates to screen coordinates.
	translate_map_point_to_screen_pixel(emitter_virt_coords.x, emitter_virt_coords.y, &emitter_x, &emitter_y);
	translate_map_point_to_screen_pixel(listener->x, listener->y, &listener_x, &listener_y);

	// d[x,y] = [x1,y1] - [x2,y2]
	difference_x = listener_x - emitter_x;
	difference_y = listener_y - emitter_y;

	// calculate the distance
	distance = sqrt((difference_x * difference_x) + (difference_y * difference_y));

	// and angle [-pi, pi] radians
	angle = atan2(difference_y, difference_x);

	// prevent overflow
	distance_value = (unsigned int)((distance / MAX_HEARING_DISTANCE) * 255);

	// we don't want distance_value to wrap around.
	distance_value = (distance_value >= 255) ? 255 : distance_value;

	// adjust the angle so that it is aligned with the game / screen coordinates properly.
	angle_value = (unsigned short)((angle * DEGREE_PER_RADIAN - 90));

	// just to prevent the angle from wrapping.
	angle_value = angle_value % 360;

	// if the emitter->listener vector is really small, use non-positional play.
	if (distance < 1) {
		play_sound_cached(filename);
	} else { 
		play_sound_cached_pos(filename, angle_value, (unsigned char)distance_value);
	}
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
	// use volume to scale the distance for the effect
	// Range for volume - distance translation: 0 ( loud ) to 255 ( soft )
	play_sound_cached_pos(SoundSampleFileName, 0, (unsigned char)(255 - (volume * 255)));
}

// Plays a sound at relative distance and angle
static void play_sound_cached_pos(const char *SoundSampleFileName, unsigned short angle, unsigned char distance)
{
	int channel_to_play_sample_on = 0;
	char fpath[PATH_MAX];
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
		if (!find_file(SoundSampleFileName, SOUND_DIR, fpath, PLEASE_INFORM)) {
			return;
		}
		Mix_Chunk *loaded_wav_chunk = Mix_LoadWAV(fpath);
		if (!loaded_wav_chunk) {
			error_message(__FUNCTION__, "Could not load sound file \"%s\": %s", PLEASE_INFORM, fpath, Mix_GetError());
			// If the sample couldn't be loaded, we just quit, not marking anything
			// as loaded and inside the cache and also not trying to play anything...
			//
			return;
		}

		// Allocate a cache entry for the loaded WAV sample
		int sample_cache_index = allocate_sample_cache_entry();
		dynamic_WAV_cache[sample_cache_index] = loaded_wav_chunk;
		sound_names_in_dynamic_wav_chache[sample_cache_index] = MyMalloc(strlen(SoundSampleFileName) + 1);
		strcpy(sound_names_in_dynamic_wav_chache[sample_cache_index], SoundSampleFileName);

		// Hoping, that this will not take up too much processor speed, we'll
		// now change the volume of the sound sample in question to what is normal
		// for sound effects right now...
		//
		Mix_VolumeChunk(dynamic_WAV_cache[sample_cache_index], (int)rintf(MIX_MAX_VOLUME * GameConfig.Current_Sound_FX_Volume));

		// We note the position of the sound file to be played
		//
		index_of_sample_to_be_played = sample_cache_index;
	}
	// Now we try to play the sound file that has just been successfully
	// loaded into memory or has resided in memory already for some time...
	//
	// In case of an error, we will of course print an error message
	// and quit...
	//
	
	// First let's find a free channel
	channel_to_play_sample_on = find_free_channel();

	// Now we try to play the sound file that has just been successfully
	// loaded into memory or has resided in memory already for some time...
	dynamic_WAV_cache_last_use[index_of_sample_to_be_played] = SDL_GetTicks();
	if (Mix_PlayChannel(channel_to_play_sample_on, dynamic_WAV_cache[index_of_sample_to_be_played], 0) <= -1) {
		error_message(__FUNCTION__,
				"The SDL mixer was unable to play a certain sound sample file, that was supposed to be cached for later.\n"
				"SoundSampleFileName: '%s' Mix_GetError(): %s",
				NO_REPORT, SoundSampleFileName, Mix_GetError());
	}

	// if we found a free channel, let's apply an effect to it.
	if (channel_to_play_sample_on != -1) {
		if (!Mix_SetPosition(channel_to_play_sample_on, angle, distance)){
			error_message(__FUNCTION__,
					"The SDL mixer was unable to register an effect on given channel.\n"
					"SoundSampleFileName: '%s' channel: '%d' Mix_GetError(): %s",
					NO_REPORT, SoundSampleFileName, channel_to_play_sample_on, Mix_GetError());
		}
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

static int allocate_sample_cache_entry(void)
{
	// When the wav file cache is full, free the least recently used entry.
	if (next_free_position_in_cache >= MAX_SOUNDS_IN_DYNAMIC_WAV_CACHE) {
		int i;
		Uint32 least_recently_used = dynamic_WAV_cache_last_use[0];
		int least_recently_used_index = 0;

		for (i = 1; i < MAX_SOUNDS_IN_DYNAMIC_WAV_CACHE; i++) {
			if (dynamic_WAV_cache_last_use[i] < least_recently_used) {
				least_recently_used = dynamic_WAV_cache_last_use[i];
				least_recently_used_index = i;
			}
		}

		free(sound_names_in_dynamic_wav_chache[least_recently_used_index]);
		Mix_FreeChunk(dynamic_WAV_cache[least_recently_used_index]);
		return least_recently_used_index;
	}

	return next_free_position_in_cache++;
}

#endif				// HAVE_LIBSDL_MIXER

#undef _sound_c
