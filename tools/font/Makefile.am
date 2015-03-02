## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = explodefont gluefont
if HAS_SDL_TTF
bin_PROGRAMS += make_bmchars
endif

# Initialize automake variables
AM_CPPFLAGS =
if BUILD_LUA
AM_CPPFLAGS += -I$(top_srcdir)/lua
endif

explodefont_LDADD =
gluefont_LDADD = -lm
make_bmchars_LDADD = -lSDL_ttf
if USE_NLS
explodefont_LDADD += @LIBINTL@
gluefont_LDADD += @LIBINTL@
make_bmchars_LDADD += @LIBINTL@
endif

CSDEPS = codeset.c codeset.h
PNGDEPS = ../../src/pngfuncs.c ../../src/pnfuncs.h

explodefont_SOURCES = explodefont.c $(CSDEPS) $(PNGDEPS)

gluefont_SOURCES = gluefont.c $(CSDEPS) $(PNGDEPS)

make_bmchars_SOURCES = make_bmchars.c $(CSDEPS) $(PNGDEPS)
