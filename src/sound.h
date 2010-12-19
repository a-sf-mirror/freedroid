#ifndef _sound_h
#define _sound_h

#include "system.h"
#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#define MAX_SOUNDS_IN_DYNAMIC_WAV_CACHE 100
#define ALL_MOD_MUSICS 1
#define MAX_SOUND_CHANNELS 5000

void play_sound_cached(const char *SoundSampleFileName);
void remove_all_samples_from_WAV_cache(void);
void LoadAndFadeInBackgroundMusic(void);

#endif
