//-------------------------------------------------------------------
// REFRESH.C
//-------------------------------------------------------------------

#include <string.h>
#include "refresh.h"

#define	DOORPIC	59


//-------------------------------------------------------------------
// Area MAXAREAS-1 will never be connected with any other area, so
// when an actor or sprite is removed, set its area to MAXAREAS-1
//
//	Scale ranges from 0xFFFF (one tile == 512 pixels high) down to
// 0x100 (one tile == 2 pixels)
//-------------------------------------------------------------------


//===================================================================
//									REFRESH ARRAYS
//===================================================================

//----------------------------
// Precalculated math tables
//----------------------------

#ifndef __ORCAC__

#include <math.h>		// Needed to calculate tables
							// (on the SNES these are linked in...)

fixed_t			sintable[ANGLES],costable[ANGLES];
ufixed_t			scaleatz[MAXZ];


// The viewangletox[viewangle + FINEANGLES/4] lookup maps the visible
// view angles to screen X coordinates, flattening the arc to a flat
// projection plane.  There will be multiple angles mapped to the
// same X.

short	viewtox[FINEANGLES/2];

// The xtoviewangle[] table maps a screen pixel to the lowest viewangle
// that maps back to X ranges from clipangle to -clipangle

short	xtoview[SCREENWIDTH+1];

// The finetangent[angle+FINEANGLES/4] table holds the fixed_t tangent
// values for view angles, ranging from MINSHORT to 0 to MAXSHORT

short	finetan[FINEANGLES/2];

// Sine values for angles 0-FINEANGLES/2, which are all positive, and
//	all a single byte used tio calculate Z values

short	finesin[FINEANGLES/2];

#endif


//----------------------------------
// Pointers to pre-calculated tables
// fudge SNES memory model
//----------------------------------

ufixed_t	*scaleatzptr = (ufixed_t *)&scaleatz;
//jgt	int	*viewangletox = (void *)&viewtox;
//jgt	int	*xtoviewangle = (void *)&xtoview;
//jgt	int	*finetangent  = (void *)&finetan;
//jgt	int	*finesine = (void *)&finesin;
short	*viewangletox = (void *)&viewtox;
short	*xtoviewangle = (void *)&xtoview;
short	*finetangent  = (void *)&finetan;
short	*finesine = (void *)&finesin;

ufixed_t	*scaleatzptr;
short	*viewangletox;
short	*xtoviewangle;
short	*finetangent;
short	*finesine;

void	VanceInit (void)
{
	scaleatzptr = (ufixed_t *)&scaleatz;
	viewangletox = (short *)viewtox;
	xtoviewangle = (short *)&xtoview;
	finetangent  = (short *)&finetan;
	finesine = (short *)&finesin;
}

#ifdef	DRAWBSP
int	drawsegcount, checksegcount, rwallcount;
#endif


#ifndef __ORCAC__
unsigned int	rw_x;
unsigned int	rw_stopx;
unsigned int	rw_scale;
unsigned int	rw_scalestep;
unsigned int	rw_distance;
unsigned int	rw_midpoint;
unsigned int	rw_mintex;
unsigned int	rw_maxtex;
byte		*rw_texture;
short	rw_centerangle;
boolean	rw_downside;		// true for dir_east and dir_south
#endif


unsigned int	normalangle;	// Angle from viewpoint to surface normal in 64k angles
unsigned int	centerangle;	// viewangle in 64k angles
unsigned int	centershort;	// viewangle in 64k angles
int	clipshortangle;

//----------------------------------
// Calculated at map load time
//----------------------------------

byte		textures[MAPSIZE*2+5][MAPSIZE];	// 0-63 is horizontal, 64-127 is vertical

//----------------------------------
// Variables
//----------------------------------

ufixed_t		xscale[SCREENWIDTH];	// Filled in by walls, used to Z clip sprites
boolean		areavis[MAXAREAS];	// Set each frame if an area borders a rendered area

//----------------------------------
// Sprite drawing/sorting vars
//----------------------------------

vissprite_t	vissprites[MAXVISSPRITES], *visspr_p;

ushort		xevents[MAXVISSPRITES], *xevent_p; /* not xevents any more, but scale events*/
ushort		sortbuffer[MAXVISSPRITES];  /* mergesort requires an extra buffer*/
ushort		*firstevent, *lastevent;
ushort		size1, size2;
ushort		*src1, *src2, *dest;


fixed_t		trx, try;

savenode_t	*nodes;
saveseg_t	*pwallseg;	// Pushwall in motion


//===================================================================
//									MATH ROUTINES
//===================================================================

#ifndef __ORCAC__

/* -8.8 * -0.8 = -8.8*/
fixed_t	FixedByFrac (fixed_t a, fixed_t b)
{
	fixed_t	c;
	c = ((long)a * (long)b)>>FRACBITS;
	return c;
}

/* -8.8 * -8.8 = -8.8*/
fixed_t	FixedMul (fixed_t a, fixed_t b)
{
	fixed_t	c;
	c = ((long)a * (long)b)>>FRACBITS;
	return c;
}

/* -8.8 * -0.8 = -8.8*/
fixed_t	SUFixedMul (fixed_t a, ufixed_t b)
{
	fixed_t	c;
	c = ((long)a * (long)b)>>FRACBITS;
	return c;
}

/* 8.8 * 8.8 = 8.8*/
ufixed_t	UFixedMul (ufixed_t a, ufixed_t b)
{
	fixed_t	c;
	c = ((long)a * (long)b)>>FRACBITS;
	return c;
}

ushort	UDiv16by8 (ushort a, ushort b)
{
	return a/b;
}

short	Div16by8 (short a, ushort b)
{
	return a/(short)b;	/* Borland and orca do this unsigned without the cast*/
}

short Mul16S8U (short a, short b)
{
	return a*b;
}

#endif




/* -8.8 / -8.8 = -8.8*/
fixed_t	FixedDiv (fixed_t a, fixed_t b)
{
	fixed_t	c;
	c = ((long)a<<FRACBITS) / b;
	return c;
}

/* 8.8 / 8.8 = 8.8 Not used in game?*/
ufixed_t	UFixedDiv (ufixed_t a, ufixed_t b)
{
	ufixed_t		c;
	c = ((long)a<<FRACBITS) / b;
	return c;
}

#ifndef __ORCAC__

fixed_t		viewsin, viewcos;

void R_SetupTransform (void)
{
	viewsin = sintable[viewangle];
	viewcos = costable[viewangle];	
}

/* R_TransformZ MUST be called first in snes implementation*/
fixed_t R_TransformX (void)
{
	fixed_t		gxt,gyt;

	gxt = -FixedByFrac(trx,viewsin);
	gyt = FixedByFrac(try,viewcos);
	return  -(gyt+gxt);
}

fixed_t R_TransformZ (void)
{
	fixed_t		gxt,gyt;

	gxt = FixedByFrac(trx,viewcos);
	gyt = -FixedByFrac(try,viewsin);
	return gxt-gyt;
}

#endif


//-------------------------------------------------------------------
// ScaleFromGlobalAngle		Takes 64k angles
//-------------------------------------------------------------------

#ifndef __ORCAC__
ufixed_t ScaleFromGlobalAngle (short visangle)
{

	fixed_t		tz;

	short		anglea, angleb;
	ushort		sinea, sineb;
	
	visangle >>= ANGLETOFINESHIFT;
	anglea = (FINEANGLES/4 + (visangle-centerangle))&(FINEANGLES/2-1);
	angleb = (FINEANGLES/4 + (visangle-normalangle))&(FINEANGLES/2-1);
	sinea = finesine[anglea];	/* bothe sines are allways positive*/
	sineb = finesine[angleb];
	
	tz = ((long)rw_distance * sinea) / sineb;

	if (tz>=MAXZ)
		tz = MAXZ-1;
	return scaleatzptr[tz];
}
#endif


//===================================================================
//										AUTOMAP
//===================================================================

extern	ushort	tilescreen[32][32];
#ifndef __ORCAC__
ushort	tilescreen[32][32];
#endif

extern	ushort	elevatorx, elevatory;

//-------------------------------------------------------------------
// DrawMapSolid		Draws special char. for all solid areas in map
//-------------------------------------------------------------------

void DrawMapSolid (ushort tx, ushort ty)
{
	ushort	x, y, maxtx, maxty;

	maxtx = tx+31;
	maxty = ty+31;

	for (y = ty; y <= maxty; y++)
		for (x = tx; x <= maxtx; x++) {
			if (!tilescreen[y-ty][x-tx] && (tilemap[y][x] & TI_BLOCKMOVE)
					&& !(tilemap[y][x] & (TI_SWITCH | TI_SECRET)))
				tilescreen[y-ty][x-tx] = 40;
		}
}

void DrawMapPrizes (ushort tx, ushort ty)
{
	ushort	i, tile, x, y, maxtx, maxty;
	static_t	*stat = statics;

	maxtx = tx+31;
	maxty = ty+31;

//----------------------------------
// Show blocking objects and prizes
//----------------------------------

	for (i = 0, stat = statics; i < numstatics; i++, stat++)
	{
		x = stat->x>>8;
		y = stat->y>>8;

		if (x < tx || x > maxtx || y < ty || y > maxty)
			continue;

		switch (stat->pic)
		{
			case S_DOG_FOOD:
			case S_FOOD:
			case S_HEALTH:
			case S_ONEUP:
				tilescreen[y-ty][x-tx] = 44;
				break;
			case S_G_KEY:
				tilescreen[y-ty][x-tx] = 41;
				break;
			case S_S_KEY:
				tilescreen[y-ty][x-tx] = 42;
				break;
			case S_AMMO:
			case S_MISSILES:
			case S_GASCAN:
			case S_AMMOCASE:
			case S_MACHINEGUN:
			case S_CHAINGUN:
			case S_FLAMETHROWER:
			case S_LAUNCHER:
			case S_BANDOLIER:
				tilescreen[y-ty][x-tx] = 45;
				break;

			case	S_CROSS:
			case	S_CHALICE:
			case	S_CHEST:
			case	S_CROWN:
				tilescreen[y-ty][x-tx] = 46;
				break;

			default:
				if (tilemap[y][x] & TI_BLOCKMOVE)
					tilescreen[y-ty][x-tx] = 43;	// Generic blocking obj
				break;
		}
	}
}

//-------------------------------------------------------------------
// DrawAutomap			The automap has to be done in quadrants for
//							various reasons
//-------------------------------------------------------------------

void DrawAutomap (ushort tx, ushort ty)
{
	ushort		i, tile;
	saveseg_t	*seg;
	ushort		x,y,xstep,ystep,count;
	ushort		min,max;
	ushort		maxtx, maxty;
	
	memset (tilescreen,0,sizeof(tilescreen));
	maxtx = tx+31;
	maxty = ty+31;

	seg = (saveseg_t *)nodes;
	for (i=0 ; i<map.numnodes ; i++, seg++)
	{
#ifdef	ID_VERSION
		if ( (seg->dir & (DIR_SEGFLAG|DIR_SEENFLAG)) !=  (DIR_SEGFLAG|DIR_SEENFLAG) )
			continue;
#else
		if (!(seg->dir & DIR_SEGFLAG) || (!gamestate.automap && !(seg->dir & DIR_SEENFLAG)))
			continue;
#endif
		min = (seg->min-1)>>1;
		max = (seg->max+3)>>1;
		
		switch (seg->dir&3)
		{
		case di_north:
			x = (seg->plane-1)>>1;
			y = min;
			xstep = 0;
			ystep = 1;
			break;
		case di_south:
			x = seg->plane>>1;
			y = min;
			xstep = 0;
			ystep = 1;
			break;
		case di_east:
			y = (seg->plane-1)>>1;
			x = min;
			xstep = 1;
			ystep = 0;
			break;
		case di_west:
			y = seg->plane>>1;
			x = min;
			xstep = 1;
			ystep = 0;
			break;
		}
		
		for (count=max-min ; count ; count--,x+=xstep,y+=ystep)
		{
			if (x < tx || x > maxtx || y < ty || y>maxty)
				continue;
			tile = tilemap[y][x];

			if (tile & TI_DOOR)
				tile = 30 + (seg->dir&1);	
			else if (tile & TI_PUSHWALL)
			{
				switch (seg->dir&3)
				{
				case di_north:
				case di_south:
					tile = textures[MAPSIZE+x][y];
					break;
				case di_east:
				case di_west:
					tile = textures[y][x];
					break;
				}
				tile = ((tile>>1)+1)&31;
			}
			else
			{
				tile = map.tilemap[y][x];

				if (! (tile&0x80) )
					continue;		// Not a solid tile
				tile &= 0x7F;
			}

			tilescreen[y-ty][x-tx] = tile;
		}		
	}

//----------------------------------
// Show the elevator in EASY mode
//----------------------------------

	if (!difficulty && elevatorx-tx < 32 && elevatory-ty < 32)
		tilescreen[elevatory-ty][elevatorx-tx] = map.tilemap[elevatory][elevatorx]&0x7f;

	if (gamestate.automap)
	{
		DrawMapSolid (tx, ty);
		DrawMapPrizes (tx, ty);
	}
}


//===================================================================


//-------------------------------------------------------------------
// StartupRendering
//-------------------------------------------------------------------

#define	PI	3.141592657

void StartupRendering (void)
{
#ifndef __ORCAC__
	int		i;
	int		minz;
	int		x;
	float		a, fv;
	long		t;
	ufixed_t	focallength;

//-------------------------
// Generate scaleatz
//-------------------------

	minz = UFixedDiv (PROJECTIONSCALE,MAXFRAC);
	for (i=0 ; i<=minz ; i++)
		scaleatz[i] = MAXFRAC;
	for ( ; i<MAXZ ; i++)
		scaleatz[i] = UFixedDiv(PROJECTIONSCALE, i);

//-------------------------
// Viewangle tangents
//-------------------------

	for (i=0 ; i<FINEANGLES/2 ; i++)
	{
		a = (i-FINEANGLES/4+0.1)*PI*2/FINEANGLES;
		fv = 256*tan (a);
		if (fv>0x7FFF)
			t = 0x7FFF;
		else if (fv<-0x7FFF)
			t = -0x7FFF;
		else
			t = fv;
		finetangent[i] = t;
	}
	
//-------------------------
// finesine table
//-------------------------

	for (i=0 ; i<FINEANGLES/2 ; i++)
	{
		a = (i+0.0)*PI*2/FINEANGLES;
		t = 256*sin (a);
		if (t>255)
			t = 255;
		if (t<1)
			t = 1;
		finesine[i] = t;
	}

//-------------------------
// Use tangent table to
//	generate viewangletox
//-------------------------

// Calc focallength so FIELDOFVIEW angles covers SCREENWIDTH

	focallength = FixedDiv (SCREENWIDTH/2
	, finetangent[FINEANGLES/4+FIELDOFVIEW/2] );
	
	for (i=0 ; i<FINEANGLES/2 ; i++)
	{
		t = SUFixedMul (finetangent[i], focallength);
		t = SCREENWIDTH/2 - t;
		if (t < -1)
			t = -1;
		else if (t>SCREENWIDTH+1)
			t = SCREENWIDTH+1;
		viewangletox[i] = t;
	}

//-------------------------
// Scan viewangletox[] to
// generate xtoviewangle[]
//-------------------------

	for (x=0;x<=SCREENWIDTH;x++)
	{
		i = 0;
		while (viewangletox[i]>=x)
			i++;
		xtoviewangle[x] = i-FINEANGLES/4-1;
	}

//-------------------------
// Take out the fencepost
// cases from viewangletox
//-------------------------

	for (i=0 ; i<FINEANGLES/2 ; i++)
	{
		t = SUFixedMul (finetangent[i], focallength);
		t = SCREENWIDTH/2 - t;
		if (viewangletox[i] == -1)
			viewangletox[i] = 0;
		else if (viewangletox[i] == SCREENWIDTH+1)
			viewangletox[i]  = SCREENWIDTH;
	}
	
#if 0
WriteFile ("scaleatz.bin", scaleatz,sizeof(scaleatz));
WriteFile ("finetan.bin",finetan,sizeof(finetan));
WriteFile ("finesin.bin", finesin,sizeof(finesin));
WriteFile ("viewtox.bin", viewtox,sizeof(viewtox));
WriteFile ("xtoview.bin", xtoview,sizeof(xtoview));
#endif


#endif

	clipshortangle = xtoviewangle[0]<<ANGLETOFINESHIFT;

// Textures for doors stay the same across maps

	w_memset (textures[128],(char)(DOORPIC+4),MAPSIZE);	// Door side
	w_memset (textures[129],(char)(DOORPIC+0),MAPSIZE);	// Regular door
	w_memset (textures[130],(char)(DOORPIC+1),MAPSIZE);	// Locked door #1
	w_memset (textures[131],(char)(DOORPIC+2),MAPSIZE);	// Locked door #2
	w_memset (textures[132],(char)(DOORPIC+3),MAPSIZE);	// Elevator
}


//-------------------------------------------------------------------
// NewMap
//-------------------------------------------------------------------

void NewMap (void)
{
	ushort	x,y, tile;
	byte		*src;
	
	memset (textures,0xff,128*64);	// Clear array so pushwall spawning can insert
												// texture numbers in empty spots

	src = &map.tilemap[0][0];
	
	for (y=0;y<MAPSIZE;y++)
		for (x=0 ; x<MAPSIZE;x++)
		{
			tile = *src++;
			if (!(tile&TI_BLOCKMOVE))	// Blocking?
				continue;
			tile &= 0x3F;
			textures[MAPSIZE+x][y] = (tile-1)*2 + 1;
			textures[y][x] = (tile-1)*2;
		}
		
	nodes = (savenode_t *)((byte *)&map + map.nodelistofs);
	pwallseg = 0;
}


/*
===============================================================================

							PUSHWALL HACKS

===============================================================================
*/

/*
======================
=
= StartPushWall
=
======================
*/

void StartPushWall (void)
{
	ushort		i;
	ushort		segdir, segplane, segmin;
	
	pwallseg = (saveseg_t *)0;
			
	switch (pwalldir)
	{
	case CD_NORTH:
		segmin = pwallx<<1;
		segplane = (pwally+1)<<1;
		segdir = di_east;
		break;
	case CD_EAST:
		segplane = pwallx<<1;
		segmin = pwally<<1;
		segdir = di_south;
		break;
	case CD_SOUTH:
		segmin = pwallx<<1;
		segplane = pwally<<1;
		segdir = di_west;
		break;
	case CD_WEST:
		segplane = (pwallx+1)<<1;
		segmin = pwally<<1;
		segdir = di_north;
		break;
	}
	
	pwallseg = (saveseg_t *)nodes;
	for (i=0 ; i<map.numnodes ; i++, pwallseg++)
	{
		if ( ! (pwallseg->dir & DIR_SEGFLAG) )
			continue;
		if ( (pwallseg->dir & 3) != segdir )
			continue;
		if (pwallseg->plane != segplane)
			continue;
		if (pwallseg->min != segmin)
			continue;
		return;
	}
	
	IO_Error ("pwall seg not found");
}

/*
======================
=
= AdvancePushWall
=
======================
*/

void AdvancePushWall (void)
{
	pwallseg->dir |= DIR_DISABLEDFLAG;
}


//===================================================================


//-------------------------------------------------------------------
// RenderView
//-------------------------------------------------------------------

void RenderView (void)
{
	ushort	frame;

	centerangle = viewangle<<4;						// 128 to 2048
	centershort = centerangle<<ANGLETOFINESHIFT;	// 2048 to 64k
	
#ifdef	DRAWBSP
	drawsegcount = 0;
	checksegcount = 0;
	rwallcount = 0;
#endif

	R_SetupTransform ();
	w_memset (areavis, (char) 0, sizeof(areavis));
	ClearClipSegs ();		// Clip first seg only to sides of screen
		
#ifdef DEBUGLINES
	IO_ClearLineDisplay ();
#endif
	IO_ClearViewBuffer ();	// Erase to ceiling/floor colors
	
#ifdef DRAWBSP
	[tileview_i lockFocus];	// NeXT only!
	PSsetrgbcolor (1,1,1);
	PSsetlinewidth (2);
#endif

	bspcoord[BSPTOP] = 0;
	bspcoord[BSPBOTTOM] = 64*FRACUNIT;
	bspcoord[BSPLEFT] = 0;
	bspcoord[BSPRIGHT] = 64*FRACUNIT;
	
	RenderBSPNode (0);	// Traverse the BSP tree from the root

#ifdef DRAWBSP
	[tileview_i unlockFocus];	// NeXT only!
#endif

	DrawSprites ();		// Sort all the sprites in any of the rendered areas
	DrawTopSprite ();		// Draw game over sprite on top of everything
	
//----------------------------
// Draw player attack shape
//----------------------------

	frame = gamestate.attackframe;
	if (frame == 4)
		frame = 1;			// Drop back frame
	IO_AttackShape (gamestate.weapon*4 + frame);

//----------------------------
// Done
//----------------------------

	IO_DisplayViewBuffer ();	// Blit to screen

//	printf ("ds: %i   rw: %i   cs: %i\n",drawsegcount, rwallcount, checksegcount);
}
