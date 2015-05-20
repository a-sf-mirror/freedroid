AC_DEFUN([DISPATCH_C_BLOCKS], [
#
# Allow configure to be passed a path to the directory where it should look
# for the Blocks runtime library, if any.
#
AC_ARG_WITH([blocks-runtime],
  [AS_HELP_STRING([--with-blocks-runtime],
    [Specify path to the blocks runtime])],
  [blocks_runtime=${withval}
    CBLOCKS_LDFLAGS="-L$blocks_runtime"]
)

#
# Detect compiler support for Blocks; perhaps someday -fblocks won't be
# required, in which case we'll need to change this.
#
AC_CACHE_CHECK([for C Blocks support], [dispatch_cv_cblocks], [
  saveCFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS -fblocks"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],[(void)^{int i; i = 0; }();])], [
    CFLAGS="$saveCFLAGS"
    dispatch_cv_cblocks="-fblocks"
  ], [
    CFLAGS="$saveCFLAGS"
    dispatch_cv_cblocks="no"
  ])
])

AS_IF([test "x$dispatch_cv_cblocks" != "xno"], [
    #
    # It may be necessary to directly link the Blocks runtime on some
    # systems, so give it a try if we can't link a C program that uses
    # Blocks.  We will want to remove this at somepoint, as really -fblocks
    # should force that linkage already.
    #
    saveCFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS -fblocks -O0"
    AC_MSG_CHECKING([whether additional libraries are required for the Blocks runtime])
    AC_TRY_LINK([], [
	^{ int j; j=0; }();
    ], [
	AC_MSG_RESULT([no]);
    ], [
      saveLIBS="$LIBS"
      LIBS="$LIBS -lBlocksRuntime"
      AC_TRY_LINK([], [
	^{ int k; k=0; }();
      ], [
	AC_MSG_RESULT([-lBlocksRuntime])
	CBLOCKS_LIBS="-lBlocksRuntime"
      ], [
	AC_MSG_ERROR([can't find Blocks runtime])
      ])
      LIBS="$saveLIBS"
    ])
    CFLAGS="$saveCFLAGS"
    have_cblocks=true
    CBLOCKS_CFLAGS="$dispatch_cv_cblocks"
], [
    have_cblocks=false
])
AM_CONDITIONAL(HAVE_CBLOCKS, $have_cblocks)
])
