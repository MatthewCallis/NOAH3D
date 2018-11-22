//-------------------------------------------------------------------
// HEX2BIN - Convert Motorolla HEX file into a BINARY file
//
// Supported formats:	.S28 (24-bit)
//
// (c)1994 Color Dreams, Inc.  All rights reserved.
//


#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#define	MAXSTR	80


typedef unsigned char	byte;
typedef unsigned int		word;
typedef unsigned long	dword;


FILE	*inf,*outf;
char	str[MAXSTR];
word	line;
dword	addr,temp;
byte	data,size;
int	i;


//-------------------------------------------------------------------

byte gethexdigit(char c)
{
	if(!isxdigit(c)) {
		printf("Non-hex digit '%c' in line %u!\n",c,line);
		exit(1);
	}

	if(isdigit(c))
		return (c-'0');
	else
		return ((toupper(c)-'A')+0x0A);
}


byte gethexbyte(char *str)
{
	return((gethexdigit(str[0])<<4)+gethexdigit(str[1]));
}


dword gethexaddr(char *str)
{
	byte a, b, c;

	a = gethexbyte(&str[0]);
	b = gethexbyte(&str[2]);
	c = gethexbyte(&str[4]);

	return ( ( (( (dword) a<<8)+b) <<8) + c);
}


//-------------------------------------------------------------------

void main(int argc, char *argv[])
{
	printf("HEX2BIN(v1.0) - HEX to BINARY file converter\n");
	printf("(c)1994 Color Dreams, Inc.  All rights reserved\n\n");

	if(argc != 3) {
		printf("Usage is: HEX2BIN <infile> <outfile>\n\n");
		exit(1);
	}

	if((inf = fopen(argv[1],"r")) == (FILE *) NULL) {
		printf("Unable to open input file '%s'!\n", argv[1]);
		exit(1);
	}

	if((outf = fopen(argv[2],"wb")) == (FILE *) NULL) {
		printf("Unable to open output file '%s'!\n", argv[2]);
		exit(1);
	}

	addr = 0x0L;	// Binary file offset
	line = 1;		// Line number

	while(!feof(inf)) {
		if(fgets(str,MAXSTR,inf) == (char *) NULL)
			break;

		if(str[0] != 'S') {
			printf("Input file not S28 format!\n\n");
			exit(1);
		}

		if(str[1] == '2') {	// Binary data...?
			size = gethexbyte(&str[2]);	// Get number of bytes on this line
			temp = gethexaddr(&str[4]);	// Get 3-byte address

			temp &= 0xFFFFF;	// Limit to SNES address range!

//			printf("$%06lX\r",temp);

			if(temp != addr) {
				if(temp < addr) {
					printf("Descending address $%06lX in line %u!\n\n",temp,line);
					exit(1);
				}
				printf("$%06lX\n",temp);
				for( ; addr < temp; addr++)
					fputc(0x00,outf);
			}

			for(i = 0; i < size-0x04; i++) {
				data = gethexbyte(&str[10+i*2]);
				fputc(data, outf);
			}

			addr += (size-0x04);
		}

		line++;
	}

	printf("$%06lX bytes\n\n",addr);

	fclose(inf);
	fclose(outf);
	exit(0);
}
