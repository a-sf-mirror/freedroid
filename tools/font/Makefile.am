## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = subdir-objects

# Initialize automake variables
AM_CFLAGS   = @FD_CFLAGS@
AM_CPPFLAGS = @FD_CPPFLAGS@
AM_LDFLAGS  = @FD_LDFLAGS@
LIBS        = @FD_LIBS@ @LIBS@

vpath %.c $(top_srcdir)/src
vpath %.h $(top_srcdir)/src


bin_PROGRAMS = explodefont gluefont
if WITH_MAKE_BMCHARS
bin_PROGRAMS += make_bmchars
endif

make_bmchars_LDADD = -lSDL_ttf

CSDEPS = codeset.c codeset.h
PNGDEPS = pngfuncs.c pngfuncs.h

explodefont_SOURCES = explodefont.c $(CSDEPS) $(PNGDEPS)

gluefont_SOURCES = gluefont.c $(CSDEPS) $(PNGDEPS)

make_bmchars_SOURCES = make_bmchars.c $(CSDEPS) $(PNGDEPS)
