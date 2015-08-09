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
#include "fire.h"

extern const int width;
extern const int height;
extern const int sc;
extern const int intence;
extern char flame_updated;

Uint8 *buf1, *buf2, *rndbuf, *rbf, *ball, *fire_buf;
Uint8 anim_enabled;
void *my_callback_flame_param;
Uint32 rnd_offs;

typedef struct
{
  Uint8 r, g, b;
} my_col;

my_col pal[256];

typedef struct
{
  float x, y;
  float dx, dy;
} bal;

bal ball1;



void
genpal (void)
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
void
clear_bufer (Uint8 * b)
{
  memset (b, 0, width * height);
}

void
fire_init (void)
{
  anim_enabled = 1;
  Uint32 i;
  genpal ();
  buf1 = (Uint8 *) malloc (width * height);
  buf2 = (Uint8 *) malloc (width * height);
  rndbuf = (Uint8 *) malloc (width * 30);
  rbf = (Uint8 *) malloc (10 * width * height);
  fire_buf = (Uint8 *) malloc (3 * width * height);
  for (i = 0; i < (10 * width * height); i++)
    rbf[i] = random () % 255;
  for (i = 0; i < (width * 30); i++)
    {
      rndbuf[i] = intence * (random () % 2);
    }
  clear_bufer (buf1);
  clear_bufer (buf2);
  fire_update();
  
}

void
Slock (SDL_Surface * screen)
{

  if (SDL_MUSTLOCK (screen))
    {
      if (SDL_LockSurface (screen) < 0)
	{
	  return;
	}
    }

}

/* ------------------------------------------------------ */
void
Sunlock (SDL_Surface * screen)
{

  if (SDL_MUSTLOCK (screen))
    {
      SDL_UnlockSurface (screen);
    }

}
void
put_pixel (SDL_Surface * scr, int x, int y, Uint8 col)
{
  Uint8 *bufp;
  bufp = (Uint8 *) scr->pixels + y * scr->pitch + x * 3;
  if (!(SDL_BYTEORDER == SDL_LIL_ENDIAN))
    {
      bufp[2] = pal[col].r;
      bufp[1] = pal[col].g;
      bufp[0] = pal[col].b;
    }
  else
    {
      bufp[0] = pal[col].r;
      bufp[1] = pal[col].g;
      bufp[2] = pal[col].b;
    }
}

void
put_pixel_no_sdl (Uint8 * buf, int x, int y, Uint8 col)
{
  Uint32 i;
  i = 3 * (x + (width * y));
  buf[i++] = pal[col].r;
  buf[i++] = pal[col].g;
  buf[i++] = pal[col].b;
}

void
endfire (void)
{
  free (buf1);
  free (buf2);
  free (rndbuf);
  free (rbf);
  free (ball);
  free (fire_buf);
}

void
rnd_frame (void)
{
  rnd_offs = random () % (6 * width * height);
}

void
flame_source (void)
{
  int i;
  long int i0;

  i0 = random () % (width * 26);
  for (i = 1; i < width * 3; i++)
    {
      buf1[(height * width) - i] = rndbuf[i + i0];
    }
}

void
view_bufer (SDL_Surface * scr, Uint8 * b)
{
  int i;
  Slock (scr);
  for (i = 0; i < height * width; i++)
    {
      put_pixel (scr, i % width, i / width, b[i]);
    }
  Sunlock (scr);
}

void
view_bufer_no_sdl (Uint8 * dst_buf, Uint8 * src_buf)
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
    rnd_frame ();
    flame_updated=1;
}
