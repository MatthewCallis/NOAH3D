@echo off
maptrace 0 30 %1
l43

echo %%% COMPRESSING MAPS %%%
snescomp map00.bin bsp\map00.cmp
snescomp map01.bin bsp\map01.cmp
snescomp map02.bin bsp\map02.cmp
snescomp map03.bin bsp\map03.cmp
snescomp map04.bin bsp\map04.cmp
snescomp map05.bin bsp\map05.cmp
snescomp map06.bin bsp\map06.cmp
snescomp map07.bin bsp\map07.cmp
snescomp map08.bin bsp\map08.cmp
snescomp map09.bin bsp\map09.cmp
snescomp map10.bin bsp\map10.cmp
snescomp map11.bin bsp\map11.cmp
snescomp map12.bin bsp\map12.cmp
snescomp map13.bin bsp\map13.cmp
snescomp map14.bin bsp\map14.cmp
snescomp map15.bin bsp\map15.cmp
snescomp map16.bin bsp\map16.cmp
snescomp map17.bin bsp\map17.cmp
snescomp map18.bin bsp\map18.cmp
snescomp map19.bin bsp\map19.cmp
snescomp map20.bin bsp\map20.cmp
snescomp map21.bin bsp\map21.cmp
snescomp map22.bin bsp\map22.cmp
snescomp map23.bin bsp\map23.cmp
snescomp map24.bin bsp\map24.cmp
snescomp map25.bin bsp\map25.cmp
snescomp map26.bin bsp\map26.cmp
snescomp map27.bin bsp\map27.cmp
snescomp map28.bin bsp\map28.cmp
snescomp map29.bin bsp\map29.cmp
snescomp map30.bin bsp\map30.cmp

del map??.bin >nul

