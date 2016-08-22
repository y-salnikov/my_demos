#include <stdlib.h>
#include "types.h"

#define CLK 1750000 // Hz


ay_context* ay_init(int samplerate)
{
	uint8_t i;
	ay_context *ayc;
	ayc=malloc(sizeof(ay_context));
	if(ayc==NULL) return NULL;
	for(i=0;i<16;i++) ayc->regs[i]=0;
	ayc->tone_wave[0]=ayc->tone_wave[1]=ayc->tone_wave[2]=0;
	ayc->tone_counters[0]=ayc->tone_counters[1]=ayc->tone_counters[2]=0;
	auc->envelope_counter=0;
	ayc->envelope=0;
	ayc->tick_fraction=0.0;
	ayc->tps=(((float)CLK)/samplerate)/16.0;
	return ayc;
}

void ay_get_sample(ay_context *ayc)
{
	
}

void ay_done(ay_context *ayc)
{
	free(ayc);
}
