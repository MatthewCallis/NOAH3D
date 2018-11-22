//-------------------------------------------------------------------
// SNDDUMP.C - Extracts sound data from id's SOUNDS.BIN file
// 10.10.94 jgt
//-------------------------------------------------------------------

//#define	EXTRACT


#include	<stdio.h>
#include	<stdlib.h>

typedef unsigned char	byte;		// 1 byte
typedef unsigned int		word;		// 2 bytes
typedef unsigned long	dword;	// 4 bytes


#define	MAXSOUNDS		64

#define	HEADER_OFFSET	0x404
#define	DATA_OFFSET		0x504
#define	DATA_ADJUST		0x700


struct {
	word	offset;
	word	loop;
} soundlist[MAXSOUNDS];

int	sound, soundlength;
FILE	*soundfile;

#ifdef	EXTRACT

FILE	*outfile;
char	outname[80];

#endif


void main (int argc, char *argv[])
{
	int	i, c;


	printf ("\nSNDDUMP(v1.0) Sound data extractor for ID's SNES code\n");
	printf ("(c)1994 Color Dreams, Inc.  All rights reserved.\n\n");

	if (argc != 2)
	{
		printf ("Usage is: SNDDUMP <sound #>\n");
		exit (1);
	}

	sound = atoi (argv[1]);

	if (sound < 0 || sound >= MAXSOUNDS)
	{
		printf ("Illegal sound number %d!\n", sound);
		exit (1);
	}

	if ((soundfile = fopen ("sounds.bin", "rb")) == (FILE *) NULL)
	{
		printf ("Error opening SOUNDS.BIN!\n");
		exit (1);
	}

	if (fseek (soundfile, HEADER_OFFSET, SEEK_SET))
	{
		printf ("Error setting file pointer!\n");
		exit (1);
	}

	if (fread (soundlist, sizeof (soundlist), 1, soundfile) != 1)
	{
		printf ("Error reading sound header!\n");
		exit (1);
	}

	printf ("Sound\t$%02X\n\n", sound);

	printf ("Offset\t$%04X\n", soundlist[sound].offset);
	printf ("Loop\t$%04X\n", soundlist[sound].loop);
	soundlength = soundlist[sound+1].offset - soundlist[sound].offset;
	printf ("Length\t$%04X\n\n", soundlength);

	if (fseek (soundfile, DATA_OFFSET-DATA_ADJUST+soundlist[sound].offset, SEEK_SET))
	{
		printf ("Error setting file pointer!\n");
		exit (1);
	}

#ifdef	EXTRACT
	sprintf (outname, "sound_%02X.bin", sound);
	outfile = fopen (outname, "wb");
#endif

	printf ("Data:");
	for (i = 0; i < soundlength; i++)
	{
		if (!(i % 16))
			printf ("\n$%04X:\t", i);
		c = fgetc (soundfile);
		printf ("%02X ", c);
#ifdef	EXTRACT
		fputc (c, outfile);
#endif
	}

	printf ("\n\n");
	fclose (soundfile);
#ifdef	EXTRACT
	fclose (outfile);
#endif
}
