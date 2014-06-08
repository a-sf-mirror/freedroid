#!/bin/bash
#########################################################################
#	Copyright (c) 2014 Scott Furry
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
#########################################################################
#
# This shell script is apart of FDRPG wiki parsing and is used in
# conjunction with wiki_parse.lua to create symbolic links between
# images in FDRPG root/graphic/ folder and the wiki upload folder
#
# Usage:
# wpImageLink.sh <path to freedroid RPG root> <path to output_images.txt> [verbose]
#	NOTE:	Do not include "output_images.txt" to the second argument
#			"output_images.txt" is the default file name
#

##################
function usage
##################
{
	echo ""
	echo "Usage:"
	echo -e "\t$0 <path to freedroid RPG root> <path to output_images.txt>"
	echo ""
	exit ${1}
}

##################
# http://www.unix.com/shell-programming-scripting/28054-error-checking-bash.html
function if_error
##################
{
	ERR_VAL=${1}
	ERR_NUM=${2}
	ERR_TEXT=${3}
	if [[ $ERR_NUM -ne $ERR_VAL ]]; then # check return code passed to function
		echo -e "ERROR: ${ERR_TEXT} To Pass: ${ERR_NUM} Rcvd: ${ERR_VAL} "
		exit $ERR_VAL
	fi
}

##
# local variables
CWD=`pwd`
SRCFILE="output_images.txt"
VERBOSITY=""

ROOTFRDRPG=""
FDRPGGRAPHICS=""
FDRPGWIKIUPLOAD=""
IMGFILE=""

##
# test argument count
if [[ $# -lt 2 || $# -gt 3 ]]; then
	echo "wrong number of arguments supplied to script"
	usage 1
else
	ROOTFRDRPG="${1}"
	FDRPGGRAPHICS="${ROOTFRDRPG}/graphics"
	FDRPGWIKIUPLOAD="${ROOTFRDRPG}/doc/wiki/uploads"
	IMGFILE="${2}/${SRCFILE}"
	if [[ $# -eq 3 ]]; then
		VERBOSITY="-v "
	fi
fi
##
# verify arguments to script
if [[ ! -d "${ROOTFRDRPG}" ]]; then
	echo "Argument for Freedroid Root folder incorrect"
	usage 1
else
	if [[ ! -d "${FDRPGGRAPHICS}" ]]; then
		echo "Argument for Freedroid Root folder incorrect"
		usage 1
	fi
fi
if [[ ! -e "${IMGFILE}" ]]; then
	echo "Argument for output_images.txt incorrect"
	usage 1
fi
##
# Test (and if necessary create) output directories
if [[ ! -d "${FDRPGWIKIUPLOAD}/Droids" ]]; then
	echo "creating directory ${FDRPGWIKIUPLOAD}/Droids"
	mkdir -p "${FDRPGWIKIUPLOAD}/Droids"
	if_error $? 0 "Create ${FDRPGWIKIUPLOAD}/Droids"
fi
if [[ ! -d "${FDRPGWIKIUPLOAD}/Items" ]]; then
	echo "creating directory ${FDRPGWIKIUPLOAD}/Items"
	mkdir -p "${FDRPGWIKIUPLOAD}/Items"
	if_error $? 0 "Create ${FDRPGWIKIUPLOAD}/Items"
fi
##
# Read in output_images.txt and process contents
cd "${ROOTFRDRPG}"
# ensure we have absolute path
ROOTFRDRPG=`pwd`
while read line
do
	while read -a VALUES ; do
		# prepend absolute path to parsed src/dest path
		SRCIMG=`echo ${VALUES[0]} | sed 's/^[\.]\(.*\)/\1/'`
		SRCIMG="${ROOTFRDRPG}${SRCIMG}"
		DESTLINK=`echo ${VALUES[1]} | sed 's/^[\.]\(.*\)/\1/'`
		DESTLINK="${ROOTFRDRPG}${DESTLINK}"
		if [[ ! -e "${SRCIMG}" ]]; then
			echo "${SRCIMG} does not exist"
			usage 1
		fi

		LINKMATCH=0
		if [[ -h "${DESTLINK}" || -f "${DESTLINK}" ]]; then
			# check existing link - matches data to be written?
			CURRENTLINK=`readlink -e ${DESTLINK}`
			if [[ "${CURRENTLINK}" == "${SRCIMG}" ]]; then
				# file exists and links are the same - move on
				LINKMATCH=1
			fi
			# has the image been updated?
			# Yes - re-update the link and force update of image
			CURRENTCHGTIME_DEST=`stat -c %Z ${DESTLINK}`
			CURRENTCHGTIME_SRC=`stat -c %Z ${SRCIMG}`
			if [[ $CURRENTCHGTIME_SRC -ge $CURRENTCHGTIME_DEST ]]; then
				LINKMATCH=0
			fi
		fi
		if [[ $LINKMATCH -eq 0 ]]; then
			# unlink if file/link exists
			if [[ -h "${DESTLINK}" ||  -f "${DESTLINK}" ]]; then
				rm ${VERBOSITY} "${DESTLINK}"
				if_error $? 0 "removing ${DESTLINK}"
			fi
			# write links
			ln -s ${VERBOSITY} "${SRCIMG}" "${DESTLINK}"
			if_error $? 0 "Creating symbolic link ${DESTLINK}"
		fi
	done <<< "${line}"
done < "${IMGFILE}"
cd "${CWD}"
exit 0
