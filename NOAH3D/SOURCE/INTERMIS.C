//-------------------------------------------------------------------
// INTERMIS.C
//-------------------------------------------------------------------

#include "sneswolf.h"
#include <string.h>

unsigned	scrollx ,scrolly, ticker, WhichBJ;
boolean	sendtilescreen;

boolean	ackedout;

#define	BJBASE	(0xC0)
#define	REDBASE	(0xC0+(2<<10))
#define	YELBASE	(0xC0+(4<<10))

void checkabort (void)
{
	if (CheckAck())
		ackedout = true;
}

#define	timex		15
#define	timey		14
#define	ratiox	20
#define	ratioy	13
#define	scorex	15
#define	scorey	24
#define	bonusx	5
#define	bonusy	21


void ClearStuff (void)
{
	memset (&tilescreen[ratioy][0],0,64*(scorey-ratioy));
}

void ClearBottom (void)
{
	memset (&tilescreen[bonusy][0],0,64*2);
}


//-------------------------------------------------------------------
// ShowScreen		Waits for a VSYNC then uploads tile and OAM data
//-------------------------------------------------------------------

void ShowScreen (void)
{
    sendtilescreen = true;
    while (sendtilescreen)
	    ;			// Wait for intermission IRQ to send screen
}



//-------------------------------------------------------------------
//  ScrollIRQ		Called every tic.  Do with it what you will.
//-------------------------------------------------------------------

void ScrollIRQ (void)
{
	BlankVideo ();

	scrolly++;		// Scroll the background
	bg2vofs = scrolly >> 1;
	bg2vofs = (scrolly&1023) >> 9;

	if (sendtilescreen) {
		sendtilescreen = false;
		DmaVram (tilescreen, 0x7C00, 0x800);	// Upload screen
	}
	else
		DmaVram (tilescreen, 0x7400, 0x800);	// Make timing equal

	DmaOAram (&oam, 0, sizeof(oam));				// Upload sprite records
	UnBlankVideo ();

// Does BJ need to breathe?

	if (WhichBJ != 2 && ++ticker > 40) {	// Yes
		DrawTilePic (BJBASE+BJ1_START+(WhichBJ*121), 1,1, BJ1_W,BJ1_H);
		WhichBJ ^= 1;
		ticker = 0;
		sendtilescreen = true;
	}

}

//-------------------------------------------------------------------
// DrawTime			Draws a time value at the given coords
//-------------------------------------------------------------------

void DrawTime (int x, int y, unsigned time)
{
	int	minutes, seconds;
	int	minTens, minOnes, secTens, secOnes;

	minutes = time/60;
	seconds = time%60;

	minTens = (minutes/10)%10;
	minOnes = minutes%10;
	secTens = (seconds/10)%10;
	secOnes = seconds%10;

	DrawTilePic (REDBASE+NUM0_START+minTens*4,x,y,NUM0_W,NUM0_H);
	DrawTilePic (REDBASE+NUM0_START+minOnes*4,x+2,y,NUM0_W,NUM0_H);
	DrawTilePic (REDBASE+COLON_START,x+4,y,COLON_W,COLON_H);
	DrawTilePic (REDBASE+NUM0_START+secTens*4,x+5,y,NUM0_W,NUM0_H);
	DrawTilePic (REDBASE+NUM0_START+secOnes*4,x+7,y,NUM0_W,NUM0_H);
}


//-------------------------------------------------------------------
// DrawScore		Draws a score value at the given coords
//-------------------------------------------------------------------

void DrawScore (int x, int y)
{
	int score1,score2,score3,score4,score5,score6;

	score1 = (gamestate.score/100000)%10;
	score2 = (gamestate.score/10000)%10;
	score3 = (gamestate.score/1000)%10;
	score4 = (gamestate.score/100)%10;
	score5 = (gamestate.score/10)%10;
	score6 = gamestate.score%10;

	DrawTilePic (REDBASE+NUM0_START+score1*4,x,y,NUM0_W,NUM0_H);
	DrawTilePic (REDBASE+NUM0_START+score2*4,x+2,y,NUM0_W,NUM0_H);
	DrawTilePic (REDBASE+NUM0_START+score3*4,x+4,y,NUM0_W,NUM0_H);
	DrawTilePic (REDBASE+NUM0_START+score4*4,x+6,y,NUM0_W,NUM0_H);
	DrawTilePic (REDBASE+NUM0_START+score5*4,x+8,y,NUM0_W,NUM0_H);
	DrawTilePic (REDBASE+NUM0_START+score6*4,x+10,y,NUM0_W,NUM0_H);
}


//-------------------------------------------------------------------
// DrawRatio	Draws a ratio value at the given coords
//-------------------------------------------------------------------

void DrawRatio (int x, int y, long theRatio)
{
	int rat1,rat2,rat3;

	rat1 = (theRatio/100)%10;
	rat2 = (theRatio/10)%10;
	rat3 = theRatio%10;

	if (rat1) DrawTilePic (REDBASE+NUM0_START+rat1*4,x,y,NUM0_W,NUM0_H);
	if (rat2 || rat1) DrawTilePic (REDBASE+NUM0_START+rat2*4,x+2,y,NUM0_W,NUM0_H);
	DrawTilePic (REDBASE+NUM0_START+rat3*4,x+4,y,NUM0_W,NUM0_H);
	DrawTilePic (REDBASE+PERCENT_START,x+6,y,PERCENT_W,PERCENT_H);
}


//-------------------------------------------------------------------
// CalcSeconds		Return how many seconds they beat the partime by.
//						No partimes will be over ten minutes, so that digit
//						is ignored.
//-------------------------------------------------------------------

unsigned CalcSeconds(unsigned partime,unsigned time)
{
	unsigned psecs,tsecs;

	psecs = ((partime /100)%10)*60 + ((partime /10)%10)*10 + partime %10;
	tsecs = ((time /100)%10)*60 + ((time /10)%10)*10 + time %10;
	return (psecs-tsecs);
}


//-------------------------------------------------------------------
// RollScore		Do a Bill-Budgey roll of the old score to the new
//						score, not bothering with the lower digit, as you
//						never get less than ten for anything.
//-------------------------------------------------------------------

void RollScore(unsigned bonusamt)
{
	int i;

	for (i=0; i<=bonusamt; i=i+10) {
		PlayLocalSound (SRC_BONUS, D_INC, FREQ_NORM, VOL_LOW);
		GivePoints(10);
		checkabort ();
		if (ackedout) {
			GivePoints( 10*(bonusamt-i) );
			i=bonusamt;
		}
		DrawScore(scorex,scorey);
		ShowScreen();
	}

	DrawScore(scorex,scorey);
	ShowScreen ();
}


//-------------------------------------------------------------------
// RollRatio		Do a Bill-Budgey roll of the ratio
//-------------------------------------------------------------------

void RollRatio(int x,int y,int ratio)
{
	int i;

	for (i=0; i<=ratio && !ackedout; i++) {
		PlayLocalSound (SRC_BONUS, D_INC, FREQ_NORM, VOL_LOW);
		DrawRatio(x,y,(long)i);
		DrawScore(scorex,scorey);
		ShowScreen();
	}

	DrawRatio(x,y,(long)ratio);
	DrawScore(scorex,scorey);
	ShowScreen();

	if (ratio==100) {
		PlayLocalSound (SRC_BONUS, D_EXTRA, FREQ_NORM, VOL_LOW);
		GivePoints (10000);
		DrawScore(scorex,scorey);
		ShowScreen();
		ticcount = 0;
		while (ticcount < 60)
			;
	}
}


//-------------------------------------------------------------------
// Perfect			Bouncing stars
//-------------------------------------------------------------------

void Perfect (void)
{
	int  i,j;
	int  xinc[8], yinc[8];

	for (i=0; i<bonusx+4; i++) {
		DrawTilePic (YELBASE+I_PERFECT_START,i,bonusy,I_PERFECT_W,I_PERFECT_H);
		ShowScreen();
	}
	ClearBottom ();
	DrawTilePic (YELBASE+I_PERFECT_START,bonusx+4,bonusy,I_PERFECT_W,I_PERFECT_H);
	ShowScreen ();

	for (i=0; i<8; i++) {
	    oam.s[i].x = (i*64 + 20)%256 +8;
	    oam.s[i].y = (i*32 + 40)%256 +8;
	    oam.s[i].name = 128+15+(i%4)*16;
	    xinc[i]=i%4 + 2;
	    yinc[i]=i%4 + 2;
	}

	ackedout = false;
	StartAck ();

	while (!ackedout) {		// Wait for a button press
		checkabort ();
		for (j=0; j<8; j++) {
			oam.s[j].x += xinc[j];
			oam.s[j].y += yinc[j];
			if ((oam.s[j].x > 240) || (oam.s[j].x < 4))
				xinc[j] = -xinc[j];
			if ((oam.s[j].y > 240) || (oam.s[j].y < 4))
				yinc[j] = -yinc[j];
			oam.s[j].name+=16;
			if (oam.s[j].name > 128+4*16)
				oam.s[j].name -= 4*16;
		}
		ShowScreen();
	}

	for (i=0;i<8;i++) {
		oam.s[i].y = 248;
		ShowScreen();
	}
}


//-------------------------------------------------------------------
// LevelCompleted		Let's show 'em how they did!
//-------------------------------------------------------------------

void LevelCompleted (unsigned level, unsigned kills, unsigned maxkills,
	unsigned treasure, unsigned maxtreasure,
	unsigned secret, unsigned maxsecret,
	unsigned time, unsigned partime)
{
	int  i, j, k;
	unsigned bonusamt;

//-------------------------------
// Setup
//-------------------------------

	memset (tilescreen, 0, sizeof(tilescreen));	// Clear to spaces

//-------------------------------
// Show BJ pic
//-------------------------------

	DrawTilePic (BJBASE+BJ1_START, 1,1, BJ1_W,BJ1_H);
	WhichBJ = 1;
	ticker = 0;

//-------------------------------
// Show what floor of which
// mission was just finished
//-------------------------------

	DrawTilePic (REDBASE+I_FLOOR_START,12,3,I_FLOOR_W,I_FLOOR_H);
	DrawTilePic (REDBASE+I_COMPLETE_START,12,7,I_COMPLETE_W,I_COMPLETE_H);

	DrawTilePic (REDBASE+NUM0_START+missionnum[level]*4,26,3,NUM0_W,NUM0_H);
	DrawTilePic (REDBASE+NUM0_START+levelnum[level]*4,26,5,NUM0_W,NUM0_H);

	DrawTilePic (REDBASE+I_SCORE_START,scorex-12,scorey,I_SCORE_W,I_SCORE_H);
	DrawScore(scorex,scorey);

	if (gamestate.mapon != 28) {
		CalcPassword ();
		TilePrint(0, 12, 10,"Floor Code:");
		TilePrint(0, 23, 10,password);
	}

	SetupIntermission ();	// Will fade in on first screen
	ackedout = false;
	StartAck ();
	StartSong (4,true);

//-------------------------------
// Display Par Time, Player's
// Time, and show bonus if any
//-------------------------------

	for (i=0; i<(timex-9);i++) {
		ClearStuff ();
		DrawTilePic (REDBASE+I_PAR_START,i+1,timey,I_PAR_W,I_PAR_H);
		DrawTilePic (REDBASE+I_TIME_START,i,timey+2,I_TIME_W,I_TIME_H);
		ShowScreen ();			// Will fade in on first screen
	}

	ClearStuff ();
	DrawTilePic (REDBASE+I_PAR_START,timex-8,timey,I_PAR_W,I_PAR_H);
	DrawTilePic (REDBASE+I_TIME_START,timex-9,timey+2,I_TIME_W,I_TIME_H);

	DrawTime (timex,timey,partime);
	DrawTime (timex,timey+2,time);
	ShowScreen();

	if (time < partime) {
		for (i=0; i<=bonusx && !ackedout ;i++) {
			checkabort ();
			DrawTilePic (YELBASE+I_SPEED_START,i,bonusy,I_SPEED_W,I_SPEED_H);
			ShowScreen ();
		}
		ClearBottom ();
		DrawTilePic (YELBASE+I_SPEED_START,bonusx,bonusy,I_SPEED_W,I_SPEED_H);
		DrawTilePic (YELBASE+I_BONUS_START,bonusx+10,bonusy,I_BONUS_W,I_BONUS_H);
		ShowScreen ();

		bonusamt = CalcSeconds(partime,time) * 50;
		RollScore(bonusamt);
	}

//-------------------------------
// Wait a small bit, then erase
// with blank and sprites
//-------------------------------

	for (i=0 ; i<200 && !ackedout ;i++) {
		checkabort ();
		ShowScreen();
	}

	k=0;
	for (i=0;i<255 && !ackedout;i+=8) {
		k++;
		for (j=timey;j<(timey+9);j++) {
			k++;
			TilePrint (0, i/8,j, " ");
			oam.s[j-timey].x = i;
			oam.s[j-timey].y = j*8;
			oam.s[j-timey].name = 0x8f + ((k&3)<<4);
		}
		ShowScreen();
	}

	ClearStuff ();

	for (i=timey;i<timey+9;i++)
		oam.s[i-timey].y = 248;
	ShowScreen();

//-------------------------------
// Show ratios for "terminations",
// treasure, and secret stuff...
// If 100% on all counts, give
// Perfect Bonus!
//-------------------------------

	DrawTilePic (REDBASE+I_ENEMY_START,ratiox-12,ratioy,I_ENEMY_W,I_ENEMY_H);
	DrawTilePic (REDBASE+I_TREASURE_START,ratiox-17,ratioy+2,I_TREASURE_W,I_TREASURE_H);
	DrawTilePic (REDBASE+I_SECRET_START,ratiox-13,ratioy+4,I_SECRET_W,I_SECRET_H);

	k=0;
	RollRatio(ratiox,ratioy,(kills*100)/maxkills);
	if (kills == maxkills)
		k++;

	if (!maxtreasure)
	{
		RollRatio (ratiox,ratioy+2,100);
		k++;
	}
	else
	{
		RollRatio(ratiox,ratioy+2,(treasure*100)/maxtreasure);
		if (treasure == maxtreasure)
			k++;
	}

	if (!maxsecret)
	{
		RollRatio (ratiox,ratioy+4,100);
		k++;
	}
	else
	{
		RollRatio(ratiox,ratioy+4,(secret*100)/maxsecret);
		if (secret == maxsecret)
			k++;
	}

	if (k==3)
		Perfect ();
	else
		ack ();

	FadeOut ();
}


//-------------------------------------------------------------------
// Intermission
//-------------------------------------------------------------------

ushort partimes[30] = {
	 30, 60, 60,
	120,105, 90,105,
	105,120,135, 60,120,
	210,150,240,135,210,
	150,165,180,210,180,180,
	105,180,180, 90,150,191,120
};

void Intermission (void)
{
	FadeOut ();

	LevelCompleted (gamestate.mapon,
		gamestate.killcount, gamestate.killtotal,
		gamestate.treasurecount, gamestate.treasuretotal,
		gamestate.secretcount, gamestate.secrettotal,
		(unsigned) playtime/60, partimes[gamestate.mapon]);

	gamestate.globaltime += playtime;
	gamestate.globaltimetotal += partimes[gamestate.mapon];
	gamestate.globalsecret += gamestate.secretcount;
	gamestate.globaltreasure += gamestate.treasurecount;
	gamestate.globalkill += gamestate.killcount;
	gamestate.globalsecrettotal += gamestate.secrettotal;
	gamestate.globaltreasuretotal += gamestate.treasuretotal;
	gamestate.globalkilltotal += gamestate.killtotal;

	StopSong ();

	SetupPlayScreen ();
}


//-------------------------------------------------------------------
// VictoryIntermission		Okay, let's face it: they won the game
//-------------------------------------------------------------------

#define  PAL  (1<<10)		// Used by Victory and Credits

void VictoryIntermission (void)
{
	int		k;
	int		partime;

	partime = 10;	// Fixme!

//-------------------------------
// Setup
//-------------------------------

	memset (tilescreen, 0, sizeof(tilescreen));	// Clear to spaces

//-------------------------------
// Show BJ Thumbs Up picture
//-------------------------------

	DrawTilePic (BJBASE+BJ3_START, 1,1, BJ1_W,BJ1_H);
	WhichBJ = 2;
	ticker = 0;

	DrawTilePic (YELBASE+E_OVERALL_START,12,5,E_OVERALL_W,E_OVERALL_H);

	DrawTilePic (REDBASE+I_SCORE_START,scorex-12,scorey,I_SCORE_W,I_SCORE_H);
	DrawScore(scorex,scorey);

	SetupIntermission ();	// Will fade in on first screen

	StartAck ();
	StartSong (6,true);

//-------------------------------
// Show ratios for "terminations",
// treasure, and secret stuff...
// If 100% on all counts, give
// Perfect Bonus!
//-------------------------------

	DrawTilePic (REDBASE+I_ENEMY_START,ratiox-12,ratioy,I_ENEMY_W,I_ENEMY_H);
	DrawTilePic (REDBASE+I_TREASURE_START,ratiox-17,ratioy+2,I_TREASURE_W,I_TREASURE_H);
	DrawTilePic (REDBASE+I_SECRET_START,ratiox-13,ratioy+4,I_SECRET_W,I_SECRET_H);

	ShowScreen ();

#ifndef	ID_VERSION
	if (!gamestate.globalkilltotal)
		gamestate.globalkilltotal++;
	if (!gamestate.globaltreasuretotal)
		gamestate.globaltreasuretotal++;
	if (!gamestate.globalsecrettotal)
		gamestate.globalsecrettotal++;
#endif

	k=0;
	RollRatio(ratiox,ratioy,(gamestate.globalkill*100)/gamestate.globalkilltotal);
	if (gamestate.globalkill == gamestate.globalkilltotal)
		k++;
	RollRatio(ratiox,ratioy+2,(gamestate.globaltreasure*100)/gamestate.globaltreasuretotal);
	if (gamestate.globaltreasure == gamestate.globaltreasuretotal)
		k++;
	RollRatio(ratiox,ratioy+4,(gamestate.globalsecret*100)/gamestate.globalsecrettotal);
	if (gamestate.globalsecret == gamestate.globalsecrettotal)
		k++;

	if (k==3)
		Perfect ();
	else
		ack ();

//-------------------------------
// Our work is done here
//-------------------------------

	StopSong ();
	FadeOut ();
	SetupPlayScreen ();
}



//-------------------------------------------------------------------
// CharacterCast		Show player the game characters
//-------------------------------------------------------------------

void BottomPrint (char *text)
{
	unsigned  count;

//-------------------------------
// Write the text at the start
//-------------------------------

	count = 0;
#ifdef	ID_VERSION
	while (count < 12 && text[count]) {
#else
	while (count < 14 && text[count]) {
#endif
		oam.s[PS_LASTSPRITE+count].name = text[count] - 32;
		count++;
	}

//-------------------------------
// Fill the remainder with blanks
//-------------------------------

	while (count < 14) {
		oam.s[PS_LASTSPRITE+count].name = 0;
		count++;
	}
}


#define NUMCAST    12

ushort caststate[NUMCAST] = {
	ST_DOG_WLK1, ST_GRD_WLK1, ST_OFC_WLK1, ST_SS_WLK1, 
	ST_MUTANT_WLK1, ST_HANS_WLK1, ST_TRANS_WLK1, ST_SCHABBS_WLK1,
	ST_UBER_WLK1, ST_DKNIGHT_WLK1, ST_MHITLER_WLK1, ST_HITLER_WLK1
};

char *casttext[NUMCAST] = {	// 14 chars. max
	"GOAT",
	"SHEEP",
	"OSTRICH",
	"ANTELOPE",
	"OX",
	"CARL CAMEL",
	"GINNY GIRAFFE",
	"MELVIN MONKEY",
	"KERRY KANGAROO",
	"ERNIE ELEPHANT",
	"HIDING BURT",
	"BURT THE BEAR"
};

class_t castclass[NUMCAST] = {	// Class types for playing sounds
	CL_DOG, CL_GUARD, CL_OFFICER, CL_SS,
	CL_MUTANT, CL_HANS, CL_TRANS, CL_SCHABBS,
	CL_UBER, CL_DKNIGHT, CL_MECHAHITLER, CL_HITLER
};

void CharacterCast (void)
{
	int		i;
	ushort	en, state, count, cycle;
	boolean	up;

//-------------------------------
// reload level and set things up
//-------------------------------

	gamestate.mapon = 30;
	PrepPlayLoop ();
	viewx = actors[0].x;
	viewy = actors[0].y;

//-------------------------------
// Set up a 2nd line of text sprs
//-------------------------------

	for (i=0 ; i < 14 ; i++) {
		oam.s[PS_LASTSPRITE+i].y = 160;
		oam.s[PS_LASTSPRITE+i].x = 24+i*8;
		oam.s[PS_LASTSPRITE+i].name = 0;
		oam.s[PS_LASTSPRITE+i].flags = 0x3E;	// Priority 2, palette 7
	}

//-------------------------------
// Remove the weapon sprites
//-------------------------------

	for (i= PS_GUN; i< PS_GUN+16; i++)
		oam.s[i].y = 240;

	sendweapon = true;

	IO_Print ("       MEET THE CAST!");
	topspritescale = 32*FRACUNIT;

//-------------------------------
// Go through the cast
//-------------------------------

	en = 0;
	cycle = 0;
	do
	{
		state = caststate[en];
		StartAck ();
		count = 1;
#ifndef JAPVERSION
		BottomPrint (casttext[en]);
#endif
		up = false;
		do {
#ifndef	ID_VERSION
			if (cycle == 0)
				PlayLocalSound (SRC_BONUS, classinfo[castclass[en]].sightsound, FREQ_NORM, VOL_NORM);
#endif

			if (++cycle > 15*4) {
				cycle = 0;
				if (++en >= NUMCAST)
				     en = 0;
				break;
			}

			if (!--count) {
				state = states[state].next;
				count = states[state].tictime;
			}

			topspritenum = states[state].shapenum;

#ifndef	ID_VERSION
	topspritescale = (cycle/2+8)*FRACUNIT;		// Walk towards
#endif

			RenderView ();
			if (!joystick1 && !up) {
				up = true;
				continue;
			}

			if (!up)
				continue;
			if (!joystick1)
				continue;

			if (joystick1 & JOYPAD_START) {
				en = NUMCAST;
				break;
			}

			if ( (joystick1 & (JOYPAD_TL|JOYPAD_LFT)) && en >0) {
				en--;
#ifndef	ID_VERSION
				cycle = 0;
#endif
				break;
			}
			if ( (joystick1 & (JOYPAD_TR|JOYPAD_RGT)) && en <NUMCAST-1) {
				en++;
#ifndef	ID_VERSION
				cycle = 0;
#endif
				break;
			}

		} while (1);
	} while (en < NUMCAST);

	StopSong ();
	FadeOut ();
}


//-------------------------------------------------------------------
// Credits
//-------------------------------------------------------------------

void Credits (boolean final)
{
	int  i;

	StartSong (10, true);

	memset (tilescreen, 0, sizeof(tilescreen));	// Clear to spaces
	memset (&oam, 0, sizeof(oam));
	for (i=0 ; i<128 ; i++)
		oam.s[i].y = 248;		// Off-screen

	WhichBJ = 2;				// Don't animate a BJ (?!?!)
								//................................
	TilePrint(PAL,  1,  2,"     Super 3-D Noah's Ark     ");
	TilePrint(PAL,  1,  3,"..............................");

	TilePrint(PAL,  1,  5,"..............................");
	TilePrint(PAL,  1,  6,"..............................");
	TilePrint(PAL,  1,  7,"..............................");
	TilePrint(PAL,  1,  8,"..............................");
	TilePrint(PAL,  1,  9,"..............................");
	TilePrint(PAL,  1, 10,"..............................");
	TilePrint(PAL,  1, 11,"..............................");
	TilePrint(PAL,  1, 12,"..............................");

	TilePrint(PAL,  1, 18,"Copyright 1994 Wisdom Tree Inc");

	TilePrint(PAL,  1, 21,"This game contains copyrighted");
	TilePrint(PAL,  1, 22,"    software code owned by    ");
	TilePrint(PAL,  1, 23,"       Id Software, Inc.      ");

	TilePrint(PAL,  1, 25,"Copyright 1992 Id Software Inc");
	TilePrint(PAL,  1, 26,"     All rights reserved.     ");

	SetupIntermission ();	// Will fade in on first screen
	ShowScreen();

	if (final)
		ack ();
	else {
		ticcount = 0;
		StartAck ();
		while (!CheckAck() && ticcount < 500)
			;
	}

	StopSong ();
	FadeOut ();
}



//-------------------------------------------------------------------
// Disclaimer
//-------------------------------------------------------------------


void Disclaimer (void)
{
	int  i;

	if (!sequence)
		StartSong (0, true);
	
	memset (tilescreen, 0, sizeof(tilescreen));	// Clear to spaces

								  //................................
	TilePrint(0x2000, 4, 10,"This product is designed");
	TilePrint(0x2000, 4, 11,"    and produced by");
	TilePrint(0x2000, 4, 12,"   Wisdom Tree, Inc.");

	TilePrint(0x2000, 4, 14,"  It is not designed,");
	TilePrint(0x2000, 4, 15,"manufactured, sponsored,");
	TilePrint(0x2000, 4, 16,"or endorsed by Nintendo.");

	TilePrint(0x2000, 4, 18,"Nintendo is a registered");
	TilePrint(0x2000, 4, 19," trademark of Nintendo");
	TilePrint(0x2000, 4, 20,"    of America, Inc.");

	if (joystick2 & JOYPAD_TR && joystick2 & JOYPAD_TL)
	{
		char		checkstr[20];
		unsigned	CheckSum;

		CheckSum = CHECKROM ();

		strcpy (checkstr, "CHECKSUM =");
		itox (CheckSum, &checkstr[strlen(checkstr)-1]);
		TilePrint (0x2000, (32-strlen(checkstr))/2, 7, checkstr);
	}

	SetupTextPicScreen (&credit, &dimpal, 25);
	bg2vofs = 255;
	bg2vofs = 0;		// 0 high byte
	FadeIn ();

	ticcount = 0;
	StartAck ();
	while (!CheckAck() && ((ticcount < 400) 
#ifndef	ID_VERSION
		|| (joystick2 & JOYPAD_TR && joystick2 & JOYPAD_TL)));
#else
		));
#endif

	StopSong ();
	FadeOut ();
}
