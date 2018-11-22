/* iosnes.h */

/* joypad masks */

#define  JOYPAD_B			0x8000
#define  JOYPAD_Y			0x4000
#define  JOYPAD_SELECT	0x2000
#define  JOYPAD_START	0x1000
#define  JOYPAD_UP		0x0800
#define  JOYPAD_DN		0x0400
#define  JOYPAD_LFT		0x0200
#define  JOYPAD_RGT		0x0100
#define  JOYPAD_A			0x0080
#define  JOYPAD_X			0x0040
#define  JOYPAD_TL		0x0020
#define  JOYPAD_TR		0x0010


extern	unsigned volatile ticcount;
extern   unsigned volatile joystick1, joystick2;
extern   short volatile mousex, mousey, mousedx, mousedy;

extern   void (*irqhook) (void);
extern   void (*nmihook) (void);

typedef struct
{
    char      x, y;
    char      name, flags;
} spriterec_t;

typedef struct
{
    spriterec_t    s[128];
    unsigned       bflags[16];
} oam_t;

extern   oam_t    oam;

extern   unsigned tilescreen[32][32];

extern   unsigned char fadelevel;		// Used to shadow the 'inidisp' register

/*
===============================================================================

                  hardware registers

===============================================================================
*/

#define	inidisp		(*(unsigned char *)0x2100)
#define	objsel		(*(unsigned char *)0x2101)

#define	oamadd		(*(unsigned *)0x2102)
#define	oamdataw		(*(unsigned char *)0x2104)

#define	bgmode		(*(unsigned char *)0x2105)
#define	bg1sc			(*(unsigned char *)0x2107)
#define	bg2sc			(*(unsigned char *)0x2108)
#define	tm				(*(unsigned char *)0x212C)
#define	ts				(*(unsigned char *)0x212D)
#define	bg12nba		(*(unsigned char *)0x210B)

#define	bg1hofs		(*(unsigned char *)0x210D)
#define	bg1vofs		(*(unsigned char *)0x210E)
#define	bg2hofs		(*(unsigned char *)0x210F)
#define	bg2vofs		(*(unsigned char *)0x2110)

#define	vmainc		(*(unsigned char *)0x2115)
#define	vmadd			(*(unsigned *)0x2116)
#define	vmdataw		(*(unsigned *)0x2118)
#define	vmdatal		(*(unsigned char *)0x2118)
#define	vmdatah		(*(unsigned char *)0x2119)

#define	m7a			(*(unsigned char *)0x211B)
#define	m7b			(*(unsigned char *)0x211C)
#define	m7c			(*(unsigned char *)0x211D)
#define	m7d			(*(unsigned char *)0x211E)
#define	m7x			(*(unsigned char *)0x211F)
#define	m7y			(*(unsigned char *)0x2120)

#define	cgadd			(*(unsigned char *)0x2121)
// Write 'cgdataw' twice
#define	cgdataw		(*(unsigned char *)0x2122)
#define	cgswsel		(*(unsigned char *)0x2130)
#define	cgadsub		(*(unsigned char *)0x2131)
#define	coldata		(*(unsigned char *)0x2132)
#define	setini		(*(unsigned char *)0x2133)

#define	mdmaen		(*(unsigned char *)0x420B)
#define	hdmaen		(*(unsigned char *)0x420C)

#define	nmitimen		(*(unsigned char *)0x4200)
#define	vtime			(*(unsigned *)0x4209)

#define	cpuspeed		(*(unsigned char *)0x420D)

#define	SPC700Port0	(*(unsigned char *)0x2140)
#define	SPC700Port0W	(*(unsigned *)0x2140)	//jgt
#define	SPC700Port1	(*(unsigned char *)0x2141)
#define	SPC700Port1W	(*(unsigned *)0x2140)	//jgt
#define	SPC700Port2	(*(unsigned char *)0x2142)
#define	SPC700Port3	(*(unsigned char *)0x2143)

#define	dma0parms	(*(unsigned char *)0x4300)
#define	dma0port		(*(unsigned char *)0x4301)
#define	dma0addr		(*(unsigned *)0x4302)
#define	dma0bank		(*(unsigned char *)0x4304)
#define	dma0len		(*(unsigned *)0x4305)

#define	dma1parms	(*(unsigned char *)0x4310)
#define	dma1port		(*(unsigned char *)0x4311)
#define	dma1addr		(*(unsigned *)0x4312)
#define	dma1bank		(*(unsigned char *)0x4314)
#define	dma1len		(*(unsigned *)0x4315)

//-------------------------------------------------------------------
// From IOSNES.ASM (formerly IOSNES.C)

void DmaCGram (void *source, unsigned dest, unsigned length);
void DmaVram (void *source, unsigned dest, unsigned length);
void DmaOAram (void *source, unsigned dest, unsigned length);
void ClearVram (void);

void DmaMode7Tiles (void *source, unsigned dest, unsigned length);
void DmaMode7Screens (void *source, unsigned dest, unsigned length);

extern void SetupHDMA (void);

//-------------------------------------------------------------------

void ack (void);

void TilePrint (unsigned basechar, unsigned x, unsigned y, char *string);
void DrawTilePic (unsigned basechar, unsigned x, unsigned y, unsigned w, unsigned h);

void SetupTileScreen (char *tileset);
void ShowTileScreen (void);

void SetupTileSpriteScreen (char *tileset, char *backset, char *spriteset);
void ShowTileSpriteScreen (void);

void FadeIn (void);
void FadeOut (void);

