#!/usr/bin/env python
#-*- encoding: utf-8 -*-
#
# migrate-translations.py
# Migrate the translations from the .dialog files to a .po file
#
# Copyright (c) 2008 Pierre "delroth" Bourdon <root@delroth.is-a-geek.org>
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

import os, sys, os.path

def between(s, sep1, sep2):
    s1 = s[s.find(sep1)+len(sep1):]
    return s1[:s1.find(sep2)]

def main():
    if len(sys.argv) < 5:
        print "Usage: %s <input-pot> <input-dir> <out-po> <lang>" % sys.argv[0]
        return 0
    input_pot, input_dir, out_po, lang = sys.argv[1:]

    os.system("msginit -i %s -o %s -l %s" % (input_pot, out_po, lang))

    out_po_f = open(out_po, 'r')
    lines = out_po_f.readlines()
    out_po_contents = [s.strip() for s in lines]
    out_po_f.close()
    got_translation = False
    filename = ""
    linenum = 0
    for i,line in enumerate(out_po_contents):
        if line.startswith('#:') and not got_translation:
            filename_and_line = line.split(' ')[1]
            filename = os.path.basename(filename_and_line.split(':')[0])
            linenum = int(filename_and_line.split(':')[1])
            got_translation = True
        if line.startswith('msgstr ') and got_translation:
            got_translation = False
	    translated_filepath = os.path.join(input_dir, filename)
	    try:
		f = open(translated_filepath, 'r')
	    except IOError:
		os.write(2, 'Could not open file %s\n' % (translated_filepath))
		continue
	    
            in_lines = [s.strip() for s in f.readlines()]
            try:
            	translation = between(in_lines[linenum - 1], '"', '"')
	    except IndexError:
		os.write(2, 'File %s seems shorter than original, line %d\n' % (translated_filepath, linenum))
	    
            out_po_contents[i] = 'msgstr "%s"' % translation
	    f.close()
    
    out_po_f = open(out_po, 'w')
    out_po_f.write('\n'.join(out_po_contents)) 
    out_po_f.close()
    return 0

if __name__ == "__main__": sys.exit(main())
