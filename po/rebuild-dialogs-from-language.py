#!/usr/bin/env python
"""
This script is in the order to rebuild .dialog file from
a gettext translation contains in a .po file, of the dialogs
file of freedroidRPG. Some modifications could be able to build any
translation from a .po and an original source file in english.
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
from methods_on_po import *
def main():
    """Main script which is rebuilding the dialogs from translation."""
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
            print " le fichier %s n'existe pas dans les dialogues originaux"
    fd_original_dialog.close()

if __name__ == '__main__':main()

