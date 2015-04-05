## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = make_atlas
if BUILD_DEV_TOOLS
bin_PROGRAMS += explode_atlas
endif

# Initialize automake variables
AM_CPPFLAGS =
if BUILD_LUA
AM_CPPFLAGS += -I$(top_srcdir)/lua
endif

make_atlas_LDADD =
explode_atlas_LDADD =
if USE_NLS
make_atlas_LDADD += @LIBINTL@
explode_atlas_LDADD += @LIBINTL@
endif

vpath %.h $(top_srcdir)/src
vpath %.c $(top_srcdir)/src

PNGDEPS = pngfuncs.c pngfuncs.h

make_atlas_SOURCES = make_atlas.c $(PNGDEPS)

explode_atlas_SOURCES = explode_atlas.c $(PNGDEPS)
