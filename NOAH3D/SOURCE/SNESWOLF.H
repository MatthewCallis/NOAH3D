/* sneswolf.h */

#include "wolfdef.h"
#include "version.h"
#include "extdata.h"
#include "noahgfx.h"
#include "iosnes.h"

extern char screenbuffer;    /* ram buffer for rendering */

extern   unsigned ackstage, checkbits, pushbits;

extern   unsigned redadd, greenadd, blueadd, setadd;

extern   unsigned  sendplayscreen;
extern   unsigned  sendoam;
extern   unsigned  sendweapon;
extern   boolean   sendtilescreen;

extern   unsigned  currentweapon;
extern   unsigned  firstframe;

extern   boolean  timing;
extern   boolean  stereosound, lefthandmouse;
extern   unsigned mouseb1;
extern   unsigned mouseb2;

extern   char				mouse1, mouse2, mouse3, mouse4;
extern   short	volatile	mousex, mousey;
extern   ushort			mouseclicks;
extern   boolean			mousepresent;


/*
** object array numbers
*/

#define  PS_FLOOR1      0
#define  PS_FLOOR2      2
#define  PS_SCORE1      4
#define  PS_SCORE2      6
#define  PS_SCORE3      8
#define  PS_SCORE4      10
#define  PS_SCORE5      12
#define  PS_SCORE6      14
#define  PS_TREASURE1   16
#define  PS_TREASURE2   18
#define  PS_FACE        20        /* 6 sprites, 2 large 4 small */
#define  PS_HEALTH1     26
#define  PS_HEALTH2     28
#define  PS_HEALTH3     30
#define  PS_AMMO1       32
#define  PS_AMMO2       34
#define  PS_AMMO3       36
#define  PS_KEY1        38
#define  PS_KEY2        40
#define  PS_GUN         42        /* 16 large sprites */
#define  PS_TEXT        58        /* 28 small sprites */
#define  PS_LIVES       86
#define  PS_STATUS      87
#define  PS_LASTSPRITE  114

/*
** sprite numbers
*/

#define  NS_0           96
#define  NS_KEY         0x6a

/*
** sound driver commands
*/
#define  DRV_NOTEON     0
#define  DRV_NOTEOFF    1
#define  DRV_SETADDR    2
#define  DRV_DOWNLOAD   3
#define  DRV_ASSIGN     4
#define  DRV_PITCHBEND  5
#define  DRV_MUSICON    6

extern unsigned missionnum[30];
extern unsigned levelnum[30];

extern   char password[8];
void CalcPassword (void);


void SetNumber (unsigned long number, unsigned spritenum, unsigned digits);

void BlitPlay (void);
void BlankVideo (void);
void UnBlankVideo (void);
void RunAutoMap (void);
void SetupIntermission (void);
void SetupPlayScreen (void);
void Intermission (void);

void Decompress (byte *src_p, byte *dest_p);
void itox (unsigned num, char *str);

void SoundTest (void);
void MusicTest (void);
void WaitUp (void);
int GridSelect (unsigned minx, unsigned miny, unsigned width, unsigned height
, unsigned startpos);

void PauseSong(void);
void UnPauseSong(void);

void StartAck (void);
boolean CheckAck (void);

void SetupIntroScreen (void);
void Intro (void);
void NintendoSplash (void);
void SetupTextPicScreen (byte *tileset, byte *palette, int height);

extern   int spron;

extern   byte		*sequence;

extern int nextmap;

void SprPrint (int x, int y, char *msg);
void SystemCheck (void);
void SellScreen (void);
