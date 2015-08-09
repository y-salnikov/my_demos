#include "SDL.h"
#include "SDL_mixer.h"
#include <math.h>

const int freq=44100;
const Uint16 format=AUDIO_S16SYS;
const int channels=2;
const int chunksize=1024;
const int n_channels=128;

const double piano_volume=0.4;
const double drum1_volume=0.7;
const double drum2_volume=0.7;
const double strings_volume=0.30;

typedef struct smpl
	{
		long int length;
		double frequency;
		Sint16 *sample_data;
		char channels;
	} sample;

char chans[16][16];
char notes[16][128];

double note_freq[128] =
{
/* C     C#    D     D#    E     F     F#    G     G#    A     A#    B */
   16,   17,   18,   19,   21,   22,   23,   24,   26,   28,   29,   31,
   33,   34,   37,   39,   41,   44,   46,   49,   52,   55,   58,   62,
   65,   69,   73,   78,   82,   87,   92,   98,  103,  110,  117,  123,
  131,  139,  147,  156,  165,  175,  185,  195,  207,  220,  233,  247,
  262,  277,  294,  311,  330,  349,  370,  392,  415,  440,  466,  494,
  523,  554,  587,  622,  659,  698,  740,  784,  831,  880,  932,  988,
 1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,
 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951,
 4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902,
 8372, 8870, 9397, 9956,10548,11175,11840,12544,13290,14080,14917,15804,
16744,17740,18795,19912,21096,22351,23680,25088
};

Mix_Chunk *piano_note_chunk[128],*drum1,*drum2,*strings_note_chunk[128];

sample piano_base_mid;
sample piano_base_high;
sample piano_base_low;

sample drum1_s,drum2_s,strings;

void free_piano_chunks(void);

void sound_end(void)
{
	int i;
	Mix_HaltChannel(-1);
	free_piano_chunks();
	Mix_FreeChunk(drum1);
	Mix_FreeChunk(drum2);
	for(i=0;i<128;i++)
	{
		Mix_FreeChunk(strings_note_chunk[i]);
	}
	Mix_CloseAudio();
}


void create_drum1(void)
{
	int i,j;
	const double a0=32767.0*5;
	const int h=5;
	const double a[6]={1.0, 100,50,25,12,6 };
	double a1,a2,et,as;
	drum1_s.frequency=30;
	drum1_s.length=(0.3)*freq*2;
	drum1_s.channels=2;
	drum1_s.sample_data=calloc(drum1_s.length,2);
	
	et=10.0/(drum1_s.length);
	for(i=0;i<drum1_s.length/drum1_s.channels;i++)
	{
		a1=exp(-et*i);
		a2=0;as=0;
		for(j=1;j<h+1;j++)
		{
			a2=a2+a[j]*sin(j*i*2.0*M_PI/(double)(freq/drum1_s.frequency));
			as+=a[j];
		}
		//a2=0.8*sin(i*2*M_PI/(double)(freq/drum1_s.frequency))+0.29*sin(i*4*M_PI/(double)(freq/drum1_s.frequency))+0.01*(1-(2.0*random()/(double)RAND_MAX));
		a2=a2/as;
		as=(a0*a1*a2);
		if(as>32767) as=32767;
		if(as<(-32767)) as=-32767;
		
		drum1_s.sample_data[i*2]=(Sint16)as;
		drum1_s.sample_data[i*2+1]=(Sint16)-as;
	}
	
	drum1=Mix_QuickLoad_RAW((void *)drum1_s.sample_data,drum1_s.length*2);
	Mix_VolumeChunk(drum1,(int)(MIX_MAX_VOLUME*drum1_volume));
	
}

void create_drum2(void)
{
	int i;
	double a0,a1,a2,et;
	drum2_s.frequency=50;
	drum2_s.length=freq*2;
	drum2_s.channels=2;
	drum2_s.sample_data=calloc(drum2_s.length,2);
	a0=32767;
	et=10.4/drum2_s.length;
	for(i=0;i<drum2_s.length;i++)
	{
		a1=exp(-et*i);
		a2=0.5*sin(i*2*M_PI/(double)(freq/drum2_s.frequency))+0.5*(1-(2.0*random()/(double)RAND_MAX));
		drum2_s.sample_data[i]=(Sint16)(a0*a1*a2);
	}
	
	drum2=Mix_QuickLoad_RAW((void *)drum2_s.sample_data,drum2_s.length*2);
	Mix_VolumeChunk(drum2,(int)(MIX_MAX_VOLUME*drum2_volume));
	
}


void create_piano_base_sample_mid(void)
{
	const double a0=32767.0;
	const int h=15;
	const double a[16]={1.0,  1850,650,240,60,150,100,50,128,0,50,25,0,70,20,30};
	
	int i,j;
	double A,An;
	
	piano_base_mid.frequency=110;
	piano_base_mid.length=freq/piano_base_mid.frequency;
	piano_base_mid.channels=1;
	piano_base_mid.sample_data=calloc(piano_base_mid.length,sizeof(Sint16));
	for(i=0;i<piano_base_mid.length;i++)
	{
		A=0;An=0;
		for(j=1;j<h+1;j++)
		{
			A=A+a[j]*sin(j*i*2.0*M_PI/(double)piano_base_mid.length);
			An+=a[j];
		}
		piano_base_mid.sample_data[i]=A*a0/An;
		//piano_base_mid.sample_data[i]=(Sint16)a0*(a1*sin(2*M_PI*i/(double)piano_base_mid.length));
	}
}

void create_strings_base_sample(void)
{
	const double a0=32767.0;
	const int h=15;
	const double a[16]={1.0,  300,1000,240,12,15,5,4,1,1,1,5,0,1,1,3};
	
	int i,j;
	double A,An;
	
	strings.frequency=110;
	strings.length=freq/strings.frequency;
	strings.channels=1;
	strings.sample_data=calloc(strings.length,sizeof(Sint16));
	for(i=0;i<strings.length;i++)
	{
		A=0;An=0;
		for(j=1;j<h+1;j++)
		{
			A=A+a[j]*(0.3*sin(j*i*M_PI/(double)strings.length)
			+0.3*sin(j*i*(1.01)*M_PI/(double)strings.length)
			+0.3*sin(j*i*1.02*M_PI/(double)strings.length));
			An+=a[j];
		}
		strings.sample_data[i]=A*a0/An;
		//strings.sample_data[i]=(Sint16)a0*(a1*sin(2*M_PI*i/(double)strings.length));
	}
}
	


void create_piano_base_sample_low(void)
{
	const double a0=32767.0;
	const int h=12;
	const double a[13]={1.0,  1500,1000,100,800,400,350,200,80};
	
	int i,j;
	double A,An;
	
	piano_base_low.frequency=110;
	piano_base_low.length=freq/piano_base_low.frequency;
	piano_base_low.channels=1;
	piano_base_low.sample_data=calloc(piano_base_low.length,sizeof(Sint16));
	for(i=0;i<piano_base_low.length;i++)
	{
		A=0;An=0;
		for(j=1;j<h+1;j++)
		{
			A=A+a[j]*sin(j*i*2.0*M_PI/(double)piano_base_low.length);
			An+=a[j];
		}
		piano_base_low.sample_data[i]=A*a0/An;
		//piano_base_low..sample_data[i]=(Sint16)a0*(a1*sin(2*M_PI*i/(double)piano_base_low..length));
	}
}


void create_piano_base_sample_high(void)
{
	const double a0=32767.0;
	const int h=20;
	const double a[21]={1.0,  1200, 2600, 937, 0,  150, 80, 120, 0,0,0,0,0,0,0,0,0,0,0,0,200 };
	
	int i,j;
	double A,An;
	
	piano_base_high.frequency=110;
	piano_base_high.length=freq/piano_base_high.frequency;
	piano_base_high.channels=1;
	piano_base_high.sample_data=calloc(piano_base_high.length,sizeof(Sint16));
	for(i=0;i<piano_base_high.length;i++)
	{
		A=0;An=0;
		for(j=1;j<h+1;j++)
		{
			A=A+a[j]*sin(j*i*2.0*M_PI/(double)piano_base_high.length);
			An+=a[j];
		}
		piano_base_high.sample_data[i]=A*a0/An;
		//piano_base_high..sample_data[i]=(Sint16)a0*(a1*sin(2*M_PI*i/(double)piano_base_high..length));
	}
}



void resample(sample *src_smp, double new_freq, char chans,  sample *dst_smp)
{
	int i;
	double di;
	Sint16 d;
	
	dst_smp->frequency=new_freq;
	dst_smp->channels=chans;
	dst_smp->length=freq/new_freq;
	di=(double)src_smp->length/((double)dst_smp->length);
	dst_smp->sample_data=calloc(dst_smp->length*chans,sizeof(Sint16));
	for(i=0;i<dst_smp->length;i++)
	{
		d=src_smp->sample_data[(int)((double)i*di)];
		dst_smp->sample_data[i*chans]=d;
		if(chans==2)dst_smp->sample_data[1+i*chans]=d;
	}
}


void resample_to_instr(sample *src_smp, double new_freq, char chans, double attak_time, double max_time, double decay_time, sample *dst_smp)
{
	long int i,si,si2,si3;
	double di,time,dt,a,ka,et,t;
	Sint16 d;
	time=attak_time+max_time+decay_time;
	
	dst_smp->frequency=new_freq;
	dst_smp->channels=chans;
	dst_smp->length=time*freq;
	di=(double)src_smp->length/((double)freq/(double)dst_smp->frequency);
	dt=1.0/(double)freq;
	dst_smp->sample_data=calloc(dst_smp->length*chans,sizeof(Sint16));
	si=0;
	ka=1.0/attak_time;
	et=10.4/decay_time;

//	printf("time=%f, length=%ld,\n",time,dst_smp->length);
	for(i=0;i<dst_smp->length;i++)
	{
		a=0;
		d=src_smp->sample_data[(int)((double)si*di)];
		t=(double)i*dt;
		if(t<attak_time) a=(t*ka);
		if((t>attak_time) && (t<max_time+attak_time)) a=1;
		if(t>max_time+attak_time) a=exp(-et*(t-(time-decay_time)));
		

		si++;
		si=si % (long int)(freq/new_freq);

		si2=(si+(int)(0.6*src_smp->length*i/(double)dst_smp->length)) % (long int)(freq/new_freq);
		si3=(si-(int)(0.6*src_smp->length*i/(double)dst_smp->length)) % (long int)(freq/new_freq);

		si2=-si2;
		if (si3<0) si3+=(long int)(freq/new_freq);
		if (si2<0) si2+=(long int)(freq/new_freq);
		
		d=src_smp->sample_data[(int)((double)si3*di)];
		dst_smp->sample_data[i*chans]=(double)d*a;
		d=src_smp->sample_data[(int)((double)si2*di)];
		if(chans==2)dst_smp->sample_data[1+i*chans]=(double)d*a;
//		if (a>0)printf("a=%f ",a);
	}
}


void create_strings_chunks(void)
{
	int i;
	sample strings_samples[128];
	create_strings_base_sample();
	for(i=0;i<128;i++)
	{
		
		if(i<0x50)resample_to_instr(&strings,note_freq[i],2,0.2,1.5,3,&strings_samples[i]);
		else resample_to_instr(&strings,note_freq[i],2,0.001,0.001,0.001,&strings_samples[i]);
		
		
		strings_note_chunk[i]=Mix_QuickLoad_RAW((void *)strings_samples[i].sample_data,strings_samples[i].channels*strings_samples[i].length*2);
	}
}


void create_piano_chunks(void)
{
	int i;
	sample piano_samples[128];
	create_piano_base_sample_low();
	create_piano_base_sample_mid();
	create_piano_base_sample_high();
	for(i=0;i<128;i++)
	{
		if(i>0x44) resample_to_instr(&piano_base_high,note_freq[i],2,0.01,0.0064,1.1,&piano_samples[i]);
		else
		if((i>0x28)&&(i<0x45))resample_to_instr(&piano_base_mid,note_freq[i],2,0.01,0.0064,1.1,&piano_samples[i]);
		else
		if(i<0x29)resample_to_instr(&piano_base_low,note_freq[i],2,0.01,0.4,5,&piano_samples[i]);
		
		piano_note_chunk[i]=Mix_QuickLoad_RAW((void *)piano_samples[i].sample_data,piano_samples[i].channels*piano_samples[i].length*2);
	}
}

void free_piano_chunks(void)
{
	int i;
	for(i=0;i<128;i++)
	{
		Mix_FreeChunk(piano_note_chunk[i]);
	}
	
}

void set_piano_volume(void)
{
	int i;
	for(i=0;i<128;i++)
	{
		Mix_VolumeChunk(piano_note_chunk[i],(int)(MIX_MAX_VOLUME*piano_volume));
	}
	
}

void set_strings_volume(void)
{
	int i;
	for(i=0;i<128;i++)
	{
		Mix_VolumeChunk(strings_note_chunk[i],(int)(MIX_MAX_VOLUME*strings_volume));
	}
	
}


void free_sample(sample *smp)
{
	free(smp->sample_data);
}

char aquare_free_channel(char instr)
{
	int i;
	for(i=0;i<16;i++)
	{
		if(chans[(int)instr][i]==0)
		{
			chans[(int)instr][i]=1;
			return i+16*instr;
		}
	}
	return -1;
}

void free_chanel(char chan)
{
	int i,c;
	i=chan/16;
	c=chan % 16;
	chans[i][c]=0;
}

void sound_play(int instr, int chan, int note)
{
	char c;
	c=aquare_free_channel(instr);
	notes[instr-1][note]=c;
	if(instr==1) Mix_PlayChannel(c,piano_note_chunk[note],0);
	if(instr==2) Mix_PlayChannel(c,drum1,0);
	if(instr==3) Mix_PlayChannel(c,strings_note_chunk[note],0);
	if(instr==4) Mix_PlayChannel(c,drum2,0);
}



void sound_stop(char instr, char chan, char note)
{
	int c;
	c=notes[(int)instr-1][(int)note];
	free_chanel(c);
	Mix_HaltChannel(c);
}



void create_samples(void)
{
	create_piano_chunks();
	set_piano_volume();
	create_strings_chunks();
	set_strings_volume();
	create_drum1();
	create_drum2();
}

void sound_init(void)
{
	int i,j;
	if(Mix_OpenAudio(freq,format,channels,chunksize)==-1)
	{
	printf("Mix_OpenAudio: %s\n", Mix_GetError());
    exit(2);
	}
	Mix_AllocateChannels(n_channels);
	create_samples();
	for (i=0;i<16;i++)
		{
			for(j=0;j<16;j++)
			chans[i][j]=0;
			for(j=0;j<128;j++)
			notes[i][j]=0;
		}
			
}
