//-------------------------------------------------------------------
// SNESMAIN.C
//-------------------------------------------------------------------

#include "sneswolf.h"
#include <string.h>
#include "refresh.h"


//#define	RECORDDEMO	// Remove leading comment to record demos


oam_t    oam;		// Was in IOSNES.C! (jgt)


void SetupScalers (void);

char     mouse1, mouse2, mouse3, mouse4;
boolean  mousepresent;

boolean  stereosound, lefthandmouse;
unsigned mouseb1 = 0x40;
unsigned mouseb2 = 0x80;	// 0x40/0x80 depending on lefthandmouse

void InstrumentTest (void);

boolean  badmouse;


//-------------------------------------------------------------------
// Decompress
//-------------------------------------------------------------------

#if 0
void Decompress (byte *src_p, byte *dest_p)
{
	byte		escapebyte;
	byte		*endout;
	unsigned	outlength, matchbits, compword, match, offset;
	unsigned	matchmask;

	outlength = *((unsigned *)src_p)++;
	escapebyte = *src_p++;
	matchbits = *src_p++;

	matchmask = (1<<matchbits)-1;
	endout = dest_p + outlength;

//-------------------------
// Decompress
//-------------------------

	while (dest_p < endout)
	{
		if (*src_p == escapebyte)
		{
			src_p++;
			if (!*src_p)
			{	/* wasted a byte signaling the escape char*/
				*dest_p++ = escapebyte;
				src_p++;
			}
			else
			{	/* compressed string*/
				compword = *((unsigned *)src_p)++;
				match = (compword&matchmask)+3;
				offset = compword >> matchbits;
				memcpy (dest_p, dest_p-offset, match);
				dest_p += match;
			}
		}
		else
			*dest_p++ = *src_p++;	// Not compressed
	}

	if (dest_p != endout)
		IO_Error ("Overshot specified length");
}
#endif


//-------------------------------------------------------------------
// Ack routines test for a pressed & released btn on joystick/mouse
//-------------------------------------------------------------------

unsigned ackstage, checkbits, pushbits, ackhit;

void StartAck (void)
{
	pushbits = 0;
	ackstage = 0;
	ackhit = 0;
}

void FetchButtons (void)
{
	unsigned	oldtic;

	oldtic = ticcount;
	while (ticcount == oldtic)
		;
	checkbits = ((joystick1>>4)&0x0F) | ((joystick1>>8)&0xF0);
	if (mousepresent)
		checkbits |= (mouse1&0xC0) <<2;
}

boolean CheckAck (void)
{
	FetchButtons ();

	switch (ackstage) {
		case 0:	if (!checkbits)
						ackstage++;		// All held buttons have been released
					break;
		case 1:	if (checkbits) {
						pushbits = joystick1;
						ackstage++;		// A button has been pressed
					}
					break;
		case 2:	if (!checkbits) {
						ackhit = true;
						return true;	// All buttons are up again
					}
	}
	return false;
}

void ack (void)
{
	StartAck ();
	while (!CheckAck ())
		;
}

void WaitUp (void)
{
	do {
		FetchButtons ();
	} while (checkbits || joystick1);
#if 0
	ackstage = 2;
	while (!CheckAck ())
		;
#endif
}


//===================================================================
// All game sections are responsible for fading out before exiting
//===================================================================

unsigned sendplayscreen;     /* set to 1 and screenbuffer will be sent to */
                             /* VRAM the next irq.  Sendplayscreen will */
                             /* then be cleared */

unsigned sendoam;            /* sprites will only be sent on frames that */
                             /* don't have sendplayscreen set */
unsigned sendweapon;         /* upload a new weapon sprite set if not 0 */

unsigned currentweapon;      /* weapon number 0-5 for attack sprites */

unsigned firstframe;         /* if non 0, the screen is still faded out */

void PlayScreenIRQ (void);
void SetupPlayScreen (void);
void DmaPlayScreen (char *screen);


char hdmalist[] = {
 100, 7,			// 100 lines of mode 7
 75, 7,			// 75 more lines of mode 7
 1, 1,			// Go into mode 1 on line 176
 0					// End of HDMA list
};


char playsprites[PS_LASTSPRITE*3] =
{
//#define  PS_FLOOR1		0
    20,188, 0,
    20,196, 0,
//#define  PS_FLOOR2		2
    36,188, 0,
    36,196, 0,
//#define  PS_SCORE1		4
    48,188, 0,
    48,196, 0,
//#define  PS_SCORE2		6
    56,188, 0,
    56,196, 0,
//#define  PS_SCORE3		8
    64,188, 0,
    64,196, 0,
//#define  PS_SCORE4		10
    72,188, 0,
    72,196, 0,
//#define  PS_SCORE5		12
    80,188, 0,
    80,196, 0,
//#define  PS_SCORE6		14
    88,188, 0,
    88,196, 0,
//#define  PS_TREASURE1	16
    104,188, 0,
    104,196, 0,
//#define  PS_TREASURE2	18
    112,188, 0,
    112,196, 0,
//#define  PS_FACE		20        6 sprites, 2 large 4 small
    128,176, 1,
    128,192, 1,
    144,176, 0,
    144,184, 0,
    144,192, 0,
    144,200, 0,
//#define  PS_HEALTH1	26
    160,188, 0,
    160,196, 0,
//#define  PS_HEALTH2	28
    168,188, 0,
    168,196, 0,
//#define  PS_HEALTH3	30
    176,188, 0,
    176,196, 0,
//#define  PS_AMMO1		32
    200,188, 0,
    200,196, 0,
//#define  PS_AMMO2		34
    208,188, 0,
    208,196, 0,
//#define  PS_AMMO3		36
    216,188, 0,
    216,196, 0,
//#define  PS_KEY1		38
    232,176, 0,
    232,184, 0,
//#define  PS_KEY2		40
    232,192, 0,
    232,200, 0,
//#define  PS_GUN			42         16 large sprites
    96,112, 1,
    112,112, 1,
    128,112, 1,
    144,112, 1,
    96,128, 1,
    112,128, 1,
    128,128, 1,
    144,128, 1,
    96,144, 1,
    112,144, 1,
    128,144, 1,
    144,144, 1,
    96,160, 1,
    112,160, 1,
    128,160, 1,
    144,160, 1,
//#define  PS_TEXT		58	// 28 small sprites
    16,24, 0,
    24,24, 0,
    32,24, 0,
    40,24, 0,
    48,24, 0,
    56,24, 0,
    64,24, 0,
    72,24, 0,
    80,24, 0,
    88,24, 0,
    96,24, 0,
    104,24, 0,
    112,24, 0,
    120,24, 0,
    128,24, 0,
    136,24, 0,
    144,24, 0,
    152,24, 0,
    160,24, 0,
    168,24, 0,
    176,24, 0,
    184,24, 0,
    192,24, 0,
    200,24, 0,
    208,24, 0,
    216,24, 0,
    224,24, 0,
    232,24, 0,
//#define  PS_LIVES		86
    126,200, 0
//#define PS_STATUS		87

//#define  PS_LASTSPRITE  114
};


//-------------------------------------------------------------------
// SetupPlayScreen	$0000	mode 7 screens / tiles
//							$4000	mode 1 tiles
//							$5C00	BG1 sc1
//							$6000	sprites
//-------------------------------------------------------------------

unsigned sizeshift[8] = {1,3,5,7,9,11,13,15};

void SetupPlayScreen (void)
{
	unsigned	i,j;
	char		*sprinfo;
	char		screenmap[16];

	inidisp = 0x80;	// Forced blank
	hdmaen = 0;			// Turn of HDMA
	nmitimen = 0;		// Turn off interrupts

	DmaCGram (&playpal, 0, 512);	// Set the palette for the tile set
	ClearVram ();
	tm = 0x11;			// Show OBJ and BG1
	bgmode = 7;

	cgswsel = 0;		// Enable color constant addition
	cgadsub = 31;		// Do NOT add color to background

	bg1hofs = 0;		// Low byte...
	bg1hofs = 0;		// ...then high byte
	bg1vofs = 0;
	bg1vofs = 0;

//-------------------
// Mode 7 portion
//-------------------

	memset (screenmap,0,sizeof(screenmap));
	DmaMode7Screens (screenmap, 0, 16);
	for (i=0 ; i<10 ; i++)
	{
		for (j=0 ; j<14 ; j++)
			screenmap[j+1] = j*16 + i + 1;
		DmaMode7Screens (screenmap, (i+1)*128, 16);
	}

//-------------------
// Status bar
//-------------------

	for (i=0 ; i<2 ; i++)
	{
		for (j=0 ; j<14 ; j++)
			screenmap[j+1] = 14*16+i*14+j+1;
		DmaMode7Screens (screenmap, (i+11)*128, 16);
	}

	DmaMode7Tiles (&m7tiles,(14*16+1)*64,28*64);


// Erase screen buffer to background for first frame

	memcpy (&screenbuffer, &playback, SCREENWIDTH*SCREENHEIGHT);

	m7a = 0x80;		// Set to 200% scale
	m7a = 0;
	m7d = 0x80;
	m7d = 0;

//-------------------
// Sprites
//-------------------

	objsel = 3;										// Start object data at 24kw, small size
	DmaVram (&psprites, 0x6000, 0x2000);	// Fill 4kw with sprites
	DmaVram (&statspr, 0x6000+64*16,32*32);// Overwrite lowercase font
	memset (&oam, 0, sizeof(oam) );			// Weapons are loaded later

	sprinfo = playsprites;
	for (i=0 ; i< PS_LASTSPRITE ; i++)	// Set sprite positions & sizes
	{
		oam.s[i].x = *sprinfo++;
		oam.s[i].y = *sprinfo++ -1;	// -1 to get from BG coordinates
		oam.s[i].flags = 0x3E;        // Priority 2, palette 7
//		oam.s[i].name = 1;
		oam.bflags[i>>3] |= (*sprinfo++) << sizeshift[i&7];	// Size bit
	}

	for (i= PS_GUN ; i<PS_GUN+16 ; i++)	// Set guns to use 2nd bank
		oam.s[i].flags |= 1;		// High bit of name

	for (i=PS_STATUS ; i<PS_LASTSPRITE ; i++)	// Status bar titles
	{
		oam.s[i].x = 8*(i-PS_STATUS+2);
		oam.s[i].y = 176;
		oam.s[i].name = 64+i-PS_STATUS;
	}
	oam.s[PS_STATUS+14].x = 23*8;		// Move the percent sign
	oam.s[PS_STATUS+14].y += 12;
	oam.s[PS_STATUS+15].x = 23*8;
	oam.s[PS_STATUS+15].y += 20;

	for (i=PS_LASTSPRITE ; i<128 ; i++)
	     oam.s[i].y = 240;		// Off screen

	DmaOAram (&oam, 0, sizeof(oam));		// Upload sprite records

	currentweapon = -1;	// First weapon set will change

//-------------------
// HDMA
//-------------------

	SetupHDMA();		// Moved to IOSNES.ASM

//-------------------
// Interrupt handler
//-------------------

	irqhook = PlayScreenIRQ;
	vtime = 207;		// This is hand adjusted to account for
							//   overhead in the IRQ handler before
							//   the forced blank is set
	sendoam = 0;
	sendweapon = 0;
	sendplayscreen = 0;
	nmitimen = 0x21;	// Turn vertical IRQ and joystick ON

//-------------------
// Done
//-------------------

	firstframe = 1;	// Fade in after drawing 1st frame
							// FIXME: 'firstframe' not used anymore?
	SetupScalers ();	// Copy scalers to ram bank (toasted by decompress)
	UnPauseSong ();
}


//-------------------------------------------------------------------
// SampleControls		Sample angular input every frame
//-------------------------------------------------------------------

ushort	olddir;
ushort	speed;
ushort	heldtime;

ushort	oldmbuttons, oldmtime;
ushort	mouseclicks;

#define CLICKTICS  20
void SampleControls (void)
{
	ushort	newdir,speed, clicktime;

	if (demomode == dm_playback)
		return;

	if (mousepresent)
	{
	    /* forward / back move */
	    if (mouse3 & 0x80)
		    mousey -= mouse3&0x7f;
	    else
		    mousey += mouse3;

		if (! (mouse4 & 0x40) )
		{    /* if & 0x40, it was a screwed up read */
			if (mouse4 & 0x80)
			{
			    mousex -= mouse4&0x7f;
			   irqangle += (mouse4&0x7f)<<7;
		    }
		   else
		   {
			    mousex += mouse4;
			   irqangle -= mouse4<<7;
		    }
		}

	/* double click to open door */
	    if ( (mouse1 ^ oldmbuttons) & mouseb2)
	    {
		    oldmbuttons = mouse1;
		    clicktime  = ticcount - oldmtime;
		    oldmtime = ticcount;
		    if (clicktime > CLICKTICS)
			    mouseclicks = 0;
		    if (mouse1 & mouseb2)
			    mouseclicks++;      /* just pressed */
	    }
	}

	if (joystick1 & JOYPAD_LFT)
		newdir = 2;
	else if (joystick1 & JOYPAD_RGT)
		newdir = 1;
	else
		newdir = 0;

	if (newdir != olddir)
	{
		heldtime = 0;
		olddir = newdir;
		if (!newdir)
		{    /* center in a displayable angle */
			irqangle &= 0xfc00;
			irqangle += 0x0200;
			return;
		}
	}
	else
	{
		if (joystick1 & (JOYPAD_UP|JOYPAD_DN))
			speed = 256;
		else if ( joystick1 & (JOYPAD_Y|JOYPAD_X) )
			speed = 512;
		else
		{
			heldtime++;
			if (heldtime > 32)
				speed = 512;
			else if (heldtime > 16)
				speed = 384;
			else
				speed = 256;
		}
		if (newdir == 2)
			irqangle += speed;
		else if (newdir == 1)
			irqangle -= speed;
	}
}


//-------------------------------------------------------------------
// PlayScreenIRQ
//-------------------------------------------------------------------

unsigned	oldticcount;

void PlayScreenIRQ (void)
{
	ushort	tics;

	playtime++;
	extratics++;
	BlankVideo ();
	if (sendplayscreen)
	{
		tics = ticcount-oldticcount;
		if (tics<4 && !timing)		// Don't go faster than 15 fps
			goto notpscreenupload;

//		extratics += tics-4;
		BlitPlay ();
		oldticcount = ticcount;
		sendplayscreen = 0;

#if 0
// Show frame tics
oam.s[PS_TEXT+27].name = '0'-32 + tics;
sendoam = 1;
#endif

		goto done;
	}
	else
	{
notpscreenupload:
		if (setadd)		// Set constant color add
		{
			setadd = 0;
			coldata = 0x20 + redadd;
			coldata = 0x40 + greenadd;
			coldata = 0x80 + blueadd;
		}
		if (sendweapon)
		{
			DmaVram (
			(void *)
			((unsigned long)&weapon1 + ((unsigned long)currentweapon<<13)),
			0x7000,0x2000);
			
//			DmaVram (&weapon1+(currentweapon<<13),0x7000,0x2000);	// Upload 256 sprites
			sendweapon = 0;
		}
		else if (sendoam)
		{
			DmaOAram (&oam, 0, sizeof(oam));	// Upload sprite records
			sendoam = 0;
		}
	}

	UnBlankVideo ();
done:;
//	SampleControls ();
//	MusicTic ();
}



//-------------------------------------------------------------------
// SetupIntroScreen		$0000	Mode 7 screens / tiles
//								$4000	Mode 1 tiles
//								$5C00	BG1 sc1
//								$6000	Sprites
//-------------------------------------------------------------------

extern void IntroIRQ (void);

void SetupIntroScreen (void)
{
	unsigned       i,j;
	char           *sprinfo;
	char           screenmap[16];

	inidisp = 0x80;	// Forced blank
	hdmaen = 0;			// Turn off HDMA
	nmitimen = 0;		// Turn off interrupts

	DmaCGram (&playpal, 0, 512);	// Set the palette for the tile set

	ClearVram ();
	tm = 0x11;			// Show OBJ and BG1
	bgmode = 7;

	cgswsel = 0;		// Enable color constant addition
	cgadsub = 31;		// Do NOT add color to background

	bg1hofs = 0;		// Low byte,
	bg1hofs = 0;		// then high byte
	bg1vofs = 0;
	bg1vofs = 0;

//-------------------
// Mode 7 portion
//-------------------

	memset (screenmap,0,sizeof(screenmap));
	DmaMode7Screens (screenmap, 0, 16);
	for (i=0 ; i<10 ; i++) {
		for (j=0 ; j<14 ; j++)
			screenmap[j+1] = j*16 + i + 1;
		DmaMode7Screens (screenmap, (i+1)*128, 16);
	}

// Erase screen buffer to black

	memset (&screenbuffer, 0, SCREENWIDTH*SCREENHEIGHT);

	m7a = 0x80;			// Set to 200% scale (X)
	m7a = 0;				// $100 = Normal, $80 = 2x, $200 = 1/2?
	m7d = 0x80;			// Low byte
	m7d = 0;				// High byte

//-------------------
// Sprites
//-------------------

	objsel = 3;										// Start object data at 24kw, small size
	DmaVram (&psprites, 0x6000, 0x2000);	// Fill 4kw with sprites
	memset (&oam, 0, sizeof(oam));			// Weapons are loaded later
	DmaOAram (&oam, 0, sizeof(oam));			// Upload sprite records

//-------------------
// HDMA
//-------------------

	SetupHDMA();		// Moved to IOSNES.ASM

//-------------------
// Interrupt handler
//-------------------

	irqhook = IntroIRQ;
	vtime = 207;		// This is hand adjusted to account for
							//   overhead in the IRQ handler before
							//   the forced blank is set
	sendoam = 0;
	sendplayscreen = 0;
	nmitimen = 0x21;	// Turn vertical IRQ and joystick ON
}


//-------------------------------------------------------------------
// IntroIRQ
//-------------------------------------------------------------------

void IntroIRQ (void)
{
	if (sendplayscreen) {
		BlankVideo ();
		BlitPlay ();
		sendplayscreen = 0;
	} else if (sendoam) {
		BlankVideo ();
		DmaOAram (&oam, 0, sizeof(oam));	// Upload sprite records
		UnBlankVideo ();
		sendoam = 0;
	}
}


//===================================================================
// Intermission
//===================================================================


//-------------------------------------------------------------------
// SetupIntermission		$0000	Sprites / tileset
//								$0C00	Additional BG1 tiles
//								$7800	BG2  sc1
//								$7C00	BG1  sc1
//-------------------------------------------------------------------

void ScrollIRQ (void);

void SetupIntermission (void)
{
	unsigned	static	backscreen[32][32];
	unsigned				i,j;
	
	memset (&oam,0,sizeof(oam));
	for (i=0 ; i<128 ; i++)
	{
		oam.s[i].flags = 0x32;	// Priority 3, palette 1
		oam.s[i].y = 248;
	}

	for (i=0 ; i<32 ; i++)
		for (j=0 ; j<32 ; j++)
			backscreen[i][j] = ((i&7)<<3) + (j&7) + 0xc0 + (5<<10);

	inidisp = 0x80;	// Forced blank
	hdmaen = 0;			// Turn off HDMA
	nmitimen = 1;		// Turn off interrupts
	bgmode = 1;			// Mode 1, 8 by 8 tiles
	bg1sc = 0x7C;		// Screen address $7C00, size 0
	bg2sc = 0x78;		// Screen address $7800, size 0
	tm = 0x13;			// Show OBJ, BG1, and BG2
	bg12nba = 0x00;	// BG1 and BG2 name = 0kw
	objsel = 0;			// Start object data at 0kw, small size

	ClearVram ();
	DmaCGram (&interpal, 0, 512);				// Set the palette for the tile set
	DmaOAram (&oam, 0, sizeof(oam));			// Upload sprite records
	DmaVram (&psprites, 0x0000, 0x2000);	// Fill 4kw with sprites
	DmaVram (&intertiles, 0x0C00, 0x8000);	// Fill 16kw with characters
	DmaVram (backscreen, 0x7800, 0x800);	// Fill 1kw with backscreen
	DmaVram (tilescreen, 0x7C00, 0x800);	// Fill 1kw with tilescreen

	irqhook = ScrollIRQ;
	vtime = 243;
	nmitimen = 0x21;	// Turn vertical IRQ and joystick ON
	FadeIn ();
}


//===================================================================
// Auto map
//===================================================================


//-------------------------------------------------------------------
// SetupAutoMap	$0000  BG1  sc0
//						$0400  BG1  sc1
//						$0800  BG1  sc2
//						$0C00  BG1  sc3
//						$1000  BG1  tileset
//-------------------------------------------------------------------

void SetupAutoMap (void)
{
	unsigned	i,j;
	char		*sprinfo;

	PauseSong ();

	inidisp = 0x80;	// Forced blank
	hdmaen = 0;			// Turn off HDMA
	nmitimen = 0;		// Turn off interruptes

	DmaCGram (&playpal, 0, 512);	// Set the palette for the tileset
	ClearVram ();
	tm = 0x11;			// Show BG1 and sprites
	bgmode = 3;

//-------------------
// Mode 3 portion
//-------------------

	bg12nba = 0x71;	// BG1 name = 1000kw, bg2 name = 0x7000w
	DmaVram (&mapicons, 0x1000, 0x1000);	// Send tileset
	bg1sc = 0x3;		// Screen address $0000, size 3

//-------------------
// Sprites
//-------------------

	objsel = 3;			// Start object data at 24kw, small size
	DmaVram (&psprites, 0x6000, 0x2000);	// Fill 4kw with sprites
	memset (&oam, 0, sizeof(oam) );			// Weapons are loaded later
	DmaOAram (&oam, 0, sizeof(oam));			// Upload sprite records

//-------------------
// Done
//-------------------
	
	irqhook = 0;

	vtime = 243;
	nmitimen = 0x21;	// Turn vertical IRQ and joystick ON
}


//-------------------------------------------------------------------
// RunAutoMap
//-------------------------------------------------------------------

#define  MAXHPOS   255
#define  MAXVPOS   287

void RunAutoMap (void)
{
	int		hpos, vpos;
	unsigned	screendest;

	unsigned	i,x,y,xb,yb, vx, vy;

	unsigned	oldjoystick1;
	int		code;
	unsigned	tile;
	unsigned	savedangle;

	savedangle = irqangle;

	FadeOut ();
	SetupAutoMap ();

//-------------------------
// Print password
//-------------------------

	spron = 0;
	SprPrint (7,2,"FLOOR CODE: ");
	SprPrint(19,2,password);
	DmaOAram (&oam, 0, sizeof(oam));		// Upload sprite records

//=========================
// Find out which tiles
// have been seen...
//=========================

//-------------------------
// Upload 4 screens of data
//-------------------------

	screendest = 0;
	vx = viewx>>8;
	vy = viewy>>8;

	for (yb=0 ; yb<=32 ; yb+=32)
		for (xb=0 ; xb<=32 ; xb+=32) {

			DrawAutomap (xb,yb);
#if 0
			memset (tilescreen,0,sizeof(tilescreen));
			for (y=0 ; y<32 ; y++)
				for (x=0 ; x<32 ; x++) {
					tile = map.tilemap[y+yb][x+xb];
					if (tile & 0x80)
						tilescreen[y][x] = tile&0x7f;
				}
#endif
	// Send one 32*32 tilescreen

			if (vx >= xb && vx < xb+32 && vy >= yb && vy < yb+32)
				tilescreen[vy-yb][vx-xb] =
					32+ ( (((ANGLES+ANGLES/16-1-viewangle) >>4) + 2) & 7 );
			DmaVram (tilescreen, screendest, 0x800);
			screendest += 0x400;
		}

//-------------------------
// Let player scroll around
// until START is pressed..
//-------------------------

	hpos = ((viewx>>5)&~1)-128;	// Go from 8 bit frac to 3 bit (tile8)
	vpos = ((viewy>>5)&~1)-112;
	if (hpos < 0)
		hpos = 0;
	else if (hpos > MAXHPOS)
		hpos = MAXHPOS;
	if (vpos < 0)
		vpos = 0;
	else if (vpos > MAXVPOS)
	vpos = MAXVPOS;

	bg1hofs = hpos;		// Set scroll registers
	bg1hofs = hpos>>8;
	bg1vofs = vpos;
	bg1vofs = vpos>>8;

	FadeIn ();

	oldjoystick1 = 0;
	code = 0;
	joystick1 = 0;

	do {
		if (joystick1 & JOYPAD_START)
			break;

//-------------------------
// Secret code entering
//-------------------------

		if ( (joystick1&JOYPAD_B) && !(oldjoystick1&JOYPAD_B) )
			code = (code<<2) + 3;
		if ( (joystick1&JOYPAD_A) && !(oldjoystick1&JOYPAD_A) )
			code = (code<<2) + 2;
		if ( (joystick1&JOYPAD_DN) && !(oldjoystick1&JOYPAD_DN) )
			code = (code<<2) + 1;
		if ( (joystick1&JOYPAD_TL) && !(oldjoystick1&JOYPAD_TL) )
			code = (code<<2) + 0;
		if ( joystick1& (JOYPAD_X|JOYPAD_DN|JOYPAD_LFT|JOYPAD_RGT|JOYPAD_SELECT|JOYPAD_TL) )
			code = -1;

//-------------------------
// Scrolling
//-------------------------

		if (mousepresent) {
			if (mouse3 & 0x80)
				vpos -= mouse3&0x7f;
			else
				vpos += mouse3;
			if (mouse4 & 0x80)
				hpos -= mouse4&0x7f;
			else
				hpos += mouse4;
		}

		if (joystick1 & JOYPAD_LFT)
			hpos-= 2;
		if (joystick1 & JOYPAD_RGT)
			hpos+= 2;
		if (joystick1 & JOYPAD_UP)
			vpos-= 2;
		if (joystick1 & JOYPAD_DN)
			vpos+= 2;

		if (hpos <0)
			hpos = 0;
		if (hpos > MAXHPOS)
			hpos = MAXHPOS;
		if (vpos <0)
			vpos = 0;
		if (vpos > MAXVPOS)
			vpos = MAXVPOS;

		oldjoystick1 = joystick1;
		ticcount = 0;
		while (!ticcount)
			;		// Sync up with video

		bg1hofs = hpos;		// Set scroll registers
		bg1hofs = hpos>>8;
		bg1vofs = vpos;
		bg1vofs = vpos>>8;
	} while (1);

	FadeOut ();
	SetupPlayScreen ();
	RedrawStatusBar ();
	extratics = 0;				// Don't skip any frames
	mousey = 0;
	irqangle = savedangle;

//-------------------------
// Handle secret codes
//-------------------------

	switch (code) {
		case 0x1B:		// STUFF: TL D A B
			gamestate.machinegun = gamestate.chaingun
			= gamestate.flamethrower = gamestate.missile = 1;
			gamestate.ammo = gamestate.maxammo = 299;
			gamestate.gas = 99;
			gamestate.missiles = 9;
			gamestate.health = 100;
			gamestate.keys = 3;
#ifndef	ID_VERSION
			gamestate.automap = true;
#endif
			break;
		case 0x62:		// FINISH: D A TL A
			playstate = EX_COMPLETED;
			break;
		case 0x9B:		// GOD: A D A B
			godmode ^= 1;
			break;
		case 0xF6:		// MAP: B B D A
	   	for (i=0 ; i<map.numnodes ; i++)
				if (nodes[i].dir & DIR_SEGFLAG)
					nodes[i].dir |= DIR_SEENFLAG;
			break;
	}
}


//===================================================================
// TitleScreen
//===================================================================

//-------------------------------------------------------------------
// SetupTextPicScreen	Call with the text already drawn into tilescreen
//								the text must be drawn with the priority bit set!
//
//								$0000	BG1 tileset
//								$6C00	BG1 sc0 (background pic)
//								$7000	BG2 tileset (128 16 color chars)
//								$7800	BG2 sc0 (empty)
//								$7C00	BG2 sc1 (text message)
//-------------------------------------------------------------------

void SetupTextPicScreen (byte *tileset, byte *palette, int height)
{
	unsigned	i,j, tile;
	char		*sprinfo;
	unsigned	screenmap[32];

	inidisp = 0x80;	// Forced blank
	hdmaen = 0;			// Turn off HDMA
	nmitimen = 0;		// Turn off interrupts

	ClearVram ();

	DmaCGram (palette, 0, 512);	// Set the palette for the tile set
	ClearVram ();
	tm = 0x13;			// Show BG1, BG2, and SPRITES
	bgmode = 3;

	bg2vofs = 0;		// Low byte,
	bg2vofs = 0;		// then high byte

//-------------------------
// Mode 3 portion
//-------------------------

	bg12nba = 0x70;	// BG1 name = 0x0000; BG2 name = 0x7000
	DmaVram (tileset, 0, 0xD800);				// BG1 tileset
	DmaVram (&psprites, 0x7000, 0x1000);	// BG2 tileset

	bg2sc = 0x78+2;	// Screen address = 0x7800, size 2
	DmaVram (tilescreen, 0x7C00, 0x800);	// Text screen

	bg1sc = 0x6C+2;	// Screen address = 0x7800?, size 2
	tile = 1;
	for (i=0; i<height; i++)
	{
		for (j=0; j<32; j++)
			screenmap[j] = tile++;
		DmaVram (screenmap, 0x6C00+(i+2)*32, 64);	// Picture screen
	}

//-------------------------
// Sprites
//-------------------------

	objsel = 3;			// Start object data at 24kw, small size
	memset (&oam, 0, sizeof(oam));

	for (i=0; i<128; i++)	// Set sprite positions & sizes
	{
		oam.s[i].y = 240;			// Off screen
		oam.s[i].flags = 0x3F;	// Priority 2, palette 7, second bank
	}

	DmaOAram (&oam, 0, sizeof(oam));	// Upload sprite records

//-------------------------
// Interrupt handler
//-------------------------

	irqhook = 0;

	vtime = 243;
	nmitimen = 0x21;	// Turn vertical IRQ and joystick ON
}


//-------------------------------------------------------------------
// TitleScreen
//-------------------------------------------------------------------

#ifdef	DEBUG
extern char CompileDate[];		//jgt
#endif

void TitleScreen (void)
{
	int	i;

	memset (tilescreen,0,sizeof(tilescreen));
	SetupTextPicScreen (&credit, &credpal, 25);
	if (!sequence)
		StartSong (0, true);

#ifdef	DEBUG
//----------------------------------//jgt
	TilePrint (0x2000,6,25,CompileDate);
	DmaVram (tilescreen, 0x7c00, sizeof(tilescreen));
	bg2vofs = 255;
	bg2vofs = 0;		// 0 high byte
//----------------------------------//jgt
#endif

#ifndef	ID_VERSION
	TilePrint (0x2000, 3, 24, "\x7F""1994 by Wisdom Tree, Inc.");
	TilePrint (0x2000, 3, 25, "    All rights reserved.     ");

	DmaVram (tilescreen, 0x7c00, sizeof(tilescreen));
	bg2vofs = 255;
	bg2vofs = 0;		// 0 high byte
#endif

	FadeIn ();

	playstate = EX_COMPLETED;

	StartAck ();
	ticcount = 0;

	do {
		if (CheckAck ())
			break;
	} while (ticcount < 300);

	FadeOut ();
}


#ifdef STOREDEMO
//-------------------------------------------------------------------
// SellScreen
//-------------------------------------------------------------------

void SellScreen (void)
{
    int  i;

    memset (tilescreen,0,sizeof(tilescreen));
    SetupTextPicScreen (&storepic, &storepal, 25);

    FadeIn ();

    StartAck ();
    ticcount = 0;
    do
    {
         if ( CheckAck () )
              break;
    } while (ticcount < 300);

    FadeOut ();
    SetupPlayScreen ();
}
#endif


//-------------------------------------------------------------------
// SoundTest
//-------------------------------------------------------------------

void GridIRQ (void)
{
    DmaOAram (&oam, 0, sizeof(oam));   /* upload sprite records */
    if (sendtilescreen)
    {
         sendtilescreen = false;
         DmaVram (tilescreen, 0x7c00, sizeof(tilescreen));
    }
}


//-------------------------------------------------------------------
// GridSelect		minx/miny in pixels
//						width/height in selection units (24*16)
//						startpos is a selection number
//-------------------------------------------------------------------

int GridSelect (unsigned minx, unsigned miny, unsigned width, unsigned height
, unsigned startpos)
{
	unsigned	maxx, maxy;
	unsigned	bullety, oldticcount, destx, desty;

	WaitUp ();

	irqhook = GridIRQ;

	maxx = minx + (width-1)*24;
	maxy = miny + (height-1)*16;

	destx = minx + (startpos%width)*24;
	desty = miny + (startpos/width)*16;
	oam.s[0].x = destx;
	oam.s[0].y = desty;

	mousex = 0;
	mousey = 0;

	do {
		oam.s[1].x = oam.s[0].x+8;
		oam.s[1].y = oam.s[0].y;
		oam.s[0].name = 0x6E + (ticcount&16);
		oam.s[1].name = 0x6F + (ticcount&16);
		oldticcount = ticcount;
		while (ticcount==oldticcount)
		    ;
#define  MOUSEDELTA     0x20
		if (mousex > MOUSEDELTA) {
			mousex-=MOUSEDELTA;
			joystick1 |= JOYPAD_RGT;
		}
		if (mousey > MOUSEDELTA) {
			mousey-=MOUSEDELTA;
			joystick1 |= JOYPAD_DN;
		}
		if (mousex < -MOUSEDELTA) {
			mousex+=MOUSEDELTA;
			joystick1 |= JOYPAD_LFT;
		}
		if (mousey < -MOUSEDELTA) {
			mousey+=MOUSEDELTA;
			joystick1 |= JOYPAD_UP;
		}

		if (oam.s[0].y < desty)
			oam.s[0].y+=2;
		else if (oam.s[0].y > desty)
			oam.s[0].y-=2;
		else if (oam.s[0].x < destx)
			oam.s[0].x+=2;
		else if (oam.s[0].x > destx)
			oam.s[0].x-=2;
		else {
			if (width == 1 && (joystick1 & (JOYPAD_LFT|JOYPAD_RGT)))
				break;	// Side switching main menu

			if ( (joystick1 & JOYPAD_UP) && (desty > miny) )
				desty -= 16;
			else if ( (joystick1 & JOYPAD_DN) && (desty < maxy) )
				desty += 16;
			if ( (joystick1 & JOYPAD_LFT) && (destx > minx) )
				destx -= 24;
			else if ( (joystick1 & JOYPAD_RGT) && (destx < maxx) )
				destx += 24;
			if (!(joystick1 &(JOYPAD_RGT|JOYPAD_LFT|JOYPAD_UP|JOYPAD_DN))) {
				FetchButtons ();
				if (checkbits)
					break;
			}
		}
	} while (1);

	irqhook = 0;

	return (desty-miny)/16*width + (destx-minx)/24;
}



//===================================================================
// StartGame
//===================================================================


//-------------------------------------------------------------------
// LevelWarp
//-------------------------------------------------------------------

void LevelWarp (void)
{
	unsigned  bullety, oldticcount, destx, desty;

	memset (tilescreen,0,sizeof(tilescreen));
	TilePrint (0x2000, 9,7,"Select Level");

	TilePrint (0x2000, 7,10,"11 12 1B 21 22 23");
	TilePrint (0x2000, 7,12,"2B 31 32 33 3B 3S");
	TilePrint (0x2000, 7,14,"41 42 43 44 4B 51");
	TilePrint (0x2000, 7,16,"52 53 54 55 5B 61");
	TilePrint (0x2000, 7,18,"62 63 64 65 6B 6S");

	SetupTextPicScreen (&credit, &dimpal, 25);
	bg2vofs = 255;
	bg2vofs = 0;		// 0 high byte
	NewGame ();			// Init basic game stuff
	FadeIn ();
	gamestate.mapon = GridSelect (7*8,10*8+4,6,5,0);
	FadeOut ();
	SetupPlayScreen ();
	GameLoop ();
	StopSong ();
}


boolean BadPass (void)
{
	TilePrint (0x2000, 7,20,"Invalid floor code");
	IO_WaitVBL(1);
	DmaVram (tilescreen, 0x7c00, sizeof(tilescreen));
	ack ();
	FadeOut ();
	return false;
}


//-------------------------------------------------------------------
// Password
//
//		Password format:
//			5	mapon
//			4	lives
//			5	ammo>>2
//			1   machinegun
//			1   chaingun
//			1   flamethrower
//			3   check bits: XOR of four previous nibbles
//			1   missilelauncher
//			3   check bits: ADD of four previous nibbles
//
//		mmmm mlll laaa aamg fccc mccc
//-------------------------------------------------------------------

#ifdef	ID_VERSION
unsigned char *passletters = "BCDFHJKLMNPQRSTV";
unsigned char *xormask = "NEXT!!";
#else
unsigned char *passletters = "BCDFHJKLMNPRSTWZ";
unsigned char *xormask = "NOAH3D";
#endif

boolean Password (void)
{
	unsigned	bullety, oldticcount, destx, desty;
	unsigned	pass[8], passchar;
	int		val, i;
	unsigned	mapon, lives, ammo;
	char		num[9];

	memset (tilescreen,0,sizeof(tilescreen));
	TilePrint (0x2000, 4,7,"Enter Floor Code: ______");

	TilePrint (0x2000, 11,10,"B  C  D  F");
	TilePrint (0x2000, 11,12,"H  J  K  L");
	TilePrint (0x2000, 11,14,"M  N  P  R");
	TilePrint (0x2000, 11,16,"S  T  W  Z");

	SetupTextPicScreen (&credit, &dimpal, 25);
	bg2vofs = 255;
	bg2vofs = 0;		// 0 high byte
	FadeIn ();

	passchar = 0;
	val = 0;
	do
	{
		IO_WaitVBL(1);
		sendtilescreen = true;
		if (passchar == 6)
			break;
		val = GridSelect (11*8-4,10*8+4,4,4,val);

		if (joystick1 & JOYPAD_START) {
		     FadeOut ();
		     return false;
		}

		if (joystick1 & JOYPAD_TL) {
		     if (passchar > 0) {
		          passchar--;
		          tilescreen[7][22+passchar] = '_'-32+0x2000;
		     }
		     continue;
		}

		password[passchar] = passletters[val];
		pass[passchar] = (val^xormask[passchar])&15;
		tilescreen[7][22+passchar] = passletters[val]-32+0x2000;

		passchar++;
	} while (1);

	WaitUp ();

//----------------------------
// Decode password
//----------------------------

	if ( ((pass[0] ^ pass[1] ^ pass[2] ^ pass[3])&7) != (pass[4]&7))
		return BadPass ();

	if ( ((pass[0] + pass[1] + pass[2] + pass[3])&7) != (pass[5]&7))
		return BadPass ();

//----------------------------
// mmmm mlll laaa aaaa cccc
//----------------------------

	mapon = (pass[0]<<1) + (pass[1]>>3);
	lives = ((pass[1]&7)<<1) + (pass[2]>>3);
	ammo = (((pass[2]&7)<<2) + (pass[3]>>2))<<2;

#ifdef STOREDEMO
	if (mapon > 2)
		return BadPass ();
#endif

	if (mapon > 30 || lives > 9 || ammo > 99)
		return BadPass ();
	if (pass[3]&2) {
		gamestate.machinegun = true;
		gamestate.weapon = gamestate.pendingweapon = WP_MACHINEGUN;
	}
	if (pass[5]&8) {
		gamestate.missile = true;
		gamestate.missiles = 10;
		gamestate.weapon = gamestate.pendingweapon = WP_MISSILE;
	}
	if (pass[4]&8) {
		gamestate.flamethrower = true;
		gamestate.gas = 30;
		gamestate.weapon = gamestate.pendingweapon = WP_FLAMETHROWER;
	}
	if (pass[3]&1) {
		gamestate.chaingun = true;
		gamestate.weapon = gamestate.pendingweapon = WP_CHAINGUN;
	}

	gamestate.mapon = mapon;
	gamestate.lives = lives;
	gamestate.ammo = ammo;

	password[6] = 0;

	FadeOut ();
	return true;
}


char password[8];

void CalcPassword (void)
{
	unsigned  i,mapon, lives, ammo;
	unsigned  pass[8];

//-------------------------------
// mmmm mlll laaa aamg fccc mccc
//-------------------------------

	mapon = nextmap;

	lives = gamestate.lives < 10 ? gamestate.lives : 9;
	ammo = gamestate.ammo < 100 ? gamestate.ammo : 99;
	ammo >>= 2;
	pass[0] = mapon>>1;
	pass[1] = ((mapon&1)<<3) + (lives>>1);
	pass[2] = ((lives&1)<<3) + (ammo>>2);
	pass[3] = (ammo&3)<<2;
	if (gamestate.machinegun || gamestate.flamethrower || gamestate.missile)
		pass[3] |= 2;
	if (gamestate.chaingun)
		pass[3] |= 1;
	pass[4] = (pass[0] ^ pass[1] ^ pass[2] ^ pass[3])&7;
	if (gamestate.flamethrower)
		pass[4] |= 8;
	pass[5] = (pass[0] + pass[1] + pass[2] + pass[3])&7;
	if (gamestate.missile)
		pass[5] |= 8;

	for (i = 0 ; i< 6 ; i++)
		password[i] = passletters[ (pass[i]^xormask[i])&15 ];
	password[i] = 0;
}


//-------------------------------------------------------------------
// DrawMenuScreen
//-------------------------------------------------------------------

void DrawMenuScreen (void)
{
	memset (tilescreen,0,sizeof(tilescreen));
	TilePrint (0x2000, 10,11,"Start Game");
	TilePrint (0x2000, 10,13,"Floor Code");

	switch (difficulty) {
		case 0:
			TilePrint (0x2000, 10,15,"Skill: easy");
			break;
		case 1:
			TilePrint (0x2000, 10,15,"Skill: normal");
			break;
		case 2:
			TilePrint (0x2000, 10,15,"Skill: hard");
			break;
	}

	if (stereosound)
		TilePrint (0x2000, 10,17,"Stereo: on");
	else
		TilePrint (0x2000, 10,17,"Stereo: off ");

	if (lefthandmouse)
		TilePrint (0x2000, 10,19,"Mouse: left");
	else
		TilePrint (0x2000, 10,19,"Mouse: right");

}


//-------------------------------------------------------------------
// StartGame
//-------------------------------------------------------------------

void StartGame (void)
{
	unsigned  menu;

	password[0] = '-';
	password[1] = 'N';
	password[2] = 'O';
	password[3] = 'N';
	password[4] = 'E';
	password[5] = '-';
	password[6] = 0;

	DrawMenuScreen ();

	SetupTextPicScreen (&credit, &dimpal, 25);
	irqhook = GridIRQ;
	if (!sequence)
		StartSong (0, true);

	bg2vofs = 255;
	bg2vofs = 0;		// 0 high byte
	FadeIn ();

	menu = 0;

getmenu:

	DrawMenuScreen ();
	sendtilescreen = true;
	IO_WaitVBL(1);
//	DmaVram (tilescreen, 0x7c00, sizeof(tilescreen));

	menu = GridSelect (8*8,11*8,1,5, menu);

	if (menu == 2) {
		if (joystick1 & JOYPAD_LFT) {
			if (difficulty == 0)
				difficulty = 2;
			else
				difficulty--;
		} else {
		    difficulty++;
		    if (difficulty == 3)
			    difficulty = 0;
		}
		goto getmenu;
	}

	if (menu == 3) {
		stereosound ^= 1;
		goto getmenu;
	}

	if (menu == 4) {
		lefthandmouse ^= 1;
		if (lefthandmouse) {
			mouseb1 = 0x80;
			mouseb2 = 0x40;
		} else {
			mouseb1 = 0x40;
			mouseb2 = 0x80;
		}
		goto getmenu;
	}

	FadeOut ();
	gamestate.mapon = 0;
	nextmap = 0;
	NewGame ();			// Init basic game stuff

	if (menu == 1) {
		if (!Password ())
			return;		// Invalid password
	}

	SetupPlayScreen ();
	GameLoop ();
	StopSong ();
}


//-------------------------------------------------------------------
// Briefing
//-------------------------------------------------------------------


char *mission1text[] = {
	"You'll be out of the ark",
	"in six days, Noah.",
	"Unfortunately, the animals",
	"are a tad bit restless and",
	"want to get out now. Good",
	"thing you brought all that",
	"food with you. You'll need",
	"it to put the busy ones to",
	"sleep.",
	"",
	"At the end of the first",
	"day, be prepared to deal",
	"with Carl the Camel.",
	"He's been real cranky",
	"lately and is a bit out",
	"of control. Good luck and", 
	"be careful...",
	"*"
};

char *mission2text[] = {
	"Wow, Noah! Carl sure was",
	"hard to calm down. But",
	"remember, you were chosen",
	"to guide this ark to",
	"safety beacause you know",
	"how to get the job done.",
	"Your family and the other",
	"animals are counting on",
	"you.",
	"",
	"The closing of day two",
	"ends with irritable",
	"Ginny the Giraffe. She may",
	"be tall and quick, but",
	"give her enough food and",
	"it's off to sleep. Oh, and",
	"Noah, remember to keep an",
	"eye out for hidden rooms...",
	"*"
};

char *mission3text[] = {
	"ZZZZZ... Great job, Noah!.",
	"Ginny is snoring away.",
	"",
	"Boy it seems the animals",
	"never seem to stay asleep",
	"very long.",
	"",
	"Keep your eye out for",
	"Melvin the Monkey. He can",
	"be pretty tricky and may",
	"try distracting you with",
	"coconuts, but it's your",
   "job to get him settled down",
	"for the rest of your time",
	"aboard the ark.",
	"*"
};

char *mission4text[] = {
	"You're doing a great job,", 
	"Noah! You took care of that",
	"Melvin like a real pro.",
	"",
	"I hope you haven't worn",
	"yourself out yet, because",
	"more challenges are ahead.",
	"",
	"Kerry the Kangaroo awaits",
	"you, and she doesn't look",
	"the least bit tired. Are",
	"you ready for her? She",
	"needs to rest like the",
	"other animals.",
	"*"
};

char *mission5text[] = {
	"Wow! Kerry was no challenge",
	"for you!",
	"",
	"Can you keep up the pace",
	"when faced with Ernie the",
	"Elephant?",
	"",
	"Don't worry. Very soon you",
	"you will be on dry land,",
	"and you won't have to",
	"chase the animals around",
	"the ark anymore.",
	"*"
};

char *mission6text[] = {
	"You are almost there!",
	"If you can get through one",
	"more day, you will be out",
	"of here for good!",
	"",
	"Rumor has it one of the",
	"bears does not want to be",
	"found. In fact he may try",
	"to hide from you. Be on the",
	"alert for Burt the Bear.",
	"Get him to sleep and your",
	"work is done!",
	"*"
};

char	**missiontext[6] = {mission1text, mission2text, mission3text,
								  mission4text, mission5text, mission6text};

unsigned	vpos, stop;

void BriefIRQ (void)
{
	if (vpos <= stop) {
		bg2vofs = vpos;
		bg2vofs = 0;		// 0 high byte
		vpos++;
	}
}

void Briefing (ushort mission)
{
#ifdef JAPVERSION
	char	text[20];

	memset (tilescreen,0,sizeof(tilescreen));
	SetupTextPicScreen (&brief, &briefpal, 24);
	bg2vofs = 255;
	bg2vofs = 0;
	memset (tilescreen,0,sizeof(tilescreen));

	switch (mission) {
		case 0:
			DrawTilePic (MISSION1_START, 16-MISSION1_W/2,8, MISSION1_W,MISSION1_H);
			break;
		case 1:
			DrawTilePic (MISSION2_START, 16-MISSION2_W/2,8, MISSION2_W,MISSION2_H);
			break;
		case 2:
			DrawTilePic (MISSION3_START, 16-MISSION3_W/2,8, MISSION3_W,MISSION3_H);
			break;
		case 3:
			DrawTilePic (MISSION4_START, 16-MISSION4_W/2,8, MISSION4_W,MISSION4_H);
			break;
		case 4:
			DrawTilePic (MISSION5_START, 16-MISSION5_W/2,8, MISSION5_W,MISSION5_H);
			break;
		case 5:
			DrawTilePic (MISSION6_START, 16-MISSION6_W/2,8, MISSION6_W,MISSION6_H);
			break;
	}

	DmaVram (tilescreen, 0x6C00, sizeof(tilescreen));    /* picture screen */
	FadeIn ();
	StartSong (1,true);
#endif


#ifndef JAPVERSION
	unsigned	line;
	char		**lines, *text;

//----------------------------
// Draw text into tilescreen
//----------------------------

	lines = missiontext[mission];

	memset (tilescreen, 0, sizeof(tilescreen));
	line = 0;

	do {
		text = lines[line];
		if (*text == '*')
			break;
		TilePrint (0x2000, 3, line, text);
		line++;
	} while (1);

	SetupTextPicScreen (&brief, &briefpal, 24);
	vpos = 0;
	stop = 256 - (28-line)*4;
	irqhook = BriefIRQ;
	FadeIn ();
	StartSong (8,true);

//----------------------------
// Scroll the screen up
//----------------------------

	StartAck ();

	do {
		if (CheckAck ()) {
			StopSong ();
			FadeOut ();
			SetupPlayScreen ();
			return;
		}

	} while (vpos < stop);
#endif	// not JAPVERSION

	ack ();
	StopSong ();
	FadeOut ();
	SetupPlayScreen ();
}

//===================================================================
// The demos are packed in by hand after the main program


#define  DEMO1		0xC0E000
#define  DEMO2		0xC0F000
#define	DEMOREC	0x7FF000


#ifdef	RECORDDEMO

unsigned demo_size;

//-------------------------------------------------------------------
// RecordDemo
//-------------------------------------------------------------------

void RecordDemo (void)
{
	memset (tilescreen,0,sizeof(tilescreen));
	TilePrint (0x2000, 9,7,"Select Level");

	TilePrint (0x2000, 7,10,"11 12 1B 21 22 23");
	TilePrint (0x2000, 7,12,"2B 31 32 33 3B 3S");
	TilePrint (0x2000, 7,14,"41 42 43 44 4B 51");
	TilePrint (0x2000, 7,16,"52 53 54 55 5B 61");
	TilePrint (0x2000, 7,18,"62 63 64 65 6B 6S");

	SetupTextPicScreen (&credit, &dimpal, 25);
	bg2vofs = 255;
	bg2vofs = 0;		// 0 high byte
	FadeIn ();
	gamestate.mapon = GridSelect (7*8,10*8+4,6,5,0);
	FadeOut ();

	NewGame ();
	SetupPlayScreen ();
	FadeIn ();
	rndindex = 0;

	demo_p = (byte *)DEMOREC;
	demomode = dm_recording;

	*demo_p++ = gamestate.mapon;

	PrepPlayLoop ();
	IO_Print ("Demo Recording");
	ack ();
	PlayLoop ();
	IO_PrintNums ("demo_p:",(int) ((long) demo_p & 0xFFFF),0,0);
	demo_size = demo_p - (byte *) DEMOREC;
	ack ();
#ifdef	ID_VERSION
	ack ();
	ack ();
#endif
	FadeOut ();
	demomode = dm_none;
}


//-------------------------------------------------------------------
// DumpDemo
//-------------------------------------------------------------------

void DumpDemo (byte *data)
{
	unsigned	offset;
	int		x, y;
	char		num[10], str[80];

	offset = 0;

	FadeIn ();

	do
	{
		BlankVideo ();
		memset (tilescreen,0,sizeof(tilescreen));
		TilePrint (0x2000, 10, 3,"Demo Data");

		strcpy (str, "Size:");
		itox (demo_size, num);
		strcat (str, num);
		TilePrint (0x2000, 2, 5, str);

		spron = 0;

		for (y = 0; y < 16; y++)
		{
			str[0] = '\0';
			itox (offset+(y * 8), num);
			strcat (str, num);
			strcat (str, ":");	// Address

			for (x = 0; x < 8; x++)
			{
				if (!(x % 2))
					strcat (str, " ");

				itox (*(data+offset+(y*8)+x), num);
				num[0] = num[4];
				num[1] = num[5];
				num[2] = '\0';
				strcat (str, num);
			}

			TilePrint (0x2000, 1, y+7, str);
		}

		SetupTextPicScreen (&credit, &dimpal, 25);
		bg2vofs = 255;
		bg2vofs = 0;		// 0 high byte
		UnBlankVideo ();

wait:
		joystick1 = 0;
		while (!joystick1)
			;

		if (joystick1 & JOYPAD_START)
			break;
		else if (joystick1 & JOYPAD_UP)
			offset -= 0x0080;
		else if (joystick1 & JOYPAD_DN)
			offset += 0x0080;
		else
			goto wait;

	} while (1);

	FadeOut ();
}
#endif


//-------------------------------------------------------------------
// PlayDemo
//-------------------------------------------------------------------

void PlayDemo (byte *data)
{
	int	olddif;

	olddif = difficulty;
	difficulty = 1;

	NewGame ();
	SetupPlayScreen ();
	rndindex = 0;
	demo_p = data;
	demomode = dm_playback;

	gamestate.mapon = *demo_p++;
	PrepPlayLoop ();
	PlayLoop ();
	FadeOut ();
	demomode = dm_none;

	difficulty = olddif;
}


//===================================================================
// main		Called by reset vector after hardware initialization
//				Video is still blanked
//===================================================================

void main (void)
{
int	startbits;

	difficulty = 1;
	stereosound = true;

#ifdef	ID_VERSION
	SetupScalers ();
#endif
	StartupRendering ();

#ifdef	RECORDDEMO
	while (1)
	{
		RecordDemo ();
		PlayDemo ((byte *)DEMOREC);
		DumpDemo ((byte *)DEMOREC);
	}
#endif

	ticcount = 0;
	while (!ticcount)
		;
	startbits = joystick1;

#ifdef	ID_VERSION
	SystemCheck ();
#endif

#ifdef STOREDEMO
	SellScreen ();
#endif

#ifdef	ID_VERSION
	Intro ();
#else
	Disclaimer ();
#endif

	while (1)
	{
		TitleScreen ();

		if (ackhit)
		{
			if (startbits == JOYPAD_TL && pushbits == (JOYPAD_DN|JOYPAD_SELECT) )
			{
				StopSong ();
				SoundTest ();
			}
			else if (startbits == JOYPAD_TL && pushbits == (JOYPAD_RGT|JOYPAD_SELECT) )
				MusicTest ();
			else if (startbits == JOYPAD_TL && pushbits == (JOYPAD_UP|JOYPAD_SELECT) )
				LevelWarp ();
#if 0
			else if (startbits == JOYPAD_TL && pushbits == (JOYPAD_LFT|JOYPAD_SELECT) )
				InstrumentTest ();
#endif
			else
				StartGame ();
			continue;
		}

		PlayDemo ((byte *)DEMO1);
		if (playstate == EX_DEMOABORTED) {
			StartGame ();
			continue;
		}

//shit//fuck		Credits (false);
//shit//fuck		if (ackhit)
//shit//fuck		    StartGame ();

#ifdef	TWODEMOS
		PlayDemo ((byte *)DEMO2);
		if (playstate == EX_DEMOABORTED) {
			StartGame ();
			continue;
		}
#endif
	}
}
