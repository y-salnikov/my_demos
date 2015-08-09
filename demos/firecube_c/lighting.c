#include <GL/gl.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "lighting.h"
#include "3dmath.h"

char lighting_init(lighting_type *lighting)
{
	lighting->points=calloc(lighting->N,sizeof(point_type));
	if (lighting->points==NULL) return 255; //error
	return 0;
}

void lighting_destroy(lighting_type *lighting)
{
	free(lighting->points);
}

void lighting_update(lighting_type *lighting)
{
	int i;
	for(i=0;i<lighting->N;i++)
	{
		lighting->points[i].y=((random() % 1024)/1024.0)*lighting->r;
		lighting->points[i].z=((random() % 1024)/1024.0)*lighting->r;
	}
}

void lighting_draw(lighting_type *lighting)
{
	double dx;
	int i;

	dx=lighting->length/(double)lighting->N;
	
	glPushMatrix();
		glColor4d(lighting->col_r,lighting->col_g,lighting->col_b,lighting->col_a);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINE_STRIP);
//		glBegin(GL_TRIANGLE_FAN);
			for(i=0;i<lighting->N;i++)
			{
				glVertex3d(dx*i,lighting->points[i].y,lighting->points[i].z);
			}
		glEnd();
		glEnable(GL_TEXTURE_2D);
	glPopMatrix();
}

void lighting_draw_to(lighting_type *lighting, double x, double y, double z)
{
	double fz,fy,dx,l,tx,ty,tz;
	struct vector
	{
		double x,y,z;
	} Y0,Z0;
	int i;


	Z0.x=0.0; Z0.y=0.0; Z0.z=1.0;
	
	l=sqrt(x*x+y*y+z*z);
	fz=asin(fabs(y/sqrt(x*x+y*y)));
	fy=asin(fabs(z/(sqrt(x*x+z*z+y*y))));
	
	
	if((z<0)&&(x>0)&&(y>0)) fy=-fy;
	if((z>0)&&(x<0)&&(y>0)) { fz=M_PI-fz;  }
	if((z<0)&&(x<0)&&(y>0)) { fz=M_PI-fz; fy=-fy;}
	if((z>0)&&(x<0)&&(y<0)) { fz=M_PI+fz;  }
	if((z<0)&&(x<0)&&(y<0)) { fz=M_PI+fz; fy=-fy;}
	if((z>0)&&(x>0)&&(y<0)) { fz=-fz; }
	if((z<0)&&(x>0)&&(y<0)) { fz=-fz; fy=-fy;}

	fz=180*fz/M_PI;
	fy=180*fy/M_PI;
	
	dx=l/(double)(lighting->N-1);
//	glPushMatrix();
//	glTranslated(x,y,z);
//	glutWireSphere(0.2,6,6);
//	glPopMatrix();
	glPushMatrix();
		glColor4d(lighting->col_r,lighting->col_g,lighting->col_b,lighting->col_a);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINE_STRIP);
//		glBegin(GL_TRIANGLE_FAN);
			for(i=0;i<lighting->N;i++)
			{
				tx=dx*i;
				ty=lighting->points[i].y;
				tz=lighting->points[i].z;
				Y0.x=0.0; Y0.y=-1.0; Y0.z=0.0;
				
				rotate_v(fz,&tx,&ty,&tz,Z0.x,Z0.y,Z0.z);
				rotate_v(fz,&Y0.x,&Y0.y,&Y0.z,Z0.x,Z0.y,Z0.z);

				
				rotate_v(fy,&tx,&ty,&tz,Y0.x,Y0.y,Y0.z);
				
				glVertex3d(tx,ty,tz);
			}
		glEnd();
		glEnable(GL_TEXTURE_2D);
	glPopMatrix();

	
}
