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


#include <stdlib.h>
#include "SDL.h"
#include "SDL_opengl.h"
#include <GL/gl.h>
#include "textures.h"
#include "fps.h"
#include "keys.h"
#include "lostship.hsc.h"
#include "hsc.h"
#include "sound.h"

int w_width=1024;
int w_height=768;
const double ratio=1024.0/768.0;
double z_dst=1;

char str[128],str_flag;
SDL_TimerID timer;
char timer_stoped=1;
SDL_Surface *screen;
void* tt_param;

Uint32 timer_tic(Uint32 interval, void *param);

void set_perspective(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
//	gluPerspective(40,(double)(w_width)/(double)(w_height),1,50);
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


int resizeWindow( int width, int height )
{
    int x0,y0;
    /* Height / width ration */
//    GLfloat ratio;

    /* Protect against a divide by zero */
//    if ( height == 0 )
//    height = 1;
    w_width=width;
    w_height=height;
    if(width/ratio > height)
    {
        w_width=height*ratio;
        w_height=height;
        x0=(width-w_width)/2;
        y0=0;
    } else
    {
        w_width=width;
        w_height=width/ratio;
        x0=0;
        y0=(height-w_height)/2;
    }
//    ratio = ( GLfloat )width / ( GLfloat )height;
    screen = SDL_SetVideoMode(width, height, 0, SDL_OPENGL |SDL_RESIZABLE | SDL_GL_DOUBLEBUFFER );
    /* Setup our viewport. */
    glViewport( x0, y0, ( GLsizei )w_width, ( GLsizei )w_height );
    
    /* change to the projection matrix and set our viewing volume. */
    set_perspective();

    return 1;
}



void init_opengl(void)
{
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
    if (NULL == (screen = SDL_SetVideoMode(w_width, w_height, 0, SDL_OPENGL |SDL_RESIZABLE | SDL_GL_DOUBLEBUFFER )))
    {
        printf("Can't set OpenGL mode: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    } 
    glClearColor(0.0,0.0,0.0,0.0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
    SDL_WM_SetCaption("Meteor",NULL);
    glViewport(0,0,w_width,w_height);
//    glEnable(GL_DEPTH_TEST);
//    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glShadeModel(GL_SMOOTH);
    glClearStencil(0);     
    glClearDepth(1.0f);
	set_perspective();
    set_resize_callback(resizeWindow);
}

void init_textures(void)
{
    texture_init();
}

void init_timer(void)
{
    timer=SDL_AddTimer(10, timer_tic, tt_param);

}

void process_keys (void)
{
  char key;
  key = key_pressed ();
  if (key == 'h');
  if (key == 'r');
}





void init(void)
{
if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
      printf ("Unable to init SDL: %s\n", SDL_GetError ());
      exit (1);
    }

  init_opengl ();
  texture_init ();
  init_textures ();
  init_fps ();
  init_timer();
  sound_init();
  start_opl_emulation();
  hsc_play((uint8_t*)data,0);
}


void draw_scene(void)
{
    glLoadIdentity();

//    gluLookAt(0,0,-3,  0,0,z_dst,  0,1,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 	// очистка буферов
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//    update_texture();
//    bind_fire_texture();
     update_bkg_texture();

    glBegin(GL_QUADS);

        glTexCoord2f(0.0,1.0);
        glVertex2f(0.0,0.0);
        
        glTexCoord2f(1.0,1.0);
        glVertex2f(1.0,0.0);
        
        glTexCoord2f(1.0,0.0);
        glVertex2f(1.0,1.0);
        
        glTexCoord2f(0.0,0.0);
        glVertex2f(0.0,1.0);
    glEnd();
       meteor_update_texture();
       glEnable(GL_BLEND);
       glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

      glBegin(GL_QUADS);

        glTexCoord2f(0.0,1.0);
        glVertex2f(0.0,0.0);
        
        glTexCoord2f(1.0,1.0);
        glVertex2f(1.0,0.0);
        
        glTexCoord2f(1.0,0.0);
        glVertex2f(1.0,1.0);
        
        glTexCoord2f(0.0,0.0);
        glVertex2f(0.0,1.0);
    glEnd();
    
        spectr_texture_update();
        glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
      glBegin(GL_QUADS);

        glTexCoord2f(0.0,0.0);
        glVertex2f(0.0,0.0);
        
        glTexCoord2f(1.0,0.0);
        glVertex2f(1.0,0.0);
        
        glTexCoord2f(1.0,1.0);
        glVertex2f(1.0,0.15);
        
        glTexCoord2f(0.0,1.0);
        glVertex2f(0.0,0.15);
    glEnd();
    
    glDisable(GL_BLEND);

    glFlush();
    
}


void render_frame(void)
{
    draw_scene();
    if (str_flag) {   SDL_WM_SetCaption(str,NULL); str_flag=0; }
    SDL_GL_SwapBuffers();
}

void end_all(void)
{
    sound_end();
}

Uint32 timer_tic(Uint32 interval, void *param)
{
    if (!timer_stoped)
    {
        fire_update();
    }
    return interval;
}

int main (int argc, char *argv[])
{
  init ();
  begin_listen_keys ();
  timer_stoped=0;
  while (!(esc_pressed ()))
    {
      render_frame ();
      frame_count ();
      process_keys ();
    }

  end_all ();
    return 0;
}
