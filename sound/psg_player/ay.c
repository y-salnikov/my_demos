#include <stdlib.h>
#include <stdio.h>
#include "types.h"

#define CLK 1750000 // 1750000 Hz

//            mixer matrix      A,     B,     C
static const float LEFT[3]=  { 0.45, 0.50,   0.05 };
static const float RIGHT[3]= { 0.05, 0.50,   0.45 };

/* AY output doesn't match the claimed levels; these levels are based
   * on the measurements posted to comp.sys.sinclair in Dec 2001 by
   * Matthew Westcott, adjusted as Philip Kendall described in a followup to his post,
   * then scaled to 0..0xffff.
   */
  static const int levels[16] = {
    0x0000, 0x0385, 0x053D, 0x0770,
    0x0AD7, 0x0FD5, 0x15B0, 0x230C,
    0x2B4C, 0x43C1, 0x5A4B, 0x732F,
    0x9204, 0xAFF1, 0xD921, 0xFFFE
  };

ay_context* ay_init(int samplerate, void(*frame_callback)(void))
{
	uint8_t i;
	ay_context *ayc;
	ayc=malloc(sizeof(ay_context));
	if(ayc==NULL) return NULL;
	for(i=0;i<16;i++) ayc->regs[i]=0;
	ayc->tone_wave[0]=ayc->tone_wave[1]=ayc->tone_wave[2]=0;
	ayc->tone_counters[0]=ayc->tone_counters[1]=ayc->tone_counters[2]=0;
	ayc->envelope_counter=0;
	ayc->envelope_state=0;
	ayc->envelope_phase=0;
	ayc->tick_fraction=0.0;
	ayc->tps=(((float)CLK)/samplerate)/16.0;
	ayc->fcb=frame_callback;
	ayc->frame_counter=0;
	ayc->tpf=samplerate*(1.0/50.0); 		// 50Hz / frame
	ayc->a=ayc->b=ayc->c=0;
	return ayc;
}

void ay_advance(ay_context *ayc)
{
	uint8_t ticks=(uint8_t)ayc->tps;
	uint16_t tc[3],i;
	uint8_t nc,noise,envelope;
	uint32_t ec;
	ayc->tick_fraction+=ayc->tps-ticks;
	if (ayc->tick_fraction>1.0)
	{
		ayc->tick_fraction-=1.0;
		ticks+=1;
	}
	
	tc[0]=ayc->regs[00]|((ayc->regs[01] & 0x0f) <<8);
	tc[1]=ayc->regs[02]|((ayc->regs[03] & 0x0f) <<8);
	tc[2]=ayc->regs[04]|((ayc->regs[05] & 0x0f) <<8);
	nc=ayc->regs[06];// & 0x1F;  // wtf?
	ec=(ayc->regs[013]|(ayc->regs[014]<<8))*16;			//16?
	
//		tone generators	
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
//		noise generator
	if((ticks>nc) && (nc>0))
	{
		fprintf(stderr,"Noise tone pitch is too high\n");
		ayc->noise_mod=1-ayc->noise_mod;
		ayc->noise_counter=0;
	}
	else
	{
		ayc->noise_counter+=ticks;
		if(ayc->noise_counter>=nc)
		{
			ayc->noise_counter-=nc;
			ayc->noise_mod=1-ayc->noise_mod;
		}
	}
	noise=ayc->noise_mod*(rand() & 0x01);

// 		envelope generator
	ayc->envelope_counter+=ticks;
	if(ayc->envelope_counter>ec)
	{
		ayc->envelope_counter-=ec;
		ayc->envelope_phase+=1;
		if(ayc->envelope_phase>15)
		{
			ayc->envelope_phase=0;
			switch(ayc->envelope_state)
			{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 9:
				case 15: ayc->envelope_state=16; break;
				case 8:
				case 12: break;
				case 11:
				case 13: ayc->envelope_state=17; break;
				case 10: ayc->envelope_state=14; break;
				case 14: ayc->envelope_state=10; break;
				case 16: break;
				case 17: break;
			}
		}
	}
	envelope=0;
	switch(ayc->envelope_state)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 8:
		case 9:
		case 10:
		case 11: envelope=15-ayc->envelope_phase; break;
		case 4:
		case 5:
		case 6:
		case 7:
		case 12:
		case 13:
		case 14:
		case 15: envelope=ayc->envelope_phase; break;
		case 16: envelope=0; break;
		case 17: envelope=15; break;
	}
// 		mixer
	ayc->a = ((ayc->regs[07] & 0x01) ? 1 : ayc->tone_wave[0] ) & ((ayc->regs[07] & 0x08) ? 1 : noise );
	ayc->b = ((ayc->regs[07] & 0x02) ? 1 : ayc->tone_wave[1] ) & ((ayc->regs[07] & 0x10) ? 1 : noise );
	ayc->c = ((ayc->regs[07] & 0x04) ? 1 : ayc->tone_wave[2] ) & ((ayc->regs[07] & 0x20) ? 1 : noise );

	if (((ayc->regs[07] & 0x01) & (ayc->regs[07] & 0x08))) ayc->a=0;
	if (((ayc->regs[07] & 0x02) & (ayc->regs[07] & 0x10))) ayc->b=0;
	if (((ayc->regs[07] & 0x04) & (ayc->regs[07] & 0x20))) ayc->c=0;

	ayc->a*= (ayc->regs[010] & 0x10) ? envelope : (ayc->regs[010] & 0x0f);
	ayc->b*= (ayc->regs[011] & 0x10) ? envelope : (ayc->regs[011] & 0x0f);
	ayc->c*= (ayc->regs[012] & 0x10) ? envelope : (ayc->regs[012] & 0x0f);
	
	ayc->frame_counter+=1;
	if(ayc->frame_counter>=ayc->tpf)
	{
		ayc->frame_counter-=ayc->tpf;
		ayc->fcb();
	}
}

void ay_fill_samples(ay_context *ayc, int16_t *sound, uint32_t length) // length in samples = bytes/4
{
	uint32_t i;
	for(i=0;i<length;i++)
	{
		ay_advance(ayc);
		sound[i*2]  =    32767-((LEFT[0]*levels[ayc->a])+(LEFT[1]*levels[ayc->b])+(LEFT[2]*levels[ayc->c]));
		sound[1+(i*2)] = 32767-((RIGHT[0]*levels[ayc->a])+(RIGHT[1]*levels[ayc->b])+(RIGHT[2]*levels[ayc->c]));
	}
	
}

void ay_done(ay_context *ayc)
{
	free(ayc);
}

void ay_write(ay_context *ayc, uint8_t reg, uint8_t val)
{
	ayc->regs[reg]=val;
	switch(reg)
	{
		case 00:
		case 01: ayc->tone_counters[0]=0; break;
		case 02:
		case 03: ayc->tone_counters[1]=0; break;
		case 04:
		case 05: ayc->tone_counters[2]=0; break;
		case 06: ayc->noise_counter=0;	 break;
		case 015: ayc->envelope_state=val; break;
	}
}
