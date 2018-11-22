/* refresh.h*/

/*#define	DEBUGLINES*/

#include "wolfdef.h"

#ifndef __BYTEBOOL__
#define __BYTEBOOL__

typedef unsigned char byte;
typedef enum {false, true} boolean;

#endif

#define	SIDESLOPE		0xa0

#ifndef NeXTWOLF
#define	PROJECTIONSCALE	((ushort)((SCREENWIDTH/2)*0x8000l/SIDESLOPE))
#else
/* use a smaller scale to allow higher res*/
#define	PROJECTIONSCALE	((ushort)((SCREENWIDTH/2)*0x4000l/SIDESLOPE))
#endif

#define	CLOSESTZ	62		/* PLAYERSIZE/sqrt(2) rounded down*/
#define	MAXSCALE	232		/* PROJECTIONSCALE/closestz*/

#define	MINZ			CLOSESTZ
#define	MAXZ			(32*FRACUNIT)

typedef struct
{
	fixed_t		x,y;
	ushort		sprite;
	ushort		areanumber;
} thing_t;

enum {BSPTOP,BSPBOTTOM,BSPLEFT,BSPRIGHT};


/*
===============================================================================

The saved data structures are held in a single list, with segs being differentiated from nodes by the
presence of DIR_SEGFLAG in the dir field

===============================================================================
*/

typedef enum
{
	or_horizontal,
	or_vertical
} orientation_t;

enum {di_north, di_east, di_south, di_west};

typedef struct
{
	byte		plane;
	byte		dir;
	ushort	children[2];
} savenode_t;

typedef struct
{
	byte		plane;		// in half tiles
	byte		dir;
	byte		min,max;		// in half tiles
	byte		texture;
	byte		area;
} saveseg_t;

#define	DIR_SEGFLAG		0x80
#define	DIR_LASTSEGFLAG	0x40
#define	DIR_DISABLEDFLAG	0x20
#define	DIR_SEENFLAG		0x10	// For automapping

//#define	DEBUGLINES
#ifdef	DRAWBSP
extern	int	drawsegcount, checksegcount, rwallcount; /* performance tuning counters*/
#endif

#define	FINEANGLES		2048
#define	FINEMASK		(FINEANGLES-1)
#define	ANGLETOFINESHIFT	5	/* 0x10000 to 0x800*/
#define	ANGLE90		0x4000
#define	ANGLE180		0x8000

//#define SCREENWIDTH	114
#define	FIELDOFVIEW	364	// fineangles in the SCREENWIDTH wide window

extern	int		clipshortangle;

extern	ushort	normalangle;		/* angle from viewpoint to surface normal*/
extern	ushort	centerangle;		/* viewangle in fineangles*/
extern	ushort	centershort;		/* viewangle in 64k angles*/

extern	ushort	rw_x;
extern	ushort	rw_stopx;
extern	ushort	rw_scale;
extern	ushort	rw_scalestep;
extern	ushort	rw_distance;
extern	ushort	rw_midpoint;
extern	ushort	rw_mintex;
extern	ushort	rw_maxtex;
extern	byte		*rw_texture;
extern	short	rw_centerangle;
extern	boolean	rw_downside;		/* true for dir_east and dir_south*/

extern	ushort	bspcoord[4];

/*
===============================================================================

						ARRAYS

===============================================================================
*/

/**/
/* precalculated math tables*/
/**/
extern	fixed_t	sintable[ANGLES],costable[ANGLES];

#ifdef __ORCAC__

/* linked in table data*/
extern	ufixed_t		scaleatz[];
extern	short		viewtox[];
extern	short		xtoview[];
extern	short		finetan[];
extern	short		finesin[];

#else

extern	ufixed_t	scaleatz[MAXZ];
extern	short	viewtox[FINEANGLES/2];
extern	short	xtoview[SCREENWIDTH+1];
extern	short	finetan[FINEANGLES/2];
extern	short	finesin[FINEANGLES/2];

#endif

/**/
/* pointers to precalculated tables to fudge snes memory model*/
/**/
extern	short	*viewangletox;
extern	short	*xtoviewangle;
extern	short	*finetangent;
extern	short	*finesine;
extern	ufixed_t	*scaleatzptr;





extern	ufixed_t	xscale[SCREENWIDTH];	/* filled in by walls, used to Z clip sprites*/
extern	boolean	areavis[MAXAREAS];		/* set each frame if an area borders a rendered area*/

extern	byte		textures[MAPSIZE*2+5][MAPSIZE];  /* 0-63 is horizontal, 64-127 is vertical*/
										/* 128 - 132 are doors*/

extern	ushort		xevents[MAXVISSPRITES], *xevent_p;
extern	ushort		sortbuffer[MAXVISSPRITES];  /* mergesort requires an extra buffer*/

extern	vissprite_t	vissprites[MAXVISSPRITES], *visspr_p;

extern	boolean		psegvisible[64];		/* for automap*/


extern	savenode_t	*nodes;
extern	saveseg_t	*pwallseg;

extern	fixed_t	viewsin, viewcos;
extern 	fixed_t	trx, try;


/**/
/* mergesort vars*/
/**/
extern	ushort	*firstevent, *lastevent;
extern	ushort	size1;
extern	ushort	size2;
extern	ushort	*src1, *src2, *dest;

/*
===============================================================================

							REFRESH.C
							
===============================================================================
*/

void StartupRendering (void);
void RenderView (void);

fixed_t	FixedByFrac (fixed_t a, fixed_t b);
fixed_t	FixedMul (fixed_t a, fixed_t b);
fixed_t	SUFixedMul (fixed_t a, ufixed_t b);
ufixed_t	UFixedMul (ufixed_t a, ufixed_t b);
fixed_t	FixedDiv (fixed_t a, fixed_t b);
ufixed_t	UFixedDiv (ufixed_t a, ufixed_t b);
short	Div16by8 (short a, ushort b);
ushort	UDiv16by8 (ushort a, ushort b);


void R_SetupTransform (void);
fixed_t R_TransformX (void);
fixed_t R_TransformZ (void);

ufixed_t ScaleFromGlobalAngle (short visangle);

/*
===============================================================================

							REFSPR.C
							
===============================================================================
*/

extern	ushort	topspritescale;
extern	ushort	topspritenum;

void DrawSprites (void);
void DrawTopSprite (void);


/*
===============================================================================

							REFBSP.C
							
===============================================================================
*/

void 	ClearClipSegs (void);
void 	RenderBSPNode (ushort bspnum);

/*
===============================================================================

							NEXT DEBUGGING
							
===============================================================================
*/

void IO_ClearLineDisplay (void);
void IO_DisplayLine (int x1, int y1, int x2, int y2, int color);
void DrawOverheadSeg (saveseg_t *seg, float r, float g, float b);
void DrawLine (ushort x1, ushort y1, ushort x2, ushort y2, float r, float g, float b);
void DrawPlane (savenode_t *seg, float r, float g, float b);



