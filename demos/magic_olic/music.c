#include "olic.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include <math.h>

const int freq=44100;
const Uint16 format=AUDIO_S16SYS;
const int channels=2;
const int chunksize=1024;
const int n_channels=128;
SDL_RWops *rw;
Mix_Chunk *music;

void sound_init(void)         
{
	if(Mix_OpenAudio(freq,format,channels,chunksize)==-1)
	{
	printf("Mix_OpenAudio: %s\n", Mix_GetError());
    exit(2);
	}
	Mix_AllocateChannels(n_channels);
	rw = SDL_RWFromMem(data,data_length);
	music=Mix_LoadWAV_RW(rw,1);
}

void play_music(void)
{
	Mix_PlayChannel(-1, music, -1);
}
