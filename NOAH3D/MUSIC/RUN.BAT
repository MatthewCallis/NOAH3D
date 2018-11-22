REM
REM This is just a cover batch file to hide some parameters. See SAMPLE.BAT
REM for recommended usage.
REM 
REM The /vol, /vmin, and /vmax parameters will probably go away.
REM

mid2snes %2 %3 %4 /vol:128 /vmin:65 /vmax:125 /play:1 %1
