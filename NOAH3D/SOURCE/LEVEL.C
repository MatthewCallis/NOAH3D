//-------------------------------------------------------------------
// LEVEL.C
//-------------------------------------------------------------------


#include "wolfdef.h"


//-------------------------------------------------------------------
//										GLOBALS
//-------------------------------------------------------------------

void SetupGameLevel (void);

byte		*spawn_p;

//-------------------------
// Static object info
//-------------------------

ushort staticflags[] =
{
	0,						// S_WATER_PUDDLE
	TI_BLOCKMOVE,		// S_GREEN_BARREL
	TI_BLOCKMOVE,		// S_CHAIR_TABLE
	TI_BLOCKMOVE,		// S_FLOOR_LAMP
	0,						// S_CHANDELIER
	TI_GETABLE,			// S_DOG_FOOD
	TI_BLOCKMOVE,		// S_COLLUMN
	TI_BLOCKMOVE,		// S_POTTED_TREE
	
	TI_BLOCKMOVE,		// S_FLAG
	TI_BLOCKMOVE,		// S_POTTED_PLANT
	TI_BLOCKMOVE,		// S_BLUE_POT
	0,						// S_DEBRIS1
	0,						// S_LIGHT
	0,						// S_BUCKET
	TI_BLOCKMOVE,		// S_ARMOUR
	TI_BLOCKMOVE,		// S_CAGE
	
	TI_GETABLE,			// S_G_KEY
	TI_GETABLE,			// S_S_KEY
	TI_GETABLE,			// S_BANDOLIER
	TI_GETABLE,			// S_AMMOCASE
	TI_GETABLE,			// S_FOOD
	TI_GETABLE,			// S_HEALTH
	TI_GETABLE,			// S_AMMO
	TI_GETABLE,			// S_MACHINEGUN
	
	TI_GETABLE,			// S_CHAINGUN
	TI_GETABLE,			// S_CROSS
	TI_GETABLE,			// S_CHALICE
	TI_GETABLE,			// S_CHEST
	TI_GETABLE,			// S_CROWN
	TI_GETABLE,			// S_ONEUP
	TI_BLOCKMOVE,		// S_WOOD_BARREL
	TI_BLOCKMOVE,		// S_WATER_WELL

	TI_GETABLE,			// S_FLAMETHROWER
	TI_GETABLE,			// S_GASCAN
	TI_GETABLE,			// S_LAUNCHER
	TI_GETABLE,			// S_MISSILES
	TI_GETABLE			// S_MAP
};

// 44 items



//===================================================================
//								SPAWNING ROUTINES
//===================================================================


//-------------------------------------------------------------------
// SpawnStatic
//-------------------------------------------------------------------

void SpawnStatic (ushort shape)
{
	if (numstatics==MAXSTATICS-1)
		IO_Error ("Static overload!");

	statics[numstatics].x = (mapx<<FRACBITS)+0x80;
	statics[numstatics].y = (mapy<<FRACBITS)+0x80;
	statics[numstatics].pic = S_WATER_PUDDLE+shape;
	statics[numstatics].areanumber = tilemap[mapy][mapx] & TI_NUMMASK;

	tilemap[mapy][mapx] |= staticflags[shape];  

	switch (statics[numstatics].pic)
	{
		case	S_CROSS:
		case	S_CHALICE:
		case	S_CHEST:
		case	S_CROWN:
		case	S_ONEUP:
		gamestate.treasuretotal++;
	}

	numstatics++;
}


//===================================================================
//								ACTOR SPAWNING
//===================================================================


//-------------------------------------------------------------------
// SpawnPlayer			Spawns player at mapx,mapy
//-------------------------------------------------------------------

void SpawnPlayer (ushort dir)
{
	viewangle = (1-dir)*ANGLES/4;
	irqangle = viewangle<<9;
	actors[0].x = (mapx<<FRACBITS)+0x80;
	actors[0].y = (mapy<<FRACBITS)+0x80;

#if 0
	actors[0].x = 0x2c65;
	actors[0].y = 0x23fc;
	mapx = actors[0].x>>8;
	mapy = actors[0].y>>8;
	viewangle = 0x7f;
	irqangle = viewangle<<9;
#endif

	actors[0].class = CL_PLAYER;
	actors[0].areanumber = tilemap[mapy][mapx]&TI_NUMMASK;

	w_memset(areabyplayer,(char) 0,sizeof(areabyplayer));
	w_memset(areaconnect,(char) 0,sizeof(areaconnect));

	areabyplayer[map.areasoundnum[ actors[0].areanumber]] = true;
}


//-------------------------------------------------------------------
// SpawnStand		Spawns a standing actor at mapx,mapy
//-------------------------------------------------------------------

void SpawnStand (class_t which)
{
	ushort		state;
	classinfo_t	*jinfo;
	
	gamestate.killtotal++;

	jinfo = &classinfo[which];
	state = jinfo->standstate;
	if (numactors==MAXACTORS)
		IO_Error ("Actor overload!");
	actor = &actors[numactors];

	tile = tilemap[mapy][mapx];
	actor->x = (mapx<<FRACBITS)+0x80;
	actor->y = (mapy<<FRACBITS)+0x80;
	actor->pic = states[state].shapenum;
	actor->ticcount = states[state].shapenum;
	actor->class = 0;	/* INIT*/
	actor->state = state;
	actor->flags = FL_SHOOTABLE;
	actor->distance = 0;
	actor->dir = nodir;
	actor->areanumber = tile&TI_NUMMASK;
	actor->hitpoints = 0;	/* INIT*/
	actor->speed = 0;		/* INIT*/
	actor->reacttime = 0;
	tilemap[mapy][mapx] = tile | TI_ACTOR;
	map.tilemap[mapy][mapx] = numactors;
	actor->goalx = mapx;
	actor->goaly = mapy;
	numactors++;	

	actor->class = which;
	actor->speed = jinfo->speed;
	actor->hitpoints = jinfo->hitpoints;
	actor->flags |= FL_SHOOTABLE | FL_NOTMOVING;
}



//-------------------------------------------------------------------
// SpawnAmbush		Spawns a standing actor at mapx,mapy
//-------------------------------------------------------------------

void SpawnAmbush (class_t which)
{
	SpawnStand (which);
	actor->flags |= FL_AMBUSH;
}


//-------------------------------------------------------------------
// SpawnDoor		Spawns a door at mapx,mapy
//-------------------------------------------------------------------

void SpawnDoor (ushort type)
{
	door_t	*jdoor;
	
	if (numdoors==MAXDOORS)
		IO_Error ("Door overload!");
	jdoor = &doors[numdoors];

	jdoor->position = 0;		// Doors start out fully closed
	jdoor->tilex	= mapx;
	jdoor->tiley	= mapy;
	jdoor->info	= type-90;
	jdoor->action	= DR_CLOSED;
	tilemap[mapy][mapx] = TI_DOOR + TI_BLOCKMOVE + TI_BLOCKSIGHT + numdoors;

	if (type & 1)	// Horizontal
	{
		jdoor->area1 = tilemap[mapy+1][mapx] & TI_NUMMASK;
		jdoor->area2 = tilemap[mapy-1][mapx] & TI_NUMMASK;
	}
	else				// Vertical
	{
		jdoor->area1 = tilemap[mapy][mapx-1] & TI_NUMMASK;
		jdoor->area2 = tilemap[mapy][mapx+1] & TI_NUMMASK;
	}

	numdoors++;
}


//-------------------------------------------------------------------
// SpawnPushWall		Spawns a push wall at mapx,mapy
//-------------------------------------------------------------------

void AddPWallTrack (ushort x, ushort y, ushort tile)
{
	if (x>=MAPSIZE || y>=MAPSIZE)
		return;		// Pushwall is on the edge of a map
	if (textures[MAPSIZE+x][y] != 0xFF)
		return;		// Solid wall
		
	textures[MAPSIZE+x][y] = tile + 1;
	textures[y][x] = tile;
}


void SpawnPushwall (void)
{
	ushort	tile;
	
	gamestate.secrettotal++;
	tilemap[mapy][mapx] |= TI_BLOCKMOVE | TI_BLOCKSIGHT | TI_PUSHWALL;

//-------------------------------
// Directly set texture values
// for rendering
//-------------------------------

	tile = (*spawn_p++ -1)<<1;
	AddPWallTrack (mapx, mapy, tile);
	AddPWallTrack (mapx-1, mapy, tile);
	AddPWallTrack (mapx-2, mapy, tile);
	AddPWallTrack (mapx+1, mapy, tile);
	AddPWallTrack (mapx+2, mapy, tile);
	AddPWallTrack (mapx, mapy-1, tile);
	AddPWallTrack (mapx, mapy-2, tile);
	AddPWallTrack (mapx, mapy+1, tile);
	AddPWallTrack (mapx, mapy+2, tile);
}


//-------------------------------------------------------------------
// SpawnElevator		Spawns an elevator wall at mapx,mapy
//-------------------------------------------------------------------

ushort	elevatorx, elevatory;

void SpawnElevator (void)
{
	elevatorx = mapx;		// For EASY mode cheating
	elevatory = mapy;
	tilemap[mapy][mapx] |= TI_BLOCKMOVE | TI_SWITCH;
}

void SpawnOut (void)
{
//	tilemap[mapy][mapx] |= TI_BLOCKMOVE | TI_SWITCH;
}

void SpawnSecret (void)
{
	tilemap[mapy][mapx] |= TI_BLOCKMOVE | TI_SECRET;
}


//-------------------------------------------------------------------
// SpawnThings		Spawn all actors and mark down special places
//						A spawn record is (X, Y, TYPE)
//-------------------------------------------------------------------

void SpawnThings (void)
{
	ushort	i;
	ushort	type;
	
	spawn_p = (byte *)&map+map.spawnlistofs;
	
	for (i=0 ; i<map.numspawn ; i++)
	{
		mapx = *spawn_p++;
		mapy = *spawn_p++;
		type = *spawn_p++;
		if (type<19)
			IO_Error ("Bad spawn type");
		else if (type<23)
			SpawnPlayer (type-19);
#ifdef	ID_VERSION
		else if (type<59)
#else
		else if (type<60)		// Map prize
#endif
			SpawnStatic (type-23);
		else if (type<90)
			IO_Error ("Bad spawn type");
		else if (type<98)
			SpawnDoor (type);
		else if (type == 98)
			SpawnPushwall ();
		else if (type == 99)
			SpawnOut ();
		else if (type == 100)
			SpawnElevator ();
		else if (type == 101)
			SpawnSecret ();
		else if (type < 108)
			IO_Error ("Bad spawn type");
		else if (type <123)
			SpawnStand (type-108);
		else if (type < 126)
			IO_Error ("Bad spawn type");
		else if (type <140)
			SpawnAmbush (type-126);
		else
			IO_Error ("Bad spawn type");	
	}
}


//===================================================================


//-------------------------------------------------------------------
// SetupGameLevel		Load and initialize everything for current level
//							Spawnlist 0-(numusable-1) are the usable
//								(door/pushwall) spawn objects
//							Polysegs 0-(numpushwalls-1) are four sided
//								pushwall segs
//-------------------------------------------------------------------

void SetupGameLevel (void)
{
	byte		*src;
	ushort	*dest;
	ushort	tile;
	
//-------------------------
// Clear counts
//-------------------------

	 gamestate.secrettotal=0;
	 gamestate.killtotal=0;
	 gamestate.treasuretotal=0;
	 gamestate.secretcount=0;
	 gamestate.killcount=0;
	 gamestate.treasurecount = 0;

//-------------------------
// Load & decompress level
//-------------------------

	IO_LoadLevel (gamestate.mapon);
		
	numstatics = 0;
	numdoors = 0;
	numactors = 1;		// Player has spot #0!
#ifdef	ID_VERSION
	nummissiles = 0;
#else
	ClearMissiles ();
#endif
	
//-------------------------
// Expand the byte width
// map to word width
//-------------------------

	src = &map.tilemap[0][0];
	dest = &tilemap[0][0];

	for (mapy=0 ; mapy<MAPSIZE ; mapy++)
		for (mapx=0 ; mapx<MAPSIZE ; mapx++)
		{
			tile = *src++;
			if (tile & TI_BLOCKMOVE)
				tile |= TI_BLOCKSIGHT;
			*dest++ = tile;
		}

//-------------------------
// Let the rendering engine
// know theirs a new map...
//-------------------------

	NewMap ();

//----------------------------------------------
// Spawn things:
//		map.tilemap is now used to hold actor #s
//		if the TI_ACTOR bit is set in the WORD
//		tilemap
//----------------------------------------------

	SpawnThings ();
}

