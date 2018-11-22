
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <process.h>
#include <dos.h>
#include <ctype.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <alloc.h>


#include "cmdlib.h"


/*
==============
=
= main
=
==============
*/

void main (int argc, char **argv)
{
	byte		escapebyte;
	byte		*infile, *endout, *outfile, *out_p;
	unsigned	inlength, outlength, matchbits, compword, match, offset;
	unsigned	matchmask;

	if (argc != 3)
		Error ("DECOMP infile outfile");

//
// load source
//
	inlength = LoadFile (argv[1], &(void *)infile);
	printf ("%5u %s\n",inlength,argv[1]);
	outfile = out_p = SafeMalloc (0x10000l);

	outlength = *((unsigned *)infile)++;
	escapebyte = *infile++;
	matchbits = *infile++;

	matchmask = (1<<matchbits)-1;
	endout = out_p + outlength;

//
// decompress
//
	while (out_p < endout)
	{
		if (*infile == escapebyte)
		{
			infile++;
			if (!*infile)
			{	// wasted a byte signaling the escape char
				*out_p++ = escapebyte;
				infile++;
			}
			else
			{	// compressed string
				compword = *((unsigned *)infile)++;
				match = (compword&matchmask)+3;
				offset = compword >> matchbits;
				memcpy (out_p, out_p-offset, match);
				out_p += match;
			}
		}
		else
			*out_p++ = *infile++;		// not compressed
	}

	if (out_p != endout)
		Error ("Overshot specified length");

//
// save file
//
	printf ("%5u %s\n",outlength,argv[2]);
	SaveFile (argv[2], outfile, outlength);
}

