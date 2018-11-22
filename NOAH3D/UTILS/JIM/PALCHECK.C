#include <stdio.h>
#include <stdlib.h>

FILE *inf;

typedef unsigned char byte;
typedef unsigned int word;

void main(int argc, char *argv[])
{
	int	i,c;
	word	color;
	byte	red,green,blue;

	printf("PalCheck(v1.0)- SNES WOLF3D palette extractor\n");
	printf("Copyright(c)1994 by Color Dreams, Inc.  All rights reserved.\n\n");

	if(argc != 2) {
		printf("Usage: PalCheck <file>\n");
		exit(1);
	}

	if((inf = fopen(argv[1],"rb")) == (FILE *) NULL) {
		printf("Error opening input file '%s'!\n",argv[1]);
		exit(2);
	}

	for(i = 0; i < 256; i++) {
		c = fgetc(inf);
		color = (byte) c + ((word) fgetc(inf) << 8);
		red = color & 0x1F;
		green = (color >> 5) & 0x1F;
		blue = (color >> 10) & 0x1F;
		printf("%02X (%04X) R%02X G%02X B%02X\n",i,color,red,green,blue);
	}

	printf("\n");

	fclose(inf);
	exit(0);
}

