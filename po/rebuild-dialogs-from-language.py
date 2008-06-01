#!/usr/bin/env python
"""
This script is in the order to rebuild .dialog file from
a gettext translation contains in a .po file, of the dialogs
file of freedroidRPG. Some modifications could be able to build any
translation from a .po and an original source file in english.
"""
############################Copyright########################################
#Copyright (c) 2008 David Kremer <david.kremer.dk@gmail.com>                 
############################License##########################################
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
#############################################################################
import os
import sys

def do_original_strings(fd_po_filename):
    translations = []
    fd_po = open(fd_po_filename, 'r')
    line = fd_po.readline()
    while line != "" :
        original_string = ''
        translation_string = ''
        filename = ''
        while not (line.startswith('#:') or line==''):
            line=fd_po.readline()
        try :
            filenames = get_filename(line)
        except ValueError :
            pass
        while not (line.startswith('msgid') or line==''):
            line = fd_po.readline()
        while not (line.startswith('msgstr') or line==''):
            original_string += get_string(line)
            line=fd_po.readline()
        
        while not (line.startswith("\n") or line==""):
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
    """This function return the biggest substring of a string between '"' 
symbols.
"""
    if ugly_line.count('"') >= 2:
        i = ugly_line.find('"') + 1
        j = ugly_line.rfind('"')
        return ugly_line[i:j]
    else:
        return ''

def sort_translations(list_translations):
    """This function return the sorted translations by filename.

There is only one file  by sentence. Sentence in multiple files give one
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
    """This function extract filenames from a freedroidrgp-xx-dialogs.po
strings in the order to classify them.
"""
    if not header_msg_line.startswith('#:'):
        raise ValueError
    else :
        filenames=[]
        nbr_files = header_msg_line.count('../dialogs/')
        for i in range(nbr_files):
            start_filename = header_msg_line.index('../dialogs/')+11
            end_filename = header_msg_line.index('.dialog')+7
            filename = header_msg_line[start_filename:end_filename]
            header_msg_line = header_msg_line[end_filename + 1:]
            filenames.append(filename)
        return filenames

def make_dico_translations(translations_sorted):
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
    """Manage command line arguments.
"""
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


def main():
    """Main script which is rebuilding the dialogs from translation.
"""
    language = get_language()
    #repertoire "../rebuilt_xx_dialogs"
    output_dialogs_path = os.pardir + os.sep + 'rebuilt_'+ language['language_code'] + '_dialogs'
    #repertoire "../dialogs"
    input_dialogs_path = os.pardir + os.sep + 'dialogs' 
    try :
        os.mkdir(output_dialogs_path)
    except OSError :
        pass
    resultat = do_original_strings(language['po_path'])
    dico_resultat = make_dico_translations(resultat)
    for key in dico_resultat :
        fd_translated_dialog = open( output_dialogs_path + os.sep + key, "w" )
        try :
            fd_original_dialog = open( input_dialogs_path + os.sep + key, "r" )
            original_strings = [ alpha['original'] for alpha in dico_resultat[key] ]
            translation_strings = [ alpha['translation'] for alpha in dico_resultat[key] ]
            for a_line in fd_original_dialog:
                my_string = get_string(a_line)
                try :
                    position = original_strings.index(my_string)
                    if translation_strings[position] != '':
                        a_line = a_line.replace(original_strings[position],translation_strings[position] )
                except ValueError :
                    pass
                fd_translated_dialog.write(a_line)
        except IOError:
            pass
    fd_original_dialog.close()

if __name__ == '__main__':main()
