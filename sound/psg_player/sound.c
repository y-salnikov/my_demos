#include <SDL_mixer.h>
#include "sound.h"

void sound_init(void)
{
    if(Mix_OpenAudio(FREQ,AUDIO_S16SYS,CHANNELS,CHUNKSIZE)==-1)
    {
	printf("Mix_OpenAudio: %s\n", Mix_GetError());
	exit(2);
    }
    Mix_AllocateChannels(N_CHANNELS);
}

void sound_done(void)
{
}