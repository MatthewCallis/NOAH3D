/* pushwall.c*/
#include "wolfdef.h"

/*
=============================================================================

							 GLOBALS

=============================================================================
*/

void MovePWalls (void);
void PushWall (void);


/*
=============================================================================

						PUSHABLE WALLS

=============================================================================
*/


#define PWALLSPEED	16

/*
===============
=
= PushWallOne
=
= Uses pwallx,pwally,pwalldir
=
===============
*/

boolean PushWallOne (void)
{
	pwallcheckx = pwallx;
	pwallchecky = pwally;

	switch (pwalldir)
	{
	case CD_NORTH:
		pwallchecky--;
		break;

	case CD_EAST:
		pwallcheckx++;
		break;

	case CD_SOUTH:
		pwallchecky++;
		break;

	case CD_WEST:
		pwallcheckx--;
		break;
	}

#if 0
	if (tilemap[pwallchecky][pwallcheckx] & TI_BLOCKMOVE)
		return false;
#endif

	tilemap[pwallchecky][pwallcheckx] |= TI_BLOCKMOVE | TI_BLOCKSIGHT;

	StartPushWall ();	/* let the refresh do some junk*/
	return true;
}


void SetPWallChange (void)
{
	ushort	pos;
	
	pos = pwallpos&(FRACUNIT-1);
	
	switch (pwalldir)
	{
	case CD_NORTH:
		pwallychange = -pos;
		break;

	case CD_EAST:
		pwallxchange = pos;
		break;

	case CD_SOUTH:
		pwallychange = pos;
		break;

	case CD_WEST:
		pwallxchange = -pos;
		break;
	}
}


/*
===============
=
= PushWall
=
= Call with pwallx,pwally,pwalldir set
=
===============
*/

void PushWall (void)
{
	
	if (!PushWallOne ())
		return;			/* pwall movement is blocked*/

	pwallcount = 2;
	gamestate.secretcount++;
	pwallpos = PWALLSPEED/2;

/**/
/* mark the segs that are being moved*/
/**/
	tilemap[mapy][mapx] &= ~TI_PUSHWALL;
	pwallxchange = 0;
	pwallychange = 0;
	SetPWallChange ();
}



/*
=================
=
= MovePWalls
=
=================
*/

void MovePWalls (void)
{
	if (!pwallcount)
		return;

	PlayPosSound (SRC_PWALL, D_PWALL, FREQ_NORM, VOL_LOW,	actors[0].areanumber,
		(pwallx<<FRACBITS)+FRACUNIT/2, (pwally<<FRACBITS)+FRACUNIT/2);
	
	pwallpos += PWALLSPEED;
	SetPWallChange ();
	
	if (pwallpos <256)
		return;
	pwallpos -= 256;
	
	/**/
	/* the tile can now be walked into*/
	/**/
	tilemap[pwally][pwallx] &= ~TI_BLOCKMOVE;
	AdvancePushWall ();		/* remove the bsp seg*/

	if (!--pwallcount)			/* been pushed 2 blocks?*/
		return;

	pwallx = pwallcheckx;
	pwally = pwallchecky;

	PushWallOne ();
}

