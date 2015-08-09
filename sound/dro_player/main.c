#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "SDL.h"
#include "sound.h"
#include "dro.h"
#include "khaos2_dro.h"


int main(int argc, char *argv[])
{
    const char bar_length=50;
    dro_status drost;
    const char spin[]="-\\|/";
    int spin_index=0;
    int i;
    
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER);
    sound_init();
    start_opl_emulation();

    dro_load(data,data_length);
    dro_play(2);
	do
	{
	    dro_get_status(&drost);
	    printf("%c[",spin[spin_index]);
	    for(i=0;i<bar_length-3;i++)
	    {
		if (i<((float)drost.position/drost.length)*(bar_length-3)) printf("=");
		else printf(".");
	    }
	    printf("]\r");
	    fflush(stdout);
	    usleep(100000);
	    spin_index=(spin_index+1) % 4;
	} while(drost.playing);
	usleep(1000000);
	
	dro_stop();	
	sound_end();
    return 0;
  }

