## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = subdir-objects

# Initialize automake variables
AM_CPPFLAGS =

if BUILD_LUA
AM_CPPFLAGS += -I$(top_srcdir)/lua
endif

bin_PROGRAMS = explodefont gluefont make_atlas explode_atlas

explodefont_SOURCES = explodefont.c ../src/pngfuncs.c ../src/pngfuncs.h

gluefont_SOURCES = gluefont.c ../src/pngfuncs.c ../src/pngfuncs.h

make_atlas_SOURCES = make_atlas.c ../src/pngfuncs.c ../src/pngfuncs.h

explode_atlas_SOURCES = explode_atlas.c ../src/pngfuncs.c ../src/pngfuncs.h
