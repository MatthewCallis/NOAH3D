*--------------------------------------------------------------------
* NEWREFA.ASM
*--------------------------------------------------------------------

	spaces	on
	llchar	.

	include	snes.equ
	include idmacros.asm

	code

;--------------------------------------------------------------------
; ushort AngleFromSlope (ushort yval, ushort xval)
;

	public	~~AngleFromSlope

	extern	~~tantoangle

	LCLSTART
	ARGW	sinea
	ARGW	sineb

	ARGSTART
	ARGW	yval
	ARGW	xval

~~AngleFromSlope:	PROLOGUE

	lda	<xval

	cmp	#$100
	blt	gotsize
	lsr	a
	lsr	<yval
	cmp	#$100
	blt	gotsize
	lsr	a
	lsr	<yval
	cmp	#$100
	blt	gotsize
	lsr	a
	lsr	<yval
	cmp	#$100
	blt	gotsize
	lsr	a
	lsr	<yval
	cmp	#$100
	blt	gotsize
	lsr	a
	lsr	<yval
	cmp	#$100
	blt	gotsize
	lsr	a
	lsr	<yval

gotsize	sta	<xval
	lda	<yval
	cmp	#$80
	blt	gotysize
	lsr	a
	lsr	<xval
	cmp	#$80
	blt	gotysize
	lsr	a
	lsr	<xval
	cmp	#$80
	blt	gotysize
	lsr	a
	lsr	<xval
	cmp	#$80
	blt	gotysize
	lsr	a
	lsr	<xval
	cmp	#$80
	blt	gotysize
	lsr	a
	lsr	<xval
	cmp	#$80
	blt	gotysize
	lsr	a
	lsr	<xval
	cmp	#$80
	blt	gotysize
	lsr	a
	lsr	<xval

;   return tantoangle[  ((long)y<<SLOPEBITS)/x];

gotysize	xba
	asl	a		  ; y<<9
	sta	>WRDIVD
	SA	;***************
	lda	<xval
	sta	>WRDIVB
	LA	;***************
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	lda	>RDDIVL
	asl	a
	tax
	lda	>~~tantoangle,x

	EPILOGUE		;RTL!


;--------------------------------------------------------------------
; ufixed_t ScaleFromGlobalAngle (short visangle)
;

FINEANGLES	equ	2048
FINEMASK	equ	FINEANGLES-1
MAXZ	equ	32*256

	extern	~~centerangle
	extern	~~finesin
	extern	~~normalangle
	extern	~~rw_distance
	extern	~~UFixedMul
	extern	~~scaleatz

;-------------------------------

	public	~~ScaleFromGlobalAngle

	LCLSTART
	LCLW	sinea
	LCLW	sineb

	ARGSTART
	ARGW	visangle

~~ScaleFromGlobalAngle:	PROLOGUE

;-------------------------------
;   visangle >>= ANGLETOFINESHIFT;
;-------------------------------

	lda	<visangle
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	clc
	adc	#FINEANGLES/4
	sta	<visangle

;-------------------------------
;   anglea = (FINEANGLES/4 + (visangle-centerangle))<and>FINEMASK;
;   sinea = finesine[anglea];	// Both sines are allways positive
;-------------------------------

	sec
	sbc	~~centerangle
	and	#1023
	asl	a
	tax
	lda	>~~finesin,x
	sta	<sinea

;-------------------------------
;   angleb = (FINEANGLES/4 + (visangle-normalangle))<and>FINEMASK;
;   sineb = finesine[angleb];
;-------------------------------

	lda	<visangle
	sec
	sbc	~~normalangle
	and	#1023
	asl	a
	tax
	lda	>~~finesin,x
	sta	<sineb

;-------------------------------
;   tz = ((long)w_abs(rw_distance) * sinea) / sineb;
;-------------------------------

	lda	<sinea
	xba
	sta	>WRDIVD
	SA	;***************
	lda	<sineb
	sta	>WRDIVB
	LA	;***************
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	lda	>RDDIVL
	pha
	lda	~~rw_distance
	bpl	distok
	eor	#-1
	inc	a
distok	pha
	jsl	~~UFixedMul

;-------------------------------
;   if (tz>=MAXZ)
;      tz = MAXZ-1;
;-------------------------------

	cmp	#MAXZ
	blt	zok
	lda	#MAXZ-1
zok	asl	a
	tax
	lda	>~~scaleatz,x

	EPILOGUE		;RTL!


;--------------------------------------------------------------------
; void ClipWallSegment (ushort top, ushort bottom)
;

SS_TOP	equ	0
SS_BOTTOM	equ	2
SS_SIZE	equ	4

	extern	~~solidsegs
	extern	~~newend
	extern	~~RenderWallRange

;--------------------------------------

	public	~~ClipWallSegment

	LCLSTART
	LCLW	next
	LCLW	startp

	ARGSTART
	ARGW	top
	ARGW	bottom

~~ClipWallSegment:	PROLOGUE

;--------------------------------------
; Find the first clippost that touches
; the source post (adjacent pixels are
; touching)
;
;    start = solidsegs;
;    while (start->bottom > top+1)
;       start++;
;--------------------------------------

	ldx	#~~solidsegs
	lda	<top
	inc	a
	inc	a
checkseg	cmp	|SS_BOTTOM,x
	bge	gotfirst
	inx
	inx
	inx
	inx
	jmp	checkseg

gotfirst:	stx	<startp

;--------------------------------------
;   if (top > start->top)
;--------------------------------------

	lda	<top
	cmp	|SS_TOP,x
	beq	nottopover
	blt	nottopover

;--------------------------------------
;   if (bottom > start->top+1)
;      RenderWallRange (top, bottom);
;--------------------------------------

	lda	|SS_TOP,x
	inc	a
	cmp	<bottom
	bge	notbotover

	pei	<bottom
	pei	<top
	jsl	~~RenderWallRange

;--------------------------------------
;   next = newend;
;   newend++;
;--------------------------------------

	lda	~~newend
	sta	<next
	tax
	clc
	adc	#SS_SIZE
	sta	~~newend

;--------------------------------------
;   while (next != start)
;   {
;      *next = *(next-1);
;      next--;
;   }
;--------------------------------------

openloop	cpx	<startp
	beq	opendone
	dex
	dex
	dex
	dex
	lda	|SS_TOP,x
	sta	|SS_TOP+SS_SIZE,x
	lda	|SS_BOTTOM,x
	sta	|SS_BOTTOM+SS_SIZE,x
	jmp	openloop

;--------------------------------------
;   next->top = top;
;   next->bottom = bottom;
;   return;
;--------------------------------------

opendone	lda	<top
	sta	|SS_TOP,x
	lda	<bottom
	sta	|SS_BOTTOM,x

	EPILOGUE


;--------------------------------------
; There is a fragment above *start
;
;    RenderWallRange (top, start->top + 1);
;    start->top = top;	// Adjust the clip size
;--------------------------------------

notbotover	lda	|SS_TOP,x
	inc	a
	pha
	pei	<top
	jsl	~~RenderWallRange
	ldx	<startp
	lda	<top
	sta	|SS_TOP,x

;--------------------------------------
; if (bottom >= start->bottom)
;     return;	// Bottom contained in start
;--------------------------------------

nottopover	lda	<bottom
	cmp	|SS_BOTTOM,x
	blt	notcontained

	EPILOGUE


;--------------------------------------
; next = start;
;--------------------------------------

notcontained:	stx	<next

;--------------------------------------
; while (bottom <= (next+1)->top+1)
;--------------------------------------

chopinterior:	lda	|SS_TOP+SS_SIZE,x
	inc	a
	cmp	<bottom
	blt	donechop

;--------------------------------------
; There is a fragment between two
; posts
;
;    RenderWallRange (next->bottom - 1, (next+1)->top + 1);
;    next++;
;--------------------------------------

	pha
	lda	|SS_BOTTOM,x
	dec	a
	pha
	txa
	clc
	adc	#SS_SIZE
	sta	<next
	jsl	~~RenderWallRange
	ldx	<next

;--------------------------------------
;   if (bottom >= next->bottom)
;   {  // Bottom is contained in next
;      start->bottom = next->bottom;	// Adjust the clip size
;      goto crunch;
;   }
;--------------------------------------

	lda	|SS_BOTTOM,x
	cmp	<bottom
	bge	chopinterior
	ldx	<startp
	sta	|SS_BOTTOM,x
	jmp	crunch

;--------------------------------------
; There is a fragment after *next
;
;    RenderWallRange (next->bottom - 1, bottom);
;    start->bottom = bottom;	// Adjust the clip size
;--------------------------------------

donechop:	pei	<bottom
	lda	|SS_BOTTOM,x
	dec	a
	pha
	jsl	~~RenderWallRange
	lda	<bottom
	ldx	<startp
	sta	|SS_BOTTOM,x
	
;--------------------------------------
; Remove start+1 to next from
; the clip list, because start
; now covers their area
;
;    if (next == start)
;       return;	// Post just extended past the bottom of one post
;--------------------------------------

crunch:	cpx	<next
	beq	done

;--------------------------------------
;   while (next++ != newend)	// Remove a post
;      *++start = *next;
;--------------------------------------

	ldx	<next
	ldy	<startp

crunchloop:	tya
	clc
	adc	#SS_SIZE
	tay
	cpx	~~newend
	beq	moveend
	txa
	clc
	adc	#SS_SIZE
	tax
	lda	|SS_TOP,x
	sta	|SS_TOP,y
	lda	|SS_BOTTOM,x
	sta	|SS_BOTTOM,y
	jmp	crunchloop

;--------------------------------------
;   newend = start+1;
;--------------------------------------

moveend:	sty	~~newend

done:	EPILOGUE		;RTL!


;--------------------------------------------------------------------

	end
