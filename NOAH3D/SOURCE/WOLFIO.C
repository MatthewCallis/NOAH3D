//-------------------------------------------------------------------
// WOLFIO.C
//-------------------------------------------------------------------


#include "refresh.h"
#include "sneswolf.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

boolean  timing = false;

unsigned missionnum[30] = {1,1,1,2,2,2,2,3,3,3,3,3,4,4,4,4,4,5,5,5,5,5,5,6,6,6,6,6,6,6};
unsigned levelnum[30]   = {1,2,3,1,2,3,4,1,2,3,4,5,1,2,3,4,5,1,2,3,4,5,6,1,2,3,4,5,6,7};


//-------------------------------------------------------------------
// SetNumber		Sets a string of 8 by 16 pixel sprite numbers into
//						a field of sprites
//-------------------------------------------------------------------

unsigned long pow10[] = {1,10,100,1000,10000,100000};

void SetNumber (unsigned long number, unsigned spritenum, unsigned digits)
{
	unsigned long	val;
	unsigned			count;
	unsigned  empty;

	empty = 1;
	while (digits)
	{
	count = 0;
		val = pow10[digits-1];

		while (number >= val)
		{
	 	   count++;
	 	   number -= val;
		}

		if (empty && !count && digits!=1)
		{    // Pad on left with blanks rather than 0s
	 	    oam.s[spritenum].name = oam.s[spritenum+1].name = 0;
		}
		else
		{
	 	    empty = 0;
	 	    oam.s[spritenum].name = NS_0+count;
	 	    oam.s[spritenum+1].name = NS_0+count+16;
		}
		spritenum+=2;
		digits--;
	}

	sendoam = 1;
}


//-------------------------------------------------------------------
// IO_Print
//-------------------------------------------------------------------

void IO_Print (char *text)
{
	unsigned  count;

// Write the text at the start

	count = 0;
	while (count < 28 && text[count])
	{
		oam.s[PS_TEXT+count].name = toupper(text[count]) - 32;
		count++;
	}

// Fill the remainder with blanks

	while (count < 28)
	{
		oam.s[PS_TEXT+count].name = 0;
		count++;
	}

	BlankVideo ();
	DmaOAram (&oam, 0, sizeof(oam));	// Upload sprite records

	UnBlankVideo ();
}

char *hexdig = "0123456789ABCDEF";

void itox (unsigned num, char *str)
{
	*str++ = ' ';
	*str++ = '$';
	*str++ = hexdig[(num>>12)&15];
	*str++ = hexdig[(num>>8)&15];
	*str++ = hexdig[(num>>4)&15];
	*str++ = hexdig[num&15];
	*str++ = 0;
}

void IO_PrintNums (char *text, int n1, int n2, int n3)
{
	char str[80];
	char num[10];

	strcpy (str, text);
	itox (n1, num);
	strcat (str, num);
	itox (n2, num);
	strcat (str, num);
	itox (n3, num);
	strcat (str, num);
	IO_Print (str);
}


unsigned inputtic;

void IO_CheckInput (void)
{
	while (inputtic==ticcount)		// Wait for new input
		;
	inputtic = ticcount;

	if (demomode == dm_playback && (joystick1 || mouse1 & 0xc0) )
	{
		playstate = EX_DEMOABORTED;
		return;
	}

//-------------------------------
// Check for auto map
//-------------------------------

	if (joystick1 & JOYPAD_START)
	{
		if (demomode == dm_recording)
		{
	 	    *demo_p++ = 0xFF;
	 	    *demo_p++ = 0xFF;
	 	    *demo_p++ = 0xFF;
	 	    playstate = EX_COMPLETED;
	 	    return;
		}
		RunAutoMap ();
	}

//-------------------------------
// DEBUG: check for freeze pause
//-------------------------------

//shit#if 0
	if ( (joystick1 & JOYPAD_TL) && (joystick1 & JOYPAD_TR) )
	{
		PauseSong ();
		while ( !(joystick1 & JOYPAD_START) )
			;
		while ( joystick1 & JOYPAD_START )
			;
		UnPauseSong ();
	}
//shit#endif

//-------------------------------
// Get control flags from joypad
//-------------------------------

	memset(buttonstate,0,sizeof(buttonstate));
	if (joystick1 & JOYPAD_UP)
		buttonstate[bt_north] = 1;
	if (joystick1 & JOYPAD_DN)
		buttonstate[bt_south] = 1;
	if (joystick1 & JOYPAD_LFT)
		buttonstate[bt_west] = 1;
	if (joystick1 & JOYPAD_RGT)
		buttonstate[bt_east] = 1;
	if (joystick1 & JOYPAD_TL)
		buttonstate[bt_left] = 1;
	if (joystick1 & JOYPAD_TR)
		buttonstate[bt_right] = 1;
	if (joystick1 & JOYPAD_B)
		buttonstate[bt_attack] = 1;
	if (joystick1 & (JOYPAD_Y|JOYPAD_X) )
		buttonstate[bt_run] = 1;
	if (joystick1 & JOYPAD_A)
		buttonstate[bt_use] = 1;
	if (joystick1 & JOYPAD_SELECT)
		buttonstate[bt_select] = 1;

	if (mousepresent)
	{
	    if (mouse1 & mouseb2)
	 	   buttonstate[bt_north] = 1;
	    if (mouse1 & mouseb1)
	 	   buttonstate[bt_attack] = 1;
	    if (mouseclicks >= 2)
	    {
	 	   mouseclicks -= 2;
	 	   buttonstate[bt_use] = 1;
	    }
   }
}


void IO_Error (char *error, ...)
{
	IO_Print (error);

	fadelevel = inidisp = 15;

	while (1)
		;
}



void IO_Debug (unsigned char bank, unsigned addr, unsigned char stat)
{
	char	str[80];

	strcpy (str, "ERROR: $XX:XXXX $XX");

	str[8] = hexdig[(bank>>4)&15];		// Code bank
	str[9] = hexdig[bank&15];

	str[11] = hexdig[(addr>>12)&15];		// Code address
	str[12] = hexdig[(addr>>8)&15];
	str[13] = hexdig[(addr>>4)&15];
	str[14] = hexdig[addr&15];
	
	str[17] = hexdig[(stat>>4)&15];		// Processor status
	str[18] = hexdig[stat&15];

	IO_Error (str);
}



unsigned redadd, greenadd, blueadd, setadd;

void IO_ColorScreen (ushort bonus, ushort damage)
{
	if (godmode)
		damage = 0;

	while (setadd)
		;

	if (bonus > damage)
	{
		blueadd = 0;
		greenadd = bonus;
		redadd = bonus;
	}
	else if (damage)
	{
		redadd = damage;
		greenadd = 0;
		blueadd = damage*2;	//noah: was 0
	}
	else
	{
		redadd = 0;
		greenadd = 0;
		blueadd = 0;
	}

	setadd = 1;
}


void IO_DrawFloor (ushort floor)
{
	SetNumber((unsigned long) missionnum[floor], PS_FLOOR1, 1);
	SetNumber((unsigned long) levelnum[floor], PS_FLOOR2, 1);
}


void IO_DrawScore (long score)
{
	SetNumber((unsigned long) score, PS_SCORE1, 6);
}


void IO_DrawLives (ushort lives)
{
	lives--;
	if (lives > 9)
		lives = 9;
	oam.s[PS_LIVES].name = '0'-32+lives;
}


void IO_DrawHealth (ushort health)
{
	SetNumber((unsigned long) health, PS_HEALTH1, 3);
}


void IO_DrawAmmo (ushort ammo)
{
	SetNumber((unsigned long) ammo, PS_AMMO1, 3);
}


void IO_DrawTreasure (ushort treasure)
{
	SetNumber((unsigned long) treasure, PS_TREASURE1, 2);
}


void IO_DrawKeys (ushort keys)
{
	if (! (keys&2) )
	{
		oam.s[PS_KEY1].name = 0;
		oam.s[PS_KEY1+1].name = 0;
	}
	else
	{
		oam.s[PS_KEY1].name = NS_KEY+1;
		oam.s[PS_KEY1+1].name = NS_KEY+0x10+1;
	}
	if (! (keys&1) )
	{
		oam.s[PS_KEY2].name = 0;
		oam.s[PS_KEY2+1].name = 0;
	}
	else
	{
		oam.s[PS_KEY2].name = NS_KEY;
		oam.s[PS_KEY2+1].name = NS_KEY+0x10;
	}

	sendoam = 1;
}


//-------------------------------------------------------------------
// IO_AttackShape
//-------------------------------------------------------------------

unsigned shapebasename[4] = {0,8,128,136};

void IO_AttackShape (ushort shape)
{
	unsigned num;

	if (shape>>2 != currentweapon)
	{
		currentweapon = shape>>2;
		sendweapon = 1;
	}

	num = shapebasename[shape&3];

	oam.s[PS_GUN].name = num;
	oam.s[PS_GUN+1].name = num+2;
	oam.s[PS_GUN+2].name = num+4;
	oam.s[PS_GUN+3].name = num+6;

	oam.s[PS_GUN+4].name = num+32;
	oam.s[PS_GUN+5].name = num+32+2;
	oam.s[PS_GUN+6].name = num+32+4;
	oam.s[PS_GUN+7].name = num+32+6;

	oam.s[PS_GUN+8].name = num+64;
	oam.s[PS_GUN+9].name = num+64+2;
	oam.s[PS_GUN+10].name = num+64+4;
	oam.s[PS_GUN+11].name = num+64+6;

	oam.s[PS_GUN+12].name = num+96;
	oam.s[PS_GUN+13].name = num+96+2;
	oam.s[PS_GUN+14].name = num+96+4;
	oam.s[PS_GUN+15].name = num+96+6;

	sendoam = true;
}


//-------------------------------------------------------------------
// IO_DrawFace
//-------------------------------------------------------------------

void IO_DrawFace (ushort face)
{
	unsigned num;

	if (face < 5)
		num = 128 + face*3;
	else
		num = 192 + (face-5)*3;

	oam.s[PS_FACE].name = num;
	oam.s[PS_FACE+1].name = num+32;
	oam.s[PS_FACE+2].name = num+2;
	oam.s[PS_FACE+3].name = num+2+16;
	oam.s[PS_FACE+4].name = num+2+32;
	oam.s[PS_FACE+5].name = num+2+48;
}


extern char map0;
extern char map1;
extern char map2;
extern char map3;
extern char map4;
extern char map5;
extern char map6;
extern char map7;
extern char map8;
extern char map9;
extern char map10;
extern char map11;
extern char map12;
extern char map13;
extern char map14;
extern char map15;
extern char map16;
extern char map17;
extern char map18;
extern char map19;
extern char map20;
extern char map21;
extern char map22;
extern char map23;
extern char map24;
extern char map25;
extern char map26;
extern char map27;
extern char map28;
extern char map29;
#ifndef	ID_VERSION
extern char map30;	// Used by CharacterCast ()
#endif

char	*mapptrs[] = {
	&map0,&map1,&map2,&map3,&map4,&map5,&map6,&map7,&map8,&map9,&map10,
	&map11,&map12,&map13,&map14,&map15,&map16,&map17,&map18,&map19,&map20,
	&map21,&map22,&map23,&map24,&map25,&map26,&map27,&map28,&map29
#ifndef	ID_VERSION
	,&map30
#endif
};


void IO_LoadLevel (ushort level)
{
#ifdef STOREDEMO
	SellScreen ();
#endif
	Decompress (mapptrs[level], (byte *)&map);
//	memcpy (&map, mapptrs[level], sizeof(map));
}


void IO_ClearViewBuffer (void)
{
	if (timing)
		return;
	while (sendplayscreen)
		;    // Wait for scren to be displayed
}


void IO_ScaleWallColumn (ushort x, ushort scale, ushort column)
{
	IO_PrintNums ("SWC:",x,scale,column);
	ack ();
}


void IO_ScaleMaskedColumnC (ushort x, ushort scale, ushort column)
{
}


void IO_DisplayViewBuffer (void)
{
	sendplayscreen = 1;

//-------------------------------------
// If this is the first frame rendered,
// upload everthing and fade in
//-------------------------------------

	if (!fadelevel)
	{
		while (sendplayscreen || sendweapon)
			;
		FadeIn ();
		firstframe = 0;
	}
}


#ifdef	ID_VERSION	//jgt: These procedures are never called!
//===================================================================

void snesack (void)
{
	int	i;

	i=0;

	do
	{
	    IO_DrawFloor (i);
	    if (++i==10)
	    	i = 0;
	    IO_CheckInput ();
	} while (!buttonstate[bt_attack]);

	do
	{
	    IO_CheckInput ();
	} while (buttonstate[bt_attack]);

	IO_DrawFloor (gamestate.mapon);
}


void snestime (void)
{
	ticcount = 0;
	timing = true;
	FadeIn ();
	firstframe = 0;

	for (viewangle = 0 ; viewangle < ANGLES ; viewangle++)
	{
		RenderView ();
	}

	IO_DrawScore ((long) ticcount);
	ack ();
	timing = false;
	IO_DrawScore (gamestate.score);
}


unsigned oldstack, newstack;

void StackCheck (void)
{
	asm	tsx;
	asm	stx	%%newstack;

	if (oldstack && newstack != oldstack)
		IO_Error ("StackCheck error");
	oldstack = newstack;
}

//===================================================================
#endif	//jgt



#if 0
//-------------------------------------------------------------------
// RenderWallLoop
//-------------------------------------------------------------------

extern   unsigned rw_x;
extern   unsigned rw_stopx;
extern   unsigned rw_scale;
extern   unsigned rw_scalestep;
extern   fixed_t *rw_tangent;
extern   unsigned rw_distance;
extern   unsigned rw_midpoint;
extern   byte *rw_texture;
extern   unsigned rw_mintex;
extern   unsigned rw_maxtex;
extern   short	rw_centerangle;

#if 0

void RenderWallLoopC (void)
{
	ushort    scaler, texturecollumn, tile;

	for ( ; rw_x < rw_stopx ; rw_x++)
	{
		scaler = rw_scale >> FRACBITS;
		texturecollumn = SUFixedMul (rw_tangent[rw_x],rw_distance) + rw_midpoint;
		if (texturecollumn < rw_mintex)
		{
			texturecollumn = rw_mintex;
		}
		else if (texturecollumn > rw_maxtex)
			texturecollumn = rw_maxtex;
		texturecollumn >>= 3;	/* each tile has 5 fractional (collumn) bits*/
		tile = rw_texture[texturecollumn>>5];
		texturecollumn = (tile<<5) + (texturecollumn & 31);
		IO_ScaleWallColumn (rw_x, scaler,texturecollumn);
		xscale[rw_x] = rw_scale;
		rw_scale += rw_scalestep;
	}
}

#endif

void RenderWallLoop (void);


//-------------------------------------------------------------------
// RenderWall		x2 is NOT drawn, x1<=collumns<x2
//						CALLED: CORE ROUTINE
//-------------------------------------------------------------------

void RenderWall (int x1, int x2, forwardseg_t *fseg)
{
	ushort		width;
	ushort		scale;
	byte			*texture;
	ushort		door;
	fixed_t		*tangent;
	ufixed_t		midpoint;
	door_t		*door_p;

	if (x2 <= x1)
		return;

	if (x2 > fseg->x2)
		x2 = fseg->x2;

	if (fseg->side)
	{
		midpoint = viewy;
		tangent = vtangents;
	}
	else
	{
		tangent = htangents;
		midpoint = viewx;
	}

	if (fseg->texture > 128)
	{
		door = fseg->texture - 129;
		door_p = &doors[door];
		texture = &textures[129 + door_p->info][0];
		midpoint -= door_p->position;
	}
	else
		texture = &textures[fseg->texture][0];

	
	if (x1 == fseg->x1)
		scale = fseg->scale1;
	else
	{
		width = x1 - fseg->x1;
		scale = fseg->scale1 + width*fseg->scalestep;
	}

//----------------------------------
// Calculate and draw each column
//----------------------------------

	rw_x = x1;
	rw_stopx = x2;
	rw_scale = scale;
	rw_scalestep = fseg->scalestep;
	rw_tangent = tangent;
	rw_distance = fseg->distance;
	rw_midpoint = midpoint;
	rw_texture = texture;
	rw_mintex = fseg->mintex;
	rw_maxtex = fseg->maxtex;

	RenderWallLoop ();
}

#endif

extern ushort	absviewsin, absviewcos;
extern fixed_t	viewsin, viewcos;


void R_SetupTransform (void)
{
	viewsin = sintable[viewangle];
	if (viewsin & 0x8000)
		absviewsin = -viewsin;
	else
		absviewsin = viewsin;

	viewcos = costable[viewangle];	
	if (viewcos & 0x8000)
		absviewcos = -viewcos;
	else
		absviewcos = viewcos;
}



void IO_WaitVBL (ushort vbls)
{
	ticcount = 0;
	while (ticcount < vbls)
		;
}


#ifdef DRAWBSP	
void DrawOverheadSeg (saveseg_t *seg, float r, float g, float b)
{
}


void DrawLine (ushort x1, ushort y1, ushort x2, ushort y2, float r, float g, float b)
{
}


void DrawPlane (savenode_t *seg, float r, float g, float b)
{
}
#endif


//===================================================================
// FadeIn / FadeOut												(from IOSNES.C)
//===================================================================

void FadeIn (void)
{
	unsigned char	i;

	coldata = 0xE0;	// Get rid of any color constant addition

	for (i=1 ; i<= 15 ; i++)
	{
		ticcount = 0;
		while (!ticcount)
			;	// Busy wait until tics have elapsed

		fadelevel = inidisp = i;
	}
}

void FadeOut (void)
{
	int	i;		//jgt: This MUST be signed for the 'for' loop to work!

	for (i=14 ; i >= 0 ; i--)
	{
		ticcount = 0;
		while (!ticcount)
			;	// Busy wait until tics have elapsed

		fadelevel = inidisp = (unsigned char) i;
	}
}


//===================================================================
//								TILESCREEN ROUTINES				(from IOSNES.C)
//===================================================================

unsigned tilescreen[32][32];

//-------------------------------------------------------------------
// TilePrint	Basechar is the number for the space character (ASCII 32)
//-------------------------------------------------------------------

void TilePrint (unsigned basechar, unsigned x, unsigned y, char *string)
{
	char		c;
	unsigned	*dest;

	dest = &tilescreen[y][x];
	do
	{
		c = *string++;
		if (!c)
			break;
		*dest++ = c - 32 + basechar;
	} while (1);
}


//-------------------------------------------------------------------
// DrawTilePic
//-------------------------------------------------------------------

void DrawTilePic (unsigned basechar, unsigned x, unsigned y, unsigned w, unsigned h)
{
	unsigned	*dest;
	unsigned	xc, delta;

	dest = &tilescreen[y][x];
	delta = 32-w;
	while (h--)
	{
		xc = w;
		while (xc--)
			*dest++ = basechar++;
		dest += delta;
	}
}
