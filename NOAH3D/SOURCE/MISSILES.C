//-------------------------------------------------------------------
// MISSILES.C
//-------------------------------------------------------------------


#include "wolfdef.h"



//===================================================================
//										GLOBALS
//===================================================================

void MoveMissiles (void);
void GetNewMissile (boolean priority);


//===================================================================
//									MISSILE SPAWNING
//===================================================================

void ClearMissiles (void)
{
	for (missileon = 0; missileon < MAXMISSILES; missileon++)
		missile->areanumber == MAXAREAS-1;

	missile = missiles;
	nummissiles = 0;
}


//-------------------------------------------------------------------
// GetNewMissile		Sets missile to a new missile spot to be filled in
//-------------------------------------------------------------------

void GetNewMissile (boolean priority)
{

// Try to reuse an old spot first

	missile = missiles;
	for (missileon = 0; missileon < nummissiles; missileon++, missile++)
		if (missile->areanumber == MAXAREAS-1)
			return;

// Get a new spot			

#ifndef	ID_VERSION
	if (nummissiles == MAXMISSILES-1)
	{
//SHIT//FUCK		if (!priority)
//SHIT//FUCK			return;	// It's just some Zzzs so reuse last spot

		for (missileon=0, missile = missiles; missileon < nummissiles; missileon++, missile++)
			if (missile->type == MI_SLEEPZ)	// Ok to replace Zzzs
				return;

		return;		// Too many missiles, just reuse the last spot
	}
#else
	if (nummissiles == MAXMISSILES-1)
		return;		// Too many missiles, just reuse the last spot
#endif
		
	nummissiles++;
}


//===================================================================
//									MISSILE MOVEMENT
//===================================================================

boolean	explodeit = false;


//-------------------------------------------------------------------
// ExplodeMissile
//-------------------------------------------------------------------

void ExplodeMissile (void)
{
	explodeit = false;
	
	missile->flags = 0;
	switch (missile->type)
	{
		case MI_PMISSILE:
			missile->pic = S_MISSBOOM;
			missile->type = 4;	// tics to stay in explosion
			PlayPosSound (SRC_MISSILE+missileon,
				D_WATHIT, FREQ_NORM, VOL_NORM,
				missile->areanumber, missile->x, missile->y);
			break;
		default:
			missile->pic = S_FIREBOOM;
			missile->type = 2;	// tics to stay in explosion
			PlayPosSound (SRC_MISSILE+missileon,
				D_COCHIT, FREQ_NORM, VOL_NORM,
				missile->areanumber, missile->x, missile->y);
			break;
	}
	missile->x -= missile->xspeed;
	missile->y -= missile->yspeed;
	missile->xspeed = 0;
	missile->yspeed = 0;
}


//-------------------------------------------------------------------
// MissileHitPlayer
//-------------------------------------------------------------------

void MissileHitPlayer (void)
{
	ushort	damage;
	
	damage = ((w_rnd()&7)+1)<<3;
	killeractor = -missileon;
	TakeDamage (damage);
}


//-------------------------------------------------------------------
// MissileHitEnemy
//-------------------------------------------------------------------

void MissileHitEnemy (void)
{
	if (missile->type == MI_PMISSILE)
	{
		DamageActoron ( ((w_rnd()&3)+1)<<4 );
		if ( !(actor->flags & FL_DEAD) )
			explodeit = true;
	}
	else
	{
		DamageActoron ( (w_rnd()&15) + 1);
		if ( !(actor->flags & FL_DEAD) )
			explodeit = true;
	}
}


//-------------------------------------------------------------------
// MissileHitBlock	Can be caused by a solid static or by a real wall
//-------------------------------------------------------------------

void MissileHitBlock (void)
{
// Check for contact with statics
	
	staticp = statics;
	for (staticon=0 ; staticon<numstatics ; staticon++, staticp++)
	{
		if (w_abs(missile->x - staticp->x) > MISSILEHITDIST)
			continue;
		if (w_abs(missile->y - staticp->y) > MISSILEHITDIST)
			continue;

		// Hit a static
	}
	
	explodeit = true;
}


//-------------------------------------------------------------------
// CheckMissileActorHits
//-------------------------------------------------------------------

void CheckMissileActorHits (void)
{
	ushort	xl, xh, yl, yh, x, y;

	xl = (missile->x>>FRACBITS)-1;
	xh = xl+2;
	yl = (missile->y>>FRACBITS)-1;
	yh = yl+2;	
	
	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			tile = tilemap[y][x];
			if (!(tile&TI_ACTOR) )
				continue;
			actor = &actors[map.tilemap[y][x]];
			deltax = w_abs( actor->x - missile->x );
			if (deltax < MISSILEHITDIST)
			{
				deltay = w_abs ( actor->y - missile->y );
				if (deltay < MISSILEHITDIST)
					MissileHitEnemy ();
			}
		}
}


//-------------------------------------------------------------------
// MoveMissiles
//-------------------------------------------------------------------

void MoveMissiles (void)
{
	missile = missiles;
	for (missileon=0 ; missileon < nummissiles ; missileon++, missile++)
	{
		if (missile->areanumber == MAXAREAS-1)
			continue;		// Removed missile


#ifndef	ID_VERSION
		if (missile->type == MI_SLEEPZ)			// Animating sleep Zs
		{
			if (! --missile->flags)					// Time to animate?
			{
				if (missile->pic == S_NONE)		// Restart after empty animation
				{
					missile->x = missile->xspeed;	// Get original position
					missile->y = missile->yspeed;
					missile->pic = S_SLEEPZ0;		// Restart from first frame
				}
				else
				{
					if (missile->pic == S_SLEEPZ2)
						missile->pic = S_NONE;		// Last frame is NOT displayed
					else
					{
						missile->pic++;				// Next frame
						missile->x += (signed) (w_rnd()/8);
						missile->y += (signed) (w_rnd()/8);
					}
				}
				missile->flags = 8;	// Animation speed kludge
			}
			continue;		// Next missile
		}
#endif	//jgt

			
		if (!missile->flags)	// Inert explosion
		{
			if (!--missile->type)
				missile->areanumber = MAXAREAS-1;
			continue;
		}
		
		explodeit = false;
		
	// Move position

		missile->x += missile->xspeed;
		missile->y += missile->yspeed;

	// Check for contact with player		

		if (missile->flags & MF_HITPLAYER)
		{
			if (w_abs(missile->x - actors[0].x) < MISSILEHITDIST
			&& w_abs(missile->y - actors[0].y) < MISSILEHITDIST)
			{
				MissileHitPlayer ();
				ExplodeMissile ();
				continue;
			}			
		}
		
	// Check for contact with actors

		if (missile->flags & MF_HITENEMIES)
		{
			CheckMissileActorHits ();
			if (explodeit)
			{
				ExplodeMissile ();
				continue;
			}
		}
		
	// Check for contact with walls and get new area

		mapx = missile->x >> FRACBITS;
		mapy = missile->y >> FRACBITS;
		tile = tilemap[mapy][mapx];
		if (tile & TI_BLOCKMOVE)
		{
			MissileHitBlock ();		// Can be a solid static or a wall
			if (explodeit)
			{
				ExplodeMissile ();
				continue;
			}
		}
		if (!(tile&TI_DOOR) )		// Doorways don't have real area numbers
			missile->areanumber = tile&TI_NUMMASK;
	}
}





