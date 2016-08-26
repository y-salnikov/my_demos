#include "types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

psg_context* psg_init(void(*write_ay_cb)(uint8_t reg, uint8_t val))
{
	psg_context *pc;
	pc=malloc(sizeof(psg_context));
	if(pc==NULL) return NULL;
	pc->state=0;
	pc->data=NULL;
	pc->pos=0;
	pc->size=0;
	pc->frame_counter=0;
	pc->write_reg_cb=write_ay_cb;
	return pc;
}

void psg_load(psg_context *pc, const char *filename)
{
	FILE *pfile;
	size_t readed;
	pfile=fopen(filename,"rb");
	if (pfile==NULL)
	{
		fprintf(stderr,"Can't open %s\n",filename);
		return;
	}
	fseek(pfile,0,SEEK_END);
	pc->size=ftell(pfile);
	fseek(pfile,0,SEEK_SET);
	pc->data=malloc(pc->size);
	if(pc->data==NULL)
	{
		fprintf(stderr,"Can't load %s, out of memory\n",filename);
		return;
	}
	readed=fread(pc->data,1,pc->size,pfile);
	if(readed!=pc->size)
	{
		fprintf(stderr,"Can't read %s\n",filename);
		return;
	}
	if(strncmp((char *)pc->data,"PSG\x1a",4))
	{
		fprintf(stderr,"Not a PSG file\n");
		return;
	}
	pc->pos=16;
}

void psg_next_frame(psg_context *pc)
{
	uint8_t b,v;
	
	if(pc->state==0) return;
	if(pc->frame_counter>0)
	{
		pc->frame_counter--;
		if(pc->frame_counter>0) return;
	}
	while(1)
	{
		if(pc->pos>=pc->size)
		{
			pc->state=0;
			return;
		}
		b=pc->data[pc->pos++];
		if(b==0xff) return;
		if(b==0xfe)
		{
			pc->frame_counter=pc->data[pc->pos++]*4;
			return;
		}
		if(pc->write_reg_cb!=NULL)
		{
			v=pc->data[pc->pos++];
			pc->write_reg_cb(b,v);
//			printf(" %02X:%02X  ",b,v);
		}
	}
	
}

void psg_play(psg_context *pc)
{
	if ( (pc->data!=NULL) && (pc->size>16))
	{
		pc->pos=16;
		pc->state=1;
	}
	else
	fprintf(stderr,"Data not loaded\n");
}

void psg_stop(psg_context *pc)
{
	pc->state=0;
}
void psg_done(psg_context *pc)
{
	pc->state=0;
	pc->write_reg_cb=NULL;
	free(pc->data);
	free(pc);
}
