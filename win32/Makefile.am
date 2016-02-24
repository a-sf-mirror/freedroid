## Process this file with automake to produce Makefile.in

EXTRA_DIST = freedroidRPG.rc freedroidRPG.ico
CLEANFILES = freedroidRPG.coff

freedroidRPG.coff: freedroidRPG.rc freedroidRPG.ico
	$(WINDRES) -i freedroidRPG.rc -o freedroidRPG.coff -O COFF

if WIN32
all: freedroidRPG.coff
endif
