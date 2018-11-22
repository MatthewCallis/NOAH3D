*--------------------------------------------------------------------
* RWALL.ASM
*--------------------------------------------------------------------

; Locations $0000 - $F000 in this bank are loaded with the compiled scalers
; and related tables.  This code is linked at $01F000 so the tables can
; be accessed locally and the wall scalers can be localy called.  The
; masked scalers are copied from this bank to RAM on startup so they can
; be patched to scale partial columns.  The offsets in the scaler tables
; are for the destination location.

	spaces	on
	llchar	.

	include	snes.equ
	include idmacros.asm

primitive	section	;offset	$01F000	;Note tables & drawing primitives


;====================================================================
; INCLUDE NOTE & VELOCITY TABLES here because there's extra space in
; this bank and because that's what ID did!
;====================================================================


	include	notes.asm
	globals	off

;====================================================================


;--------------------------------------------------------------------
; void SetupScalers(void)
;

	public	~~SetupScalers

~~SetupScalers:	phb

; Set RAM bank pointer to second bank.  This is the default value
; throught the game.  If anything (memset) changes it, it should
; be reset to bank 1 before exiting

	SA	;***************
	lda	#1
	sta	>WMBANK		;Writing into the second ram bank
	LA	;***************

; Copy masked scalers to RAM
;    *OPTIMIZE: use DMA

	phk
	plb			;Tables are in this bank

	ldx	maskedstart	;Source
	ldy	#~~ramscalers	;Destination
	lda	#$4FFF		;Bytes to copy
	mvn	^~~SetupScalers,^~~ramscalers
	
	plb
	rtl

;--------------------------------------------------------------------
; Table locations in scaler rom (bank $C1)

rwtables	equ	$C10000	;SCALER TABLES in bank $C1

	public	~~wallscalers,~~maskedinfoofs,~~maskedinfo

~~wallscalers	equ	$C10000	;word wallscalers[128]
~~maskedinfoofs	equ	$C10100	;word maskedinfoofs[128]
~~maskedinfo	equ	$C10200	;word maskedinfo[85][130] (pixel skip/entry point)

maskedstart	equ	22612	;$C15854 - WORD offset
maskedend	equ	22614	;$C15856 - WORD offset

; Second RAM bank:

	public	~~rambuffer
	public	~~screenbuffer
	public	~~ramscalers
	public	~~freeram

; See WOLFASM.ASM if ~~ramscalers is changed!

~~rambuffer	gequ	$7F0000
~~screenbuffer	gequ	$7F2000		;This could be moved down to 0
~~ramscalers	gequ	$7F4300		;  but I would need to reorg scalers
~~freeram	gequ	$7F9300


;-----------------------------------------------
; Scaler tables should return valid (possibly
; duplicated) data for scaler numbers 0 - 127
; *OPTIMIZE: page align
;-----------------------------------------------

scalerskip	dw	40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25
	dw	24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9
	dw	 8, 7, 6, 5, 4, 3, 2, 1

	REPT	88
	dw	0
	ENDREPT


;-----------------------------------------------
; Start of each vertical column in screenbuffer
;-----------------------------------------------

	public	~~columnstart

~~columnstart

.column	set	~~screenbuffer

	MACLIST	ON
	REPT	112
	dw	.column
.column	set	.column+80
	ENDREPT
	MACLIST	OFF


;--------------------------------------------------------------------
; void RenderWallLoop (void);
;

	public	~~RenderWallLoop

;---------------------------------------

	public	~~rw_direct

	page0		;Use zero-page for local variables
~~rw_direct	equ	$0000	;Use DP = $0000 to access zero-page

	public	~~rw_x,~~rw_stopx,~~rw_scale,~~rw_scalestep
	public	~~rw_tangent,~~rw_distance,~~rw_midpoint
	public	~~rw_texture,~~rw_mintex,~~rw_maxtex
	public	~~rw_centerangle,~~rw_downside


~~rw_x	dw	0	;First column to draw
~~rw_stopx	dw	0	;Stop before drawing this column
			; 	 (MUST be > ~~rw_x)
~~rw_scale	dw	0	;Fractional scale at ~~rw_x
~~rw_scalestep	dw	0	;Scale step for each column
~~rw_tangent	dl	0	;Long pointer to tangent table
~~rw_distance	dw	0	;Fractional cardinal distance to wall
~~rw_midpoint	dw	0	;Midpoint in texture
~~rw_texture	dl	0	;Text tile strip to map on wall
~~rw_mintex	dw	0	;Low column number for wall segment
~~rw_maxtex	dw	0	;High column number for wall segment
~~rw_centerangle  dw	0	;View center in finetan array
~~rw_downside	dw	0	;True for SOUTH and EAST

temp	dw	0
Answer	dl	0	;Multiply temp 16.16 frac.
tangvalue	dw	0	;Value for last column to track delta

	ends

;---------------------------------------

; External vars:
; xscale[SCREENWIDTH]	Store scale of each column for z clipping
; wcolumns[]		The actual wall pixels to scale

	extern	~~xtoview
	extern	~~finetan
	extern	~~xscale
	extern	~~wcolumns

;---------------------------------------

~~RenderWallLoop:

	phd
	phb			;Save C dpage and bank

	phk
	plb			;Look at tables in this code bank

	lda	#~~rw_direct
	tcd			;Set aligned direct page

	asl	~~rw_x
	asl	~~rw_stopx	;Preshift values

;---------------------------------------
; Set Answer to be the calculated texture
; for the first column the rest of the
; columns will be deltas off of this
;---------------------------------------

	ldx	<~~rw_x
	lda	>~~xtoview,x
	clc
	adc	<~~rw_centerangle
	asl	a
	tax
	lda	>~~finetan,x	;Tangent for column / orientation
	sta	<tangvalue
	jmp	RWSUFixedMul

;---------------------------------------
; *** RWSUFixedMul jumps back here ***
; If downside is set, subtract the negate
; the delta value
;---------------------------------------

gotfirsttexture:
	lda	<~~rw_downside
	beq	noflipstart
	lda	<~~rw_midpoint
	sec
	sbc	<Answer+1
	sta	<Answer+1
	jmp	prepmain

noflipstart:	lda	<Answer+1
	clc
	adc	<~~rw_midpoint
	sta	<Answer+1

prepmain:	lda	<~~rw_x

;---------------------------------------
; Store current scale for z clipping of
; sprites
;---------------------------------------

mainloop:	tax
	txy			;X will get changed, so keep Y set
	lda	<~~rw_scale
	sta	>~~xscale,x

;---------------------------------------
; Set up for call and return from
; compiled scaler strip off fractional
; bits from scale to get compiled scaler
; number
;---------------------------------------

	xba
	asl	a		;Shift for table lookup
	and	#$00FF
	tax			;Compiled scaler number
	pea	returnpoint-1	;Location to return to from scaler
	lda	|~~wallscalers,x;Lookup table at begining of bank
	pha			;Start of scaler to jump to

	lda	scalerskip,x	;Y offset from top of column
	clc
	adc	~~columnstart,y	;X offset from start of ~~screenbuffer
	sei			;Don't let ints happed while using
				;  the increment register
	sta	>WMADD		;Hardware auto increment register

;---------------------------------------
; Advance scale for next column
;---------------------------------------

	lda	<~~rw_scale
	adc	<~~rw_scalestep
	sta	<~~rw_scale

;---------------------------------------
; texturecollumn = 
;    SUFixedMul (tangent[x],fseg->distance)
;	 + midpoint;
; This is an 8.8 number, with a 0-255
; range / tile.  Each tile only has 32
; graphic columns in it, so there are 3
; bits of extra fractions
;---------------------------------------

	tyx
	lda	>~~xtoview,x	 ;OPTIMIZE: make ~~xtoview in bank

	clc
	adc	<~~rw_centerangle
	asl	a
	tax
	lda	>~~finetan,x	 ;Tangent for column / orientation
	tax
	sec
	sbc	<tangvalue	 ;Find delta from last calculated value
	beq	firsttexture	 ;First column already calculated
	stx	<tangvalue

;---------------------------------------
; If downside is set, negate delta value
;---------------------------------------

	ldx	<~~rw_downside
	beq	rightsign
	eor	#-1
	inc	a
	bpl	positivedelta
	jmp	negativedelta

rightsign	ora	#0		 ;Set sign bit
	bpl	positivedelta
	jmp	negativedelta

firsttexture	lda	<Answer+1

;---------------------------------------
; *** Both deltas jump back here ***
; Bound texturecolumn to min / max
; (A holds Answer+1)
;---------------------------------------

gotnewtexture:	cmp	<~~rw_mintex
	blt	boundmin
	cmp	<~~rw_maxtex
	bge	boundmax
bounded	tax

;---------------------------------------
; Find the tile picture in the texture
; that the column maps to
;---------------------------------------

	xba
	and	#$00FF		;Get high byte of texturecolumn
	tay
	lda	[~~rw_texture],y

;---------------------------------------
; Calculate the wall picture column to
; draw.  The wall pictures are all
; placed adjacent in memory, so picture
; columns 0-31 are tile 0, 32-63 are
; tile 1, etc.  The walls occupy one
; entire bank.
;---------------------------------------

	xba			;Move tile pic num into high byte
	SA	;***************
	txa			;Move tile frac into low byte
	and	#%11111000	;Unused frac bits

	pea	$0400+(^~~wcolumns)	;High byte is LONGA,I, no IRQs
	plb			;Bank for pictures
	plp			;Pull extra 0 byte, back to long IM
	LONGA	ON	;*******
	asl	a
	asl	a		;Now multiplied by 32
	tay			;Low two bytes of address

	lda	#$2100
	tcd			;Set DPAGE for scalers
	SA	;***************
	rts			;Jump to compiled scaler

;---------------------------------------
; On entry to compiled scalers:
;    B : bank of wall pictures
;    D : $2100 for access to WMDATA register
;    Y : start of wall picture
;    LONGA OFF, LONGI ON
;    WMADD set to first pixel to be drawn
;---------------------------------------

returnpoint:	rep	#$25	  	;Clear I,C,M
	LONGA	ON	;*******
	lda	#~~rw_direct
	tcd			;Restore local dpage
	phk
	plb			;Restore local bank

;---------------------------------------
; loop:	advance x
;---------------------------------------

	lda	<~~rw_x
	adc	#2
	sta	<~~rw_x
	cmp	<~~rw_stopx
	beq	done
	jmp	mainloop

done:	plb
	pld
	rtl

;---------------------------------------

boundmin	lda	<~~rw_mintex
	jmp	bounded
boundmax	lda	<~~rw_maxtex
	dec	a
	jmp	bounded

;---------------------------------------

positivedelta:	pea	$0
	plb
	plb			;Hardware bank

checkover	cmp	#$100	 	;Check for > one byte delta
	blt	over1
	sec
	sbc	#$100
	tax
	lda	<Answer+1
	clc
	adc	<~~rw_distance
	sta	<Answer+1
	txa
	jmp	checkover

over1:	SI	;***************

	tax
	stx	MULTA	 	;Increment will be multiplied twice
	ldy	<~~rw_distance
	sty	MULTB

	lda	<Answer
	clc
	ldy	<~~rw_distance+1
	pha
	pla

	adc	PRODC
	sty	MULTB	 	;Start second multiply

	sta	<Answer
	bcc	noinc
	inc	<Answer+2
	clc

noinc:	lda	<Answer+1
	LI	;***************
	phk
	plb

	adc	>PRODC
	sta	<Answer+1	;Overflow is ignored

	jmp	gotnewtexture

;---------------------------------------

negativedelta:	pea	$0
	plb
	plb			;Hardware bank

	eor	#-1
	inc	a		;Negate value
checkunder	cmp	#$100	 	;Check for > one byte delta
	blt	under1
	sec
	sbc	#$100
	tax
	lda	<Answer+1
	sec
	sbc	<~~rw_distance
	sta	<Answer+1
	txa
	jmp	checkunder

under1:	SI	;***************

	tax
	stx	MULTA	 	;Increment will be multiplied twice
	ldy	<~~rw_distance
	sty	MULTB

	lda	<Answer
	sec
	ldy	<~~rw_distance+1
	pha
	pla

	sbc	PRODC
	sty	MULTB	 	;Start second multiply

	sta	<Answer
	bcs	nodec
	dec	<Answer+2
	sec

nodec:	lda	<Answer+1
	LI	;***************
	phk
	plb
	sbc	>PRODC
	sta	<Answer+1	;Overflow is ignored

	jmp	gotnewtexture



;--------------------------------------------------------------------
; Signed * an unsigned value

RWSUFixedMul:	phb
	pea	$0000		;Set bank to hardware
	plb
	plb

	lda	<tangvalue	;Get the sign of the mul
	php
	bpl	tangvalueOk
	eor	#-1
	inc	a
tangvalueOk	sta	<temp		;don't mess up tangvalue

;---------------------------------------
; Mul by tangvalue+0
;---------------------------------------

	SI	;***************
	ldx	<~~rw_distance
	stx	MULTA		;tangvalue * ~~rw_distance
	ldy	<temp
	sty	MULTB		;Save the mul value

	ldx	<~~rw_distance+1	;Get High * High
	ldy	<temp+1
	pha
	pla

	lda	PRODC		;Get the Answer
	stx	MULTA		;Start the next mul
	sty	MULTB		;tangvalue+1 * ~~rw_distance+1

	sta	<Answer		;Save low word of the Answer
	ldx	<temp		;Low byte
	clc
	pha
	pla

	lda	PRODC		;Get the Answer
	stx	MULTB		;~~rw_distance+1 * tangvalue

	sta	<Answer+2	;Save as high word
	ldx	<~~rw_distance
	pha
	pla

	lda	PRODC
	stx	MULTA
	sty	MULTB		;~~rw_distance * Val+1

	adc	<Answer+1	;Add into Answer
	clc			;Reset carry
	pha
	pla
	pha
	pla

	adc	PRODC		;Get Answer again
	plp			;Restore MX and sign
	LONGI	ON	;*******
	bpl	NoUnNeg
	eor	#-1		;Negate it
	inc	a		;Inc

NoUnNeg:	sta	<Answer+1
	plb			;Restore bank
	jmp	gotfirsttexture

;--------------------------------------------------------------------

	ends
	end

