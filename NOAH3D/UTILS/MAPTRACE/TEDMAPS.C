// make sure word alignment is OFF!

//shit	#include <libc.h>
#include "cmdlib.h"

#ifdef __BORLANDC__
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <alloc.h>
#endif

#ifdef	__WATCOMC__
#include	<stdio.h>
#endif

//========================================

typedef struct
{
	short		RLEWtag;
	long		headeroffsets[100];
	byte		tileinfo[1];
} mapfiletype;


typedef	struct
{
	long		planestart[3];
	USHORT	planelength[3];
	USHORT	width,height;
	char		name[16];
} maptype;

//========================================

mapfiletype			*tinf;
maptype				header;

int					maphandle;
USHORT				*mapplanes[3];

//========================================


/*
======================
=
= CA_RLEWexpand
= length is EXPANDED length
=
======================
*/

void CA_RLEWexpand (USHORT *source, USHORT *dest,long length,
  unsigned rlewtag)
{
	USHORT 	value,count,i;
	USHORT		*end;
	
	end = dest + length;
	//
	// expand it
	//
	do
	{
		value = IntelShort(*source++);
		if (value != rlewtag)
		//
		// uncompressed
		//
			*dest++=value;
		else
		{
		//
		// compressed string
		//
			count = IntelShort(*source++);
			value = IntelShort(*source++);
			for (i=1;i<=count;i++)
				*dest++ = value;
		}
	} while (dest<end);
}



/*
======================
=
= LoadTedHeader
=
======================
*/

void LoadTedHeader (const char *extension)
{
	int		i;
	char		name[200];
	
//
// load maphead.ext (offsets and tileinfo for map file)
//
	strcpy (name, "maphead.");
	strcat (name, extension);
	LoadFile (name, (void *)&tinf);
#ifdef	ID_SUCKS
	tinf = (void *)((short *)tinf - 1);	// fix structure alignment
#endif
	for (i=0 ; i<100 ; i++)
		tinf->headeroffsets[i] = IntelLong (tinf->headeroffsets[i]);
//
// open the data file
//
	strcpy (name, "maptemp.");
	strcat (name, extension);
	maphandle = SafeOpenRead (name);
}



/*
======================
=
= LoadMap
=
======================
*/

void LoadMap (int mapnum)
{
	long		pos,compressed,expanded;
	int		plane, i;
	byte		*dest,  *buffer;
	USHORT	*source;

//
// load map header
//
	pos = tinf->headeroffsets[mapnum];
	if (pos<0)						// $FFFFFFFF start is a sparse map
		Error ("LoadMap: Tried to load a non existent map!");

	lseek(maphandle,pos,SEEK_SET);
	SafeRead (maphandle,&header,sizeof(maptype));

	for (i=0 ; i<3; i++)
	{
		header.planestart[i] = IntelLong (header.planestart[i]);
		header.planelength[i] = IntelShort (header.planelength[i]);
	}
	header.width = IntelShort (header.width);
	header.height = IntelShort (header.height);
	

//
// load the planes in
//
	expanded = header.width * header.height * 2;

	for (plane = 0; plane<=2; plane++)
	{
		pos = header.planestart[plane];
		lseek(maphandle,pos,SEEK_SET);

		compressed = header.planelength[plane];
		buffer = SafeMalloc(compressed);
		SafeRead( maphandle,buffer,compressed);

		mapplanes[plane] = SafeMalloc(expanded);

		//
		// unRLEW, skipping expanded length
		//
		CA_RLEWexpand ((USHORT *)(buffer+2), (USHORT *)mapplanes[plane]
		,expanded>>1,0xabcd);

		free (buffer);
	}
}


