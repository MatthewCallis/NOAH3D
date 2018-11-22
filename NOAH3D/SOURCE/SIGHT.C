/* sight.c*/
#include "wolfdef.h"

/*
============================================================================

							PUBLICS

============================================================================
*/

boolean CheckLine (void);
void FirstSighting (void);
void T_SightPlayer (void);


/*
============================================================================

							FUNCTIONS

============================================================================
*/

/*
=====================
=
= CheckLine
=
= Returns true if a straight line between the player and actoron is unobstructed
=
=====================
*/

boolean CheckLine (void)
{
	ushort	x1,y1,xt1,yt1,x2,y2,xt2,yt2;
	ushort	xl,xh,yl,yh;
	short	xstep,ystep;
	ushort	partial;
	short	delta;
	ushort	xfrac,yfrac,deltafrac;
	ushort	intercept;


#if 0
	x1 = (actor->x+31) & ~63;		/* low bits not significant*/
	y1 = (actor->y+31) & ~63;
	xt1 = x1>>FRACBITS;
	yt1 = y1>>FRACBITS;

	x2 = (actors[0].x+31) & ~63;
	y2 = (actors[0].y+31) & ~63;
	xt2 = x2>>FRACBITS;
	yt2 = y2>>FRACBITS;
#endif

	x1 = actor->x;
	y1 = actor->y;
	xt1 = x1>>FRACBITS;
	yt1 = y1>>FRACBITS;

	x2 = actors[0].x;
	y2 = actors[0].y;
	xt2 = x2>>FRACBITS;
	yt2 = y2>>FRACBITS;

/**/
/* check for solid tiles at x crossings*/
/**/
	if (w_abs(xt2-xt1))
	{
		if (xt2 > xt1)
		{
			partial = (256-(x1&0xff))&0xff;
			xl = xt1;
			xh = xt2;
			yl = y1;
			yh = y2;
			deltafrac = x2-x1;
		}
		else
		{
			partial = (256-(x2&0xff))&0xff;
			xl = xt2;
			xh = xt1;
			yl = y2;
			yh = y1;
			deltafrac = x1-x2;
		}

		delta = yh-yl;

		if (delta >= 16*FRACUNIT || deltafrac >= 16*FRACUNIT)
			return false;
#if 0
		delta <<= 2;		/* set up fro div to return a fixed_t*/
		deltafrac >>= 6;
		ystep = Div16by8 (delta,deltafrac);
#endif
		ystep = FixedDiv (delta,deltafrac);
		yfrac = yl + FixedByFrac (ystep, partial);

		for (mapx=xl+1 ; mapx <= xh ;mapx++)
		{
			mapy = yfrac>>8;
			intercept = yfrac&(FRACUNIT-1);
			yfrac += ystep;
#if 0
			if (intercept < 16 &&( tilemap[mapy-1][mapx]&TI_BLOCKSIGHT) )
				return false;
			if (intercept > 240 &&( tilemap[mapy+1][mapx]&TI_BLOCKSIGHT) )
				return false;
#endif
			tile = tilemap[mapy][mapx];
			if (tile & TI_DOOR)
			{	/* see if the door is open enough*/
				dooron = tile&TI_NUMMASK;
				intercept = (yfrac-ystep/2)&255;
				if (intercept > doors[dooron].position)
					return false;
			}
			else if (tile & TI_BLOCKSIGHT)
				return false;
		}
	}


/**/
/* check for solid tiles at y crossings*/
/**/
	if (w_abs(yt2-yt1) > 0)
	{
		if (yt2 > yt1)
		{
			partial = (256-(y1&0xff))&0xff;
			xl = x1;
			xh = x2;
			yl = yt1;
			yh = yt2;
			deltafrac = y2-y1;
		}
		else
		{
			partial = (256-(y2&0xff))&0xff;
			xl = x2;
			xh = x1;
			yl = yt2;
			yh = yt1;
			deltafrac = y1-y2;
		}

		delta = xh - xl;
		if (delta >= 16*FRACUNIT || deltafrac >= 16*FRACUNIT)
			return false;
#if 0
		delta <<= 2;		/* set up for div to return a fixed_t*/
		deltafrac >>= 6;
		xstep = Div16by8 (delta,deltafrac);
#endif
		xstep = FixedDiv (delta,deltafrac);

		xfrac = xl + FixedByFrac (xstep, partial);

		for (mapy=yl+1 ; mapy<= yh ; mapy++)
		{
			mapx = xfrac>>8;
			intercept = xfrac&(FRACUNIT-1);
			xfrac += xstep;
#if 0
			if (intercept < 16 &&( tilemap[mapy][mapx-1]&TI_BLOCKSIGHT) )
				return false;
			if (intercept > 240 &&( tilemap[mapy][mapx+1]&TI_BLOCKSIGHT) )
				return false;
#endif
			tile = tilemap[mapy][mapx];
			if (tile & TI_DOOR)
			{	/* see if the door is open enough*/
				dooron = tile&TI_NUMMASK;
				intercept = (xfrac-(xstep>>1))&255;
				if (intercept > doors[dooron].position)
					return false;
			}
			else if (tile & TI_BLOCKSIGHT)
				return false;
		}
	}

	return true;
}


//-------------------------------------------------------------------
// FirstSighting		Puts actoron into attack mode, either after
//							reaction time or being shot
//-------------------------------------------------------------------

void FirstSighting (void)
{
	classinfo_t	*jinfo;
	ushort		sound, r;
	
	jinfo = &classinfo[actor->class];
	sound = jinfo->sightsound;

#ifdef	ID_VERSION
	if (sound == D_ESEE)
	{	// Make random human sound
		r = w_rnd ();
		if (r<128)
			sound = D_ESEE;
		else
			sound = D_ESEE2;
		// Add more as space permits
	}
	
	if (actor->class >= CL_HANS)
		PlayLocalSound (SRC_ACTOR+actoron, sound,	RndFreq (jinfo->sightfreq), VOL_HIGH);
	else
		PlayPosSound (SRC_ACTOR+actoron, sound, RndFreq (jinfo->sightfreq), VOL_NORM,
			actor->areanumber, actor->x, actor->y);
#else
	PlayPosSound (SRC_ACTOR+actoron, sound, RndFreq (jinfo->sightfreq), VOL_NORM,
		actor->areanumber, actor->x, actor->y);
#endif
		
	NewState (jinfo->sightstate);
	actor->flags |= FL_ACTIVE;
}


/*
===============
=
= T_Stand
=
= Called by actors that ARE NOT chasing the player.  If the player
= is detected (by sight, noise, or proximity), the actor is put into
= it's combat frame and true is returned.
=
= Incorporates a random reaction delay
=
===============
*/

void T_Stand (void)
{
	if (actor->reacttime)
	{
/**/
/* count down reaction time*/
/**/
		actor->reacttime--;
		if (actor->reacttime)
			return;

		if ( actor->flags&FL_AMBUSH )
		{
			if (!CheckLine ())
			{
				actor->reacttime++;		/* be very ready, but*/
				return;					/* don't attack yet*/
			}
			actor->flags &= ~FL_AMBUSH;
		}

		FirstSighting ();
	}

/**/
/* haven't seen player yet*/
/**/
	if (!madenoise && !CheckLine ())
		return;

	actor->reacttime = 1 + (w_rnd()& classinfo[actor->class].reactionmask);
}


