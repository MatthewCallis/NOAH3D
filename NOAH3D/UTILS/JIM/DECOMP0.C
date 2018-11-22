#include	<stdio.h>
#include <stdlib.h>

typedef unsigned char byte;
typedef unsigned int word;

FILE	*infile,*outfile;
int	c,i,lb;
word	length;


void main(int argc, char *argv[])
{
	if(argc != 3) {
		printf("Usage: DECOMP0 infile outfile\n");
		exit(1);
	}

	printf("DECOMP0(v1.0) - 0-byte RLE decoder\n");
	printf("(c)1994 by Color Dreams, Inc.  All rights reserved.\n\n");

	if((infile = fopen(argv[1],"rb")) == (FILE *) NULL) {
		printf("Error opening input file '%s'!\n");
		exit(1);
	}

	if((outfile = fopen(argv[2],"wb")) == (FILE *) NULL) {
		printf("Error opening output file '%s'!\n");
		exit(1);
	}

	length = fgetc(infile) + fgetc(infile)*256;
	printf("Decompressed length = $%04X (%u) bytes\n", length, length);

	while(!feof(infile)) {
		c = fgetc(infile);
		if(!feof(infile)) {
			lb = c;
			if(((byte) c) == 0x00) {	// Only runs of 0's are compressed
				c = fgetc(infile);		// Get count
				if(c == 0x00) {
					printf("0-LENGTH RUN!\n");
					c = 256;
				}
				for(i = 0; i < c; i++)
					fputc(0x00, outfile);
			} else							// Non-zero bytes are not compressed
				fputc(c, outfile);
		}
	}
	printf("LAST BYTE = $%02X\n", (byte) lb);

	fclose(infile);
	fclose(outfile);

	exit(0);
}

