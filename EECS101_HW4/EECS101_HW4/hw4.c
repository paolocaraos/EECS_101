#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROWS		(int)480
#define COLUMNS		(int)640

#define GX_			0
#define GY_			1
#define SGM_		2

#define MAX(x, y)			(x > y) ? x:y
#define SQRD(x)				x*x
#define ABS(x)				(x < 0)? -x:x

void clear(unsigned char image[][COLUMNS]);
void header(int row, int col, unsigned char head[32]);

int main(int argc, char **argv)
{

	int				i, j, k, threshold[3], max[3];
	double			x, y, s;
	FILE			*fp;
	unsigned char	image[ROWS][COLUMNS], ximage[ROWS][COLUMNS], yimage[ROWS][COLUMNS], head[32];
	char			filename[6][50], ifilename[50], ch[50];

	strcpy(filename[0], "image1");
	strcpy(filename[1], "image2");
	strcpy(filename[2], "image3");
	header(ROWS, COLUMNS, head);

	threshold[0] = 30;
	threshold[1] = 15;
	threshold[2] = 15;

		printf("Maximum of Gx, Gy, SGM\n");
	for (k = 0; k < 3; k++)
	{
		clear(ximage);
		clear(yimage);

		/* Read in the image */
		strcpy(ifilename, filename[k]);
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
		strcpy(ifilename, filename[k]);
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
				x  = image[i - 1][j - 1] * -1;
				x += image[i][j - 1]     * -2;
				x += image[i + 1][j - 1] * -1;

				x += image[i - 1][j + 1];
				x += image[i][j + 1] * 2;
				x += image[i + 1][j + 1];

				y  = image[i - 1][j - 1] * -1;
				y += image[i - 1][j]     * -2;
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
		strcpy(ifilename, filename[k]);
		if (!(fp = fopen(strcat(ifilename, "-x.ras"), "wb")))
		{
			fprintf(stderr, "error: could not open %s\n", ifilename);
			exit(1);
		}
		fwrite(head, 4, 8, fp);
		for (i = 0; i < ROWS; i++) fwrite(ximage[i], 1, COLUMNS, fp);
		fclose(fp);

		/* Write Gy to a new image */
		strcpy(ifilename, filename[k]);
		if (!(fp = fopen(strcat(ifilename, "-y.ras"), "wb")))
		{
			fprintf(stderr, "error: could not open %s\n", ifilename);
			exit(1);
		}
		fwrite(head, 4, 8, fp);
		for (i = 0; i < ROWS; i++) fwrite(yimage[i], 1, COLUMNS, fp);
		fclose(fp);

		/* Write SGM to a new image */
		strcpy(ifilename, filename[k]);
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
				if (image[i][j] >= threshold[k]) {//b(x,y) = 1
					image[i][j] = 255;
				}
				else {//b(x,y) = 0
					image[i][j] = 0;
				}
			}
		}
		/* Write the binary image to a new image */
		strcpy(ifilename, filename[k]);
		if (!(fp = fopen(strcat(ifilename, "-b.ras"), "wb")))
		{
			fprintf(stderr, "error: could not open %s\n", ifilename);
			exit(1);
		}
		fwrite(head, 4, 8, fp);
		for (i = 0; i < ROWS; i++) fwrite(image[i], 1, COLUMNS, fp);
		fclose(fp);
		printf("%d %d %d\n", max[0], max[1], max[2]);
	}

	printf("Press any key to exit: ");
	scanf(" %s", ch);
	return 0;
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
