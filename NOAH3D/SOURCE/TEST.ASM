*--------------------------------------------------------------------
* TEST.ASM
*--------------------------------------------------------------------

; This is the basic interrupt vectoring code.  It is assembled to run
; at bank $C0, but it will be shadowed into the upper 32k of bank 0.
; The vectors point to a stup routine that executes in bank 0 and
; just does a far jump to the real routine in bank $C0.

	spaces	on
	llchar	.

	include	snes.equ
	include idmacros.asm


boot	section	;offset	$C0FC00	;Boot code & related

;--------------------------------------------------------------------

	extern	~~joystick1
	extern	~~joystick2
	extern	~~mouse1
	extern	~~mouse2
	extern	~~mouse3
	extern	~~mouse4
	extern	~~mousepresent
	extern	~~badmouse
	extern	~~ticcount
	extern	~~irqhook
	extern	~~nmihook
	extern	~~SampleControls
	extern	~~MusicTic
	extern	~~main
	extern	~~sounds

;--------------------------------------------------------------------

	data
tmouse	dw	0
	ends

;--------------------------------------------------------------------

	public	~~DEBUG		;jgt;
	extern	~~IO_Debug

~~DEBUG:	SI	;***************
	ply			;Get processor status
	pla			;Get BRK address
	plx			;Get BRK bank
	LI	;***************
	phy			;Pass them to IO_Debug as
	pha			;  word args. in reverse order
	phx
	jsl	~~IO_Debug	;Never returns!
	brk

;--------------------------------------------------------------------

	IF	DEBUG		;jgt;
	data

	public	~~CompileDate

~~CompileDate	DATE

	boot
	ENDIF			;jgt;

;--------------------------------------------------------------------
; For some reason ID placed 'pause' in the normal code segment!

	code

pause:	SA	;***************
p1	lda	>~~joystick1
	beq	p1
p2	lda	>~~joystick1
	bne	p2
	LA	;***************
	rtl

	boot

;====================================================================
; B should be C data bank
; Wait for auto read

ReadControls:	SA	;***************
joystickwait	lda	>HVBJOY		;Wait until joypad is ready
	and	#1		;Joystick busy flag
	bne	joystickwait

;Read the joystick

	lda	>JOY1		;Get joypad value
	sta	~~joystick1
	lda	>JOY1+1
	sta	~~joystick1+1

;%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%;shit;fuck;
	lda	>JOY2
	sta	~~joystick2
	lda	>JOY2+1
	sta	~~joystick2+1
;%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%;shit;fuck;

;Read the mouse

	lda	>JOY2		;Get auto read vals
	sta	~~mouse1
	and	#$F
	cmp	#1		;Mouse signature
	beq	mouseconnected
	stz	~~mousepresent
	jmp	mdone

mouseconnected	lda	~~mousepresent
	bne	readrest
	inc	~~mousepresent

;The mouse has not been initialized yet, make sure speed is cycled

	lda	#$FF
	sta	~~mouse1	;Make sure speed is not correct

;If the mouse speed is not set to 0, cycle it and reread

readrest	lda	~~mouse1
	and	#$30
	beq	speedcorrect

	lda	#1		;Cycle speed
	sta	>$4016
	lda	>$4017
	lda	#0
	sta	>$4016
	jmp	joystickwait

speedcorrect	lda	>JOY2+1
	sta	~~mouse2

	stz	~~badmouse

	lda	>$4017
	lsr	a
	rol	tmouse
	lda	>$4017
	lsr	a
	rol	tmouse
	lda	>$4017
	lsr	a

	rol	tmouse
	lda	>$4017
	lsr	a
	rol	tmouse
	lda	>$4017
	lsr	a
	rol	tmouse
	lda	>$4017
	lsr	a

	rol	tmouse
	lda	>$4017
	lsr	a
	rol	tmouse
	lda	>$4017
	lsr	a
	rol	tmouse
	lda	tmouse
	sta	~~mouse3

	lda	>$4017
	lsr	a
	rol	tmouse
	lda	>$4017
	lsr	a
	rol	tmouse

	lda	>$4017
	lsr	a
	rol	tmouse
	lda	>$4017
	lsr	a
	rol	tmouse
	lda	>$4017
	lsr	a
	rol	tmouse

	lda	>$4017
	lsr	a
	rol	tmouse
	lda	>$4017
	lsr	a
	rol	tmouse
	lda	>$4017
	lsr	a
	rol	tmouse
	lda	tmouse
	sta	~~mouse4

;If joystick busy was ever set, the value is bad

	lda	>HVBJOY		;If set, reread
	ora	~~badmouse
	and	#1
	beq	mdone

waitnobusy	lda	>HVBJOY
	and	#1
	bne	waitnobusy
waitbusy	lda	>HVBJOY
	and	#1
	beq	waitbusy
	jmp	joystickwait

mdone	LA	;***************
	rts


;==============================================================================

irqhandler:	LAI	;***************

	phb			;Save everything
	phd
	pha
	phx
	phy

	pea	$7E7E
	plb
	plb			;Set data bank

	lda 	>TIMEUP		;Read & reset IRQ status flag

	inc	~~ticcount	;Increment global vbl counter

;--- Call user handler -----------------

	lda	>irqflag
	bne	skipirq
	inc	a
	sta	>irqflag

	jsr	ReadControls

	lda	~~irqhook
	ldx	~~irqhook+2
	beq	.returnpoint	;If bank word is 0, don't call it
	SI	;***************;Use short index for bank bytes

	ldy	#^(.returnpoint-1)
	phy
	pea	.returnpoint-1	;Push the address to return to

	phx
	dec	a		;RTL goes one past
	pha			;Push the address to call to

	LI	;***************
	rtl			;Jump to the hook

;---------------------------------------
; The hook will return to here:      (1)
;---------------------------------------

.returnpoint
	jsl	~~SampleControls
	jsl	~~MusicTic

; Done
	stz	irqflag
skipirq:	ply
	plx
	pla
	pld
	plb
	rti

	data
nmiflag:	dw	0
irqflag:	dw	0
	boot

;====================================================================

nmihandler:	LAI	;***************;Full 16-bit mode

	phb			;Save everything
	phd
	pha
	phx
	phy

	lda	>nmiflag
	bne	skipnmi
	inc	a
	sta	>nmiflag

	pea	$7E7E
	plb
	plb			;Set data bank

	lda	>RDNMI		;Read and clear NMI flag

	inc	~~ticcount	;Increment global vbl counter

;--- Call user handler -----------------

	lda	~~nmihook
	ldx	~~nmihook+2
	beq	.returnpoint	;If bank word is 0, don't call it
	SI	;***************;Use short index for bank bytes

	ldy	#^(.returnpoint-1)
	phy
	pea	.returnpoint-1			; push the address to return to

	phx
	dec	a		;RTL goes one past
	pha			;Push the address to call to

	LI	;***************
	rtl			;Jump to the hook


;---------------------------------------
; The hook will return to here:	     (2)
;---------------------------------------

.returnpoint	jsr	ReadControls
	jsl	~~MusicTic

	stz	nmiflag

skipnmi:	ply
	plx
	pla
	pld
	plb
	rti


;====================================================================

resethandler:	clc		;jgt; These 2 instructions were in ID's
	xce		;jgt; final ROM but NOT in the source!

	LAI	;***************;Full 16-bit mode

	cld			;Clear decimal mode
	lda	#$0FFF
	tcs			;Stack just below $1000
	lda	#0
	tcd			;Direct page of 0

	jsr	clearhardware
	jsr	initsound

	IF	0	;%%%%%%%

	SA	;***************
	lda	#$81		;Enable NMI and joystick reading
	sta	>NMITIMEN	;jgt??? (should be #$21 ???)
	LA	;***************
	SA	;***************
	lda	#5
	sta	>INIDISP
	LA	;***************
	jsl	pause
	SA	;***************
	lda	#10
	sta	>INIDISP
	LA	;***************
	jsl	pause

	ENDIF	;%%%%%%%%%%%%%%%

	jsr	initrambank

	IF	ID_VERSION	;jgt;
;---------------------------------------
; Actually, this section was NOT in ID's
; ROM version of WOLF3D, but it was
; in their 'final' source code...
;

	SA	;***************
	lda	#15
	sta	>INIDISP
	LA	;***************
*	jsl	pause

;---------------------------------------
	ENDIF			;jgt;

	SA	;***************
	lda	#$81		;Enable NMI and joystick reading
	sta	>NMITIMEN	;jgt??? (should be #$21 ???)
	LA	;***************

	cli			;Begin normal operation
	jml	~~main		;Go to C code


;====================================================================

clearhardware:	LI	;***************
	SA	;***************

	pea	$0000
	plb
	plb			;Set bank to 0 for hardware regs

	ldy	#0

setbyte	ldx	inittable,y	;Get register address
	beq	chdone		;0 address is end of list
	iny
	iny
	lda	inittable,y	;Get initial value
	iny
	sta	|0,x		;Set register
	bra	setbyte

chdone	LAI	;***************;Back to 16-bit mode
	rts

;---------------------------------------

VREG	MACRO	reg,val
	dw	reg
	db	val
	ENDM

inittable	VREG	$2100,$8F
	VREG	$2101,0
	VREG	$2102,0
	VREG	$2103,0

	VREG	$2105,0
	VREG	$2106,0
	VREG	$2107,0
	VREG	$2108,0
	VREG	$2109,0
	VREG	$210A,0
	VREG	$210B,0
	VREG	$210C,0
	VREG	$210D,0
	VREG	$210D,0
	VREG	$210E,0
	VREG	$210E,0
	VREG	$210F,0
	VREG	$210F,0
	VREG	$2110,0
	VREG	$2110,0
	VREG	$2111,0
	VREG	$2111,0
	VREG	$2112,0
	VREG	$2112,0
	VREG	$2113,0
	VREG	$2113,0
	VREG	$2114,0
	VREG	$2114,0
	VREG	$2115,$80
	VREG	$2116,0
	VREG	$2117,0

	VREG	$211A,0
	VREG	$211B,0
	VREG	$211B,1
	VREG	$211C,0
	VREG	$211C,0
	VREG	$211D,0
	VREG	$211D,0
	VREG	$211E,0
	VREG	$211E,1
	VREG	$211F,0
	VREG	$211F,0
	VREG	$2120,0
	VREG	$2120,0
	VREG	$2121,0

	VREG	$2123,0
	VREG	$2124,0
	VREG	$2125,0
	VREG	$2126,0
	VREG	$2127,0
	VREG	$2128,0
	VREG	$2129,0
	VREG	$212A,0
	VREG	$212B,0
	VREG	$212C,0
	VREG	$212D,0
	VREG	$212E,0
	VREG	$212F,0
	VREG	$2130,$30
	VREG	$2131,0
	VREG	$2132,$E0
	VREG	$2133,0

	VREG	$4200,0
	VREG	$4201,$FF
	VREG	$4202,0
	VREG	$4203,0
	VREG	$4204,0
	VREG	$4205,0
	VREG	$4206,0
	VREG	$4207,0
	VREG	$4208,0
	VREG	$4209,0
	VREG	$420A,0
	VREG	$420B,0
	VREG	$420C,0
	VREG	$420D,1		; 1 = high speed

	dw	0		;End of list marker


;====================================================================
; Setup DATA & UDATA sections in RAM

	xref	_BEG_DATA
	xref	_END_DATA
	xref	_END_KDATA
	xref	_BEG_UDATA
	xref	_END_UDATA

initrambank:

;---------------------------------------
; Set up DATA BANK register to point to
; the DATA section
;---------------------------------------

	SA	;***************
	lda	#^_BEG_DATA	;Get bank of data
	pha
	plb			;Set data bank register
	LA	;***************

;---------------------------------------
; Copy initialized data from ROM to RAM;
; Initialized data follows constant data
; section (KDATA)...
;---------------------------------------

; NOTE: SNES for some reason will crash
;  if MVN is used to transfer the data!

	ldy	#_END_DATA-_BEG_DATA ;# of bytes to copy
	beq	.skip		;If none, just skip

	ldx	#0		;Clear index (!Note: Assumes start of bank!)
	SA	;***************
.dloop	lda	>_END_KDATA,x	;Get byte from ROM
	sta	|_BEG_DATA,x	;Store into RAM
	inx			;Bump pointer
	dey			;Decrement count
	bne	.dloop		;Continue until done
	LA	;***************

;---------------------------------------
;Clear uninitialized data area (UDATA)
;---------------------------------------

.skip	ldy	#_END_UDATA-_BEG_UDATA  ;Get number of bytes to clear
	beq	.done		;Nothing to do

	lda	#0		;Get a zero for storing
	SA	;***************
	ldx	#_BEG_UDATA	;Get beginning of zeros

.uloop	sta	|0,x		;Clear memory
	inx			;Bump pointer
	dey			;Decrement count
	bne	.uloop		;Continue until done
	LA	;***************

.done	rts


;====================================================================

initsound:	pea	0
	plb
	plb			;For hardware regs
	stz	SPC700Port1	;Clear all four sound ports
	stz	SPC700Port3

	pea	$0000+^~~sounds
	pea	~~sounds
	jsl	UploadSnd

	IF	0	;%%%%%%%

	lda	#$FF
wait	bit	SPC700Port1 	;Wait for driver to start up
	bne	wait

	ENDIF	;%%%%%%%%%%%%%%%

	rts


;---------------------------------------
; Upload sound data to the SPC700
; processor...
; 
; Data must be formatted with:
;
; DA Length
; DA TrueAddress
; DB Bytes
;
; DA 0
; DA Jump address
;---------------------------------------

	LCLSTART

	ARGSTART
	ARGD	UploadPointer

UploadSnd:	PROLOGUE

	php			;Save interrupt flag
	sei
	ldy	#0		;Init my master index
	lda	#$BBAA		;Main signature
WaitBBAA	cmp	SPC700Port1	;Wait until SPCPort is ready
	bne	WaitBBAA
	SA	;***************
	lda	#$CC		;First Handshake byte
	bra	BeginLoad	;Merge

BeginPacket	lda	[UploadPointer],Y ;Get a byte of data
	iny			;+1 to pointer
	xba			;Save byte in "B"
	lda	#0		;Init handshake byte
	bra	Merge2		;Merge

* Main data transfer loop
* "X" has byte count

ByteLoop	xba			;Put handshake byte in "B"
	lda	[UploadPointer],Y ;Get byte to transfer
	iny
	xba
WaitHand1	cmp	SPC700Port1	;Wait for handshake
	bne	WaitHand1
	inc	a		;+1 to handshake byte (8 bit)
Merge2	LA	;***************;16-bit
	sta	SPC700Port1	;Write data and sync byte
	SA	;***************;8-bit
	dex			;Count down
	bne	ByteLoop

WaitHand2	cmp	SPC700Port1	;Wait for final handshake
	bne	WaitHand2
FixHand	adc	#4-1		;Make a new handshake byte (BUT DON'T
	beq	FixHand		; use #0!!)

* Get the header for an upload packet

BeginLoad	pha			;Save initial handshake value
	LA	;***************;16-bit
	lda	[UploadPointer],Y ;Get length of data
	iny			;+2 to pointer
	iny
	tax			;Put length in X
	lda	[UploadPointer],Y ;Get address of data transfer
	iny
	iny
	sta	SPC700Port3	;Save data transfer address to SPC700

	SA	;***************;8-bit
	cpx	#1		;Zero bytes to transfer?
	lda	#0		;0 or 1
	rol	a		;Save a 1 if a packet is to be sent
	sta	SPC700Port2	;  otherwise send a zero
	adc	#$7F		;Set overflow if #1
	pla			;Restore handshake byte
	sta	SPC700Port1	;Send handshake (Initiate command)
WaitAgain	cmp	SPC700Port1	;Wait for ACK
	bne	WaitAgain
	bvs	BeginPacket	;Packet? (Begin it)
	LA	;***************;Restore 16-bit mode
	stz	SPC700Port1	;End transfer by clearing all ports
	stz	SPC700Port3
	plp			;Restore IRQ flags

	EPILOGUE		;RTL!


;====================================================================

irqvector:	clc
	xce
	jml	>irqhandler	;Jump to fast rom

nmivector:	clc
	xce
	jml	>nmihandler	;Jump to fast rom

resetvector:
	IF	ID_VERSION	;jgt;
;---------------------------------------
; Actually, this section was NOT in ID's
; ROM version of WOLF3D, but it was
; in their 'final' source code...
;

	SA	;***************
	lda	#0
	sta	INIDISP

;---------------------------------------
	ENDIF			;jgt;

	clc
	xce
	jml	>resethandler	;Jump to fast rom


;====================================================================
;		65816 Interrupt vectors
;====================================================================

vectors	section	;offset	$C0FFE0	;65816 Interrupt vectors, etc.

vectors:	dw	~~DEBUG		;$FFE0		;NATIVE
	dw	~~DEBUG		;$FFE2
	dw	~~DEBUG		;$FFE4	COP (COProcessor)
	dw	~~DEBUG		;$FFE6	BRK
	dw	~~DEBUG		;$FFE8	ABORT
	dw	nmivector	;$FFEA	IRQ
	dw	resetvector	;$FFEC	RESET
	dw	irqvector	;$FFEE	IRQ

	dw	~~DEBUG		;$FFF0		;EMULATION
	dw	~~DEBUG		;$FFF2
	dw	~~DEBUG		;$FFF4
	dw	~~DEBUG		;$FFF6
	dw	~~DEBUG		;$FFF8	ABORT
	dw	nmivector	;$FFFA	NMI
	dw	resetvector	;$FFFC	RESET
	dw	irqvector	;$FFFE	IRQ

;--------------------------------------------------------------------

	ends
	end
