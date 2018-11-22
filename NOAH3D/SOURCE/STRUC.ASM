*--------------------------------------------------------------------
* STRUC.ASM
*--------------------------------------------------------------------

	globals	on
	spaces	on
	llchar	.

	include	snes.equ
	include idmacros.asm

	data

;--------------------------------------------------------------------
; Fast lookup for door structurs

MAXDOORS	equ	64

dr_tilex	equ	0
dr_tiley	equ	2
dr_position	equ	4
dr_info	equ	6
dr_action	equ	8
dr_ticcount	equ	10
dr_area1	equ	12
dr_area2	equ	14
dr_strucsize	equ	16	;Not a var, just for total size

	MACLIST	ON

~~doortable:	;Offsets to each door entry

	extern	~~doors
.offset	set	0

	REPT	64

	dw	~~doors+.offset
.offset	set	.offset+dr_strucsize

	ENDREPT

;--------------------------------------------------------------------

fsegstruc:

fs_change     equ	0
fs_prev       equ	4
fs_next       equ	8
fs_texture    equ	12
fs_x1         equ	14
fs_x2         equ	16
fs_scale1     equ	18
fs_scale2     equ	20
fs_scalestep  equ	22
fs_distance   equ	24
fs_mintex     equ	26
fs_maxtex     equ	28
fs_side       equ	30
fs_strucsize  equ	32	;Not a var.!

;--------------------------------------------------------------------

vertstruc:

vt_x	equ	0
vt_y	equ	2
vt_tx	equ	4
vt_tz	equ	6
vt_tzslope	equ	8
vt_clipz	equ	10
vt_scale	equ	12
vt_flags	equ	14
vt_ipx	equ	16

;--------------------------------------------------------------------

vs_pos	equ	0
vs_columnstep	equ	4
vs_x1	equ	6
vs_x2	equ	8
vs_actornum	equ	10
vs_clipscale	equ	12
vs_strucsize	equ	14	;Not a var.!

~~visspritetable:		;Offsets in vissprite_t structures

	extern	~~vissprites
.offset	set	0

	REPT	64

	dw	~~vissprites+.offset
.offset	set	.offset+vs_strucsize

	ENDREPT

;--------------------------------------------------------------------

thingstruc:

th_x	equ	0
th_y	equ	2
th_pic	equ	4
th_areanumber	equ	6

;--------------------------------------------------------------------

patchstruc:

pa_width	equ	0
pa_pos	equ	2	;DWORD!

;--------------------------------------------------------------------

	end
