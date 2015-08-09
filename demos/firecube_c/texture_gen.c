#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <math.h>
#include <unistd.h>




const int bpp=4;
const int n_of_stars=1000;
const int color_deviation=30;


char *space_texture;

char time_texture[]={	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
						0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
						0x00, 0x00, 0xff, 0x88, 0x88, 0xff, 0x00, 0x00,
						0x00, 0x00, 0xff, 0x88, 0x88, 0xff, 0x00, 0x00,
						0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00,
						0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00,
						0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00,
						0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00,
						0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00,
						0x00, 0xff, 0x00, 0x88, 0x88, 0x00, 0xff, 0x00,
						0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
						0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

GLuint bkg_texture2_id;
GLuint spark_texture2_id,time_texture_id;

char *blue_fire_texture;

char *green_fire_texture;
GLuint blue_fire_texture_id;
GLuint green_fire_texture_id;
char *rnd_textures[128];
GLuint rnd_textures_id[128];
int n_of_rnd_textures;
int cur_rnd_texture=0;

GLuint marble_texture2_id;
char marble_alpha=200;

long int st_width,st_height,bf_width,bf_height,rn_width,rn_height;

void create_time_texture(void)
{
	
	glGenTextures(1,&time_texture_id);
	glBindTexture(GL_TEXTURE_2D, time_texture_id);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 8, 16, 0, GL_LUMINANCE,GL_UNSIGNED_BYTE, time_texture);
}

void bind_time_texture(void)
{
	glBindTexture(GL_TEXTURE_2D, time_texture_id);
}

void create_bkg_texture2(void)
{
	const int w=256;
	const int h=256;
	const char bpp=3;
	const int N_points=50;
	const double max=2.02;
	const double min=1.99;
	double *map,tmp,x,y,lum;
	char *pixels,p;
	int i,j,k;
//	struct 
//	{
//		double x,y;	
//	} *seed_points;
	double *seed_points_x;
	double *seed_points_y;
	
	seed_points_x=calloc(N_points,sizeof(double));
	seed_points_y=calloc(N_points,sizeof(double));
	map=calloc(w*h,sizeof(double));
	pixels=malloc(h*w*bpp);

	for(i=0;i<N_points;i++)
	{
		seed_points_x[i]=((double)random()/RAND_MAX)*(double)w ;
		seed_points_y[i]=((double)random()/RAND_MAX)*(double)h ;
	}

	for(j=0;j<h;j++)
	for(i=0;i<w;i++)
	{
		tmp=0;
		for(k=0;k<N_points;k++)
		{
			x=(double)i-seed_points_x[k];
			y=(double)j-seed_points_y[k];
			lum=sqrt((x*x)+(y*y));
			if(lum==0)lum=0.00001;
			tmp=tmp+1/lum;
			x=(double)i-seed_points_x[k]+w;
			y=(double)j-seed_points_y[k];
			lum=sqrt((x*x)+(y*y));
			if(lum==0)lum=0.00001;
			tmp=tmp+1/lum;
			x=(double)i-seed_points_x[k]-w;
			y=(double)j-seed_points_y[k];
			lum=sqrt((x*x)+(y*y));
			if(lum==0)lum=0.00001;
			tmp=tmp+1/lum;
			x=(double)i-seed_points_x[k];
			y=(double)j-seed_points_y[k]+h;
			lum=sqrt((x*x)+(y*y));
			if(lum==0)lum=0.00001;
			tmp=tmp+1/lum;
			x=(double)i-seed_points_x[k];
			y=(double)j-seed_points_y[k]-h;
			lum=sqrt((x*x)+(y*y));
			if(lum==0)lum=0.00001;
			tmp=tmp+1/lum;
			x=(double)i-seed_points_x[k]+w;
			y=(double)j-seed_points_y[k]+h;
			lum=sqrt((x*x)+(y*y));
			if(lum==0)lum=0.00001;
			tmp=tmp+1/lum;
			x=(double)i-seed_points_x[k]-w;
			y=(double)j-seed_points_y[k]+h;
			lum=sqrt((x*x)+(y*y));
			if(lum==0)lum=0.00001;
			tmp=tmp+1/lum;
			x=(double)i-seed_points_x[k]+w;
			y=(double)j-seed_points_y[k]-h;
			lum=sqrt((x*x)+(y*y));
			if(lum==0)lum=0.00001;
			tmp=tmp+1/lum;
			x=(double)i-seed_points_x[k]-w;
			y=(double)j-seed_points_y[k]-h;
			lum=sqrt((x*x)+(y*y));
			if(lum==0)lum=0.00001;
			tmp=tmp+1/lum;
			sleep(0);
		}
		map[i+j*w]=tmp;
	}

	for(i=0;i<(w*h);i++)
	{
		tmp=map[i];
		if((tmp>max)||(tmp<min)) lum=0;
		else
			lum=tmp-min;
		if(lum>((max-min)/2.0)) lum=((max-min)/2.0)-(lum-((max-min)/2.0));
			
		p=30+(char)(120.0*lum/((max-min)/2.0));
		
		
		pixels[i*bpp+0]=p;
		pixels[i*bpp+1]=p;
		pixels[i*bpp+2]=p;
	}

	glGenTextures(1,&bkg_texture2_id);
	glBindTexture(GL_TEXTURE_2D, bkg_texture2_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB,GL_UNSIGNED_BYTE, pixels);
	
	free(pixels);
	free(map);
	free(seed_points_y);
	free(seed_points_x);
}

void create_marble_texture2(void)
{
	const int w=256;
	const int h=256;
	const char bpp=4;
	const int N_points=30;
//	const double max=0.79;
	const double min=1.210;
	double *map,tmp,x,y,lum;
	char *pixels,p,q;
	int i,j,k;
//	struct 
//	{
//		double x,y;	
//	} *seed_points;
	double *seed_points_x;
	double *seed_points_y;
	
	seed_points_x=calloc(N_points,sizeof(double));
	seed_points_y=calloc(N_points,sizeof(double));
	map=calloc(w*h,sizeof(double));
	pixels=malloc(h*w*bpp);

	for(i=0;i<N_points;i++)
	{
		seed_points_x[i]=((double)random()/RAND_MAX)*(double)w ;
		seed_points_y[i]=((double)random()/RAND_MAX)*(double)h ;
	}

	for(j=0;j<h;j++)
	for(i=0;i<w;i++)
	{
		tmp=0;
		for(k=0;k<N_points;k++)
		{
			x=(double)i-seed_points_x[k];
			y=(double)j-seed_points_y[k];
			lum=sqrt((x*x)+(y*y));
			if(lum==0)lum=0.00001;
			tmp=tmp+1/lum;
			x=(double)i-seed_points_x[k]+w;
			y=(double)j-seed_points_y[k];
			lum=sqrt((x*x)+(y*y));
			if(lum==0)lum=0.00001;
			tmp=tmp+1/lum;
			x=(double)i-seed_points_x[k]-w;
			y=(double)j-seed_points_y[k];
			lum=sqrt((x*x)+(y*y));
			if(lum==0)lum=0.00001;
			tmp=tmp+1/lum;
			x=(double)i-seed_points_x[k];
			y=(double)j-seed_points_y[k]+h;
			lum=sqrt((x*x)+(y*y));
			if(lum==0)lum=0.00001;
			tmp=tmp+1/lum;
			x=(double)i-seed_points_x[k];
			y=(double)j-seed_points_y[k]-h;
			lum=sqrt((x*x)+(y*y));
			if(lum==0)lum=0.00001;
			tmp=tmp+1/lum;
			x=(double)i-seed_points_x[k]+w;
			y=(double)j-seed_points_y[k]+h;
			lum=sqrt((x*x)+(y*y));
			if(lum==0)lum=0.00001;
			tmp=tmp+1/lum;
			x=(double)i-seed_points_x[k]-w;
			y=(double)j-seed_points_y[k]+h;
			lum=sqrt((x*x)+(y*y));
			if(lum==0)lum=0.00001;
			tmp=tmp+1/lum;
			x=(double)i-seed_points_x[k]+w;
			y=(double)j-seed_points_y[k]-h;
			lum=sqrt((x*x)+(y*y));
			if(lum==0)lum=0.00001;
			tmp=tmp+1/lum;
			x=(double)i-seed_points_x[k]-w;
			y=(double)j-seed_points_y[k]-h;
			lum=sqrt((x*x)+(y*y));
			if(lum==0)lum=0.00001;
			tmp=tmp+1/lum;
			sleep(0);
		}
		map[i+j*w]=tmp;
	}

	for(i=0;i<(w*h);i++)
	{
		tmp=map[i];
		q=50+(char)(10*(double)random()/RAND_MAX);
		if(tmp<min) p=255; 
		else
			p=marble_alpha;
	//	if (fabs(tmp-min)<0.0015) q=0;
		
		
		
		pixels[i*bpp+0]=q;
		pixels[i*bpp+1]=q;
		pixels[i*bpp+2]=q;
		pixels[i*bpp+3]=p;
	}

	

	glGenTextures(1,&marble_texture2_id);
	glBindTexture(GL_TEXTURE_2D, marble_texture2_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,GL_UNSIGNED_BYTE, pixels);
	
	free(pixels);
	free(map);
	free(seed_points_y);
	free(seed_points_x);
}


void create_spark_texture2(void)
{
	const int w=32;
	const int h=32;
	const char bpp=4;
	double *map,tmp;
	char *pixels;
	int i,j;
	
	map=calloc(w*h,sizeof(double));
	pixels=malloc(h*w*bpp);

	for(j=0;j<h;j++)
		for(i=0;i<w;i++)
		{
			map[i+j*w]=sqrt((i-w/2.0)*(i-w/2.0)+(j-h/2.0)*(j-h/2.0));
		}

	for(i=0;i<(w*h);i++)
	{
		tmp=((255*((w/2.0)-map[i]))/16.0);
		if(tmp<0)tmp=0;
		
		pixels[bpp*i+0]=255;
		pixels[bpp*i+1]=255;
		pixels[bpp*i+2]=255;
		pixels[bpp*i+3]=(char)tmp;
	}

	
	
	glGenTextures(1,&spark_texture2_id);
	glBindTexture(GL_TEXTURE_2D, spark_texture2_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,GL_UNSIGNED_BYTE, pixels);
	
	free(pixels);
	free(map);
	
}


void bind_marble_texture2(void)
{
	glBindTexture(GL_TEXTURE_2D, marble_texture2_id);
}


void bind_spark_texture2(void)
{
	glBindTexture(GL_TEXTURE_2D, spark_texture2_id);
}




void bind_bkg_texture2()
{
	glBindTexture(GL_TEXTURE_2D, bkg_texture2_id);
}




void create_blue_fire_texture(int width,int height)
{
	bf_width=width;
	bf_height=height;
	glGenTextures(1, &blue_fire_texture_id);
	blue_fire_texture=malloc(width*height*bpp);
	
}

void create_green_fire_texture(int width,int height)
{
	bf_width=width;
	bf_height=height;
	glGenTextures(1, &green_fire_texture_id);
	green_fire_texture=malloc(width*height*bpp);
	
}

void free_blue_fire_texture(void)
{
	free(blue_fire_texture);
}

void free_green_fire_texture(void)
{
	free(green_fire_texture);
}


void convert_r_to_b(char *texture)
{
	long int i,j=0,k=0;
	for(i=0;i<(bf_width*bf_height);i++)
	{
		blue_fire_texture[j++]=texture[k+2];
		blue_fire_texture[j++]=texture[k+1];
		blue_fire_texture[j++]=texture[k]; 
		blue_fire_texture[j++]=255;
		j=j-4;
		green_fire_texture[j++]=texture[k+2];
		green_fire_texture[j++]=texture[k];
		green_fire_texture[j++]=texture[k+1]; k=k+3;
		green_fire_texture[j++]=255;
	}
	
//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, blue_fire_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
//    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bf_width, bf_height, 0, GL_RGBA,GL_UNSIGNED_BYTE, blue_fire_texture);
	glBindTexture(GL_TEXTURE_2D, green_fire_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
//    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bf_width, bf_height, 0, GL_RGBA,GL_UNSIGNED_BYTE, green_fire_texture);
}

void bind_blue_fire_texture(void)
{
	glBindTexture(GL_TEXTURE_2D, blue_fire_texture_id);
}

void bind_green_fire_texture(void)
{
	glBindTexture(GL_TEXTURE_2D, green_fire_texture_id);
}

void create_rnd_texture(int width, int height, int n)
{
	long int i,j,k;
	rn_width=width;
	rn_height=height;
	n_of_rnd_textures=n;
	glGenTextures(n,rnd_textures_id);
	for(i=0;i<n;i++)
	{
		rnd_textures[i]=malloc(width*height*bpp);
		k=0;
		for(j=0;j<(width*height);j++)
		{
			rnd_textures[i][k++]=random() % 255;
			rnd_textures[i][k++]=random() % 255;
			rnd_textures[i][k++]=random() % 255;
			rnd_textures[i][k++]=255;
		}
	glBindTexture(GL_TEXTURE_2D, rnd_textures_id[i]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rn_width, rn_height, 0, GL_RGBA,GL_UNSIGNED_BYTE, rnd_textures[i]);
	}
	
}

void free_rnd_texture(void)
{
	int i;
	for(i=0;i<n_of_rnd_textures;i++)
	{
		free(rnd_textures[i]);
	}
}

void bind_rnd_texture(void)
{
	glBindTexture(GL_TEXTURE_2D,rnd_textures_id[cur_rnd_texture]);
}

void update_rnd_texture(void)
{
	cur_rnd_texture=(cur_rnd_texture+1) % n_of_rnd_textures;
	bind_rnd_texture();
	
}


