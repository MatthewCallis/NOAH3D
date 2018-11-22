/* doors.c*/
#include "wolfdef.h"

#define DOORSPEED	(TILEGLOBAL/8)

/*
=============================================================================

							 GLOBALS

=============================================================================
*/

void OperateDoor (void);
void OpenDoor (void);
void MoveDoors (void);


/*
=============================================================================

							DOORS

dr_position[] holds the amount the door is open, ranging from 0 to 0xff

The number of doors is limited to 64 because various fields are only 6 bits

Open doors conect two areas, so sounds will travel between them and sight
	will be checked when the player is in a connected area.

Areaconnect is incremented/decremented by each door. If >0 they connect

Every time a door opens or closes the areabyplayer matrix gets recalculated.
	An area is true if it connects with the player's current spor.

=============================================================================
*/

#define OPENTICS	50


/*
==============
=
= ConnectAreas
=
= Scans outward from playerarea, marking all connected areas
=
==============
*/

void RecursiveConnect (int areanumber)
{
	int	i;

	areabyplayer[areanumber] = true;
	
	for (i=0;i<MAXAREAS;i++)
	{
		if (areaconnect[areanumber][i] && !areabyplayer[i])
			RecursiveConnect (i);
	}
}


void ConnectAreas (void)
{
	w_memset(areabyplayer,(char)0,sizeof(areabyplayer));
	RecursiveConnect(map.areasoundnum[ actors[0].areanumber]);
}


/*===========================================================================*/

/*
=====================
=
= OpenDoor
=
= Call with door set
=
=====================
*/

void OpenDoor (void)
{
	if (door->action  == DR_OPEN)
		door->ticcount = 0;			/* reset open time*/
	else
		door->action = DR_OPENING;	/* start it opening*/
		
/* the door will be made passable when it is totally open */
}


/*
=====================
=
= CloseDoor
=
= Call with door set
=
=====================
*/

void CloseDoor (void)
{
	ushort	tile, tilex,tiley,area;
	short	delta;

//----------------------------------------
// Don't close on anything solid
//----------------------------------------

	tilex = door->tilex;
	tiley = door->tiley;

	if (door->action == DR_OPENING)
		goto closedoor;

//----------------------------------------
// Don't close on an actor or bonus item
//----------------------------------------

	tile = tilemap[tiley][tilex];
	if (tile & TI_BODY)
	{
		door->action = DR_WEDGEDOPEN;	// Bodies never go away
		return;
	}
	
	if (tile & (TI_ACTOR | TI_GETABLE) )
	{
		door->ticcount = 0;		// Wait a while before trying to close again
		return;
	}
	
//----------------------------------------
// Don't close on the player
//----------------------------------------

	delta = actors[0].x - ((tilex<<8)+0x80);
	if (w_abs(delta) <= 0x82+PLAYERSIZE)
	{
		delta = actors[0].y - ((tiley<<8)+0x80);
		if (w_abs(delta) <= 0x82+PLAYERSIZE)
			return;
	}
		
//----------------------------------------
// Play door sound if in a connected area
//----------------------------------------

closedoor:

#ifndef	ID_VERSION
		PlayPosSound (SRC_DOOR, D_CLOSDR, FREQ_NORM, VOL_NORM,
			door->area1, (door->tilex<<FRACBITS)+FRACUNIT/2,
			(door->tiley<<FRACBITS)+FRACUNIT/2);
#endif

	area = door->area1;

	door->action = DR_CLOSING;

//----------------------------------------
// Make the door space a solid tile
//----------------------------------------

	tilemap[tiley][tilex] |= TI_BLOCKMOVE;
}



/*
=====================
=
= OperateDoor
=
= The player wants to change the door's direction
= Call with dooron set
=
=====================
*/

void OperateDoor (void)
{
	ushort	type;
	
	door = &doors[dooron];
	type = door->info>>1;
	
	if ( ( type==1 && !(gamestate.keys&1)) || (type==2 && !(gamestate.keys&2)) )
	{
		PlayLocalSound (SRC_PLAYER, GODDBD, FREQ_NORM, VOL_NORM);
		return;
	}

	switch (door->action)
	{
	case DR_CLOSED:
	case DR_CLOSING:
		OpenDoor ();
		break;
	case DR_OPEN:
	case DR_OPENING:
		CloseDoor ();
		break;
	}
}


/*===========================================================================*/

/*
===============
=
= DoorOpen
=
= Close the door after a few seconds
=
===============
*/

void DoorOpen (void)
{
	door->ticcount++;
	if ( door->ticcount == OPENTICS)
	{
		door->ticcount = OPENTICS-1;	/* so if the door can't close it will keep trying*/
		CloseDoor ();
	}
}



/*
===============
=
= DoorOpening
=
===============
*/

void DoorOpening (void)
{
	unsigned	position;

	position = door->position;

	if (!position)
	{
	/**/
	/* door is just starting to open, so connect the areas*/
	/**/
		area1 = map.areasoundnum[ door->area1];
		area2 = map.areasoundnum[ door->area2];

		areaconnect[area1][area2]++;
		areaconnect[area2][area1]++;
		if (areabyplayer[area1] || areabyplayer[area2])
		{
			ConnectAreas ();
			PlayPosSound (SRC_DOOR, D_OPENDR, FREQ_NORM, VOL_NORM,
				door->area1, (door->tilex<<FRACBITS)+FRACUNIT/2,
				(door->tiley<<FRACBITS)+FRACUNIT/2);
		}
	}

/**/
/* slide the door open a bit*/
/**/
	position += DOORSPEED;
	if (position >= TILEGLOBAL-1)
	{
	/**/
	/* door is all the way open*/
	/**/
		position = TILEGLOBAL-1;
		door->ticcount = 0;
		door->action = DR_OPEN;

		mapx = door->tilex;
		mapy = door->tiley;
		tilemap[mapy][mapx] &= ~(TI_BLOCKMOVE);
	}

	door->position = position;
}


/*
===============
=
= DoorClosing
=
===============
*/

void DoorClosing (void)
{
	int		position;

	position = door->position;

/**/
/* slide the door by an adaptive amount*/
/**/
	position -= DOORSPEED;

	if (position <= 0)
	{
	/**/
	/* door is closed all the way, so disconnect the areas*/
	/**/
		position = 0;

		area1 = map.areasoundnum[ door->area1];
		area2 = map.areasoundnum[ door->area2];

		areaconnect[area1][area2]--;
		areaconnect[area2][area1]--;

		if (areabyplayer[area1] || areabyplayer[area2])
			ConnectAreas ();
		door->action = DR_CLOSED;
	}

	door->position = position;
}




/*
=====================
=
= MoveDoors
=
= Called from PlayLoop
=
=====================
*/

void MoveDoors (void)
{
	for (dooron = 0, door=doors ; dooron < numdoors ; dooron++, door++)
		switch (door->action)
		{
		case DR_OPEN:
			DoorOpen ();
			break;

		case DR_OPENING:
			DoorOpening();
			break;

		case DR_CLOSING:
			DoorClosing();
			break;

		case DR_CLOSED:
			break;
		}
}

