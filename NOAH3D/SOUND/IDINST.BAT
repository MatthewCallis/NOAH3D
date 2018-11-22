@echo off

copy sound_00.bin goddbd.sns /y >nul
copy sound_01.bin goddsne.sns /y >nul
copy sound_02.bin chihat.sns /y >nul

copy sound_03.bin org.sns /y >nul
copy sound_04.bin oboe.sns /y >nul
copy sound_05.bin panflte.sns /y >nul
copy sound_06.bin trnpt.sns /y >nul
copy sound_07.bin hbass.sns /y >nul

echo	ID's loop values must be set in SNDLINK.TXT!!

REM	%%% ID LOOP VALUES %%%%
REM	goddbd- $1270
REM	goodsne- $FE0
REM	chihat- $F80
REM	org- $6D0
REM	oboe- $660
REM	panflte- $350
REM	trnpt- $4D0
REM	hbass- $1780

REM	%%% OUR LOOP VALUES %%%
REM	goddbd
REM	goodsne
REM	chihat
REM	org- 2059
REM	oboe- 5388
REM	panflte- 3008
REM	trnpt- 4022
REM	hbass- 4653
