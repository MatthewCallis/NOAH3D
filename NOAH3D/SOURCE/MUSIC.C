//-------------------------------------------------------------------
// MUSIC.C
//-------------------------------------------------------------------

#include <string.h>

#include "sneswolf.h"
#include "refresh.h"

#ifndef	NULL
#define	NULL	(byte *)0
#endif	/*jgt*/

#define	MAXVOICES	8
#define	NOTE_NORM		0x56

byte *music[] = {
	&song_01, &song_02, &song_03, &song_04, &song_05,
	&song_06, &song_07, &song_08,	&song_09, &song_10,
	&song_11
};

extern   ushort    notetable1, notetable2, notetable3, notetable4, notetable5;
ushort   *notetable1_p = &notetable1;
ushort   *notetable2_p = &notetable2;
ushort   *notetable3_p = &notetable3;
ushort   *notetable4_p = &notetable4;
ushort   *notetable5_p = &notetable5;

extern   byte      veltable1,veltable2,veltable3,veltable4;
extern   byte      veltable5,veltable6,veltable7,veltable8;
byte     *veltable1_p = &veltable1;
byte     *veltable2_p = &veltable2;
byte     *veltable3_p = &veltable3;
byte     *veltable4_p = &veltable4;
byte     *veltable5_p = &veltable5;
byte     *veltable6_p = &veltable6;
byte     *veltable7_p = &veltable7;
byte     *veltable8_p = &veltable8;

boolean	loopsong;
boolean	songover;
boolean  pausesong;
byte		*sequencestart;
byte		*sequence;
ushort	seqticks,seqnextevt;

unsigned		curinst[MAXVOICES];
unsigned		curvel[MAXVOICES];

unsigned      currate[MAXVOICES];

void SndWait (void);
void SndCmd (unsigned cmd);

#if 0
unsigned noterate[128] = {
0x004c, 0x0051, 0x0056, 0x005b, 0x0060, 0x0066, 0x006c, 0x0072, 
0x0079, 0x0081, 0x0088, 0x0090, 0x0099, 0x00a2, 0x00ac, 0x00b6, 
0x00c1, 0x00cc, 0x00d9, 0x00e5, 0x00f3, 0x0102, 0x0111, 0x0121, 
0x0132, 0x0145, 0x0158, 0x016c, 0x0182, 0x0199, 0x01b2, 0x01cb, 
0x01e7, 0x0204, 0x0222, 0x0243, 0x0265, 0x028a, 0x02b1, 0x02d9, 
0x0305, 0x0333, 0x0364, 0x0397, 0x03ce, 0x0408, 0x0445, 0x0486, 
0x04cb, 0x0514, 0x0562, 0x05b3, 0x060a, 0x0666, 0x06c8, 0x072f, 
0x079c, 0x0810, 0x088b, 0x090d, 0x0997, 0x0a29, 0x0ac4, 0x0b67, 
0x0c15, 0x0ccd, 0x0d90, 0x0e5e, 0x0f39, 0x1021, 0x1116, 0x121b, 
0x132e, 0x1452, 0x1588, 0x16cf, 0x182b, 0x199a, 0x1b20, 0x1cbd, 
0x1e73, 0x2042, 0x222d, 0x2436, 0x265d, 0x28a5, 0x2b10, 0x2d9f, 
0x3056, 0x3335, 0x3641, 0x397b, 0x3ce6, 0x4085, 0x445b, 0x486c, 
0x4cba, 0x514a, 0x5620, 0x5b3f, 0x60ac, 0x666b, 0x6c82, 0x72f6, 
0x79cc, 0x810a, 0x88b7, 0x90d8, 0x9975, 0xa295, 0xac40, 0xb67e, 
0xc158, 0xccd7, 0xd905, 0xe5ed, 0xf399, 0xffff, 0xffff, 0xffff, 
0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
};
#endif

unsigned pitchscale[32] = {0x2a0,0xf60,0x1000,0xfc,0x1a4,0x68,0xcd,0x35e};

unsigned velocityscale[32] = {0x1c,0x18,0x34,0x0c,0x0a,0x10,0x20,0x20};


//-------------------------------------------------------------------
// IO_NoteOn		MIDI numbers
//-------------------------------------------------------------------

void IO_NoteOn (ushort channel, ushort instrument, ushort note, ushort velocity);

ushort sfxtoinst[] = {128+35,128+38,128+42,17,68,75,57,36};
#if 0

void IO_NoteOn (ushort channel, ushort instrument, ushort note, ushort velocity)
{
	ushort	sfx, rate;

	asm {
		php
		sei
	};

	switch (instrument)
	{
		case 128+35:	// Bass drum
		case 128+36:
			sfx = 0;
			rate = 0xE21;
			velocity = veltable1_p[velocity];
			break;
		case 128+38:	// Snare drum
		case 128+40:
			sfx = 1;
			rate = 0x52C2;
			velocity = veltable2_p[velocity];
			break;
		case 128+42:	// High hat
		case 128+44:
		case 128+46:
			sfx = 2;
			rate = 0x5620;
			velocity = veltable3_p[velocity];
			break;

		case 17:			// Organ
			sfx = 3;
			rate = notetable1_p[note];
			velocity = veltable4_p[velocity];
			break;
		case 68:			// Oboe
			sfx = 4;
			rate = notetable2_p[note];
			velocity = veltable5_p[velocity];
			break;
		case 75:			// Pan flute
			sfx = 5;
			rate = notetable3_p[note];
			velocity = veltable6_p[velocity];
			break;
		case 57:			// Trombone
			sfx = 6;
			rate = notetable4_p[note];
			velocity = veltable7_p[velocity];
			break;
		case 36:			// Slap bass
			sfx = 7;
			rate = notetable5_p[note];
			velocity = veltable8_p[velocity];
			break;
		default:
			return;
	}

// Start a sound effect

	SndWait ();
	SPC700Port0 = channel;
	SPC700Port1 = sfx;
	SndCmd (DRV_ASSIGN);
	SndWait ();

	currate[channel] = rate;

	SPC700Port0W = rate;

	velocity = 0x0C;	//jgt????

	SPC700Port2 = velocity;
	SndCmd ( (channel<<4) | DRV_MUSICON);
	asm {plp};
}

#endif


//-------------------------------------------------------------------
// IO_NoteOff
//-------------------------------------------------------------------

void IO_NoteOff (ushort channel)
{
	asm	php;
	asm	sei;

	SndWait ();
	SPC700Port0 = channel;
	SndCmd ( DRV_NOTEOFF );

	asm	plp;
}


//-------------------------------------------------------------------
// pitchbend
//-------------------------------------------------------------------

void pitchbend (int voice, unsigned bend)
{
	unsigned	temp;

	asm	php;
	asm	sei;

// IO_PrintNums ("bend:",voice,bend,0);

	SndWait ();
	SPC700Port0 = voice;

// Can't use multiply hardware in a ISR!

	SPC700Port1W = ((long)currate[voice]*(0xe0 + (bend>>2)) ) >>8;

	SndCmd ( DRV_PITCHBEND );

	asm	plp;
}


void loadinstrument (int instr)
{
// Do sound RAM uploading if needed
}


//-------------------------------------------------------------------
// docommand
//-------------------------------------------------------------------

int docommand(unsigned voice,unsigned cmd)
{
	unsigned	vel,note;
	unsigned	length;

	length = 0;
	switch (cmd)
	{
		case 1:		// Note on
			if (curinst[voice] & 0x80)
			{
				vel = curvel[voice] = sequence[length++];
				IO_NoteOn (voice,curinst[voice],0,vel);
			}
			else
			{
				note = sequence[length++];
				if (note & 0x80)
				{
					curvel[voice] = sequence[length++];
					note &= 0x7f;
				}
				IO_NoteOn (voice,curinst[voice],note,curvel[voice]);
			}
			break;
		case 2:		// Note off
			IO_NoteOff (voice);
			break;
		case 3:		// Pitch bend
			pitchbend(voice, sequence[length++]);
			break;
		case 4:		// Instrument reassign
			curinst[voice] = sequence[length++];
			break;
		case 5:		// Loop or terminate
			songover = true;
			break;
		case 6:		// Percussive note on
	 		IO_NoteOn (voice,curinst[voice],0,curvel[voice]);
			break;
	}
	return(length);
}


//-------------------------------------------------------------------
// MusicTic
//-------------------------------------------------------------------

void MusicTic (void)
{
	unsigned	value,voice;
	unsigned	length;
	unsigned	v;

	if (pausesong)
		return;

// While there are events to process

	while (sequence && (seqticks >= seqnextevt))
	{
	// Compute time till next event
		value = *sequence++;
		if ((value & 3) == 3)
		{
			v = *sequence++;
			if (!v)
				v = 0x100;
			seqnextevt += v;
		}
		else
			seqnextevt += value & 3;

	// Determine which voice should be affected by this command

		voice = (value >> 2) & 7;
		length = docommand(voice,value >> 5);

	// Adjust pointer to next instruction

		sequence += length;

	// If the song's now over, loop back to the start

		if (songover)
		{
			if (loopsong)
			{
			// Reset counters

				seqticks = 0;
				seqnextevt = 0;

			// Reset pointer to the beginning of the song

				sequence = sequencestart;
				songover = false;
			}
			else
				sequence = NULL;	// Otherwise, kill it
		}
	}

// Bump time counter

	seqticks++;
}


//-------------------------------------------------------------------
// StopSong
//-------------------------------------------------------------------

void StopSong(void)
{
	int	i;

	sequence =NULL;
	
	for (i = 0;i < MAXVOICES;i++)
		IO_NoteOff(i);
}


//-------------------------------------------------------------------
// PauseSong
//-------------------------------------------------------------------

void PauseSong(void)
{
	int	i;

	pausesong = true;
	for (i = 0;i < MAXVOICES;i++)
		IO_NoteOff(i);
}


void UnPauseSong(void)
{
    pausesong = false;
}



//-------------------------------------------------------------------
// StartSong
//-------------------------------------------------------------------

void StartSong (music_t songnum,boolean loops)
{
	byte	*song;
	int	i,numinsts;

	song = music[songnum];
	StopSong ();

	for (i = 0;i < MAXVOICES;i++)
	{
		curvel[i] = 0;
		curinst[i] = 0;
	}

	numinsts = *song++;
	for (i = 0; i < numinsts; i++)
		loadinstrument(*song++);
	songover = false;
	sequence = song;
	sequencestart = song;
	loopsong = loops;
	seqticks = 0;
	seqnextevt = 0;
	pausesong = false;
}


//-------------------------------------------------------------------
// SndWait		Wait for the sound driver to get ready
//-------------------------------------------------------------------

unsigned lastcmd = 0;
unsigned driverflag = 0x80;
unsigned lastshared = 256;

extern   unsigned sharedofs[99];
extern   unsigned sharedlen[99];

void SndWait (void)
{
	while (SPC700Port3 != lastcmd)
		;		// Make sure the driver is ready
}


void SndCmd (unsigned cmd)
{
	cmd |= driverflag;
	driverflag ^= 0x80;
	lastcmd = cmd;
	SPC700Port3 = cmd;
}

void SoundDownload (byte *src, unsigned blocks);


//-------------------------------------------------------------------
// IO_PlaySound		Volumes should range from 0-127
//							(midi velocity range)
//-------------------------------------------------------------------

char *shsounds_p = &shsounds;

void IO_PlaySound (ushort channel, ushort sample, ushort rate, ushort leftvol, ushort rightvol)
{
	byte	 *srcaddr;
	unsigned  count, vol;

	if (sample >= NUMSOUNDS)
		return;

	SndWait ();

// Upload sample data if needed

	if (sample >= SHAREDSTART && sample != lastshared)
	{
		// FIXME: key off any voices playing lastshared

		// Set upload address

		asm	php;
		asm	sei;

		lastshared = sample;
		SPC700Port0 = SHAREDORG;
		SPC700Port1 = SHAREDORG>>8;
		SndCmd (DRV_SETADDR);
		SndWait ();

		asm	plp;

		// Upload new sample three bytes at a time

		count = sharedlen[sample-SHAREDSTART];
		srcaddr = (char *) ((unsigned long) shsounds_p + (unsigned long) sharedofs[sample-SHAREDSTART]);

#if 1
		SoundDownload (srcaddr, count);
		SndWait ();
#else
		while (count--)
		{

			asm	php;
			asm	sei;

			SPC700Port0 = *srcaddr++;
			SPC700Port1 = *srcaddr++;
			SPC700Port2 = *srcaddr++;
			SndCmd(DRV_DOWNLOAD);
			SndWait ();

			asm	plp;

		}
#endif
	}

// Start a sound effect

	asm	php;
	asm	sei;

	SPC700Port0 = channel;
	SPC700Port1 = sample;
	SndCmd (DRV_ASSIGN);
	SndWait ();

	SPC700Port0W = rate;
	if (stereosound)
		SPC700Port2 = ((leftvol<<1)&0xF0) | (rightvol>>3);
	else
	{
		vol = (leftvol+rightvol)&0xF0;
		SPC700Port2 = vol + (vol>>4);
	}
	SndCmd ( (channel<<4) | DRV_NOTEON);

	asm	plp;
}


//===================================================================
//										SOUND TEST
//===================================================================


//-------------------------------------------------------------------
// SoundTest
//-------------------------------------------------------------------

void SoundTest (void)
{
	int		snd;
	ushort	freq;

	memset (tilescreen,0,sizeof(tilescreen));
	TilePrint (0x2000, 10,4,"SoundTest");

	TilePrint (0x2000, 4,6, "00 01 02 03 04 05 06 07");
	TilePrint (0x2000, 4,8, "08 09 10 11 12 13 14 15");
	TilePrint (0x2000, 4,10,"16 17 18 19 20 21 22 23");
	TilePrint (0x2000, 4,12,"24 25 26 27 28 29 30 31");
	TilePrint (0x2000, 4,14,"32 33 34 35 36 37 38 39");
	TilePrint (0x2000, 4,16,"40 41 42 43 44 45 46 47");
	TilePrint (0x2000, 4,18,"48 49 50 51 52 53 54 55");
	TilePrint (0x2000, 4,20,"56 57 58 59 60 61 62 63");

	SetupTextPicScreen (&credit, &dimpal,25);
	bg2vofs = 255;
	bg2vofs = 0;		// 0 high byte

	FadeIn ();
	snd = 0;
	freq = 0x560;

	do
	{
		snd = GridSelect (4*8,6*8+4,8,8, snd);
		if (joystick1 & JOYPAD_START)
			break;
		if (joystick1 & JOYPAD_X)
			IO_PlaySound (0,snd, FREQ_HIGH, 0x40, 0x40);
		else if (joystick1 & JOYPAD_Y)
			IO_PlaySound (0,snd, FREQ_LOW, 0x40, 0x40);
		else
			IO_PlaySound (0,snd, FREQ_NORM, 0x40, 0x40);

	} while (1);

	FadeOut ();
}


//-------------------------------------------------------------------
// MusicTest
//-------------------------------------------------------------------

void MusicTest (void)
{
	int	  mus;

	memset (tilescreen,0,sizeof(tilescreen));
	TilePrint (0x2000, 10,3,"Music Test");

	TilePrint (0x2000, 12,5, "Song 1");
	TilePrint (0x2000, 12,7, "Song 2");
	TilePrint (0x2000, 12,9 ,"Song 3");
	TilePrint (0x2000, 12,11,"Song 4");
	TilePrint (0x2000, 12,13,"Song 5");
	TilePrint (0x2000, 12,15,"Song 6");
	TilePrint (0x2000, 12,17,"Song 7");
	TilePrint (0x2000, 12,19,"Song 8");
	TilePrint (0x2000, 12,21,"Song 9");
	TilePrint (0x2000, 12,23,"Song 10");
	TilePrint (0x2000, 12,25,"Song 11");

	SetupTextPicScreen (&credit, &dimpal,25);
	bg2vofs = 255;
	bg2vofs = 0;		// 0 high byte

	FadeIn ();
	mus = 0;
	StopSong ();

	do
	{
		mus = GridSelect (10*8,5*8,1,11, mus);
		if (joystick1 & JOYPAD_START)
			break;

		if (joystick1 & JOYPAD_B)
			StartSong (mus,true);
		else if (joystick1 & JOYPAD_SELECT)
			StopSong ();
		else
			StartSong (mus,false);

	} while (1);

	nmihook = 0;

	FadeOut ();
}


#ifdef	ID_VERSION
//-------------------------------------------------------------------
// InstrumentTest
//-------------------------------------------------------------------

short    value[6] = {3,50,64,0,0,0};
short    minvalue[6] = {0,0,0,0,0,0};
short    maxvalue[6] = {7,127,127,0x7FFF,0x7FFF,10};

void InstrumentTest (void)
{
	char	 str[10];
	short    y, i;

	memset (tilescreen,0,sizeof(tilescreen));
	TilePrint (0x2000, 6,8,"Instrument Test");

	TilePrint (0x2000, 6,10,"Instrument    :");
	TilePrint (0x2000, 6,11,"MIDI note     :");
	TilePrint (0x2000, 6,12,"MIDI velocity :");
	TilePrint (0x2000, 6,13,"Pitch scale   :");
	TilePrint (0x2000, 6,14,"Velocity scale:");
	TilePrint (0x2000, 6,15,"Test song     :");

	SetupTextPicScreen (&credit, &dimpal, 25);
	bg2vofs = 255;
	bg2vofs = 0;		// 0 high byte

	FadeIn ();

	y = 0;
	oam.s[0].x = 27*8;
	oam.s[1].x = 28*8;

	do
	{
		value[3] = pitchscale[value[0]];
		value[4] = velocityscale[value[0]];

		for (i=0 ; i<6 ; i++)
		{
			itox(value[i],str);
			TilePrint (0x2000,21,i+10,str);
		}

		oam.s[1].y = oam.s[0].y = 8*(y+10);
		oam.s[0].name = 0x6e + 2*(ticcount&8);
		oam.s[1].name = 0x6f + 2*(ticcount&8);
		IO_WaitVBL(4);
		DmaOAram (&oam, 0, sizeof(oam));		// Upload sprite records
		DmaVram (tilescreen, 0x7C00, sizeof(tilescreen));


// 	    if (joystick1 & JOYPAD_START)
// 		    break;


		if ( (joystick1 & JOYPAD_UP) && (y>0) )
		{
			y--;
			while (joystick1 & JOYPAD_UP )
			;
		}

		if ( (joystick1 & JOYPAD_DN) && (y<5) )
		{
			y++;
			while (joystick1 & JOYPAD_DN )
				;
		}

		if ( joystick1 & JOYPAD_LFT )
			value[y]--;
		if ( joystick1 & JOYPAD_RGT )
			value[y]++;
		if ( joystick1 & JOYPAD_TL )
			value[y]-=0x10;
		if ( joystick1 & JOYPAD_TR )
			value[y]+=0x10;
		if (value[y] < minvalue[y])
			value[y] = minvalue[y];
		if (value[y] > maxvalue[y])
			value[y] = maxvalue[y];
		if (y == 3)
			pitchscale[value[0]] = value[y];
		if (y == 4)
			velocityscale[value[0]] = value[y];

		if (joystick1 & JOYPAD_B )
		{
			StopSong ();
			IO_NoteOn (0,sfxtoinst[value[0]], value[1], value[2]);
			while (joystick1)
			;
			IO_NoteOff (0);
		}
		if (joystick1 & JOYPAD_A )
		{
			StartSong (value[5],false);
			while (joystick1)
				;
		}


	} while (1);

	StopSong ();
	nmihook = 0;
	FadeOut ();
}
#endif

