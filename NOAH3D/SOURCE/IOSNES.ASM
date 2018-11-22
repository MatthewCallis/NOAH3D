*--------------------------------------------------------------------
* IOSNES.ASM
*--------------------------------------------------------------------

; This is IOSNES.C translated into 65816 assembly because it was too
; much of a pain to force the Zardoz assembler to output correct code
; for the port accesses (also, the code generated was pretty ugly...)

	include	IDMACROS.ASM	;Common assembly macros
	include	SNES.EQU	;SNES hardware reg. addresses

	spaces	on
	llchar	.
	code

;--------------------------------------------------------------------
;		GLOBAL VARIABLES
;--------------------------------------------------------------------

	udata			;UNINITIALIZED DATA

	public	~~ticcount	;Global timer
~~ticcount	dw	0
	public	~~joystick1	;Controller
~~joystick1	dw	0
	public	~~joystick2
~~joystick2	dw	0
	public	~~fadelevel	;Shadow if 'inidisp' register
~~fadelevel	db	0

	public	~~irqhook
~~irqhook	dl	0
	public	~~nmihook
~~nmihook	dl	0

	code

;--------------------------------------------------------------------
; void DmaMode7Tiles (void *source, unsigned dest, unsigned length)

	public	~~DmaMode7Tiles

	ARGSTART
	ARGD	source
	ARGW	dest
	ARGW	length

~~DmaMode7Tiles	PROLOGUE		;Put arguments in direct page

	phb			;Save data bank
	pea	0		;Set data bank to 0 for hardware regs
	plb
	plb

	php			;Save processor status
	sei			;Disable interrupts
	SI	;***************

	lda	<source		;Get source address
	sta	DMA0ADDR
	ldx	<source+2	;Get source bank
	stx	DMA0BANK
	lda	<length		;Get source length
	sta	DMA0LEN
	ldx	#0		;Transfer bytes, incrementing
	stx	DMA0PARMS
	ldx	#(VMDATAH-$2100);Transfer to high byte of VRAM
	stx	DMA0PORT

	lda	<dest		;Get destination address
	sta	VMADD		;Set OARAM position
	ldx	#$80		;Increment 1 by 1 on low byte
	stx	VMAINC

	ldx	#1		;Start channel 0 DMA
	stx	MDMAEN

	LI	;***************
	plp			;Restore processor status
	plb			;Restore data bank

	EPILOGUE		;RTL!


;--------------------------------------------------------------------
; void DmaMode7Screens (void *source, unsigned dest, unsigned length)

	public	~~DmaMode7Screens

	ARGSTART
	ARGD	source
	ARGW	dest
	ARGW	length

~~DmaMode7Screens	PROLOGUE	;Put arguments in direct page

	phb			;Save data bank
	pea	0		;Set data bank to 0 for hardware regs
	plb
	plb

	php			;Save processor status
	sei			;Disable interrupts
	SI	;***************

	lda	<source		;Get source address
	sta	DMA0ADDR
	ldx	<source+2	;Get source bank
	stx	DMA0BANK
	lda	<length		;Get source length
	sta	DMA0LEN
	ldx	#0		;Transfer bytes, incrementing
	stx	DMA0PARMS
	ldx	#(VMDATAL-$2100);Transfer to low byte of VRAM
	stx	DMA0PORT

	lda	<dest		;Get destination address
	sta	VMADD		;Set OARAM position
	ldx	#0		;Increment 1 by 1 on low byte
	stx	VMAINC

	ldx	#1		;Start channel 0 DMA
	stx	MDMAEN

	LI	;***************
	plp			;Restore processor status
	plb			;Restore data bank

	EPILOGUE		;RTL!


;--------------------------------------------------------------------
; void DmaPlayScreen (void *screen)

	public	~~DmaPlayScreen
~~DmaPlayScreen
	rtl


;--------------------------------------------------------------------
; void DmaOAram (void *source, unsigned dest, unsigned length)
;

	public	~~DmaOAram

	ARGSTART
	ARGD	source
	ARGW	dest
	ARGW	length

~~DmaOAram	PROLOGUE		;Put arguments in direct page

	phb			;Save data bank
	pea	0		;Set data bank to 0 for hardware regs
	plb
	plb

	php			;Save processor status
	sei			;Disable interrupts
	SI	;***************

	lda	<source		;Get source address
	sta	DMA0ADDR
	ldx	<source+2	;Get source bank
	stx	DMA0BANK
	lda	<length		;Get source length
	sta	DMA0LEN
	ldx	#2		;Transfer wide bytes, incrementing
	stx	DMA0PARMS
	ldx	#(OAMDATAW-$2100);B bus DMA address
	stx	DMA0PORT

	lda	<dest		;Get destination address
	sta	OAMADD		;Set OARAM position
	ldx	#1		;Start channel 0 DMA
	stx	MDMAEN

	LI	;***************
	plp			;Restore processor status
	plb			;Restore data bank

	EPILOGUE		;RTL!


;--------------------------------------------------------------------
; void DmaCGram (void *source, unsigned dest, unsigned length)

	public	~~DmaCGram

	ARGSTART
	ARGD	source
	ARGW	dest
	ARGW	length

~~DmaCGram	PROLOGUE		;Put arguments in direct page

	phb			;Save data bank
	pea	0		;Set data bank to 0 for hardware regs
	plb
	plb

	php			;Save processor status
	sei			;Disable interrupts
	SI	;***************

	lda	<source		;Get source address
	sta	DMA0ADDR
	ldx	<source+2	;Get source bank
	stx	DMA0BANK
	ldx	#2		;Transfer wide bytes, incrementing
	stx	DMA0PARMS
	ldx	#(CGDATAW-$2100);B bus DMA address
	stx	DMA0PORT

	lda	<dest		;Get destination address
	sta	VMADD		;Set VRAM position
	ldx	#$80		;Increment 1 by 1 on second byte
	stx	VMAINC

	lda	<source		;Get source address
	eor	#$FFFF		;Get length to end of bank
	inc	a
	cmp	<length		;Make sure it's less than desired size
	bcc	.ok		;Skip if so

	lda	<length		;Get desired size

.ok	sta	DMA0LEN		;Set DMA length
	ldx	#1		;Start channel 0 DMA
	stx	MDMAEN

	cmp	<length		;See if transferred full amount
	bcs	.done		;Exit if so

	sec			;Calculate extra length
	sbc	<length		;Amount Transferred - Length < 0!
	eor	#$FFFF		;Negate it
	inc	a
	sta	DMA0LEN		;Set DMA length

	ldx	<source+2	;Get source bank
	inx			;Wrap to next bank
	stx	DMA0BANK	;Set DMA bank

	stz	DMA0ADDR	;Wrap around to next bank

	ldx	#1		;Start channel 0 DMA
	stx	MDMAEN

.done	LI	;***************
	plp			;Restore processor status
	plb			;Restore data bank

	EPILOGUE		;RTL!


;--------------------------------------------------------------------
; void DmaVram (void *source, unsigned dest, unsigned length)

	public	~~DmaVram

	ARGSTART
	ARGD	source
	ARGW	dest
	ARGW	length

~~DmaVram	PROLOGUE		;Put arguments in direct page

	phb			;Save data bank
	pea	0		;Set data bank to 0 for hardware regs
	plb
	plb

	php			;Save processor status
	sei			;Disable interrupts
	SI	;***************

	lda	<source		;Get source address
	sta	DMA0ADDR
	ldx	<source+2	;Get source bank
	stx	DMA0BANK
	ldx	#1		;Transfer words, incrementing address
	stx	DMA0PARMS
	ldx	#(VMDATAW-$2100);B bus DMA address
	stx	DMA0PORT

	lda	<dest		;Get destination address
	sta	VMADD		;Set VRAM position
	ldx	#$80		;Increment 1 by 1 on second byte
	stx	VMAINC

	lda	<source		;Get source address
	eor	#$FFFF		;Get length to end of bank
	inc	a
	cmp	<length		;Make sure it's less than desired size
	bcc	.ok		;Skip if so

	lda	<length		;Get desired size

.ok	sta	DMA0LEN		;Set DMA length
	ldx	#1		;Start channel 0 DMA
	stx	MDMAEN

	cmp	<length		;See if transferred full amount
	bcs	.done		;Exit if so

	sec			;Calculate extra length
	sbc	<length		;Amount Transferred - Length < 0!
	eor	#$FFFF		;Negate it
	inc	a
	sta	DMA0LEN		;Set DMA length

	ldx	<source+2	;Get source bank
	inx			;Wrap to next bank
	stx	DMA0BANK	;Set DMA bank

	stz	DMA0ADDR	;Wrap around to next bank

	ldx	#1		;Start channel 0 DMA
	stx	MDMAEN

.done	LI	;***************
	plp			;Restore processor status
	plb			;Restore data bank

	EPILOGUE		;RTL!


;--------------------------------------------------------------------
; void ClearVram (void)

	public	~~ClearVram

~~ClearVram:	SI	;***************
	phb			;Save data bank
	pea	0		;Set data bank to 0 for hardware regs
	plb
	plb
	stz	$4305		;DMA length
	lda	#<zerobyte
	sta	$4302		;Source address
	ldx	#^zerobyte
	stx	$4304		;Source bank
	ldx	#8+1
	stx	$4300		;Transfer words, non-incrementing
	ldx	#($2118-$2100)	;B bus DMA address
	stx	$4301
	stz	$2116		;VRAM position
	ldx	#$80		;Increment 1 by 1 on second byte
	stx	$2115
	ldx	#1		;Start channel 0 DMA
	stx	$420B
	plb			;Restore data bank
	LI	;***************

	rtl

;---------------------------------------

	data			;Must be in INITIALIZED data!
zerobyte	db	$00		;DMA source to clear VRAM
	code


;--------------------------------------------------------------------
; void SetupHDMA (void)

	extern	~~hdmalist
	public	~~SetupHDMA

~~SetupHDMA:	SI	;***************
	phb			;Save data bank
	pea	0		;Set data bank to 0 for hardware regs
	plb
	plb
	lda	#<~~hdmalist	;Source address
	sta	DMA1ADDR
	lda	#^~~hdmalist	;Source bank
	sta	DMA1BANK
	ldx	#0		;Transfer bytes, incrementing
	stx	DMA1PARMS
	ldx	#BGMODE		;Change mode every scan line
	stx	DMA1PORT
	plb			;Restore data bank
	LI	;***************

	rtl
;--------------------------------------------------------------------

	end
