#include "SDL.h"
#include "SDL_mixer.h"
#include "opl.h"


const int freq=44100;
const Uint16 format=AUDIO_S16SYS;
const int channels=2;
const int chunksize=4096;
const int n_channels=4;

Sint16 *buf;

void sound_init(void)
{
	int i,j;
	if(Mix_OpenAudio(freq,format,channels,chunksize)==-1)
	{
	printf("Mix_OpenAudio: %s\n", Mix_GetError());
    exit(2);
	}
	Mix_AllocateChannels(n_channels);
}

void stop_opl_emulation()
{
    Mix_HookMusic(NULL,NULL);
    
}

void sound_end(void)
{
//	Mix_Quit();
	stop_opl_emulation();
	free(buf);
}

void fm_init(void)
{
    int i;
    adlib_init(freq);
    for(i=0;i<0x1ff;i++)
    {
	adlib_write(i,0);
    }
    
}


void opl_callback(void *udata, Uint8 *stream, int len)
{
    int i;
    adlib_getsample((void*)stream, len/4);
    if((len/2)>=1024)
    {
	for(i=0;i<1024;i++)
	{
	    buf[i]=stream[i*2];
	}
    }
}


void start_opl_emulation()
{
    fm_init();
    buf=malloc(chunksize*channels*2);
    Mix_HookMusic(opl_callback,NULL);
}


int16_t* get_buf(void)
{
    return buf;
}
