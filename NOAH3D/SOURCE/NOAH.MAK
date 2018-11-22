#--------------------------------------------------------------------
# Makefile for SNES Super 3d Noah's Ark
#--------------------------------------------------------------------


CC = \zardoz\bin\zcc
ASM = \zardoz\bin\zas
LINK = \zardoz\bin\zln


#--------------------------------------------------------------------
# NOAH.HEX
#--------------------------------------------------------------------

NOAH = \
test.obj \
wolfasm.obj \
struc.obj \
tables.obj \
sounds.obj \
iosnes.obj \
wolfio.obj \
snesmain.obj \
intermis.obj \
newrefa.obj \
extdata.obj \
rwall.obj \
rsprite.obj \
music.obj \
intro.obj \
doors.obj \
enmove.obj \
enthink.obj \
level.obj \
missiles.obj \
plmove.obj \
plstuff.obj \
plthink.obj \
pushwall.obj \
refresh.obj \
refbsp.obj \
refspr.obj \
sight.obj \
wolfmain.obj \
statedef.obj

noah.hex:	$(NOAH) noah.lnk
	$(LINK) -F noah.lnk -L\zardoz\lib\cl

#--------------------------------------------------------------------
# ASM FILES
#--------------------------------------------------------------------

test.obj:	snes.equ idmacros.asm test.asm
	$(ASM) test.asm

wolfasm.obj:	snes.equ idmacros.asm wolfasm.asm
	$(ASM) wolfasm.asm

struc.obj: struc.asm
	$(ASM) struc.asm

iosnes.obj:	snes.equ idmacros.asm iosnes.asm
	$(ASM) iosnes.asm

newrefa.obj:	snes.equ idmacros.asm newrefa.asm
	$(ASM) newrefa.asm

extdata.obj: extdata.asm
	$(ASM) extdata.asm

rwall.obj:	snes.equ idmacros.asm notes.asm rwall.asm
	$(ASM) rwall.asm

rsprite.obj:	snes.equ idmacros.asm rsprite.asm
	$(ASM) rsprite.asm


#--------------------------------------------------------------------
# C FILES
#--------------------------------------------------------------------

tables.obj:	wolfdef.h sprites.h tables.c sounds.h states.h
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include tables.c

sounds.obj: sounds.c
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include sounds.c

wolfio.obj:	refresh.h sneswolf.h wolfdef.h sprites.h version.h extdata.h noahmisc.h iosnes.h wolfio.c sounds.h states.h
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include wolfio.c

snesmain.obj:	refresh.h sneswolf.h wolfdef.h sprites.h version.h extdata.h noahmisc.h iosnes.h snesmain.c sounds.h states.h
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include snesmain.c

intermis.obj:	sneswolf.h wolfdef.h sprites.h version.h extdata.h noahmisc.h iosnes.h intermis.c sounds.h states.h
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include intermis.c

music.obj:	refresh.h sneswolf.h wolfdef.h sprites.h version.h extdata.h noahmisc.h iosnes.h music.c sounds.h states.h
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include music.c

intro.obj:	wolfdef.h sprites.h sneswolf.h wolfdef.h sprites.h version.h extdata.h noahmisc.h iosnes.h intro.c sounds.h states.h
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include intro.c

doors.obj:	wolfdef.h sprites.h doors.c sounds.h states.h
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include doors.c

enmove.obj:	wolfdef.h sprites.h enmove.c sounds.h states.h
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include enmove.c

enthink.obj:	wolfdef.h sprites.h enthink.c sounds.h states.h
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include enthink.c

level.obj:	wolfdef.h sprites.h level.c sounds.h states.h
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include level.c

missiles.obj:	wolfdef.h sprites.h missiles.c sounds.h states.h
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include missiles.c

plmove.obj:	wolfdef.h sprites.h plmove.c sounds.h states.h
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include plmove.c

plstuff.obj:	wolfdef.h sprites.h plstuff.c sounds.h states.h
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include plstuff.c

plthink.obj:	wolfdef.h sprites.h plthink.c sounds.h states.h
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include plthink.c

pushwall.obj:	wolfdef.h sprites.h pushwall.c sounds.h states.h
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include pushwall.c

refresh.obj:	refresh.h refresh.c
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include refresh.c

refbsp.obj:	refresh.h refbsp.c
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include refbsp.c

refspr.obj:	refresh.h sprstruc.c refspr.c
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include refspr.c

sight.obj:	wolfdef.h sprites.h sight.c sounds.h states.h
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include sight.c

wolfmain.obj:	wolfdef.h sprites.h wolfmain.c sounds.h states.h
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include wolfmain.c

statedef.obj:	wolfdef.h sprites.h statedef.c sounds.h states.h
	$(CC) -D__ORCAC__ -ML -SO -WL -I c:\zardoz\include statedef.c

