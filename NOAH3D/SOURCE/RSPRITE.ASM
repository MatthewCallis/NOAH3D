*--------------------------------------------------------------------
* RSPRITE.ASM
*--------------------------------------------------------------------

; Locations $0000 - $F000 in this bank are loaded with the compiled scalers
; and related tables.  This code is linked at $01F000 so the tables can
; be accessed locally and the wall scalers can be localy called.  The
; masked scalers are copied from this bank to ram on startup so they can
; be patched to scale partial columns.  The offsets in the scaler tables
; are for the destination location.

	spaces	on
	llchar	.

	include	snes.equ
	include idmacros.asm

primitive	section	;offset	$01F000	;Note tables & drawing primitives


;--------------------------------------------------------------------
; void RenderSprite(void);
;

	public	~~RenderSprite

;---------------------------------------

OP_RTL	equ	$6B		;For patching the RAM scalers
OP_LDAY	equ	$B9

	extern	~~spritedat
	extern	~~maskedinfoofs
	extern	~~xscale

	extern	vs_pos
	extern	vs_clipscale
	extern	vs_columnstep
	extern	vs_x1

;---------------------------------------

	public	~~rs_direct

	page0		;Use zero-page for local variables
~~rs_direct	equ	$0000	;Use DP = $0000 to access zero-page

	public	~~rs_x1,~~rs_x2,~~rs_vseg

~~rs_x1	dw	0
~~rs_x2	dw	0
~~rs_vseg	dl	0

column	dw	0		;Fractional column number
columnstep	dw	0
clipscale	dw	0

;--- Scale col. vars. ---

scalerskip	dl	0		;65 pixel skips
scalerentry	dl	0		;65 entry points
spritepos	dl	0
columnptr	dw	0		;Pointer into spritepos
screenofs	dw	0		;Top of screen column

	ends

;---------------------------------------

~~RenderSprite:	phd			;Save C vars
	phb

	lda	#~~rs_direct
	tcd			;Set dpage for parms and locals

;---------------------------------------
; Copy vars from struct pointer to dpage
;---------------------------------------

	ldx	<~~rs_vseg
	lda	|vs_pos,x
	sta	<spritepos
	lda	|vs_pos+2,x
	clc
	adc	#^~~spritedat
	sta	<spritepos+2	;Offset to sprite record
	lda	|vs_clipscale,x
	sta	<clipscale
	lda	|vs_columnstep,x
	asl	a		;Shift column by two to avoid shift in loop
	sta	<columnstep

;---------------------------------------
; Look up info for this masked scaler
;---------------------------------------

	lda	|vs_clipscale,x
	xba
	and	#$7F
	asl	a		;Clears carry
	tax
	lda	>~~maskedinfoofs,x ;Table in compiled scaler bank
	sta	<scalerskip
	adc	#65*2
	sta	<scalerentry
	lda	#^~~maskedinfoofs	;OPTIMIZE: if dpages get uniqued,
	sta	<scalerskip+2		;Init these once on startup
	sta	<scalerentry+2

;---------------------------------------
;   if (x1 > vseg->x1)
;      column += (x1-vseg->x1)*vseg->columnstep;
;---------------------------------------

	stz	<column
	lda	<~~rs_x1
	ldy	<~~rs_vseg
	sec
	sbc	|vs_x1,y	;This is now in a different data bank
	bcs	nocoladd
	SA	;***************
	sta	>MULTA
	lda	<columnstep
	sta	>MULTB
	LA	;***************
	pha
	pla
	nop
	lda	>PRODC
	sta	column
	SA	;***************
	lda	columnstep+1	;High byte
	sta	>MULTB
	LA	;***************
	pha
	pla
	nop
	lda	>PRODC
	xba			;Only low byte will by set
	clc
	adc	<column
	sta	<column

;---------------------------------------
; Calculate and draw each collumn:
;
; The bank is set to the sprite data
; for the duration...
;
;    for (x=x1; x <= x2; x++, column += vseg->columnstep)
;---------------------------------------

nocoladd:	pei	<spritepos+2	;Set data bank to sprite
	plb
	plp

	lda	<~~rs_x1

;---------------------------------------
;   if (~~xscale[x] > vseg->clipscale)
;      continue;	// Obscured by a wall
;---------------------------------------

columnloop:	asl	a
	tax
	lda	<clipscale
	cmp	>~~xscale,x
	bge	~~IO_ScaleMaskedColumn

;---------------------------------------
;   x++, column += vseg->columnstep)
;---------------------------------------

rscontinue:	lda	<columnstep
	clc
	adc	<column
	sta	<column
	inc	<~~rs_x1
	lda	<~~rs_x1
	cmp	<~~rs_x2
	blt	columnloop
	beq	columnloop

	plb			;Restore C data bank
	pld			; and direct page
	rtl


;--------------------------------------------------------------------
; IO_ScaleMaskedColumn
;

	extern	~~columnstart
	extern	~~ramscalers
	public	~~IO_ScaleMaskedColumn

~~IO_ScaleMaskedColumn:
;---------------------------------------
; Look up top of column in screen buffer
;---------------------------------------

	lda	>~~columnstart,x
	sta	<screenofs

;---------------------------------------
; Look up masked column to draw
;
;    texturecolumn = column>>FRACBITS;
;---------------------------------------

	lda	<column
	xba
	and	#126
	tay
	lda	(spritepos),y
	sta	<columnptr

;---------------------------------------
; Draw a post in the column
;---------------------------------------

drawpost	tax
	ldy	|0,x		;First word is y1 (preshifted by 1)
	bmi	rscontinue	;A -1 signals end of column

;---------------------------------------
; Set up stack for two far returns
;---------------------------------------

	phk			;Return to this program bank
	pea	returnpoint-1	;Push offset-1 to return to
	SA	;***************
	lda	#^~~ramscalers	;Get bank of scalers
	pha			;Push program bank to jump to
	LA	;***************

;---------------------------------------
; Find first pixel to be drawn
;---------------------------------------

	lda	[scalerskip],y	;Screen pixels to skip
	clc
	adc	<screenofs
	sei			;Don't let an int happen when
				;  using the increment register
	sta	>WMADD
	lda	[scalerentry],y	;Entry point for scaler
	pha

;---------------------------------------
; Patch an RTL after the last drawn pixel
;---------------------------------------

	ldy	|2,x		;Second word is y2 (preshifted by 1)
	lda	[scalerentry],y	;  one before spot to patch

;---------------------------------------
; Set Y to the top of the virtual column
; being drawn (as if it were a full 64)
;---------------------------------------

	ldy	|4,x
	tax			;X holds patch point for duration

;---------------------------------------
; On entry to compiled scalers:
;    B : bank of sprite data
;    D : $2100 for access to WMDATA register
;    Y : start of sprite virtual column (as if it was 64 high)
;    LONGA OFF, LONGI ON
;    WMADD set to first pixel to be drawn
;---------------------------------------

	lda	#$2100
	tcd
	SA	;***************
	lda	#OP_RTL		;Patch return in
	sta	$7F0001,x	;+1 because entry is for rts to
	rtl			;Jump to compiled scaler

returnpoint:	cli			;Through with inc register
	lda	#OP_LDAY	;Clear out patched return
	sta	$7F0001,x	;+1 because entry is for rts to

	rep	#$21		;Clear m,x,carry
	LA	;***************
	lda	#~~rs_direct
	tcd			;Restore direct page

	lda	<columnptr
	adc	#6		;Next post
	sta	<columnptr
	jmp	drawpost


;====================================================================
; Put the rest of the subroutines in the normal code segment!

	ends
	code


;--------------------------------------------------------------------

MAXVISSPRITES	equ	64
SCREENWIDTH	set	112

;--- C structure sizes -----------------

MISSILESIZE	equ	18
ACTORSIZE	equ	30
STATICSIZE	equ	10

	extern	~~numvisspr
	extern	~~vissprites
	extern	~~visspr_p
	extern	~~xevents
	extern	~~xevent_p
	extern	~~statics
	extern	~~numstatics
	extern	~~areavis
	extern	~~actors
	extern	~~numactors
	extern	~~missiles
	extern	~~nummissiles
	extern	~~SortEvents
	extern	~~firstevent
	extern	~~lastevent
	extern	~~visspritetable
	extern	vs_x2
	extern	th_areanumber


;--------------------------------------------------------------------
; void DrawSprites(void)
;

	public	~~DrawSprites

	LCLSTART		;LOCAL VARIABLES:
	LCLW	xe
	LCLW	sprite_p
	LCLW	count
	ARGSTART		;NO ARGUMENTS!


~~DrawSprites:	PROLOGUE

;---------------------------------------
;   ~~visspr_p = ~~vissprites;	// No sprites in view yet
;   ~~numvisspr = 0
;---------------------------------------

	stz	~~numvisspr
	lda	#~~vissprites
	sta	~~visspr_p
	lda	#^~~vissprites
	sta	~~visspr_p+2
	
;---------------------------------------
; X events are now used as depth events
;    ~~xevent_p = ~~xevents;
;---------------------------------------

	lda	#~~xevents
	sta	~~xevent_p
	lda	#^~~xevents
	sta	~~xevent_p+2

;---------------------------------------
;Add all sprites in visareas:
;    for (i=0,stat = ~~statics; i<~~numstatics; i++,stat++)
;       if (~~areavis[stat->areanumber])
;          AddSprite ((thing_t *)stat, 0);
;---------------------------------------

	ldx	#~~statics
	lda	~~numstatics

	beq	donestatics
	sta	<count

staticloop:	lda	|th_areanumber,x
	asl	a
	tay
	lda	~~areavis,y
	bne	visstatic
	txa
	clc
	adc	#STATICSIZE
	tax
	dec	<count
	bne	staticloop
	jmp	donestatics

visstatic:	pea	0			;'actornum'
	pea	$0000+(^~~statics)	;Bank of 'statics'
	phx				;Offset into 'statics'
	stx	<sprite_p
	jsl	AddSprite
	lda	<sprite_p
	clc
	adc	#STATICSIZE
	tax
	dec	<count
	bne	staticloop

;--------------------------------------
;for (i=1,actor = ~~actors+1; i<~~numactors; i++,actor++)
;   if (~~areavis[actor->areanumber])
;      AddSprite ((thing_t *)actor, i);
;---------------------------------------

donestatics:	ldx	#~~actors+ACTORSIZE	;Player doesn't have a sprite
	lda	#1
	cmp	~~numactors
	beq	doneactors
	sta	<count

actorloop:	lda	|th_areanumber,x
	asl	a
	tay
	lda	~~areavis,y
	bne	visactor
	txa
	clc
	adc	#ACTORSIZE
	tax
	inc	<count
	lda	<count
	cmp	~~numactors
	bne	actorloop
	jmp	doneactors

visactor:	pei	<count			;'actornum'
	pea	$0000+(^~~actors)	;Bank of 'actors'
	phx				;Offset into 'actors'
	stx	<sprite_p
	jsl	AddSprite
	lda	<sprite_p
	clc
	adc	#ACTORSIZE
	tax
	inc	<count
	lda	<count
	cmp	~~numactors
	bne	actorloop

;---------------------------------------
;   for (i=0,missile = ~~missiles ; i<~~nummissiles ; i++,missile++)
;      if (~~areavis[missile->areanumber])
;         AddSprite ((thing_t *)missile,0);
;---------------------------------------

doneactors:	ldx	#~~missiles
	lda	~~nummissiles
	beq	donemissiles
	sta	<count

missileloop:	lda	|th_areanumber,x
	asl	a
	tay
	lda	~~areavis,y
	bne	vismissile
	txa
	clc
	adc	#MISSILESIZE
	tax
	dec	<count
	bne	missileloop
	jmp	donemissiles

vismissile:	pea	0			;'actornum'
	pea	$0000+(^~~missiles)	;Bank of 'missiles'
	phx				;Offset into 'missiles'
	stx	<sprite_p
	jsl	AddSprite
	lda	<sprite_p
	clc
	adc	#MISSILESIZE
	tax
	dec	<count
	bne	missileloop

;---------------------------------------
; Sort sprites from back to front
;---------------------------------------

donemissiles:	jsl	~~SortEvents

;---------------------------------------
; Draw from smallest scale to largest
;    for (xe=~~firstevent ; xe < ~~lastevent ; xe++)
;---------------------------------------

	lda	~~firstevent
	cmp	~~lastevent
	beq	alldone
	sta	<xe

;---------------------------------------
;    dseg = ~~vissprites + (*xe<AND>(MAXVISSPRITES-1));
;---------------------------------------

xeloop:	lda	(xe)
	and	#MAXVISSPRITES-1
	asl	a
	tax
	lda	>~~visspritetable,x
	tax

;---------------------------------------
;    x1 = dseg->x1;
;    if (x1<0)
;       x1 = 0;
;    rs_x1 = x1;
;---------------------------------------

	lda	|vs_x1,x
	bpl	x1ok
	lda	#0
x1ok:	sta	|~~rs_x1

;---------------------------------------
;    x2 = dseg->x2;
;    if (x2>=SCREENWIDTH)
;       x2 = SCREENWIDTH-1;
;    rs_x2 = x2;
;---------------------------------------

	lda	|vs_x2,x
	cmp	#SCREENWIDTH
	blt	x2ok
	lda	#SCREENWIDTH-1
x2ok:	sta	|~~rs_x2

;---------------------------------------
;rs_vseg = dseg;
;---------------------------------------

	lda	#^~~vissprites
	sta	|~~rs_vseg+2
	stx	|~~rs_vseg

	jsl	~~RenderSprite

	lda	<xe
	inc	a
	inc	a
	cmp	~~lastevent
	bge	alldone
	sta	<xe
	jmp	xeloop

alldone:	EPILOGUE		;RTL!


;--------------------------------------------------------------------
; void AddSprite(thing_t *thing, ushort actornum)
;

SCREENWIDTH	set	112
CENTERX	equ	SCREENWIDTH/2
MINZ	equ	62
MAXZ	equ	$2000

	extern	~~viewx
	extern	~~viewy
	extern	~~trx
	extern	~~try
	extern	~~R_TransformX
	extern	~~R_TransformZ
	extern	~~SUFixedMul
	extern	~~UFixedMul
	extern	~~UDiv16by8
	extern	~~scaleatz
	extern	~~sprites

	extern	th_x
	extern	th_y
	extern	th_pic
	extern	pa_width
	extern	pa_pos
	extern	vs_x2
	extern	vs_actornum
	extern	vs_strucsize

;--------------------------------------------------------------------
; void AddSprite (thing_t *thing, ushort actornum);
;

	LCLSTART
	LCLW	tx
	LCLW	tz
	LCLW	scale
	LCLW	px
	LCLW	patch
	LCLW	x1
	LCLW	x2
	LCLW	width
	LCLW	zslope

	ARGSTART
	ARGD	thing
	ARGW	actornum

	public	AddSprite

AddSprite:	PROLOGUE

;---------------------------------------
; Transform the origin point
;    ~~trx = thing->x - ~~viewx;
;    ~~try = ~~viewy - thing->y;
;---------------------------------------

	ldx	<thing
	lda	|th_x,x
	sec
	sbc	~~viewx
	sta	~~trx

	lda	~~viewy
	sec
	sbc	|th_y,x
	sta	~~try

;---------------------------------------
;   tz = R_TransformZ ();
;   tz -= 0x40;	// Shift forward a bit because the sprite doesn't represent a crosssection
;   if (tz < MINZ)
;   {
;      if (missilesprite)
;         tz = MINZ;	// This is a hack to ensure that missile explosions will allways be visible
;      else
;         return;	// Thing is behind view plane
;    }
;---------------------------------------

	jsl	~~R_TransformZ
	ora	#0		;Set flags
	bpl	noclip
exit:	EPILOGUE		;RTL!

;---------------------------------------
;    if (tz>=MAXZ)
;       tz = MAXZ-1;
;---------------------------------------

noclip:	cmp	#MAXZ
	blt	belowmax
	lda	#MAXZ-1
belowmax	sta	<tz

;---------------------------------------
;    zslope = tz + (tz>>1);
;---------------------------------------

	lsr	a
	lsr	a
	clc
	adc	<tz
	sta	<zslope

;---------------------------------------
;    tx = R_TransformX ();
;---------------------------------------

	jsl	~~R_TransformX
	sta	<tx
	ora	#0		;Set flags

;---------------------------------------
;    if ( (tx > 0 <AND> (ushort)tx> zslope) || (tx<0 <AND> (ushort)-tx>zslope) )
;       return;	// Off the sides
;---------------------------------------

	bmi	negtx
	cmp	<zslope
	bge	exit
	jmp	inview

negtx:	eor	#-1
	inc	a
	cmp	<zslope
	bge	exit

;---------------------------------------
;    scale = ~~scaleatzptr[tz];
;---------------------------------------

inview:	lda	<tz
	asl	a
	tax
	lda	>~~scaleatz,x
	sta	<scale

;---------------------------------------
;    px = CENTERX + (SUFixedMul (tx,scale<<1) >>8);
;---------------------------------------

	asl	a
	pha
	pei	<tx
	jsl	~~SUFixedMul
	xba
	bit	#$80
	beq	positivexm
	ora	#$FF00		;Sign extend
	jmp	gotxm
positivexm	and	#$00FF		;Zero extend
gotxm	clc
	adc	#CENTERX
	sta	<px

;---------------------------------------
; Calculate a shifted forward scale to
; increase sprite size
;
;    tz-=0x40;
;    scale = ~~scaleatzptr[tz];
;---------------------------------------

	lda	<tz
	sec
	sbc	#$40
	bmi	bound
	cmp	#62
	bge	goodz
bound	lda	#62

goodz:	asl	a
	tax
	lda	>~~scaleatz,x
	sta	<scale


;---------------------------------------
; Calculate edges of the shape
;     patch = <AND>sprites[thing->sprite];
;---------------------------------------

	ldx	<thing
	lda	|th_pic,x
	asl	a
	sta	<patch
	asl	a
	adc	<patch		;6 byte structure
	adc	#~~sprites
	sta	<patch
	tax

;---------------------------------------
;    width = UFixedMul (patch->width,scale)>>5;	// OPTIMIZE
;    if (!width)
;       return;	// Too far away
;---------------------------------------

	pei	<scale
	lda	|pa_width,x
	pha
	jsl	~~UFixedMul
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	beq	exit2
	sta	<width

;---------------------------------------
;    x1 = px - (short)(width>>1);
;    if (x1 >= SCREENWIDTH)
;       return;	// Off the right side
;---------------------------------------

	lsr	a
	eor	#-1
	inc	a
	clc
	adc	<px
	bmi	notoffright
	cmp	#SCREENWIDTH
	bge	exit2

notoffright:	sta	<x1

;---------------------------------------
;    x2 = x1 + width - 1;
;    if (x2 < 0)
;       return;	// Off the left side
;---------------------------------------

	clc
	adc	<width
	dec	a
	bmi	exit2
	sta	<x2

;---------------------------------------
;    ~~visspr_p->pos = patch->pos;
;    ~~visspr_p->x1 = x1;
;    ~~visspr_p->x2 = x2;
;    ~~visspr_p->clipscale = scale;
;    ~~visspr_p->actornum = actornum;
;---------------------------------------

	ldy	<patch
	ldx	~~visspr_p
	lda	|pa_pos,y
	sta	|vs_pos,x
	lda	|pa_pos+2,y
	sta	|vs_pos+2,x
	lda	<x1
	sta	|vs_x1,x
	lda	<x2
	sta	|vs_x2,x
	lda	<scale
	sta	|vs_clipscale,x
	lda	<actornum
	sta	|vs_actornum,x

;---------------------------------------
; *OPTIMIZE: base this on scale
;    ~~visspr_p->columnstep =  UDiv16by8 (patch->width<<8, x2-x1+1);
;---------------------------------------

	pei	<width
	lda	|pa_width,y
	xba
	pha
	jsl	~~UDiv16by8
	ldx	~~visspr_p
	sta	|vs_columnstep,x

;---------------------------------------
; Pack the vissprite number into the low
; 6 bits of the scale for sorting
;
;    *~~xevent_p++ = (scale<AND>~(MAXVISSPRITES-1)) |  ~~numvisspr);
;    ~~visspr_p++;
;    ~~numvisspr++
;---------------------------------------

	lda	<scale
	and	#$FFC0
	ora	~~numvisspr
	ldx	~~xevent_p
	sta	|0,x
	inx
	inx
	stx	~~xevent_p

	inc	~~numvisspr
	lda	~~visspr_p
	clc
	adc	#vs_strucsize
	sta	~~visspr_p

exit2:	EPILOGUE		;RTL!

;--------------------------------------------------------------------

	ends
	end
