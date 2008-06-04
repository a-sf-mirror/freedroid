"""
This methods are used in 'rebuild-dialogs-from-language.py' and 
'check-pot-file.py' scripts.
"""
###########################Copyright########################################
#Copyright (c) 2008 David Kremer <david.kremer.dk@gmail.com>                 
###########################License##########################################
#This program is free software; you can redistribute it and/or modify       
#it under the terms of the GNU General Public License as published by       
#the Free Software Foundation; either version 2 of the License, or          
#(at your option) any later version.                                        
#                                                                           
#This program is distributed in the hope that it will be useful,            
#but WITHOUT ANY WARRANTY; without even the implied warranty of             
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              
#GNU General Public License for more details.                               
#                                                                           
#You should have received a copy of the GNU General Public License          
#along with this program; if not, write to the Free Software                
#Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA 
############################################################################
import os
import sys

def sort_translations(list_translations):
    """This function return the sorted translations by filename.

There is only one file by sentence. Sentence in multiple files give one
sentence per file in the list object returned.
"""
    new_list_translations = []
    for this_translation in list_translations :
        for a_filename in this_translation['filenames'] :
            new_list_translations.append( 
                { 'filename' : a_filename,
                'original' : this_translation['original'],
                'translation' : this_translation['translation'] } )
    new_list_translations.sort()
    return new_list_translations

def get_filename(header_msg_line):
    """This function extract filenames a string beginning by '#:'."""
    if not header_msg_line.startswith('#:'):
        raise ValueError
    else :
        start_filename = header_msg_line.index('../dialogs/')+11
        end_filename = header_msg_line.index('.dialog')+7
        filename = header_msg_line[start_filename:end_filename]
        return filename

def do_original_strings(fd_po_filename):
    """Extract strings and translation from a .po.

'fd_po_filename' is the file name of the .po.
"""
    translations = []
    fd_po = open(fd_po_filename, 'r')
    line = fd_po.readline()
    while line != "" :
        original_string = ''
        translation_string = ''
        filenames = list()
        while (not line.startswith('#:')) and (line!=''):
            line=fd_po.readline()
        while (not line.startswith('msgid')) and (line!=''):
            try : 
                a=get_filename(line)
                filenames.append(a)
            except : 
                print 'weird line', line
            line = fd_po.readline()
        while (not line.startswith('msgstr')) and (line!=''):
            original_string += get_string(line)
            line=fd_po.readline()
        while (not line.startswith('#:')) and (line!=""):
            translation_string += get_string(line)
            line = fd_po.readline()
        a = { 'filenames' : filenames,
        'original': original_string, 
        'translation': translation_string}
        translations.append(a)
    fd_po.close()
    translations = sort_translations(translations)
    return translations

def get_string(ugly_line):
    """This function return the substring between '"' separator.

Because I use rfind, the substring extracted is the biggest possible.
Any '"' separator in this biggest substring is kept in the returned string.
"""
    if ugly_line.count('"') >= 2:
        i = ugly_line.find('"') + 1
        j = ugly_line.rfind('"')
        return ugly_line[i:j]
    else:
        return ''

def make_dico_translations(translations_sorted):
    """Return a dico on strings of the dialog's files.

The dictionnary has one entry by dialog's file, the key is
the filename. The data is a list made of dictionnaries as like :
{'original' : "original string",
'translation' : "translation string"}
"""
    files_handled = dict()
    for translation in translations_sorted :
        if files_handled.has_key(translation['filename']):
            files_handled[translation['filename']].append(
               { 'original' : translation['original'] , 
                'translation' : translation['translation'] } )
        else:
            files_handled[translation['filename']] = list()
            files_handled[translation['filename']].append(
                {'original' : translation['original'] , 
                'translation' : translation['translation'] } )
    return files_handled

def get_language():
    """Manage command line arguments."""
    if len(sys.argv) == 2:
        path_to_po = sys.argv[1] + os.sep + 'LC_MESSAGES' + os.sep + 'freedroidrpg_dialogs.po'
        if os.path.isfile(path_to_po):
            return {'po_path':path_to_po, 'language_code':sys.argv[1]}
        else:
            print 'no translation file'
            print "usage : %s xx_XX"%sys.argv[0]
            print "where xx_XX is the language code, for example fr_FR for french language"
            sys.exit(1)
    else:
        print "usage : %s xx_XX"%sys.argv[0]
        print "where xx_XX is the language code, for example fr_FR for french language"
        sys.exit(1)


