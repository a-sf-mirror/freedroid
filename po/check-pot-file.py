#!/usr/bin/env python
"""
This script check only if all the strings in .dialog files
are in the .pot base file for translation.
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
    out_file = open('strings_are_missing.txt','w')
    Header = """This strings in the dialog files are missing in freedroidrpg_dialogs.pot
base translation file. Each string is missing is write like below :

"string is missing", original_file, at_line


"""
    out_file.write(Header)
    counter_of_missed=0
    translations_sorted = do_original_strings('freedroidrpg_dialogs.pot')
    print "%d strings in the pot files"%len(translations_sorted)
    dico_translations = make_dico_translations(translations_sorted)
    for filename in dico_translations : 
        file = open(os.pardir + os.sep + 'dialogs' + os.sep + filename , 'r')
        the_lines = file.readlines()
        original_strings = [alpha['original'] for alpha in dico_translations[filename]]
        for a_line in the_lines :
            if ('Subtitle' in a_line) or ('OptionText' in a_line):
                dial_string = get_string(a_line)
                if (not(dial_string in original_strings)) and (dial_string!=''):
                    print dial_string
                    line_number = the_lines.index(a_line)+1
                    out_file.write(dial_string + ", %s, %d\n"%(filename, line_number))
                    counter_of_missed+=1
        file.close()
    Footer="\nThis %d strings are missing in current .pot\n"%counter_of_missed
    print Footer
    out_file.write(Footer)    
    out_file.close()

if __name__ == '__main__':main()

