# Process this file with autoconf to produce a configure script.
AC_INIT(freedroidRPG, 0.13)

dnl Detect the canonical host and target build environment
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

dnl Setup for automake
AM_INIT_AUTOMAKE([tar-ustar dist-bzip2])

AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

AC_HEADER_STDC
# Checks for libraries.
AC_CHECK_LIB([m], [sin],, 
	AC_MSG_ERROR([libm not found!! 
No maths library?? What kinda crazy system is that??]))

AC_PATH_X
AC_PATH_XTRA
if test x$have_x = xyes; then
	CFLAGS="$CFLAGS $X_CFLAGS"
fi

CFLAGS="$CFLAGS -Wall"

case "$target" in
	*-*-cygwin* | *-*-mingw32*)
	CFLAGS="$CFLAGS  -DFD_DATADIR='\".\"'"
	SYS_GL_LIB=opengl32
	win32=true
	;;

	*darwin*)
	## Darwin has single-precision math-functions in here
	CFLAGS="$CFLAGS  -DFD_DATADIR='\"\$(pkgdatadir)\"'"   #avoidexpansion of $pkgdatadir ! 
	LDFLAGS="$LDFLAGS -lmx"
	macosx=true
	;;

	*bsd*)
	CFLAGS="$CFLAGS  -DFD_DATADIR='\"\$(pkgdatadir)\"'"   #avoidexpansion of $pkgdatadir ! 
	LDFLAGS="$LDFLAGS -rdynamic"
	SYS_GL_LIB=GL
	bsd=true
	;;

	*)
	CFLAGS="$CFLAGS  -DFD_DATADIR='\"\$(pkgdatadir)\"'"   #avoid expansion of $pkgdatadir !
	LDFLAGS="$LDFLAGS -rdynamic"
	SYS_GL_LIB=GL
	;;
esac

dnl Commandline options

AC_ARG_ENABLE(opengl,       [  --disable-opengl        Do not build the OpenGL support (not recommended!) [[default=auto]]], want_gl="$enableval", want_gl=yes)
AC_ARG_ENABLE(vorbis,       [  --disable-vorbis        Do not include support for OGG/Vorbis audio files (no music) [[default=auto]]], want_vorbis="$enableval", want_vorbis=yes)
AC_ARG_ENABLE(fastmath,     [  --enable-fastmath       Compile with -ffast-math [[default=yes]]], fast_math="$enableval", fast_math=yes)
AC_ARG_ENABLE(profiling,    [  --enable-profiling      Compile with profiling code [[default=no]]], use_prof="$enableval", use_prof=no)
AC_ARG_ENABLE(debug,        [  --enable-debug          Compile with debugging support [[default=no]]], use_debug="$enableval", use_debug=no)
AC_ARG_WITH(extra-warnings, [  --with-extra-warnings   Generate more warning messages when compiling [[default=no]]], more_warn="$withval", more_warn=no)
AC_ARG_ENABLE(backtrace,    [  --disable-backtrace     Disable the use of backtrace() to generate nice bug reports [[default=enabled]]], want_backtrace="$enableval", want_backtrace=yes)

if test x$use_prof = xyes; then
	CFLAGS="$CFLAGS -pg"
fi

if test x$use_debug = xyes; then
	CFLAGS="$CFLAGS -g3 -ggdb"
fi

if test x$more_warn = xyes; then
	CFLAGS="$CFLAGS -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-align -Wnested-externs -Wunreachable-code"
fi

if test x$fast_math = xyes; then
	CFLAGS="$CFLAGS -ffast-math"
fi

dnl Check for SDL
AC_MSG_NOTICE([[Checking for compulsory SDL libraries:]])

SDL_VERSION=1.2.3
AM_PATH_SDL($SDL_VERSION,
            :,
	    AC_MSG_ERROR([*** SDL version $SDL_VERSION not found! You need to install SDL to build and play FreedroidRPG. This includes the -dev packages on distributions that have them. Please refer to the README file.])
)
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

AC_CHECK_LIB([jpeg], [jpeg_start_compress],,
	AC_MSG_ERROR([libjpeg needed to run FreedroidRPG!
see http://www.ijg.org/]))

AC_CHECK_LIB([z], [compress],,
	AC_MSG_ERROR([zlib is needed to run FreedroidRPG!
see http://www.gzip.org/zlib/]))

AC_CHECK_LIB([png], [png_read_png],,
	AC_MSG_ERROR([libpng needed to run FreedroidRPG
see http://www.libpng.org/pub/png/libpng.html]))

AC_CHECK_LIB([SDL_image], [IMG_LoadJPG_RW],,
	AC_MSG_ERROR([SDL_image library needed for FreedroidRPG!
see http://www.libsdl.org/]))

AC_MSG_NOTICE([[Checking for optional SDL libraries:]])
AC_CHECK_LIB([SDL_mixer], [Mix_ChannelFinished],, AC_MSG_WARN([
--------------------------------------------------
libSDL_mixer not found!
You need the SDL_mixer library (version >= 1.2.1) if you want sound!
(see see http://www.libsdl.org/)
--> compiling without sound support
--------------------------------------------------]))

if test x$want_vorbis = xyes; then
AC_CHECK_LIB([ogg], [oggpack_read],, AC_MSG_WARN([
--------------------------------------------------
libogg not found!
You need the Ogg libs installed if you want
Freedroid to be able to play Ogg files (e.g. the Intro theme)
--------------------------------------------------]))

AC_CHECK_LIB([vorbis], [vorbis_block_init],, AC_MSG_WARN([
--------------------------------------------------
libvorbis not found!
You need the Vorbis libs installed if you want
Freedroid to be able to play Ogg files (e.g. the Intro theme)
--------------------------------------------------]))
else
AC_DEFINE(HAVE_LIBOGG,0, [Define to 1 if ogg libs were found])
AC_DEFINE(HAVE_LIBVORBIS,0, [Define to 1 if Vorbis libs were found])
fi

if test x$bsd = xtrue; then
	AC_CHECK_LIB([execinfo], [backtrace],, AC_MSG_WARN([
--------------------------------------------------
libexecinfo not found!
Backtraces can't be generated on BSD systems if libexecinfo is not present
--------------------------------------------------]))
fi

dnl , [ -logg -lvorbisenc -lvorbisfile  ]

dnl Don't Check for getopt_long
dnl just use bundled version like hello does, and gnuchess

dnl the 'new way' to check for openGL (sets 'no_gl=yes' if not found)
ACX_PTHREAD
AX_CHECK_GL

AC_MSG_CHECKING(for openGL libraries)
if test x$no_gl = xyes; then
   AC_MSG_RESULT(no)
   AC_MSG_WARN([ openGL libs not found!
----------------------------------------------------------------------
WARNING: openGL libraries/headers could not be found.
==> Therefore we are building freedroid only with SDL-graphics support. 
    No 3D acceleration will be available. 
    It is recommended to install openGL.

(if compiling on Darwin, try './configure --with-apple-opengl-framework')
----------------------------------------------------------------------
])
else
   if test x$want_gl = xyes; then
	   AC_MSG_RESULT(yes)
	   dnl for backwards compatibility with our previous check, define
	   dnl the corresponding config.h-define
	   AC_DEFINE(HAVE_LIBGL,1, [Define to 1 if openGL libs were found])
	   LIBS="$LIBS $GL_LIBS"
	   CFLAGS="$CFLAGS $GL_CFLAGS"
   else
	   AC_MSG_RESULT(disabled by request)
	   AC_MSG_WARN([ openGL disabled, but the libraries were found!
----------------------------------------------------------------------
The gaming experience would be much better with openGL rendering.
It is recommended to use openGL.
----------------------------------------------------------------------
])
   fi
fi

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/ioctl.h sys/time.h time.h \
			  unistd.h dirent.h sys/soundcard.h soundcard.h execinfo.h signal.h locale.h stdint.h libintl.h libgen.h langinfo.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_CHECK_FUNCS([alarm getcwd gettimeofday memset sqrt strstr strtok alphasort scandir nl_langinfo dirname])

if test x$want_backtrace = xyes; then
   AC_CHECK_FUNCS([backtrace])
fi

dnl send a signal to automake if we're compiling for win32
AM_CONDITIONAL(WIN32, test x$win32 = xtrue)

AM_CONDITIONAL(NONLINUX, test x$macosx = xtrue -o x$win32 = xtrue)

AC_CONFIG_FILES([Makefile src/Makefile gluem/Makefile croppy/Makefile win32/Makefile])

AC_OUTPUT
