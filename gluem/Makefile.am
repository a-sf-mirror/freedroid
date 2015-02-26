## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = subdir-objects

# Initialize automake variables
AM_CPPFLAGS =
explodefont_LDADD =
gluefont_LDADD = -lm
make_atlas_LDADD =
explode_atlas_LDADD =

if BUILD_LUA
AM_CPPFLAGS += -I$(top_srcdir)/lua
endif

bin_PROGRAMS = explodefont gluefont make_atlas explode_atlas

if BUILD_ALL_GLUEM
make_bmchars_LDADD = -lSDL_ttf
bin_PROGRAMS += make_bmchars
endif

explodefont_SOURCES = explodefont.c codeset.c codeset.h ../src/pngfuncs.c ../src/pngfuncs.h

gluefont_SOURCES = gluefont.c codeset.c codeset.h ../src/pngfuncs.c ../src/pngfuncs.h

make_atlas_SOURCES = make_atlas.c ../src/pngfuncs.c ../src/pngfuncs.h

explode_atlas_SOURCES = explode_atlas.c ../src/pngfuncs.c ../src/pngfuncs.h

make_bmchars_SOURCES = make_bmchars.c codeset.c codeset.h ../src/pngfuncs.c ../src/pngfuncs.h
