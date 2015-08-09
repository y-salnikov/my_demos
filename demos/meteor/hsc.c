/* This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Based on hsc.cpp - HSC Player by Simon Peter <dn.tlp@gmx.net> from adplug
 * by y.salnikov y.salnikov65535@gmail.com
 */





#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include "opl.h"



extern const int freq;
uint8_t hsc_enabled=0;
typedef struct hscnote
  {
    unsigned char note, effect;
  } hscnote;	// note type in HSC pattern

typedef  struct hscchan
  {
    unsigned char inst;			// current instrument
    signed char slide;			// used for manual slide-effects
    unsigned short freq;		// actual replaying frequency
  }hscchan;	// HSC channel data
uint8_t hsc_loop;
hscchan channel[9];			// player channel-info
unsigned char instr[128][12];		// instrument data
unsigned char song[0x80];		// song-arrangement (MPU-401 Trakker enhanced)
hscnote patterns[50][64*9];		// pattern data
unsigned char pattpos,songpos,	// various bytes & flags
		    pattbreak,songend,mode6,bd,fadein;
unsigned int speed,del;
unsigned char adl_freq[9];		// adlib frequency registers
int mtkmode;				// flag: MPU-401 Trakker mode on/off
const unsigned short note_table[12] =
  {363, 385, 408, 432, 458, 485, 514, 544, 577, 611, 647, 686};

const unsigned char op_table[9] =
  {0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12};

void hsc_timer(void);

uint8_t* hsc_load(char *filename)
{
	FILE *f;
	uint64_t length,rd;
	uint8_t *ptr;
	
	f=fopen(filename,"rb");
	if (f==NULL) return NULL;
	fseek(f, 0, SEEK_END);
	length=ftell(f);
	fseek(f, 0, SEEK_SET);
	if (length==0) goto err;
	ptr=malloc(length);
	if (ptr==NULL) goto err;
	rd=fread(ptr,1,length,f);
	if(rd!=length) goto err;
	fclose(f);
	return ptr;


err:	fclose(f);
		return NULL;
}

void hsc_parse(uint8_t *hsc)
{
	uint32_t i;
	
  for(i=0;i<128*12;i++)		// load instruments
    	*((unsigned char *)instr + i) = *(hsc++);
  for (i=0;i<128;i++)
  {			// correct instruments
    instr[i][2] ^= (instr[i][2] & 0x40) << 1;
    instr[i][3] ^= (instr[i][3] & 0x40) << 1;
    instr[i][11] >>= 4;			// slide
  }
  for(i=0;i<51;i++) song[i] = *(hsc++);	// load tracklist
  for(i=0;i<50*64*9;i++)			// load patterns
    *((char *)patterns + i) = *(hsc++);
}

void setvolume(unsigned char chan, int volc, int volm)
{
  unsigned char	*ins = instr[channel[chan].inst];
  char		op = op_table[chan];

  adlib_write(0x43 + op,volc | (ins[2] & ~63));
  if (ins[8] & 1)							// carrier
    adlib_write(0x40 + op,volm | (ins[3] & ~63));
  else
    adlib_write(0x40 + op, ins[3]);		// modulator
}


void setfreq(unsigned char chan, unsigned short freq)
{
  adl_freq[chan] = (adl_freq[chan] & ~3) | (freq >> 8);

  adlib_write(0xa0 + chan, freq & 0xff);
  adlib_write(0xb0 + chan, adl_freq[chan]);
}



void setinstr(unsigned char chan, unsigned char insnr)
{
  unsigned char	*ins = instr[insnr];
  char		op = op_table[chan];

  channel[chan].inst = insnr;		// set internal instrument
  adlib_write(0xb0 + chan,0);			// stop old note

  // set instrument
  adlib_write(0xc0 + chan, ins[8]);
  adlib_write(0x23 + op, ins[0]);        // carrier
  adlib_write(0x20 + op, ins[1]);        // modulator
  adlib_write(0x63 + op, ins[4]);        // bits 0..3 = decay; 4..7 = attack
  adlib_write(0x60 + op, ins[5]);
  adlib_write(0x83 + op, ins[6]);        // 0..3 = release; 4..7 = sustain
  adlib_write(0x80 + op, ins[7]);
  adlib_write(0xe3 + op, ins[9]);        // bits 0..1 = Wellenform
  adlib_write(0xe0 + op, ins[10]);
  setvolume(chan, ins[2] & 63, ins[3] & 63);
}



bool hsc_update(void)
{
  // general vars
  unsigned char		chan,pattnr,note,effect,eff_op,inst,vol,Okt,db;
  unsigned short	Fnr;
  unsigned long		pattoff;

  del--;                      // player speed handling
  if(del)
    return !songend;		// nothing done

  if(fadein)					// fade-in handling
    fadein--;

  pattnr = song[songpos];
  if(pattnr == 0xff) {			// arrangement handling
    songend = 1;				// set end-flag
    songpos = 0;
    pattnr = song[songpos];
  } else
    if ((pattnr & 128) && (pattnr <= 0xb1)) { // goto pattern "nr"
      songpos = song[songpos] & 127;
      pattpos = 0;
      pattnr = song[songpos];
      songend = 1;
    }

  pattoff = pattpos*9;
  for (chan=0;chan<9;chan++) {			// handle all channels
    note = patterns[pattnr][pattoff].note;
    effect = patterns[pattnr][pattoff].effect;
    pattoff++;

    if(note & 128) {                    // set instrument
      setinstr(chan,effect);
      continue;
    }
    eff_op = effect & 0x0f;
    inst = channel[chan].inst;
    if(note)
      channel[chan].slide = 0;

    switch (effect & 0xf0) {			// effect handling
    case 0:								// global effect
      /* The following fx are unimplemented on purpose:
       * 02 - Slide Mainvolume up
       * 03 - Slide Mainvolume down (here: fade in)
       * 04 - Set Mainvolume to 0
       *
       * This is because i've never seen any HSC modules using the fx this way.
       * All modules use the fx the way, i've implemented it.
       */
      switch(eff_op) {
      case 1: pattbreak++; break;	// jump to next pattern
      case 3: fadein = 31; break;	// fade in (divided by 2)
      case 5: mode6 = 1; break;	// 6 voice mode on
      case 6: mode6 = 0; break;	// 6 voice mode off
      }
      break;
    case 0x20:
    case 0x10:		                    // manual slides
      if (effect & 0x10) {
	channel[chan].freq += eff_op;
	channel[chan].slide += eff_op;
      } else {
	channel[chan].freq -= eff_op;
	channel[chan].slide -= eff_op;
      }
      if(!note)
	setfreq(chan,channel[chan].freq);
      break;
    case 0x50:							// set percussion instrument (unimplemented)
      break;
    case 0x60:							// set feedback
      adlib_write(0xc0 + chan, (instr[channel[chan].inst][8] & 1) + (eff_op << 1));
      break;
    case 0xa0:		                    // set carrier volume
      vol = eff_op << 2;
      adlib_write(0x43 + op_table[chan], vol | (instr[channel[chan].inst][2] & ~63));
      break;
    case 0xb0:		                    // set modulator volume
      vol = eff_op << 2;
      if (instr[inst][8] & 1)
	adlib_write(0x40 + op_table[chan], vol | (instr[channel[chan].inst][3] & ~63));
      else
	adlib_write(0x40 + op_table[chan],vol | (instr[inst][3] & ~63));
      break;
    case 0xc0:		                    // set instrument volume
      db = eff_op << 2;
      adlib_write(0x43 + op_table[chan], db | (instr[channel[chan].inst][2] & ~63));
      if (instr[inst][8] & 1)
	adlib_write(0x40 + op_table[chan], db | (instr[channel[chan].inst][3] & ~63));
      break;
    case 0xd0: pattbreak++; songpos = eff_op; songend = 1; break;	// position jump
    case 0xf0:							// set speed
      speed = eff_op;
      del = ++speed;
      break;
    }

    if(fadein)						// fade-in volume setting
      setvolume(chan,fadein*2,fadein*2);

    if(!note)						// note handling
      continue;
    note--;

    if ((note == 0x7f-1) || ((note/12) & ~7)) {    // pause (7fh)
      adl_freq[chan] &= ~32;
      adlib_write(0xb0 + chan,adl_freq[chan]);
      continue;
    }

    // play the note
    if(mtkmode)		// imitate MPU-401 Trakker bug
      note--;
    Okt = ((note/12) & 7) << 2;
    Fnr = note_table[(note % 12)] + instr[inst][11] + channel[chan].slide;
    channel[chan].freq = Fnr;
    if(!mode6 || chan < 6)
      adl_freq[chan] = Okt | 32;
    else
      adl_freq[chan] = Okt;		// never set key for drums
    adlib_write(0xb0 + chan, 0);
    setfreq(chan,Fnr);
    if(mode6) {
      switch(chan) {		// play drums
      case 6: adlib_write(0xbd,bd & ~16); bd |= 48; break;	// bass drum
      case 7: adlib_write(0xbd,bd & ~1); bd |= 33; break;	// hihat
      case 8: adlib_write(0xbd,bd & ~2); bd |= 34; break;	// cymbal
      }
      adlib_write(0xbd,bd);
    }
  }

  del = speed;		// player speed-timing
  if(pattbreak) {		// do post-effect handling
    pattpos=0;			// pattern break!
    pattbreak=0;
    songpos++;
    songpos %= 50;
    if(!songpos)
      songend = 1;
  } else {
    pattpos++;
    pattpos &= 63;		// advance in pattern data
    if (!pattpos) {
      songpos++;
      songpos %= 50;
      if(!songpos)
	songend = 1;
    }
  }
  return !songend;		// still playing
}

void hsc_rewind(int subsong)
{
  int i;								// counter

  // rewind HSC player
  pattpos = 0; songpos = 0; pattbreak = 0; speed = 2;
  del = 1; songend = 0; mode6 = 0; bd = 0; fadein = 0;

//  fm_init();						// reset OPL chip
  adlib_write(1,32); adlib_write(8,128); adlib_write(0xbd,0);

  for(i=0;i<9;i++)
    setinstr((char) i,(char) i);	// init channels
}



void hsc_init(void)
{
	opl_set_advance_callback(hsc_timer);
}



void hsc_play(uint8_t *hsc,uint8_t loop)
{
	hsc_init();
	hsc_parse(hsc);
	hsc_rewind(0);
	hsc_enabled=1;
	hsc_loop=loop;
}

void hsc_timer(void)
{
	static double time,time2;
	if(!hsc_enabled) return;
	time+=(1.0/freq);
	if(time>=time2)
	{
		time2=time2+(1.0/18.2);
		if(hsc_loop)
		{
			if(!hsc_update())
			{
				hsc_loop--;
				if(!hsc_loop) hsc_enabled=0;
			}
		}else
		if (!hsc_update())
		{
		    hsc_rewind(0);
		}
	}
	
}

uint8_t hsc_get_status(void)
{
	return hsc_enabled;
}
uint32_t hsc_get_pos(void)
{
	return songpos;
}

uint32_t hsc_get_pattpos(void)
{
	return pattpos;
}
