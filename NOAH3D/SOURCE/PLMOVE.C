/* plmove.c*/
#include "wolfdef.h"

/*
=============================================================================

						 GLOBALS

=============================================================================
*/

void ControlMovement (void);

/*
=============================================================================

						PLAYER MOVEMENT

=============================================================================
*/

/*
===================
=
= TryMove
=
= See if players current position is ok
= returns true if move ok
=
===================
*/

boolean TryMove (void)
{
	ushort		xl,yl,xh,yh,tile;

	xl = (checkx - PLAYERSIZE)>>FRACBITS;
	xh = (checkx + PLAYERSIZE)>>FRACBITS;
	yl = (checky - PLAYERSIZE)>>FRACBITS;
	yh = (checky + PLAYERSIZE)>>FRACBITS;
	
//-------------------------
// Check for solid walls
//-------------------------

	for (mapy=yl;mapy<=yh;mapy++)
		for (mapx=xl;mapx<=xh;mapx++)
		{
			tile = tilemap[mapy][mapx];

#ifndef	ID_VERSION
			if ((tile & TI_SWITCH) || (tile & TI_SECRET))
			{
				PlayLocalSound (SRC_BONUS, GODDBD, FREQ_NORM, VOL_HIGH);
				if (tile & TI_SECRET)
					playstate = EX_SECRET;
				else
					playstate = EX_COMPLETED;
			}
#endif

			if (tile & TI_GETABLE)
				GetBonus ();
			if (tile & (TI_BLOCKMOVE))
				return false;
		}

//-------------------------
// Check for actors
//-------------------------

	yl--;
	yh++;
	xl--;
	xh++;

	for (mapy=yl;mapy<=yh;mapy++)
		for (mapx=xl;mapx<=xh;mapx++)
		{	
			tile = tilemap[mapy][mapx];
			if (!(tile&TI_ACTOR) )
				continue;
			actor = &actors[map.tilemap[mapy][mapx]];
			deltax = w_abs( actor->x - checkx );
			if (deltax < MINACTORDIST)
			{
				deltay = w_abs ( actor->y - checky );
				if (deltay < MINACTORDIST)
					return false;
			}
		}

	playermoving = true;
	return true;
}


/*
===================
=
= ClipMove
=
= Clip player move
=
===================
*/

void ClipMove (void)
{
/**/
/* try complete move*/
/**/
	checkx = actors[0].x + xmove;
	checky = actors[0].y + ymove;
	if (TryMove ())
	{
		actors[0].x = checkx;
		actors[0].y = checky;
		return;
	}

/**/
/* try just horizontal*/
/**/
	checky = actors[0].y;
	if (TryMove ())
	{
		actors[0].x = checkx;
		return;
	}

/**/
/* try just vertical*/
/**/
	checkx = actors[0].x;
	checky = actors[0].y + ymove;
	if (TryMove ())
	{
		actors[0].y = checky;
		return;
	}
}

/*==========================================================================*/


/*
===================
=
= Thrust
=
= Adds movement to xmove / ymove
===================
*/

void Thrust (ushort angle, ushort speed)
{
	angle &= ANGLES-1;
	if (speed >= TILEGLOBAL)
		speed = TILEGLOBAL-1;
		
	xmove += FixedByFrac(speed,costable[angle]);
	ymove += -FixedByFrac(speed,sintable[angle]);
}



/*
=======================
=
= ControlMovement
=
= Changes the player's angle and position
=
=======================
*/

#define TURNSPEED		3
#define FASTTURN		5
#define WALKSPEED		100
#define RUNSPEED		130

void ControlMovement (void)
{
	ushort	turn, move, total, buttons;
	
	playermoving = false;
	xmove = 0;
	ymove = 0;
	
/* if recording a demo, save irqangle, mousey, and buttons*/
	if (demomode == dm_recording)
	{
		*demo_p++ = irqangle>>8;
		*demo_p++ = mousey;
		buttons = 0;
		if (buttonstate[bt_attack])
			buttons |= 1;
		if (buttonstate[bt_use])
			buttons |= 2;
		if (buttonstate[bt_run])
			buttons |= 4;
		if (buttonstate[bt_left])
			buttons |= 8;
		if (buttonstate[bt_right])
			buttons |= 16;
		if (buttonstate[bt_select])
			buttons |= 32;
		if (buttonstate[bt_north])
			buttons |= 64;
		*demo_p++ = buttons;
	}
/* if playing a demo back, fetch the commands*/
	if (demomode == dm_playback)
	{
		irqangle = (*demo_p++) << 8;

		mousey = *(signed char *)demo_p++;

		buttons = *demo_p++;
		buttonstate[bt_attack] = buttons & 1;
		buttonstate[bt_use] = buttons & 2;
		buttonstate[bt_run] = buttons & 4;
		buttonstate[bt_left] = buttons & 8;
		buttonstate[bt_right] = buttons & 16;
		buttonstate[bt_select] = buttons & 32;
		buttonstate[bt_north] = buttons & 64;
		
		if (buttons & 128)	/* end of demo*/
			playstate = EX_COMPLETED;
	}

	if (buttonstate[bt_run])
	{
#ifndef __ORCAC__
		if (buttonstate[bt_north] || buttonstate[bt_south])
			turn = 1;
		else
			turn = FASTTURN;
#endif
		move = RUNSPEED;
	}
	else
	{
#ifndef __ORCAC__
		if (buttonstate[bt_north] || buttonstate[bt_south])
			turn = 1;
		else
			turn = TURNSPEED;
#endif
		move = WALKSPEED;
	}

/**/
/* turning*/
/*	*/
#ifdef __ORCAC__
	viewangle = irqangle>>9;	/* the irq handles angle sampling*/
#else
	if (buttonstate[bt_east])
		viewangle -= turn;
	if (buttonstate[bt_west])
		viewangle += turn;
	viewangle &= ANGLES-1;
#endif
/**/
/* movement*/
/**/
	if (buttonstate[bt_right])
		Thrust (viewangle - ANGLES/4, move>>1);
	if (buttonstate[bt_left])
		Thrust (viewangle + ANGLES/4, move>>1);
		
	total = buttonstate[bt_north] ? move : 0;
	if (mousey < 0)
		total -= mousey<<3;
	if (total)
		Thrust (viewangle, total);

	total = buttonstate[bt_south] ? move : 0;
	if (mousey > 0)
		total += mousey<<3;
	if (total)
		Thrust (viewangle+ANGLES/2, total);
	mousey = 0;
	
	if (xmove || ymove)
	{
		ClipMove();
		tile = tilemap[actors[0].y>>FRACBITS][actors[0].x>>FRACBITS];
		if (!(tile&TI_DOOR) )
			actors[0].areanumber = tile & TI_NUMMASK;
	}
}

