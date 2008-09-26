#!/usr/bin/env python
#-*- encoding: utf-8 -*-
#
# extract-messages.py
# Extracts messages from an archetype file.
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

import os.path, sys, time, glob, re

#############################################################################
## POT FILE GENERATION PART
#############################################################################
class TranslationStringCatalog(object):
    def __init__(self):
        self.strings = []
        self.occurences = []

    def append(self, string, file=None, line=None):
        occurence = (file, line)
        if string in self.strings:
            index = self.strings.index(string)
            if occurence in self.occurences[index]: return
            else: self.occurences[index].append(occurence)
        else:
            self.strings.append(string)
            self.occurences.append([occurence])

    def render(self):
        def split_to_78(s):
            return [s[i:i+78] for i in xrange(0, len(s), 78)]
        result = ''
        for i,s in enumerate(self.strings):
            li = split_to_78(s)
            occurences = self.occurences[i]
            for o in occurences:
                file, line = o
                result += '#: %s:%s\n' % (file, line)
            result += 'msgid '
            if len(li) > 1: result += '""\n'
            for j,e in enumerate(li):
                result += '"' + e + '"'
                if j != len(li) - 1: result += "\n"
            result += '\nmsgstr ""\n\n'
        return result

class POTFile(object):
    def __init__(self, project, bugs_email):
        self.project = project
        self.bugs_email = bugs_email
        self.strings = TranslationStringCatalog()

    def render_to_file(self, filename):
        date_format = "%Y-%m-%d %H:%M+0000"
        date = time.strftime(date_format, time.gmtime())
        s = '''msgid ""
msgstr ""
"Project-Id-Version: %s\\n"
"Report-Msgid-Bugs-To: %s\\n"
"POT-Creation-Date: %s\\n"
"PO-Revision-Date: %s\\n"
"Last-Translator: \\n"
"Language-Team: \\n"
"MIME-Version: 1.0\\n"
"Content-Type: text/plain; charset=ISO-8859-15\\n"
"Content-Transfer-Encoding: 8bit\\n"\n\n'''% (self.project,
            self.bugs_email, date, date)
        open(filename, 'w').write(s + self.strings.render())

#############################################################################
## LINE PARSING PART
#############################################################################
regexp = re.compile(r'_"(.+?)"', re.M | re.S)
class ArchetypeFile(object):
    def __init__(self, fname):
        self.filename = fname
        self.content = open(fname, 'r').read()

    def extract_strings_to(self, potfile):
        to_translate = regexp.findall(self.content)
        contentbis = self.content
        for string in to_translate:
            index = contentbis.find('_"' + string + '"')
            line = self.content[:index].count('\n') + 1
            index += len(string) + 3
            contentbis = " " * (index) + contentbis[index:]
            potfile.strings.append(string.replace('\n', '\\n'), self.filename, line)

#############################################################################
## SCRIPT MAIN FUNCTION
#############################################################################
def main():
    if len(sys.argv) < 4:
        print "Usage: %s <input-dir> <extension> <output-file>" % sys.argv[0]
        sys.exit(1)
    input_dir, extension, output_file = sys.argv[1:]
    of = POTFile('CHANGE THIS', 'CHANGE THIS TOO')
    files = [ArchetypeFile(f)
            for f in glob.glob(os.path.join(input_dir, '*' + extension))]
    for f in files:
        f.extract_strings_to(of)
    of.render_to_file(output_file)

if __name__ == "__main__": main()