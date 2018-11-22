#include	<stdio.h>
#include <stdlib.h>

typedef unsigned char byte;
typedef unsigned int word;

FILE	*infile,*outfile;
int	c,i;
word	length;


void main(int argc, char *argv[])
{
	if(argc != 3) {
		printf("Usage: ZEROPACK infile outfile\n");
		exit(1);
	}

	printf("ZEROPACK(v1.0) - 0-byte RLE encoder\n");
	printf("(c)1994 by Color Dreams, Inc.  All rights reserved.\n\n");

	if((infile = fopen(argv[1],"rb")) == (FILE *) NULL) {
		printf("Error opening input file '%s'!\n");
		exit(1);
	}

	if((outfile = fopen(argv[2],"wb")) == (FILE *) NULL) {
		printf("Error opening output file '%s'!\n");
		exit(1);
	}

	fputc(0x00, outfile);	// Write dummy length header
	fputc(0x00, outfile);

	i = 0;	// Run-length counter
	while(!feof(infile)) {
		c = fgetc(infile);
		if(!feof(infile)) {
			length++;
			if(((byte) c) == 0x00) {	// 0's are run-length compressed
				i++;							// Increment 0 counter
				if(i == 0xFF) {
					fputc(0x00, outfile);
					fputc(i, outfile);
					i = 0;
				}
			} else {							// Non-zero bytes are not compressed
				if(i) {						// Write out any 0 runs
					fputc(0x00, outfile);
					fputc(i, outfile);
					i = 0;
				}
				fputc(c, outfile);
			}
		}
	}

	if(i) {					// Write out any 0 runs
		fputc(0x00, outfile);
		fputc(i, outfile);
		i = 0;
	}

	printf("Decompressed length = $%04X (%u) bytes\n", length, length);
	rewind(outfile);
	fputc(length & 0xFF, outfile);
	fputc(length >> 8, outfile);

	fclose(infile);
	fclose(outfile);

	exit(0);
}

