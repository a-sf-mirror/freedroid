# Process this file with autoconf to produce a configure script.
AC_INIT(freedroidRPG, 0.15+git)

dnl Detect the canonical host and target build environment
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

dnl Setup for automake
AM_INIT_AUTOMAKE([tar-ustar dist-bzip2 -Wno-portability])

AC_CONFIG_HEADERS(config.h)

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_CC_C_O
AC_PROG_MAKE_SET
XIPH_C_COMPILER_IS_CLANG
AM_PATH_PYTHON([2.6])
AM_GNU_GETTEXT_VERSION([0.18])
AM_GNU_GETTEXT([external])
AM_ICONV

AC_HEADER_STDC

dnl Commandline options

AC_ARG_ENABLE(opengl,            [  --disable-opengl            Do not build the OpenGL support (not recommended!) [[default=auto]]], want_gl="$enableval", want_gl=yes)
AC_ARG_ENABLE(vorbis,            [  --disable-vorbis            Do not include support for OGG/Vorbis audio files (no music) [[default=auto]]], want_vorbis="$enableval", want_vorbis=yes)
AC_ARG_ENABLE(fastmath,          [  --enable-fastmath           Compile with -ffast-math [[default=yes]]], fast_math="$enableval", fast_math=yes)
AC_ARG_ENABLE(debug,             [  --enable-debug              Compile with debugging support [[default=no]]], use_debug="$enableval", use_debug=no)
AC_ARG_ENABLE(native-arch,       [  --enable-native-arch        Compile with -march=native and -mtune=native [[default=no]]], arch_native="$enableval",arch_native=no)
AC_ARG_ENABLE(sanitize-address,  [  --enable-sanitize-address   Compile with -fsanitize=address [[default=no]]], sanitize_address="$enableval",sanitize_address=no)
AC_ARG_WITH(extra-warnings,      [  --with-extra-warnings       Generate more warning messages when compiling [[default=no]]], more_warn="$withval", more_warn=no)
AC_ARG_ENABLE(backtrace,         [  --disable-backtrace         Disable the use of backtrace() to generate nice bug reports [[default=enabled]]], want_backtrace="$enableval", want_backtrace=yes)
AC_ARG_WITH(embedded-lua,        [  --with-embedded-lua         Uses the Lua libraries bundled with FreedroidRPG instead of system libraries [[default=system lua]]], embedded_lua="$withval", embedded_lua=no)
AC_ARG_ENABLE(rtprof,            [  --enable-rtprof             Enable the integration of the ingame real-time profiler [[default=no]]], want_rtprof="$enableval", want_rtprof=no)
AC_ARG_ENABLE(dev-tools,         [  --enable-dev-tools          Compile the additional dev tools [[default=no]]], dev_tools="$enableval", dev_tools=no)

# Checks for libraries.
AC_CHECK_LIB([m], [sin],, 
	AC_MSG_ERROR([libm not found!! 
No maths library?? What kinda crazy system is that??]))

AC_PATH_X
AC_PATH_XTRA
if test x$have_x = xyes; then
	CFLAGS="$CFLAGS $X_CFLAGS"
fi

CFLAGS="$CFLAGS -Wall -Wno-format-zero-length -pipe"

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
	LDFLAGS="$LDFLAGS -rdynamic -Wl,--as-needed"
	SYS_GL_LIB=GL
	;;
esac

# Check of lua2dox
AC_PATH_PROG(LUADOXPATH,lua2dox,"",[$PATH])

AC_MSG_NOTICE([[Checking for compulsory SDL libraries:]])
SDL_VERSION=1.2.3
AM_PATH_SDL($SDL_VERSION,
            :,
	    AC_MSG_ERROR([*** SDL version $SDL_VERSION not found! You need to install SDL to build and play FreedroidRPG. This includes the -dev packages on distributions that have them. Please refer to the INSTALL.freedroidRPG file.])
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

AC_CHECK_LIB([SDL_gfx], [zoomSurface],,
	AC_MSG_ERROR([SDL_gfx library needed for FreedroidRPG!
Please refer to the INSTALL.freedroidRPG file]))

AC_MSG_NOTICE([[Checking for optional SDL libraries:]])
summary_sound="yes"
AC_CHECK_LIB([SDL_mixer], [Mix_ChannelFinished],, AC_MSG_WARN([
--------------------------------------------------
libSDL_mixer not found!
You need the SDL_mixer library (version >= 1.2.1) if you want sound!
(see see http://www.libsdl.org/)
--> compiling without sound support
--------------------------------------------------])
summary_sound="SDL_mixer not found")

if test x$dev_tools = xyes; then
summary_SDLttf="yes"
AC_CHECK_LIB([SDL_ttf], [TTF_RenderUNICODE_Blended],, AC_MSG_WARN([
--------------------------------------------------
libSDL_ttf not found!
You need the SDL_ttf library to compile all font tools.
Please refer to the INSTALL.freedroidRPG file.
--> compiling with some font tools disabled.
--------------------------------------------------])
summary_SDLttf="make_bmchars not built due to SDL_ttf lib missing -optional-")
fi

if test x$want_vorbis = xyes; then
AC_CHECK_LIB([ogg], [oggpack_read],, AC_MSG_WARN([
--------------------------------------------------
libogg not found!
You need the Ogg libs installed if you want
Freedroid to be able to play Ogg files (e.g. the Intro theme)
--------------------------------------------------])
summary_sound="no libogg")

AC_CHECK_LIB([vorbis], [vorbis_block_init],, AC_MSG_WARN([
--------------------------------------------------
libvorbis not found!
You need the Vorbis libs installed if you want
Freedroid to be able to play Ogg files (e.g. the Intro theme)
--------------------------------------------------])
summary_sound="no libvorbis")
else
AC_DEFINE(HAVE_LIBOGG,0, [Define to 1 if ogg libs were found])
AC_DEFINE(HAVE_LIBVORBIS,0, [Define to 1 if Vorbis libs were found])
summary_sound="disabled"
fi

summary_rtprof="no"
if test x$want_rtprof = xyes; then
summary_rtprof="yes"
AC_CHECK_LIB([rt], [clock_gettime],
  AC_DEFINE(WITH_RTPROF, 1, [Define to 1 if rt lib were found and rtprof was enabled]),
  AC_MSG_WARN([
--------------------------------------------------
librt (or clock_gettime) not found!
The ingame real-time profiler will not be compiled.
--------------------------------------------------])
  summary_rtprof="no")
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
   summary_have_gl="no"
else
   if test x$want_gl = xyes; then
	   AC_MSG_RESULT(yes)
	   dnl for backwards compatibility with our previous check, define
	   dnl the corresponding config.h-define
	   AC_DEFINE(HAVE_LIBGL, 1, [Define to 1 if openGL libs were found])
	   LIBS="$LIBS $GL_LIBS"
	   CFLAGS="$CFLAGS $GL_CFLAGS"
	   summary_have_gl="yes"
   else
	   AC_MSG_RESULT(disabled by request)
	   AC_MSG_WARN([ openGL disabled, but the libraries were found!
----------------------------------------------------------------------
The gaming experience would be much better with openGL rendering.
It is recommended to use openGL.
----------------------------------------------------------------------
])
	   summary_have_gl="disabled"
   fi
fi

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/ioctl.h sys/time.h time.h \
			  unistd.h dirent.h sys/soundcard.h soundcard.h execinfo.h signal.h locale.h stdint.h libintl.h libgen.h langinfo.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_CHECK_FUNCS([alarm getcwd gettimeofday memset sqrt strstr strtok alphasort scandir nl_langinfo dirname setenv unsetenv])

if test x$want_backtrace = xyes; then
   AC_CHECK_FUNCS([backtrace])
fi

dnl We need anonymous lambda-like functions.
dnl With GCC, it needs -fnested-functions while with CLANG it needs -fblocks
AM_CONDITIONAL(HAVE_CBLOCKS, false)
if test "x$GCC" = xyes -a "x${xiph_cv_c_compiler_clang}" = xno ; then
  AC_MSG_CHECKING(for -fnested-functions)
  AC_COMPILE_IFELSE(
    [
      AC_LANG_PROGRAM([], [void foo(void) {}; foo();])
    ],
    [
      AC_MSG_RESULT(no)
      []
    ],
    [
      AC_MSG_RESULT(required)
      [CFLAGS="$CFLAGS -fnested-functions"]
    ]
  )
elif test "x${xiph_cv_c_compiler_clang}" = xyes ; then
  DISPATCH_C_BLOCKS
fi

dnl Set build flags accordingly with asked options

if test x$arch_native = xyes; then
	CFLAGS="$CFLAGS -O3 -march=native -mtune=native"
fi

if test x$sanitize_address = xyes; then
	AC_MSG_NOTICE([Building with -fsanitize=address -fno-omit-frame-pointer. Also using embedded Lua!])
	embedded_lua=yes;
	CFLAGS="$CFLAGS -fsanitize=address -fno-omit-frame-pointer"
fi

if test x$use_debug = xyes; then
	CFLAGS="$CFLAGS -g3 -ggdb -O0"
fi

if test x$more_warn = xyes; then
	CFLAGS="$CFLAGS -Wextra -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-align -Wnested-externs -Wunreachable-code"
fi

if test x$fast_math = xyes; then
	CFLAGS="$CFLAGS -ffast-math"
fi

if test x$embedded_lua = xno; then
	KYUA_LUA()
	if test x$lua_found = xno; then
		AC_MSG_NOTICE([No system-wide Lua library found! Using embedded Lua instead])
		embedded_lua=yes;
	else
		CFLAGS="$CFLAGS $LUA_CFLAGS"
		LIBS="$LIBS $LUA_LIBS"
	fi
fi

dnl send a signal to automake if we're compiling for win32
AM_CONDITIONAL(WIN32, test x$win32 = xtrue)

AM_CONDITIONAL(NONLINUX, test x$macosx = xtrue -o x$win32 = xtrue)

AM_CONDITIONAL(CROSS_COMPILING, test x$cross_compiling = xyes)

AM_CONDITIONAL(BUILD_LUA, test x$embedded_lua = xyes)

AM_CONDITIONAL(USE_NLS, test x$USE_NLS = xyes)

AM_CONDITIONAL(HAS_SDL_TTF, test x$summary_SDLttf = xyes)

AM_CONDITIONAL(BUILD_DEV_TOOLS, test x$dev_tools = xyes)

AC_CONFIG_FILES([
	Makefile lua/Makefile src/Makefile
	tools/Makefile tools/atlas/Makefile
	graphics/obstacles/Makefile graphics/floor_tiles/Makefile
	win32/Makefile
	doc/Doxyfile doc/Makefile doc/manual/Doxyfile doc/manual/Makefile
	po/Makefile po/po-src/Makefile.in po/po-dialogs/Makefile.in po/po-data/Makefile.in po/l10n-others/Makefile])

if test x$dev_tools = xyes; then
AC_CONFIG_FILES([tools/font/Makefile tools/image/Makefile])
fi

AC_OUTPUT

actual_compiler=$CC
if test x${xiph_cv_c_compiler_clang} = xyes ; then
       actual_compiler=clang
fi

echo "
Summary:

	CFLAGS:  $CFLAGS
	LDFLAGS: $LDFLAGS
	LIBS:    $LIBS

	compiler:                                    $actual_compiler
	use embedded Lua library (not system-wide):  $embedded_lua
	compile with OpenGL support:                 $summary_have_gl
	compile with sound support:                  $summary_sound
	compile dev tools:                           $dev_tools
	embed the real-time profiler:                $summary_rtprof
	localization enabled:                        $USE_NLS"

if test x$cross_compiling = xyes; then
	echo "	cross-compiling detected, so will not build some subdirs"
fi

echo ""
