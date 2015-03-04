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
# program is not installed
noinst_PROGRAMS=fddnm

ACLOCAL_AMFLAGS= -I m4 --install

# sources to build target
fddnm_SOURCES=dialog_node.cpp \
	dialog_file.cpp \
	dialog_dot.cpp \
	fddnm.cpp \
	main.cpp

EXTRA_DIST = autogen_fddnm.sh README.fddnm INSTALL.fddnm

LDFLAGS_BOOST = \
	$(BOOST_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS) \
	$(BOOST_FILESYSTEM_LDFLAGS) \
	$(BOOST_PROGRAM_OPTIONS_LDFLAGS)

LIBS_BOOST = \
	$(BOOST_SYSTEM_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_PROGRAM_OPTIONS_LIB)

AM_CPPFLAGS = $(BOOST_CPPFLAGS)
AM_LDFLAGS = $(LDFLAGS_BOOST)
LIBS += $(LIBS_BOOST)
