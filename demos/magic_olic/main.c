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
#include "fire.h"
#include "fps.h"
#include "keys.h"
#include "shtepsel.h"
#include "rozetka.h"
#include "math.h"
#include "music.h" 

const int w_width=800;
const int w_height=600;
const int Nr=25;

const double x0=-4.5;
const double vx=0.01;
const double xm=4.5;
const double dscale=0.4;
const double dx=1.5;
const double dz=2;
const double ra=2;
const double cam_fr=0.5;
const double cam_a=0.5;
const double fire_r=0.7;
const double fire_z=-0.0;
const double fire_y=1.5;
const double fire_alpha=0.3;
typedef struct rzt
{
    double x,y,z,a,da,scale;
}  rozetka_t;

struct
    {
        double ax,ay,az,dax,day,daz;
    } shtepsel;

double z_dst=1;



char str[128],str_flag;
SDL_TimerID timer;
char timer_stoped=1;
SDL_Surface *screen;
void* tt_param;
rozetka_t *roz;
double cam_h=0;
double cam_f=0;

GLint shtepsel_id, rozetka_id;

Uint32 timer_tic(Uint32 interval, void *param);
extern GLint GenRozetkaList(void);


void set_perspective(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	gluPerspective(40,(double)(w_width)/(double)(w_height),1,50);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void init_opengl(void)
{
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
    if (NULL == (screen = SDL_SetVideoMode(w_width, w_height, 0, SDL_OPENGL)))
    {
        printf("Can't set OpenGL mode: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    } 
    glClearColor(0.0,0.0,0.0,0.0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
    SDL_WM_SetCaption("Magic Olik",NULL);
    glViewport(0,0,w_width,w_height);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
//    glEnable(GL_TEXTURE_2D);
//    glEnable(GL_LINE_SMOOTH);
//    glEnable(GL_POINT_SMOOTH);
    glShadeModel(GL_SMOOTH);
//    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glClearStencil(0);     
    glClearDepth(1.0f);
    glEnable(GL_RESCALE_NORMAL);
	set_perspective();
}


void init_textures(void)
{
    fire_init();
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

void init_light(void)
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
}

void set_material_1(void)
{
  //  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 200.0 };
    GLfloat mat_diffuse[] = { 0.9, 0.9, 0.9, 1.0 };
    GLfloat mat_emission[] = { 0.0, 0.0, 0.0 };

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);
//glShadeModel (GL_FLAT);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
}

void set_light()
{

    GLfloat light_position[] = { 1.0, 1.0,1.0, 0.0 };
    GLfloat light_position1[] = { -2.0, -0.5, 1.0, 0.0 };
    GLfloat light_diffuse[] = { 0.9,0.9, 0.9, 1.0 };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

}


void init_objects(void)
{
    int i;
    shtepsel_id=shtepsel_generate_list();
    rozetka_id=rozetka_generate_list();
    shtepsel.ax=0.0;
    shtepsel.ay=0.0;
    shtepsel.az=0.0;
    shtepsel.dax=0.0;
    shtepsel.day=0.5;
    shtepsel.daz=0.0;
    roz=calloc(Nr,sizeof(rozetka_t));
    for(i=0;i<Nr;i++)
    {
        roz[i].x=x0+dx*i;
        roz[i].y=0;
        roz[i].z=0;
        roz[i].a=random() % 360;
        roz[i].da=(((double)random()/RAND_MAX)-0.5)*ra;
        roz[i].scale=1-(((double)random()/RAND_MAX)*dscale);
        if(i>Nr/3)
        {
            roz[i].z=dz;
            roz[i].x=x0+dx*(i-Nr/3);
        }
        if(i>2*(Nr/3))
        {
            roz[i].z=dz*2;
            roz[i].x=x0+dx*(i-2*(Nr/3));
        }
    }
    
}

void rozetka_reinit(int i)
{
    roz[i].x=x0;
    roz[i].a=random() % 360;
    roz[i].da=(((double)random()/RAND_MAX)-0.5)*ra;
    roz[i].scale=1-(((double)random()/RAND_MAX)*dscale);
}

void move_camera(void)
{
    cam_f+=cam_fr;
    cam_h=cam_a*sin(cam_f*M_PI/180);
    
}

void move_shtepsel(void)
{
    shtepsel.ax+=shtepsel.dax; if(shtepsel.ax>360.0) shtepsel.ax-=360.0; if(shtepsel.ax<-360.0) shtepsel.ax+=360.0;
    shtepsel.ay+=shtepsel.day; if(shtepsel.ay>360.0) shtepsel.ay-=360.0; if(shtepsel.ay<-360.0) shtepsel.ay+=360.0;
    shtepsel.az+=shtepsel.daz; if(shtepsel.az>360.0) shtepsel.az-=360.0; if(shtepsel.az<-360.0) shtepsel.az+=360.0;
}


void init(void)
{
if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0)
    {
      printf ("Unable to init SDL: %s\n", SDL_GetError ());
      exit (1);
    }
    sound_init();
    play_music();
  init_opengl ();
  fire_init ();
  init_textures ();
  init_light();
    set_material_1();
    set_light();
  init_objects();
  init_fps ();
  init_timer();
  timer_stoped=0;
}


void draw_shtepsel(void)
{
    glPushMatrix();
    glScalef(0.125,0.125,0.125);
    glTranslatef(0,-5,0);
    glRotated(shtepsel.ax,1,0,0);
    glRotated(shtepsel.ay,0,1,0);
    glRotated(shtepsel.az,0,0,1);
    glCallList(shtepsel_id);
    glPopMatrix();
}

void draw_fire(void)
{
    glTranslated(0,fire_y,0);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    bind_fire_texture();
    glColor4d(1,1,1,fire_alpha);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0);
        glVertex3d(-fire_r,fire_r,fire_z);
        glTexCoord2f(1,0);
        glVertex3d(fire_r,fire_r,fire_z);
        glTexCoord2f(1,1);
        glVertex3d(fire_r,-fire_r,fire_z);
        glTexCoord2f(0,1);
        glVertex3d(-fire_r,-fire_r,fire_z);
        
    glEnd();
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void draw_rozetka(void)
{
    glScalef(0.25,0.25,0.25);
    glTranslatef(0,-1,0);
    glCallList(rozetka_id);
    
}

void draw_roz(void)
{
    int i;
    for(i=0;i<Nr;i++)
    {
        glPushMatrix();
        glTranslated(roz[i].x,roz[i].y,roz[i].z);
        glRotated(roz[i].a,0,1,0);
        glScaled(roz[i].scale,roz[i].scale,roz[i].scale);
        draw_rozetka();
        glPopMatrix();
    }
    
}



void draw_scene(void)
{
    glLoadIdentity();
    gluLookAt(0,cam_h,-3,  0,0,z_dst,  0,1,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 	// очистка буферов
    glTranslatef(0,0,z_dst);
    draw_shtepsel();

    glTranslated(0,-1.0,0);
//    draw_rozetka();
    draw_roz();
    draw_fire();
}


void render_frame(void)
{
    draw_scene();
    if (str_flag) {   SDL_WM_SetCaption(str,NULL); str_flag=0; }
    SDL_GL_SwapBuffers();
}

void end_all(void)
{
    free(roz);
}

void move_rozetka(void)
{
    int i;
    for(i=0;i<Nr;i++)
    {
        roz[i].x+=vx;
        if(roz[i].x>xm) rozetka_reinit(i);
        roz[i].a+=roz[i].da;
        if(roz[i].a>360) roz[i].a+=-360;
        if(roz[i].a<-360) roz[i].a+=360;
    }
}

Uint32 timer_tic(Uint32 interval, void *param)
{
    if (!timer_stoped)
    {
        move_shtepsel();
        move_rozetka();
        move_camera();
        fire_update();
    }
    return interval;
}

int main (int argc, char *argv[])
{
  init ();
  begin_listen_keys ();
  while (!(esc_pressed ()))
    {
      render_frame ();
      frame_count ();
      process_keys ();
    }
    timer_stoped=1;
  end_all ();
    return 0;
}
