/* plstuff.c*/
#include "wolfdef.h"

/*
=============================================================================

						 PUBLICS

=============================================================================
*/

void	TakeDamage (ushort points);
void	GivePoints (ushort points);
void	GetBonus (void);


/*
=============================================================================

						INVENTORY STUFF

=============================================================================
*/

void	GiveAmmo (ushort ammo);

/*
===============
=
= TakeDamage
=
= Actoron is doing damage to player
=
===============
*/

void	TakeDamage (ushort points)
{
	short	angle;
	
	StartDamageFlash (points);
	if (godmode)
		return;
		
	if (gamestate.health <= points)
	{
		playstate = EX_DIED;
		gamestate.health = 0;
	}
	else
	{
		PlayLocalSound (SRC_PLAYER, D_PAIN, FREQ_NORM, VOL_HIGH);
		gamestate.health -= points;
	}

	IO_DrawHealth (gamestate.health);

/* change face to look at attacker*/

	angle = PointToAngle (actor->x, actor->y) - (viewangle<<SHORTTOANGLESHIFT);
	if (angle > 0)
	{
		if (angle > 0x2000)
			faceframe = 3;
		else
			faceframe = 1;
	}
	else
	{
		if (angle < -0x2000)
			faceframe = 2;
		else
			faceframe = 0;
	}
	if (gamestate.health <= 25)
		faceframe += 5;			/* use beat up frames*/
	IO_DrawFace (faceframe);

	facecount = 90;
}


/*
===============
=
= HealSelf
=
===============
*/

void	HealSelf (ushort points)
{
	gamestate.health += points;
	if (gamestate.health>100)
		gamestate.health = 100;

	IO_DrawHealth (gamestate.health);
	if (gamestate.health <= 25)
		faceframe = 5;			/* use beat up frames*/
	else
		faceframe = 0;
	IO_DrawFace (faceframe);

}


/*
===============
=
= GiveExtraMan
=
===============
*/

void	GiveExtraMan (void)
{
	if (gamestate.lives<10)
		gamestate.lives++;
	PlayLocalSound (SRC_BONUS, D_EXTRA, FREQ_NORM, VOL_NORM);
	IO_DrawLives (gamestate.lives);
}


/*
===============
=
= GivePoints
=
===============
*/

void	GivePoints (ushort points)
{
	gamestate.score += points;

#ifndef	ID_VERSION
	if (gamestate.score > MAX_SCORE)
		gamestate.score = MAX_SCORE;
#endif

	while (gamestate.score >= gamestate.nextextra)
	{
		gamestate.nextextra += EXTRAPOINTS;
		GiveExtraMan ();
	}
	IO_DrawScore (gamestate.score);
}


/*
===============
=
= GiveTreasure
=
===============
*/

void	GiveTreasure (ushort treasure)
{
	gamestate.treasure += treasure;
	while (gamestate.treasure >= 50)
	{
		gamestate.treasure -= 50;
		GiveExtraMan ();
	}
	IO_DrawTreasure (gamestate.treasure);
}



/*
==================
=
= GiveWeapon
=
==================
*/

void GiveWeapon (ushort weapon)
{
	GiveAmmo (6);

	if (gamestate.pendingweapon < weapon)
		gamestate.pendingweapon = weapon;
}



/*
===============
=
= GiveAmmo
=
===============
*/

void	GiveAmmo (ushort ammo)
{
	gamestate.ammo += ammo;
	if (gamestate.ammo > gamestate.maxammo)
		gamestate.ammo = gamestate.maxammo;
	if (gamestate.weapon == WP_PISTOL || gamestate.weapon == WP_MACHINEGUN
	|| gamestate.weapon == WP_CHAINGUN)
		IO_DrawAmmo (gamestate.ammo);
	else	if (gamestate.weapon == WP_KNIFE)
	{
		if (gamestate.chaingun)
			gamestate.pendingweapon = WP_CHAINGUN;
		else if (gamestate.machinegun)
			gamestate.pendingweapon = WP_MACHINEGUN;
		else
			gamestate.pendingweapon = WP_PISTOL;
	}
}

/*
===============
=
= GiveGas
=
===============
*/

void	GiveGas (ushort ammo)
{
	gamestate.gas += ammo;
	if (gamestate.gas > 99)
		gamestate.gas = 99;
	if (gamestate.weapon == WP_FLAMETHROWER)		IO_DrawAmmo (gamestate.gas);
	else	if (gamestate.weapon == WP_KNIFE && gamestate.flamethrower)
		gamestate.pendingweapon = WP_FLAMETHROWER;
}


/*
===============
=
= GiveMissile
=
===============
*/

void	GiveMissile (ushort ammo)
{
	gamestate.missiles += ammo;
	if (gamestate.missiles > 99)
		gamestate.missiles = 99;
	if (gamestate.weapon == WP_MISSILE)
		IO_DrawAmmo (gamestate.missiles);
	else	if (gamestate.weapon == WP_KNIFE && gamestate.missile)
		gamestate.pendingweapon = WP_MISSILE;
}



/*
==================
=
= GiveKey
=
==================
*/

void GiveKey (ushort key)
{
	gamestate.keys |= (1<<key);
	IO_DrawKeys (gamestate.keys);
}


void BonusSound (void)
{
	PlayLocalSound (SRC_BONUS, D_BONUS, FREQ_NORM, VOL_NORM);
}

void WeaponSound (void)
{
	PlayLocalSound (SRC_BONUS, D_BONUS, FREQ_LOW, VOL_HIGH);
}

void HealthSound (void)
{
	PlayLocalSound (SRC_BONUS, D_BONUS, FREQ_HIGH, VOL_HIGH);
}

void KeySound (void)
{
	PlayLocalSound (SRC_BONUS, D_KEY, FREQ_HIGH, VOL_HIGH);
}

/*
===================
=
= GetBonus
=
= picks up the bonus item at mapx,mapy
= it is possible to have multiple items in a single tile, and they may not all get picked up (maxed out)
===================
*/

void GetBonus (void)
{
	static_t	*stat;
	int		i;
	ushort	touched, got;
	
	touched = 0;
	got = 0;
	
	for (i=0, stat=statics ; i<numstatics ; i++,stat++)
	{
		if (stat->x>>FRACBITS != mapx || stat->y>>FRACBITS != mapy)
			continue;

		touched++;
		got++;
		switch (stat->pic)
		{
#ifndef	ID_VERSION
		case	S_MAP:
			BonusSound ();
			gamestate.automap = true;
			break;
#endif
		case	S_HEALTH:
			if (gamestate.health == 100)
			{
				got--;
				goto skipremove;
			}
			HealthSound ();
			HealSelf (25);
			break;
	
		case	S_G_KEY:
		case	S_S_KEY:
			GiveKey (stat->pic - S_G_KEY);
			KeySound ();
			break;
	
		case	S_CROSS:
		case	S_CHALICE:
		case	S_CHEST:
		case	S_CROWN:
			BonusSound ();
			GiveTreasure (1);
			gamestate.treasurecount++;
			break;
	
		case	S_AMMO:
			if (gamestate.ammo == gamestate.maxammo)
			{
				got--;
				goto skipremove;
			}
			BonusSound ();
			GiveAmmo (5);
			break;
	
		case	S_MISSILES:
			if (gamestate.missiles == 99)
			{
				got--;
				goto skipremove;
			}
			BonusSound ();
			GiveMissile (5);
			break;
	
		case	S_GASCAN:
			if (gamestate.gas == 99)
			{
				got--;
				goto skipremove;
			}
			BonusSound ();
			GiveGas (14);
			break;
	
		case	S_AMMOCASE:
			if (gamestate.ammo == gamestate.maxammo)
			{
				got--;
				goto skipremove;
			}
			BonusSound ();
			GiveAmmo (25);
			break;
	
		case	S_MACHINEGUN:
			WeaponSound ();
			gamestate.machinegun = true;
			GiveWeapon (WP_MACHINEGUN);
			break;
			
		case	S_CHAINGUN:
			WeaponSound ();
			gamestate.chaingun = true;
			GiveWeapon (WP_CHAINGUN);
			IO_DrawFace (4);

			facecount = 32;
			break;
	
		case	S_FLAMETHROWER:
			WeaponSound ();
			gamestate.flamethrower = true;
			GiveWeapon (WP_FLAMETHROWER);
			GiveGas (20);
			IO_DrawFace (4);

			facecount = 32;
			break;
	
		case	S_LAUNCHER:
			WeaponSound ();
			gamestate.missile = true;
			GiveWeapon (WP_MISSILE);
			GiveMissile (5);
			IO_DrawFace (4);

			facecount = 32;
			break;
	
		case	S_ONEUP:
			HealSelf (99);
			GiveExtraMan ();
			gamestate.treasurecount++;
			break;
	
		case	S_FOOD:
			if (gamestate.health == 100)
			{
				got--;
				goto skipremove;
			}
			HealthSound ();
			HealSelf (10);
			break;
	
		case	S_DOG_FOOD:
			if (gamestate.health == 100)
			{
				got--;
				goto skipremove;
			}
			BonusSound ();
			HealSelf (4);
			break;
	
		case	S_BANDOLIER:
#ifdef	ID_VERSION
			if (gamestate.maxammo == 299)
			{
				got--;
				goto skipremove;
			}
			gamestate.maxammo += 100;
#else
			gamestate.maxammo += 100;
			if (gamestate.maxammo > 299)
				gamestate.maxammo = 299;

			GiveAmmo (10);		// Also checks for re-arming weapons!
			GiveGas (1);
			GiveMissile (1);
#endif
			BonusSound ();
			break;	
					
		default:
			touched--;
			got--;
			goto skipremove;		/* can happen if a bonus item is dropped on a scenery pic*/
		}

	/* remove the static object*/
		stat->areanumber = MAXAREAS-1;	//jgt: These appear to have
		stat->x = 0xFFFF;						//jgt:  absolutely no effect!
#ifndef	ID_VERSION
		stat->pic = 0;		// Remove the static object (jgt!)
#endif

skipremove: ;
	}
	
	if (got)
		StartBonusFlash ();
	if (touched == got)
		tilemap[mapy][mapx] &= ~TI_GETABLE;
}


