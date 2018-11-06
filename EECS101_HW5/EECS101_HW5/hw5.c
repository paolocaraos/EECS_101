#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define ROWS	480
#define COLUMNS	640

#define GX_			0
#define GY_			1
#define SGM_		2

#define M_PI						3.14159265358979323846
#define U_CHAR_MAX					255
#define	THETA_MAX					180
#define RHO_MAX						1600

#define sqr(x)						((x)*(x))
#define SQRD(x)						((x)*(x))
#define MAX(x, y)					(x > y) ?  x:y
#define MIN(x, y)					(x < y) ?  x:y
#define ABS(x)						(x < 0) ? -x:x
#define NORMALIZE(x, min, max)		((double) (x-min) / (max-min))
#define DENORMALIZE(x, min, max)    x *(max-min) + min;

void clear(unsigned char image[][COLUMNS]);
void header(int row, int col, unsigned char head[32]);
void edge_detect(int max[]);
void hough_transform(int max[]);
int rho(int x, int y, int theta);

int main(int argc, char** argv)
{
	char ch[50];
	int max[3];
	edge_detect(&max);
	printf("%d %d %d\n", max[0], max[1], max[2]);
	hough_transform(&max);

	printf("Press any key to exit: ");
	scanf("%s", ch);
	return 0;
}


void hough_transform(int max[])
{
	int				x, y, th, rh, max_rh = 0, min_rh = 2147483600;
	int				lx, ly, rho_f;
	int				i, j;
	int				max_f = 0, min_f = 2147483600, max_l = 0;
	int				dedx, dedy, sgm;
	int				sgm_threshold, hough_threshold, voting[THETA_MAX][RHO_MAX];
	FILE*			fp;
	unsigned char	image[ROWS][COLUMNS], head[32];
	char			filename[50];

	strcpy(filename, "image-b.ras");
	memset(voting, 0, sizeof(int) * THETA_MAX * RHO_MAX);

	sgm_threshold = 250;
	hough_threshold = 250;
	printf("sgm_threshold = %d, hough threshold = %d\n", sgm_threshold, hough_threshold);
	if (!(fp = fopen(filename, "rb")))
	{
		fprintf(stderr, "error: couldn't open %s\n", filename);
		exit(1);
	}
	for (x = 0; x < ROWS; x++)
		if (!(COLUMNS == fread(image[x], sizeof(char), COLUMNS, fp)))
		{
			fprintf(stderr, "error: couldn't read %s\n", filename);
			exit(1);
		}
	fclose(fp);

	//find the range of rho
	for (y = 1; y < ROWS - 1; y++) {
		for (x = 1; x < COLUMNS - 1; x++) {
			if (image[y][x] > sgm_threshold) {
				for (th = 0; th < THETA_MAX; th++) {
					rh = rho(x, y, th);
					max_rh = MAX(rh, max_rh);
					min_rh = MIN(rh, min_rh);
					voting[th][rh + RHO_MAX/2]++;
				}
			}
		}
	}
	
	clear(image);
	
	for(x = 0; x < RHO_MAX; x++){
		for(y = 0 ; y < THETA_MAX; y++){
			if(voting[y][x] > hough_threshold){				
				int max_l = voting[y][x];
				/*find the local max within 3x9 grid
				for (ly = -4; ly <= 4; ly++) {
					for (lx = -4; lx <= 4; lx++) {
						if (((ly + y) >= 0 && THETA_MAX >(ly + y)) && ((lx + x) >= 0 && (lx + x) < RHO_MAX))
						{
							if (voting[y + ly][x + lx] > max_l)
							{
								j = y + ly;
								i = x + lx;
								max_l = voting[y + ly][x + lx];
							}
						}
					}
				}
				if (max_l > voting[y][x])
					continue;*/
					
				rho_f = x - RHO_MAX/2;
				printf("::rho = %d, theta = %d :: votes = %d\n", rho_f, y, voting[y][x]);
				//print it
				for (j = 0; j < ROWS; j++) {
					for (i = 0; i < COLUMNS; i++) {
						if (rho_f == rho(i, j, y)) {
							image[j][i] = UCHAR_MAX;
						}						
					}
				}
				x += 2;
				for (ly = -2; ly <= 2; ly++) {
					for (lx = -10; lx <= 10; lx++) {
						if (((ly + y) >= 0 && THETA_MAX >(ly + y)) && ((lx + x) >= 0 && (lx + x) < RHO_MAX))
						{
							if (((y + ly) != j) && ((x + lx) != i))
							{
								voting[y + ly][x + lx] = 0;
							}
						}
					}
				}
			}
		}
	}
	header(ROWS, COLUMNS, head);
	if (!(fp = fopen("hough.ras", "wb")))
	{
		fprintf(stderr, "error: could not open %s\n", "hough.ras");
		exit(1);
	}
	fwrite(head, 4, 8, fp);
	for (x = 0; x < ROWS; x++) fwrite(image[x], 1, COLUMNS, fp);
	fclose(fp);
}

int rho(int x, int y, int theta)
{
	return y * sin(theta*((double)M_PI / 180)) + x * cos(theta*((double)M_PI / 180));
}

void edge_detect(int max[])
{
	int				i, j, threshold;
	double			x, y, s;
	FILE			*fp;
	unsigned char	image[ROWS][COLUMNS], ximage[ROWS][COLUMNS], yimage[ROWS][COLUMNS], head[32];
	char			filename[20], ifilename[20];

	strcpy(filename, "image");
	header(ROWS, COLUMNS, head);

	threshold = 30;

	printf("Maximum of Gx, Gy, SGM\n");
	{
		clear(ximage);
		clear(yimage);

		/* Read in the image */
		strcpy(ifilename, filename);
		if ((fp = fopen(strcat(ifilename, ".raw"), "rb")) == NULL)
		{
			fprintf(stderr, "error: couldn't open %s\n", ifilename);
			exit(1);
		}
		for (i = 0; i < ROWS; i++)
			if (fread(image[i], sizeof(char), COLUMNS, fp) != COLUMNS)
			{
				fprintf(stderr, "error: couldn't read enough stuff\n");
				exit(1);
			}
		fclose(fp);

		/* Write the original image to a new image */
		strcpy(ifilename, filename);
		if (!(fp = fopen(strcat(ifilename, ".ras"), "wb")))
		{
			fprintf(stderr, "error: could not open %s\n", ifilename);
			exit(1);
		}
		fwrite(head, 4, 8, fp);
		for (i = 0; i < ROWS; i++) fwrite(image[i], 1, COLUMNS, fp);
		fclose(fp);

		max[0] = 0; //maximum of Gx
		max[1] = 0; //maximum of Gy
		max[2] = 0; //maximum of SGM

					/* Compute Gx, Gy, SGM, find out the maximum and normalize*/
					/*Find max of GX and GY*/
		for (i = 1; i < ROWS - 1; i++) {
			for (j = 1; j < COLUMNS - 1; j++) {
				x = image[i - 1][j - 1] * -1;
				x += image[i][j - 1] * -2;
				x += image[i + 1][j - 1] * -1;

				x += image[i - 1][j + 1];
				x += image[i][j + 1] * 2;
				x += image[i + 1][j + 1];

				y = image[i - 1][j - 1] * -1;
				y += image[i - 1][j] * -2;
				y += image[i - 1][j + 1] * -1;

				y += image[i + 1][j - 1];
				y += image[i + 1][j] * 2;
				y += image[i + 1][j + 1];

				x = ABS(x);
				y = ABS(y);

				max[GY_] = MAX(y, max[GY_]);
				max[GX_] = MAX(x, max[GX_]);
			}
		}

		/*normalize GX and GY, and find SGM*/
		for (i = 1; i < ROWS - 1; i++) {
			for (j = 1; j < COLUMNS - 1; j++) {
				x = image[i - 1][j - 1] * -1;
				x += image[i][j - 1] * -2;
				x += image[i + 1][j - 1] * -1;

				x += image[i - 1][j + 1];
				x += image[i][j + 1] * 2;
				x += image[i + 1][j + 1];

				y = image[i - 1][j - 1] * -1;
				y += image[i - 1][j] * -2;
				y += image[i - 1][j + 1] * -1;

				y += image[i + 1][j - 1];
				y += image[i + 1][j] * 2;
				y += image[i + 1][j + 1];

				x = ABS(x);
				y = ABS(y);

				ximage[i][j] = (x / max[GX_]) * 255;
				yimage[i][j] = (y / max[GY_]) * 255;

				max[SGM_]
					= MAX(SQRD(ximage[i][j]) + SQRD(yimage[i][j]), max[SGM_]);
			}
		}

		/*normalize SGM*/
		for (i = 0; i < ROWS; i++) {
			for (j = 0; j < COLUMNS; j++) {
				s = SQRD(ximage[i][j]) + SQRD(yimage[i][j]);
				image[i][j] = (s / max[SGM_]) * 255;
			}
		}

		/* Write Gx to a new image*/
		strcpy(ifilename, filename);
		if (!(fp = fopen(strcat(ifilename, "-x.ras"), "wb")))
		{
			fprintf(stderr, "error: could not open %s\n", ifilename);
			exit(1);
		}
		fwrite(head, 4, 8, fp);
		for (i = 0; i < ROWS; i++) fwrite(ximage[i], 1, COLUMNS, fp);
		fclose(fp);

		/* Write Gy to a new image */
		strcpy(ifilename, filename);
		if (!(fp = fopen(strcat(ifilename, "-y.ras"), "wb")))
		{
			fprintf(stderr, "error: could not open %s\n", ifilename);
			exit(1);
		}
		fwrite(head, 4, 8, fp);
		for (i = 0; i < ROWS; i++) fwrite(yimage[i], 1, COLUMNS, fp);
		fclose(fp);

		/* Write SGM to a new image */
		strcpy(ifilename, filename);
		if (!(fp = fopen(strcat(ifilename, "-s.ras"), "wb")))
		{
			fprintf(stderr, "error: could not open %s\n", ifilename);
			exit(1);
		}
		fwrite(head, 4, 8, fp);
		for (i = 0; i < ROWS; i++) fwrite(image[i], 1, COLUMNS, fp);
		fclose(fp);

		/* Compute the binary image */
		for (i = 0; i < ROWS; i++) {
			for (j = 0; j < COLUMNS; j++) {
				if (image[i][j] >= threshold) {//b(x,y) = 1
					image[i][j] = 255;
				}
				else {//b(x,y) = 0
					image[i][j] = 0;
				}
			}
		}
		/* Write the binary image to a new image */
		strcpy(ifilename, filename);
		if (!(fp = fopen(strcat(ifilename, "-b.ras"), "wb")))
		{
			fprintf(stderr, "error: could not open %s\n", ifilename);
			exit(1);
		}
		fwrite(head, 4, 8, fp);
		for (i = 0; i < ROWS; i++) fwrite(image[i], 1, COLUMNS, fp);
		fclose(fp);
	}
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
