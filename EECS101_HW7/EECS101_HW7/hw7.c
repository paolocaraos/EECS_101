#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ROWS		(int)480
#define COLUMNS		(int)640
#define X_OFFSET	COLUMNS / 2
#define Y_OFFSET	ROWS / 2
#define UCHAR_MAX	255

#define SQRD(x)						(double) ((x)*(x))
#define MAX(x, y)					(x > y) ?  x:y
#define MIN(x, y)					(x < y) ?  x:y
#define ABS(x)						(x < 0) ? -x:x

void clear(unsigned char image[][COLUMNS]);
void header(int row, int col, unsigned char head[32]);

void surfaceNormal(double normal_vector[3], double r, int x, int y);
void hypotheticalNormal(double h[3], double surface_v[3], double camera_v[3]);
double angleBetweenVectors(double v1[3], double v2[3]);
double lambertian(double theta);
double specular(double alpha, double m);
double sceneRadiance(double La, double Ls, double a);

int main(int argc, char **argv)
{
	FILE			*fp;
	unsigned char	image[ROWS][COLUMNS], head[32];
	double			L[ROWS][COLUMNS];
	int				i, j, x, y;

	double			normal_mag, max_L = 0;
	double			hypo_normal_vector[3];
	double			normal_vector[3];
	double			camera_vector[3] = { 0,0,1 };
	double			source_vector[3];

	double			source_v[9][3];
	int 			radius[9], r;
	double 			a[9];
	double			m[9];
	char			fname[9][50], ifilename[50];
	char			ch[50];

	strcpy(fname[0], "image-a.ras");
	strcpy(fname[1], "image-b.ras");
	strcpy(fname[2], "image-c.ras");
	strcpy(fname[3], "image-d.ras");
	strcpy(fname[4], "image-e.ras");
	strcpy(fname[5], "image-f.ras");
	strcpy(fname[6], "image-g.ras");
	strcpy(fname[7], "image-h.ras");
	strcpy(fname[8], "image-i.ras");
	
	//a)
	source_v[0][0] = 0;
	source_v[0][1] = 0;
	source_v[0][2] = 1;
	radius[0]		  = 50;
	a[0]			  =	0.5;
	m[0]			  = 1;
	
	//b)
	source_v[1][0] = 0.57735;
	source_v[1][1] = 0.57735;
	source_v[1][2] = 0.57735;
	radius[1]		  = 50;
	a[1]			  =	0.5;
	m[1]			  = 1;

	//c)
	source_v[2][0] = 1;
	source_v[2][1] = 0;
	source_v[2][2] = 0;
	radius[2]		  = 50;
	a[2]			  =	0.5;
	m[2]			  = 1;

	//d)
	source_v[3][0] = 0;
	source_v[3][1] = 0;
	source_v[3][2] = 1;
	radius[3]		  = 10;
	a[3]			  =	0.5;
	m[3]			  = 1;

	//e)
	source_v[4][0] = 0;
	source_v[4][1] = 0;
	source_v[4][2] = 1;
	radius[4]		  = 100;
	a[4]			  =	0.5;
	m[4]			  = 1;

	//f)
	source_v[5][0] = 0;
	source_v[5][1] = 0;
	source_v[5][2] = 1;
	radius[5]		  = 50;
	a[5]			  =	0.1;
	m[5]			  = 1;

	//g)
	source_v[6][0] = 0;
	source_v[6][1] = 0;
	source_v[6][2] = 1;
	radius[6]		  = 50;
	a[6]			  =	1;
	m[6]			  = 1;

	//h)
	source_v[7][0] = 0;
	source_v[7][1] = 0;
	source_v[7][2] = 1;
	radius[7]		  = 50;
	a[7]			  =	0.5;
	m[7]			  = 0.1;

	//i)
	source_v[8][0] = 0;
	source_v[8][1] = 0;
	source_v[8][2] = 1;
	radius[8]		  = 50;
	a[8]			  =	0.5;
	m[8]			  = 10000;	
	
	header(ROWS, COLUMNS, head);

	for (j = 0; j < 9; j++) {
		clear(image);
		max_L = 0;
		//Calculate L for each point on the surface of the sphere
		for (y = -radius[j]; y < radius[j]; y++){
			for (x = -radius[j]; x < radius[j]; x++)
			{
				r = sqrt(SQRD(x) + SQRD(y));
				//check if this x,y is lies within the circle
				if (r <= radius[j]) {
					//find surface normal
					surfaceNormal(&normal_vector, radius[j], x, y);

					//find hypothetical normal
					hypotheticalNormal(&hypo_normal_vector, normal_vector, camera_vector);

					//calculate L for this (x,y)
					L[y + Y_OFFSET][x + X_OFFSET]  = sceneRadiance(lambertian(angleBetweenVectors(source_v[j], normal_vector)),
						specular(angleBetweenVectors(normal_vector, hypo_normal_vector), m[j]), a[j]);
				}
			}
		}
		//normalize L to 255 and clear L
		for (y = 0; y < ROWS; y++) {
			for (x = 0; x < COLUMNS; x++) {
				if (L[y][x] > 0) {
					image[y][x] = L[y][x] * UCHAR_MAX;
					L[y][x] = 0;
				}
			}
		}

		if (!(fp = fopen(fname[j], "wb"))){
			fprintf(stderr, "error: could not open %s\n", fname[j]);
			exit(1);
		}
		fwrite(head, 4, 8, fp);
		for (i = 0; i < ROWS; i++) fwrite(image[i], 1, COLUMNS, fp);
		fclose(fp);
	}

	printf("Press any key to exit: ");
	scanf(" %s", &ch);
	return 0;
}

//assume vectors are unit vectors
double angleBetweenVectors(double v1[3], double v2[3])
{
	return acos(v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
}

double lambertian(double theta)
{
	return cos(theta);
	//return sv[0] * nv[0] + sv[1] * nv[1] + sv[2] * nv[2];
}

//alpha is the angle between surface normal and hypothetical surface normal
double specular(double alpha, double m)
{
	return exp(-SQRD(alpha / m));
}

double sceneRadiance(double La, double Ls, double a)
{
	return a*La + (1 - a) * Ls;
}

void surfaceNormal(double normal_vector[3], double r, int x, int y)
{
	double p, q, magnitude;
	p = -x / (sqrt(SQRD(r) - SQRD(x) - SQRD(y)));
	q = -y / (sqrt(SQRD(r) - SQRD(x) - SQRD(y)));
	magnitude = sqrt(SQRD(p) + SQRD(q) + 1);
	normal_vector[0] = -p / magnitude;
	normal_vector[1] = -q / magnitude;
	normal_vector[2] = 1 / magnitude;
}

//returns unit hypothetical surface normal
void hypotheticalNormal(double h[3], double surface_v[3], double camera_v[3])
{
	h[0] = surface_v[0] + camera_v[0];
	h[1] = surface_v[1] + camera_v[1];
	h[2] = surface_v[2] + camera_v[2];

	double magnitude = sqrt(SQRD(h[0]) + SQRD(h[1]) + SQRD(h[2]));

	h[0] /= magnitude;
	h[1] /= magnitude;
	h[2] /= magnitude;
}

void clear(unsigned char image[][COLUMNS])
{
	int	i, j;
	for (i = 0; i < ROWS; i++)
		for (j = 0; j < COLUMNS; j++) image[i][j] = 0;
}

void header(int row, int col, unsigned char head[32])
{
	int *p = (int *)head;
	char *ch;
	int num = row * col;

	/* Choose little-endian or big-endian header depending on the machine. Don't modify this */
	/* Little-endian for PC */

	*p = 0x956aa659;
	*(p + 3) = 0x08000000;
	*(p + 5) = 0x01000000;
	*(p + 6) = 0x0;
	*(p + 7) = 0xf8000000;

	ch = (char*)&col;
	head[7] = *ch;
	ch++;
	head[6] = *ch;
	ch++;
	head[5] = *ch;
	ch++;
	head[4] = *ch;

	ch = (char*)&row;
	head[11] = *ch;
	ch++;
	head[10] = *ch;
	ch++;
	head[9] = *ch;
	ch++;
	head[8] = *ch;

	ch = (char*)&num;
	head[19] = *ch;
	ch++;
	head[18] = *ch;
	ch++;
	head[17] = *ch;
	ch++;
	head[16] = *ch;


	/* Big-endian for unix */
	/*
	*p = 0x59a66a95;
	*(p + 1) = col;
	*(p + 2) = row;
	*(p + 3) = 0x8;
	*(p + 4) = num;
	*(p + 5) = 0x1;
	*(p + 6) = 0x0;
	*(p + 7) = 0xf8;
	*/
}
