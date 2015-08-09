 /*
  *      This program is free software; you can redistribute it and/or modify
  *      it under the terms of the GNU General Public License as published by
  *      the Free Software Foundation; either version 2 of the License, or
  *      (at your option) any later version.
  *      
  *      This program is distributed in the hope that it will be useful,
  *      but WITHOUT ANY WARRANTY; without even the implied warranty of
  *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *      GNU General Public License for more details.
  *      
  *      You should have received a copy of the GNU General Public License
  *      along with this program; if not, write to the Free Software
  *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  *      MA 02110-1301, USA.
  */

#include "SDL.h"
#include <math.h>
#include <GL/gl.h>
#include "textures.h"
#include "textures_constants.h"
#include "sound.h"
#include "fft.h"

#define meteor_x width*0.7
#define meteor_y height*0.7
#define meteor_w width/4
#define meteor_h height/5

Cmplx spectr_data[512];


double ry[]={0.0, 255.0,   255.0,      200.0,      150.0,    250,         250.0,  250,    200.0,    200.0,    60,      40,     255.0};
double gy[]={0.0, 0.0,   000.0,      200.0,      100.0,    200,         250.0,   240,    100.0,    60.0,    000,       000,     255.0};
double by[]={0.0, 0.0,   000.0,      000.0,      100.0,    000,         000.0,  200,    000.0,    000.0,    000,       000,     255.0};
double ay[]={0.0, 255.0, 255.0,      255.0,      255.0,    255,         255.0,  255,    255.0,    250,      255,      200,     0.0};
double xx[]={0.0, 0000.00, 900,       1000,       3100,     45000,      50000,  51000,  55000,    63000,   64000,   65000,     65535};

GLuint fire_texture;
GLuint bkg_texture;
GLuint meteor_texture;
GLuint spectr_texture;

Uint8 *buf1, *buf2, *rndbuf, *rbf, *ball, *fire_buf, *bkg_buf, *stars_buf, *meteor_img_buf, *spectr_buf;
Uint16 *meteor_raw_buf1,*meteor_raw_buf2;

Uint8 anim_enabled;
void *my_callback_flame_param;
Uint32 rnd_offs;

typedef struct
{
  Uint8 r, g, b;
} my_col;

typedef struct
{
  Uint8 r, g, b,a;
} my_col_alpha;


my_col pal[256];

my_col_alpha pal16[65536];

typedef struct
{
  float x, y;
  float dx, dy;
} bal;

bal ball1;

void put_pixel16 (Uint8 * buf, int x, int y, Uint16 col);
void bkg_update(void)
{
    double static ph;
    uint16_t y0,y1,i,j,yz;
    uint8_t c;
    uint16_t r,g,b;
    float k;
    
    ph+=0.312;
    if(ph>360.0) ph-=360.0;
    yz=(height/4);
    y0=yz+(uint16_t)((height/8.0)*sin(M_PI*ph/180.0));
    y1=(y0+(height/4));
    k=255.0/(float)(height-y0);
    
    for(i=0;i<height;i++)
    {
        for(j=0;j<width;j++)
        {
            c=0;
            
            if(i<y1)
            {
                c=stars_buf[((width*(int)(i-y0+yz*2))+j)];
                bkg_buf[3*((width*i)+j)]=c;
                bkg_buf[3*((width*i)+j)+1]=c;
                bkg_buf[3*((width*i)+j)+2]=c;
            }
            if(i>y0)
            {
                r=c;
                g=c+(0.5*k*(i-y0)); if (g>255) g=255;
                b=c+(0.7*k*(i-y0)); if (b>255) b=255;
                bkg_buf[3*((width*i)+j)]=r;
                bkg_buf[3*((width*i)+j)+1]=g;
                bkg_buf[3*((width*i)+j)+2]=b;
            }
        }   
    }

}




void bind_bkg_texture(void)
{
    glBindTexture(GL_TEXTURE_2D, bkg_texture);
}

void update_bkg_texture(void)
{
    glBindTexture(GL_TEXTURE_2D, bkg_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,GL_UNSIGNED_BYTE,bkg_buf );
}


void genpal16(void)
{
    uint16_t i,cur;
    float kr,r0,r1,x;
    float kg,g0,g1;
    float kb,b0,b1;
    float ka,a0,a1;
    cur=1;
    r0=ry[1];
    r1=ry[2];
    kr=(r1-r0)/(xx[cur+1]-xx[cur]);
    g0=gy[1];
    g1=gy[2];
    kg=(g1-g0)/(xx[cur+1]-xx[cur]);
    b0=by[1];
    b1=by[2];
    kb=(b1-b0)/(xx[cur+1]-xx[cur]);
    a0=ay[1];
    a1=ay[2];
    ka=(a1-a0)/(xx[cur+1]-xx[cur]);
    x=xx[cur];
    for(i=0;i<65535;i++)
    {
        if(i>=xx[cur+1])
        {
            cur=cur+1;
            r0=r1;
            r1=ry[cur+1];
            kr=(r1-r0)/(xx[cur+1]-xx[cur]);
            x=xx[cur];
            g0=g1;
            g1=gy[cur+1];
            kg=(g1-g0)/(xx[cur+1]-xx[cur]);
            b0=b1;
            b1=by[cur+1];
            kb=(b1-b0)/(xx[cur+1]-xx[cur]);
            a0=a1;
            a1=ay[cur+1];
            ka=(a1-a0)/(xx[cur+1]-xx[cur]);
        }
    
        pal16[65535-i].r=r0+kr*(i-x);
        pal16[65535-i].g=g0+kg*(i-x);
        pal16[65535-i].b=b0+kb*(i-x);
        pal16[65535-i].a=a0+ka*(i-x);

//        printf("%d\n",(int)(r0+kr*(i-x)));
    }
}


void meteor_source()
{
    uint16_t x,y,x0,y0,c,h,w,i;
    float r=meteor_w;
    x0=meteor_x + random() % (int)(r/10);
    y0=meteor_y + random() % (int)(r/10);
    long int i0;

    w=meteor_w;
    h=meteor_h;

  i0 = random () % (width);
  for (y=0;y<meteor_h;y++)
  {
      i0 = random () % (width);
    for (x=0;x<meteor_w;x++)  
    {
        if ((rndbuf[i0+x]) && (hypot((x-w/2), (y-h/2))<r/4.3))
        {
            c=65535;
            meteor_raw_buf1[(x+x0)+(y+y0)*width]=c;
        }
    }
}

    for(i=0;i<3;i++)
    {
        i0=random() % (width*height);
        meteor_raw_buf1[i0]=65535;
    }
}

void meteor_blure(void)
{
    uint16_t x,y;
    uint8_t m_scroll_x=4;
    uint8_t m_scroll_y=2;
    uint32_t c,p;
    
    for(y=0;y<(height-m_scroll_y-1);y++)
    {
        for(x=0;x<(width-m_scroll_x-1);x++)
        {
            p=(x+m_scroll_x)+(width*(y+m_scroll_y));
            c=(double)(meteor_raw_buf1[p-width-1]*0.8+meteor_raw_buf1[p-width]+meteor_raw_buf1[p-width+1]*0.8+
               meteor_raw_buf1[p-1]  +    meteor_raw_buf1[p]*1.4      +meteor_raw_buf1[p+1]+
               meteor_raw_buf1[p+width-1]*0.8+meteor_raw_buf1[p+width]+meteor_raw_buf1[p+width+1]*0.8)/9.1;
            meteor_raw_buf2[x+(y*width)]=c;
        }
    }
    memcpy(meteor_raw_buf1,meteor_raw_buf2,width*height*2);
}

void meteor_update(void)
{
    meteor_source();
    meteor_blure();
}

void meteor_update_texture(void)
{
    int i;
  for (i = 0; i < height * width; i++)
    {
      put_pixel16 (meteor_img_buf, i % width, i / width, meteor_raw_buf2[i]);
    }


    glBindTexture(GL_TEXTURE_2D, meteor_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,GL_UNSIGNED_BYTE,meteor_img_buf );
 
}

void bind_meteor_texture(void)
{
        glBindTexture(GL_TEXTURE_2D, meteor_texture);
}


void spectr_update(void)
{
    int16_t *buf,i,j;
    buf=get_buf();
    if(!buf) return;
    float a;
    for(i=0;i<512;i++)
    {
        spectr_data[i].Re=buf[i]*sin(i*M_PI/512);
        spectr_data[i].Im=0;
    }
    FFTC(spectr_data,512);
    for(i=0;i<256*64;i++)
    {

        spectr_buf[4*(i)+0]=0;
        spectr_buf[4*(i)+1]=0;
        spectr_buf[4*(i)+2]=0;
        spectr_buf[4*(i)+3]=100;
    }
    for(i=0;i<256;i++)
    {
        a=(hypotf(spectr_data[i].Re,spectr_data[i].Im)/128);
        if(a>64) a=64;
        for(j=5;j<a;j++)
        {
            spectr_buf[4*(i+(j*256))]=255;
            spectr_buf[4*(i+(j*256))+1]=255;
            spectr_buf[4*(i+(j*256))+2]=255;
        }
    }
  
}


void spectr_texture_update(void)
{
    glBindTexture(GL_TEXTURE_2D, spectr_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 64, 0, GL_RGBA,GL_UNSIGNED_BYTE,spectr_buf );
}

void genpal (void)
{
  int i, j;
  float kr, kg, kb;

  for (i = 0; i < 256; i++)
    {
      pal[i].r = 0;
      pal[i].g = 0;
      pal[i].b = 0;
    }

  for (i = 72; i < 256; i++)
    {
      j = i - 72;
      pal[i].r = 255;
      pal[i].g = 255;
      pal[i].b = 255;
    }
  kb = 255 / (72 - 64);
  for (i = 64; i < 72; i++)
    {
      j = i - 64;
      pal[i].r = 255;
      pal[i].g = 255;
      pal[i].b = (Uint8) (kb * j);

    }

  kg = 255 / (64 - 50);
  for (i = 50; i < 64; i++)
    {
      j = i - 50;
      pal[i].r = 255;
      pal[i].g = (Uint8) (kg * j);
      pal[i].b = 0;
    }
  kr = 25.5;
  for (i = 40; i < 50; i++)
    {
      j = i - 40;
      pal[i].r = (Uint8) (kr * j);
      pal[i].g = 0;
      pal[i].b = 0;
    }
}




void clear_bufer (Uint8 * b)
{
  memset (b, 0, width * height);
}
void clear_bufer16 (Uint16 *b)
{
  memset (b, 0, width * height);
}

void texture_init (void)
{
    uint32_t t;
 glGenTextures(1,&fire_texture);
 glGenTextures(1,&bkg_texture);
 glGenTextures(1,&meteor_texture);
 glGenTextures(1,&spectr_texture);
  anim_enabled = 1;
  Uint32 i;
  genpal ();
  genpal16();
  buf1 = (Uint8 *) malloc (width * height);
  buf2 = (Uint8 *) malloc (width * height);
  rndbuf = (Uint8 *) malloc (width * 30);
  rbf = (Uint8 *) malloc (10 * width * height);
  fire_buf = (Uint8 *) malloc (3 * width * height);
  bkg_buf= (Uint8 *) malloc (3 * width * height);
  stars_buf= (Uint8 *) malloc (width * height);
  meteor_img_buf= (Uint8 *) malloc (4 * width * height);
  meteor_raw_buf1= malloc (2*width * height);
  meteor_raw_buf2= malloc (2*width * height);
  spectr_buf=malloc(4*256*64);
  for (i = 0; i < (10 * width * height); i++)
    rbf[i] = random () % 255;
  for (i = 0; i < (width * 30); i++)
    {
      rndbuf[i] = intence * (random () % 2);
    }

   for (i=0;i<128;i++)
   {
       t=random() % (width*height);
       stars_buf[t]=random() % 255;
   }
    
  clear_bufer (buf1);
  clear_bufer (buf2);
  clear_bufer16(meteor_raw_buf1);
  clear_bufer16(meteor_raw_buf2);
  fire_update();
  
}


/* ------------------------------------------------------ */

void put_pixel_no_sdl (Uint8 * buf, int x, int y, Uint8 col)
{
  Uint32 i;
  i = 3 * (x + (width * y));
  buf[i++] = pal[col].r;
  buf[i++] = pal[col].g;
  buf[i++] = pal[col].b;
}


void put_pixel16 (Uint8 * buf, int x, int y, Uint16 col)
{
  Uint32 i;
  i = 4 * (x + (width * y));
/*  if(col)
  {
  buf[i++] = 0;
  buf[i++] = 0;
  buf[i++] = 0;
  buf[i++] = 255;
  }
 */ 
  buf[i++] = pal16[col].r;
  buf[i++] = pal16[col].g;
  buf[i++] = pal16[col].b;
  buf[i++] = pal16[col].a;
}




void endfire (void)
{
  free (buf1);
  free (buf2);
  free (rndbuf);
  free (rbf);
  free (ball);
  free (fire_buf);
}

void rnd_frame (void)
{
  rnd_offs = random () % (6 * width * height);
}

void flame_source (void)
{
  int i;
  long int i0;

  i0 = random () % (width * 26);
  for (i = 1; i < width * 3; i++)
    {
      buf1[(height * width) - i] = rndbuf[i + i0];
    }
}


void view_bufer_no_sdl (Uint8 * dst_buf, Uint8 * src_buf)
{
  int i;
  for (i = 0; i < height * width; i++)
    {
      put_pixel_no_sdl (dst_buf, i % width, i / width, src_buf[i]);
    }
}

void
flame_frame (void)
{
  Uint32 i, j, w, t;
  w = width;
  memcpy (buf1, buf2, height * w);
  flame_source ();

  for (i = 0; i < (w * (height - sc - 2) - 1); i++)
    {
      j = i + (w * sc);
      t =
	((Uint16)
	 (buf1[j - w - 1] + buf1[j - w] + buf1[j - w + 1] + buf1[j - 1] +
	  buf1[j + 1] + buf1[j + w - 1] + buf1[j + w] + buf1[j + w +
							     1]) >> 3);
      buf2[i] = (Uint8) t;
    }
}

Uint8 *
bufer (void)
{
  return buf2;
}

Uint8 *
bufer2 (void)
{
  Uint8 *p;
  p = (Uint8 *) rbf + rnd_offs;
  return p;
}

Uint8 *
bufer3 (void)
{
  return fire_buf;
}

/*void
anim_stop (void)
{
  anim_enabled = 0;
}

void
anim_start (void)
{
  anim_enabled = 1;
}
*/
void fire_update(void)
{

    
    flame_frame ();
//    move_ball ();
//    rnd_frame ();
    flame_updated=1;
    bkg_update();
    meteor_update();
    spectr_update();
}


void update_texture(void)
{
        view_bufer_no_sdl(fire_buf,buf1);
    
    glBindTexture(GL_TEXTURE_2D, fire_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,GL_UNSIGNED_BYTE,fire_buf );
}

void bind_fire_texture(void)
{
    glBindTexture(GL_TEXTURE_2D, fire_texture);
}


