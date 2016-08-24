#include <stdlib.h>
#include <stdio.h>
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
	ayc->envelope_counter=0;
	ayc->envelope=0;
	ayc->envelope_state=0;
	ayc->tick_fraction=0.0;
	ayc->tps=(((float)CLK)/samplerate)/16.0;
	return ayc;
}

void ay_get_sample(ay_context *ayc)
{
	uint8_t ticks=(uint8_t)ayc->tps;
	uint16_t tc[3],i;
	ayc->tick_fraction+=ayc->tps-ticks;
	if (ayc->tick_fraction>1.0)
	{
		ayc->tick_fraction-=1.0;
		ticks+=1;
	}

	tc[0]=ayc->regs[0]|((ayc->regs[1]<<8) & 0x0F);
	tc[1]=ayc->regs[2]|((ayc->regs[3]<<8) & 0x0F);
	tc[2]=ayc->regs[4]|((ayc->regs[5]<<8) & 0x0F);

	for(i=0;i<3;i++)
	{
		if((ticks>tc[i]) && (tc[i]>0))
		{
			fprintf(stderr,"Tone #%d pitch is too high\n",i);
			ayc->tone_counters[i]=0;
			ayc->tone_wave[i]=1-ayc->tone_wave[i];
		}
		else
		{
			ayc->tone_counters[i]+=ticks;
			if(ayc->tone_counters[i]>=tc[i])
			{
				ayc->tone_counters[i]-=tc[i];
				ayc->tone_wave[i]=1-ayc->tone_wave[i];
			}
		}
	}
}

void ay_done(ay_context *ayc)
{
	free(ayc);
}
