## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = subdir-objects

# Initialize automake variables
AM_CFLAGS   = @FD_CFLAGS@
AM_CPPFLAGS = @FD_CPPFLAGS@
AM_LDFLAGS  = @FD_LDFLAGS@
LIBS        = @FD_LIBS@ @LIBS@

vpath %.c $(top_srcdir)/src
vpath %.h $(top_srcdir)/src

bin_PROGRAMS = croppy

croppy_SOURCES = croppy.c
nodist_croppy_SOURCES = pngfuncs.c pngfuncs.h

EXTRA_DIST = croppy_background_fill.png
