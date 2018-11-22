*--------------------------------------------------------------------
* IDMACROS.ASM
*--------------------------------------------------------------------

	MACFIRST ON
	MACLIST	 OFF

;--------------------------------------------------------------------

LAI	MACRO			;16-bit Accumulator & Indexes
	LONGA	ON
	LONGI	ON
	REP	#$30
	ENDM

SAI	MACRO			;8-bit Acummulator & Indexes
	LONGA	OFF
	LONGI	OFF
	SEP	#$30
	ENDM

LA	MACRO			;16-bit Accumulator
	LONGA	ON
	REP	#$20
	ENDM

SA	MACRO			;8-bit Accumulator
	LONGA	OFF
	SEP	#$20
	ENDM

LI	MACRO			;16-bit Indexes
	LONGI	ON
	REP	#$10
	ENDM

SI	MACRO			;8-bit Indexes
	LONGI	OFF
	SEP	#$10
	ENDM


;====================================================================
; LOCAL VARIABLES:
;   Local variables are allocated from the STACK upon entry to the
; subroutine, BUT they are accessed from the Direct Page (DP)
; register.  All local variable used MUST be declared BEFORE
; stack-based ARGUMENTS (such as those passed from a C function)
; or the program will crash!
;====================================================================


LCLSTART	MACRO		;Start local var. definitions
LCLSIZE	SET	0
	ENDM

LCLB	MACRO	LBL		;Define BYTE local var.
LBL	SET	LCLSIZE+1	;1st is at SP+1
LCLSIZE	SET	LCLSIZE+1
	ENDM

LCLW	MACRO	LBL		;Define WORD local var.
LBL	SET	LCLSIZE+1	;1st is at SP+1
LCLSIZE	SET	LCLSIZE+2
	ENDM

LCLD	MACRO	LBL		;Define	DWORD local var.
LBL	SET	LCLSIZE+1	;1st is at SP+1
LCLSIZE	SET	LCLSIZE+4
	ENDM

	LCLSTART	;Initialize


;====================================================================
; ARGUMENTS (Stack passed)
;   Arguments are passed on the STACK but are accessed within the
; subroutine via the Direct Page (DP) register.  They MUST be
; declared AFTER all LOCAL VARIABLES (see above) or the program will
; crash!
;====================================================================

ARGSTART	MACRO		;Start argument definitions
ARGSIZE	SET	0
	ENDM

ARGB	MACRO	LBL		;Define BYTE argument
LBL	SET	ARGSIZE+4+LCLSIZE
ARGSIZE	SET	ARGSIZE+1
	ENDM

ARGW	MACRO	LBL		;Define WORD argument
LBL	SET	ARGSIZE+4+LCLSIZE
ARGSIZE	SET	ARGSIZE+2
	ENDM

ARGD	MACRO	LBL		;Define	DWORD argument
LBL	SET	ARGSIZE+4+LCLSIZE
ARGSIZE	SET	ARGSIZE+4
	ENDM

	ARGSTART	;Initialize

;====================================================================

	LONGA	ON	;Must be in LONGA mode!!!

PROLOGUE	MACRO		;Must be called at beginning of subroutine
			; after ARGSTART/ARGEND

	tsc		;Get stack pointer in accumulator
	sec		;Adjust for size of local variables
	sbc	#LCLSIZE
	tcs		;Reserve space on stack
	phd		;Save caller's direct page
	tcd		;Make direct page point to stack

	ENDM


EPILOGUE	MACRO		;Must be called at end of subroutine
			;  (includes RTL!!!)

	tay			;Save return value

	lda	<$02+LCLSIZE		;Get return addr BANK & MSB from DP
	sta	<$02+ARGSIZE+LCLSIZE	;Store in DP/STACK at end of vars.
	lda	<$01+LCLSIZE		;Get return addr LSB (& MSB!) from DP
	sta	<$01+ARGSIZE+LCLSIZE	;Store in DP/STACK at end of vars.

	pld			;Restore direct page
	tsc			;Get stack pointer in accumulator
	clc
	adc	#ARGSIZE+LCLSIZE;Skip to end of variables (last 3 bytes for RTL!)
	tcs			;Set stack pointer for RTL

	tya			;Restore return value (upper byte in X)
	rtl			;Back to calling routine

	ENDM
	

;--------------------------------------------------------------------
; End
