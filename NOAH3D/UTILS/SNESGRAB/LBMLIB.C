// lbmlib.c

#include <stdio.h>
#include <string.h>
#include <alloc.h>
#include <dos.h>
#include <mem.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <io.h>
#include <process.h>
#include <bios.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>

#include "cmdlib.h"
#include "lbmlib.h"



/*
============================================================================

						LBM STUFF

============================================================================
*/


#define FORMID	('F'+('O'<<8)+((long)'R'<<16)+((long)'M'<<24))
#define ILBMID	('I'+('L'<<8)+((long)'B'<<16)+((long)'M'<<24))
#define PBMID   ('P'+('B'<<8)+((long)'M'<<16)+((long)' '<<24))
#define BMHDID  ('B'+('M'<<8)+((long)'H'<<16)+((long)'D'<<24))
#define BODYID  ('B'+('O'<<8)+((long)'D'<<16)+((long)'Y'<<24))
#define CMAPID  ('C'+('M'<<8)+((long)'A'<<16)+((long)'P'<<24))


bmhd_t	bmhd;

long	Align (long l)
{
	if (l&1)
		return l+1;
	return l;
}



/*
================
=
= LBMRLEdecompress
=
= Source must be evenly aligned!
=
================
*/

byte huge *LBMRLEDecompress (byte *source,byte *unpacked
	,int bpwidth)
{
	int 	count,plane;
	byte	b,rept;

	count = 0;

	do
	{
		rept = *source++;

		if (rept > 0x80)
		{
			rept = (rept^0xff)+2;
			b = *source++;
			memset(unpacked,b,rept);
			unpacked += rept;
		}
		else if (rept < 0x80)
		{
			rept++;
			memcpy(unpacked,source,rept);
			unpacked += rept;
			source += rept;
		}
		else
			rept = 0;		// rept of 0x80 is NOP

		count += rept;

	} while (count<bpwidth);

	if (count>bpwidth)
		Error ("Decompression exceeded width!\n");


	return source;
}


#define BPLANESIZE	128
byte	bitplanes[9][BPLANESIZE];	// max size 1024 by 9 bit planes


/*
=================
=
= MungeBitPlanes8
=
= This destroys the bit plane data!
=
=================
*/

void MungeBitPlanes8 (int width, byte *dest)
{
asm	les	di,[dest]
asm	mov	si,-1
asm	mov	cx,[width]
mungebyte:
asm	inc	si
asm	mov	dx,8
mungebit:
asm	shl	[BYTE PTR bitplanes + BPLANESIZE*7 +si],1
asm	rcl	al,1
asm	shl	[BYTE PTR bitplanes + BPLANESIZE*6 +si],1
asm	rcl	al,1
asm	shl	[BYTE PTR bitplanes + BPLANESIZE*5 +si],1
asm	rcl	al,1
asm	shl	[BYTE PTR bitplanes + BPLANESIZE*4 +si],1
asm	rcl	al,1
asm	shl	[BYTE PTR bitplanes + BPLANESIZE*3 +si],1
asm	rcl	al,1
asm	shl	[BYTE PTR bitplanes + BPLANESIZE*2 +si],1
asm	rcl	al,1
asm	shl	[BYTE PTR bitplanes + BPLANESIZE*1 +si],1
asm	rcl	al,1
asm	shl	[BYTE PTR bitplanes + BPLANESIZE*0 +si],1
asm	rcl	al,1
asm	stosb
asm	dec	cx
asm	jz	done
asm	dec	dx
asm	jnz	mungebit
asm	jmp	mungebyte

done:
}


void MungeBitPlanes4 (int width, byte *dest)
{
asm	les	di,[dest]
asm	mov	si,-1
asm	mov	cx,[width]
mungebyte:
asm	inc	si
asm	mov	dx,8
mungebit:
asm	xor	al,al
asm	shl	[BYTE PTR bitplanes + BPLANESIZE*3 +si],1
asm	rcl	al,1
asm	shl	[BYTE PTR bitplanes + BPLANESIZE*2 +si],1
asm	rcl	al,1
asm	shl	[BYTE PTR bitplanes + BPLANESIZE*1 +si],1
asm	rcl	al,1
asm	shl	[BYTE PTR bitplanes + BPLANESIZE*0 +si],1
asm	rcl	al,1
asm	stosb
asm	dec	cx
asm	jz	done
asm	dec	dx
asm	jnz	mungebit
asm	jmp	mungebyte

done:
}


void MungeBitPlanes2 (int width, byte *dest)
{
asm	les	di,[dest]
asm	mov	si,-1
asm	mov	cx,[width]
mungebyte:
asm	inc	si
asm	mov	dx,8
mungebit:
asm	xor	al,al
asm	shl	[BYTE PTR bitplanes + BPLANESIZE*1 +si],1
asm	rcl	al,1
asm	shl	[BYTE PTR bitplanes + BPLANESIZE*0 +si],1
asm	rcl	al,1
asm	stosb
asm	dec	cx
asm	jz	done
asm	dec	dx
asm	jnz	mungebit
asm	jmp	mungebyte

done:
}


void MungeBitPlanes1 (int width, byte *dest)
{
asm	les	di,[dest]
asm	mov	si,-1
asm	mov	cx,[width]
mungebyte:
asm	inc	si
asm	mov	dx,8
mungebit:
asm	xor	al,al
asm	shl	[BYTE PTR bitplanes + BPLANESIZE*0 +si],1
asm	rcl	al,1
asm	stosb
asm	dec	cx
asm	jz	done
asm	dec	dx
asm	jnz	mungebit
asm	jmp	mungebyte

done:
}


/*
=================
=
= LoadLBM
=
=================
*/

void LoadLBM (char *filename, byte **picture, byte **palette)
{
	byte	*LBMbuffer, *picbuffer, *cmapbuffer;
	int		i,y,p,planes;
	byte	huge *LBM_P, huge *LBMEND_P;
	byte	*pic_p;
	byte	huge *body_p;
	unsigned	linesize,rowsize;

	long	formtype,formlength;
	long	chunktype,chunklength;
	void	(*mungecall) (int, byte *);

//
// load the LBM
//
	LoadFile (filename, (void **)&LBMbuffer);

//
// parse the LBM header
//
	LBM_P = LBMbuffer;
	if ( *(long *)LBMbuffer != FORMID )
	   Error ("No FORM ID at start of file!\n");

	LBM_P += 4;
	formlength = MotoLong( *(long *)LBM_P );
	LBM_P += 4;
	LBMEND_P = LBM_P + Align(formlength);

	formtype = *(long *)LBM_P;

	if (formtype != ILBMID && formtype != PBMID)
		Error ("Unrecognized form type: %c%c%c%c\n", formtype&0xff
		,(formtype>>8)&0xff,(formtype>>16)&0xff,(formtype>>24)&0xff);

	LBM_P += 4;

//
// parse chunks
//

	while (LBM_P < LBMEND_P)
	{
		chunktype = *(long *)LBM_P;
		LBM_P += 4;
		chunklength = MotoLong(*(long *)LBM_P);
		LBM_P += 4;

		switch (chunktype)
		{
		case BMHDID:
			memcpy (&bmhd,LBM_P,sizeof(bmhd));
			bmhd.w = MotoShort(bmhd.w);
			bmhd.h = MotoShort(bmhd.h);
			bmhd.x = MotoShort(bmhd.x);
			bmhd.y = MotoShort(bmhd.y);
			bmhd.pageWidth = MotoShort(bmhd.pageWidth);
			bmhd.pageHeight = MotoShort(bmhd.pageHeight);
			break;

		case CMAPID:
			cmapbuffer = SafeMalloc (768);
			memset (cmapbuffer, 0, 768);
			memcpy (cmapbuffer, LBM_P, chunklength);
			break;

		case BODYID:
			body_p = LBM_P;

			pic_p = picbuffer = SafeMalloc (bmhd.w*bmhd.h);

			if (formtype == PBMID)
			{
			//
			// unpack PBM
			//
				for (y=0 ; y<bmhd.h ; y++, pic_p += bmhd.w)
				{
					if (bmhd.compression == cm_rle1)
						body_p = LBMRLEDecompress ((byte *)body_p
						, pic_p , bmhd.w);
					else if (bmhd.compression == cm_none)
					{
						memcpy (pic_p,body_p,bmhd.w);
						body_p += Align(bmhd.w);
					}
				}

			}
			else
			{
			//
			// unpack ILBM
			//
				planes = bmhd.nPlanes;
				if (bmhd.masking == ms_mask)
					planes++;
				rowsize = (bmhd.w+15)/16 * 2;
				switch (bmhd.nPlanes)
				{
				case 1:
					mungecall = MungeBitPlanes1;
					break;
				case 2:
					mungecall = MungeBitPlanes2;
					break;
				case 4:
					mungecall = MungeBitPlanes4;
					break;
				case 8:
					mungecall = MungeBitPlanes8;
					break;
				default:
					Error ("Can't munge %i bit planes!\n",bmhd.nPlanes);
				}

				for (y=0 ; y<bmhd.h ; y++, pic_p += bmhd.w)
				{
					for (p=0 ; p<planes ; p++)
						if (bmhd.compression == cm_rle1)
							body_p = LBMRLEDecompress ((byte *)body_p
							, bitplanes[p] , rowsize);
						else if (bmhd.compression == cm_none)
						{
							memcpy (bitplanes[p],body_p,rowsize);
							body_p += rowsize;
						}

					mungecall (bmhd.w , pic_p);
				}
			}
			break;
		}

		LBM_P += Align(chunklength);
	}

	free (LBMbuffer);

	*picture = picbuffer;
	*palette = cmapbuffer;
}


/*
============================================================================

							WRITE LBM

============================================================================
*/

/*
==============
=
= WriteLBMfile
=
==============
*/

void WriteLBMfile (char *filename, byte *data, int width, int height, byte *palette)
{
	byte	*lbm, *lbmptr;
	long	*formlength, *bmhdlength, *cmaplength, *bodylength;
	long	length;
	int		handle;
	bmhd_t	basebmhd;

	lbm = lbmptr = SafeMalloc (width*height+1000);

//
// start FORM
//
	*lbmptr++ = 'F';
	*lbmptr++ = 'O';
	*lbmptr++ = 'R';
	*lbmptr++ = 'M';

	formlength = (long*)lbmptr;
	lbmptr+=4;			// leave space for length

	*lbmptr++ = 'P';
	*lbmptr++ = 'B';
	*lbmptr++ = 'M';
	*lbmptr++ = ' ';

//
// write BMHD
//
	*lbmptr++ = 'B';
	*lbmptr++ = 'M';
	*lbmptr++ = 'H';
	*lbmptr++ = 'D';

	bmhdlength = (long *)lbmptr;
	lbmptr+=4;			// leave space for length

	memset (&basebmhd,0,sizeof(basebmhd));
	basebmhd.w = MotoShort(width);
	basebmhd.h = MotoShort(height);
	basebmhd.nPlanes = MotoShort(8);
	basebmhd.xAspect = MotoShort(5);
	basebmhd.yAspect = MotoShort(6);
	basebmhd.pageWidth = MotoShort(width);
	basebmhd.pageHeight = MotoShort(height);

	memcpy (lbmptr,&basebmhd,sizeof(basebmhd));
	lbmptr += sizeof(basebmhd);

	length = lbmptr-(byte *)bmhdlength-4;
	*bmhdlength = MotoLong(length);
	if (length&1)
		*lbmptr++ = 0;		// pad chunk to even offset

//
// write CMAP
//
	*lbmptr++ = 'C';
	*lbmptr++ = 'M';
	*lbmptr++ = 'A';
	*lbmptr++ = 'P';

	cmaplength = (long *)lbmptr;
	lbmptr+=4;			// leave space for length

	memcpy (lbmptr,palette,768);
	lbmptr += 768;

	length = lbmptr-(byte *)cmaplength-4;
	*cmaplength = MotoLong(length);
	if (length&1)
		*lbmptr++ = 0;		// pad chunk to even offset

//
// write BODY
//
	*lbmptr++ = 'B';
	*lbmptr++ = 'O';
	*lbmptr++ = 'D';
	*lbmptr++ = 'Y';

	bodylength = (long *)lbmptr;
	lbmptr+=4;			// leave space for length

	memcpy (lbmptr,data,width*height);
	lbmptr += width*height;

	length = lbmptr-(byte *)bodylength-4;
	*bodylength = MotoLong(length);
	if (length&1)
		*lbmptr++ = 0;		// pad chunk to even offset

//
// done
//
	length = lbmptr-(byte *)formlength-4;
	*formlength = MotoLong(length);
	if (length&1)
		*lbmptr++ = 0;		// pad chunk to even offset

//
// write output file
//
	SaveFile (filename, lbm, lbmptr-lbm);
	free (lbm);
}

