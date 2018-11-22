//-------------------------------------------------------------------
// WOLFMAIN.C
//-------------------------------------------------------------------

#include "wolfdef.h"
#ifndef __ORCAC__
#include <math.h>
#endif

#ifndef NULL
#define NULL 0
#endif

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//						   WOLFENSTEIN 3D
//						   by John Carmack
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


//===================================================================
//						 GLOBAL VARIABLES
//===================================================================

int		nextmap;

short		extratics;
boolean	frameskipped;
int		difficulty;					// 0 = easy, 1= normal, 2=hard

//----------------------------
// Game variables
//----------------------------

gametype_t	gamestate;

unsigned 	long playtime;
ushort 		facecount, faceframe;	// Vars for BJ head


boolean		madenoise;				// true when shooting or screaming
exit_t		playstate;

boolean		godmode;

short volatile	mousex,mousey;		// SNES mouse movement

ushort		irqangle;				// SNES interrupt sampled turning

short			killeractor;
ushort		mapx,mapy;
ushort		area;
ushort		area1,area2;
ushort		tile,info;
ushort		think;

ushort		checkx, checky;
short			xmove, ymove;
boolean		playermoving;


short			deltax,deltay;			// Distancwe from player
ushort		distance;				// ABS of greatest delta

ushort		move;

demo_t		demomode;
byte			*demo_p;

//----------------------------
// Current user input
//----------------------------

boolean		buttonstate[NUMBUTTONS];
boolean		attackheld,useheld, selectheld;

//----------------------------
// Palette shifting stuff
//----------------------------

ushort		redshift, goldshift;
ushort		damagecount,bonuscount;

//----------------------------
// Static object
//----------------------------

ushort	numstatics,staticon;

static_t	statics[MAXSTATICS], *staticp;

//----------------------------
// Door actor
//----------------------------

ushort	numdoors,dooron;

door_t	doors[MAXDOORS], *door;


//----------------------------
// Actor state structure
//----------------------------

state_t		*state;

//----------------------------
// Thinking actor structure
//----------------------------

actor_t	actors[MAXACTORS], *actor;

ushort	numactors,actoron;

missile_t	missiles[MAXMISSILES], *missile;

ushort	nummissiles, missileon;

classinfo_t	classinfo[CL_LASTCLASS] =
{
	{D_SHEEP, FREQ_NORM,	D_EDIE, FREQ_NORM,
	ST_GRD_WLK1, ST_GRD_STND, ST_GRD_ATK1,ST_GRD_PAIN,ST_GRD_DIE
	, 100, 20, 15, 6},
	
	{D_OSTRCH, FREQ_NORM, D_EDIE, 0x5a0,
	ST_OFC_WLK1, ST_OFC_STND, ST_OFC_ATK1
	,ST_OFC_PAIN,ST_OFC_DIE,400, 40, 1, 12},
	
	{D_ANTLPE, FREQ_LOW,	D_EDIE, FREQ_LOW,
	ST_SS_WLK1, ST_SS_STND, ST_SS_ATK1
	,ST_SS_PAIN,ST_SS_DIE,500, 25, 7, 25},
	
	{D_GOAT, FREQ_NORM, D_EDIE, FREQ_NORM,
	ST_DOG_WLK1, ST_DOG_STND, ST_DOG_ATK1
	,0,ST_DOG_DIE,200, 64, 7, 1},
	
	{D_OX, FREQ_NORM,	D_EDIE, FREQ_LOW-0x100,
	ST_MUTANT_WLK1, ST_MUTANT_STND, ST_MUTANT_ATK1
	,ST_MUTANT_PAIN,ST_MUTANT_DIE,400, 30, 1, 18},
	
	{D_CAMEL, FREQ_NORM,	D_EDIE, FREQ_LOW,
	ST_HANS_WLK1, ST_HANS_STND, ST_HANS_ATK1
	,0,ST_HANS_DIE,5000, 30, 1, 175},
	
	{D_MONKEY, FREQ_NORM,	D_EDIE, FREQ_LOW,
	ST_SCHABBS_WLK1, ST_SCHABBS_STND, ST_SCHABBS_ATK1
	,0,ST_SCHABBS_DIE,5000, 20, 1, 350},
	
	{D_GIRAFE, FREQ_NORM,	D_EDIE, FREQ_LOW,
	ST_TRANS_WLK1, ST_TRANS_STND, ST_TRANS_ATK1
	,0,ST_TRANS_DIE,5000, 30, 1, 300},
	
	{D_KANGRO, FREQ_NORM,	D_EDIE, FREQ_LOW,
	ST_UBER_WLK1, ST_UBER_STND, ST_UBER_ATK1
	,0,ST_UBER_DIE,5000, 35, 1, 400},
	
	{D_ELPHNT, FREQ_NORM,	D_EDIE, FREQ_LOW,
	ST_DKNIGHT_WLK1, ST_DKNIGHT_STND, ST_DKNIGHT_ATK1
	,0,ST_DKNIGHT_DIE,5000, 30, 1, 450},
	
	{D_BEAR, FREQ_NORM,	D_EDIE, FREQ_LOW,
	ST_MHITLER_WLK1, ST_MHITLER_STND, ST_MHITLER_ATK1
	,0, ST_HITLER_DIE,5000, 30, 1, 500},
	
	{D_BEAR, FREQ_NORM,	D_EDIE, FREQ_LOW,
	ST_HITLER_WLK1, ST_MHITLER_STND, ST_HITLER_ATK1
	,0,ST_HITLER_DIE,5000, 35, 1, 500},
};


//----------------------------
// Pushable wall vars
//----------------------------

ushort	pwallcount;						// Blocks still to move
ushort	pwallpos;						// Amount a pushable wall has been moved in it's tile
ushort	pwallx,pwally;					// The tile the pushwall edge is in now
ushort	pwallcheckx,pwallchecky;	// The tile it will be in next
ushort	pwalldir;
short		pwallxchange, pwallychange;// Adjust coordinates this much


//----------------------------
// Map array
//----------------------------

loadmap_t	map;
ushort		tilemap[MAPSIZE][MAPSIZE];


//----------------------------
// Refresh variables
//----------------------------

ushort		viewx;		// The focal point
ushort		viewy;
ushort		viewangle;


//----------------------------
// Area boundary variables
//----------------------------

byte			areaconnect[MAXAREAS][MAXAREAS];
boolean		areabyplayer[MAXAREAS];


//===================================================================
//								LIBRARY ROUTINES
//===================================================================

void w_memset(void *dest,char fill,int size)
{
	while (size--)
		*((char *)dest)++ = fill;
}

void w_memcpy (void *dest, void *src, int size)
{
	while (size--)
		*((char *)dest)++ = *((char *)src)++;
}

unsigned short w_abs (short v)
{
	return v>=0 ? v : -v;
}

unsigned char rndtable[256] = {
	0,   8, 109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66,
	74,  21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36,
	95, 110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188,
	52, 140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224,
	149, 104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242,
	145,  42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0,
	175, 143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235,
	25,  92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113,
	94, 161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75,
	136, 156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196,
	135, 106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113,
	80, 250, 108,   7, 255, 237, 129, 226,  79, 107, 112, 166, 103, 241,
	24, 223, 239, 120, 198,  58,  60,  82, 128,   3, 184,  66, 143, 224,
	145, 224,  81, 206, 163,  45,  63,  90, 168, 114,  59,  33, 159,  95,
	28, 139, 123,  98, 125, 196,  15,  70, 194, 253,  54,  14, 109, 226,
	71,  17, 161,  93, 186,  87, 244, 138,  20,  52, 123, 251,  26,  36,
	17,  46,  52, 231, 232,  76,  31, 221,  84,  37, 216, 165, 212, 106,
	197, 242,  98,  43,  39, 175, 254, 145, 190,  84, 118, 222, 187, 136,
	120, 163, 236, 249
};
ushort	rndindex = 0;

unsigned char w_rnd (void)
{
	rndindex = (rndindex+1)&0xff;
	return rndtable[rndindex];
}


//===================================================================
// Sounds		A source number is any ushort that identifies the
//					  source of the sound, so the death scream of an actor
//				     will always override the sighting sound of the same
//					  actor if it is still playing
//					A source number of 0 will not bother checking
//===================================================================

#define	EFFECTCHANNELS	2
#define	EFFECTBASE		6

ushort	sndsources[EFFECTCHANNELS], nextchan;


//-------------------------------------------------------------------
// GetChannel
//-------------------------------------------------------------------

ushort GetChannel (ushort source)
{
	ushort	channel;

	if (source)
	{	// If the source is currently in use, use that channel
		for (channel = 0 ; channel < EFFECTCHANNELS ; channel++)
			if (sndsources[channel] == source)
			{	// If the channel found is the nextchan, advance nextchan
				if (nextchan == channel && ++nextchan == EFFECTCHANNELS)
						nextchan = 0;
				return channel;
			}
	}
	
// Use the next channel in line

	channel = nextchan;
	if (++nextchan == EFFECTCHANNELS)
		nextchan = 0;
	sndsources[channel] = source;
	return channel;
}


//-------------------------------------------------------------------
// PlayLocalSound
//-------------------------------------------------------------------

void PlayLocalSound (ushort source, ushort sample, ushort frequency, ushort volume)
{
	ushort	channel;
	
	channel = GetChannel (source);
	IO_PlaySound (EFFECTBASE+channel, sample, frequency, volume, volume);
}


//-------------------------------------------------------------------
// PlayPosSound		Adjusts stereo volumes based on position
//-------------------------------------------------------------------

byte	posvolume[15][15] = {	// Right ear volume
{ 1, 1, 2, 2, 3, 3, 4, 4, 4, 4, 4, 4, 4, 3, 2},
{ 1, 2, 2, 3, 3, 4, 4, 5, 5, 5, 5, 5, 4, 4, 3},
{ 1, 2, 2, 3, 4, 4, 5, 6, 6, 6, 6, 6, 5, 5, 4},
{ 1, 2, 3, 3, 4, 5, 6, 6, 7, 7, 7, 7, 6, 5, 5},
{ 1, 2, 2, 3, 4, 5, 6, 7, 8, 8, 8, 7, 7, 6, 5},
{ 1, 2, 2, 3, 4, 5, 6, 8, 9, 9, 9, 8, 7, 6, 6},
{ 1, 1, 2, 2, 3, 4, 6, 8,10,10,10, 9, 8, 7, 6},
{ 1, 1, 1, 2, 2, 3, 3, 6,12,11,10, 9, 8, 7, 6},
{ 1, 1, 1, 1, 1, 1, 1, 2,10,10, 9, 9, 8, 7, 6},
{ 1, 1, 1, 1, 1, 1, 2, 4, 7, 9, 9, 8, 7, 6, 6},
{ 1, 1, 1, 1, 1, 1, 3, 4, 6, 7, 7, 7, 7, 6, 5},
{ 1, 1, 1, 1, 1, 2, 3, 4, 5, 6, 6, 6, 6, 5, 5},
{ 1, 1, 1, 1, 1, 2, 3, 4, 5, 5, 5, 5, 5, 5, 4},
{ 1, 1, 1, 1, 1, 2, 3, 3, 4, 4, 5, 4, 4, 4, 3},
{ 1, 1, 1, 1, 1, 2, 2, 3, 3, 4, 4, 4, 3, 3, 3}
};

extern fixed_t	trx, try;
fixed_t R_TransformX (void);
fixed_t R_TransformZ (void);

void PlayPosSound (ushort source, ushort sample, ushort frequency, ushort volume
	, ushort area, ushort x, ushort y)
{
	ushort	channel;
	short	tablex, tablez;
	ushort	leftvol, rightvol;
	
	if (!areabyplayer[ map.areasoundnum [area] ])
		return;		// Closed off by a door

//----------------------------
// Transform the sound source
// relative to view angle
//----------------------------

	trx = x - viewx;
	try = viewy - y;
	tablez = R_TransformZ();
	tablex = R_TransformX();
	
	tablez = (7*TILEGLOBAL+TILEGLOBAL/2 - tablez)>>FRACBITS;
	tablex = (tablex + 7*TILEGLOBAL+TILEGLOBAL/2)>>FRACBITS;
	if (tablex<0)
		tablex = 0;
	else if (tablex>14)
		tablex = 14;
	if (tablez<0)
		tablez = 0;
	else if (tablez> 14)
		tablez = 14;
	
	channel = GetChannel (source);
	rightvol = (volume * posvolume[tablez][tablex])>>3;
	if (rightvol > 127)
		rightvol = 127;
	tablex = 14-tablex;
	leftvol = (volume * posvolume[tablez][tablex])>>3;
	if (leftvol > 127)
		leftvol = 127;
	
	IO_PlaySound (EFFECTBASE+channel, sample, frequency, leftvol, rightvol);
}

ushort	RndFreq (ushort freq)
{
	freq -= 0x80;
	freq += w_rnd();
	
	return freq;
}


//-------------------------------------------------------------------
// PointToAngle
//-------------------------------------------------------------------

// To get a global angle from cartesian coordinates, the coordinates
// are flipped until they are in the first octant of the coordinate
// system, then the Y ( <= X ) is scaled and divided by X to get
// a tangent (slope) value which is looked up in the tantoangle[]
// table.  The +1 size is to handle the case when X==Y without
// additional checking.

#define	SLOPERANGE	512
#define	SLOPEBITS		9
#ifdef __ORCAC__
extern	// the tantoangle table is an initialized table in the SNES version
#endif
ushort	tantoangle[SLOPERANGE+1];

ushort	AngleFromSlope (ushort y, ushort x);
#ifndef __ORCAC__
ushort	AngleFromSlope (ushort y, ushort x)
{
// X >= Y
	return tantoangle[  ((long)y<<SLOPEBITS)/x];
}
#endif

#define	ANG90		0x4000
#define	ANG180	0x8000
#define	ANG270	0xC000
angle_t PointToAngle (fixed_t x, fixed_t y)
{	
	x -= viewx;
	y = viewy - y;
	
	if (x>= 0)
	{	/* x >=0*/
		if (y>= 0)
		{	/* y>= 0*/
			if (x>y)
				return AngleFromSlope (y,x);     /* octant 0*/
			else
				return ANG90-1-AngleFromSlope (x,y);  /* octant 1*/
		}
		else
		{	/* y<0*/
			y = -y;
			if (x>y)
				return -AngleFromSlope (y,x);  /* octant 8*/
			else
				return ANG270+AngleFromSlope (x,y);  /* octant 7*/
		}
	}
	else
	{	/* x<0*/
		x = -x;
		if (y>= 0)
		{	/* y>= 0*/
			if (x>y)
				return ANG180-1-AngleFromSlope (y,x); /* octant 3*/
			else
				return ANG90+ AngleFromSlope (x,y);  /* octant 2*/
		}
		else
		{	/* y<0*/
			y = -y;
			if (x>y)
				return ANG180+AngleFromSlope (y,x);  /* octant 4*/
			else
				return ANG270-1-AngleFromSlope (x,y);  /* octant 5*/
		}
	}	
	
	return 0;
}


//-------------------------------------------------------------------
// InitPointToAngle
//-------------------------------------------------------------------

void InitPointToAngle (void)
{
#ifndef __ORCAC__
	int	i;
	long	t;
	float	f;

// Slope (tangent) to angle lookup

	for (i=0 ; i<=SLOPERANGE ; i++)
	{
		f = atan( (float)i/SLOPERANGE )/(3.141592657*2);
		t = 0x10000*f;
		tantoangle[i] = t;
	}
#endif
}


//===================================================================
//							PALETTE SHIFTING STUFF
//===================================================================

#define	BONUSTICS	16

//-------------------------------------------------------------------
// StartBonusFlash
//-------------------------------------------------------------------

void StartBonusFlash (void)
{
	bonuscount = BONUSTICS;	// Gold palette
}


//-------------------------------------------------------------------
// StartDamageFlash
//-------------------------------------------------------------------

void StartDamageFlash (int damage)
{
	damagecount += damage;
	if (damagecount > 100)
		damagecount = 100;	// Just for GOD mode convenience
}


//-------------------------------------------------------------------
//	UpdatePaletteShifts		There are 8 levels of red shift and 4
//									levels of gold shift
//-------------------------------------------------------------------

#define	NUMREDSHIFTS	32
#define	NUMGOLDSHIFTS	16


void UpdatePaletteShifts (void)
{
	int	red,gold;

	if (bonuscount)
		bonuscount--;
	if (damagecount)
		damagecount--;

	red = damagecount>>1;
	if (red >= NUMREDSHIFTS)
		red = NUMREDSHIFTS-1;
	gold = bonuscount>>1;
	if (gold >= NUMGOLDSHIFTS)
		gold = NUMGOLDSHIFTS-1;
		
	if (red != redshift || gold != goldshift)
	{
		redshift = red;
		goldshift = gold;
		IO_ColorScreen (goldshift, redshift);
	}
}

/*==========================================================================*/

/*
==================
=
= GameOver
=
= Died () has allready spun the view toward the killer
= GameOver () scales the game over sprite in and pulses it until an ack
==================
*/

void GameOver (void)
{
	ushort	red;
	
	topspritenum = S_GAMEOVER;
	
	for (topspritescale = FRACUNIT*4 ; topspritescale<FRACUNIT*60 
		; topspritescale += FRACUNIT*4)
	{
		while (damagecount > 0)	/* fade the red out*/
		{
			damagecount -= 2;
			if (damagecount > 0)
				damagecount = 0;
			red = damagecount>>1;
			IO_ColorScreen (0, red);
		}
		RenderView ();
	}
	
	StartAck ();
	do
	{
		for ( ; topspritescale>FRACUNIT*50 
			; topspritescale -= FRACUNIT)
		{
			if (CheckAck ())
				return;
			RenderView ();
		}
		for ( ; topspritescale<FRACUNIT*60 
			; topspritescale += FRACUNIT)
		{
			if (CheckAck ())
				return;
			RenderView ();
		}
		
	} while (1);
}

/*==============================================================================*/


/*
==================
=
= VictoryScale
=
= Scale the VICTORY! sprite in
=
==================
*/

void VictoryScale (void)
{
	ushort	red;
	
	topspritenum = S_VICTORY;
	
	for (topspritescale = FRACUNIT*4 ; topspritescale<FRACUNIT*60 
		; topspritescale += FRACUNIT*4)
	{
		while (damagecount > 0)	/* fade the red out*/
		{
			damagecount -= 2;
			if (damagecount > 0)
				damagecount = 0;
			red = damagecount>>1;
			IO_ColorScreen (0, red);
		}
		RenderView ();
	}
	
	StartAck ();
	do
	{
		for ( ; topspritescale>FRACUNIT*50 
			; topspritescale -= FRACUNIT)
		{
			if (CheckAck ())
				return;
			RenderView ();
		}
		for ( ; topspritescale<FRACUNIT*60 
			; topspritescale += FRACUNIT)
		{
			if (CheckAck ())
				return;
			RenderView ();
		}
		
	} while (1);
}



/*==========================================================================*/

/*
==================
=
= Died
=
==================
*/

void Died (void)
{
	ushort	red;
	short	dx,dy;
	ushort	iangle,clockwise,counter;
	short	change;

/**/
/* fade up to red*/
/**/
	gamestate.attackframe = 0;		/* don't die with gun up*/
	PlayLocalSound (SRC_PLAYER, D_PDIE, FREQ_NORM, VOL_HIGH);
	IO_DrawFace (9);
	while (damagecount < NUMREDSHIFTS<<1)
	{
		damagecount += 2;
		red = damagecount>>1;
		if (red >= NUMREDSHIFTS)
			red = NUMREDSHIFTS-1;
		IO_ColorScreen (0, red);
		IO_WaitVBL (1);
	}
		
	
/**/
/* find angle to face attacker*/
/**/
	if (killeractor > 0)
	{
		dx = actors[killeractor].x;
		dy = actors[killeractor].y;
	}
	else
	{
		dx = missiles[-killeractor].x;
		dy = missiles[-killeractor].y;
	}
	iangle = PointToAngle (dx,dy)>>9;
			
/**/
/* rotate to attacker*/
/**/
	viewangle &= ~1;
	iangle &= ~1;
	if (viewangle > iangle)
	{
		counter = viewangle - iangle;
		clockwise = ANGLES-viewangle + iangle;
	}
	else
	{
		clockwise = iangle - viewangle;
		counter = viewangle + ANGLES-iangle;
	}

	if (clockwise<counter)
		change = 2;
	else
		change = -2;

	while (viewangle != iangle)
	{
		viewangle = (viewangle+change)&(ANGLES-1);
		RenderView ();
	}
	
/**/
/* done*/
/**/
	if (!gamestate.lives)
	{
		GameOver ();
	}
	else
	{
		ack ();
		
		gamestate.health = 100;
		if (difficulty > 0)
		{
			gamestate.weapon = gamestate.pendingweapon = WP_PISTOL;
			gamestate.machinegun = false;
			gamestate.chaingun = false;
			gamestate.missile = false;
			gamestate.flamethrower = false;
			gamestate.ammo = STARTAMMO;
			gamestate.missiles = 0;
			gamestate.gas = 0;
			gamestate.maxammo = 99;
			gamestate.keys = 0;
#ifndef	ID_VERSION
			gamestate.automap = false;
#endif
		}
		else
		{
			if (gamestate.ammo < STARTAMMO*2)
				gamestate.ammo = STARTAMMO*2;
		}
		gamestate.attackframe = 0;
		gamestate.attackcount = 0;
	}
	
	FadeOut ();
}


/*
===============
=
= UpdateFace
=
= Calls draw face if time to change
=
===============
*/

void	UpdateFace (void)
{
	if (--facecount)
		return;		/* don't change yet*/
		
	faceframe = (faceframe&1)^1;	/* go to one of the normal looking frames*/
	facecount = (w_rnd ()&31)+1;
	if (gamestate.health <= 25)
		faceframe += 5;			/* use beat up frames*/
	IO_DrawFace (faceframe);
}


//===================================================================
//								CORE PLAYLOOP
//===================================================================

music_t mapmusic[] =
{
	0,		// 1-1
	1,		// 1-2
	6,		// 1-B

	2,		// 2-1
	0,		// 2-2
	1,		// 2-3
	6,		// 2-B

	3,		// 3-1
	2,		// 3-2
	0,		// 3-3
	6,		// 3-B
	8,		// 3-S

	5,		// 4-1
	3,		// 4-2
	1,		// 4-3
	2,		// 4-4
	6,		// 4-B

	7,		// 5-1
	0,		// 5-2
	5,		// 5-3
	3,		// 5-4
	9,		// 5-5
	6,		// 5-B

	10,	// 6-1
	1,		// 6-2
	2,		// 6-3
	7,		// 6-4
	5,		// 6-5
	6,		// 6-B
	8,		// 6-S

	1		// CAST
};


//-------------------------------------------------------------------
// PrepPlayLoop
//-------------------------------------------------------------------

void PrepPlayLoop (void)
{
	SetupGameLevel ();

	topspritescale = 0;
	faceframe = 1;
	facecount = 1;		// Change face next tick
	playstate = 0;
	pwallcount = 0;
	w_memset(buttonstate,(char) 0,sizeof(buttonstate));
	bonuscount = 0;
	damagecount = 0;
	extratics = 0;
	frameskipped = false;
	goldshift = 0;
	redshift = 0;
	playtime = 0;

	RedrawStatusBar ();
	mousey = 0;
	
	StartSong (mapmusic[gamestate.mapon],true);	// Start the music
}


//-------------------------------------------------------------------
// PlayLoop
//-------------------------------------------------------------------

void PlayLoop (void)
{
	do
	{
		useheld = buttonstate[bt_use];
		IO_CheckInput();

		madenoise = false;

		MoveDoors ();
		MovePWalls ();
		MovePlayer ();
		MoveActors ();
		MoveMissiles ();
		
		UpdateFace ();
		UpdatePaletteShifts ();

		viewx = actors[0].x;
		viewy = actors[0].y;
		
		if (frameskipped && extratics >4)
			extratics = 0;		// Don't skip more than 1 frame at a time
		if (extratics <= 4 || demomode)
		{
			frameskipped = false;
			RenderView ();
		}
		else
			frameskipped = true;
		extratics -= 4;

#if 0
		StackCheck ();
#endif
		
	} while (!playstate);
	StopSong ();
}


//-------------------------------------------------------------------
// NewGame		Set up new game to start from the beginning
//-------------------------------------------------------------------

void NewGame (void)
{
	unsigned temp;
	
	temp = gamestate.mapon;
	w_memset(&gamestate,(char) 0,sizeof(gamestate));
	gamestate.weapon = gamestate.pendingweapon = WP_PISTOL;
	gamestate.health = 100;
	gamestate.ammo = STARTAMMO;
	if (!difficulty)
		gamestate.ammo += STARTAMMO;
	gamestate.maxammo = 99;
	gamestate.lives = 3;
	gamestate.nextextra = EXTRAPOINTS;	
	gamestate.mapon = temp;
#ifndef	ID_VERSION
	gamestate.automap = false;
#endif
}


//-------------------------------------------------------------------
// RedrawStatusBar
//-------------------------------------------------------------------

void RedrawStatusBar (void)
{
	IO_DrawFloor (gamestate.mapon);
	IO_DrawScore (gamestate.score);
	IO_DrawTreasure (gamestate.treasure);
	IO_DrawLives (gamestate.lives);
	IO_DrawHealth (gamestate.health);

	switch (gamestate.weapon)
	{
		case WP_FLAMETHROWER:
			IO_DrawAmmo (gamestate.gas);
			break;
		case WP_MISSILE:
			IO_DrawAmmo (gamestate.missiles);
			break;
		default:
			IO_DrawAmmo (gamestate.ammo);
			break;
	}
	IO_DrawKeys (gamestate.keys);
	IO_DrawFace (faceframe);

	IO_ColorScreen (goldshift, redshift);
}


//-------------------------------------------------------------------
// GameLoop
//-------------------------------------------------------------------

void GameLoop (void)
{
	do
	{
		switch (gamestate.mapon)
		{
			case 0:
				Briefing (0);
				break;
			case 3:
				Briefing (1);
				break;
			case 7:
				Briefing (2);
				break;
			case 12:
				Briefing (3);
				break;
			case 17:
				Briefing (4);
				break;
			case 23:
				Briefing (5);
				break;
		}

skipbrief:		
		PrepPlayLoop ();
		PlayLoop ();

		if (playstate == EX_DIED)
		{
			gamestate.lives--;
			Died ();
			if (!gamestate.lives)
				return;
			goto skipbrief;
		}

		if (gamestate.mapon == 28)
		{
			VictoryScale ();
			Intermission ();
			VictoryIntermission ();
			CharacterCast ();
//shit//fuck			Credits (true);
			return;
		}

		if (playstate == EX_SECRET)
		{
			if (gamestate.mapon == 7)
				nextmap = 11;
			else if (gamestate.mapon == 25)
				nextmap = 29;
			else
				IO_Error ("Bad secret elevator");
		}
		else
		{
			if (gamestate.mapon == 10)
				nextmap = 12;		// Skip past secret level
			else if (gamestate.mapon == 11)
				nextmap = 8;
			else if (gamestate.mapon == 29)
				nextmap = 26;
			else
				nextmap = gamestate.mapon+1;
		}

		gamestate.keys = 0;
#ifndef	ID_VERSION
		gamestate.automap = false;
#endif
		IO_WaitVBL(8);
		PlayLocalSound( SRC_PLAYER, 0, FREQ_LOW, VOL_HIGH );
		IO_WaitVBL(20);
		Intermission ();

		gamestate.mapon = nextmap;
	} while (1);
}



#ifdef	ID_VERSION	//jgt: This procedure is never called!
//-------------------------------------------------------------------
// InitWolf			One time only initialization
//-------------------------------------------------------------------

void Intro (void);

void InitWolf (void)
{
//	Intro ();
	difficulty = 1;
	StartupRendering ();
	InitPointToAngle ();
}
#endif	//jgt
