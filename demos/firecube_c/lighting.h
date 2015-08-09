typedef struct
{
	double y;
	double z;
} point_type;

typedef struct
{
	int 	N;
	double 	length;
	double	r;
	point_type	*points;
	double	col_r,col_g,col_b,col_a;
} lighting_type;




char lighting_init(lighting_type *lighting);
void lighting_destroy(lighting_type *lighting);
void lighting_draw(lighting_type *lighting);
void lighting_update(lighting_type *lighting);
void lighting_draw_to(lighting_type *lighting, double x, double y, double z);
