
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

int	windowsize;
int	matchbits;
int	maxmatchsize;


byte	escapebyte;


unsigned	counts[256];

int		bestmatch;
unsigned	bestoffset;

unsigned	inlength, outlength, byteon;
unsigned	compword;
byte		*infile, *outfile, *out_p;


/*
==============
=
= FindEscapeByte
=
==============
*/

void FindEscapeByte (void)
{
	byte	*buf;
	unsigned	length, i;

	length = inlength;
	buf = infile;

	memset (counts,0,sizeof(counts));

	while (length--)
		counts[*buf++]++;

	escapebyte = 0;

	for (i=1 ; i<256 ; i++)
		if (counts[i] < counts[escapebyte])
			escapebyte = i;
}



/*
==============
=
= ScanForMatches
=
==============
*/

void ScanForMatches (void)
{
	byte	*new, *scan;
	byte	*stopscan;
	int		match, maxmatch, matchbound;

	new = infile + byteon;
	if (byteon > windowsize)
		stopscan = new - windowsize;
	else
		stopscan = infile;

	matchbound = inlength - byteon;
	if (matchbound > maxmatchsize)
		matchbound = maxmatchsize;

	bestmatch = 3;		// need a match of at least 4 chars to compress

	for (scan = new-4 ; scan >= stopscan ; scan--)
	{
		if (*new != *scan)
			continue;

		match = 1;
		maxmatch = new - scan;
		if (maxmatch > matchbound)
			maxmatch = matchbound;
		do
		{
			if (new[match] != scan[match])
				break;
			match++;
		} while (match != maxmatch);

		if (match <= bestmatch)
			continue;

		bestmatch = match;
		bestoffset = new-scan;
	}


}



/*
=============
=
= main
=
=============
*/

void main (int argc, char **argv)
{
	int		oldshift;

	if (argc != 3 && argc != 4)
		Error ("SNESCOMP infile outfile [matchbits]");

	if (argc == 4)
		matchbits = atoi(argv[3]);
	else
		matchbits = 4;

	maxmatchsize = (1<<matchbits)+2;	// 0 is special, 1 is a match of 4
	windowsize = (1<<(16-matchbits))-1;


//
// load source
//
	inlength = LoadFile (argv[1], &(void *)infile);
	printf ("%5u %s\n",inlength,argv[1]);
	outfile = out_p = SafeMalloc (0x10000l);

//
// find the least used byte for escape byte
//
	FindEscapeByte ();
	*((unsigned *)out_p)++ = inlength;
	*out_p++ = escapebyte;
	*out_p++ = matchbits;
	printf ("matchbits: %i  escapebyte: %i\n",matchbits, escapebyte);


//
// compress the data bytes
//
	byteon = 0;
	oldshift = 0;
	while (byteon < inlength)
	{
		if (byteon >> 10 != oldshift)
		{
			printf (".");
			oldshift = byteon>>10;
		}

		ScanForMatches ();
		if (bestmatch < 4)
		{
		// no compression
			*out_p++ = infile[byteon];
			if (infile[byteon] == escapebyte)
			// extra code to output escape
				*out_p++ = 0;		// a first compbyte of 0 is not a string
			byteon++;
		}
		else
		{
		// compressed string
			*out_p++ = escapebyte;
			compword = (bestoffset<<matchbits) + (bestmatch-3);
			*((unsigned *)out_p)++ = compword;
			byteon += bestmatch;
		}
	}

//
// save the new file
//
	outlength = out_p - outfile;
	printf ("\n%5u %s\n",outlength,argv[2]);
	printf ("%i%% compression\n", 100 - (100l*outlength/inlength));
	SaveFile (argv[2], outfile, outlength);

	exit (0);
}

