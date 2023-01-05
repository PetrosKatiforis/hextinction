#ifndef _AUDIO_H
#define _AUDIO_H

#include <SDL_mixer.h>

// This is just for consistency, although really pointless and confusing
typedef Mix_Chunk* audio_t;

audio_t load_audio(const char* file);

void play_audio(audio_t audio);

#endif
