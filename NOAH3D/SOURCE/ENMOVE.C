/* enmove.c*/
#include "wolfdef.h"

/*
=============================================================================

							  GLOBALS

=============================================================================
*/

boolean TryWalk (void);
void NewState (ushort state);
void SelectDodgeDir (void);
void SelectChaseDir (void);
void MoveActoron (ushort move);


/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/


dirtype opposite[9] =
	{west,southwest,south,southeast,east,northeast,north,northwest,nodir};

dirtype diagonal[9][9] =
{
/* east */	{nodir,nodir,northeast,nodir,nodir,nodir,southeast,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
/* north */ {northeast,nodir,nodir,nodir,northwest,nodir,nodir,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
/* west */  {nodir,nodir,northwest,nodir,nodir,nodir,southwest,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
/* south */ {southeast,nodir,nodir,nodir,southwest,nodir,nodir,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir}
};


/*
=============================================================================

						 FUNCTIONS

=============================================================================
*/


/*
===================
=
= NewState
=
= Changes actoron to a new state, setting ticcount to the max for that state
=
===================
*/

void NewState (ushort state)
{
	actor->state = state;
	actor->ticcount = states[state].tictime;
	actor->pic = states[state].shapenum;
}




/*
=============================================================================

				ENEMY TILE WORLD MOVEMENT CODE

=============================================================================
*/

boolean	usedoor;

/*
==================================
=
= TryWalk
=
= Attempts to move actoron in its current (ob->dir) direction.
=
= If blocked by either a wall or an actor returns FALSE
=
= If move is either clear or blocked only by a door, returns TRUE and sets
=
= ob->tilex		= new destination
= ob->tiley
= ob->areanumber  = the floor tile number (0-(MAXAREAS-1)) of destination
= ob->distance  	= TILEGLOBAL, or doornumber if a door is blocking the way
=
= If a door is in the way, an OpenDoor call is made to start it opening.
= The actor code should wait until
= 	doorobjlist[-ob->distance].action = dr_open, meaning the door has been
=	fully opened
=
==================================
*/


boolean CheckDiag(void)
{
	/* anything blocking stops diagonal move*/
	if (tilemap[mapy][mapx]&(TI_BLOCKMOVE|TI_ACTOR))
		return false;
	return true;
}

boolean CheckSide(void)
{
	ushort	temp;

	temp=tilemap[mapy][mapx];

	if (temp & TI_DOOR) 
	{
		if (!(temp&TI_BLOCKMOVE))
			return true;	/* door is open*/
		if (actor->class == CL_DOG)
			return false;	/* dogs can't open doors*/
		usedoor = true;	/* try to open the door*/
		return true;
	}

	if (temp&(TI_BLOCKMOVE|TI_ACTOR))
		return false;

	return true;
}

boolean TryWalk (void)
{
	usedoor = false;

	mapx = actor->goalx;
	mapy = actor->goaly;

	switch (actor->dir)
	{
	case north:
		mapy--;
		if (!CheckSide())
			return false;
		break;

	case northeast:
		mapx++;
		if (!CheckDiag())
			return false;
		mapx--;
		mapy--;
		if (!CheckDiag())
			return false;
		mapx++;
		if (!CheckDiag())
			return false;
		break;

	case east:
		mapx++;
		if (!CheckSide())
			return false;
		break;

	case southeast:
		mapx++;
		if (!CheckDiag())
			return false;
		mapx--;
		mapy++;
		if (!CheckDiag())
			return false;
		mapx++;
		if (!CheckDiag())
			return false;
		break;

	case south:
		mapy++;
		if (!CheckSide())
			return false;
		break;

	case southwest:
		mapx--;
		if (!CheckDiag())
			return false;
		mapx++;
		mapy++;
		if (!CheckDiag())
			return false;
		mapx--;
		if (!CheckDiag())
			return false;
		break;

	case west:
		mapx--;
		if (!CheckSide())
			return false;
		break;

	case northwest:
		mapx--;
		if (!CheckDiag())
			return false;
		mapx++;
		mapy--;
		if (!CheckDiag())
			return false;
		mapx--;
		if (!CheckDiag())
			return false;
		break;

	default:
		IO_Error ("Walk: Bad dir");
	}

	if (usedoor)
	{
		door = &doors[ tilemap[mapy][mapx]&TI_NUMMASK ];
		OpenDoor ();
		actor->flags |= FL_WAITDOOR|FL_NOTMOVING;
		return true;
	}

/**/
/* invalidate the move if moving onto the player*/
/**/
	if (areabyplayer[map.areasoundnum[ actor->areanumber]])
	{
		if ( w_abs(actors[0].x - (mapx>>FRACBITS)) < MINACTORDIST
		&& w_abs (actors[0].x - (mapx>>FRACBITS)) < MINACTORDIST  )
			return false;
	}


/**/
/* the move is ok*/
/**/

/* remove old actor marker*/
	tilemap[actor->goaly][actor->goalx] &= ~TI_ACTOR;
	
/* place new actor marker*/
	tile = tilemap[mapy][mapx];
	tilemap[mapy][mapx] = tile | TI_ACTOR;
	map.tilemap[mapy][mapx] = actoron;
	actor->goalx = mapx;
	actor->goaly = mapy;
	
	if (tile&TI_BLOCKMOVE)
		IO_Error ("Enemy walked into a solid area!");

	if (! (tile&TI_DOOR) )			/* doorways are not areas*/
		actor->areanumber = tile&TI_NUMMASK;
		
	actor->distance = TILEGLOBAL;
	actor->flags &= ~(FL_WAITDOOR|FL_NOTMOVING);

	return true;
}


/*
==================================
=
= SelectDodgeDir
=
= Attempts to choose and initiate a movement for actoron that sends it towards
= the player while dodging
=
==================================
*/

void SelectDodgeDir (void)
{
	short	deltax,deltay,i;
	ushort	absdx,absdy;
	dirtype 	dirtry[5];
	dirtype 	turnaround,tdir;

	turnaround=opposite[actor->dir];

	deltax = actors[0].x - actor->x;
	deltay = actors[0].y - actor->y;

/**/
/* arange 5 direction choices in order of preference*/
/* the four cardinal directions plus the diagonal straight towards*/
/* the player*/
/**/

	if (deltax>0)
	{
		dirtry[1]= east;
		dirtry[3]= west;
	}
	else if (deltax<=0)
	{
		dirtry[1]= west;
		dirtry[3]= east;
	}

	if (deltay>0)
	{
		dirtry[2]= south;
		dirtry[4]= north;
	}
	else if (deltay<=0)
	{
		dirtry[2]= north;
		dirtry[4]= south;
	}

/**/
/* randomize a bit for dodging*/
/**/
	absdx = w_abs(deltax);
	absdy = w_abs(deltay);

	if (absdx > absdy)
	{
		tdir = dirtry[1];
		dirtry[1] = dirtry[2];
		dirtry[2] = tdir;
		tdir = dirtry[3];
		dirtry[3] = dirtry[4];
		dirtry[4] = tdir;
	}

	if (w_rnd() & 1)
	{
		tdir = dirtry[1];
		dirtry[1] = dirtry[2];
		dirtry[2] = tdir;
		tdir = dirtry[3];
		dirtry[3] = dirtry[4];
		dirtry[4] = tdir;
	}

	dirtry[0] = diagonal [ dirtry[1] ] [ dirtry[2] ];

/**/
/* try the directions until one works*/
/**/
	for (i=0;i<5;i++)
	{
		if ( dirtry[i] == nodir || dirtry[i] == turnaround)
			continue;

		actor->dir = dirtry[i];
		if (TryWalk())
			return;
	}

/**/
/* turn around only as a last resort*/
/**/
	if (turnaround != nodir)
	{
		actor->dir = turnaround;

		if (TryWalk())
			return;
	}

	actor->dir = nodir;
	actor->flags |= FL_NOTMOVING;
}


/*
============================
=
= SelectChaseDir
=
= As SelectDodgeDir, but doesn't try to dodge
=
============================
*/

void SelectChaseDir (void)
{
	short deltax,deltay;
	dirtype d[3];
	dirtype tdir, olddir, turnaround;

	olddir = actor->dir;
	turnaround=opposite[olddir];

	deltax = actors[0].x - actor->x;
	deltay = actors[0].y - actor->y;

	d[1]=nodir;
	d[2]=nodir;

	if (deltax>0)
		d[1]= east;
	else if (deltax<0)
		d[1]= west;
	if (deltay>0)
		d[2]=south;
	else if (deltay<0)
		d[2]=north;

	if (w_abs(deltay)>w_abs(deltax))
	{
		tdir=d[1];
		d[1]=d[2];
		d[2]=tdir;
	}

	if (d[1]==turnaround)
		d[1]=nodir;
	if (d[2]==turnaround)
		d[2]=nodir;


	if (d[1]!=nodir)
	{
		actor->dir = d[1];
		if (TryWalk())
			return;     /*either moved forward or attacked*/
	}

	if (d[2]!=nodir)
	{
		actor->dir =d[2];
		if (TryWalk())
			return;
	}

/* there is no direct path to the player, so pick another direction */

	if (olddir!=nodir)
	{
		actor->dir =olddir;
		if (TryWalk())
			return;
	}

	if (w_rnd()&1) 	/*randomly determine direction of search*/
	{
		for (tdir=north;tdir<=west;tdir++)
		{
			if (tdir!=turnaround)
			{
				actor->dir =tdir;
				if ( TryWalk() )
					return;
			}
		}
	}
	else
	{
		for (tdir=west;tdir>=north;tdir--)
		{
			if (tdir!=turnaround)
			{
				actor->dir =tdir;
				if ( TryWalk() )
				return;
			}
		}
	}

	if (turnaround !=  nodir)
	{
		actor->dir =turnaround;
		if ( TryWalk() )
			return;
	}

	actor->dir = nodir;		/* can't move*/
	actor->flags |= FL_NOTMOVING;
}



/*
=================
=
= MoveActoron
=
= Moves actor  <move> global units in actor->dir direction
= Actors are not allowed to move inside the player
= Does NOT check to see if the move is tile map valid
=
=================
*/

void MoveActoron (ushort move)
{
	ushort		delta;
	ushort		tryx,tryy;

	tryx = actor->x;
	tryy = actor->y;

	switch (actor->dir)
	{
	case north:
		tryy -= move;
		break;
	case northeast:
		tryx += move;
		tryy -= move;
		break;
	case east:
		tryx += move;
		break;
	case southeast:
		tryx += move;
		tryy += move;
		break;
	case south:
		tryy += move;
		break;
	case southwest:
		tryx -= move;
		tryy += move;
		break;
	case west:
		tryx -= move;
		break;
	case northwest:
		tryx -= move;
		tryy -= move;
		break;

	default:
		IO_Error ("MoveActoron: bad dir!");
	}

/**/
/* check to make sure it's not moving on top of player*/
/**/
	if (areabyplayer[map.areasoundnum[ actor->areanumber]])
	{
		delta = w_abs(tryx - actors[0].x);
		if (delta < MINACTORDIST)
		{
			delta = w_abs( tryy - actors[0].y );
			if (delta < MINACTORDIST)
				return;
		}
	}
	
	actor->distance-= move;
	actor->x = tryx;
	actor->y = tryy;
}


