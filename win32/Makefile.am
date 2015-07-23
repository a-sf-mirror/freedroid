## Process this file with automake to produce Makefile.in

EXTRA_DIST = freedroidRPG.rc w32icon_64x64.png

WINDRES = $(target)-windres
freedroidRPG.coff: freedroidRPG.rc w32icon_64x64.ico
	$(WINDRES) -i freedroidRPG.rc -o freedroidRPG.coff -O COFF

