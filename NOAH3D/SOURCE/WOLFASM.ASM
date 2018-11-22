*--------------------------------------------------------------------
* WOLFASM.ASM
*--------------------------------------------------------------------

	spaces	on
	llchar	.

	include idmacros.asm
	include	snes.equ

SPC700Port0	set	$2140
SPC700Port1	set	$2141
SPC700Port2	set	$2142
SPC700Port3	set	$2143

	code

	public	~~UnBlankVideo
	public	~~BlankVideo

;====================================================================
;		EXTERNALS
;====================================================================

	extern	~~ramscalers
	extern	~~unwound
	extern	~~unwoundcount
	extern	~~basesrc
	extern	~~screenbuffer

	extern	~~SndWait
	extern	~~SndCmd
	extern	~~currate
	extern	~~introhack
	extern	~~playback
	extern	~~lastcmd
	extern	~~driverflag
	extern	~~~MUL2
	extern	~~fadelevel

	extern	~~notetable1,~~notetable2,~~notetable3
	extern	~~notetable4,~~notetable5

	extern	~~veltable1,~~veltable2,~~veltable3
	extern	~~veltable4,~~veltable5,~~veltable6
	extern	~~veltable7,~~veltable8


;====================================================================
; unsigned int CHECKROM (void)
;
; Attempt to match check sum produced by EEPROM programer:
;  A WORD value equal to the SUM of all BYTES in ROM ???
;

	public	~~CHECKROM

	page0
checksum	dw	0
	ends

~~CHECKROM:	phd
	lda	#0
	tcd

	stz	<checksum

	ldx	#0		;Index

.checkbank	SA	;***************
	lda	>$C00000,x	;Get byte
	LA	;***************
	and	#$00FF
	clc
	adc	<checksum
	sta	<checksum

	SA	;***************
	lda	>$C10000,x	;Get byte
	LA	;***************
	and	#$00FF
	clc
	adc	<checksum
	sta	<checksum

	SA	;***************
	lda	>$C20000,x	;Get byte
	LA	;***************
	and	#$00FF
	clc
	adc	<checksum
	sta	<checksum

	SA	;***************
	lda	>$C30000,x	;Get byte
	LA	;***************
	and	#$00FF
	clc
	adc	<checksum
	sta	<checksum

	SA	;***************
	lda	>$C40000,x	;Get byte
	LA	;***************
	and	#$00FF
	clc
	adc	<checksum
	sta	<checksum

	SA	;***************
	lda	>$C50000,x	;Get byte
	LA	;***************
	and	#$00FF
	clc
	adc	<checksum
	sta	<checksum

	SA	;***************
	lda	>$C60000,x	;Get byte
	LA	;***************
	and	#$00FF
	clc
	adc	<checksum
	sta	<checksum

	SA	;***************
	lda	>$C70000,x	;Get byte
	LA	;***************
	and	#$00FF
	clc
	adc	<checksum
	sta	<checksum

	SA	;***************
	lda	>$C80000,x	;Get byte
	LA	;***************
	and	#$00FF
	clc
	adc	<checksum
	sta	<checksum

	SA	;***************
	lda	>$C90000,x	;Get byte
	LA	;***************
	and	#$00FF
	clc
	adc	<checksum
	sta	<checksum

	SA	;***************
	lda	>$CA0000,x	;Get byte
	LA	;***************
	and	#$00FF
	clc
	adc	<checksum
	sta	<checksum

	SA	;***************
	lda	>$CB0000,x	;Get byte
	LA	;***************
	and	#$00FF
	clc
	adc	<checksum
	sta	<checksum

	SA	;***************
	lda	>$CC0000,x	;Get byte
	LA	;***************
	and	#$00FF
	clc
	adc	<checksum
	sta	<checksum

	SA	;***************
	lda	>$CD0000,x	;Get byte
	LA	;***************
	and	#$00FF
	clc
	adc	<checksum
	sta	<checksum

	SA	;***************
	lda	>$CE0000,x	;Get byte
	LA	;***************
	and	#$00FF
	clc
	adc	<checksum
	sta	<checksum

	SA	;***************
	lda	>$CF0000,x	;Get byte
	LA	;***************
	and	#$00FF
	clc
	adc	<checksum
	sta	<checksum

	inx
	beq	.done
	jmp	.checkbank

.done	pld

	rtl


;====================================================================


	IF	ID_VERSION	;jgt;
;--------------------------------------------------------------------
; void FastMap (ushort basesrc)
;

	public	~~FastMap

~~ramscalers	equ	$7F4300	;See RWALL.ASM!!
bigpic	equ	~~ramscalers

	LCLSTART
	LCLW	map
	LCLW	count
	LCLW	ofs1
	LCLW	ofs2
	LCLW	ofs3
	LCLW	ofs4

	ARGSTART
	ARGW	basesrc

~~FastMap:	PROLOGUE

	phb

	lda	~~unwound
	sta	<map
	lda	~~unwoundcount
	lsr	a
	lsr	a
	sta	<count

	pea	$7F7F
	plb
	plb
	ldx	<map

	clc

.loop	ldy	<basesrc
	tya
	adc	!$7F0004,x	;Source address
	sta	<ofs2
	tya
	adc	!$7F0008,x	;Source address
	sta	<ofs3
	tya
	adc	!$7F000C,x	;Source address
	sta	<ofs4
	tya
	adc	!$7F0000,x	;Source address
*	sta	<ofs1
	tay

	SA	;***************

*	ldy	<ofs1
	lda	!bigpic,y
	ldy	!$7F0002,x	;Dest address
	sta	!~~screenbuffer+16*80,y

	ldy	<ofs2
	lda	!bigpic,y
	ldy	!$7F0006,x	;Dest address
	sta	!~~screenbuffer+16*80,y

	ldy	<ofs3
	lda	!bigpic,y
	ldy	!$7F000A,x	;Dest address
	sta	!~~screenbuffer+16*80,y

	ldy	<ofs4
	lda	!bigpic,y
	ldy	!$7F000E,x	;Dest address
	sta	!~~screenbuffer+16*80,y

	LA	;***************

	txa
	adc	#16
	tax

	dec	<count
	bne	.loop

	plb

	EPILOGUE		;RTL!
	ENDIF	;jgt;


;--------------------------------------------------------------------
; void IO_NoteOn (ushort channel, ushort instrument, ushort note
;	  ushort velocity)


DRV_ASSIGN	equ  4
DRV_MUSICON	equ  6

	public	~~IO_NoteOn

	LCLSTART
	LCLW	sfx
	LCLW	rate

	ARGSTART
	ARGW	channel
	ARGW	instrument
	ARGW	note
	ARGW	velocity

~~IO_NoteOn:	PROLOGUE

	php
	sei

	lda	<instrument
	cmp	#128+35
	beq	bassdrum
	cmp	#128+36
	beq	bassdrum
	cmp	#128+38
	beq	snaredrum
	cmp	#128+40
	beq	snaredrum
	cmp	#128+42
	beq	hihat
	cmp	#128+44
	beq	hihat
	cmp	#128+46
	beq	hihat

	cmp	#17
	beq	organ
	cmp	#68
	beq	oboe
notoboe	cmp	#75
	bne	notpanflute
	jmp	panflute
notpanflute	cmp	#57
	bne	nottrombone
	jmp	trombone
nottrombone	cmp	#36
	bne	notslapbass
	jmp	slapbass

notslapbass	jmp	done		;jgt;

bassdrum:	stz	<sfx
	lda	#$e21
	sta	<rate
	ldx	<velocity
	lda	>~~veltable1,x
	sta	<velocity
	jmp	playit

snaredrum:	lda	#1
	sta	<sfx
	lda	#$52c2
	sta	<rate
	ldx	<velocity
	lda	>~~veltable2,x
	sta	<velocity
	jmp	playit

hihat:	lda	#2
	sta	<sfx
	lda	#$5620
	sta	<rate
	ldx	<velocity
	lda	>~~veltable3,x
	sta	<velocity
	jmp	playit

organ:	lda	#3
	sta	<sfx
	lda	<note
	asl	a
	tax
	lda	>~~notetable1,x
	sta	<rate
	ldx	<velocity
	lda	>~~veltable4,x
	sta	<velocity
	jmp	playit

oboe:	lda	#4
	sta	<sfx
	lda	<note
	asl	a
	tax
	lda	>~~notetable2,x
	sta	<rate
	ldx	<velocity
	lda	>~~veltable5,x
	sta	<velocity
	jmp	playit

panflute:	lda	#5
	sta	<sfx
	lda	<note
	asl	a
	tax
	lda	>~~notetable3,x
	sta	<rate
	ldx	<velocity
	lda	>~~veltable6,x
	sta	<velocity
	jmp	playit

trombone:	lda	#6
	sta	<sfx
	lda	<note
	asl	a
	tax
	lda	>~~notetable4,x
	sta	<rate
	ldx	<velocity
	lda	>~~veltable7,x
	sta	<velocity
	jmp	playit

slapbass:	lda	#7
	sta	<sfx
	lda	<note
	asl	a
	tax
	lda	>~~notetable5,x
	sta	<rate
	ldx	<velocity
	lda	>~~veltable8,x
	sta	<velocity

; Assign the instrument

playit:	jsl	~~SndWait
	SA	;***************
	lda	<channel
	sta	>SPC700Port0
	lda	<sfx
	sta	>SPC700Port1
	LA	;***************
	pea	DRV_ASSIGN
	jsl	~~SndCmd

; Start the note

	jsl	~~SndWait

	lda	<channel
	asl	a
	tax
	lda	<rate
	sta	~~currate,x	;Rate for pitch bends
	sta	>SPC700Port0

	SA	;***************
	lda	<velocity
	sta	>SPC700Port2
	LA	;***************

	lda	<channel
	asl	a
	asl	a
	asl	a
	asl	a
	ora	#DRV_MUSICON
	pha
	jsl	~~SndCmd	;SndCmd( (channel<<4) | DRV_MUSICON)

done:	plp

	EPILOGUE		;RTL!


;====================================================================

	public	~~BlitPlay

~~BlitPlay:	phb			;Save C data bank
	phd			;Save C direct page
	pea	0
	plb
	plb			;Set bank for hardware regs
	pea	$4300		;Save a few cycles with direct page
	pld

	SI	;***************

	xref	~~bigpic

	lda	#~~screenbuffer
	sta	DMA0ADDR
	ldx	#^~~screenbuffer
	stx	DMA0BANK
	ldx	#0
	stx	DMA0PARMS	;Transfer bytes, incrementing
	ldx	#VMDATAH	;Transfer to high byte of VRAM
	stx	DMA0PORT

	ldx	#$80+12
	stx	VMAINC		;Increment by 8 for 128 times

	ldx	#80		;Always transfer 80 bytes
	ldy	#1		; enable DMA channel 0

;---------------------------------------
; DMA 112 columns of 80 bytes
;---------------------------------------

.column	set	0

	REPT	112

	stx	<DMA0LEN-$4300
	lda	#.column*128+8
	sta	VMADD
	sty	MDMAEN
.column	set	.column+1

	ENDREPT

;---------------------------------------
; Release forced blank
;---------------------------------------

	jsl	~~UnBlankVideo	;Returns in LAI mode!
	lda	>~~introhack
	bne	.done

; Clear the play screen to background colors

	SI	;***************

	ldx	#^~~playback
	stx	<DMA0BANK-$4300

	lda	#~~playback	;A few blank columns for the floor / ceiling
	sta	<DMA0ADDR-$4300

	ldx	#WMDATA		;Transfer to work ram
	stx	DMA0PORT

	lda	#~~screenbuffer	;Copy to ~~screenbuffer in work ram
	sta	WMADD

	lda	#112*80		;Copy entire screen
	sta	<DMA0LEN-$4300

	ldy	#1		;Enable dma channel 0
	sty	MDMAEN

	LI	;***************

.done	pld			;Restore C direct page
	plb			;Restore C data bank

	rtl


;==============================================================================
	public	~~SoundDownload

	LCLSTART

	ARGSTART
	ARGD	src_p
	ARGW	count

~~SoundDownload:
	PROLOGUE

	ldx	<count
	ldy	#0

	SA	;***************

; Download three bytes

.loop	php
	sei

; Make sure the sound CPU is ready

.wait	lda	>SPC700Port3
	cmp	~~lastcmd
	bne	.wait

	LA	;***************
	lda	[src_p],y
	sta	>SPC700Port0
	iny
	iny
	SA	;***************
	lda	[src_p],y
	sta	>SPC700Port2
	iny

	lda	~~driverflag	;Toggle bit
	ora	#3		;Download command
	sta	>SPC700Port3
	sta	~~lastcmd	;For receipt checking
	lda	~~driverflag
	eor	#$80
	sta	~~driverflag

	plp

	dex
	bne	.loop

	LA	;***************
	EPILOGUE		;RTL!


;====================================================================
; This could be considerably faster with unwinding

	public	~~Decompress

	LCLSTART
	LCLW	escapebyte
	LCLW	endout
	LCLW	outlength
	LCLW	matchbits
	LCLW	match
	LCLW	offset
	LCLW	matchmask
	LCLW	xtemp
	LCLW	ytemp

	ARGSTART
	ARGD	src_p
	ARGD	dest_p

~~Decompress:	PROLOGUE

	phb			;Save data bank

;    outlength = *((unsigned *)src_p)++;
;    escapebyte = *src_p++;
;    matchbits = *src_p++;
;    matchmask = (1<<matchbits)-1;

	lda	[src_p]
	sta	<outlength
	clc
	adc	<dest_p
	sta	<endout
	ldy	#3
	lda	[src_p],y
	and	#$FF
	sta	<matchbits
	asl	a
	tax
	lda	>bittable,x
	sta	<matchmask

	SA	;***************

	ldy	#2
	lda	[src_p],y
	sta	<escapebyte

; Decompress

	lda	<dest_p+2
	pha
	plb			;Data bank to destination
	ldy	#4		;Y is source offset
	ldx	<dest_p		;X is dest offset

;    if (*src_p == escapebyte)

decompress:	lda	[src_p],y
	iny
	cmp	<escapebyte
	beq	escapecode

;    *dest_p++ = *src_p++;	// Not compressed

storebyte:	sta	!0,x
	inx
	cpx	<endout
	bne	decompress
	LA	;***************
	plb

	EPILOGUE		;RTL!

	LONGA	OFF	;***************

;    if (!*src_p) {	// Wasted a byte signaling the escape char
;       *dest_p++ = escapebyte;
;       src_p++;
;    }

escapecode:	lda	[src_p],y
	bne	repeatrun
	iny
	lda	<escapebyte
	jmp	storebyte

;    else {		// compressed string
;       compword = *((unsigned *)src_p)++;
;       match = (compword<and>matchmask)+3;
;       offset = compword >> matchbits;
;       memcpy (dest_p, dest_p-offset, match);
;       dest_p += match;
;    }

repeatrun:	and	<matchmask
	clc
	adc	#3
	sta	<match

	LA	;***************

	lda	[src_p],y
	iny
	iny
	stx	<xtemp
	sty	<ytemp
	ldx	<matchbits
shiftloop	lsr	a
	dex
	bne	shiftloop
	sta	<offset
	ldx	<xtemp
	txa
	sec
	sbc	<offset
	tay			;Y is source for repeat

	SA	;***************

repeatloop	lda	!0,y
	sta	!0,x
	iny
	inx
	dec	<match
	bne	repeatloop

	ldy	<ytemp
	cpx	<endout
	bne	decompress

	LA	;***************

	plb

	EPILOGUE		;RTL!

;ID has this data in the CODE segment!

bittable:	dw	0,1,3,7,15,31,63,127,255


	IF	ID_VERSION	;jgt;
;====================================================================
; Input: A,X	Output: A

~UMUL2:	cmp	#$100
	bge	fullmul
	cpx	#$100
	bge	fullmul

	php

	SA	;***************

	sei

	sta	>MULTA
	txa
	sta	>MULTB
	nop
	nop
	nop
	nop
	nop

	LA	;***************

	lda	>PRODC
	plp
	rtl

fullmul:	jml	~~~MUL2
	ENDIF			;jgt;


;====================================================================
; Sets forced blank during a horizontal retrace

~~BlankVideo:	phb
	pea	0
	plb
	plb

	SAI	;***************

	lda	>~~fadelevel	;Shadow for INIDISP
	ora	#$80		;Set forced black
	tax
waitnohbl	lda	HVBJOY
	and	#$40		;Test for hblank
	bne	waitnohbl
waithbl	lda	HVBJOY
	and	#$40		;Test for hblank
	beq	waithbl
	stx	INIDISP		;Forced blank

	LAI	;***************

	plb
	rtl


;====================================================================
; Clears forced blank during a horizontal retrace

~~UnBlankVideo:	phb
	pea	0
	plb
	plb

	SAI	;***************

	lda	>~~fadelevel	;Shadow for INIDISP
	and	#$7F		;Mask off forced blank
	tax
	lda	#$40
.waitnohbl	bit	HVBJOY
	bne	.waitnohbl	;Wait for no hblank
.waithbl	bit	HVBJOY
	beq	.waithbl	;Wait for hblank
	stx	INIDISP		;Release forced blank

	LAI	;***************

	plb
	rtl


	IF	ID_VERSION	;jgt;
;====================================================================
; Signed 16-bit by unsigned 8-bit

	LCLSTART

	ARGSTART
	ARGW	v1
	ARGW	v2

Mul16S8U:	PROLOGUE

	lda	<v1
	php
	bpl	positive
	eor	#-1
	inc	a
	sta	<v1

positive:	SA	;***************;*OPTIMIZE: Do this better

	lda	<v2
	sta	>MULTA
	lda	<v1+1
	sta	>MULTB
	nop
	nop
	nop
	nop
	nop
	nop
	lda	>PRODC
	xba			;Low byte is not significant
	lda	<v1
	sta	>MULTB

	LA	;***************

	and	#$FF00
	clc
	nop
	nop
	nop
	adc	>PRODC
	plp
	bpl	positiveans
	eor	#-1
	inc	a

positiveans:	EPILOGUE		;RTL!

	
;====================================================================

	LCLSTART

	ARGSTART
	ARGW	v1
	ARGW	v2

SUFixedDiv:	PROLOGUE

	ldx	#8		; right shifts for answer

	lda	<v1
	bne	nonzero

	EPILOGUE		;RTL!

nonzero:	bpl	checknum
	eor	#-1
	inc	a
checknum	cmp	#$8000
	bge	numgood
	dex
	asl	a
	jmp	checknum

numgood	sta	>WRDIVD

	lda	<v2
checkden	cmp	#$100
	blt	dengood
	lsr	a
	dex
	jmp	checkden

dengood	SA	;***************
	sta	>WRDIVB
	LA	;***************

	pha
	pla
	pha
	pla
	pha
	pla

	lda	>RDDIVL

	cpx	#0
	beq	.done
	bmi	.negshift
.posshift	asl	a
	dex
	bne	.posshift
	jmp	.done

.negshift	lsr	a
	inx
	bne	.negshift

.done	ldx	<v1
	bpl	alldone
	eor	#-1
	inc	a

alldone	EPILOGUE		;RTL!

	ENDIF			;jgt;


;====================================================================
; Unsigned 16-bit by unsigned 8-bit

	public	~~UDiv16by8

	LCLSTART

	ARGSTART
	ARGW	v1
	ARGW	v2

~~UDiv16by8:	PROLOGUE

	lda	<v1
	sta	>WRDIVD

	SA	;***************

	lda	<v2
	sta	>WRDIVB

	LA	;***************

	pha
	pla
	pha
	pla
	pha
	pla

	lda	>RDDIVL

	EPILOGUE		;RTL!


;====================================================================
; Signed 16-bit by unsigned 8-bit

	public	~~Div16by8

	LCLSTART

	ARGSTART
	ARGW	v1
	ARGW	v2

~~Div16by8:	PROLOGUE

	lda	<v1
	bpl	notneg
	eor	#-1
	inc	a
notneg:	sta	>WRDIVD

	SA	;***************

	lda	<v2
	sta	>WRDIVB

	LA	;***************

	pha
	pla
	pha
	pla
	pha
	pla
	pha
	pla

	lda	>RDDIVL
	bit	<v1
	bpl	notneg2
	eor	#-1
	inc	a

notneg2	EPILOGUE		;RTL!


;====================================================================
; Merges src1/size1 and src2/size2 to dest
; The pointers must be updated before exiting

	extern	~~src1
	extern	~~size1
	extern	~~src2
	extern	~~size2
	extern	~~dest

	public	~~Merge

	LCLSTART
	LCLW	lsrc1
	LCLW	lsrc2
	LCLW	ldest

	ARGSTART		;NO ARGUMENTS!

~~Merge:	PROLOGUE

	lda	~~src1
	sta	<lsrc1
	lda	~~src2
	sta	<lsrc2
	lda	~~dest
	sta	<ldest

	ldy	#0		;Y is used for dest stepping

	lda	(lsrc1)
	cmp	(lsrc2)
	bge	mergefrom2	;if (*src1 > *src2)
	
; A always holds (l~~src1) here

mergefrom1:	inc	<lsrc1
	inc	<lsrc1
	sta	(ldest),y
	iny
	iny			;*dest++ = *src1++;

	dec	~~size1
	beq	finishfrom2

	lda	(lsrc1)
	cmp	(lsrc2)
	blt	mergefrom1	;if (*src1 > *src2)
	
mergefrom2:	lda	(lsrc2)
	inc	<lsrc2
	inc	<lsrc2
	sta	(ldest),y
	iny
	iny			;*dest++ = *src2++;

	dec	~~size2
	beq	finishfrom1

	lda	(lsrc1)
	cmp	(lsrc2)
	blt	mergefrom1	;if (*src1 > *src2)

	jmp	mergefrom2

finishfrom2:	lda	(lsrc2)
	inc	<lsrc2
	inc	<lsrc2
	sta	(ldest),y
	iny
	iny			;*dest++ = *src2++;
	dec	~~size2
	bne	finishfrom2
	jmp	finished

finishfrom1:	lda	(lsrc1)
	inc	<lsrc1
	inc	<lsrc1
	sta	(ldest),y
	iny
	iny			;*dest++ = *src1++;
	dec	~~size1
	bne	finishfrom1

finished:	lda	<lsrc1
	sta	~~src1
	lda	<lsrc2
	sta	~~src2
	tya
	clc
	adc	<ldest
	sta	~~dest

	EPILOGUE


;====================================================================

	public	~~R_TransformZ

	public	~~viewsin
	public	~~absviewsin
	public	~~viewcos
	public	~~absviewcos

	extern	~~trx
	extern	~~try

;---------------------------------------

	public	~~transdp

	page0		;Use zero-page for local variables
~~transdp	equ	$0000	;Use DP = $0000 to access zero-page

~~viewsin	dw	0
~~absviewsin	dw	0
~~viewcos	dw	0
~~absviewcos	dw	0

abstrx	dw	0
abstry	dw	0
gxt	dw	0
gyt	dw	0

	ends

;---------------------------------------


~~R_TransformZ:	phd			;Save direct page
	lda	#~~transdp	;Use private dpage
	tcd

	phb			;Save data bank
	pea	$0
	plb			;Set data bank for hardware
	plb

;---------------------------------------
; Get absolute values for ~~trx/~~try
;---------------------------------------

	lda	>~~trx
	bpl	gotabstrx
	eor	#-1
	inc	a
gotabstrx	sta	abstrx

	lda	>~~try
	bpl	gotabstry
	eor	#-1
	inc	a
gotabstry	sta	<abstry

	SI	;***************

;---------------------------------------
;   gxt = ~~trx * ~~viewcos
;---------------------------------------

	ldx	<~~absviewcos
	stx	MULTA
	ldy	<abstrx
	sty	MULTB		;Start first multiply
	ldy	<abstrx+1
	pha
	pla
	lda	PRODC		;Get first answer
	sty	MULTB		;Start second multiply
	xba
	and	#$FF		;Only low byte significant
	clc
	nop
	nop
	nop
	adc	PRODC		;Get second answer
	sta	<gxt

	lda	>~~trx		;Calculate sign
	eor	<~~viewcos
	bpl	.gxtpos
	lda	<gxt
	eor	#-1
	inc	a
	sta	<gxt

;---------------------------------------
;   gyt = ~~try * ~~viewsin;
;---------------------------------------

.gxtpos	ldx	<~~absviewsin
	stx	MULTA
	ldy	<abstry
	sty	MULTB		;Start first multiply
	ldy	<abstry+1
	pha
	pla
	lda	PRODC		;Get first answer
	sty	MULTB		;Start second multiply
	xba
	and	#$FF		;Only low byte significant
	clc
	nop
	nop
	nop
	adc	PRODC		;Get second answer
	sta	<gyt

	lda	>~~try		;Calculate sign
	eor	<~~viewsin
	bpl	.gytpos
	lda	<gyt
	eor	#-1
	inc	a
	sta	<gyt

;---------------------------------------
;   return gxt + gyt;
;---------------------------------------

.gytpos	LI	;***************

	lda	<gxt
	clc
	adc	<gyt

	plb
	pld

	rtl


;====================================================================
; R_TransformZ MUST have been called first
;	gxt = FixedByFrac(~~trx,~~viewsin);
;	gyt = FixedByFrac(~~try,~~viewcos);
;	return  gxt - gyt;

	public	~~R_TransformX

~~R_TransformX:	phd			;Save direct page
	lda	#~~transdp	;Use private dpage
	tcd

	phb			;Save data bank
	pea	$0
	plb			;Set data bank for hardware
	plb

	SI	;***************

;---------------------------------------
; gxt = ~~trx * ~~viewsin
;---------------------------------------

	ldx	<~~absviewsin
	stx	MULTA
	ldy	<abstrx
	sty	MULTB		;Start first multiply
	ldy	<abstrx+1
	pha
	pla
	lda	PRODC		;Get first answer
	sty	MULTB		;Start second multiply
	xba
	and	#$FF		;Only low byte significant
	clc
	nop
	nop
	nop
	adc	PRODC		;Get second answer
	sta	<gxt

	lda	>~~trx		;Calculate sign
	eor	<~~viewsin
	bpl	.gxtpos
	lda	<gxt
	eor	#-1
	inc	a
	sta	<gxt

;---------------------------------------
; gyt = ~~try * ~~viewcos
;---------------------------------------

.gxtpos	ldx	<~~absviewcos
	stx	MULTA
	ldy	<abstry
	sty	MULTB		;Start first multiply
	ldy	<abstry+1
	pha
	pla
	lda	PRODC		;Get first answer
	sty	MULTB		;Start second multiply
	xba
	and	#$FF		;Only low byte significant
	clc
	nop
	nop
	nop
	adc	PRODC		;Get second answer
	sta	<gyt

	lda	>~~try		;Calculate sign
	eor	<~~viewcos
	bpl	.gytpos
	lda	<gyt
	eor	#-1
	inc	a
	sta	<gyt

;---------------------------------------
; return gxt - gyt
;---------------------------------------

.gytpos	LI	;***************

	lda	<gxt
	sec
	sbc	<gyt

	plb
	pld

	rtl



;====================================================================
; Fill memory with a constant

	IF	0

w_memset	
	DUM	1	;Local record
RTLVal	BLOCK	3	;Return address
dest	LONG		;Pointer to memory
fill	WORD		;Byte to fill with
size	WORD		;Number of bytes to fill
EndDirect DSectSize	;Return here
	DEND

			TSC
	PHD		;Make my stack frame
	TCD
	SEP	#$20
	LDA	fill	;Get the fill char
	LDY	size	;No bytes to fill?
	BEQ	Exit	;Exit now
	DEY		;-1
	BEQ	EndLoop	;Only one
Loop	STA	[dest],Y	;Fill memory
	DEY		;Count down
	BNE	Loop
EndLoop	STA	[dest]	;Save the last byte
Exit	REP	#$21
	PLD
	LDA	#RTLVal+1,S	;Get the return address
	STA	#RTLVal+8+1,S
	PLA
	STA	#RTLVal+8-2,S
	TSC
	CLC
	ADC	#8-2
	TCS
	RTL		;Exit with new value..
	END

w_memcpy	
	DUM	1	;Local record
RTLVal	BLOCK	3	;Return address
dest	LONG		;Pointer to memory
source	LONG		;Byte to fill with
size	WORD		;Number of bytes to fill
EndDirect DSectSize	;Return here
	DEND

			TSC
	PHD		;Make my stack frame
	TCD
	SEP	#$20
	LDY	size	;No bytes to fill?
	BEQ	Exit	;Exit now
	DEY		;-1
	BEQ	EndLoop	;Only one
Loop	LDA	[source],Y
	STA	[dest],Y	;Fill memory
	DEY		;Count down
	BNE	Loop
EndLoop  LDA	[source]
	STA	[dest]	;Save the last byte
Exit	REP	#$21
	PLD
	LDA	#RTLVal+1,S
	STA	#RTLVal+1+10,S
	PLA
	STA	#RTLVal+10-2,S	;Remove the excess
	TSC
	ADC	#10-2
	TCS
	RTL		;Exit with new value..
	END

w_rnd	
	LDA	#1
	RTL
	END

w_abs	
	DUM	1	;Local record
RTLVal	BLOCK	3	;Return address
Value	WORD		;Value to convert
EndDirect DSectSize	;Return here
	DEND

	LDA	Value,S	;Get value to test
	BPL	Use
	EOR	#-1	;Negate it
	INC	A
Use	TAX		;Save answer in X
	LDA	#RTLVal+1,S	;Only one word to remove
	STA	#RTLVal+3,S
	PLA
	STA	#RTLVal,S	;Remove the excess
	TXA
	RTL

	end

	ENDIF


;====================================================================
; fixed_t FixedByFrac (fixed_t Val1, fixed_t Val2)
; * Mul a signed fixed by a frac *
;

	public	~~FixedByFrac

	LCLSTART

	ARGSTART
	ARGW	Val1		;Value to convert
	ARGW	Val2		;Value to convert

~~FixedByFrac:	PROLOGUE

	phb			;Save data bank
	pea	$0000		;Set bank to hardware
	plb
	plb
	lda	<Val1		;Get the sign of the mul
	eor	<Val2
	php			;Save the sign
	lda	<Val1		;Absolute value of #1
	bpl	.Val1Ok
	eor	#-1
	inc	a
	sta	<Val1
.Val1Ok	tay
	lda	<Val2		;Absolute value of #2
	bpl	.Val2Ok
	eor	#-1
	inc	a
	sta	<Val2

;---------------------------------------
; Mul by Val1+0
;---------------------------------------

.Val2Ok	tax			;X = Val2
	SI	;***************
	stx	MULTA		;Val1 * Val2
	sty	MULTB		;Save the mul value
	ldy	<Val1+1		;Get Val2+1
	pha
	pla
	lda	PRODC+1		;Get the answer
	sty	MULTB		;Val1+1 * Val2+1
	and	#$FF		;Mask for byte
	clc
	pha
	pla
	adc	PRODC
	ldx	#0
	plp			;Restore MX and sign
	LI	;***************
	bpl	.NoUnNeg
	eor	#-1		;Negate it
	inc	a
	dex
.NoUnNeg	tay			;Use index
	plb			;Restore data bank
	tya

	EPILOGUE		;RTL!


;====================================================================
; fixed_t FixedMul (fixed_t Val1, fixed_t Val2)
; * Mul a signed fixed by a signed fixed *
;

	public	~~FixedMul

	LCLSTART
	LCLD	Answer

	ARGSTART
	ARGW	Val1		;Value to convert
	ARGW	Val2		;Value to convert

~~FixedMul:	PROLOGUE

	phb			;Save data bank
	pea	$0000		;Set bank to hardware
	plb
	plb
	lda	<Val1		;Get the sign of the mul
	eor	<Val2
	php			;Save the sign
	lda	<Val1		;Absolute value of #1
	bpl	.Val1Ok
	eor	#-1
	inc	a
	sta	<Val1

.Val1Ok	lda	<Val2		;Absolute value of #2
	bpl	.Val2Ok
	eor	#-1
	inc	a
	sta	<Val2

;---------------------------------------
; Mul by Val1+0
;---------------------------------------

.Val2Ok	tax			;X = Val2
	SI	;***************
*	ldx	<Val2		;Zero?
	stx	MULTA		;Val1 * Val2
	ldy	<Val1
	sty	MULTB		;Save the mul value
	ldx	<Val2+1		;Get High * High
	ldy	<Val1+1
	pha
	pla
	lda	PRODC		;Get the answer
	stx	MULTA		;Start the next mul
	sty	MULTB		;Val1+1 * Val2+1
	sta	<Answer		;Save low word of the answer

	ldx	<Val1		;Low byte
	clc
	pha
	pla
	lda	PRODC		;Get the answer
	stx	MULTB		;Val2+1 * Val1
	sta	<Answer+2	;Save as high word

	ldx	<Val2
	pha
	pla
	ldx	PRODC
	stx	MULTA
	sty	MULTB		;Val2 * Val+1
	pha
	pla
	adc	<Answer+1	;Add into answer
*	bcc	.NoInc
*	inc	<Answer+3	;Inc high byte
	clc			;Reset carry
.NoInc	pha
	pla
	adc	PRODC		;Get answer again
*	sta	<Answer+1	;Save result
*	bcc	.NoInc2
*	inc	<Answer+3
.NoInc2	ldx	#0
	pla			;Restore MX and sign
	LI	;***************
	bpl	.NoUnNeg
	eor	#-1		;Negate it
	inc	a		;Inc
	dex
.NoUnNeg	plb			;Restore the bank

	EPILOGUE		;RTL!


;====================================================================
; fixed_t SUFixedMul (fixed_t Val1, fixed_t Val2)
; * Signed * an unsigned value *
;

	public	~~SUFixedMul

	LCLSTART
	LCLD	Answer

	ARGSTART
	ARGW	Val1		;Value to convert
	ARGW	Val2		;Value to convert

~~SUFixedMul:	PROLOGUE

	phb			;Save data bank
	pea	$0000		;Set bank to hardware
	plb
	plb
	lda	<Val1		;Get the sign of the mul
	php
	bpl	.Val1Ok
	eor	#-1
	inc	a
	sta	<Val1

;---------------------------------------
; Mul by Val1+0
;---------------------------------------

.Val1Ok	SI	;***************
	ldx	<Val2		;Zero?
	stx	MULTA		;Val1 * Val2
	ldy	<Val1
	sty	MULTB		;Save the mul value
	ldx	<Val2+1		;Get High * High
	ldy	<Val1+1
	pha
	pla
	lda	PRODC		;Get the answer
	stx	MULTA		;Start the next mul
	sty	MULTB		;Val1+1 * Val2+1
	sta	<Answer		;Save low word of the answer

	ldx	<Val1		;Low byte
	clc
	pha
	pla
	lda	PRODC		;Get the answer
	stx	MULTB		;Val2+1 * Val1
	sta	<Answer+2	;Save as high word

	ldx	<Val2
	pha
	pla
	lda	PRODC
	stx	MULTA
	sty	MULTB		;Val2 * Val+1
	pha
	pla
	adc	<Answer+1	;Add into answer
*	bcc	.NoInc
*	inc	<Answer+3	;Inc high byte
	clc			;Reset carry
.NoInc	pha
	pla
	adc	PRODC		;Get answer again
*	sta	<Answer+1	;Save result
*	bcc	.NoInc2
*	inc	<Answer+3
.NoInc2	ldx	#0
	LI	;***************
	plp			;Restore MX and sign
	bpl	.NoUnNeg
	eor	#-1		;Negate it
	inc	a		;Inc
	dex
.NoUnNeg	plb			;Restore data bank

	EPILOGUE		;RTL!


;====================================================================
; ufixed_t UFixedMul (ufixed_t Val1, ufixed_t Val2)
; * Mul an unsigned fixed by an unsigned fixed *
;

	public	~~UFixedMul

	LCLSTART
	LCLD	Answer

	ARGSTART
	ARGW	Val1		;Value to convert
	ARGW	Val2		;Value to convert

~~UFixedMul:	PROLOGUE

	phb			;Save data bank
	pea	$0000		;Set bank to hardware
	plb
	plb

;---------------------------------------
; Mul by Val1+0
;---------------------------------------

	SI	;***************
	ldx	<Val2		;Zero?
	stx	MULTA		;Val1 * Val2
	ldy	<Val1
	sty	MULTB		;Save the mul value
	ldx	<Val2+1		;Get High * High
	ldy	<Val1+1
	pha
	pla
	lda	PRODC		;Get the answer
	stx	MULTA		;Start the next mul
	sty	MULTB		;Val1+1 * Val2+1
	sta	<Answer		;Save low word of the answer

	ldx	<Val1		;Low byte
	clc
	pha
	pla
	lda	PRODC		;Get the answer
	stx	MULTB		;Val2+1 * Val1
	sta	<Answer+2	;Save as high word

	ldx	<Val2
	pha
	pla
	lda	PRODC
	stx	MULTA
	sty	MULTB		;Val2 * Val+1
	pha
	pla
	adc	<Answer+1	;Add into answer
*	bcc	.NoInc
*	inc	<Answer+3	;Inc high byte
	clc			;Reset carry
.NoInc	pha
	pla
	adc	PRODC		;Get answer again
*	sta	<Answer+1	;Save result
*	bcc	.NoInc2
*	inc	<Answer+3
.NoInc2	LONGI	ON	;*******
	rep	#$11
	plb			;Restore data bank

	ldx	#0		;Clear high byte of answer (X:A)

	EPILOGUE		;RTL!

;--------------------------------------------------------------------

	end

