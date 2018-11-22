//-------------------------------------------------------------------
// ENTHINK.C
//-------------------------------------------------------------------

#include "wolfdef.h"


//-------------------------------------------------------------------
//										GLOBALS
//-------------------------------------------------------------------

void	T_Chase (void);

void	A_Bite (void);
void	A_Shoot (void);
void	A_Scream (void);
void	A_DeathScream (void);
void	A_HitlerMorph (void);
void	A_MechaSound (void);
void	A_Victory (void);


//-------------------------------------------------------------------
// PlaceItemType		Drops a bonus item at mapx,mapy
//							If there are no free item spots, nothing is done
//-------------------------------------------------------------------

void PlaceItemType (ushort shape)
{

//----------------------------
// Find a spot in statobjlist
// to put it in...
//----------------------------

	for (staticon=0 ; staticon<numstatics ; staticon++)
		if (!statics[staticon].pic)		// 0 is a free spot
			goto placeit;

	if (numstatics==MAXSTATICS-1)
#ifndef	ID_VERSION
//shit//fuck		if (shape == S_G_KEY) {
			IO_Error ("TOO MANY STATICS");	//shit!
//shit//fuck		} else
#else
		return;
#endif

	numstatics++;		// Add it to the end

//----------------------------
// Place it
//----------------------------

placeit:
	statics[staticon].pic = shape;
	statics[staticon].x = (mapx<<FRACBITS)+0x80;
	statics[staticon].y = (mapy<<FRACBITS)+0x80;
	statics[staticon].areanumber = actor->areanumber;
	
	tilemap[mapy][mapx] |= TI_GETABLE;
}


#ifndef	ID_VERSION
//-------------------------------------------------------------------
// Sleep				Places "Z" animation for "sleeping" enemies	(jgt)
//-------------------------------------------------------------------

void Sleep (void)
{

//----------------------------
// Use an inert missile for Zs
//----------------------------

	GetNewMissile (false);			// Low priority
	missile->x = actor->x;
	missile->y = actor->y;
	missile->areanumber = actor->areanumber;
	missile->xspeed = actor->x;	// Save starting position for re-animation
	missile->yspeed = actor->y;
	missile->pic = S_SLEEPZ0;
	missile->flags = 8;				// Animation speed kludge
	missile->type = MI_SLEEPZ;
}
#endif	//jgt


//-------------------------------------------------------------------
// KillActoron			Kills actoron
//-------------------------------------------------------------------

void KillActoron (void)
{

//-----------------------------------
// Drop bonus items on closest tile,
// rather than goal tile (unless
// it's a closing door)
//-----------------------------------

	mapx = actor->x >> FRACBITS;
	mapy = actor->y >> FRACBITS;
	tile = tilemap[mapy][mapx];
	if ( (tile&TI_BLOCKMOVE) && !(tile &TI_ACTOR) )
	{
		mapx = actor->goalx;
		mapy = actor->goaly;
	}
	GivePoints (classinfo[actor->class].points);

	switch (actor->class)
	{
		case CL_OFFICER:
		case CL_MUTANT:
		case CL_GUARD:
			PlaceItemType (S_AMMO);
			break;

		case CL_SS:
			if (!gamestate.machinegun && !gamestate.chaingun)
				PlaceItemType (S_MACHINEGUN);
			else
				PlaceItemType (S_AMMO);
			break;

		case CL_HANS:
		case CL_SCHABBS:
		case CL_TRANS:
		case CL_UBER:
		case CL_DKNIGHT:
			PlaceItemType (S_G_KEY);
			break;
	}

	gamestate.killcount++;
	actor->flags = FL_DEAD;		// Remove old actor marker
	tilemap[actor->goaly][actor->goalx] &= ~TI_ACTOR;
	mapx = actor->x >> FRACBITS;
	mapy = actor->y >> FRACBITS;
	tilemap[mapy][mapx] |= TI_BODY;	// Body flag on most apparent, no matter what

//----------------------------
// Start the death animation
//----------------------------

	NewState ( classinfo[actor->class].deathstate );
#ifndef	ID_VERSION
	Sleep ();	//jgt
#endif
}



//-------------------------------------------------------------------
// DamageActoron		Called when the player succesfully hits an enemy
//							Does damage points to enemy actoron, either
//							putting it into a stun frame or killing it.
//-------------------------------------------------------------------

void DamageActoron (ushort damage)
{
	ushort	pain;
	
	madenoise = true;

//----------------------------
// Do 2x damage if shooting a
// non-attack mode actor
//----------------------------

	if ( !(actor->flags & FL_ACTIVE) )
	{
		damage <<= 1;
		FirstSighting ();		// Put into combat mode
	}

	if (damage >= actor->hitpoints)
	{
		KillActoron ();
		return;
	}
	
	actor->hitpoints -= damage;
	if (actor->class == CL_MECHAHITLER && actor->hitpoints <= 250 && actor->hitpoints+damage > 250)
	{	// Hitler losing armor
		PlayPosSound (SRC_ACTOR+actoron,	D_BEAR, FREQ_LOW, VOL_NORM,
			actor->areanumber, actor->x, actor->y);
		pain = ST_MHITLER_DIE1;
	}
	else
	{
		PlayPosSound (SRC_ACTOR+actoron, D_EAT, FREQ_NORM, VOL_NORM,
			actor->areanumber, actor->x, actor->y);
		pain = classinfo[actor->class].painstate;
	}
	if (pain)		// Some classes don't have pain frames
		NewState (pain);
}


//===================================================================
//							ACTION ROUTINES
//===================================================================


//-------------------------------------------------------------------
// A_Throw
//-------------------------------------------------------------------

void	A_Throw (void)
{
	ushort	angle;
	short	speed;
	
	PlayPosSound (SRC_ACTOR+actoron,	D_WATTHR, FREQ_NORM, VOL_NORM,
		actor->areanumber, actor->x, actor->y);
	GetNewMissile (true);	// High priority
	missile->x = actor->x;
	missile->y = actor->y;
	missile->areanumber = actor->areanumber;

//----------------------------
// Get direction from player
// to boss
//----------------------------

	angle = PointToAngle (actor->x, actor->y);
	angle >>= SHORTTOANGLESHIFT;
	speed = costable[angle];
	speed = (speed>>1)+(speed>>2);
	missile->xspeed = -speed;
	speed = sintable[angle];
	speed = (speed>>1)+(speed>>2);
	missile->yspeed = speed;

	missile->pic = S_NEEDLE;
	missile->flags = MF_HITPLAYER | MF_HITSTATICS;
	missile->type = MI_NEEDLE;
}


//-------------------------------------------------------------------
// A_Launch
//-------------------------------------------------------------------

void	A_Launch (void)
{
	ushort	angle;
	short	speed;
	
	PlayPosSound (SRC_ACTOR+actoron,	D_COCTHR, FREQ_NORM, VOL_NORM,
		actor->areanumber, actor->x, actor->y);
	GetNewMissile (true);	// High priority
	missile->x = actor->x;
	missile->y = actor->y;
	missile->areanumber = actor->areanumber;

//----------------------------
// Get direction from player
// to boss
//----------------------------

	angle = PointToAngle (actor->x, actor->y);
	angle >>= SHORTTOANGLESHIFT;
	speed = costable[angle];
	speed = (speed>>1)+(speed>>2);
	missile->xspeed = -speed;
	speed = sintable[angle];
	speed = (speed>>1)+(speed>>2);
	missile->yspeed = speed;

	missile->pic = S_ENMISSILE;
	missile->flags = MF_HITPLAYER | MF_HITSTATICS;
	missile->type = MI_EMISSILE;

//----------------------------
// Also shoot a bullet
//----------------------------

	A_Shoot ();
}


//-------------------------------------------------------------------
// A_Scream
//-------------------------------------------------------------------

void A_Scream (void)
{
	PlayPosSound (SRC_ACTOR+actoron,
		classinfo[actor->class].deathsound, RndFreq(classinfo[actor->class].deathfreq),
		VOL_NORM, actor->areanumber, actor->x, actor->y);
}


//-------------------------------------------------------------------
// A_Victory
//-------------------------------------------------------------------

void A_Victory (void)
{
	playstate = EX_COMPLETED;
}


//-------------------------------------------------------------------
// A_HitlerMorph
//-------------------------------------------------------------------

void A_HitlerMorph (void)
{
//----------------------------
// Use an inert missile for
// the armor remnants
//----------------------------

	GetNewMissile (true);	// High priority
	missile->x = actor->x;
	missile->y = actor->y;
	missile->areanumber = actor->areanumber;
	missile->xspeed = 0;
	missile->yspeed = 0;
	missile->pic = S_MHITLER_DIE4;
	missile->flags = 0;
	missile->type = 0;
	
	actor->class = CL_HITLER;
	actor->speed = 40;		// Faster without armor
}


//-------------------------------------------------------------------
// A_Shoot				Try to damage the player, based on skill level
//							and player's speed
//-------------------------------------------------------------------

void A_Shoot (void)
{
	ushort		damage;

	if (!areabyplayer[map.areasoundnum[ actor->areanumber]])
		return;

	madenoise = true;

#ifndef	ID_VERSION
	if (actor->class == CL_OFFICER)
		PlayPosSound (SRC_ACTOR+actoron,	D_SPIT, RndFreq(FREQ_HIGH),	VOL_NORM,
			actor->areanumber, actor->x, actor->y);
	else if (actor->class == CL_MUTANT)
		PlayPosSound (SRC_ACTOR+actoron,	D_SPIT, RndFreq(FREQ_LOW),	VOL_NORM,
			actor->areanumber, actor->x, actor->y);
	else
#endif
	PlayPosSound (SRC_ACTOR+actoron,	D_SPIT, RndFreq(FREQ_NORM),	VOL_NORM,
		actor->areanumber, actor->x, actor->y);

	if (!CheckLine ())		// Player is behind a wall
	  return;

	CalcDistance ();

	if (distance >= 16*TILEGLOBAL)	// Too far away
		return;

	distance >>= 4;			// 0-255 range

	if (actor->class == CL_OFFICER || actor->class >= CL_HANS)	// Better shots
	{
		if (distance < 16)
			distance = 0;
		else
			distance -= 16;
	}

	if (playermoving)			// Harder to hit when moving
	{
		if (distance >= 224)
			return;
		distance += 32;
	}

//----------------------------
// See if the shot was a hit
//----------------------------

	if (w_rnd()>distance)
	{
		switch (difficulty)
		{
		case 0:
			damage = (w_rnd()&3)+1;
			break;
		case 1:
			damage = (w_rnd()&7)+1;
			break;
		case 2:
			damage = (w_rnd()&7)+3;
			break;
		}
		if (distance<32)
			damage <<= 2;
		else if (distance<64)
			damage <<= 1;

		killeractor = actoron;
		TakeDamage (damage);
	}
}


//-------------------------------------------------------------------
// A_Bite
//-------------------------------------------------------------------

void A_Bite (void)
{
	ushort	dmg;
	
	PlayPosSound (SRC_ACTOR+actoron,	D_DBITE, FREQ_NORM, VOL_NORM,
		actor->areanumber, actor->x, actor->y);

	CalcDistance ();
	if (distance > BITERANGE)
		return;

	switch (difficulty)
	{
	case 0:
		dmg = (w_rnd()&3)+3;
		break;
	case 1:
		dmg = (w_rnd()&7)+3;
		break;
	case 2:
		dmg = (w_rnd()&15)+4;
		break;
	}
		
	killeractor = actoron;
	TakeDamage ( dmg );
}


//===================================================================
//								THINKING
//===================================================================


//-------------------------------------------------------------------
// CalcDistance
//-------------------------------------------------------------------

void CalcDistance (void)
{
	ushort	absdx;
	ushort	absdy;
	
	deltax = actor->x - actors[0].x;
	deltay = actor->y - actors[0].y;

	absdx = w_abs(deltax);
	absdy = w_abs(deltay);

	distance = absdx > absdy ? absdx : absdy;
}


//-------------------------------------------------------------------
// A_Target				Called every few frames to check for sighting
//							and attacking the player
//-------------------------------------------------------------------

ushort	shootchance[8] = 	{256,64,32,24,20,16,12,8};

void A_Target (void)
{
	ushort	chance;
	
	if (!areabyplayer[map.areasoundnum[ actor->areanumber]] || !CheckLine ())
	{
		actor->flags &= ~FL_SEEPLAYER;
		return;
	}
	
	actor->flags |= FL_SEEPLAYER;
	
	CalcDistance ();

	if (distance >= TILEGLOBAL*8)
		return;
		
	if (distance < BITERANGE)		// Always attack when this close
		goto attack;

	if (actor->class == CL_SCHABBS && distance <= TILEGLOBAL*4)
		goto attack;
		
	if (actor->class == CL_DOG)
		return;
		
	chance = shootchance[distance>>FRACBITS] ;
	if (difficulty == 2)
		chance <<= 1;

	if ( w_rnd() < chance)
	{
//----------------------------
// Go into attack frame
//----------------------------
attack:
		NewState (classinfo[actor->class].attackstate);
	}
}


//-------------------------------------------------------------------
// A_MechStep
//-------------------------------------------------------------------

void A_MechStep (void)
{
	PlayPosSound (SRC_ACTOR+actoron,	D_MSTEP, FREQ_NORM, VOL_NORM,
		actor->areanumber, actor->x, actor->y);
	A_Target ();
}


//-------------------------------------------------------------------
// T_Chase
//-------------------------------------------------------------------

void T_Chase (void)
{
//----------------------------
// If still centered in a
// tile, try to find a move
//----------------------------

	if (actor->flags&FL_NOTMOVING)
	{
		if (actor->flags&FL_WAITDOOR)
			TryWalk ();		// Waiting for a door to open
		else if (actor->flags & FL_SEEPLAYER)
			SelectDodgeDir ();
		else
			SelectChaseDir ();

		if (actor->flags & FL_NOTMOVING)
			return;			// Still blocked in
	}


// OPTIMIZE: integral steps / tile movement

//----------------------------
// Cover some distance
//----------------------------

	move = actor->speed;		// This could be put in the class info array

	while (move)
	{
		if (move < actor->distance)
		{
			MoveActoron (move);
			return;
		}

	//----------------------------
	// Reached goal tile, so
	// select another one
	//----------------------------

		move -= actor->distance;
		MoveActoron (actor->distance);	// Move the last 1 to center

		if (actor->flags & FL_SEEPLAYER)
			SelectDodgeDir ();
		else
			SelectChaseDir ();

		if (actor->flags & FL_NOTMOVING)
			return;				// Object is blocked in
	}
}


//===================================================================
//									ACTOR LOOP
//===================================================================

typedef void (*call_t)(void);

call_t thinkcalls[] =
{
	0,
	T_Stand,
	T_Chase,
	0
};

call_t actioncalls[] =
{
	0,
	A_Target,
	A_Shoot,
	A_Bite,
	A_Throw,
	A_Launch,
	A_HitlerMorph,
	A_MechStep,
	A_Victory,
	A_Scream,
	0
};


//-------------------------------------------------------------------
// MoveActors			Actions are performed as the state is entered
//-------------------------------------------------------------------

void MoveActors (void)
{
	for (actoron = 1, actor=&actors[1]; actoron<numactors; actoron++, actor++)
	{
		if (actor->areanumber == MAXAREAS-1)
			continue;		// Dead actor
		if (!(actor->flags&FL_ACTIVE) 
				&& !areabyplayer[map.areasoundnum[actor->areanumber]])
			continue;

		actor->ticcount--;
		state = &states[actor->state];

	//----------------------------
	// Change state if time
	//----------------------------

		if (!actor->ticcount)
		{
			actor->state = state->next;
			state = &states[actor->state];
			actor->ticcount = state->tictime;
			actor->pic = state->shapenum;

		//----------------------------
		// Action think
		//----------------------------

			think = state->action;
			if (think)
				actioncalls[think] ();
		}

	//----------------------------
	// Think
	//----------------------------

		think = state->think;
		if (think)
			thinkcalls[think] ();
	}
}

