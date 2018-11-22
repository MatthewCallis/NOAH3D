//-------------------------------------------------------------------
// PLTHINK.C
//-------------------------------------------------------------------

#include "wolfdef.h"


//===================================================================
//										GLOBALS
//===================================================================

void T_Player (void);
void Cmd_ChangeWeapon (void);


//===================================================================
//									PLAYER ACTIONS
//===================================================================


//-------------------------------------------------------------------
// ChangeWeapon
//-------------------------------------------------------------------

void ChangeWeapon (void)
{
	switch (gamestate.weapon)
	{
	case WP_KNIFE:
		break;
		
	case WP_PISTOL:
	case WP_MACHINEGUN:
	case WP_CHAINGUN:
		IO_DrawAmmo (gamestate.ammo);
		break;

	case WP_FLAMETHROWER:
		IO_DrawAmmo (gamestate.gas);
		break;
	case WP_MISSILE:
		IO_DrawAmmo (gamestate.missiles);
		break;
	default:
		IO_Error ("Bad ChangeWeapon");
	}		
}


//-------------------------------------------------------------------
// Cmd_Fire		Switch to knife if no ammo in current weapon
//-------------------------------------------------------------------

void Cmd_Fire (void)
{
	switch (gamestate.weapon)
	{
		case WP_KNIFE:
			break;

		case WP_CHAINGUN:
		case WP_MACHINEGUN:
		case WP_PISTOL:
			if (!gamestate.ammo)
				Cmd_ChangeWeapon ();
			break;
		case WP_FLAMETHROWER:
			if (!gamestate.gas)
				Cmd_ChangeWeapon ();
			break;
		case WP_MISSILE:
			if (!gamestate.missiles)
				Cmd_ChangeWeapon ();
			break;
		default:
			IO_Error ("Bad fire");
	}
	
	gamestate.attackframe = 1;
	gamestate.attackcount = 2;
}


//-------------------------------------------------------------------
// Cmd_Use
//-------------------------------------------------------------------

void Cmd_Use (void)
{
	ushort	dir;

// Find which cardinal direction the player is facing

	mapx = actors[0].x >> FRACBITS;
	mapy = actors[0].y >> FRACBITS;
	
	dir = ((viewangle+ANGLES/8)&(ANGLES-1)) >> 5;

	switch (dir)
	{
		case 0:
			mapx++;
			dir = CD_EAST;
			break;
		case 1:
			mapy--;
			dir = CD_NORTH;
			break;
		case 2:
			mapx--;
			dir = CD_WEST;
			break;
		case 3:
			mapy++;
			dir = CD_SOUTH;
			break;
	}

	tile = tilemap[mapy][mapx];
	
//----------------------------
// Doors
//----------------------------

	if (tile & TI_DOOR)
	{
		dooron = tile & TI_NUMMASK;
		OperateDoor ();
		return;
	}

//----------------------------
// Pushwalls
//----------------------------

	if (!pwallcount && (tile&TI_PUSHWALL))
	{
		pwallx = mapx;
		pwally = mapy;
		pwalldir = dir;
		PushWall ();
		return;
	}

//----------------------------
// Elevator switch
//----------------------------

	if (tile & TI_SWITCH)
	{
		PlayLocalSound (SRC_BONUS, GODDBD, FREQ_LOW, VOL_HIGH);
		playstate = EX_COMPLETED;
		return;
	}
	if (tile & TI_SECRET)
	{
		PlayLocalSound (SRC_BONUS, GODDBD, FREQ_LOW, VOL_HIGH);
		playstate = EX_SECRET;
		return;
	}

	if (tile & TI_BLOCKMOVE)
	{
		PlayLocalSound (SRC_BONUS, D_PUSH, FREQ_LOW, VOL_LOW);
		return;
	}
}



//-------------------------------------------------------------------
// Cmd_ChangeWeapon
//-------------------------------------------------------------------

void Cmd_ChangeWeapon (void)
{
	boolean	wrapped;
	
	wrapped = false;
	                                                                                                                                                                                                                                                                                                                                                                                          
	do
	{
		switch (++gamestate.pendingweapon)
		{
			case WP_PISTOL:
				if (gamestate.machinegun || gamestate.chaingun)
					break;	/* switch up*/
				if (gamestate.ammo)
					return;
				break;
			case WP_MACHINEGUN:
				if (gamestate.chaingun)
					break;	/* switch up*/
				if (gamestate.machinegun && gamestate.ammo)
					return;
				break;
			case WP_CHAINGUN:
				if (gamestate.chaingun && gamestate.ammo)
					return;
				break;
			case WP_FLAMETHROWER:
				if (gamestate.flamethrower && gamestate.gas)
					return;
				break;
			case WP_MISSILE:
				if (gamestate.missile && gamestate.missiles)
					return;
				break;
			default:
				gamestate.pendingweapon = WP_KNIFE;
				if (wrapped)
					return;			// No other working weapon
				wrapped = true;
				break;
		}		
	} while (1);
}


//====================================================================
//									PLAYER CONTROL
//====================================================================


//--------------------------------------------------------------------
// TargetEnemy		Sets actor to the closest enemy in the line of fire,
//						or 0 if there is no valid target
//--------------------------------------------------------------------

extern	ufixed_t	xscale[SCREENWIDTH];	// Filled in by walls, used to Z clip sprites

void TargetEnemy (void)
{
	ushort		*xe;
	vissprite_t	*dseg;

	actor = 0;

// Look at the drawn sprites from closest to farthest

	for (xe=lastevent-1 ; xe >= firstevent ;  xe--)
	{
		dseg = vissprites + (*xe&(MAXVISSPRITES-1));
		if (!dseg->actornum)
			continue;		// Static sprite or missile
		if (xscale[CENTERX] > dseg->clipscale)
			continue;		// Obscured by a wall
		if (dseg->x1 <= CENTERX+8 && dseg->x2 >= CENTERX-8)
		{
			actoron = dseg->actornum;
			actor = &actors[dseg->actornum];
			if (actor->flags & FL_DEAD)
			{
				actor = 0;
				continue;		// Death explosion
			}
			CalcDistance ();
			return;
		}
	}
}


//--------------------------------------------------------------------
// KnifeAttack
//--------------------------------------------------------------------

void KnifeAttack (void)
{
	PlayLocalSound (SRC_WEAPON, D_KNIFE, FREQ_NORM, VOL_NORM);

	TargetEnemy ();

	if (!actor)
		return;
	
	if (distance>KNIFEDIST)
		return;

	DamageActoron (w_rnd() & 3);
}


//--------------------------------------------------------------------
// GunAttack
//--------------------------------------------------------------------

void GunAttack (void)
{
	ushort	sound;
	ushort	damage;
	ushort	freq;
	
	switch (gamestate.weapon)
	{
		case WP_MACHINEGUN:
			sound = D_SLING2;
			freq = FREQ_NORM;
			break;
		case WP_CHAINGUN:
			sound = D_GATLIN;
			freq = FREQ_NORM;
			break;
		default:
			sound = D_GUNSHT;
			freq = FREQ_NORM;
			break;
	}

	PlayLocalSound (SRC_WEAPON, sound, RndFreq(freq), VOL_NORM);
	madenoise = true;
	gamestate.ammo--;
	IO_DrawAmmo (gamestate.ammo);

	TargetEnemy ();

	if (!actor)
		return;

	if (distance<2*TILEGLOBAL)
		damage = w_rnd()&15;
	else
		damage = w_rnd() & 7;

	DamageActoron (damage);
}


//-------------------------------------------------------------------
// FlameAttack
//-------------------------------------------------------------------

void FlameAttack (void)
{
	int	x;
	
	PlayLocalSound (SRC_WEAPON, D_COCTHR, FREQ_LOW, VOL_NORM);
	gamestate.gas--;
	IO_DrawAmmo (gamestate.gas);

//----------------------------
// Flame sprite
//----------------------------

	GetNewMissile (true);	// High priority
	missile->areanumber = actors[0].areanumber;
	missile->x = actors[0].x;
	missile->y = actors[0].y;
	x = costable[viewangle];
	missile->x += x>>2;
#ifdef	ID_VERSION
	missile->xspeed = (x>>1) + (x>>2);
#else
	missile->xspeed = (x>>1) + (x>>2) + (x>>3);
#endif
	x = -sintable[viewangle];
	missile->y += x>>2;
#ifdef	ID_VERSION
	missile->yspeed = (x>>1) + (x>>2);
#else
	missile->yspeed = (x>>1) + (x>>2) + (x>>3);
#endif
	missile->pic = S_FIREBALL;
	missile->flags = MF_HITENEMIES | MF_HITSTATICS;
	missile->type = MI_PFLAME;
	missile->x += missile->xspeed>>2;	// Make sure it is a bit in front
	missile->y += missile->yspeed>>2;	//   of the player
}


//-------------------------------------------------------------------
// MissileAttack
//-------------------------------------------------------------------

void MissileAttack (void)
{
	int	x;
	
	PlayLocalSound (SRC_WEAPON, D_WATTHR, FREQ_NORM, VOL_NORM);
	madenoise = true;
	gamestate.missiles--;
	IO_DrawAmmo (gamestate.missiles);

//----------------------------
// Missile sprite
//----------------------------

	GetNewMissile (true);	// High priority
	missile->x = actors[0].x;
	missile->y = actors[0].y;
	missile->areanumber = actors[0].areanumber;
	x = costable[viewangle];
	missile->x += x>>2;
	missile->xspeed = (x>>1) + (x>>2);
	x = -sintable[viewangle];
	missile->y += x>>2;
	missile->yspeed =  (x>>1) + (x>>2);
	missile->pic = S_MISSILE;
	missile->flags = MF_HITENEMIES | MF_HITSTATICS;
	missile->type = MI_PMISSILE;
}



//-------------------------------------------------------------------
// MovePlayer
//-------------------------------------------------------------------

void MovePlayer (void)
{
	ControlMovement ();

//-------------------------------
// Check for use
//-------------------------------

	if ( buttonstate[bt_use] )
	{
		if (!useheld)
		{
			useheld = true;
			Cmd_Use ();
		}
	}
	else
		useheld = false;

//-------------------------------
// Check for weapon select
//-------------------------------

	if ( buttonstate[bt_select] )
	{
		if ( !selectheld )
		{
			selectheld = true;
			Cmd_ChangeWeapon ();
		}
	}
	else
		selectheld = false;
		
//-------------------------------
// Check for starting an attack
//-------------------------------

	if (buttonstate[bt_attack])
	{
		if ( !attackheld && !gamestate.attackframe)
		{
			attackheld = true;
			Cmd_Fire ();
		}
	}
	else
		attackheld = false;
		
//-------------------------------
// Advance attacking action
//-------------------------------

	if (!gamestate.attackframe)
	{
		if (gamestate.pendingweapon != gamestate.weapon)
		{
			gamestate.weapon = gamestate.pendingweapon;
			ChangeWeapon ();
		}
		return;
	}
		
	if ( (--gamestate.attackcount) > 0)
		return;

//-------------------------------
// Change frame
//-------------------------------

	gamestate.attackcount = 2;
	gamestate.attackframe++;

//-------------------------------
// Frame 2: attack frame
//-------------------------------

	if (gamestate.attackframe == 2)
	{
		switch (gamestate.weapon)
		{
			case WP_KNIFE:
				KnifeAttack ();
				break;
			case WP_PISTOL:
				if (!gamestate.ammo)
				{
					gamestate.attackframe = 4;
					return;
				}
				GunAttack ();
				break;
			case WP_MACHINEGUN:
				if (!gamestate.ammo)
				{
					gamestate.attackframe = 4;
					return;
				}
				GunAttack ();
				break;
			case WP_CHAINGUN:
				if (!gamestate.ammo)
				{
					gamestate.attackframe = 4;
					return;
				}
				GunAttack ();
				break;
			case WP_FLAMETHROWER:
				if (!gamestate.gas)
				{
					gamestate.attackframe = 4;
					return;
				}
				FlameAttack ();
				break;
			case WP_MISSILE:
				if (!gamestate.missiles)
				{
					gamestate.attackframe = 4;
					return;
				}
				MissileAttack ();
				break;
			default:
				;
		}		
	}

//-------------------------------
// Frame 3: 2nd shot for chaingun
//				and flamethrower
//-------------------------------

	if (gamestate.attackframe == 3)
	{
		if ( gamestate.weapon == WP_CHAINGUN )
		{
			if (!gamestate.ammo)
				gamestate.attackframe++;
			else
				GunAttack ();
		}
		else if (gamestate.weapon == WP_FLAMETHROWER ) 
		{
			if (!gamestate.gas)
				gamestate.attackframe++;
			else
				FlameAttack ();
		}

		return;
	}

//-------------------------------
// Frame 4: possible auto fire
//-------------------------------

	if (gamestate.attackframe == 4)
	{
		if (buttonstate[bt_attack])
		{
			if ( ( gamestate.weapon == WP_MACHINEGUN 
			|| gamestate.weapon == WP_CHAINGUN) && gamestate.ammo) 
			{
				gamestate.attackframe = 2;
				GunAttack ();
			}
			if (gamestate.weapon == WP_FLAMETHROWER && gamestate.gas) 
			{
				gamestate.attackframe=2;
				FlameAttack ();	
			}
		}
		return;
	}


	if (gamestate.attackframe == 5)
	{
	// End of attack

		switch (gamestate.weapon)
		{
			case WP_CHAINGUN:
			case WP_MACHINEGUN:
			case WP_PISTOL:
				if (!gamestate.ammo)
					Cmd_ChangeWeapon ();
				else if (gamestate.weapon == WP_PISTOL && buttonstate[bt_attack]
				&& !attackheld)
				{
					gamestate.attackframe = 1;
					return;
				}
				break;
			case WP_FLAMETHROWER:
				if (!gamestate.gas)
					Cmd_ChangeWeapon ();
				break;
			case WP_MISSILE:
				if (!gamestate.missiles)
					Cmd_ChangeWeapon ();
			default:
				break;
		}		
		gamestate.attackcount = 0;
		gamestate.attackframe = 0;
	}
}


