//-------------------------------------------------------------------
// SNDCODE.C - SNES Digitized sound grabber
//
// Ported from Id's NeXT source code AUG 1994 by Jim Treadway
//-------------------------------------------------------------------


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>

#define	VOLUME		12

#define	MAXFILENAME	80

typedef	unsigned char	byte;
typedef	unsigned int	word;
typedef	unsigned long	dword;


struct {
	char	IDString[0x14];
	word	DataOffset;
	word	Version;
	word	CheckSum;
} VocHeader;


word	SampleRate;
dword	Samples;

int			source[18];		// Signed 16 bit
signed char	dest[18];		// Signed 4-bit

int			filtered[4][16];
int			filteredrange[4];

FILE			*inf, *outf;

int			minf;				// Filter number with lowest range

//-------------------------------------------------------------------

void Error (char const *format, ...)
{
	char		msg[1025];
	va_list 	args;
	
	va_start(args, format);
	vsprintf(msg, format, args);
	va_end(args);
	strcat (msg,"\n");

	printf(msg);
	exit(1);
}


//-------------------------------------------------------------------

byte *ReadVocFile(word DataOffset)
{
	int	done, i, c;
	byte	*data;
	dword	BlockLen;

	done = SampleRate = Samples = 0;
	data = NULL;

	if(fseek(inf,DataOffset,SEEK_SET))
		Error("\x07" "Error reading VOC file!");

	while(!done) {
		if(feof(inf))
			Error("\x07" "Unexpected EOF reading VOC file!");

		c = fgetc(inf);	// Get block type

//		printf("Block Type %02X\n", c);

		switch(c) {
			case 0x00:	done = 1; break;

			case 0x03:	Error("\x07" "SILENCE blocks not supported!"); break;

			case 0x06:
			case 0x07:	Error("\x07" "REPEAT loops not supported!"); break;

			case 0x01:
					fread(&BlockLen, 3, 1, inf); BlockLen &= 0xFFFFFF;

					c = fgetc(inf);

					if(!SampleRate) {
						SampleRate = 1000000 / (256 - c);
//						printf("(%d kHz) ", SampleRate);
					}		

					if((c = fgetc(inf)) != 0x00) // Pack
						Error("\x07" "Packed VOC files not supported!");

					BlockLen -= 2;

					if(data == NULL) {
						if((data = malloc(BlockLen)) == NULL)
							Error("\x07" "Cannot allocate %d bytes!", BlockLen-2);
						fread(data, BlockLen, 1, inf);
					} else {
						if((data = realloc(data, Samples+BlockLen)) == NULL)
							Error("\x07" "Cannot re-allocate %d bytes!", Samples+BlockLen);
						fread(data+Samples, BlockLen, 1, inf);
					}

					Samples += BlockLen;
					break;
					
			case 0x08:
					fread(&BlockLen, 3, 1, inf); BlockLen &= 0xFFFFFF;
					if(BlockLen != 4)
						Error("\x07" "Error in EXTENDED block length!");

					fread(&SampleRate, sizeof SampleRate, 1, inf);
					SampleRate = 256000000 / (65532 - SampleRate);
//					printf("(%d kHz) ", SampleRate);

					if((c = fgetc(inf)) != 0x00) // Pack
						Error("\x07" "Packed VOC files not supported!");

					if((c = fgetc(inf)) != 0x00) // Mode
						Error("\x07" "Stereo VOC files not supported!");

					break;

			case 0x02:
					fread(&BlockLen, 3, 1, inf); BlockLen &= 0xFFFFFF;

					if(data == NULL) {
						if((data = malloc(BlockLen)) == NULL)
							Error("\x07" "Cannot allocate %d bytes!", BlockLen-2);
						fread(data, BlockLen, 1, inf);
					} else {
						if((data = realloc(data, Samples+BlockLen)) == NULL)
							Error("\x07" "Cannot re-allocate %d bytes!", Samples+BlockLen);
						fread(data+Samples, BlockLen, 1, inf);
					}

					Samples += BlockLen;
					break;

			case 0x04:	// Marker
			case 0x05:	// ASCII text
					
					fread(&BlockLen, 3, 1, inf); BlockLen &= 0xFFFFFF;
					for(i = 0; i < BlockLen; i++)
						c = fgetc(inf);
					break;

			default:		Error("\x07" "Unsupported block type $%02X!",c);

		}
	}
	
	return	data;
}


//-------------------------------------------------------------------

float	Afilter[4] = {0, 0.9375, 1.90625 , 1.796875};
float	Bfilter[4] = {0, 0, -0.9375 , -0.8125 };

void FilterSource (void)
{
	int	i,f;
	int	val, max, min;
	float	a,b;
	
	min = MAXINT;
	for (f=0 ; f<4 ; f++)
	{
		a = Afilter[f];
		b = Bfilter[f];
		max = 0;
		for (i = 0; i < 16; i++)
		{
			val = source[i+2] - (a*source[i+1] + b*source[i]);
			filtered[f][i] = val;
			val = abs(val);
			if (val > max)
				max = val;
		}
		filteredrange[f] = max;
		if (max < min)
		{
			min = max;
			minf = f;
		}
	}
}


//-------------------------------------------------------------------
// CodeSource		Bit codes one of the filtered source ranges
//-------------------------------------------------------------------

void CodeSource (int filter, int flags)
{
	unsigned			shift, s, round;
	unsigned char	databyte;
	
//------------------------
// Find shift bits
//------------------------

	for (shift = 0; shift < 15; shift ++ )
	{
		if (shift > 0)
			round = 1<<(shift-1);
		else
			round = 0;
		if ( (filteredrange[filter]+round) >> shift <= 7)
			break;
	}
	
//	printf ("(%i:%i)",filter,shift);

//------------------------
// Scale filtered samples
//------------------------

	for (s = 0; s < 16; s++)
	{
		dest[s] = ((int)(filtered[filter][s]+round))>>shift;

		if (dest[s] < -7 || dest[s] > 7) {
			printf("\n\n dest = %d\n", dest[s]);
			Error ("\x07" "Bad bit range");
		}
	}

//------------------------
// Write the packet
//------------------------

	databyte = (shift<<4) | (filter<<2) | flags;
	putc (databyte, outf);
	for (s = 0; s < 16; s += 2)
	{
		databyte = ((dest[s]&15)<<4) + (dest[s+1]&15);
		putc (databyte, outf);
	}
}


//-------------------------------------------------------------------
// ConvertSoundFile
//-------------------------------------------------------------------

void ConvertSoundFile (char const *name)
{
	int		cursample;
	int		f, frames, i;
	byte		*data;			// Original samples
	unsigned	s;
	char		outfname[MAXFILENAME], infname[MAXFILENAME],*temp;
	
//------------------------------------------
// Load sound
//------------------------------------------

	strcpy(infname,name);
	printf("Processing '%s': ",infname);
	strcat(infname,".voc");

	if((inf = fopen(infname,"rb")) == (FILE *) NULL) {
		printf("Unable to open '%s'!\n", infname);
		exit(1);
	}

//------------------------------------------
// Read VOC file
//------------------------------------------

	fread(&VocHeader, sizeof VocHeader, 1, inf);

	VocHeader.IDString[0x13] = '\0';

	if(strcmp(VocHeader.IDString,"Creative Voice File"))
		Error("\x07" "VOC file ID string invalid!");
	if(VocHeader.CheckSum != (~VocHeader.Version + 0x1234))
		Error("\x07" "VOC file checksum invalid!");

	printf("(VOC File Ver %d.%02d) ", (VocHeader.Version>>8)&0xFF,VocHeader.Version&0xFF);

	data = ReadVocFile(VocHeader.DataOffset);

	printf("(%d kHz, %d samples) \n", SampleRate, Samples);

//------------------------------------------
// Open output file
//------------------------------------------

	strcpy (outfname,infname);
	if ((temp = strchr(outfname,'.')) != NULL)	// Clip off extension
		*temp = '\0';
	strcat (outfname,".sns");

	if((outf = fopen(outfname,"wb")) == (FILE *) NULL)
		Error("\x07" "Can't open output file '%s'!", outfname);

	memset (source,0,sizeof(source));

//------------------------------------------
// Convert to SNES packets of 16 samples
//------------------------------------------

	frames = Samples/16;
	for (f=0 ; f<frames; f++)
	{
		printf("Frame %d\r", f);

	//---------------------------------------
	// Copy last 2 samples to start for
	// back filtering
	//---------------------------------------

		source[0] = source[16];
		source[1] = source[17];
		
	//---------------------------------------
	// Get source samples
	//---------------------------------------

		if (f == frames-1 && (Samples&15)) {	// Partial packet
			memset (source,0,sizeof(source));
			for (cursample = 0; cursample < (Samples&15); cursample++)
				source[cursample] = (*data++)-0x80;
		} else {											// Full 16 samples
			for (cursample = 0; cursample < 16; cursample++)
				source[cursample] = (*data++)-0x80;
		}

		for (s = 2; s < 18; s++) {
			//shit//fuck: This should be '<<= 8'!!!!
			source[s] <<= 7;		// Convert to 16-bit samples
		}

		FilterSource ();
		minf = 0;	// DEBUG: others not working?
		if (f == 0)
			CodeSource (0,0);	// must use 0 order filter on first frame
		else
		{	// code the filter with the smallest range
			if (f==frames-1)
				CodeSource (minf,1);	// last frame flag
			else
				CodeSource (minf,0);
		}			
	}

	printf ("Done!       ");
	fclose (outf);

//	free(data);
	printf("\n");
}


//-------------------------------------------------------------------
// main()
//-------------------------------------------------------------------

void main(int argc, char *argv[])
{
	int	i;

	printf("SNDCODE(v1.0) - SNES Digitized sound converter\n");
	printf("From ID's NeXT source code.\n\n");

	if(argc == 1) {
		printf("Usage is: SNDCODE <file1> [<file2> [<file3] ..]\n");
		exit(1);
	}

	for(i = 1; i < argc; i++)
		ConvertSoundFile(argv[i]);

	exit(0);
}

