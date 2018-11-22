#.silent
#.AUTODEPEND

#--------------------------
# Makefile for SNDLINK.EXE
#--------------------------

CC = c:\bc31\bin\bcc.exe
LINK = c:\bc31\bin\tlink.exe
LIBPATH = c:\bc31\lib
INCLUDEPATH = c:\bc31\include


.c.obj:
  $(CC) -c -mm {$< }


sndlink.exe: sndlink.obj cmdlib.obj scriplib.obj
  $(LINK) c0m+sndlink+cmdlib+scriplib,sndlink,,cm


sndlink.c: cmdlib.h scriplib.h

cmdlib.c: cmdlib.h

scriplib.c: scriplib.h cmdlib.h
