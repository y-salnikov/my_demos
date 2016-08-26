#include <SDL.h>
#include "unistd.h"
#include "types.h"
#include "sound.h"
#include "ay.h"
#include "psg.h"

ay_context *ayc;
psg_context *pc;

void sound_callback(void *udata, Uint8 *stream, int len)
{
    ay_fill_samples(ayc,(void*)stream, len/4);
}

void write_ay_regs_callback(uint8_t reg,uint8_t val)
{
	ay_write(ayc,reg,val);
}

void frame_callback(void)
{
	psg_next_frame(pc);
}

int main(void)
{

//Initialize SDL 
    if( SDL_Init( SDL_INIT_AUDIO ) < 0 ) 
    { 
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() ); 
    }
	pc=psg_init(write_ay_regs_callback);
	ayc=ay_init(FREQ,frame_callback);
    sound_init(sound_callback);
	psg_load(pc,"demo.psg");
	psg_play(pc);
	while(pc->state)
	{
		sleep(1);
	}
    sound_done();
 //Quit SDL subsystems 
    SDL_Quit();
    return 0;
}
