#include "audio.h"
#include "utils.h"

audio_t load_audio(const char* file)
{
    audio_t audio = Mix_LoadWAV(file);

    // Making sure that the audio file was successfully loaded
    assert_panic(!audio, "Something went wrong, couldn't load audio!");

    return audio;
}

void play_audio(audio_t audio)
{
    Mix_PlayChannel(-1, audio, 0);
}


