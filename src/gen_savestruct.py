#!/usr/bin/env python
#-*- encoding: utf-8 -*-
#
# struct_to_io.py
# Extracts structures from a C header file and generates in/output code.
#
# Copyright (c) 2008 Pierre "delroth" Bourdon <root@delroth.is-a-geek.org>
# Copyright (c) 2009 Arthur Huillet
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

import sys, re

# Parts of regexps
c_id = r'[\w\d]+' # Standard C identifier
c_type = r'((?:(?:unsigned|signed|short|long)\s+)*' + c_id + r'(?:(?:\s+)|(?:\s*\*\s*)))' # C type

# Regexp which search for a structure
find_structure_typedef_rxp = re.compile(r'typedef struct .+?'
                                r'\{'
                                r'([^\}]+)'
                                r'\}'
                                r'\s*(' + c_id + ').*?;', re.M | re.S)
find_structure_notypedef_rxp = re.compile(
                                r'^struct (' + c_id + ').'
                                r'\{'
                                r'([^\}]+)'
                                r'\};', re.M | re.S)

# Regexp which search for a field
find_members_rxp = re.compile(r'\s*' + c_type + r'\s*(' + c_id + r')(?:\s*\[(.+)\])?\s*?;.*')

# Special types replacements
special_types = {
    #32 bit integers
    'unsigned_int': 'uint32_t',
    'unsigned_long' : 'uint32_t',
    'long' : 'int32_t',
    'int' : 'int32_t',
    #16 bit
    'short_int': 'int16_t',
    'unsigned_short_int' : 'uint16_t',
    'short': 'int16_t',
    #8 bit
    'signed_char' : 'char',
    'unsigned_char' : 'uchar'
}

only_dump_structs = [ "gps", "point", "moderately_finepoint", "finepoint", "tux_t", "item", "enemy", "bullet", "melee_shot", "mission", "configuration_for_freedroid", "npc", "upgrade_socket" ]

def main():
    if len(sys.argv) < 3:
        print "Usage: %s <input.h> <output>" % sys.argv[0]
        sys.exit(1)

    # Filenames
    inpfn, outfn = sys.argv[1:]
    inpf = open(inpfn, 'r')
    outf = open(outfn+'.c', 'w')
    outh = open(outfn+'.h', 'w')

    #Prelude
    outf.write('#include "struct.h"\n#include "global.h"\n#include "proto.h"\n#include "' + outfn + '.h"\n\n\n')

    data = {}

    header = inpf.read()
    structures = find_structure_typedef_rxp.findall(header)
    for s in find_structure_notypedef_rxp.findall(header):
        structures.append((s[1], 'struct ' + s[0]))
    for s in structures:
        # s is a tuple which contains (code, name) with code = the code inside the structure
        code, name = s

	if name not in only_dump_structs:
	    continue

        # Avoid matching things in comments
        lines = []
        in_comment_block = False
        for l in code.split('\n'):
            l = l.split('//')[0]
            if "/*" in l:
                l = l.split('/*')[0]
                lines.append(l)
                in_comment_block = True
            elif "*/" in l:
                in_comment_block = False
                l = l.split('*/')[1]
            if not in_comment_block: lines.append(l)

        # This list will contain all the (type, field, size) tuples
        a = []
        for l in lines:
            m = find_members_rxp.findall(l)
            if len(m) == 0: continue
            else: a.append(m[0])

        data[name] = []

        # Transform the raw tuple
        for f in a:
            type, field, size = f
            type = type.lower().strip()

            #print("got type " + str(type) + " field is " + str(field) + " size is " + str(size))

            # Spaces
            type = type.replace(' ', '_')
            if type in special_types.keys(): type = special_types[type]

	    if size:
		type += '[%s]' % size
            data[name].append((type, field))

    # Writing loop
    for s_name in data.keys():
        str_save = str_read = ''
        func_name = s_name.replace('struct ', '')
	header = 'int save_%s(char *, %s *);\n int read_%s(char *, char *, %s *);\n' % (func_name, s_name, func_name, s_name)
	str_save += 'int save_%s(char * tag, %s * target)\n{\nautostr_append(savestruct_autostr, "<%%s>\\n",tag);\n' % (func_name,s_name)
        str_read += '''int read_%s(char* buffer, char * tag, %s * target)\n{\n
		char search[strlen(tag) + 5];
		sprintf(search, "<%%s>", tag);
	        char * pos = strstr(buffer, search);
		if ( ! pos ) return 1;
		pos += 1 + strlen(tag);
		sprintf(search, "</%%s>", tag);
		char * epos = strstr(buffer, search);
		if ( ! epos ) return 2;
		*epos = 0;
		''' % (func_name, s_name)

        for (type, field) in data[s_name]:
            size = None
            if "[" in type:
                size = type.split('[')[1][:-1]
                elt_type = type.split('[')[0]
                type = elt_type + '_array'
            if '*' in type:
                # Pointers are not saved (it does not make sense). They are initialized to NULL during loading
		if size == None:
                    str_read += 'target->%s = NULL;\n' % field
                else:
                    str_read += 'memset(target->%s, 0, %s * sizeof(%s));\n' % (field, size, elt_type)
            else:
                str_save += 'save_%s("%s", %s(target->%s)%s);\n' % (type, field, '' if size else '&', field, (', %s' % size) if size else '')
                str_read += 'read_%s(pos, "%s", %s %s(target->%s)%s);\n' % (type, field, '', '' if size else '&', field, (', %s' % size) if size else '')

        str_save += 'autostr_append(savestruct_autostr, "</%s>\\n", tag);\nreturn 0;\n}\n\n'
        str_read += '''*epos = '>'; \nreturn 0;\n}\n\n'''
        outf.write(str_save)
        outf.write(str_read)
	outh.write(header)

if __name__ == '__main__': main()
