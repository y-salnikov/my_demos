/*     (c) Jarik (y.salnikov)
 * 
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
#include <math.h>
#include "SDL.h"
#include "SDL_opengl.h" 
#include <GL/gl.h>
#include <GL/glext.h>
//#include <GL/glut.h>
#include <unistd.h>
#include "fire.h"
#include "fps.h"
#include "keys.h"
#include "fire_constants.h"
#include "texture_gen.h"
#include "lighting.h"
#include "3dmath.h"
#include "sound.h"
#include "music.h"


//const GLfloat dxa=0.0,dya=0.0,dza=0.0, dz=0.002;
const GLfloat dxa=0.12,dya=0.15,dza=0.2, dz=0.002;
const double spark_size=0.04;
const int N_part=100;
const double part_size=0.01;
const double particles_dr=0.004;
const double particles_g=0.0002;
const double part_color_r=0.8;
const double part_color_g=0.8;
const double part_color_b=0.6;
const double part_color_d=0.2;
const int w_width=800;
const int w_height=600;
const double fps0=200;
const int blur_texture_width=512;
const int blur_texture_height=512;
const int combined_texture_width=128;
const int combined_texture_height=128;
const double fb_blur_alpha=0.980;					// шлейф за сферами
const double fb_blur_alpha_r=0.001;
const double rnd_alpha=0.3;						// прозрачность "снега"
const double blur_k_alpha=0.8;
const double blur_k_r=0.03;
const double blur_freq=0.005;
const double camera_height_max=1.5;
double camera_height=0;
const double camera_freq=0.005;
double camera_ang=-M_PI/2;

double blur_alpha=0;
double blur_r=0;
char blur_enabled=0;
char blur_func=0;
char rtt_enabled=0;
char fire_updated=0;
Uint32 state,tik;
SDL_Surface *screen;
Uint8 *p,*fbuf,*rbuf;
Uint8 *ftexture;

SDL_Surface *sdl_texture;
GLfloat x_ang=0.01,y_ang=0.0003,z_ang=0, z_dst=-1.47;
GLuint texture_id,texture2_id,blur_texture,fb_blur_texture,combined_texture;
GLuint fbo_id[2], fbo_texture_id[6], fbo_depth_id[2], fbo_stencil_id[2];

GLUquadric* q;
char str[128];
char str_flag;
char flame_flag2=0;
double floor_height=1;
double blur_state=0;
void* tt_param;
char fb_blur=0;
Uint32 timer_tic(Uint32 interval, void *param);

void render_to_texture(void);
void reload_textures(void);
 void init_particles(void);
 void draw_time(void);

SDL_TimerID rotate_timer;
char timer_stoped=0;

GLuint current_blur_texture=0,cube;
GLuint current_fb_texture=2;

struct {
    double fx, fy, fz, dfx, dfy, dfz, r;
} sphere;

struct {
    double fx, fy, fz, dfx, dfy, dfz, r, rr;
} fireball;

typedef struct {
				double x;
				double y;
				double z;
				double dx,dy,dz;
				double	color_r,color_g,color_b;
				int time;
				} particle;        

lighting_type lighting1,lighting2;

particle *particles;

struct
{
	double x;
	double y;
	double z;	
} spark1,spark2;

// FBO extentions
static PFNGLISRENDERBUFFEREXTPROC    glIsRenderbufferEXT    = NULL;
static PFNGLBINDRENDERBUFFEREXTPROC    glBindRenderbufferEXT     = NULL;
static PFNGLDELETERENDERBUFFERSEXTPROC    glDeleteRenderbuffersEXT    = NULL;
static PFNGLGENRENDERBUFFERSEXTPROC    glGenRenderbuffersEXT    = NULL;
static PFNGLRENDERBUFFERSTORAGEEXTPROC    glRenderbufferStorageEXT    = NULL;
static PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC    glGetRenderbufferParameterivEXT    = NULL;
static PFNGLISFRAMEBUFFEREXTPROC    glIsFramebufferEXT    = NULL;
static PFNGLBINDFRAMEBUFFEREXTPROC   glBindFramebufferEXT    = NULL;
static PFNGLDELETEFRAMEBUFFERSEXTPROC    glDeleteFramebuffersEXT    = NULL;
static PFNGLGENFRAMEBUFFERSEXTPROC    glGenFramebuffersEXT    = NULL;
static PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC    glCheckFramebufferStatusEXT    = NULL;
static PFNGLFRAMEBUFFERTEXTURE1DEXTPROC    glFramebufferTexture1DEXT    = NULL;
static PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    glFramebufferTexture2DEXT    = NULL;
static PFNGLFRAMEBUFFERTEXTURE3DEXTPROC    glFramebufferTexture3DEXT    = NULL;
static PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC    glFramebufferRenderbufferEXT    = NULL;
static PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC    glGetFramebufferAttachmentParameterivEXT    = NULL;
static PFNGLGENERATEMIPMAPEXTPROC   glGenerateMipmapEXT     = NULL;



//------------------------------------------------------------------------------------------------------------

char IsExtensionSupported( char* szTargetExtension )
{
	const unsigned char *pszExtensions = NULL;
	const unsigned char *pszStart;
	unsigned char *pszWhere, *pszTerminator;

	// Extension names should not have spaces
	pszWhere = (unsigned char *) strchr( szTargetExtension, ' ' );
	if( pszWhere || *szTargetExtension == '\0' )
		return 0;

	// Get Extensions String
	pszExtensions = glGetString( GL_EXTENSIONS );

	// Search The Extensions String For An Exact Copy
	pszStart = pszExtensions;
	for(;;)
	{
		pszWhere = (unsigned char *) strstr( (const char *) pszStart, szTargetExtension );
		if( !pszWhere )
			break;
		pszTerminator = pszWhere + strlen( szTargetExtension );
		if( pszWhere == pszStart || *( pszWhere - 1 ) == ' ' )
			if( *pszTerminator == ' ' || *pszTerminator == '\0' )
				return 1;
		pszStart = pszTerminator;
	}
	return 0;
}

char checkfbo(void)
{
	/* lots of booring errorchecking stuff, this will display an error code and then kill the app if it failed*/
	switch(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)) {                                          
          case GL_FRAMEBUFFER_COMPLETE_EXT: 
			  	return 0;
			break; 


			 
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
				printf("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT\n");
				 return 1;
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
				printf("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT\n");
				return 1;
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
				printf("GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT\n");
				 return 1;
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
				printf("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT\n");
				return 1;
				 break;
			case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
				printf("GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT\n");
				return 1;
				 break;
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
				printf("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT\n");
				return 1;
				 break;
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
				printf("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT\n");
				return 1;
				 break;


			case GL_FRAMEBUFFER_UNSUPPORTED_EXT: 
					printf("GL_FRAMEBUFFER_UNSUPPORTED_EXT\n");
					return 1;
		
		     break;   
                                              
          default:                                               
            return 1;                                       
	}
}



void init_objects(void)
{
	cube=glGenLists(2);
	q=gluNewQuadric();
	glNewList(cube,GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, texture_id);
//		bind_bkg_texture2();
//		bind_marble_texture2();
//		bind_time_texture();
//		glBindTexture(GL_TEXTURE_2D, fbo_texture_id);
	    glBegin(GL_QUADS);
	         glColor4ub(0, 0, 127,127);
                glTexCoord2f(1.0,0.0);
			glVertex3f(-0.5, 0.5, -0.5);
	        glTexCoord2f(0.0,0.0);
                glVertex3f(0.5, 0.5, -0.5);
			glTexCoord2f(0.0,1.0);
                glVertex3f(0.5, -0.5, -0.5);
			glTexCoord2f(1.0,1.0);
                glVertex3f(-0.5, -0.5, -0.5);
		glEnd();
		bind_green_fire_texture();
		glBegin(GL_QUADS);
                glColor4ub(127,12 , 0,127); //2
			glTexCoord2f(1.0,0.0);
                glVertex3f(0.5, -0.5, 0.5);
			glTexCoord2f(0.0,0.0);
                glVertex3f(-0.5, -0.5, 0.5);
			glTexCoord2f(0.0,1.0);
                glVertex3f(-0.5, 0.5, 0.5);
			glTexCoord2f(1.0,1.0);
                glVertex3f(0.5, 0.5, 0.5);
        glEnd();
        
		bind_blue_fire_texture();
		glBegin(GL_QUADS);
			glColor3ub(0, 255, 0); //3
			glTexCoord2f(1.0,0.0);		
                glVertex3f(-0.5, 0.5, 0.5);
			glTexCoord2f(0.0,0.0);		
                glVertex3f(-0.5, 0.5, -0.5);
			glTexCoord2f(0.0,1.0);		
                glVertex3f(-0.5, -0.5, -0.5);
			glTexCoord2f(1.0,1.0);		
                glVertex3f(-0.5, -0.5, 0.5);
        glEnd();
        glBindTexture(GL_TEXTURE_2D,texture_id);
        glBegin(GL_QUADS);
        	glColor3ub(255, 255, 0); //4
        
			glTexCoord2f(1.0,0.0);		
                glVertex3f(0.5, -0.5, -0.5);
			glTexCoord2f(0.0,0.0);		
                glVertex3f(-0.5, -0.5, -0.5);
			glTexCoord2f(0.0,1.0);		
                glVertex3f(-0.5, -0.5, 0.5);
			glTexCoord2f(1.0,1.0);		
                glVertex3f(0.5, -0.5, 0.5);
                glColor3ub(75, 75, 75); //5
			glTexCoord2f(1.0,0.0);		
                glVertex3f(-0.5, 0.5, 0.5);
			glTexCoord2f(0.0,0.0);		
                glVertex3f(0.5, 0.5, 0.5);
			glTexCoord2f(0.0,1.0);		
                glVertex3f(0.5, 0.5, -0.5);
			glTexCoord2f(1.0,1.0);		
                glVertex3f(-0.5, 0.5, -0.5);
		glEnd();

	
	//	glBindTexture(GL_TEXTURE_2D, texture2_id);
    //	glBindTexture(GL_TEXTURE_2D, combined_texture);
		glBindTexture(GL_TEXTURE_2D, fbo_texture_id[4]);
    
		glBegin(GL_QUADS);	
                glColor3ub(95, 95, 95); //6
			glTexCoord2f(0.0,0.0);		
                glVertex3f(0.5, 0.5, -0.5);
			glTexCoord2f(1.0,0.0);		
                glVertex3f(0.5, 0.5, 0.5);
			glTexCoord2f(1.0,1.0);		
                glVertex3f(0.5, -0.5, 0.5);
			glTexCoord2f(0.0,1.0);		
                glVertex3f(0.5, -0.5, -0.5);
    	glEnd();

/*		glBindTexture(GL_TEXTURE_2D, fb_blur_texture);
    	glColor4d(1.0, 1.0, 1.0, fb_blur_alpha);
    	glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,  GL_ONE);
		glBegin(GL_QUADS);	
                glColor3ub(95, 95, 95); //6
			glTexCoord2f(1.0,0.0);		
                glVertex3f(0.5, 0.5, -0.5);
			glTexCoord2f(0.0,0.0);		
                glVertex3f(0.5, 0.5, 0.5);
			glTexCoord2f(0.0,1.0);		
                glVertex3f(0.5, -0.5, 0.5);
			glTexCoord2f(1.0,1.0);		
                glVertex3f(0.5, -0.5, -0.5);
    	glEnd();

		glBindTexture(GL_TEXTURE_2D, texture2_id);
		glColor4d(1.0, 1.0, 1.0, rnd_alpha);
    	glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,  GL_ONE);
		glBegin(GL_QUADS);	
        //      glColor3ub(95, 95, 95); //6
			glTexCoord2f(1.0,0.0);		
                glVertex3f(0.5, 0.5, -0.5);
			glTexCoord2f(0.0,0.0);		
                glVertex3f(0.5, 0.5, 0.5);
			glTexCoord2f(0.0,1.0);		
                glVertex3f(0.5, -0.5, 0.5);
			glTexCoord2f(1.0,1.0);		
                glVertex3f(0.5, -0.5, -0.5);
    	glEnd();
	glDisable(GL_BLEND);
	*/
	glEndList();
	init_particles();
}


void init_sphere(void)
{
    sphere.dfx = 0.1;
    sphere.dfy = 0.23;
    sphere.dfz = 0.143;
    sphere.fx = 0.0;
    sphere.fy = 0.0;
    sphere.fz = 0.0;
    sphere.r = 45.0;
}
void init_fireball(void)
{
    fireball.dfx = 0.0;
    fireball.dfy = 0.8;
    fireball.dfz = 0.0;
    fireball.fx = 0.0;
    fireball.fy = 0.0;
    fireball.fz = 0.0;
    fireball.r= 0.15;
    fireball.rr=1.5;

}

void draw_particle(int N)
{
	glPushMatrix();
	glTranslated(0.0,0.0,z_dst);
//	glTranslated(particles[N].x,particles[N].y,particles[N].z);
	glDisable(GL_TEXTURE_2D);
	glColor3d(particles[N].color_r,particles[N].color_g,particles[N].color_b);
	glBegin(GL_TRIANGLES);
		glVertex3d(particles[N].x,particles[N].y,particles[N].z);
		glVertex3d(particles[N].x,particles[N].y-part_size,particles[N].z);
		glVertex3d(particles[N].x-part_size,particles[N].y-part_size,particles[N].z);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glPopMatrix();
}



void refresh_particle(int N)
{
	int spark_n;

	spark_n=(N % 2);
	if (spark_n==0)
	{
		particles[N].x=spark1.x;
		particles[N].y=spark1.y;
		particles[N].z=spark1.z;
	}
	if (spark_n==1)
	{
		particles[N].x=spark2.x;
		particles[N].y=spark2.y;
		particles[N].z=spark2.z;
	}
	particles[N].dx=(0.5-((double)random()/RAND_MAX))*particles_dr;
	particles[N].dz=(0.5-((double)random()/RAND_MAX))*particles_dr;
	particles[N].dy=(0.5-((double)random()/RAND_MAX))*particles_dr;
	particles[N].time=0;
	particles[N].color_r=part_color_r+((2.0*(double)random()/RAND_MAX)-1)*part_color_d;
	particles[N].color_g=part_color_g+((2.0*(double)random()/RAND_MAX)-1)*part_color_d;
	particles[N].color_b=part_color_b+((2.0*(double)random()/RAND_MAX)-1)*part_color_d;
}

void init_particles(void)
{
	int i;
	particles=calloc(N_part,sizeof(particle));
	for(i=0;i<N_part;i++)
	{
		particles[i].time=random() % (N_part);
//		refresh_particle(i);
	}
}

void update_particles(void)
{
	int i;
	for(i=0;i<N_part;i++)
	{
		if(particles[i].time>1) { particles[i].time--; return; }
		if(particles[i].time==1) refresh_particle(i);

		particles[i].x+=particles[i].dx;
		particles[i].z+=particles[i].dz;
		particles[i].y+=particles[i].dy;
		particles[i].dy+=-particles_g;

		if(particles[i].y<(-floor_height)) refresh_particle(i);

//		draw_particle(i);
		
	}	
}

void draw_particles(void)
{
	int i;
	for(i=0;i<N_part;i++) draw_particle(i);
}

void end_particles(void)
{
	free(particles);
}

void init_lighting()
{
	lighting1.N=20;
	lighting1.length=fireball.rr*2;
	lighting1.r=0.03;
	lighting1.col_r=0.8;
	lighting1.col_g=0.8;
	lighting1.col_b=1.0;
	lighting1.col_a=1.0;
	lighting_init(&lighting1);
	lighting_update(&lighting1);

	lighting2.N=20;
	lighting2.length=fireball.rr*2;
	lighting2.r=0.04;
	lighting2.col_r=0.9;
	lighting2.col_g=0.5;
	lighting2.col_b=0.5;
	lighting2.col_a=1.0;
	lighting_init(&lighting2);
	lighting_update(&lighting2);

	spark1.x=0;
	spark1.y=0;
	spark1.z=0;
	spark2.x=0;
	spark2.y=0;
	spark2.z=0;
}

void rotate_sphere(void)
{
    sphere.fx += sphere.dfx;
    sphere.fy += sphere.dfy;
    sphere.fz += sphere.dfz;

    if (sphere.fx > 360)
	sphere.fx -= 360;
    if (sphere.fy > 360)
	sphere.fy -= 360;
    if (sphere.fz > 360)
	sphere.fz -= 360;
}

void rotate_fireball(void)
{
    fireball.fx += fireball.dfx;
    fireball.fy += fireball.dfy;
    fireball.fz += fireball.dfz;

    if (fireball.fx > 360)
	fireball.fx -= 360;
    if (fireball.fy > 360)
	fireball.fy -= 360;
    if (fireball.fz > 360)
	fireball.fz -= 360;
}


void set_perspective(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	gluPerspective(40,(double)(w_width)/(double)(w_height),1,50);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void init_fbo(void)
{
	int i;
	if (!IsExtensionSupported("GL_EXT_framebuffer_object"))
	{
		printf("Sorry, but you can't use FBOs, try upgrading you drivers or hardware\n");
	}
	else
	{
	// create the needed ext bundings for fbos
	glIsRenderbufferEXT    = (PFNGLISRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glIsRenderbufferEXT");
	glBindRenderbufferEXT    = (PFNGLBINDRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glBindRenderbufferEXT");
	glDeleteRenderbuffersEXT    = (PFNGLDELETERENDERBUFFERSEXTPROC)SDL_GL_GetProcAddress("glDeleteRenderbuffersEXT");
	glGenRenderbuffersEXT    = (PFNGLGENRENDERBUFFERSEXTPROC)SDL_GL_GetProcAddress("glGenRenderbuffersEXT");
	glRenderbufferStorageEXT    = (PFNGLRENDERBUFFERSTORAGEEXTPROC)SDL_GL_GetProcAddress("glRenderbufferStorageEXT");
	glGetRenderbufferParameterivEXT    = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)SDL_GL_GetProcAddress("glGetRenderbufferParameterivEXT");
	glIsFramebufferEXT    = (PFNGLISFRAMEBUFFEREXTPROC)SDL_GL_GetProcAddress("glIsFramebufferEXT");
	glBindFramebufferEXT    = (PFNGLBINDFRAMEBUFFEREXTPROC)SDL_GL_GetProcAddress("glBindFramebufferEXT");
	glDeleteFramebuffersEXT    = (PFNGLDELETEFRAMEBUFFERSEXTPROC)SDL_GL_GetProcAddress("glDeleteFramebuffersEXT");
	glGenFramebuffersEXT    = (PFNGLGENFRAMEBUFFERSEXTPROC)SDL_GL_GetProcAddress("glGenFramebuffersEXT");
	glCheckFramebufferStatusEXT    = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatusEXT");
	glFramebufferTexture1DEXT    = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)SDL_GL_GetProcAddress("glFramebufferTexture1DEXT");
	glFramebufferTexture2DEXT    = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)SDL_GL_GetProcAddress("glFramebufferTexture2DEXT");
	glFramebufferTexture3DEXT    = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)SDL_GL_GetProcAddress("glFramebufferTexture3DEXT");
	glFramebufferRenderbufferEXT    = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glFramebufferRenderbufferEXT");
	glGetFramebufferAttachmentParameterivEXT    = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)SDL_GL_GetProcAddress("glGetFramebufferAttachmentParameterivEXT");
	glGenerateMipmapEXT     = (PFNGLGENERATEMIPMAPEXTPROC)SDL_GL_GetProcAddress("glGenerateMipmapEXT");
	checkfbo();

	
		
	glGenFramebuffersEXT(1, &fbo_id[0]);													// create a new framebuffer
	glGenFramebuffersEXT(1, &fbo_id[1]);													// create a new framebuffer
	glGenRenderbuffersEXT(1, &fbo_depth_id[0]);											// And finaly a new depthbuffer
	glGenRenderbuffersEXT(1, &fbo_depth_id[1]);											// And finaly a new depthbuffer
	glGenRenderbuffersEXT(1, &fbo_stencil_id[0]);											// And finaly a new depthbuffer
	glGenRenderbuffersEXT(1, &fbo_stencil_id[1]);											// And finaly a new depthbuffer
	
	for(i=0;i<6;i++)
	{
		glGenTextures(1, &fbo_texture_id[i]);													// and a new texture used as a color buffer
		glBindTexture(GL_TEXTURE_2D, fbo_texture_id[i]);										// Bind the colorbuffer texture	
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);				// make it linear filterd
		if(i<4) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, blur_texture_width, blur_texture_height, 0,GL_RGB, GL_INT, NULL);	// Create the texture data
		else glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, combined_texture_width, combined_texture_height, 0,GL_RGB, GL_INT, NULL);	// Create the texture data
	}
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_id[0]);									// switch to the new framebuffer
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D, fbo_texture_id[0], 0); // attach it to the framebuffer
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbo_depth_id[0]);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, 0x84F9, blur_texture_width, blur_texture_height);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbo_depth_id[0]);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbo_depth_id[0]);



	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_id[1]);									// switch to the new framebuffer
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D, fbo_texture_id[4], 0); // attach it to the framebuffer
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbo_depth_id[1]);							// bind the depth renderbuffer
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,GL_DEPTH_COMPONENT24,combined_texture_width, combined_texture_height);	// get the data space for it
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_RENDERBUFFER_EXT, fbo_depth_id[1]); // bind it to the renderbuffer
//	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbo_stencil_id[1]);
//	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,GL_STENCIL_INDEX,combined_texture_width, combined_texture_height);	// get the data space for it
//	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_STENCIL_ATTACHMENT_EXT,GL_RENDERBUFFER_EXT, fbo_stencil_id[1]);
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);									// Swithch back to normal framebuffer rendering
	checkfbo();																		// make shure it all went ok
	}
}



void init_opengl(void)
{
//  SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   5);
//  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
//  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  5);
//int ggg;
SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
  if (NULL == (screen = SDL_SetVideoMode(w_width, w_height, 0, SDL_OPENGL)))
  {
    printf("Can't set OpenGL mode: %s\n", SDL_GetError());
    SDL_Quit();
    exit(1);
  } 
  glClearColor(0.0,0.0,0.0,0.0);
  
//  SDL_GL_GetAttribute( SDL_GL_STENCIL_SIZE, &ggg);
//  printf("Stencil size = %d\n",ggg);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
  SDL_WM_SetCaption("Firecube",NULL);
  
//  glShadeModel(GL_FLAT);
  glViewport(0,0,w_width,w_height);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_TEXTURE_2D);
//  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POINT_SMOOTH);
  glShadeModel(GL_SMOOTH);
  
  glClearStencil(0);     // Очистка буфера шаблона 0
   glClearDepth(1.0f);
	set_perspective();
	init_fbo();
}

void init_texture(GLuint *texture)
{
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
  	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void init_textures(void)
{
	char *void_texture;
	int i;
	

	create_bkg_texture2();
	create_blue_fire_texture(width,height-5);
	create_green_fire_texture(width,height-5);
	create_marble_texture2();
	create_spark_texture2();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	init_texture(&texture_id);
	init_texture(&texture2_id);
	init_texture(&blur_texture);
	init_texture(&fb_blur_texture);
	init_texture(&combined_texture);
	fbuf=bufer();
	rbuf=bufer2();
	glBindTexture(GL_TEXTURE_2D, texture2_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height-5, 0, GL_RGB,GL_UNSIGNED_BYTE, rbuf);
	void_texture=malloc(blur_texture_width*blur_texture_height*4);
	for(i=0;i<(blur_texture_width*blur_texture_height);i++)
	{
		void_texture[4*i+0]=0; //r
		void_texture[4*i+1]=0; //g
		void_texture[4*i+2]=0; //b
		void_texture[4*i+3]=255; //a
	}
	glBindTexture(GL_TEXTURE_2D, fb_blur_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, blur_texture_width, blur_texture_height, 0, GL_RGBA,GL_UNSIGNED_BYTE, void_texture);
	free(void_texture);
}

void init(void)
{
//	int tmp=0;
//	char *s="vvv";
	
 if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER ) < 0 )
 {
  printf("Unable to init SDL: %s\n", SDL_GetError());
  exit(1);
 }
 
//glutInit(&tmp,&s);
state=0;
tik=0;
init_opengl();
draw_time();
init_sphere();
init_fireball();
fire_init();
init_lighting();
init_textures();
init_objects();
init_fps();
sound_init();
reload_textures();

rotate_timer=SDL_AddTimer(10, timer_tic, tt_param);
play_music();
}

void move_camera(void)
{
	camera_height=(camera_height_max/2)*(1+sin(camera_ang));
	camera_ang+=camera_freq;
	if(camera_ang>2.0*M_PI) camera_ang=0;
}

void bluring(void)
{
	blur_state+=blur_freq;
	if(blur_state>(2.0*M_PI))
	{
		blur_state-=2.0*M_PI;
		blur_func=1-blur_func;
	}
	blur_alpha=sin(blur_state);
	if(blur_alpha<0.5) blur_alpha=0;
	blur_r=sin(blur_state);
	if(blur_r<0.5)blur_r=0;
	blur_alpha=2*(blur_alpha-0.5)*blur_k_alpha;;
	blur_r=2*(blur_r-0.5)*blur_k_r;
	
}

void reload_textures(void)
{
		if(fire_updated)
		{
		//fire_update();
	    ftexture=bufer3();
	    view_bufer_no_sdl(ftexture,fbuf);
		rbuf=bufer2();
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height-5, 0, GL_RGB,GL_UNSIGNED_BYTE, ftexture);
		convert_r_to_b(ftexture);
		glBindTexture(GL_TEXTURE_2D, texture2_id);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height-5, 0, GL_RGB,GL_UNSIGNED_BYTE, rbuf);
		fire_updated=0;
	}
}


Uint32 timer_tic(Uint32 interval, void *param)
{
	if (timer_stoped==0)
	{
		if(state==0)
			{
   			tik++;
   			if(tik>800) state=1;
			}	

	
		if (state==1)
			{
			z_dst+=dz;
			if(z_dst>1)
				{
					state=2;
					z_dst=1;
				}
			}
		if  (state==2)
			{
				x_ang += dxa;
        		y_ang += dya;
        		z_ang += dza;
		        if(x_ang>360) x_ang=x_ang-360;
    		    if(y_ang>360) y_ang=y_ang-360;
        		if(z_ang>360) z_ang=z_ang-360;
        		if(x_ang<-360) x_ang=x_ang+360;
        		if(y_ang<-360) y_ang=y_ang+360;
        		if(z_ang<-360) z_ang=z_ang+360;

    			move_camera();
				blur_enabled=1;
				bluring();
			}
		if(rtt_enabled)render_to_texture();
		rotate_sphere();
		rotate_fireball();
		lighting_update(&lighting1);
		lighting_update(&lighting2);
		fire_update(); fire_updated=1;
		update_particles();
//		reload_textures();

	}
//	play_music();
	return interval;
}






void cube_vertex_coord(double *x, double *y, double *z)
{
	double Xx,Xy,Xz,Yx,Yy,Yz,Zx,Zy,Zz;
	Xx=1; Xy=0; Xz=0;
	Yx=0; Yy=1; Yz=0;
	Zx=0; Zy=0; Zz=1;
	
	rotate_v(x_ang,x,y,z,Xx,Xy,Xz);
	rotate_v(x_ang,&Yx,&Yy,&Yz,Xx,Xy,Xz);
	rotate_v(x_ang,&Zx,&Zy,&Zz,Xx,Xy,Xz);

	rotate_v(y_ang,x,y,z,Yx,Yy,Yz);
	rotate_v(y_ang,&Xx,&Xy,&Xz,Yx,Yy,Yz);
	rotate_v(y_ang,&Zx,&Zy,&Zz,Yx,Yy,Yz);

	rotate_v(z_ang,x,y,z,Zx,Zy,Zz);
	rotate_v(z_ang,&Xx,&Xy,&Xz,Zx,Zy,Zz);
	rotate_v(z_ang,&Yx,&Yy,&Yz,Zx,Zy,Zz);
	
}

void fb_center_coord(char fb_n, double *x, double *y, double *z)
{
	double Xx,Xy,Xz,Yx,Yy,Yz,Zx,Zy,Zz,x0,y0,z0;
	Xx=1; Xy=0; Xz=0;
	Yx=0; Yy=1; Yz=0;
	Zx=0; Zy=0; Zz=1;

	x0=fireball.rr; y0=0; z0=0;
	if (fb_n) x0=-fireball.rr;
	
	rotate_v(fireball.fy,&x0,&y0,&z0,Yx,Yy,Yz);
	rotate_v(fireball.fy,&Xx,&Xy,&Xz,Yx,Yy,Yz);
	rotate_v(fireball.fy,&Zx,&Zy,&Zz,Yx,Yy,Yz);

	rotate_v(fireball.fx,&x0,&y0,&z0,Xx,Xy,Xz);
	rotate_v(fireball.fx,&Yx,&Yy,&Yz,Xx,Xy,Xz);
	rotate_v(fireball.fx,&Zx,&Zy,&Zz,Xx,Xy,Xz);

	rotate_v(fireball.fz,&x0,&y0,&z0,Zx,Zy,Zz);
	rotate_v(fireball.fz,&Yx,&Yy,&Yz,Zx,Zy,Zz);
	rotate_v(fireball.fz,&Xx,&Xy,&Xz,Zx,Zy,Zz);

	*x=x0; *y=y0; *z=z0;
}

void render_cube(void)
{
// 	double tx,ty,tz;
	
	glPushMatrix();
	glTranslatef(0,0,z_dst);
        glRotatef(x_ang,1,0,0);
        glRotatef(y_ang,0,1,0);
        glRotatef(z_ang,0,0,1);
		glCallList(cube);
	glPopMatrix();
/*
	glPushMatrix();
	glTranslatef(0,0,z_dst);
		tx=-0.7; ty=0.7; tz=-0.7;
		cube_vertex_coord(&tx,&ty,&tz);
		glTranslated(tx,ty,tz);
		glutWireSphere(0.1,5,5);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(0,0,z_dst);
		glRotatef(x_ang,1,0,0);
        glRotatef(y_ang,0,1,0);
        glRotatef(z_ang,0,0,1);
		glBegin(GL_LINES);
			glVertex3d(0,0,0);
			glVertex3d(-0.7,0.7,-0.7);
		glEnd();
	glPopMatrix(); */
}


void render_sphere(void)
{
	glPushMatrix();
//glDisable(GL_TEXTURE_2D);
//q=gluNewQuadric();
 	glTranslated(0.0,0.0,z_dst);
    glRotatef(sphere.fy,0.0,1.0,0.0);
    glRotatef(sphere.fx,1.0,0.0,0.0);
    glRotatef(sphere.fz,0.0,0.0,1.0);
    gluQuadricTexture(q,GL_TRUE);
    bind_bkg_texture2();
//	bind_spark_texture();
	glColor4d(1,1,1,1);
	gluQuadricDrawStyle(q,GLU_FILL);
    gluSphere(q,sphere.r,10,10);
 
//glEnable(GL_TEXTURE_2D);
	glPopMatrix();
}

void render_fireball(void)
{
	glPushMatrix();
	glColor3d(1,1,1);
	glTranslatef(0,0,z_dst);
//glDisable(GL_TEXTURE_2D);
 
    glRotatef(fireball.fy,0.0,1.0,0.0);
    glRotatef(fireball.fx,1.0,0.0,0.0);
    glRotatef(fireball.fz,0.0,0.0,1.0);
    glTranslatef(fireball.rr,0,0);
//	glScaled(-1,1,1);
//	lighting_draw(&lighting1);
//	glScaled(-1,1,1);
    glRotatef(90,1.0,0.0,0.0);
    gluQuadricTexture(q,GL_TRUE);
    glBindTexture(GL_TEXTURE_2D, texture_id);
	gluQuadricDrawStyle(q,GLU_FILL);
    gluSphere(q,fireball.r,8,8);
 	glPopMatrix();
	glPushMatrix();
	glTranslatef(0,0,z_dst);

// q=gluNewQuadric();
    glRotatef(fireball.fy,0.0,1.0,0.0);
    glRotatef(fireball.fx,1.0,0.0,0.0);
    glRotatef(fireball.fz,0.0,0.0,1.0);
    glTranslatef(-fireball.rr,0,0);
    glRotatef(-90,1.0,0.0,0.0);
    gluQuadricTexture(q,GL_TRUE);
//    glBindTexture(GL_TEXTURE_2D, texture2_id);
	bind_blue_fire_texture();
	gluQuadricDrawStyle(q,GLU_FILL);
    gluSphere(q,fireball.r,8,8);
 	glPopMatrix();
//glEnable(GL_TEXTURE_2D); 	 	
}

void render_floor()
{
	glPushMatrix();
	glTranslatef(0,0,z_dst);
//	glEnable( GL_BLEND );
//    glDisable( GL_DEPTH_TEST );
	bind_marble_texture2();
	glBegin(GL_QUADS);
	glTexCoord2f(1.0,0.0);		
    glVertex3f(1.5, -floor_height, -1);
	glTexCoord2f(0.0,0.0);		
    glVertex3f(-1.5, -floor_height, -1);
	glTexCoord2f(0.0,1.0);		
    glVertex3f(-1.5, -floor_height, 1.5);
	glTexCoord2f(1.0,1.0);		
    glVertex3f(1.5, -floor_height, 1.5);
    glEnd();

/*    bind_blue_fire_texture();                 //смешивание текстур
    glColor4f(1.0f, 1.0f, 1.0f, 0.4f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_ALPHA,  GL_ONE);

	glBegin(GL_QUADS);
	glTexCoord2f(1.0,0.0);		
    glVertex3f(1, -1, -1);
	glTexCoord2f(0.0,0.0);		
    glVertex3f(-1, -1, -1);
	glTexCoord2f(0.0,1.0);		
    glVertex3f(-1, -1, 1);
	glTexCoord2f(1.0,1.0);		
    glVertex3f(1, -1, 1);
    glEnd();
//    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_BLEND);
    

//    glEnable( GL_DEPTH_TEST );
*/
	glPopMatrix();
}


void draw_spark(double x, double y, double z)
{
	glPushMatrix();
		glTranslated(0,0,z_dst);
		glTranslated(x,y,z);
		glEnable(GL_BLEND);
//		glEnable(GL_ALPHA_TEST);
//		glAlphaFunc(GL_LESS,0.8);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
    	glEnable(GL_TEXTURE_2D);
		bind_spark_texture2();
//		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
		glBlendFunc(GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA);
		glBegin(GL_QUADS);	
			glTexCoord2f(0.0,0.0);		
            glVertex3f(spark_size,spark_size,0);
			glTexCoord2f(1.0,0.0);		
            glVertex3f(-spark_size,spark_size,0);
			glTexCoord2f(1.0,1.0);		
            glVertex3f(-spark_size,-spark_size,0);
			glTexCoord2f(0.0,1.0);		
            glVertex3f(spark_size,-spark_size,0);
		glEnd();
		glDisable(GL_BLEND);
//		glDisable(GL_ALPHA_TEST);
	glPopMatrix();
}

void render_sparks(void)
{
	draw_spark(spark1.x,spark1.y,spark1.z);
	draw_spark(spark2.x,spark2.y,spark2.z);

}

void render_lighting(void)
{
	struct vector
	{
		double x,y,z;
	} cube_vertex[8], fb1,fb2;
	int n,n1,n2;
	double min1,min2,tmp;
	double dx,dy,dz;
	glPushMatrix();

	n=0;
	cube_vertex[n].x=-0.5; cube_vertex[n].y=-0.5; cube_vertex[n++].z=-0.5;
	cube_vertex[n].x=-0.5; cube_vertex[n].y=0.5; cube_vertex[n++].z=-0.5;
	cube_vertex[n].x=0.5; cube_vertex[n].y=0.5; cube_vertex[n++].z=-0.5;
	cube_vertex[n].x=0.5; cube_vertex[n].y=-0.5; cube_vertex[n++].z=-0.5;
	cube_vertex[n].x=-0.5; cube_vertex[n].y=-0.5; cube_vertex[n++].z=0.5;
	cube_vertex[n].x=-0.5; cube_vertex[n].y=0.5; cube_vertex[n++].z=0.5;
	cube_vertex[n].x=0.5; cube_vertex[n].y=0.5; cube_vertex[n++].z=0.5;
	cube_vertex[n].x=0.5; cube_vertex[n].y=-0.5; cube_vertex[n++].z=0.5;

	fb_center_coord(0,&fb1.x,&fb1.y,&fb1.z);
	fb_center_coord(1,&fb2.x,&fb2.y,&fb2.z);
	
	min1=min2=999.9; n1=n2=0;
	for(n=0;n<8;n++)
	{
		cube_vertex_coord(&cube_vertex[n].x,&cube_vertex[n].y,&cube_vertex[n].z);
		tmp=vector_length(fb1.x,fb1.y,fb1.z,cube_vertex[n].x,cube_vertex[n].y,cube_vertex[n].z);
		if(tmp<min1)
		{
			min1=tmp;
			n1=n;
		}
		tmp=vector_length(fb2.x,fb2.y,fb2.z,cube_vertex[n].x,cube_vertex[n].y,cube_vertex[n].z);
		if(tmp<min2)
		{
			min2=tmp;
			n2=n;
		}
	}

	dx=cube_vertex[n1].x-fb1.x;
	dy=cube_vertex[n1].y-fb1.y;
	dz=cube_vertex[n1].z-fb1.z;
	glTranslatef(0,0,z_dst);
	spark1.x=cube_vertex[n1].x;
	spark1.y=cube_vertex[n1].y;
	spark1.z=cube_vertex[n1].z;
	spark2.x=cube_vertex[n2].x;
	spark2.y=cube_vertex[n2].y;
	spark2.z=cube_vertex[n2].z;
	
	glTranslated(fb1.x,fb1.y,fb1.z);
	lighting_draw_to(&lighting1,dx,dy,dz);
	glPopMatrix();
	glPushMatrix();
	dx=cube_vertex[n2].x-fb2.x;
	dy=cube_vertex[n2].y-fb2.y;
	dz=cube_vertex[n2].z-fb2.z;
	glTranslatef(0,0,z_dst);
	glTranslated(fb2.x,fb2.y,fb2.z);
	lighting_draw_to(&lighting2,dx,dy,dz);

	glPopMatrix();
	draw_particles();

}

void mirror(void)
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 	// очистка буферов
	glColorMask(0,0,0,0);															// запрет рисования на "экране"
    glEnable(GL_STENCIL_TEST);														// вкл. стенсильный буфер
    glStencilFunc(GL_ALWAYS, 1, 1);												// заполняем 1-ми там где рисуем 
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);                                     // 
    glDisable(GL_DEPTH_TEST);                                                      // пох на глубину
    render_floor();																    // рисуем отражающую плоскость
    glEnable(GL_DEPTH_TEST);                                                        // включаем глубину, рисование, и фиксируем стенсильный буфер
    glColorMask(1,1,1,0);
    glStencilFunc(GL_EQUAL, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); 
    // сюда можно добавить clip plane
    glPushMatrix();
	glScalef(1.0f, -1.0f, 1.0f); //отражаем по Y
    glTranslatef(0.0f, 2*floor_height, 0.0f);						// смещаем под зеркало
		glEnable(GL_TEXTURE_2D);
		render_cube();
		render_lighting();
    	render_fireball();

    	glPopMatrix();
    	glPushMatrix();
    	glScalef(1.0f, -1.0f, 1.0f); //отражаем по Y
    	glTranslatef(0.0,-5.0,0.0);
    	render_sphere();
		glPopMatrix();
    	glPushMatrix();
    	glScalef(1.0f, -1.0f, 1.0f); //отражаем по Y
		glTranslatef(0.0f, 2*floor_height, 0.0f);						// смещаем под зеркало
		render_sparks();
	glPopMatrix();
    glDisable(GL_STENCIL_TEST);
    
}

void draw_only_fb(void)
{
	glDisable(GL_TEXTURE_2D);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 	// очистка буферов
	glColorMask(0,0,0,0);															// запрет рисования на "экране"
    glEnable(GL_STENCIL_TEST);														// вкл. стенсильный буфер
    glStencilFunc(GL_ALWAYS, 1, 1);												// заполняем 1-ми там где рисуем 
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);                                     // 
//    glDisable(GL_DEPTH_TEST);                                                      // пох на глубину
    render_fireball();									   
    glEnable(GL_DEPTH_TEST);                                                        // включаем глубину, рисование, и фиксируем стенсильный буфер
    glStencilFunc(GL_ALWAYS, 0, 0);										
	render_cube();
    glColorMask(1,1,1,1);
    glStencilFunc(GL_EQUAL, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glEnable(GL_TEXTURE_2D);
	render_fireball();
	glDisable(GL_STENCIL_TEST);
}

void draw_fb_blur(double alpha)
{
	double r;
		glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 	// очистка буферов
	glOrtho(0,w_width,0,w_height,-1,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_2D);
	 r=1-(1-alpha)*(fps0/(double)get_fps());
	 if(r>alpha) r=alpha;
//	r=fb_blur_alpha;
		glBindTexture(GL_TEXTURE_2D, fbo_texture_id[current_fb_texture]);
	glColor4d(0, 0, 0, r);
	glEnable(GL_BLEND);

	r=fb_blur_alpha_r*(fps0/(double)get_fps());
	if (r<fb_blur_alpha_r) r=fb_blur_alpha_r;
	
	glBlendFunc(GL_SRC_ALPHA,  GL_ONE);
		glBegin(GL_QUADS);	
        //      glColor3ub(95, 95, 95); //6
		glTexCoord2f(0.0,0.0-r);		
                glVertex2f(0.0, 0.0);
		glTexCoord2f(0.0,1.0-r);		
                glVertex2f(0.0, (float)w_height);
		glTexCoord2f(1.0,1.0-r);		
                glVertex2f( (float)w_width, (float)w_height);
		glTexCoord2f(1.0,0.0-r);		
                glVertex2f((float)w_width, 0.0 );
    glEnd();
	
glDisable(GL_BLEND);
glDisable(GL_TEXTURE_2D);
glColor3d(0.0, 0.0, 0.0);
		glBegin(GL_QUADS);	
        //      glColor3ub(95, 95, 95); //6
                glVertex2f(0.0, 0.0);
                glVertex2f(0.0, 5);
		        glVertex2f( (float)w_width, 5);
		        glVertex2f((float)w_width, 0.0 );
    glEnd();

glEnable(GL_TEXTURE_2D);
set_perspective();

}

void draw_time(void)
{
	create_time_texture();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 	// очистка буферов
	glOrtho(0,w_width,0,w_height,-1,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_2D);
	bind_time_texture();
	glColor4d(0.5,0.5,0.5,1);
	glBegin(GL_QUADS);	
        //      glColor3ub(95, 95, 95); //6
		glTexCoord2f(0.0,1.0);		
                glVertex2f(0.0, 0.0);
		glTexCoord2f(0.0,0.0);		
                glVertex2f(0.0, (float)w_height);
		glTexCoord2f(1.0,0.0);		
                glVertex2f( (float)w_width, (float)w_height);
		glTexCoord2f(1.0,1.0);		
                glVertex2f((float)w_width, 0.0 );
    glEnd();

//	glDisable(GL_BLEND);
//	glDisable(GL_TEXTURE_2D);
	set_perspective();
	SDL_GL_SwapBuffers();	
}

void draw_combined_texture(void)
{
	if(1)
	{
		
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 	// очистка буферов
	glOrtho(0,combined_texture_width,0,combined_texture_height,-1,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_2D);
//	glBindTexture(GL_TEXTURE_2D, blur_texture);
	glBindTexture(GL_TEXTURE_2D, fbo_texture_id[0]);
	glColor4d(1.0, 1.0, 1.0, 1.0);
    glDisable(GL_BLEND);
	
	glBegin(GL_QUADS);	
        //      glColor3ub(95, 95, 95); //6
		glTexCoord2f(0.0,0.0);		
                glVertex2f(0.0, 0.0);
		glTexCoord2f(0.0,1.0);		
                glVertex2f(0.0, (float)combined_texture_height);
		glTexCoord2f(1.0,1.0);		
                glVertex2f( (float)combined_texture_width, (float)combined_texture_height);
		glTexCoord2f(1.0,0.0);		
                glVertex2f((float)combined_texture_width, 0.0 );
    glEnd();
	glBindTexture(GL_TEXTURE_2D, fbo_texture_id[2]);
   	glColor4d(1.0, 1.0, 1.0, fb_blur_alpha);
   	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,  GL_ONE);
	glBegin(GL_QUADS);	
        //      glColor3ub(95, 95, 95); //6
		glTexCoord2f(0.0,0.0);		
                glVertex2f(0.0, 0.0);
		glTexCoord2f(0.0,1.0);		
                glVertex2f(0.0, (float)combined_texture_height);
		glTexCoord2f(1.0,1.0);		
                glVertex2f( (float)combined_texture_width, (float)combined_texture_height);
		glTexCoord2f(1.0,0.0);		
                glVertex2f((float)combined_texture_width, 0.0 );
    glEnd();
	glBindTexture(GL_TEXTURE_2D, texture2_id);
	glColor4d(1.0, 1.0, 1.0, rnd_alpha);
    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,  GL_ONE);
	glBegin(GL_QUADS);	
        //      glColor3ub(95, 95, 95); //6
		glTexCoord2f(0.0,0.0);		
                glVertex2f(0.0, 0.0);
		glTexCoord2f(0.0,1.0);		
                glVertex2f(0.0, (float)combined_texture_height);
		glTexCoord2f(1.0,1.0);		
                glVertex2f( (float)combined_texture_width, (float)combined_texture_height);
		glTexCoord2f(1.0,0.0);		
                glVertex2f((float)combined_texture_width, 0.0 );
	glEnd();
	glDisable(GL_BLEND);
	set_perspective();
	}
}

void draw_blur(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 	// очистка буферов
	glOrtho(0,w_width,0,w_height,-1,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, fbo_texture_id[current_blur_texture]);
	glColor4d(1.0, 1.0, 1.0, blur_alpha);
    glEnable(GL_BLEND);

	if (blur_func) glBlendFunc(GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA);
	else glBlendFunc(GL_SRC_ALPHA,  GL_ONE);
	
	glBegin(GL_QUADS);	
        //      glColor3ub(95, 95, 95); //6
		glTexCoord2f(0.0+blur_r,0.0+blur_r);		
                glVertex2f(0.0, 0.0);
		glTexCoord2f(0.0+blur_r,1.0-blur_r);		
                glVertex2f(0.0, (float)w_height);
		glTexCoord2f(1.0-blur_r,1.0-blur_r);		
                glVertex2f( (float)w_width, (float)w_height);
		glTexCoord2f(1.0-blur_r,0.0+blur_r);		
                glVertex2f((float)w_width, 0.0 );
    glEnd();

	glDisable(GL_BLEND);
//	glDisable(GL_TEXTURE_2D);
	set_perspective();
}

void draw_scene(void)
{
//    int i;
	reload_textures();
	glLoadIdentity();
    gluLookAt(0,camera_height,-3,  0,0,z_dst,  0,1,0);
//	glViewport(0,0,320,200);
    mirror();
//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 	// очистка буферов

        render_cube();
        render_sphere();
//		glEnable(GL_BLEND);
		render_lighting();
//		glEnable(GL_BLEND);
        render_fireball();

		

        glEnable(GL_BLEND);
        glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
         glBlendFunc(GL_ONE,  GL_ONE_MINUS_SRC_ALPHA);
         render_floor();

		glDisable(GL_BLEND);
        render_sparks();
	if (str_flag) {   SDL_WM_SetCaption(str,NULL); str_flag=0; }
	draw_blur();
//	draw_fb_blur(1);
	glFlush();
}


void render_to_texture(void)
{
	
	glViewport(0,0,blur_texture_width,blur_texture_height);
	glBindTexture(GL_TEXTURE_2D, 0);								// unlink textures because if we dont it all is gonna fail
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_id[0]);					// switch to rendering on our FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D, fbo_texture_id[current_blur_texture], 0);
	checkfbo();
	current_blur_texture=1-current_blur_texture;
	
	draw_scene();
	
//	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
//	glBindTexture(GL_TEXTURE_2D, blur_texture);
//	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 0, 0, blur_texture_width, blur_texture_height, 0);   // GL_LUMINANCE -> GL_RGB8
	glBindTexture(GL_TEXTURE_2D, 0);								// unlink textures because if we dont it all is gonna fail
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D, fbo_texture_id[current_fb_texture], 0);
	if(current_fb_texture==2)current_fb_texture=3; else current_fb_texture=2;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 	// очистка буферов
	glLoadIdentity();
    gluLookAt(0,camera_height,-3,  0,0,z_dst,  0,1,0);
	draw_only_fb();                             // !!!!!!!!!!!!!!!!!!!!!!!11
	draw_fb_blur(fb_blur_alpha);
//	glBindTexture(GL_TEXTURE_2D, fb_blur_texture);
//	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 0, 0, blur_texture_width, blur_texture_height, 0);
	glBindTexture(GL_TEXTURE_2D, 0);								// unlink textures because if we dont it all is gonna fail
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_id[1]);					// switch to rendering on our FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D, fbo_texture_id[4], 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 	// очистка буферов
	glLoadIdentity();
	glViewport(0,0,combined_texture_width,combined_texture_height);
	draw_combined_texture();
//	draw_scene();
//	glBindTexture(GL_TEXTURE_2D, combined_texture);
//	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 0, 0, combined_texture_width, combined_texture_height, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 	// очистка буферов
	glViewport(0,0,w_width,w_height);
	
}


void render_frame(void)
{
    render_to_texture();
	draw_scene();
	draw_blur();
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 	// очистка буферов
	draw_fb_blur(1);
	rtt_enabled=0;
    SDL_GL_SwapBuffers();
}


void end_all(void)
{
	stop_music();
	timer_stoped=1;
	endfire();
	free_blue_fire_texture();
	free_rnd_texture();
	free_green_fire_texture();
	lighting_destroy(&lighting1);
	end_particles();
	sound_end();
	SDL_Quit();
}

void process_keys(void)
{
	char key;
	key=key_pressed();
	if(key=='h') timer_stoped=1;
	if(key=='r') timer_stoped=0;
}

int main(int argc, char *argv[])
{
  init();
  begin_listen_keys();
  while(!(esc_pressed()))
  {
    render_frame();
    frame_count();
     process_keys();
     sleep(0);
}
  
  end_all();
  return 0;
}
