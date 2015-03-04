###########################################################################
#	Copyright (c) 2015 Scott Furry
#
#	This file is part of Freedroid
#
#	Freedroid is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	Freedroid is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with Freedroid; see the file COPYING. If not, write to the
#	Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#	MA  02111-1307  USA
###########################################################################

AC_INIT(fddnm, 1.0)
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR(fddnm.cpp)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR([m4])

##
# need extra macros
# ax_cxx_compile_stdcxx_11.m4

# Checks for programs.
AC_PROG_CXX
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX_11(noext,mandatory)

# Checks for libraries.
# check for boost library files
AC_MSG_NOTICE([Checking for boost libraries:])
AX_BOOST_BASE([1.55],,
	AC_MSG_ERROR([Boost Library not found - check path to boost libraries.
	Boost Library is needed to build Freedroid Dialog Node Mapping Tool!
	see www.boost.org])
)
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_PROGRAM_OPTIONS

# check for graphviz library files
AC_MSG_NOTICE([Checking for graphviz libraries:])
AC_CHECK_LIB([gvc],[gvContext],[],
	AC_MSG_ERROR([Graphviz Libraries not found - check path to libgvc
	Graphviz Libraries are needed to build Freedroid Dialog Node Mapping Tool!
	see http://www.graphviz.org/])
	
)

AC_CHECK_LIB([cgraph],[agnode],[],
	AC_MSG_ERROR([Graphviz Libraries not found - check path to libcgraph
	Graphviz Libraries are needed to build Freedroid Dialog Node Mapping Tool!
	see http://www.graphviz.org/])
)

AC_CHECK_LIB([cdt],[dtopen],[],
	AC_MSG_ERROR([Graphviz Libraries not found - check path to libcdt
	Graphviz Libraries are needed to build Freedroid Dialog Node Mapping Tool!
	see http://www.graphviz.org/])
)

case "$target" in
	*-*-cygwin* | *-*-mingw32*)
	win32=true
	;;

	*darwin*)
	macosx=true
	;;

	*)
	;;
esac

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
