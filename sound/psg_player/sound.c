#include <SDL_mixer.h>
#include "types.h"
#include "sound.h"
#include "ay.h"

void sound_init(void(*sound_cb)(void *udata, Uint8 *stream, int len))
{
    if(Mix_OpenAudio(FREQ,AUDIO_S16SYS,CHANNELS,CHUNKSIZE)==-1)
    {
	printf("Mix_OpenAudio: %s\n", Mix_GetError());
	exit(2);
    }
    Mix_AllocateChannels(N_CHANNELS);
    Mix_HookMusic(sound_cb,NULL);
}

void sound_done(void)
{
	Mix_HookMusic(NULL,NULL);
}

