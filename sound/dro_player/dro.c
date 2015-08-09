#include <stdio.h>
#include <unistd.h>
#include "opl.h"
//#include <string.h>
#include "dro.h"

void cb_dro_player(void);

static struct dro_file
{
	char cSignature[9];
	uint16_t *iVersionMajor, *iVersionMinor;
	uint8_t *song_data,*song_data_const;
	uint32_t *iLengthPairs,	*iLengthMS;
	uint8_t *iHardwareType,*iFormat,*iCompression,*iShortDelayCode,*iLongDelayCode,*iCodemapLength,*iCodemap;
	uint8_t status;
	float delay,overdelay;
	uint32_t ticks;
	int8_t loop;
}dro;

uint8_t dro_enabled=0;



void dro_get_status(dro_status *stat)
{
	stat->playing=dro_enabled;
	stat->loop=dro.loop;
	stat->position=(dro.song_data-dro.song_data_const)/2;
	stat->length=*dro.iLengthPairs;
}



uint8_t dro_load(void *data, uint32_t size)
{
	dro.status=0;
	dro.overdelay=0;
	dro.delay=0;
	dro_enabled=0;
//	memcpy(&dro.cSignature,data,8);
	data+=8; 
//	dro.cSignature[8]=0;
	dro.iVersionMajor=data; data+=2;
	dro.iVersionMinor=data; data+=2;

//	if(strcmp("DBRAWOPL",dro.cSignature))
//	{
//		printf("Bad data.\n");
//		return 1;
//	}
	if(*dro.iVersionMajor<2)
	{
		return 2;
	}
	dro.iLengthPairs=data; data+=4;
	dro.iLengthMS=data; data+=4;
	dro.iHardwareType = data++;
	dro.iFormat= data++;
	dro.iCompression= data++;
	dro.iShortDelayCode= data++;
	dro.iLongDelayCode= data++;
	dro.iCodemapLength= data++;
	dro.iCodemap= data;
	data+= *dro.iCodemapLength;
	dro.song_data=data;
	dro.song_data_const=data;

	
	
//printf("sign - '%s'\nmajor version %d\nminor version %d\nlength = %d pairs, %d mS\n",dro.cSignature,*dro.iVersionMajor,*dro.iVersionMinor,*dro.iLengthPairs, *dro.iLengthMS);
//printf("hardware type %d\nformat %d\ncompression %d\nshort delay code %X\nLong delay code %X\nCodemap length %d\n",*dro.iHardwareType,*dro.iFormat,*dro.iCompression,*dro.iShortDelayCode,*dro.iLongDelayCode,*dro.iCodemapLength);
if(*dro.iFormat==0 && *dro.iCompression==0){ dro.status=1; return 0;}
	return 3;
}

void dro_play(int8_t loop)
{
	int rc;
	if(dro.status)dro_enabled=1;
	dro.delay=0;
	opl_set_advance_callback(cb_dro_player);
	dro.loop=loop;
	
}

void dro_stop()
{
	dro_enabled=0;
}


static void dro_set_delay(uint32_t delay)
{
	dro.delay=delay/1000.0;
}

static void dro_process_pair()
{
	uint16_t reg,reg2;
	uint8_t second_bank,val;
	
	reg=*dro.song_data; dro.song_data++;
	val=*dro.song_data; dro.song_data++;
	second_bank=(reg & 0x80);

	if(reg==*dro.iShortDelayCode) { dro_set_delay(val+1); return; }
	if(reg==*dro.iLongDelayCode) { dro_set_delay((val+1)<<8); return; }

	reg=dro.iCodemap[(reg&0x7f)];
//	reg2=0x100;
	if(second_bank) reg2=0x100;
	adlib_write(reg2|reg,val);
//	printf("%03X %02X\n",reg,val)	;
}


void cb_dro_player(void)
{
	static double time;
	static int counter;
	if(!dro_enabled) return;
	
	dro.ticks++;
	time+=(1.0/44100);
	if (dro.delay>0)
	{
		dro.delay-=(1.0/44100);
		return;
	}
	dro_process_pair();
	if(++counter>=*dro.iLengthPairs)
		{
			if(--dro.loop)
			{
				dro.song_data=dro.song_data_const;
				counter=0;
				if(dro.loop<0) dro.loop=-1;
			}
			else
			{
			dro_enabled=0;
			counter=0;
			return;
			}
	
		}	
}
