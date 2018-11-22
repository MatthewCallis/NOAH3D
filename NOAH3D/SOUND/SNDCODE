From johnc@idcube.idsoftware.com Fri Aug 19 13:46:52 1994
Date: Thu, 18 Aug 94 20:21:29 -0600
From: John Carmack <johnc@idcube.idsoftware.com>
To: Vance Kozik <vance@crl.com>
Subject: Re: SNES snds

Ok, here is the code for the utility that generated SNES sounds (a  
tiny NeXT program).  I wrote this, but only because the original  
development plan fell apart.  I make no claim that the sound code in  
wolf is worth a damn.

John Carmack

#import <appkit/appkit.h>
#import "SSCoord.h"

/*
===================
=
= Error
=
===================
*/

void Error (char const *format, ...)
{
	char		msg[1025];
	va_list 	args;
	
	va_start(args, format);
	vsprintf(msg, format, args);
	va_end(args);
	strcat (msg,"\n");
	
	NXRunAlertPanel ("Error",msg,0,0,0);
	NX_RAISE (NX_APPBASE, msg, 0);
}



short	source[18];		// signed 16 bit
short	dest[18];			// signed 4 bit

int		filtered[4][16];
int		filteredrange[4];

FILE		*outfile;

int		minf;				// filter number with  
lowest range

/*
==============
=
= FilterRange
=
==============
*/

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
		for (i=0 ; i<16 ; i++)
		{
			val = source[i+2] - ( a* source[i+1] + b*  
source[i] );
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

/*
=============
=
= CodeSource
=
= Bit codes one of the filtered source ranges
=============
*/

void CodeSource (int filter, int flags)
{
	unsigned	shift, s, round;
	unsigned char databyte;
	
//
// find shift bits
//
	for (shift = 0 ; shift < 15 ; shift ++ )
	{
		if (shift > 0)
			round = 1<<(shift-1);
		else
			round = 0;
		if ( (filteredrange[filter]+round) >> shift <= 7)
			break;
	}
	
//	printf ("(%i:%i)",filter,shift);
//
// scale filtered samples
//			
	for (s=0 ; s<16 ; s++)
	{
		dest[s] = (filtered[filter][s]+round) >>shift;
		if (dest[s] < -7 || dest[s] > 7)
			Error ("Bad bit range");
	}

//
// write the packet
//
	databyte = (shift<<4) | (filter<<2) | flags;
	putc (databyte, outfile);
	for (s=0 ; s<16 ; s+=2)
	{
		databyte = ((dest[s]&15)<<4) + (dest[s+1]&15);
		putc (databyte, outfile);
	}
}


/*
================
=
= ConvertSoundFile
=
================
*/

void ConvertSoundFile (char const *path)
{
	struct Sound *sound;
	int		err, samples;
	int		f, frames;
	short	*data;				// original samples
	unsigned	 s;
	char		outname[MAXPATHLEN];
	
//
// load sound
//
printf ("Converting %s...\n",path);
	sound = [[Sound alloc] initFromSoundfile: path];
	if (!sound)
		Error ("Couldn't init %s",path);
//	[sound play];

//
// convert to 16 bit linear, 32khz sampling
//
	err = [sound 

		convertToFormat: SND_FORMAT_LINEAR_16
		samplingRate:		11025
		channelCount:	1
	];
	
	if (err)
		Error ("Conversion error %i on %s",err,path);

//
// open output file
//
	strcpy (outname,path);
	if (strlen(outname) > 4)
		outname[strlen(outname)-4] = 0;	// clip off extension
	strcat (outname,".sns");
	outfile = fopen (outname,"w");
	if (!outfile)
		Error ("Couldn't open %s",outname);
		
//===============================	
//
// convert to SNES packets of 16 samples
//
//===============================	
	data = (short *)[sound data];
	samples = [sound sampleCount];
	
	frames = samples/16;
	for (f=0 ; f<frames ; f++)
	{
	// copy last two samples to start for back filtering
		source[0] = source[16];
		source[1] = source[17];
		
	// get source samples
		if (f  == frames-1 && (samples&15) )
		{	// partial packet
			memset (source,0,sizeof(source));
			memcpy (source+2, data, 2*(samples&15));
		}
		else
		{	// full 16 samples
			memcpy (source+2, data, 32);
			data += 16;
		}
		for (s=2 ; s<18 ; s++)
			source[s] = NXSwapBigShortToHost (source[s]);
		FilterSource ();
minf = 0;	// DEBUG: others not working?
		if (f==0)
			CodeSource (0,0);	// must use 0 order  
filter on first frame
		else
		{	// code the filter with the smallest range
			if (f==frames-1)
				CodeSource (minf,1);	// last frame  
flag
			else
				CodeSource (minf,0);
		}			
	}
	
	fclose (outfile);
	[sound free];
//printf ("\n");
}

@implementation SSCoord

/*
==================
=
= open
=
==================
*/

- open:sender
{
	id			openpanel;
	static char	*suffixlist[] = {"snd", 0};
	const char	* const *filenames;
	char			fullpath[256];

	openpanel = [OpenPanel new];
	[openpanel allowMultipleFiles: YES];
	[openpanel setTitle: "Convert sounds"];
	if (! [openpanel runModalForTypes:suffixlist] )
		return self;
	filenames = [openpanel filenames];

	if (filenames)
	{
		while (*filenames)
		{
			strcpy (fullpath,[openpanel directory]);
			strcat (fullpath,"/");
			strcat (fullpath,*filenames);
			ConvertSoundFile(fullpath);
			filenames++;	
		}	
	}

	return self;
}


- appDidInit: sender
{
	return [self open:self];
}

@end
