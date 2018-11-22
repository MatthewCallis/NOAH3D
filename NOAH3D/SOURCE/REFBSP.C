//-------------------------------------------------------------------
// REFBSP.C
//-------------------------------------------------------------------

#include "refresh.h"
#include <stdio.h>

saveseg_t	*drawseg;

//-------------------------------------------------------------------
//	RenderWallLoop		CALLED: CORE LOOPING ROUTINE
//-------------------------------------------------------------------

void RenderWallLoop (void);

#ifndef __ORCAC__
void RenderWallLoop (void)
{
	fixed_t	texturecollumn;
	ushort	tile,scaler, angle;

/**/
/* calculate and draw each collumn*/
/**/
	if (rw_stopx >SCREENWIDTH || rw_x > rw_stopx)
		IO_Error ("Bad RenderWallLoop: %i to %i",rw_x, rw_stopx);


	for ( ; rw_x < rw_stopx ; rw_x++,rw_scale+=rw_scalestep)
	{
		scaler = rw_scale >> FRACBITS;
		angle = rw_centerangle + xtoviewangle[rw_x];
		texturecollumn = SUFixedMul (finetangent[angle],rw_distance);
		if (rw_downside)
			texturecollumn = rw_midpoint - texturecollumn;
		else
			texturecollumn = rw_midpoint + texturecollumn;
		xscale[rw_x] = rw_scale;
		if ((ushort)texturecollumn < rw_mintex)
			texturecollumn = rw_mintex;
		else if ((ushort)texturecollumn >= rw_maxtex)
			texturecollumn = rw_maxtex-1;
		texturecollumn >>= 3;		/* each tile has 5 fractional (collumn) bits*/
		tile = rw_texture[texturecollumn>>5];
		texturecollumn = (tile<<5) + (texturecollumn & 31);
		IO_ScaleWallColumn (rw_x, scaler,texturecollumn);
	}
/*IO_DisplayViewBuffer ();	// blit to screen*/
}
#endif


//-------------------------------------------------------------------
// RenderWallRange	Draw a wall segment between start and stop
//								angles (inclusive) (short angles)
//							No clipping is needed
//-------------------------------------------------------------------

void RenderWallRange (ushort start, ushort stop)
{
	ufixed_t	scale2;
	ushort	vangle;

#ifdef DRAWBSP
	rwallcount++;
#endif

// Mark the segment as visible for auto map

	drawseg->dir |= DIR_SEENFLAG;	// For AUTOMAP
	areavis[drawseg->area] = 1;	// For sprite drawing

	start -= ANGLE180;
	stop -= ANGLE180;
	vangle = (ushort)(start+ANGLE90)>>ANGLETOFINESHIFT;
	rw_x = viewangletox[ vangle ];
	vangle = (ushort)(stop+ANGLE90-1)>>ANGLETOFINESHIFT;   // Make non-inclusive
	rw_stopx = viewangletox[ vangle ];
	if (rw_stopx == rw_x)
		return;		// Less than one column wide
	rw_scale = ScaleFromGlobalAngle (start + centershort);
	if (rw_stopx > rw_x+1 )
	{
		scale2 = ScaleFromGlobalAngle (stop+centershort);
		rw_scalestep = Div16by8 (scale2 - rw_scale , rw_stopx-rw_x);
	}
	RenderWallLoop ();
}

#ifdef	ID_VERSION
void RenderWallEnd (void)
{ }
#endif


//-------------------------------------------------------------------
// ClipWallSegment	Clips the given screenpost and includes it in
//							newcollumn
//-------------------------------------------------------------------

// A screenpost_t is a solid range of visangles, used to clip and
// detect span exposures/hidings

typedef struct
{
	ushort	top, bottom;
} screenpost_t;

#define	MAXSEGS	16

screenpost_t	solidsegs[MAXSEGS], *newend;	// newend is one past the last valid seg


void ClipWallSegment (ushort top, ushort bottom);

#ifndef __ORCAC__
void ClipWallSegment (ushort top, ushort bottom)
{
	screenpost_t	*next, *start;

//	printf ("ClipWallSegment: %i to %i\n",top,bottom);

//-------------------------------------------
// Find the first clippost that touches the
// source post (adjacent pixels are touching)
//-------------------------------------------

	start = solidsegs;
	while (start->bottom > top+1)
		start++;

	if (top > start->top)
	{
		if (bottom > start->top+1)
		{	// Post is entirely visible (above start), so insert a new clippost
			RenderWallRange (top, bottom);
			next = newend;
			newend++;
			while (next != start)
			{
				*next = *(next-1);
				next--;
			}
			next->top = top;
			next->bottom = bottom;
			return;
		}

		// There is a fragment above *start

		RenderWallRange (top, start->top + 1);
		start->top = top;		// Adjust the clip size
	}

	if (bottom >= start->bottom)
		return;		// Bottom contained in start

	next = start;
	while (bottom <= ((screenpost_t *)(next+1))->top+1)
	{
		// There is a fragment between two posts

		RenderWallRange (next->bottom - 1, ((screenpost_t *)(next+1))->top + 1);
		next++;
		if (bottom >= next->bottom)
		{	// Bottom is contained in next

			start->bottom = next->bottom;	// Adjust the clip size
			goto crunch;
		}
	}

	// There is a fragment after *next

	RenderWallRange (next->bottom - 1, bottom);
	start->bottom = bottom;		// Adjust the clip size

//-------------------------------------------
// Remove start+1 to next from the clip list,
// because start now covers their area
//-------------------------------------------

crunch:
	if (next == start)
		return;		// Post just extended past the bottom of one post

	while (next++ != newend)	// Remove a post
		*++start = *next;
	newend = start+1;
}
#endif


//-------------------------------------------------------------------
// ClearClipSegs
//-------------------------------------------------------------------

void ClearClipSegs (void)
{
	solidsegs[0].top = 0xFFFF;
	solidsegs[0].bottom = ANGLE180 + clipshortangle;
	solidsegs[1].top = ANGLE180 - clipshortangle;
	solidsegs[1].bottom = 0;
	newend = solidsegs+2;
}


//-------------------------------------------------------------------
// P_DrawSeg		Clips and draws the given segment
//-------------------------------------------------------------------

void P_DrawSeg (saveseg_t *seg)
{
	ushort	segplane;
	ushort	door;
	door_t	*door_p;
	ushort	span, tspan;
	ushort	angle1, angle2;
	int		texslide;

	if (seg->dir & DIR_DISABLEDFLAG)
		return;		// Pushwall part

#ifdef DRAWBSP
	drawsegcount++;	 
	DrawOverheadSeg (seg, 0,1,0);
#endif

	segplane = (ushort)seg->plane << 7;
	rw_mintex = (ushort)seg->min << 7;
	rw_maxtex = (ushort)seg->max << 7;

//----------------------------
// Adjust pushwall segs
//----------------------------

	if (seg == pwallseg)
	{
		if (seg->dir&1)
		{	// East/West
			segplane += pwallychange;
		}
		else
		{	// North/South
			segplane += pwallxchange;
		}
	}

//----------------------------
// Get texture
//----------------------------

	if (seg->texture > 128)
	{	// Segment is a door
		door = seg->texture - 129;
		door_p = &doors[door];
		rw_texture = &textures[129 + (door_p->info>>1)][0];
		texslide = door_p->position;
		rw_mintex += texslide;
	}
	else
	{
		texslide = 0;
		rw_texture = &textures[seg->texture][0];
	}

	switch (seg->dir&3)	// Mask off the flags
	{
		case di_north:
			rw_distance = viewx - segplane;
			if ((short)rw_distance <= 0)
				return;		// Back side
			rw_downside = false;
			rw_midpoint = viewy;
			normalangle = 2*FINEANGLES/4;
			angle1 = PointToAngle (segplane, rw_maxtex);
			angle2 = PointToAngle (segplane, rw_mintex);
			break;
		case di_south:
			rw_distance = segplane - viewx;
			if ((short)rw_distance <= 0)
				return;		// Back side
			rw_downside = true;
			rw_midpoint = viewy;
			normalangle = 0*FINEANGLES/4;
			angle1 = PointToAngle (segplane, rw_mintex);
			angle2 = PointToAngle (segplane, rw_maxtex);
			break;
		case di_east:
			rw_distance = viewy - segplane;
			if ((short)rw_distance <= 0)
				return;		// Back side
			rw_downside = true;
			rw_midpoint = viewx;
			normalangle = 1*FINEANGLES/4;
			angle1 = PointToAngle (rw_mintex, segplane);
			angle2 = PointToAngle (rw_maxtex, segplane);
			break;
		case di_west:
			rw_distance = segplane - viewy;
			if ((short)rw_distance <= 0)
				return;		// Back side
			rw_downside = false;
			rw_midpoint = viewx;
			normalangle = 3*FINEANGLES/4;
			angle1 = PointToAngle (rw_maxtex, segplane);
			angle2 = PointToAngle (rw_mintex, segplane);
			break;
	}

//----------------------------
// Clip to view edges
//----------------------------

	span = angle1 - angle2;
	if (span >= 0x8000)
		return;
	angle1 -= centershort;
	angle2 -= centershort;
	angle2++;	// Make angle 2 non inclusive

	tspan = angle1 + clipshortangle;
	if (tspan > 2*clipshortangle)
	{
		tspan -= 2*clipshortangle;
		if (tspan >= span)
			return;	// Totally off the left edge
		angle1 = clipshortangle;
	}
	tspan = clipshortangle - angle2;
	if (tspan > 2*clipshortangle)
	{
		tspan -= 2*clipshortangle;
		if (tspan >= span)
			return;	// Totally off the left edge
		angle2 = -clipshortangle;
	}

//----------------------------
// Calc center angle for
// texture mapping
//----------------------------

	rw_centerangle = (centerangle-normalangle)&FINEMASK;
	if (rw_centerangle > FINEANGLES/2)
		rw_centerangle -= FINEANGLES;
	rw_centerangle += FINEANGLES/4;

	rw_midpoint -= texslide;
	rw_mintex -= texslide;

	angle1 += ANGLE180;		// Adjust so angles are unsigned
	angle2 += ANGLE180;
	ClipWallSegment (angle1, angle2);
}


//-------------------------------------------------------------------
// CheckBSPNode	Returns true if some part of the BSP dividing line
//						might be visible
//----------------------------

boolean CheckBSPNode (short x1, short y1, short x2, short y2)
{
	short 	angle1, angle2;
	ushort	span, tspan;
	ushort	uangle1, uangle2;
	screenpost_t	*start;

#ifdef DRAWBSP
	checksegcount++;
	DrawLine (x1,y1,x2,y2, 1,0,0);
#endif
	angle1 = PointToAngle (x1, y1) - centershort;
	angle2 = PointToAngle (x2, y2) - centershort;

//----------------------------
// Check clip list for an open
// space
//----------------------------

	span = angle1 - angle2;
	if (span >= 0x8000)
		return true;	// Sitting on a line
	tspan = angle1 + clipshortangle;
	if (tspan > 2*clipshortangle)
	{
		tspan -= 2*clipshortangle;
		if (tspan >= span)
			return false;	// Totally off the left edge
		angle1 = clipshortangle;
	}
	tspan = clipshortangle - angle2;
	if (tspan > 2*clipshortangle)
	{
		tspan -= 2*clipshortangle;
		if (tspan >= span)
			return false;	// Totally off the left edge
		angle2 = -clipshortangle;
	}

//----------------------------
// Find the first clippost
// that touches the source
// post (adjacent pixels are
// touching)
//----------------------------

	uangle1 = angle1 + ANGLE180;
	uangle2 = angle2 + ANGLE180;
	start = solidsegs;
	while (start->bottom > uangle1+1)
		start++;
	if (uangle1 <= start->top && uangle2 >= start->bottom)
		return false;	/* the clippost contains the new span*/

	return true;
}


//-------------------------------------------------------------------
// TerminalNode		Draw one or more segments
//-------------------------------------------------------------------

void TerminalNode (saveseg_t *seg)
{
	do
	{
		drawseg = seg;
		P_DrawSeg (seg);
		if (seg->dir & DIR_LASTSEGFLAG)
			return;
		seg++;
	} while (1);
}


//-------------------------------------------------------------------
// RenderBSPNode
//-------------------------------------------------------------------

ushort	checkcoord[12][4] = {
	{3,0, 2,1},
	{3,0, 2,0},
	{3,1, 2,0},
		//jgt	{},
		{NULL },	//jgt
	{2,0, 2,1},
	{0,0,0,0},
	{3,1, 3,0},
		//jgt	{},
		{NULL },	//jgt
	{2,0, 3,1},
	{2,1, 3,1},
	{2,1, 3,0}
};

ushort	bspcoord[4];
ushort	boxx, boxy, boxpos;
ushort	ckx1, cky1, ckx2, cky2;


void RenderBSPNode (ushort bspnum)
{
	savenode_t 	*bsp;
	ushort		side;
	ushort		coordinate, savednum, savedcoordinate;

#ifdef DRAWBSP
	PSsetrgbcolor (1,1,1);
	PSsetalpha (0.1);
	PScompositerect (
		bspcoord[BSPLEFT]*FRACTODPS, 
		(64*FRACUNIT-bspcoord[BSPBOTTOM])*FRACTODPS,
		(bspcoord[BSPRIGHT]-bspcoord[BSPLEFT])*FRACTODPS, 
		(bspcoord[BSPBOTTOM]-bspcoord[BSPTOP])*FRACTODPS,
		NX_SOVER );
	PSsetalpha (1);
	NXPing();
#endif

	bsp = &nodes[bspnum];
	if (bsp->dir & DIR_SEGFLAG)
	{
		TerminalNode ((saveseg_t *)bsp);
		return;
	}
	
//----------------------------
// Decision node
//----------------------------

#ifdef DRAWBSP	
	DrawPlane (bsp, 1,1,1);
#endif

	coordinate = bsp->plane<<7;	// Stored as half tiles
	
	if (bsp->dir == or_vertical)
	{
		side = viewx > coordinate;	// Vertical decision line
		savednum = BSPLEFT + (side^1);
	}
	else
	{
		side = viewy > coordinate;	// Horizontal decision line
		savednum = BSPTOP + (side^1);
	}
	
	savedcoordinate = bspcoord[savednum];
	bspcoord[savednum] = coordinate;
	
	RenderBSPNode (bsp->children[side^1]);	// Recursively divide front space

	bspcoord[savednum] = savedcoordinate;
	savednum ^= 1;
	savedcoordinate = bspcoord[savednum];
	bspcoord[savednum] = coordinate;
	
//----------------------------
// If the back side node is a
// single seg, don't bother
// explicitly checking
// visibility
//----------------------------

	if ( ! ( nodes[bsp->children[side]].dir & DIR_LASTSEGFLAG ) )
	{
	//----------------------------
	// Don't flow into the back
	// space if it is not going
	// to be visible
	//----------------------------

		if (viewx <= bspcoord[BSPLEFT])
			boxx = 0;
		else if (viewx < bspcoord[BSPRIGHT])
			boxx = 1;
		else
			boxx = 2;
			
		if (viewy <= bspcoord[BSPTOP])
			boxy = 0;
		else if (viewy < bspcoord[BSPBOTTOM])
			boxy = 1;
		else
			boxy = 2;
			
		boxpos = (boxy<<2)+boxx;
		if (boxpos == 5)
			IO_Error ("boxpos == 5");
			
		ckx1 = bspcoord[checkcoord[boxpos][0]];
		cky1 = bspcoord[checkcoord[boxpos][1]];
		ckx2 = bspcoord[checkcoord[boxpos][2]];
		cky2 = bspcoord[checkcoord[boxpos][3]];
		
		if (!CheckBSPNode (ckx1, cky1, ckx2, cky2 ) )
			goto skipback;
	}
	RenderBSPNode (bsp->children[side]);  // Recursively divide back space

skipback:
	bspcoord[savednum] = savedcoordinate;
}


