//-------------------------------------------------------------------
// WOLFDEF.H
//-------------------------------------------------------------------


#ifndef __WOLFDEF__
#define __WOLFDEF__

extern void DEBUG (void);	//jgt: Fake LOGIC ANALYZER 'breakpoint' @ $00FC00
extern unsigned int CHECKROM (void);	// Perform ROM checksum

#ifndef __BYTEBOOL__
#define __BYTEBOOL__

typedef unsigned char byte;
typedef enum {false, true} boolean;

#endif


//If NeXTWOLF is defined, build a double res version for next game
//#define	NeXTWOLF


//-------------------------------------------------------------------
//										DATA
//-------------------------------------------------------------------

#include "sounds.h"

#include "sprites.h"

typedef enum
{
	SONG_01,
	SONG_02,
	SONG_03,
	SONG_04,
	SONG_05,
	SONG_06,
	SONG_07,
	SONG_08,
	SONG_09,
	SONG_10,
	SONG_11,
} music_t;



//-------------------------------------------------------------------
//								GLOBAL CONSTANTS
//-------------------------------------------------------------------

#ifndef NeXTWOLF
#define	SCREENWIDTH		112
#define	SCREENHEIGHT	80
#else
#define	SCREENWIDTH		112*2
#define	SCREENHEIGHT	80*2
#endif

#define	CENTERY		(SCREENHEIGHT/2)
#define	CENTERX		(SCREENWIDTH/2)

#define	ANGLES		128
#define	FRACBITS		8
#define	FRACUNIT		(1<<FRACBITS)
#define	MAXFRAC		0x7FFF
#define	MAXUFRAC		0xFFFF

#define	PLAYERSIZE	88		// Radius


#define MAXACTORS		127	// Max number of nazis, etc.
#define MAXSTATICS	200	// Max number of lamps, bonus, etc
#define MAXMISSILES	32		// Max missiles, flames, smokes, etc
#define MAXDOORS		64		// Max number of sliding doors
#define MAXWALLTILES	64		// Max number of wall tiles
#define MAXVISABLE	48		// Max visable objects at once
#define MAXAREAS		64		// MAXAREAS-1 is a void area

#define MAXDAMAGECOLOR	32
#define MAXBONUSCOLOR	8

//-------------------

#define MAX_SCORE		999999
#define EXTRAPOINTS	20000
#define STARTAMMO		16


#define TILEGLOBAL		256
#define MINACTORDIST		0xC0	// Minimum dist from player center
										//   to any actor center
#define KNIFEDIST			480	// Max dist for a knife hit
#define BITERANGE      	450	// Max dist for a bite hit
#define MISSILEHITDIST	120

#define MAPSIZE			64

#define SHOOTDELTA		8		// Slop range for targeting actors


//----------------------
// Object flag values
//----------------------

#define FL_SHOOTABLE	1
#define FL_ACTIVE		2
#define FL_SEEPLAYER	4		// If true, dodge instead of moving straight forward
#define FL_AMBUSH		16
#define FL_WAITDOOR	32		// If set, ac_dir points at a door
#define FL_NOTMOVING	64		// Still centered in dest tile
#define FL_DEAD		128	// If set, don't shoot this anymore (death explosion)


//----------------------
// State constants
//----------------------

enum {
	T_NULL,
	T_STAND,
	T_CHASE
};

enum {
	A_NULL,
	A_TARGET,
	A_SHOOT,
	A_BITE,
	A_THROW,
	A_LAUNCH,
	A_HITLERMORPH,
	A_MECHSTEP,
	A_VICTORY,
	A_SCREAM
};

#include "states.h"


//----------------------
// Control info
//----------------------

typedef enum {
	bt_north,
	bt_east,
	bt_south,
	bt_west,
	bt_left,
	bt_right,
	bt_attack,
	bt_use,
	bt_run,
	bt_select
} buttonname_t;


//-------------------------------------------------------------------
//									GLOBAL TYPES
//-------------------------------------------------------------------

#ifndef USHORT
//jgt	typedef unsigned short	ushort;	// this is defined in a unix header...
typedef	unsigned	ushort;	//jgt
#endif

// An angle_t occupies an entire 16 bits so wraparound is automatically handled
typedef unsigned short	angle_t;
#define	SHORTTOANGLESHIFT	9	// 0x10000 to ANGLES

typedef short fixed_t;
typedef unsigned short ufixed_t;
 
typedef enum {
	CD_NORTH,
	CD_EAST,
	CD_SOUTH,
	CD_WEST
} cardinaldir_t;		// Used for spawning and wall pushing

typedef enum {
	east,
	northeast,
	north,
	northwest,
	west,
	southwest,
	south,
	southeast,
	nodir
} dirtype;

typedef enum	{
	WP_KNIFE,
	WP_PISTOL,
	WP_MACHINEGUN,
	WP_CHAINGUN,
	WP_FLAMETHROWER,
	WP_MISSILE,
	NUMWEAPONS
} weapontype;


//----------------------
// Gamestate structure
//----------------------

typedef	struct
{
	ushort	mapon;
	long		score,nextextra;
	ushort	treasure;
		
	ushort	lives;
	ushort	health;
	ushort	ammo, maxammo;
	ushort	gas, missiles;
	ushort	keys;
	boolean	machinegun,chaingun, missile, flamethrower;
#ifndef	ID_VERSION
	boolean	automap;
#endif
	weapontype	weapon, pendingweapon;
	ushort	attackframe,attackcount;

	ushort	secretcount,treasurecount,killcount;
	ushort	secrettotal,treasuretotal,killtotal;
	ushort	globalsecret, globaltreasure, globalkill;
	ushort	globalsecrettotal, globaltreasuretotal, globalkilltotal;
	long		globaltime, globaltimetotal;
} gametype_t;


typedef	enum	{
	EX_STILLPLAYING,
	EX_COMPLETED,
	EX_SECRET,
	EX_WARPED,
	EX_DIED,
	EX_DEMOABORTED
} exit_t;


typedef	enum
{
	DR_OPEN,
	DR_CLOSED,
	DR_OPENING,
	DR_CLOSING,
	DR_WEDGEDOPEN
} dooraction_t;


typedef struct
{
	byte		tilemap[64][64];
	byte		areasoundnum[64];
	ushort	numspawn;
	ushort	spawnlistofs;
	ushort	numnodes;
	ushort	nodelistofs;
	byte		data[10600];		// Nodes, and spawn list
	// 10600 bytes should be enough, but there is no error checking!	//jgt
	// *** THIS IS PUSHING THE 64K LIMIT OF 'UDATA'!! ***
} loadmap_t;

//----------------------
// Actor class info
//----------------------

typedef enum {
	CL_GUARD,
	CL_OFFICER,
	CL_SS,
	CL_DOG,
	CL_MUTANT,
	
	CL_HANS,
	CL_SCHABBS,
	CL_TRANS,
	CL_UBER,
	CL_DKNIGHT,
	CL_MECHAHITLER,
	CL_HITLER,
	
	CL_PLAYER,
	CL_LASTCLASS
} class_t;


typedef struct
{
	ushort	sightsound, sightfreq;
	ushort	deathsound, deathfreq;
	ushort	sightstate;
	ushort	standstate;
	ushort	attackstate;
	ushort	painstate;
	ushort	deathstate;
	ushort	points;
	ushort	speed;
	ushort	reactionmask;		// reaction time = 1 + w_rnd()&reactionmask
	ushort	hitpoints;
} classinfo_t;

typedef enum { dm_none, dm_playback, dm_recording} demo_t;



//-------------------------------------------------------------------
//									GLOBAL DATA
//-------------------------------------------------------------------

extern	short			extratics;
extern	int			difficulty;		// 0 = easy, 1= normal, 2=hard

//----------------------
// Game variables
//----------------------

extern	gametype_t	gamestate;
extern	unsigned long playtime;
extern	ushort 		facecount, faceframe;	// Vars for BJ head

extern	boolean		madenoise;		// True when shooting or screaming
extern	exit_t		playstate;

extern	boolean		godmode;

extern	short volatile	mousex,mousey;	// SNES mouse movement

extern	ushort		irqangle;			// SNES interrupt sampled turning

extern	short			killeractor;
extern	ushort		mapx,mapy;
extern	ushort		area;
extern	ushort		area1,area2;
extern	ushort		tile,info;
extern	ushort		think;

extern	ushort		checkx, checky;
extern	short			xmove,ymove;		/* for player movement*/
extern	boolean		playermoving;

extern	short			deltax,deltay;			/* distacne from player*/
extern	ushort		distance;				/* abs of greatest delta*/

extern	ushort		move;

extern	demo_t		demomode;
extern	byte			*demo_p;

extern	ushort		rndindex;

//----------------------
// Current user input
//----------------------

#define	NUMBUTTONS	10
extern	boolean		buttonstate[NUMBUTTONS];
extern	boolean		attackheld,useheld, selectheld;

//----------------------
// Palette shifting
//----------------------

extern	ushort		redshift, goldshift;
extern	ushort		damagecount,bonuscount;


//----------------------
// Static object
//----------------------

extern	ushort	numstatics,staticon;

typedef struct
{
	ushort	x;
	ushort	y;
	ushort	pic;
	ushort	areanumber;
	
	ushort	item;
} static_t;

extern	static_t	statics[MAXSTATICS], *staticp;

//----------------------
// Door actor
//----------------------

extern	ushort	numdoors,dooron;

typedef struct
{
	ushort	tilex;
	ushort	tiley;
	ushort	position;
	ushort	info;
	ushort	action;
	ushort	ticcount;
	ushort	area1;
	ushort	area2;
} door_t;

extern	door_t	doors[MAXDOORS], *door;


//----------------------
// Actor state structure
//----------------------

typedef struct
{
	ushort	shapenum;
	ushort	tictime;
	ushort	think;
	ushort	action;
	ushort	next;
} state_t;

extern	state_t	states[NUMSTATES],*state;

//----------------------
// Thinking actor
//----------------------

typedef struct
{
	ushort	x;
	ushort	y;
	ushort	pic;
	ushort	areanumber;

	ushort	ticcount;
	ushort	class;
	ushort	state;
	ushort	flags;
	ushort	distance;
	ushort	dir;
	ushort	hitpoints;
	ushort	speed;
	ushort	goalx;		// Tile number
	ushort	goaly;
	ushort	reacttime;
} actor_t;


extern	actor_t	actors[MAXACTORS], *actor;
extern	ushort	numactors,actoron;

extern	classinfo_t	classinfo[CL_LASTCLASS];


//-------------------------
// Missile actor structure
//-------------------------

typedef enum
{
	MI_PMISSILE,
	MI_PFLAME,
	MI_EMISSILE,
	MI_SMOKE,
	MI_NEEDLE,
	MI_SLEEPZ		//jgt
} mtype_t;


#define	MF_HITPLAYER	1
#define	MF_HITENEMIES	2
#define	MF_HITSTATICS	4

typedef struct
{
	ushort	x;
	ushort	y;
	ushort	pic;
	ushort	areanumber;

	ushort	parent;		// Actor that spawned the missile
	ushort	type;			// Also used as a ticcount for explosions
	ushort	flags;
	short	xspeed;
	short	yspeed;
} missile_t;

extern	missile_t	missiles[MAXMISSILES], *missile;
extern	ushort	nummissiles, missileon;


//----------------------
// Pushable wall vars
//----------------------

extern	ushort		pwallcount;			// Blocks still to move
extern	ushort		pwallpos;			// Amount a pushable wall has been moved in it's tile
extern	ushort		pwallx,pwally;		// The tile the pushwall edge is in now
extern	ushort		pwallcheckx,pwallchecky;	// The tile it will be in next
extern	ushort		pwalldir;
extern	short		pwallxchange, pwallychange;	// Adjust coordinates this much

//----------------------
// Map arrays
//----------------------

extern	loadmap_t	map;

#define	TI_SECRET		0x8000
#define	TI_BODY			0x4000
#define	TI_BLOCKSIGHT	0x2000
#define	TI_GETABLE		0x1000
#define	TI_ACTOR			0x800
#define	TI_DOOR			0x400
#define	TI_PUSHWALL		0x200
#define	TI_SWITCH		0x100
#define	TI_BLOCKMOVE	0x80
#define	TI_NUMMASK		0x7F		// Can be an area, door number, or pwall number

extern	ushort			tilemap[MAPSIZE][MAPSIZE];


//----------------------
// Refresh variables
//----------------------

extern	ushort		viewx,viewy;		// The focal point
extern	ushort		viewangle, irqangle;
extern	fixed_t	sintable[ANGLES],costable[ANGLES];

//----------------------
// Area boundary vars
//----------------------

extern	byte			areaconnect[MAXAREAS][MAXAREAS];
extern	boolean		areabyplayer[MAXAREAS];


extern	ushort	topspritescale, topspritenum;


//-------------------------------------------------------------------
//										FUNCTIONS
//-------------------------------------------------------------------

// IO
void StartAck (void);
boolean CheckAck (void);
void ack (void);
void FadeOut (void);
void FadeIn (void);
void snestime (void);
void CheckStack (void);

void IO_Print (char *text);
void IO_PrintNums (char *text, int n1, int n2, int n3);
void IO_PlaySound (ushort channel, ushort sample, ushort rate, ushort leftvol, ushort rightvol);
void IO_CheckInput (void);

void IO_Error (char *error, ...);

void IO_ColorScreen (ushort bonus, ushort damage);

void IO_DrawFace (ushort face);
void IO_DrawFloor (ushort floor);
void IO_DrawScore (long score);
void IO_DrawTreasure (ushort treasure);
void IO_DrawLives (ushort lives);
void IO_DrawHealth (ushort health);
void IO_DrawAmmo (ushort ammo);
void IO_DrawKeys (ushort weapon);

void IO_AttackShape (ushort shape);

void IO_LoadLevel (ushort level);

void IO_ClearViewBuffer (void);
void IO_ScaleWallColumn (ushort x, ushort scale, ushort column);
void IO_ScaleMaskedColumn (ushort x, ushort scale, unsigned long sprite, ushort column);
void IO_DisplayViewBuffer (void);

void IO_WaitVBL (ushort vbls);


// non gameloop functions

void Briefing (ushort mission);
void Intermission (void);
void VictoryIntermission (void);
void CharacterCast (void);
void Credits (boolean final);
#ifndef	ID_VERSION
void Disclaimer (void);
#endif

// WOLFMAIN

void w_memset (void *dest, char fill, int size);
void w_memcpy (void *dest, void *src, int size);

unsigned char w_rnd (void);
unsigned short w_abs (short v);
angle_t PointToAngle (fixed_t x, fixed_t y);
void RedrawStatusBar (void);
void InitWolf (void);
void NewGame (void);
void GameLoop (void);
void PrepPlayLoop (void);
void PlayLoop (void);
void StartDamageFlash (int damage);
void StartBonusFlash (void);
void Died (void);

void MusicTic (void);
void	StartSong (music_t songnum,boolean loops);
void	StopSong(void);


// sound effect source numbers
#define	SRC_BONUS	1
#define	SRC_WEAPON	2
#define	SRC_PWALL	3
#define	SRC_PLAYER	4
#define	SRC_ACTOR	0x100	// + actoron
#define	SRC_MISSILE	0x300	// + missileon
#define	SRC_DOOR		0x400	// + dooron

#define	VOL_NORM			0x40
#define	VOL_LOW			0x30
#define	VOL_HIGH			0x60

#define	FREQ_NORM		0x560
#define	FREQ_LOW			0x440
#define	FREQ_HIGH		0x700

void PlayLocalSound (ushort source, ushort sample, ushort frequency, ushort volume);
void PlayPosSound (ushort source, ushort sample, ushort frequency, ushort volume
	, ushort area, ushort x, ushort y);
ushort	RndFreq (ushort freq);

// LEVEL

void SetupGameLevel (void);

// SIGHT

boolean CheckLine (void);
void FirstSighting (void);
void T_Stand (void);

// DAMAGE

void DamageActoron (ushort damage);

// ENMOVE

boolean TryWalk (void);
void NewState (ushort state);
void SelectDodgeDir (void);
void SelectChaseDir (void);
void MoveActoron (ushort move);

// WL_PLMOV

void ControlMovement (void);

// WL_PLSTF

void	TakeDamage (ushort points);
void	GivePoints (ushort points);
void	GetBonus (void);

// PLTHINK

void	MovePlayer (void);
void ChangeWeapon (void);

// DOORS

void OperateDoor (void);
void OpenDoor (void);
void MoveDoors (void);

// PWALL

void PushWall (void);
void MovePWalls (void);

// ENTHINK

void CalcDistance (void);
void MoveActors (void);

// missiles

#ifndef	ID_VERSION
void ClearMissiles (void);
#endif
void MoveMissiles (void);
void GetNewMissile (boolean priority);

// refresh

// Sprite rendering info is also used to target gun attacks

typedef struct
{
	long			pos;				// Position of sprite info
	ufixed_t		columnstep;
	short			x1,x2;
	ushort		actornum;		// 0 if a static sprite / missile
	ufixed_t		clipscale;
} vissprite_t;

#define	MAXVISSPRITES	64
extern	vissprite_t	vissprites[MAXVISSPRITES], *visspr_p;
extern	ushort		xevents[MAXVISSPRITES];
extern	ushort		*firstevent, *lastevent;

extern	byte		textures[MAPSIZE*2+5][MAPSIZE];  //   0-63  is horizontal, 64-127 is vertical
																	// 128-132 are doors

fixed_t	FixedByFrac (fixed_t a, fixed_t b);
fixed_t	FixedMul (fixed_t a, fixed_t b);
fixed_t	SUFixedMul (fixed_t a, ufixed_t b);
ufixed_t	UFixedMul (ufixed_t a, ufixed_t b);
fixed_t	FixedDiv (fixed_t a, fixed_t b);
ufixed_t	UFixedDiv (ufixed_t a, ufixed_t b);
short		Div16by8 (short a, ushort b);
ushort	UDiv16by8 (ushort a, ushort b);
short 	Mul16S8U (short a, short b);

void StartupRendering (void);
void NewMap (void);
void RenderView (void);
void DrawAutomap (ushort tx, ushort ty);



void StartPushWall (void);
void AdvancePushWall (void);
void StopPushWall (void);


#endif // __WOLFDEF__ 

