#define VERSION	"v0.5"

/*
=============================================================================

							  WALLGRAB

						  by John Carmack

=============================================================================
*/


#include <stdio.h>
#include <string.h>
#include <alloc.h>
#include <dir.h>
#include <dos.h>
#include <mem.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <io.h>
#include <process.h>
#include <bios.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>

#include "types.h"
#include "minmisc.h"
#include "script.h"

#define SCREENWIDTH		320
#define SCREENHEIGHT	200

#define PEL_WRITE_ADR		0x3c8
#define PEL_READ_ADR		0x3c7
#define PEL_DATA			0x3c9
#pragma hdrstop


void LoadLBM (char *filename);


byte	lumpbuffer[16384], *lump_p;

int			datahandle;
unsigned	iocount;


/*
=============================================================================

							GRABBING

=============================================================================
*/

typedef struct
{
	short	width;
	short	dataofs[64];		// only [width] used
} sprpic_t;

#define SCRN(x,y)	(*(byte far *)MK_FP(0xa000,y*SCREENWIDTH+x))

/*
==============
=
= GrabSnesWall
=
= Position is 0-31
=
==============
*/

void GrabSnesWall (int position)
{
	int		x,y,xl,yl,xh,yh;
	byte		far *screen_p;

	xl = 40*(position&7);
	yl = 40*(position/8);

	xh = xl+32;
	yh = yl+32;

	lump_p = lumpbuffer;

	for (x=xl ; x<xh ; x++)
	{
		screen_p = MK_FP(0xa000,yl*SCREENWIDTH+x);
		for (y=yl ; y<yh ; y++)
		{
			*lump_p++ = *screen_p;
			*screen_p = 0;
			screen_p += SCREENWIDTH;
		}
	}
}



/*
=============================================================================

							MAIN

=============================================================================
*/


/*
=================
=
= main
=
=================
*/

void main(void)
{
	int			i, count;
	long		size,offset;
	int			collumns,sprites;
	long		datapos;
	char		name[80];


//
// read in the script file
//
	LoadScriptFile ("wlscript.txt");

//
// init output structures
//
	datahandle = SafeOpenWrite ("wcolumns.bin");
	datapos = 0;
//
// parse script
// filename.lbm numtograb
//
	while (1)
	{
		GetToken (true);
		if (endofscript)
			break;

		LoadLBM (token);
		GetToken (false);
		count = atoi (token);
		for (i=0 ; i<count ; i++)
		{
			GrabSnesWall (i);
		//
		// write the raw data
		//
			size = lump_p - lumpbuffer;
			SafeWrite (datahandle,lumpbuffer, size);
			datapos += size;
		}
	}

//
// end files
//
	close (datahandle);

//
// all done
//
	asm	mov	ax,0x3		// go into text mode
	asm	int	0x10

	printf ("data: %lu\n", datapos);

	exit (0);
}


