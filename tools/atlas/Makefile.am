## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = subdir-objects

# Initialize automake variables
AM_CFLAGS   = @FD_CFLAGS@
AM_CPPFLAGS = @FD_CPPFLAGS@
AM_LDFLAGS  = @FD_LDFLAGS@
LIBS        = @FD_LIBS@ @LIBS@

vpath %.h $(top_srcdir)/src
vpath %.c $(top_srcdir)/src

bin_PROGRAMS = make_atlas explode_atlas

PNGDEPS = pngfuncs.c pngfuncs.h

make_atlas_SOURCES = make_atlas.c
nodist_make_atlas_SOURCES = $(PNGDEPS)

explode_atlas_SOURCES = explode_atlas.c
nodist_explode_atlas_SOURCES = $(PNGDEPS)