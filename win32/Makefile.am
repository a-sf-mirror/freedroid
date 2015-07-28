## Process this file with automake to produce Makefile.in

EXTRA_DIST = freedroidRPG.rc paraicon_64x64.ico

WINDRES = $(target)-windres
freedroidRPG.coff: freedroidRPG.rc paraicon_64x64.ico
	$(WINDRES) -i freedroidRPG.rc -o freedroidRPG.coff -O COFF

