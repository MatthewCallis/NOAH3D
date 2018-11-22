//-------------------------------------------------------------------
// INTRO.C
//-------------------------------------------------------------------

#include <string.h>

#include "sneswolf.h"
#include "wolfdef.h"

#define		SCRRADIUS	40
#define		SRCRADIUS	16
#define		SRCSIZE		(SRCRADIUS*4)

byte			*pic;
byte			*maptable;
byte			*bytebuffer;
byte			*bigpic;
ushort		*unwound;
ushort		unwoundcount;

extern byte	ramscalers;

int			introhack;


#ifdef	ID_VERSION
//-------------------------------------------------------------------
// MapSphere		Give origin X and Y with four fractional bits
//-------------------------------------------------------------------

void MapSphere (fixed_t x, fixed_t y)
{
	byte		*map;
	ushort	sx,sy;
	ushort	mx, my;
	ushort	ix,iy;
	ushort	ofs;
	byte		pixel;
	
	map = maptable;
	
	for (sy=0;sy<SCRRADIUS;sy++)
	{
		for (sx=0;sx<=SCRRADIUS;sx++)
		{
			mx = *map++;
			if  (mx == 0xFF)
				break;			// End of row
			my = *map++;
			
//-------------------------
// Quad 0
//-------------------------

			ix = ((x+mx)>>3)&63;
			iy = ((y-my)>>3)&63;
			pixel = pic[iy*64+ix];
			ofs = (SCRRADIUS-1-sy) + (SCRRADIUS+sx)*SCREENHEIGHT;
			bytebuffer[ofs] = pixel;

//-------------------------
// Quad 1
//-------------------------

			ix = ((x-mx)>>3)&63;
			iy = ((y-my)>>3)&63;
			pixel = pic[iy*64+ix];
			ofs =  (SCRRADIUS-1-sy) + (SCRRADIUS-1-sx)*SCREENHEIGHT;
			bytebuffer[ofs] = pixel;

//-------------------------
// Quad 2
//-------------------------

			ix = ((x-mx)>>3)&63;
			iy = ((y+my)>>3)&63;
			pixel = pic[iy*64+ix];
			ofs = (SCRRADIUS+sy) + (SCRRADIUS-1-sx)*SCREENHEIGHT;
			bytebuffer[ofs] = pixel;

//-------------------------
// Quad 3
//-------------------------

			ix = ((x+mx)>>3)&63;
			iy = ((y+my)>>3)&63;
			pixel = pic[iy*64+ix];
			ofs = (SCRRADIUS+sy) + (SCRRADIUS+sx)*SCREENHEIGHT;
			bytebuffer[ofs] = pixel;
		}
	}
}


//-------------------------------------------------------------------
// BuildLogos
//-------------------------------------------------------------------

void BuildLogos (void)
{
	ushort	y;
	byte		*dest;

	pic = &logos;

	dest = bigpic;

	for (y=0; y<64; y++) {
		memcpy (dest, pic, 64);
		memcpy (dest+64, pic, 64);
		memcpy (dest+64*128, pic, 64);
		memcpy (dest+64*129, pic, 64);
		pic += 64;
		dest += 128;
	}
}


//-------------------------------------------------------------------
// UnwindMapping
//-------------------------------------------------------------------

void UnwindMapping (void)
{
	byte		*map;
	ushort	x,y;
	ushort	*dest;
	ushort	sx,sy;
	ushort	mx, my;
	ushort	ix,iy;
	ushort	ofs;
	
	map = maptable;
	dest = unwound;

	x = 256+8;
	y = 256+8;

	for (sy=0;sy<SCRRADIUS;sy++)
	{
		for (sx=0;sx<=SCRRADIUS;sx++)
		{
			mx = *map++;
			if  (mx == 0xFF)
				break;		// End of row
			my = *map++;

//-------------------
// Quad 0
//-------------------

			ix = ((x+mx)>>3)&63;
			iy = ((y-my)>>3)&63;
			ofs = (SCRRADIUS-1-sy) + (SCRRADIUS+sx)*SCREENHEIGHT;
			*dest++ = (iy<<7)+ix;	// Pixel souce
			*dest++ = ofs;

//-------------------
// Quad 1
//-------------------

			ix = ((x-mx)>>3)&63;
			iy = ((y-my)>>3)&63;
			ofs =  (SCRRADIUS-1-sy) + (SCRRADIUS-1-sx)*SCREENHEIGHT;
			*dest++ = (iy<<7)+ix;	// Pixel source
			*dest++ = ofs;

//-------------------
// Quad 2
//-------------------

			ix = ((x-mx)>>3)&63;
			iy = ((y+my)>>3)&63;
			ofs = (SCRRADIUS+sy) + (SCRRADIUS-1-sx)*SCREENHEIGHT;
			*dest++ = (iy<<7)+ix;	// Pixel source
			*dest++ = ofs;

//-------------------
// Quad 3
//-------------------

			ix = ((x+mx)>>3)&63;
			iy = ((y+my)>>3)&63;
			ofs = (SCRRADIUS+sy) + (SCRRADIUS+sx)*SCREENHEIGHT;
			*dest++ = (iy<<7)+ix;	// Pixel source
			*dest++ = ofs;
		}
	}
	unwoundcount = (dest-unwound)>>1;
}


//-------------------------------------------------------------------
// FastMap
//-------------------------------------------------------------------

void FastMap (ushort basesrc);

#if 0
void FastMap (ushort basesrc)
{
	ushort	*map;
	ushort	src,dest;
	ushort	count;

	map = unwound;
	count = unwoundcount;
	while (count--)
	{
		src = *map++;
		dest = *map++;
		bytebuffer[dest] = bigpic[basesrc+src];
	}
}
#endif

#endif


//-------------------------------------------------------------------
// SprPrint
//-------------------------------------------------------------------

int spron;
int spryofs,sprxofs;

void SprPrint (int x, int y, char *msg)
{
	while (*msg) {
		oam.s[spron].x = x*8 + sprxofs;
		oam.s[spron].y = y*8 + spryofs;
		oam.s[spron].name = *msg - 32;
		oam.s[spron].flags = 0x3E;			// Priority 2, palette 7
		x++;
		spron++;
		msg++;
	}
}


#ifdef	ID_VERSION
//-------------------------------------------------------------------
// Intro
//-------------------------------------------------------------------

void Intro (void)
{
	ushort	frame, org;

	maptable = &ballmap;
	bigpic = &ramscalers;
	unwound = (ushort *)(bigpic+0x4000);

	bytebuffer = &screenbuffer + 16*SCREENHEIGHT;

	SetupIntroScreen ();

	BuildLogos ();
	UnwindMapping ();

	frame = 0;
	introhack = true;
	spron = 0;

#ifdef JAPVERSION
	spryofs = -4;
#else
	spryofs = -7;
#endif
	sprxofs = 4;
	SprPrint(0, 23,"Copyright 1994 Wisdom Tree, Inc");
	SprPrint(0, 24," Published by Wisdom Tree, Inc");
	sprxofs = 0;
	SprPrint(1, 25,"under license from Id Software");
#ifndef JAPVERSION
	SprPrint(1, 26,"  Not licensed by Nintendo!");
#endif
	spryofs = 0;
	BlankVideo ();
	DmaOAram (&oam, 0, sizeof(oam));	// Upload sprite records

	StartAck ();
	StartSong (0,true);
	do
	{
		org = frame&63;
		org = (org<<7) + org;
		FastMap (org);
//		MapSphere (org<<4, org<<4);
		sendplayscreen = 1;
		while (sendplayscreen)
			;
		if (!frame)
			FadeIn ();
		frame++;

#ifndef JAPVERSION
		if (frame < 15)
			continue;	// Don't let out for a while
#endif
		if (CheckAck ())
			break;
	} while (frame < 128);

	FadeOut ();
	introhack = false;
}


//-------------------------------------------------------------------
// SystemCheck
//-------------------------------------------------------------------

#define	reg	(*(unsigned char *)0x213F)


void SystemCheck (void)
{
	boolean	palmode;
	unsigned	cheat, i;

	palmode = ((reg & 16) == 16);

#ifndef PALCHECK
	if (!palmode)
		return;
#endif
#ifdef PALCHECK
	if (palmode)
		return;
#endif

	SetupIntroScreen ();

	spron = 0;
	SprPrint (1,6, "This version of Super 3D Noah's Ark");
	SprPrint (6,8 ,     "is not designed for your");
	SprPrint (5,10,    "SUPER FAMICOM or SUPER NES");
	SprPrint (9,12,        "-- Wisdom Tree --");
	BlankVideo ();
	DmaOAram (&oam, 0, sizeof(oam));   // Upload sprite records
	FadeIn ();

	cheat = 0;
	for (i=0 ; i<16 ; i++)
	{
		ack ();
		cheat = (cheat<<1) + ((pushbits&JOYPAD_X)>0);
	}

	if (cheat == ('i'<<8) + 'd' )
		return;

freeze:
	goto freeze;

}

#endif
