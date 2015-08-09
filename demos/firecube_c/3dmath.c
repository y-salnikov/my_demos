#include <math.h>


void rotate_x(double a, double *x, double *y, double *z)
{
	double tx,ty,tz;
	double aa;

	aa=a*M_PI/180;
	tx=*x;
	ty=*y*cos(aa)-*z*sin(aa);
	tz=(*y)*sin(aa)+*z*cos(aa);

	*x=tx;
	*y=ty;
	*z=tz;
}

void rotate_y(double a, double *x, double *y, double *z)
{
	double tx,ty,tz;
double aa;

	aa=a*M_PI/180;

	tx=*x*cos(aa)+(*z)*sin(aa);
	ty=*y;
	tz=-(*x)*sin(aa)+(*z)*cos(aa);	

	*x=tx;
	*y=ty;
	*z=tz;
}

void rotate_z(double a, double *x, double *y, double *z)
{
	double tx,ty,tz;
double aa;

	aa=a*M_PI/180;
	tx=*x*cos(aa)-(*y)*sin(aa);
	ty=(*x)*sin(aa)+*y*cos(aa);
	tz=*z;

	*x=tx;
	*y=ty;
	*z=tz; 
}


void rotate_v(double a, double *x, double *y, double *z, double xv, double yv, double zv)
{
		double tx,ty,tz;
		double aa;
		double sina,cosa;

		aa=a*M_PI/180;
		sina=sin(aa);
		cosa=cos(aa);

		tx=*x*(cosa+(1-cosa)*xv*xv)+*y*((1-cosa)*xv*yv-sina*zv)+*z*((1-cosa)*xv*zv+sina*yv);
		ty=*x*((1-cosa)*yv*xv+sina*zv)+*y*(cosa+(1-cosa)*yv*yv)+*z*((1-cosa)*yv*zv-sina*xv);
		tz=*x*((1-cosa)*zv*xv-sina*yv)+*y*((1-cosa)*zv*yv+sina*xv)+*z*(cosa+(1-cosa)*zv*zv);

		*x=tx;
		*y=ty;
		*z=tz; 
}

double vector_length(double x1, double y1, double z1, double x2, double y2, double z2)
{
	return (sqrt(((x1-x2)*(x1-x2))+((y1-y2)*(y1-y2))+((z1-z2)*(z1-z2))));
}
