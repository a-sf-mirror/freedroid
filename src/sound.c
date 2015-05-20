/*
 *   Copyright (c) 2014 Samuel Degrande
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

// Number of slots in the SFX cache
#define MAX_SOUNDS_IN_SFX_CACHE 100
// Number of SDL channels allocated to play sounds
// TODO: Does it really make sense to mix so much samples ? This should be
// analyzed (with the RTProfiler) to tweak it.
#define ALLOCATED_AUDIO_CHANNELS 100
// FadeIn and FadeOut time in ms
#define MUSIC_FADE_INOUT 1000
// Max number of times a same sound is played
#define MAX_SOUND_OVERLAPPING 6

#ifndef WITH_SOUND

////////////////////////////////////////////////////////////////////
// These functions are defined if there is no sound. they do nothing
////////////////////////////////////////////////////////////////////

void init_audio(void) {}
void close_audio(void) {}
void set_music_volume(float volume) {}
void set_SFX_volume(float volume) {}
void switch_background_music(char *filename) {}
int play_sound(const char *filename) { return 0; }
void play_sound_v(const char *filename, float volume) {}
void play_sound_at_position(const char *filename, struct gps *listener, struct gps *emitter) {}

#else

static char *music_filename = NULL;      // The background music to play (once the current one is stopped)
static Mix_Music *loaded_music = NULL;   // Keep reference to previously loaded background music

////////////////////////////////////////////////////////////////////
// SFX cache
////////////////////////////////////////////////////////////////////

struct sound_cache {
	struct sound_cache_slot {
		Mix_Chunk *chunk;              // Pointer to the cached sound chunk
		int play_counter;              // Number of simultaneous play of the chunk
		uint32_t last_used;            // Last time the chunk was played
		char *sound_name;              // Filename of the sound chunk (allocated)
	} slots[MAX_SOUNDS_IN_SFX_CACHE];
	int next_free_slot;                // Index of the first free slot (when all slots are not filled)
};

static struct sound_cache SFX_cache;

/*
 * Initialize the SFX cache, all fields are set to 0.
 */
static void _SFX_cache_init(void)
{
	int i;
	for (i = 0; i < sizeof(SFX_cache.slots) / sizeof(SFX_cache.slots[0]); i++) {
		struct sound_cache_slot *slot = &SFX_cache.slots[i];
		slot->chunk = NULL;
		slot->last_used = 0;
		slot->play_counter = 0;
		slot->sound_name = NULL;
	}
	SFX_cache.next_free_slot = 0;
}

/*
 * Free and clear a given SFX cache slot.
 * Must not be called as long as the sound chunk is still playing.
 */
static void _SFX_cache_free_slot(int index)
{
	struct sound_cache_slot *slot = &SFX_cache.slots[index];

	slot->last_used = 0;
	slot->play_counter = 0;
	free(slot->sound_name);
	slot->sound_name = NULL;
	Mix_FreeChunk(slot->chunk);
	slot->chunk = NULL;
}

/*
 * Free and clear all slots of the SFX cache.
 * Call _SFX_cache_free_slot() to clear each slot.
 */
static void _SFX_cache_clear(void)
{
	int i;
	for (i = 0; i < SFX_cache.next_free_slot; i++) {
		_SFX_cache_free_slot(i);
	}
	SFX_cache.next_free_slot = 0;
}

/*
 * Find an unused SFX cache slot, and return its index.
 * When the cache is full, find the least recently used inactive slot and
 * free it ("inactive" means that the stored chunk is not currently played).
 * If no slot is found, warn and return -1.
 */
static int _SFX_cache_allocate_slot(void)
{
	if (SFX_cache.next_free_slot < MAX_SOUNDS_IN_SFX_CACHE) {
		return SFX_cache.next_free_slot++;
	}

	// When the wav file cache is full, free the least recently used inactive entry
	int i;
	uint32_t least_recently_used = UINT_MAX;
	int least_recently_used_index = -1;

	for (i = 0; i < SFX_cache.next_free_slot; i++) {
		if (SFX_cache.slots[i].play_counter == 0 && SFX_cache.slots[i].last_used < least_recently_used) {
			least_recently_used = SFX_cache.slots[i].last_used;
			least_recently_used_index = i;
		}
	}

	// No inactive slot found
	if (least_recently_used == -1) {
		error_once_message(ONCE_PER_GAME, __FUNCTION__,
			"Could not find an inactive slot to remove from SFX cache.\n",
			PLEASE_INFORM);
		return -1;
	}

	// Clean the found slot
	_SFX_cache_free_slot(least_recently_used_index);

	return least_recently_used_index;
}

/*
 * Fill a cache slot with the sound filename and sound chunk to cache.
 */
static void _SFX_cache_fill_slot(int index, const char *filename, Mix_Chunk *wav_chunk)
{
	struct sound_cache_slot *slot = &SFX_cache.slots[index];

	slot->last_used = 0;
	slot->play_counter = 0;
	slot->sound_name = my_strdup((char *)filename);
	slot->chunk = wav_chunk;
}

/*
 * Mark a cache slot as being currently played.
 * Current time is stored (used by the LRU algorithm in _SFX_cache_allocate_slot().
 * The play_counter is incremented, so we know that the slot is active.
 */
static void _SFX_cache_touch_slot(int index)
{
	struct sound_cache_slot *slot = &SFX_cache.slots[index];

	slot->last_used = SDL_GetTicks();
	slot->play_counter++;
}

/*
 * Mark a cache slot as being no more played.
 * The play_counter is decremented. Once it comes to zero, it means that
 * the sound chunk is no more played, and so the slot can be reused.
 */
static void _SFX_cache_release_slot(int index)
{
	struct sound_cache_slot *slot = &SFX_cache.slots[index];
	slot->play_counter--;
	// Postcondition. Avoid potential bug.
	if (slot->play_counter < 0)
		slot->play_counter = 0;
}

/*
 * Find a cache slot given a sound filename, and return its index.
 * Return -1 if the slot is not found.
 */
static int _SFX_cache_find_filename(const char *filename)
{
	int i;
	for (i = 0; i < SFX_cache.next_free_slot; i++) {
		if (!strcmp(SFX_cache.slots[i].sound_name, filename)) {
			return i;
		}
	}
	return -1;
}

/*
 * Find a cache slot given a pointer to a sound chunk, and return its index.
 * Return -1 if the slot is not found.
 */
static int _SFX_cache_find_chunk(Mix_Chunk *chunk)
{
	int i;
	for (i = 0; i < SFX_cache.next_free_slot; i++) {
		if (SFX_cache.slots[i].chunk == chunk) {
			return i;
		}
	}
	return -1;
}

/*
 * Return the chunk of a cached sound
 */
static Mix_Chunk *_SFX_cache_get_chunk(int index)
{
	return SFX_cache.slots[index].chunk;
}

/*
 * Return the play_counter of a cached sound
 */
static int _SFX_cache_get_counter(int index)
{
	return SFX_cache.slots[index].play_counter;
}

////////////////////////////////////////////////////////////////////
// SDL mixer callbacks
////////////////////////////////////////////////////////////////////

/*
 * Called by SDL when a channel finishes to play a sound.
 *
 * Releases the related SFX cache slot.
 */
static void _channel_done(int channel)
{
	Mix_Chunk *chunk = Mix_GetChunk(channel);
	if (chunk == NULL) {
		error_once_message(ONCE_PER_GAME, __FUNCTION__,
			"The associated sound chunk can not be found. This should never happen.\n"
			"Channel index: %d\n",
			PLEASE_INFORM, channel);
		return;
	}

	int cache_index = _SFX_cache_find_chunk(chunk);
	if (cache_index == -1) {
		error_once_message(ONCE_PER_GAME, __FUNCTION__,
			"The associated SFX cache slot can not be found. This should never happen.\n"
			"Channel index: %d\n",
			PLEASE_INFORM, channel);
		return;
	}

	_SFX_cache_release_slot(cache_index);
}

/*
 * Called by SDL when the fadeout of the current background music is finished.
 *
 * Free the memory used by the previous music, and loads the new one, those
 * filename was stored in new_music_filename by the call to switch_background_music().
 */

static void _load_background_music(void)
{
	char fpath[PATH_MAX];

	if (!sound_on)
		return;

	// Free the previous music's memory
	if (loaded_music) {
		// Note: Mix_FreeMusic() is called when the current music has already
		// been faded out, so it should not block waiting for the music to stop
		Mix_FreeMusic(loaded_music);
		loaded_music = NULL;
	}

	// Load a new background music, if one is set to be played
	if (music_filename) {
		if (find_file(music_filename, MUSIC_DIR, fpath, PLEASE_INFORM)) {
			loaded_music = Mix_LoadMUS(fpath);
			if (!loaded_music) {
				error_message(__FUNCTION__, "The music file %s could not be loaded: %s", PLEASE_INFORM, music_filename, Mix_GetError());
				return;
			}
			// No music is currently playing, so this call should not block
			Mix_FadeInMusic(loaded_music, -1, MUSIC_FADE_INOUT);
			Mix_VolumeMusic((int)rintf(GameConfig.Current_BG_Music_Volume * MIX_MAX_VOLUME));
		} else {
			// No music is currently played, remove the reference to its filename
			free(music_filename);
			music_filename = NULL;
		}
	}
}

////////////////////////////////////////////////////////////////////
// Public API
////////////////////////////////////////////////////////////////////

/**
 * \brief Initialize the SDL audio subsystem.
 *
 * \details This function initializes all the audio related stuff.
 * It tries to open the audio device using the configured number of channels,
 * falling back to opening it in stereo if a requested surround mode is not
 * available.
 * Some audio channels are allocated, and the SFX cache is initialized.
 */
void init_audio(void)
{
	const int audio_rate = 44100;
	const Uint16 audio_format = AUDIO_S16;
	const int audio_buffers = 4096;
	int audio_channels;

	if (!sound_on)
		return;
	
	switch (GameConfig.Current_Sound_Output_Fmt) {
	case SOUND_OUTPUT_FMT_SURROUND40:
		audio_channels = 4;
		break;
	case SOUND_OUTPUT_FMT_SURROUND51:
		audio_channels = 6;
		break;
	default:
		audio_channels = 2;
	}

	// Initialize the SDL audio subsystem
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
		error_message(__FUNCTION__,
				"The SDL AUDIO SUBSYSTEM COULD NOT BE INITIALIZED.\n"
				"Please check that your sound card is properly configured.\n"
				"If you for some reason cannot get your sound card ready, "
				"you can choose to play without sound: 'freedroid -q'.\n"
				"SDL error: %s\n",
				NO_REPORT, SDL_GetError());
		sound_on = FALSE;
		return;
	}

	// Open the audio device, using the configured number of audio channels (surround or stereo)
	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0) {
		error_message(__FUNCTION__,
				"A Problem occurred while trying to open the audio device with %d channels.\n"
				"Reverting to default settings (stereo).\n"
				"SDL mixer error: %s\n",
				NO_REPORT, audio_channels, Mix_GetError());

		// Revert to stereo if we fail to open the audio device in surround mode
		GameConfig.Current_Sound_Output_Fmt = SOUND_OUTPUT_FMT_STEREO;

		if (Mix_OpenAudio(audio_rate, audio_format, 2, audio_buffers) < 0) {
			error_message(__FUNCTION__,
					"We were not even able to open the audio device in stereo mode.\n"
					"Please check that your sound card is properly configured.\n"
					"SDL mixer error: %s\n",
					NO_REPORT, Mix_GetError());
			sound_on = FALSE;
			return;
		}
	}

	// Allocate a bunch of mixing channels.
	Mix_AllocateChannels(ALLOCATED_AUDIO_CHANNELS);

	// Initialize the SFX cache
	_SFX_cache_init();

	// Add callback functions, called when a sound or a music is finished
	Mix_ChannelFinished(_channel_done);
	Mix_HookMusicFinished(_load_background_music);
}

/**
 * \brief Close the audio subsystem.
 *
 * \details Free all audio stuff and close the audio device.
 */
void close_audio(void)
{
	if (!sound_on)
		return;

	Mix_ChannelFinished(NULL); // Avoid Mix_HaltChannel to call the ChannelFinished callback
	Mix_HaltChannel(-1); // Ensure that all channels are halted before to free the sound chunks
	_SFX_cache_clear();

	Mix_HookMusicFinished(NULL); // Prevent to call the MusicFinished callback
	if (loaded_music) {
		Mix_HaltMusic();
		Mix_FreeMusic(loaded_music);
		loaded_music = NULL;
	}

	if (music_filename) {
		free(music_filename);
		music_filename = NULL;
	}

	Mix_CloseAudio();
}

/**
 * \brief Set the volume of the currently played background music.
 *
 * \param volume The new volume (between 0.0 and 1.0)
 */
void set_music_volume(float volume)
{
	if (!sound_on)
		return;
	Mix_VolumeMusic((int)rintf(volume * MIX_MAX_VOLUME));
}

/**
 * \brief Set the volume of all SFX sounds.
 *
 * \details This changes the sound of all currently played sound.
 * Note that this will also change the sounds started with play_sound_v()...
 *
 * \param volume The new volume (between 0.0 and 1.0)
 */
void set_SFX_volume(float volume)
{
	if (!sound_on)
		return;
	Mix_Volume(-1, (int)rintf(volume * MIX_MAX_VOLUME));
}

/**
 * \brief Play a new background music in loop
 *
 * \details FadeOut the current background a music and prepare the loading of a new one by
 * setting 'new_music_filename'. The new background music will be started when the previous
 * one will been faded out (i.e. when SDL will call the MusicFinished callback).
 *
 * \param filename New background music to play (relative to MUSIC_DIR). If NULL, stops the current music.
 */
void switch_background_music(char *filename)
{
	if (!sound_on)
		return;

	// No music to play and no music currently playing: nothing to do
	if (!filename && !music_filename) {
		return;
	}

	// If the new background music is already the one being played: nothing to do
	if (filename && music_filename && !strcmp(music_filename, filename)) {
		return;
	}

	// Keep reference to the new music to play
	if (music_filename) {
		free(music_filename);
		music_filename = NULL;
	}
	music_filename = my_strdup(filename);

	// And play it
	if (!loaded_music) {
		// No music is currently played: immediately play the new music
		_load_background_music();
	} else {
		// First fade out the current music, and play the new one once done
		// (through a callback).
		// Note: if 'filename' is NULL, the callback will not start to play
		// any music.
		Mix_FadeOutMusic(2000);
		Mix_HookMusicFinished(_load_background_music);
	}
}

/**
 * \brief Play an SFX sound
 *
 * \details A sound cannot be played (and the funtion returns -1) when:
 * no unused slot is found in the SFX cache, or no audio channel is available,
 * or the sound is already played too many times.
 *
 * \param filename Filename of the SFX sound (relative to SOUND_DIR)
 *
 * \return Either the audio channel used to play the sound or -1 if error or the sound can not be played.
 */
int play_sound(const char *filename)
{
	// In case sound has been disabled, or not sound file name is given,
	// do nothing

	if (!sound_on || filename == NULL || filename[0] == '\0')
		return -1;

	// First we go take a look if maybe the sound sample is already in the
	// SFX cache. If not, the sample is loaded and put in cache.

	int cache_index = _SFX_cache_find_filename(filename);

	if (cache_index == -1) {
		char fpath[PATH_MAX];

		// Try to load the requested sound file into memory
		if (!find_file(filename, SOUND_DIR, fpath, PLEASE_INFORM)) {
			return -1;
		}
		Mix_Chunk *wav_chunk = Mix_LoadWAV(fpath);
		if (!wav_chunk) {
			error_message(__FUNCTION__, "Could not load sound file \"%s\": %s", PLEASE_INFORM, fpath, Mix_GetError());
			return -1;
		}

		// Allocate a cache entry for the loaded WAV sample
		cache_index = _SFX_cache_allocate_slot();
		if (cache_index == -1) {
			Mix_FreeChunk(wav_chunk);
			return -1;
		}
		_SFX_cache_fill_slot(cache_index, filename, wav_chunk);
	}

	// Mixing a same sound too many times can possibly lead to sound clipping.
	// Since the independent samples will not really be distinguishable, we
	// 'artificially' limit the overlap.
	// This mainly happens when several bots fire a burst with a delay between
	// 2 bullets that is shorter than the duration of the bullet sound.

	if (_SFX_cache_get_counter(cache_index) >= MAX_SOUND_OVERLAPPING)
		return -1;

	// Now we try to play the sound file

	int mix_channel = Mix_PlayChannel(-1, _SFX_cache_get_chunk(cache_index), 0);
	if (mix_channel <= -1) {
		error_once_message(ONCE_PER_GAME, __FUNCTION__,
				"The SDL mixer was unable to play a certain sound sample file,"
				"probably due to no audio channel being available.\n"
				"Mix_GetError(): %s",
				NO_REPORT, Mix_GetError());
		return -1;
	}

	Mix_Volume(mix_channel, GameConfig.Current_Sound_FX_Volume * MIX_MAX_VOLUME);
	_SFX_cache_touch_slot(cache_index);

	return mix_channel;
}

/**
 * \brief Play an SFX sound at a given ratio of the global sound volume
 *
 * \brief The function first calls play_sound() and then sets the volume.
 *
 * \param filename Filename of the SFX sound (relative to SOUND_DIR)
 * \param ratio    Multiplied by the global sound volume (game configuration) (between 0.0 and 1.0)
 */
void play_sound_v(const char *filename, float ratio)
{
	if (!sound_on)
		return;

	int sound_channel = play_sound(filename);
	if (sound_channel != -1) {
		Mix_Volume(sound_channel, (int)rintf(ratio * GameConfig.Current_Sound_FX_Volume * MIX_MAX_VOLUME));
	}
}

/**
 * \brief Play an SFX sound, with a spatialization effect.
 *
 * \details Simulate a 3D sound, based on the position of the sound source
 * and the position of the listener (Tux).
 * The function first calls play_sound() and then adds a positional effect.
 *
 * \param filename      Filename of the SFX sound (relative to SOUND_DIR)
 * \param listener_gps  GPS position of the listener
 * \param emitter_gps   GPS position of the sound source
 */
void play_sound_at_position(const char *filename, struct gps *listener_gps, struct gps *emitter_gps)
{
	struct gps emitter_virt_gps;
	struct point emitter;
	struct point listener;
	struct point delta;
	float squared_distance;
	float distance;
	float angle;
	Uint8 sound_distance;
	Sint16 sound_angle;
	int sound_channel;

	if (!sound_on)
		return;

	// Check some preconditions. If should not happen, so it's just a
	// fall-back to prevent any bug in the following code
	if (emitter_gps->z == -1 || listener_gps->z == -1) {
		// Un-positioned sound
		play_sound(filename);
		return;
	}

	// If listener and emitter are identical (such as for Tux attack sounds),
	// play un-positioned sound
	if (listener_gps == emitter_gps) {
		play_sound(filename);
		return;
	}

	// The positional sound parameters (distance and angle) are defined
	// relatively to screen coordinates.
	// So, we get the emitter's coordinates as if it were on the same level as
	// the listener, and we translate the emitter and listener coordinates to
	// screen coordinates.
	update_virtual_position(&emitter_virt_gps, emitter_gps, listener_gps->z);
	translate_map_point_to_screen_pixel(emitter_virt_gps.x, emitter_virt_gps.y, &emitter.x, &emitter.y);
	translate_map_point_to_screen_pixel(listener_gps->x, listener_gps->y, &listener.x, &listener.y);

	// d[x,y] = [x2,y2] - [x1,y1]
	delta.x = emitter.x - listener.x;
	delta.y = emitter.y - listener.y;
	squared_distance = (delta.x * delta.x) + (delta.y * delta.y);

	// If the sound source is very near the listener, play un-positioned
	// '10' (pixels) is just a 'small value'.
	if (squared_distance < 10*10) {
		play_sound(filename);
		return;
	}

	// Compute the 3D sound distance, which is to be between 0 (closest)
	// and 255 (farthest).
	// The computed value is capped:
	// - if the distance is less than 20% of the screen width, we cap to 0
	// - we also cap to 0.9 * 255, so that a sound is always audible
	// Note: if the emitter is on the left (or right) border of the screen,
	// this leads to a value of 3/4 * 255
	distance = sqrt(squared_distance) / ((float)GameConfig.screen_width / 1.5);
	if (distance < 0.2)
		sound_distance = 0;
	else if (distance > 0.9)
		sound_distance = (Uint8)(0.9 * 255.0);
	else
		sound_distance = (Uint8)(distance * 255.0);

	// Compute the 3D sound angle, which is clockwise, with 0 pointing north
	// (the 'angle' value is anti-clockwise, with 0 pointing east, hence the '90 - angle')
	angle = atan2(-delta.y, delta.x); // Y screen axis points down, so we revert the sign
	sound_angle = (Sint16)(90.0 - angle * (180.0/M_PI)) % 360;
	if (sound_angle < 0) sound_angle = 360 + sound_angle;

	// Try to play the sound
	sound_channel = play_sound(filename);

	// If sound is played, add positional effect.
	if (sound_channel != -1) {
		// TODO: MIX_SetPosition() does not preserve the audio power (referring
		// to linear pan rule or constant power pan rule).
		// We should compute ourself the volume to apply to each speaker (at least
		// in stereo) and use Mix_SetPanning().
		if (!Mix_SetPosition(sound_channel, sound_angle, sound_distance)){
			error_message(__FUNCTION__,
					"The SDL mixer was unable to register an effect on given channel.\n"
					"FileName: '%s' channel: '%d' Mix_GetError(): %s",
					NO_REPORT, filename, sound_channel, Mix_GetError());
		}
	}
}

#endif // HAVE_LIBSDL_MIXER

#undef _sound_c
