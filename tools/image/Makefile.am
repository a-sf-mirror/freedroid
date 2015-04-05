## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = croppy

vpath %.c $(top_srcdir)/src
vpath %.h $(top_srcdir)/src

croppy_SOURCES = pngfuncs.c pngfuncs.h croppy.c

EXTRA_DIST = croppy_background_fill.png
