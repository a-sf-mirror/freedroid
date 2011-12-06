#!/usr/bin/env python
#-*- encoding: utf-8 -*-
#
# Set the list of files to instal and uninstall in the win32 installer.
#
# Copyright (c) 2011 Samuel Degrande
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import os, sys
from string import Template

class MyTemplate(Template):
	delimiter = '%'

def main():

	if len(sys.argv) < 3:
		print("Usage: %s <template file> <path>" % sys.argv[0])
		sys.exit(1)
 
	template_file = open(sys.argv[1], 'r')
	path = sys.argv[2]
	install_file_list = ""
	uninstall_file_list = ""

	for filename in os.listdir(path):
		if os.path.isdir(os.path.join(path, filename)):
			continue
		install_file_list   += '\tFile "' + os.path.join(path, filename) + '"\n'
		uninstall_file_list += '\tDelete "$INSTDIR/' + filename + '"\n'

	subst_template = MyTemplate(template_file.read())
	subst_dict = {
		       'INSTALL_FILE_LIST' : install_file_list.replace("/", "\\"),
		       'UNINSTALL_FILE_LIST' : uninstall_file_list.replace("/", "\\")
		     }

	result = subst_template.safe_substitute(subst_dict)

	template_file.close()

	print result

if __name__ == '__main__': main()
