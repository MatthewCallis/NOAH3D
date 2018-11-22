;---------------------------------------
; Title screen
;---------------------------------------

$load	title.lbm
$pic	-	8	39	0	1	1
$pic	-	8	0	0	32	25
$save	title.bin

$palette
$save	titlepal.bin

$load	title2.lbm
$dimpalette
$save	dimpal.bin

;---------------------------------------
; Briefing screen
;---------------------------------------

$load	brief.lbm
$pic	-	8	39	0	1	1
$pic	-	8	4	0	32	24
$save	brief.bin
$palette
$save	briefpal.bin


;---------------------------------------
; Playloop graphics
;---------------------------------------

$load	playback.lbm
$raw			16	16	80	112
$save	playback.bin
$palette
$save	playpal.bin
$pic	-	7	2	22	14	2
$save	m7tiles.bin

;---------------------------------------
; Font & status bar faces
;---------------------------------------

$load	psprites.lbm
$pic	-	4	2	2	16	16
$save	psprites.bin
$pic	-	4	2	19	27	1
$save statspr.bin

;---------------------------------------
; Map icons
;---------------------------------------

$load	mapicons.lbm
$pic	-	8	0	7	40	1
$pic	-	8	0	8	7	1
$save	mapicons.bin

;---------------------------------------
; Weapon sprites
;---------------------------------------

$load	weapon1.lbm
$pic	-	4	2	2	16	16
$save	weapon1.bin

$load	weapon2.lbm
$pic	-	4	2	2	16	16
$save	weapon2.bin

$load	weapon3.lbm
$pic	-	4	2	2	16	16
$save	weapon3.bin

$load	weapon4.lbm
$pic	-	4	2	2	16	16
$save	weapon4.bin

$load	weapon5.lbm
$pic	-	4	2	2	16	16
$save	weapon5.bin

$load	weapon6.lbm
$pic	-	4	2	2	16	16
$save	weapon6.bin

;---------------------------------------
; Intermission graphics
;---------------------------------------

$load	inter0.lbm
$palette
$save	interpal.bin

; "Wall" piece:

$pic	-	4	2	2	8	8

; BJ:

$pic	BJ1	4       11	2	11	11
$pic	BJ2	4       23	2	11	11
$pic	BJ3	4       11	14	11	11

$load	inter1.lbm
$pic	I_FLOOR		4       2	2	13	4
$pic	I_COMPLETE	4       2	7	17	2
$pic	I_TIME		4       29	16	9	2
$pic	I_ENEMY		4       7	10	12	2
$pic	I_PAR		4       30	19	8	2
$pic	I_TREASURE	4       2	13	17	2
$pic	I_SCORE		4       20	11	11	2
$pic	I_SECRET	4       6	16	13	2
$pic	I_BONUS		4       20	2	12	2
$pic	I_PERFECT	4       20	8	15	2
$pic	I_SPEED		4       20	5	12	2

; End stuff:

$pic	E_OF		4	20	22	4	2
$pic	E_OVERALL	4	4	22	15	2

; Chars for stats:

$pic	COLON	4	2	19	1	2
$pic	NUM0	4	3	19	2	2
$pic	NUM1	4	5	19	2	2
$pic	NUM2	4	7	19	2	2
$pic	NUM3	4	9	19	2	2
$pic	NUM4	4	11	19	2	2
$pic	NUM5	4	13	19	2	2
$pic	NUM6	4	15	19	2	2
$pic	NUM7	4	17	19	2	2
$pic	NUM8	4	19	19	2	2
$pic	NUM9	4	21	19	2	2
$pic	PERCENT	4	23	19	2	2
$pic	EXPOINT	4	25	19	1	2

$save	tiles1.bin


