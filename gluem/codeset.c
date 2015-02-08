/*
 *
 *  Copyright (c) 2015 Samuel Degrande
 *
 *  This file is part of Freedroid
 *
 *  Freedroid is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Freedroid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Freedroid; see the file COPYING. If not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeset.h"

static struct codeset_element ascii[] = {
		[  0]= { 1, "NULL" },
		[  1]= { 1, "START_OF_HEADING" },
		[  2]= { 1, "START_OF_TEXT" },
		[  3]= { 1, "END_OF_TEXT" },
		[  4]= { 1, "END_OF_TRANSMISSION" },
		[  5]= { 1, "ENQUIRY" },
		[  6]= { 1, "ACKNOWLEDGE" },
		[  7]= { 1, "BELL" },
		[  8]= { 1, "BACKSPACE" },
		[  9]= { 1, "CHARACTER_TABULATION" },
		[ 10]= { 1, "LINE_FEED" },
		[ 11]= { 1, "LINE_TABULATION" },
		[ 12]= { 1, "FORM_FEED" },
		[ 13]= { 1, "CARRIAGE_RETURN" },
		[ 14]= { 1, "SHIFT_OUT" },
		[ 15]= { 1, "SHIFT_IN" },
		[ 16]= { 1, "DATA_LINK_ESCAPE" },
		[ 17]= { 1, "DEVICE_CONTROL_ONE" },
		[ 18]= { 1, "DEVICE_CONTROL_TWO" },
		[ 19]= { 1, "DEVICE_CONTROL_THREE" },
		[ 20]= { 1, "DEVICE_CONTROL_FOUR" },
		[ 21]= { 1, "NEGATIVE_ACKNOWLEDGE" },
		[ 22]= { 1, "SYNCHRONOUS_IDLE" },
		[ 23]= { 1, "END_OF_TRANSMISSION_BLOCK" },
		[ 24]= { 1, "CANCEL" },
		[ 25]= { 1, "END_OF_MEDIUM" },
		[ 26]= { 1, "SUBSTITUTE" },
		[ 27]= { 1, "ESCAPE" },
		[ 28]= { 1, "INFORMATION_SEPARATOR_FOUR" },
		[ 29]= { 1, "INFORMATION_SEPARATOR_THREE" },
		[ 30]= { 1, "INFORMATION_SEPARATOR_TWO" },
		[ 31]= { 1, "INFORMATION_SEPARATOR_ONE" },
		[ 32]= { 1, "SPACE" },
		[ 33]= { 0, "EXCLAMATION_MARK" },
		[ 34]= { 0, "QUOTATION_MARK" },
		[ 35]= { 0, "NUMBER_SIGN" },
		[ 36]= { 0, "DOLLAR_SIGN" },
		[ 37]= { 0, "PERCENT_SIGN" },
		[ 38]= { 0, "AMPERSAND" },
		[ 39]= { 0, "APOSTROPHE" },
		[ 40]= { 0, "LEFT_PARENTHESIS" },
		[ 41]= { 0, "RIGHT_PARENTHESIS" },
		[ 42]= { 0, "ASTERISK" },
		[ 43]= { 0, "PLUS_SIGN" },
		[ 44]= { 0, "COMMA" },
		[ 45]= { 0, "HYPHEN_MINUS" },
		[ 46]= { 0, "FULL_STOP" },
		[ 47]= { 0, "SOLIDUS" },
		[ 48]= { 0, "DIGIT_ZERO" },
		[ 49]= { 0, "DIGIT_ONE" },
		[ 50]= { 0, "DIGIT_TWO" },
		[ 51]= { 0, "DIGIT_THREE" },
		[ 52]= { 0, "DIGIT_FOUR" },
		[ 53]= { 0, "DIGIT_FIVE" },
		[ 54]= { 0, "DIGIT_SIX" },
		[ 55]= { 0, "DIGIT_SEVEN" },
		[ 56]= { 0, "DIGIT_EIGHT" },
		[ 57]= { 0, "DIGIT_NINE" },
		[ 58]= { 0, "COLON" },
		[ 59]= { 0, "SEMICOLON" },
		[ 60]= { 0, "LESS_THAN_SIGN" },
		[ 61]= { 0, "EQUALS_SIGN" },
		[ 62]= { 0, "GREATER_THAN_SIGN" },
		[ 63]= { 0, "QUESTION_MARK" },
		[ 64]= { 0, "COMMERCIAL_AT" },
		[ 65]= { 0, "LATIN_CAPITAL_LETTER_A" },
		[ 66]= { 0, "LATIN_CAPITAL_LETTER_B" },
		[ 67]= { 0, "LATIN_CAPITAL_LETTER_C" },
		[ 68]= { 0, "LATIN_CAPITAL_LETTER_D" },
		[ 69]= { 0, "LATIN_CAPITAL_LETTER_E" },
		[ 70]= { 0, "LATIN_CAPITAL_LETTER_F" },
		[ 71]= { 0, "LATIN_CAPITAL_LETTER_G" },
		[ 72]= { 0, "LATIN_CAPITAL_LETTER_H" },
		[ 73]= { 0, "LATIN_CAPITAL_LETTER_I" },
		[ 74]= { 0, "LATIN_CAPITAL_LETTER_J" },
		[ 75]= { 0, "LATIN_CAPITAL_LETTER_K" },
		[ 76]= { 0, "LATIN_CAPITAL_LETTER_L" },
		[ 77]= { 0, "LATIN_CAPITAL_LETTER_M" },
		[ 78]= { 0, "LATIN_CAPITAL_LETTER_N" },
		[ 79]= { 0, "LATIN_CAPITAL_LETTER_O" },
		[ 80]= { 0, "LATIN_CAPITAL_LETTER_P" },
		[ 81]= { 0, "LATIN_CAPITAL_LETTER_Q" },
		[ 82]= { 0, "LATIN_CAPITAL_LETTER_R" },
		[ 83]= { 0, "LATIN_CAPITAL_LETTER_S" },
		[ 84]= { 0, "LATIN_CAPITAL_LETTER_T" },
		[ 85]= { 0, "LATIN_CAPITAL_LETTER_U" },
		[ 86]= { 0, "LATIN_CAPITAL_LETTER_V" },
		[ 87]= { 0, "LATIN_CAPITAL_LETTER_W" },
		[ 88]= { 0, "LATIN_CAPITAL_LETTER_X" },
		[ 89]= { 0, "LATIN_CAPITAL_LETTER_Y" },
		[ 90]= { 0, "LATIN_CAPITAL_LETTER_Z" },
		[ 91]= { 0, "LEFT_SQUARE_BRACKET" },
		[ 92]= { 0, "REVERSE_SOLIDUS" },
		[ 93]= { 0, "RIGHT_SQUARE_BRACKET" },
		[ 94]= { 0, "CIRCUMFLEX_ACCENT" },
		[ 95]= { 0, "LOW_LINE" },
		[ 96]= { 0, "GRAVE_ACCENT" },
		[ 97]= { 0, "LATIN_SMALL_LETTER_A" },
		[ 98]= { 0, "LATIN_SMALL_LETTER_B" },
		[ 99]= { 0, "LATIN_SMALL_LETTER_C" },
		[100]= { 0, "LATIN_SMALL_LETTER_D" },
		[101]= { 0, "LATIN_SMALL_LETTER_E" },
		[102]= { 0, "LATIN_SMALL_LETTER_F" },
		[103]= { 0, "LATIN_SMALL_LETTER_G" },
		[104]= { 0, "LATIN_SMALL_LETTER_H" },
		[105]= { 0, "LATIN_SMALL_LETTER_I" },
		[106]= { 0, "LATIN_SMALL_LETTER_J" },
		[107]= { 0, "LATIN_SMALL_LETTER_K" },
		[108]= { 0, "LATIN_SMALL_LETTER_L" },
		[109]= { 0, "LATIN_SMALL_LETTER_M" },
		[110]= { 0, "LATIN_SMALL_LETTER_N" },
		[111]= { 0, "LATIN_SMALL_LETTER_O" },
		[112]= { 0, "LATIN_SMALL_LETTER_P" },
		[113]= { 0, "LATIN_SMALL_LETTER_Q" },
		[114]= { 0, "LATIN_SMALL_LETTER_R" },
		[115]= { 0, "LATIN_SMALL_LETTER_S" },
		[116]= { 0, "LATIN_SMALL_LETTER_T" },
		[117]= { 0, "LATIN_SMALL_LETTER_U" },
		[118]= { 0, "LATIN_SMALL_LETTER_V" },
		[119]= { 0, "LATIN_SMALL_LETTER_W" },
		[120]= { 0, "LATIN_SMALL_LETTER_X" },
		[121]= { 0, "LATIN_SMALL_LETTER_Y" },
		[122]= { 0, "LATIN_SMALL_LETTER_Z" },
		[123]= { 0, "LEFT_CURLY_BRACKET" },
		[124]= { 0, "VERTICAL_LINE" },
		[125]= { 0, "RIGHT_CURLY_BRACKET" },
		[126]= { 0, "TILDE" },
		[127]= { 1, "DELETE" },
		[128]= { 1, "UNDEFINED_ONE" },
		[129]= { 1, "UNDEFINED_TWO" },
		[130]= { 1, "BREAK_PERMITTED_HERE" },
		[131]= { 1, "NO_BREAK_HERE" },
		[132]= { 1, "UNDEFINED_THREE" },
		[133]= { 1, "NEXT_LINE" },
		[134]= { 1, "START_OF_SELECTED_AREA" },
		[135]= { 1, "END_OF_SELECTED_AREA" },
		[136]= { 1, "CHARACTER_TABULATION_SET" },
		[137]= { 1, "CHARACTER_TABULATION_WITH_JUSTIFICATION" },
		[138]= { 1, "LINE_TABULATION_SET" },
		[139]= { 1, "PARTIAL_LINE_FORWARD" },
		[140]= { 1, "PARTIAL_LINE_BACKWARD" },
		[141]= { 1, "REVERSE_LINE_FEED" },
		[142]= { 1, "SINGLE_SHIFT_TWO" },
		[143]= { 1, "SINGLE_SHIFT_THREE" },
		[144]= { 1, "DEVICE_CONTROL_STRING" },
		[145]= { 1, "PRIVATE_USE_ONE" },
		[146]= { 1, "PRIVATE_USE_TWO" },
		[147]= { 1, "SET_TRANSMIT_STATE" },
		[148]= { 1, "CANCEL_CHARACTER" },
		[149]= { 1, "MESSAGE_WAITING" },
		[150]= { 1, "START_OF_GUARDED_AREA" },
		[151]= { 1, "END_OF_GUARDED_AREA" },
		[152]= { 1, "START_OF_STRING" },
		[153]= { 1, "UNDEFINED_FOUR" },
		[154]= { 1, "SINGLE_CHARACTER_INTRODUCER" },
		[155]= { 1, "CONTROL_SEQUENCE_INTRODUCER" },
		[156]= { 1, "STRING_TERMINATOR" },
		[157]= { 1, "OPERATING_SYSTEM_COMMAND" },
		[158]= { 1, "PRIVACY_MESSAGE" },
		[159]= { 1, "APPLICATION_PROGRAM_COMMAND" }
};

static struct codeset_element iso_8859_15[] = {
		[160]= { 1, "NO_BREAK_SPACE" },
		[161]= { 1, "INVERTED_EXCLAMATION_MARK" },
		[162]= { 1, "CENT_SIGN" },
		[163]= { 1, "POUND_SIGN" },
		[164]= { 1, "EURO_SIGN" },
		[165]= { 1, "YEN_SIGN" },
		[166]= { 1, "LATIN_CAPITAL_LETTER_S_WITH_CARON" },
		[167]= { 1, "SECTION_SIGN" },
		[168]= { 1, "LATIN_SMALL_LETTER_S_WITH_CARON" },
		[169]= { 1, "COPYRIGHT_SIGN" },
		[170]= { 1, "FEMININE_ORDINAL_INDICATOR" },
		[171]= { 1, "LEFT_POINTING_DOUBLE_ANGLE_QUOTATION_MARK" },
		[172]= { 1, "NOT_SIGN" },
		[173]= { 1, "SOFT_HYPHEN" },
		[174]= { 1, "REGISTERED_SIGN" },
		[175]= { 1, "MACRON" },
		[176]= { 1, "DEGREE_SIGN" },
		[177]= { 1, "PLUS_MINUS_SIGN" },
		[178]= { 1, "SUPERSCRIPT_TWO" },
		[179]= { 1, "SUPERSCRIPT_THREE" },
		[180]= { 1, "LATIN_CAPITAL_LETTER_Z_WITH_CARON" },
		[181]= { 1, "MICRO_SIGN" },
		[182]= { 1, "PILCROW_SIGN" },
		[183]= { 1, "MIDDLE_DOT" },
		[184]= { 1, "LATIN_SMALL_LETTER_Z_WITH_CARON" },
		[185]= { 1, "SUPERSCRIPT_ONE" },
		[186]= { 1, "MASCULINE_ORDINAL_INDICATOR" },
		[187]= { 1, "RIGHT_POINTING_DOUBLE_ANGLE_QUOTATION_MARK" },
		[188]= { 0, "LATIN_CAPITAL_LIGATURE_OE" },
		[189]= { 0, "LATIN_SMALL_LIGATURE_OE" },
		[190]= { 0, "LATIN_CAPITAL_LETTER_Y_WITH_DIAERESIS" },
		[191]= { 0, "INVERTED_QUESTION_MARK" },
		[192]= { 0, "LATIN_CAPITAL_LETTER_A_WITH_GRAVE" },
		[193]= { 0, "LATIN_CAPITAL_LETTER_A_WITH_ACUTE" },
		[194]= { 0, "LATIN_CAPITAL_LETTER_A_WITH_CIRCUMFLEX" },
		[195]= { 0, "LATIN_CAPITAL_LETTER_A_WITH_TILDE" },
		[196]= { 0, "LATIN_CAPITAL_LETTER_A_WITH_DIAERESIS" },
		[197]= { 0, "LATIN_CAPITAL_LETTER_A_WITH_RING_ABOVE" },
		[198]= { 0, "LATIN_CAPITAL_LETTER_AE" },
		[199]= { 0, "LATIN_CAPITAL_LETTER_C_WITH_CEDILLA" },
		[200]= { 0, "LATIN_CAPITAL_LETTER_E_WITH_GRAVE" },
		[201]= { 0, "LATIN_CAPITAL_LETTER_E_WITH_ACUTE" },
		[202]= { 0, "LATIN_CAPITAL_LETTER_E_WITH_CIRCUMFLEX" },
		[203]= { 0, "LATIN_CAPITAL_LETTER_E_WITH_DIAERESIS" },
		[204]= { 0, "LATIN_CAPITAL_LETTER_I_WITH_GRAVE" },
		[205]= { 0, "LATIN_CAPITAL_LETTER_I_WITH_ACUTE" },
		[206]= { 0, "LATIN_CAPITAL_LETTER_I_WITH_CIRCUMFLEX" },
		[207]= { 0, "LATIN_CAPITAL_LETTER_I_WITH_DIAERESIS" },
		[208]= { 0, "LATIN_CAPITAL_LETTER_ETH" },
		[209]= { 0, "LATIN_CAPITAL_LETTER_N_WITH_TILDE" },
		[210]= { 0, "LATIN_CAPITAL_LETTER_O_WITH_GRAVE" },
		[211]= { 0, "LATIN_CAPITAL_LETTER_O_WITH_ACUTE" },
		[212]= { 0, "LATIN_CAPITAL_LETTER_O_WITH_CIRCUMFLEX" },
		[213]= { 0, "LATIN_CAPITAL_LETTER_O_WITH_TILDE" },
		[214]= { 0, "LATIN_CAPITAL_LETTER_O_WITH_DIAERESIS" },
		[215]= { 0, "MULTIPLICATION_SIGN" },
		[216]= { 0, "LATIN_CAPITAL_LETTER_O_WITH_STROKE" },
		[217]= { 0, "LATIN_CAPITAL_LETTER_U_WITH_GRAVE" },
		[218]= { 0, "LATIN_CAPITAL_LETTER_U_WITH_ACUTE" },
		[219]= { 0, "LATIN_CAPITAL_LETTER_U_WITH_CIRCUMFLEX" },
		[220]= { 0, "LATIN_CAPITAL_LETTER_U_WITH_DIAERESIS" },
		[221]= { 0, "LATIN_CAPITAL_LETTER_Y_WITH_ACUTE" },
		[222]= { 0, "LATIN_CAPITAL_LETTER_THORN" },
		[223]= { 0, "LATIN_SMALL_LETTER_SHARP_S" },
		[224]= { 0, "LATIN_SMALL_LETTER_A_WITH_GRAVE" },
		[225]= { 0, "LATIN_SMALL_LETTER_A_WITH_ACUTE" },
		[226]= { 0, "LATIN_SMALL_LETTER_A_WITH_CIRCUMFLEX" },
		[227]= { 0, "LATIN_SMALL_LETTER_A_WITH_TILDE" },
		[228]= { 0, "LATIN_SMALL_LETTER_A_WITH_DIAERESIS" },
		[229]= { 0, "LATIN_SMALL_LETTER_A_WITH_RING_ABOVE" },
		[230]= { 0, "LATIN_SMALL_LETTER_AE" },
		[231]= { 0, "LATIN_SMALL_LETTER_C_WITH_CEDILLA" },
		[232]= { 0, "LATIN_SMALL_LETTER_E_WITH_GRAVE" },
		[233]= { 0, "LATIN_SMALL_LETTER_E_WITH_ACUTE" },
		[234]= { 0, "LATIN_SMALL_LETTER_E_WITH_CIRCUMFLEX" },
		[235]= { 0, "LATIN_SMALL_LETTER_E_WITH_DIAERESIS" },
		[236]= { 0, "LATIN_SMALL_LETTER_I_WITH_GRAVE" },
		[237]= { 0, "LATIN_SMALL_LETTER_I_WITH_ACUTE" },
		[238]= { 0, "LATIN_SMALL_LETTER_I_WITH_CIRCUMFLEX" },
		[239]= { 0, "LATIN_SMALL_LETTER_I_WITH_DIAERESIS" },
		[240]= { 0, "LATIN_SMALL_LETTER_ETH" },
		[241]= { 0, "LATIN_SMALL_LETTER_N_WITH_TILDE" },
		[242]= { 0, "LATIN_SMALL_LETTER_O_WITH_GRAVE" },
		[243]= { 0, "LATIN_SMALL_LETTER_O_WITH_ACUTE" },
		[244]= { 0, "LATIN_SMALL_LETTER_O_WITH_CIRCUMFLEX" },
		[245]= { 0, "LATIN_SMALL_LETTER_O_WITH_TILDE" },
		[246]= { 0, "LATIN_SMALL_LETTER_O_WITH_DIAERESIS" },
		[247]= { 0, "DIVISION_SIGN" },
		[248]= { 0, "LATIN_SMALL_LETTER_O_WITH_STROKE" },
		[249]= { 0, "LATIN_SMALL_LETTER_U_WITH_GRAVE" },
		[250]= { 0, "LATIN_SMALL_LETTER_U_WITH_ACUTE" },
		[251]= { 0, "LATIN_SMALL_LETTER_U_WITH_CIRCUMFLEX" },
		[252]= { 0, "LATIN_SMALL_LETTER_U_WITH_DIAERESIS" },
		[253]= { 0, "LATIN_SMALL_LETTER_Y_WITH_ACUTE" },
		[254]= { 0, "LATIN_SMALL_LETTER_THORN" },
		[255]= { 0, "LATIN_SMALL_LETTER_Y_WITH_DIAERESIS" }
};

static struct codeset_element iso_8859_2[] = {
		[160]= { 1, "NO_BREAK_SPACE" },
		[161]= { 0, "LATIN_CAPITAL_LETTER_A_WITH_OGONEK" },
		[162]= { 1, "BREVE" },
		[163]= { 0, "LATIN_CAPITAL_LETTER_L_WITH_STROKE" },
		[164]= { 1, "CURRENCY_SIGN" },
		[165]= { 0, "LATIN_CAPITAL_LETTER_L_WITH_CARON" },
		[166]= { 0, "LATIN_CAPITAL_LETTER_S_WITH_ACUTE" },
		[167]= { 1, "SECTION_SIGN" },
		[168]= { 1, "DIAERESIS" },
		[169]= { 0, "LATIN_CAPITAL_LETTER_S_WITH_CARON" },
		[170]= { 0, "LATIN_CAPITAL_LETTER_S_WITH_CEDILLA" },
		[171]= { 0, "LATIN_CAPITAL_LETTER_T_WITH_CARON" },
		[172]= { 0, "LATIN_CAPITAL_LETTER_Z_WITH_ACUTE" },
		[173]= { 1, "SOFT_HYPHEN" },
		[174]= { 0, "LATIN_CAPITAL_LETTER_Z_WITH_CARON" },
		[175]= { 0, "LATIN_CAPITAL_LETTER_Z_WITH_DOT_ABOVE" },
		[176]= { 1, "DEGREE_SIGN" },
		[177]= { 0, "LATIN_SMALL_LETTER_A_WITH_OGONEK" },
		[178]= { 1, "OGONEK" },
		[179]= { 0, "LATIN_SMALL_LETTER_L_WITH_STROKE" },
		[180]= { 1, "ACUTE_ACCENT" },
		[181]= { 0, "LATIN_SMALL_LETTER_L_WITH_CARON" },
		[182]= { 0, "LATIN_SMALL_LETTER_S_WITH_ACUTE" },
		[183]= { 1, "CARON" },
		[184]= { 1, "CEDILLA" },
		[185]= { 0, "LATIN_SMALL_LETTER_S_WITH_CARON" },
		[186]= { 0, "LATIN_SMALL_LETTER_S_WITH_CEDILLA" },
		[187]= { 0, "LATIN_SMALL_LETTER_T_WITH_CARON" },
		[188]= { 0, "LATIN_SMALL_LETTER_Z_WITH_ACUTE" },
		[189]= { 1, "DOUBLE_ACUTE_ACCENT" },
		[190]= { 0, "LATIN_SMALL_LETTER_Z_WITH_CARON" },
		[191]= { 0, "LATIN_SMALL_LETTER_Z_WITH_DOT_ABOVE" },
		[192]= { 0, "LATIN_CAPITAL_LETTER_R_WITH_ACUTE" },
		[193]= { 0, "LATIN_CAPITAL_LETTER_A_WITH_ACUTE" },
		[194]= { 0, "LATIN_CAPITAL_LETTER_A_WITH_CIRCUMFLEX" },
		[195]= { 0, "LATIN_CAPITAL_LETTER_A_WITH_BREVE" },
		[196]= { 0, "LATIN_CAPITAL_LETTER_A_WITH_DIAERESIS" },
		[197]= { 0, "LATIN_CAPITAL_LETTER_L_WITH_ACUTE" },
		[198]= { 0, "LATIN_CAPITAL_LETTER_C_WITH_ACUTE" },
		[199]= { 0, "LATIN_CAPITAL_LETTER_C_WITH_CEDILLA" },
		[200]= { 0, "LATIN_CAPITAL_LETTER_C_WITH_CARON" },
		[201]= { 0, "LATIN_CAPITAL_LETTER_E_WITH_ACUTE" },
		[202]= { 0, "LATIN_CAPITAL_LETTER_E_WITH_OGONEK" },
		[203]= { 0, "LATIN_CAPITAL_LETTER_E_WITH_DIAERESIS" },
		[204]= { 0, "LATIN_CAPITAL_LETTER_E_WITH_CARON" },
		[205]= { 0, "LATIN_CAPITAL_LETTER_I_WITH_ACUTE" },
		[206]= { 0, "LATIN_CAPITAL_LETTER_I_WITH_CIRCUMFLEX" },
		[207]= { 0, "LATIN_CAPITAL_LETTER_D_WITH_CARON" },
		[208]= { 0, "LATIN_CAPITAL_LETTER_D_WITH_STROKE" },
		[209]= { 0, "LATIN_CAPITAL_LETTER_N_WITH_ACUTE" },
		[210]= { 0, "LATIN_CAPITAL_LETTER_N_WITH_CARON" },
		[211]= { 0, "LATIN_CAPITAL_LETTER_O_WITH_ACUTE" },
		[212]= { 0, "LATIN_CAPITAL_LETTER_O_WITH_CIRCUMFLEX" },
		[213]= { 0, "LATIN_CAPITAL_LETTER_O_WITH_DOUBLE_ACUTE" },
		[214]= { 0, "LATIN_CAPITAL_LETTER_O_WITH_DIAERESIS" },
		[215]= { 0, "MULTIPLICATION_SIGN" },
		[216]= { 0, "LATIN_CAPITAL_LETTER_R_WITH_CARON" },
		[217]= { 0, "LATIN_CAPITAL_LETTER_U_WITH_RING_ABOVE" },
		[218]= { 0, "LATIN_CAPITAL_LETTER_U_WITH_ACUTE" },
		[219]= { 0, "LATIN_CAPITAL_LETTER_U_WITH_DOUBLE_ACUTE" },
		[220]= { 0, "LATIN_CAPITAL_LETTER_U_WITH_DIAERESIS" },
		[221]= { 0, "LATIN_CAPITAL_LETTER_Y_WITH_ACUTE" },
		[222]= { 0, "LATIN_CAPITAL_LETTER_T_WITH_CEDILLA" },
		[223]= { 0, "LATIN_SMALL_LETTER_SHARP_S" },
		[224]= { 0, "LATIN_SMALL_LETTER_R_WITH_ACUTE" },
		[225]= { 0, "LATIN_SMALL_LETTER_A_WITH_ACUTE" },
		[226]= { 0, "LATIN_SMALL_LETTER_A_WITH_CIRCUMFLEX" },
		[227]= { 0, "LATIN_SMALL_LETTER_A_WITH_BREVE" },
		[228]= { 0, "LATIN_SMALL_LETTER_A_WITH_DIAERESIS" },
		[229]= { 0, "LATIN_SMALL_LETTER_L_WITH_ACUTE" },
		[230]= { 0, "LATIN_SMALL_LETTER_C_WITH_ACUTE" },
		[231]= { 0, "LATIN_SMALL_LETTER_C_WITH_CEDILLA" },
		[232]= { 0, "LATIN_SMALL_LETTER_C_WITH_CARON" },
		[233]= { 0, "LATIN_SMALL_LETTER_E_WITH_ACUTE" },
		[234]= { 0, "LATIN_SMALL_LETTER_E_WITH_OGONEK" },
		[235]= { 0, "LATIN_SMALL_LETTER_E_WITH_DIAERESIS" },
		[236]= { 0, "LATIN_SMALL_LETTER_E_WITH_CARON" },
		[237]= { 0, "LATIN_SMALL_LETTER_I_WITH_ACUTE" },
		[238]= { 0, "LATIN_SMALL_LETTER_I_WITH_CIRCUMFLEX" },
		[239]= { 0, "LATIN_SMALL_LETTER_D_WITH_CARON" },
		[240]= { 0, "LATIN_SMALL_LETTER_D_WITH_STROKE" },
		[241]= { 0, "LATIN_SMALL_LETTER_N_WITH_ACUTE" },
		[242]= { 0, "LATIN_SMALL_LETTER_N_WITH_CARON" },
		[243]= { 0, "LATIN_SMALL_LETTER_O_WITH_ACUTE" },
		[244]= { 0, "LATIN_SMALL_LETTER_O_WITH_CIRCUMFLEX" },
		[245]= { 0, "LATIN_SMALL_LETTER_O_WITH_DOUBLE_ACUTE" },
		[246]= { 0, "LATIN_SMALL_LETTER_O_WITH_DIAERESIS" },
		[247]= { 0, "DIVISION_SIGN" },
		[248]= { 0, "LATIN_SMALL_LETTER_R_WITH_CARON" },
		[249]= { 0, "LATIN_SMALL_LETTER_U_WITH_RING_ABOVE" },
		[250]= { 0, "LATIN_SMALL_LETTER_U_WITH_ACUTE" },
		[251]= { 0, "LATIN_SMALL_LETTER_U_WITH_DOUBLE_ACUTE" },
		[252]= { 0, "LATIN_SMALL_LETTER_U_WITH_DIAERESIS" },
		[253]= { 0, "LATIN_SMALL_LETTER_Y_WITH_ACUTE" },
		[254]= { 0, "LATIN_SMALL_LETTER_T_WITH_CEDILLA" },
		[255]= { 1, "DOT_ABOVE" }
};

// List of available codesets

static struct codeset_spec {
	char *name;
	struct codeset_element *codeset;
} codeset_specs[] = {
		{ "ASCII",      ascii        },
		{ "ISO-8859-15", iso_8859_15 },
		{ "ISO-8859-2",  iso_8859_2  }
};

/*
 * Return a pointer to the codeset, whose name is 'cs'
 */
static struct codeset_element *cs_get_codeset(const char *cs)
{
	int i;
	for (i = 0; i < sizeof(codeset_specs)/sizeof(codeset_specs[0]); i++) {
		if (!strcmp(cs, codeset_specs[i].name))
			return codeset_specs[i].codeset;
	}
	return NULL;
}

/*
 * Return a string containing the list of the available codesets.
 * To be used, for instance, in the help comment of a tool.
 */
char *cs_available_codesets()
{
	static char str[2048];
	str[0] = '\0';
	int i;
	for (i = 0; i < sizeof(codeset_specs)/sizeof(codeset_specs[0]); i++) {
		strcat(str, codeset_specs[i].name);
		strcat(str, " ");
	}
	str[strlen(str)-1] = '\0';
	return str;
}

/*
 * Terminate execution if a codeset is not available
 */
void cs_check_codeset(const char *cs)
{
	int i;
	for (i = 0; i < sizeof(codeset_specs)/sizeof(codeset_specs[0]); i++) {
		if (!strcmp(cs, codeset_specs[i].name))
			return;
	}
	fprintf(stderr, "Unknown codeset %s\nAvailable codesets: %s\n", cs, cs_available_codesets());
	exit(1);
}

/*
 * Return 1 if an empty bitmap is to be used for the 8-bit 'code' of a given codeset
 */
int cs_code_is_empty(unsigned int code, const char *cs)
{
	if (code > 255) {
		fprintf(stderr, "%s: Wrong code %d\n", __FUNCTION__, code);
		exit(1);
	}

	struct codeset_element *codeset = cs_get_codeset(cs);
	if (!codeset) {
		fprintf(stderr, "%s: Unknown codeset %s\n", __FUNCTION__, cs);
		exit(1);
	}

	if (code < 160) {
		return ascii[code].empty;
	} else if (!strcmp(cs, "ASCII")) {
		return 1;
	}
	return codeset[code].empty;
}

/*
 * Return the standard name of a 8-bit 'code' of a given codeset
 */
char *cs_code_to_name(unsigned int code, const char *cs)
{
	if (code > 255) {
		fprintf(stderr, "%s: Wrong code %d\n", __FUNCTION__, code);
		exit(1);
	}

	struct codeset_element *codeset = cs_get_codeset(cs);
	if (!codeset) {
		fprintf(stderr, "%s: Unknown codeset %s\n", __FUNCTION__, cs);
		exit(1);
	}

	if (code < 160) {
		return ascii[code].name;
	}
	return codeset[code].name;
}

/*
 * Return the 8-bit 'code' of a standard name in a given codeset
 */
unsigned int cs_name_to_code(const char *name, const char *cs)
{
	int i;

	struct codeset_element *codeset = cs_get_codeset(cs);
	if (!codeset) {
		fprintf(stderr, "%s: Unknown codeset %s\n", __FUNCTION__, cs);
		exit(1);
	}

	for (i=0; i<160; i++) {
		if (!strcmp(name, ascii[i].name)) {
			return i;
		}
	}

	for (i=160; i<=255; i++) {
		if (!strcmp(name, codeset[i].name)) {
			return i;
		}
	}
	fprintf(stderr, "%s: Name %s not found in codeset %s\n", __FUNCTION__, name, cs);
	exit(1);
}

/*
 * Return a file path in the form: font_path/font_name_without_extension-standard_char_name.png
 */
char *cs_font_char_name(char *font_name, char *codeset, unsigned int code, const char *font_path)
{
	static char font_char_name[4096];

	char *font_base = strdup(basename(font_name));
	char *ptr = strchr(font_base, '.');
	if (ptr) *ptr = '\0';
	sprintf(font_char_name, "%s/%s-%s.png", font_path, font_base, cs_code_to_name(code, codeset));
	free(font_base);

	return font_char_name;
}
