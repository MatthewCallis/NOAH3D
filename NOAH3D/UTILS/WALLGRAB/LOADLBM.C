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


#include "lumpy.h"
#pragma hdrstop


/*
============================================================================

						LBM STUFF

============================================================================
*/

#define PEL_WRITE_ADR		0x3c8
#define PEL_READ_ADR		0x3c7
#define PEL_DATA			0x3c9


#define FORMID	('F'+('O'<<8)+((long)'R'<<16)+((long)'M'<<24))
#define ILBMID	('I'+('L'<<8)+((long)'B'<<16)+((long)'M'<<24))
#define PBMID   ('P'+('B'<<8)+((long)'M'<<16)+((long)' '<<24))
#define BMHDID  ('B'+('M'<<8)+((long)'H'<<16)+((long)'D'<<24))
#define BODYID  ('B'+('O'<<8)+((long)'D'<<16)+((long)'Y'<<24))
#define CMAPID  ('C'+('M'<<8)+((long)'A'<<16)+((long)'P'<<24))

typedef unsigned char	UBYTE;
typedef short			WORD;
typedef unsigned short	UWORD;
typedef long			LONG;

typedef enum
{
	ms_none,
	ms_mask,
	ms_transcolor,
	ms_lasso
} mask_t;

typedef enum
{
	cm_none,
	cm_rle1
} compress_t;

typedef struct
{
	UWORD		w,h;
	WORD		x,y;
	UBYTE		nPlanes;
	UBYTE		masking;
	UBYTE		compression;
	UBYTE		pad1;
	UWORD		transparentColor;
	UBYTE		xAspect,yAspect;
	WORD		pageWidth,pageHeight;
} bmhd_t;

bmhd_t	bmhd;

unsigned	ShortSwap (unsigned l)
{
	byte	b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

long	LongSwap (long l)
{
	byte	b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((long)b1<<24) + ((long)b2<<16) + ((long)b3<<8) + b4;
}

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

byte huge *LBMRLEDecompress (byte far *source,byte far *unpacked
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
			_fmemset(unpacked,b,rept);
			unpacked += rept;
		}
		else if (rept < 0x80)
		{
			rept++;
			_fmemcpy(unpacked,source,rept);
			unpacked += rept;
			source += rept;
		}
		else
			rept = 0;		// rept of 0x80 is NOP

		count += rept;

	} while (count<bpwidth);

	if (count>bpwidth)
		MS_Quit ("Decompression exceeded width!\n");


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

void MungeBitPlanes8 (int width, byte far *dest)
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


void MungeBitPlanes4 (int width, byte far *dest)
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


void MungeBitPlanes2 (int width, byte far *dest)
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


void MungeBitPlanes1 (int width, byte far *dest)
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

void LoadLBM (char *filename)
{
	int		handle;
	int		i,y,p,planes;
	long	size,remaining;
	unsigned	readsize,iocount;
	byte	far	*LBMbuffer;
	byte	huge *LBM_P, huge *LBMEND_P;
	byte	far  *pic_p;
	byte	huge *body_p;
	unsigned	linesize,rowsize;

	long	formtype,formlength;
	long	chunktype,chunklength;
	void	(*mungecall) (int, byte far *);

//
// load the LBM
//
	handle = _open (filename,O_RDONLY);
	if ( handle == -1)
		MS_Quit ("Cannot open %s\n",filename);

	size = filelength (handle);
	LBMbuffer = farmalloc (size);
	if (!LBMbuffer)
		MS_Quit ("Cannot allocate %lu byte\n",size);

	remaining = size;
	LBM_P = LBMbuffer;

	while (remaining)
	{
		readsize = remaining < 0xf000 ? remaining : 0xf000;
		_dos_read (handle,LBM_P,readsize,&iocount);
		if (iocount != readsize)
			MS_Quit ("Read failure on %s\n",filename);
		remaining -= readsize;
		LBM_P += readsize;
	}

	close (handle);

//
// parse the LBM header
//
	LBM_P = LBMbuffer;
	if ( *(long far *)LBMbuffer != FORMID )
	   MS_Quit ("No FORM ID at start of file!\n");

	LBM_P += 4;
	formlength = LongSwap( *(long far *)LBM_P );
	LBM_P += 4;
	LBMEND_P = LBM_P + Align(formlength);

	formtype = *(long far *)LBM_P;

	if (formtype != ILBMID && formtype != PBMID)
		MS_Quit ("Unrecognized form type: %c%c%c%c\n", formtype&0xff
		,(formtype>>8)&0xff,(formtype>>16)&0xff,(formtype>>24)&0xff);

	LBM_P += 4;

//
// parse chunks
//
  asm	mov	ax,0x13		// go into VGA mode
  asm	int	0x10


	while (LBM_P < LBMEND_P)
	{
		chunktype = *(long far *)LBM_P;
		LBM_P += 4;
		chunklength = LongSwap(*(long far *)LBM_P);
		LBM_P += 4;

		switch (chunktype)
		{
		case BMHDID:
			_fmemcpy (&bmhd,LBM_P,sizeof(bmhd));
			bmhd.w = ShortSwap(bmhd.w);
			bmhd.h = ShortSwap(bmhd.h);
			bmhd.x = ShortSwap(bmhd.x);
			bmhd.y = ShortSwap(bmhd.y);
			bmhd.pageWidth = ShortSwap(bmhd.pageWidth);
			bmhd.pageHeight = ShortSwap(bmhd.pageHeight);
			break;

		case CMAPID:
			outportb (PEL_WRITE_ADR,0);
			for (i=0;i<chunklength;i++)
				outportb(PEL_DATA,LBM_P[i] >> 2);
			break;

		case BODYID:
			body_p = LBM_P;

			if (formtype == PBMID)
			{
			//
			// unpack PBM
			//
				for (y=0 ; y<bmhd.h ; y++)
				{
					pic_p = MK_FP(0xa000,SCREENWIDTH*y);
					if (bmhd.compression == cm_rle1)
						body_p = LBMRLEDecompress ((byte far *)body_p
						, pic_p , bmhd.w);
					else if (bmhd.compression == cm_none)
					{
						_fmemcpy (pic_p,body_p,bmhd.w);
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
					MS_Quit ("Can't munge %i bit planes!\n",bmhd.nPlanes);
				}

				for (y=0 ; y<bmhd.h ; y++)
				{
					pic_p = MK_FP(0xa000,SCREENWIDTH*y);
					for (p=0 ; p<planes ; p++)
						if (bmhd.compression == cm_rle1)
							body_p = LBMRLEDecompress ((byte far *)body_p
							, bitplanes[p] , rowsize);
						else if (bmhd.compression == cm_none)
						{
							_fmemcpy (bitplanes[p],body_p,rowsize);
							body_p += rowsize;
						}

					mungecall (bmhd.w , pic_p);
				}
			}
			break;
		}

		LBM_P += Align(chunklength);
	}

	farfree (LBMbuffer);
}

