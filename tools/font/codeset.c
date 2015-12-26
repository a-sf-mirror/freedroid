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

// See ftp://ftp.unicode.org/Public/MAPPINGS/ISO8859

static struct codeset_element ascii[] = {
		[  0]= { 1, 0x0000, "NULL" },
		[  1]= { 1, 0x0001, "START_OF_HEADING" },
		[  2]= { 1, 0x0002, "START_OF_TEXT" },
		[  3]= { 1, 0x0003, "END_OF_TEXT" },
		[  4]= { 1, 0x0004, "END_OF_TRANSMISSION" },
		[  5]= { 1, 0x0005, "ENQUIRY" },
		[  6]= { 1, 0x0006, "ACKNOWLEDGE" },
		[  7]= { 1, 0x0007, "BELL" },
		[  8]= { 1, 0x0008, "BACKSPACE" },
		[  9]= { 1, 0x0009, "CHARACTER_TABULATION" },
		[ 10]= { 1, 0x000A, "LINE_FEED" },
		[ 11]= { 1, 0x000B, "LINE_TABULATION" },
		[ 12]= { 1, 0x000C, "FORM_FEED" },
		[ 13]= { 1, 0x000D, "CARRIAGE_RETURN" },
		[ 14]= { 1, 0x000E, "SHIFT_OUT" },
		[ 15]= { 1, 0x000F, "SHIFT_IN" },
		[ 16]= { 1, 0x0010, "DATA_LINK_ESCAPE" },
		[ 17]= { 1, 0x0011, "DEVICE_CONTROL_ONE" },
		[ 18]= { 1, 0x0012, "DEVICE_CONTROL_TWO" },
		[ 19]= { 1, 0x0013, "DEVICE_CONTROL_THREE" },
		[ 20]= { 1, 0x0014, "DEVICE_CONTROL_FOUR" },
		[ 21]= { 1, 0x0015, "NEGATIVE_ACKNOWLEDGE" },
		[ 22]= { 1, 0x0016, "SYNCHRONOUS_IDLE" },
		[ 23]= { 1, 0x0017, "END_OF_TRANSMISSION_BLOCK" },
		[ 24]= { 1, 0x0018, "CANCEL" },
		[ 25]= { 1, 0x0019, "END_OF_MEDIUM" },
		[ 26]= { 1, 0x001A, "SUBSTITUTE" },
		[ 27]= { 1, 0x001B, "ESCAPE" },
		[ 28]= { 1, 0x001C, "INFORMATION_SEPARATOR_FOUR" },
		[ 29]= { 1, 0x001D, "INFORMATION_SEPARATOR_THREE" },
		[ 30]= { 1, 0x001E, "INFORMATION_SEPARATOR_TWO" },
		[ 31]= { 1, 0x001F, "INFORMATION_SEPARATOR_ONE" },
		[ 32]= { 1, 0x0020, "SPACE" },
		[ 33]= { 0, 0x0021, "EXCLAMATION_MARK" },
		[ 34]= { 0, 0x0022, "QUOTATION_MARK" },
		[ 35]= { 0, 0x0023, "NUMBER_SIGN" },
		[ 36]= { 0, 0x0024, "DOLLAR_SIGN" },
		[ 37]= { 0, 0x0025, "PERCENT_SIGN" },
		[ 38]= { 0, 0x0026, "AMPERSAND" },
		[ 39]= { 0, 0x0027, "APOSTROPHE" },
		[ 40]= { 0, 0x0028, "LEFT_PARENTHESIS" },
		[ 41]= { 0, 0x0029, "RIGHT_PARENTHESIS" },
		[ 42]= { 0, 0x002A, "ASTERISK" },
		[ 43]= { 0, 0x002B, "PLUS_SIGN" },
		[ 44]= { 0, 0x002C, "COMMA" },
		[ 45]= { 0, 0x002D, "HYPHEN_MINUS" },
		[ 46]= { 0, 0x002E, "FULL_STOP" },
		[ 47]= { 0, 0x002F, "SOLIDUS" },
		[ 48]= { 0, 0x0030, "DIGIT_ZERO" },
		[ 49]= { 0, 0x0031, "DIGIT_ONE" },
		[ 50]= { 0, 0x0032, "DIGIT_TWO" },
		[ 51]= { 0, 0x0033, "DIGIT_THREE" },
		[ 52]= { 0, 0x0034, "DIGIT_FOUR" },
		[ 53]= { 0, 0x0035, "DIGIT_FIVE" },
		[ 54]= { 0, 0x0036, "DIGIT_SIX" },
		[ 55]= { 0, 0x0037, "DIGIT_SEVEN" },
		[ 56]= { 0, 0x0038, "DIGIT_EIGHT" },
		[ 57]= { 0, 0x0039, "DIGIT_NINE" },
		[ 58]= { 0, 0x003A, "COLON" },
		[ 59]= { 0, 0x003B, "SEMICOLON" },
		[ 60]= { 0, 0x003C, "LESS_THAN_SIGN" },
		[ 61]= { 0, 0x003D, "EQUALS_SIGN" },
		[ 62]= { 0, 0x003E, "GREATER_THAN_SIGN" },
		[ 63]= { 0, 0x003F, "QUESTION_MARK" },
		[ 64]= { 0, 0x0040, "COMMERCIAL_AT" },
		[ 65]= { 0, 0x0041, "LATIN_CAPITAL_LETTER_A" },
		[ 66]= { 0, 0x0042, "LATIN_CAPITAL_LETTER_B" },
		[ 67]= { 0, 0x0043, "LATIN_CAPITAL_LETTER_C" },
		[ 68]= { 0, 0x0044, "LATIN_CAPITAL_LETTER_D" },
		[ 69]= { 0, 0x0045, "LATIN_CAPITAL_LETTER_E" },
		[ 70]= { 0, 0x0046, "LATIN_CAPITAL_LETTER_F" },
		[ 71]= { 0, 0x0047, "LATIN_CAPITAL_LETTER_G" },
		[ 72]= { 0, 0x0048, "LATIN_CAPITAL_LETTER_H" },
		[ 73]= { 0, 0x0049, "LATIN_CAPITAL_LETTER_I" },
		[ 74]= { 0, 0x004A, "LATIN_CAPITAL_LETTER_J" },
		[ 75]= { 0, 0x004B, "LATIN_CAPITAL_LETTER_K" },
		[ 76]= { 0, 0x004C, "LATIN_CAPITAL_LETTER_L" },
		[ 77]= { 0, 0x004D, "LATIN_CAPITAL_LETTER_M" },
		[ 78]= { 0, 0x004E, "LATIN_CAPITAL_LETTER_N" },
		[ 79]= { 0, 0x004F, "LATIN_CAPITAL_LETTER_O" },
		[ 80]= { 0, 0x0050, "LATIN_CAPITAL_LETTER_P" },
		[ 81]= { 0, 0x0051, "LATIN_CAPITAL_LETTER_Q" },
		[ 82]= { 0, 0x0052, "LATIN_CAPITAL_LETTER_R" },
		[ 83]= { 0, 0x0053, "LATIN_CAPITAL_LETTER_S" },
		[ 84]= { 0, 0x0054, "LATIN_CAPITAL_LETTER_T" },
		[ 85]= { 0, 0x0055, "LATIN_CAPITAL_LETTER_U" },
		[ 86]= { 0, 0x0056, "LATIN_CAPITAL_LETTER_V" },
		[ 87]= { 0, 0x0057, "LATIN_CAPITAL_LETTER_W" },
		[ 88]= { 0, 0x0058, "LATIN_CAPITAL_LETTER_X" },
		[ 89]= { 0, 0x0059, "LATIN_CAPITAL_LETTER_Y" },
		[ 90]= { 0, 0x005A, "LATIN_CAPITAL_LETTER_Z" },
		[ 91]= { 0, 0x005B, "LEFT_SQUARE_BRACKET" },
		[ 92]= { 0, 0x005C, "REVERSE_SOLIDUS" },
		[ 93]= { 0, 0x005D, "RIGHT_SQUARE_BRACKET" },
		[ 94]= { 0, 0x005E, "CIRCUMFLEX_ACCENT" },
		[ 95]= { 0, 0x005F, "LOW_LINE" },
		[ 96]= { 0, 0x0060, "GRAVE_ACCENT" },
		[ 97]= { 0, 0x0061, "LATIN_SMALL_LETTER_A" },
		[ 98]= { 0, 0x0062, "LATIN_SMALL_LETTER_B" },
		[ 99]= { 0, 0x0063, "LATIN_SMALL_LETTER_C" },
		[100]= { 0, 0x0064, "LATIN_SMALL_LETTER_D" },
		[101]= { 0, 0x0065, "LATIN_SMALL_LETTER_E" },
		[102]= { 0, 0x0066, "LATIN_SMALL_LETTER_F" },
		[103]= { 0, 0x0067, "LATIN_SMALL_LETTER_G" },
		[104]= { 0, 0x0068, "LATIN_SMALL_LETTER_H" },
		[105]= { 0, 0x0069, "LATIN_SMALL_LETTER_I" },
		[106]= { 0, 0x006A, "LATIN_SMALL_LETTER_J" },
		[107]= { 0, 0x006B, "LATIN_SMALL_LETTER_K" },
		[108]= { 0, 0x006C, "LATIN_SMALL_LETTER_L" },
		[109]= { 0, 0x006D, "LATIN_SMALL_LETTER_M" },
		[110]= { 0, 0x006E, "LATIN_SMALL_LETTER_N" },
		[111]= { 0, 0x006F, "LATIN_SMALL_LETTER_O" },
		[112]= { 0, 0x0070, "LATIN_SMALL_LETTER_P" },
		[113]= { 0, 0x0071, "LATIN_SMALL_LETTER_Q" },
		[114]= { 0, 0x0072, "LATIN_SMALL_LETTER_R" },
		[115]= { 0, 0x0073, "LATIN_SMALL_LETTER_S" },
		[116]= { 0, 0x0074, "LATIN_SMALL_LETTER_T" },
		[117]= { 0, 0x0075, "LATIN_SMALL_LETTER_U" },
		[118]= { 0, 0x0076, "LATIN_SMALL_LETTER_V" },
		[119]= { 0, 0x0077, "LATIN_SMALL_LETTER_W" },
		[120]= { 0, 0x0078, "LATIN_SMALL_LETTER_X" },
		[121]= { 0, 0x0079, "LATIN_SMALL_LETTER_Y" },
		[122]= { 0, 0x007A, "LATIN_SMALL_LETTER_Z" },
		[123]= { 0, 0x007B, "LEFT_CURLY_BRACKET" },
		[124]= { 0, 0x007C, "VERTICAL_LINE" },
		[125]= { 0, 0x007D, "RIGHT_CURLY_BRACKET" },
		[126]= { 0, 0x007E, "TILDE" },
		[127]= { 1, 0x007F, "DELETE" },
		[128]= { 1, 0x0080, "UNDEFINED_ONE" },
		[129]= { 1, 0x0081, "UNDEFINED_TWO" },
		[130]= { 1, 0x0082, "BREAK_PERMITTED_HERE" },
		[131]= { 1, 0x0083, "NO_BREAK_HERE" },
		[132]= { 1, 0x0084, "UNDEFINED_THREE" },
		[133]= { 1, 0x0085, "NEXT_LINE" },
		[134]= { 1, 0x0086, "START_OF_SELECTED_AREA" },
		[135]= { 1, 0x0087, "END_OF_SELECTED_AREA" },
		[136]= { 1, 0x0088, "CHARACTER_TABULATION_SET" },
		[137]= { 1, 0x0089, "CHARACTER_TABULATION_WITH_JUSTIFICATION" },
		[138]= { 1, 0x008A, "LINE_TABULATION_SET" },
		[139]= { 1, 0x008B, "PARTIAL_LINE_FORWARD" },
		[140]= { 1, 0x008C, "PARTIAL_LINE_BACKWARD" },
		[141]= { 1, 0x008D, "REVERSE_LINE_FEED" },
		[142]= { 1, 0x008E, "SINGLE_SHIFT_TWO" },
		[143]= { 1, 0x008F, "SINGLE_SHIFT_THREE" },
		[144]= { 1, 0x0090, "DEVICE_CONTROL_STRING" },
		[145]= { 1, 0x0091, "PRIVATE_USE_ONE" },
		[146]= { 1, 0x0092, "PRIVATE_USE_TWO" },
		[147]= { 1, 0x0093, "SET_TRANSMIT_STATE" },
		[148]= { 1, 0x0094, "CANCEL_CHARACTER" },
		[149]= { 1, 0x0095, "MESSAGE_WAITING" },
		[150]= { 1, 0x0096, "START_OF_GUARDED_AREA" },
		[151]= { 1, 0x0097, "END_OF_GUARDED_AREA" },
		[152]= { 1, 0x0098, "START_OF_STRING" },
		[153]= { 1, 0x0099, "UNDEFINED_FOUR" },
		[154]= { 1, 0x009A, "SINGLE_CHARACTER_INTRODUCER" },
		[155]= { 1, 0x009B, "CONTROL_SEQUENCE_INTRODUCER" },
		[156]= { 1, 0x009C, "STRING_TERMINATOR" },
		[157]= { 1, 0x009D, "OPERATING_SYSTEM_COMMAND" },
		[158]= { 1, 0x009E, "PRIVACY_MESSAGE" },
		[159]= { 1, 0x009F, "APPLICATION_PROGRAM_COMMAND" }
};

static struct codeset_element iso_8859_15[] = {
		[160]= { 1, 0x00A0, "NO_BREAK_SPACE" },
		[161]= { 1, 0x00A1, "INVERTED_EXCLAMATION_MARK" },
		[162]= { 1, 0x00A2, "CENT_SIGN" },
		[163]= { 1, 0x00A3, "POUND_SIGN" },
		[164]= { 1, 0x20AC, "EURO_SIGN" },
		[165]= { 1, 0x00A5, "YEN_SIGN" },
		[166]= { 1, 0x0160, "LATIN_CAPITAL_LETTER_S_WITH_CARON" },
		[167]= { 1, 0x00A7, "SECTION_SIGN" },
		[168]= { 1, 0x0161, "LATIN_SMALL_LETTER_S_WITH_CARON" },
		[169]= { 1, 0x00A9, "COPYRIGHT_SIGN" },
		[170]= { 1, 0x00AA, "FEMININE_ORDINAL_INDICATOR" },
		[171]= { 1, 0x00AB, "LEFT_POINTING_DOUBLE_ANGLE_QUOTATION_MARK" },
		[172]= { 1, 0x00AC, "NOT_SIGN" },
		[173]= { 1, 0x00AD, "SOFT_HYPHEN" },
		[174]= { 1, 0x00AE, "REGISTERED_SIGN" },
		[175]= { 1, 0x00AF, "MACRON" },
		[176]= { 1, 0x00B0, "DEGREE_SIGN" },
		[177]= { 1, 0x00B1, "PLUS_MINUS_SIGN" },
		[178]= { 1, 0x00B2, "SUPERSCRIPT_TWO" },
		[179]= { 1, 0x00B3, "SUPERSCRIPT_THREE" },
		[180]= { 1, 0x017D, "LATIN_CAPITAL_LETTER_Z_WITH_CARON" },
		[181]= { 1, 0x00B5, "MICRO_SIGN" },
		[182]= { 1, 0x00B6, "PILCROW_SIGN" },
		[183]= { 1, 0x00B7, "MIDDLE_DOT" },
		[184]= { 1, 0x017E, "LATIN_SMALL_LETTER_Z_WITH_CARON" },
		[185]= { 1, 0x00B9, "SUPERSCRIPT_ONE" },
		[186]= { 1, 0x00BA, "MASCULINE_ORDINAL_INDICATOR" },
		[187]= { 1, 0x00BB, "RIGHT_POINTING_DOUBLE_ANGLE_QUOTATION_MARK" },
		[188]= { 0, 0x0152, "LATIN_CAPITAL_LIGATURE_OE" },
		[189]= { 0, 0x0153, "LATIN_SMALL_LIGATURE_OE" },
		[190]= { 0, 0x0178, "LATIN_CAPITAL_LETTER_Y_WITH_DIAERESIS" },
		[191]= { 0, 0x00BF, "INVERTED_QUESTION_MARK" },
		[192]= { 0, 0x00C0, "LATIN_CAPITAL_LETTER_A_WITH_GRAVE" },
		[193]= { 0, 0x00C1, "LATIN_CAPITAL_LETTER_A_WITH_ACUTE" },
		[194]= { 0, 0x00C2, "LATIN_CAPITAL_LETTER_A_WITH_CIRCUMFLEX" },
		[195]= { 0, 0x00C3, "LATIN_CAPITAL_LETTER_A_WITH_TILDE" },
		[196]= { 0, 0x00C4, "LATIN_CAPITAL_LETTER_A_WITH_DIAERESIS" },
		[197]= { 0, 0x00C5, "LATIN_CAPITAL_LETTER_A_WITH_RING_ABOVE" },
		[198]= { 0, 0x00C6, "LATIN_CAPITAL_LETTER_AE" },
		[199]= { 0, 0x00C7, "LATIN_CAPITAL_LETTER_C_WITH_CEDILLA" },
		[200]= { 0, 0x00C8, "LATIN_CAPITAL_LETTER_E_WITH_GRAVE" },
		[201]= { 0, 0x00C9, "LATIN_CAPITAL_LETTER_E_WITH_ACUTE" },
		[202]= { 0, 0x00CA, "LATIN_CAPITAL_LETTER_E_WITH_CIRCUMFLEX" },
		[203]= { 0, 0x00CB, "LATIN_CAPITAL_LETTER_E_WITH_DIAERESIS" },
		[204]= { 0, 0x00CC, "LATIN_CAPITAL_LETTER_I_WITH_GRAVE" },
		[205]= { 0, 0x00CD, "LATIN_CAPITAL_LETTER_I_WITH_ACUTE" },
		[206]= { 0, 0x00CE, "LATIN_CAPITAL_LETTER_I_WITH_CIRCUMFLEX" },
		[207]= { 0, 0x00CF, "LATIN_CAPITAL_LETTER_I_WITH_DIAERESIS" },
		[208]= { 0, 0x00D0, "LATIN_CAPITAL_LETTER_ETH" },
		[209]= { 0, 0x00D1, "LATIN_CAPITAL_LETTER_N_WITH_TILDE" },
		[210]= { 0, 0x00D2, "LATIN_CAPITAL_LETTER_O_WITH_GRAVE" },
		[211]= { 0, 0x00D3, "LATIN_CAPITAL_LETTER_O_WITH_ACUTE" },
		[212]= { 0, 0x00D4, "LATIN_CAPITAL_LETTER_O_WITH_CIRCUMFLEX" },
		[213]= { 0, 0x00D5, "LATIN_CAPITAL_LETTER_O_WITH_TILDE" },
		[214]= { 0, 0x00D6, "LATIN_CAPITAL_LETTER_O_WITH_DIAERESIS" },
		[215]= { 0, 0x00D7, "MULTIPLICATION_SIGN" },
		[216]= { 0, 0x00D8, "LATIN_CAPITAL_LETTER_O_WITH_STROKE" },
		[217]= { 0, 0x00D9, "LATIN_CAPITAL_LETTER_U_WITH_GRAVE" },
		[218]= { 0, 0x00DA, "LATIN_CAPITAL_LETTER_U_WITH_ACUTE" },
		[219]= { 0, 0x00DB, "LATIN_CAPITAL_LETTER_U_WITH_CIRCUMFLEX" },
		[220]= { 0, 0x00DC, "LATIN_CAPITAL_LETTER_U_WITH_DIAERESIS" },
		[221]= { 0, 0x00DD, "LATIN_CAPITAL_LETTER_Y_WITH_ACUTE" },
		[222]= { 0, 0x00DE, "LATIN_CAPITAL_LETTER_THORN" },
		[223]= { 0, 0x00DF, "LATIN_SMALL_LETTER_SHARP_S" },
		[224]= { 0, 0x00E0, "LATIN_SMALL_LETTER_A_WITH_GRAVE" },
		[225]= { 0, 0x00E1, "LATIN_SMALL_LETTER_A_WITH_ACUTE" },
		[226]= { 0, 0x00E2, "LATIN_SMALL_LETTER_A_WITH_CIRCUMFLEX" },
		[227]= { 0, 0x00E3, "LATIN_SMALL_LETTER_A_WITH_TILDE" },
		[228]= { 0, 0x00E4, "LATIN_SMALL_LETTER_A_WITH_DIAERESIS" },
		[229]= { 0, 0x00E5, "LATIN_SMALL_LETTER_A_WITH_RING_ABOVE" },
		[230]= { 0, 0x00E6, "LATIN_SMALL_LETTER_AE" },
		[231]= { 0, 0x00E7, "LATIN_SMALL_LETTER_C_WITH_CEDILLA" },
		[232]= { 0, 0x00E8, "LATIN_SMALL_LETTER_E_WITH_GRAVE" },
		[233]= { 0, 0x00E9, "LATIN_SMALL_LETTER_E_WITH_ACUTE" },
		[234]= { 0, 0x00EA, "LATIN_SMALL_LETTER_E_WITH_CIRCUMFLEX" },
		[235]= { 0, 0x00EB, "LATIN_SMALL_LETTER_E_WITH_DIAERESIS" },
		[236]= { 0, 0x00EC, "LATIN_SMALL_LETTER_I_WITH_GRAVE" },
		[237]= { 0, 0x00ED, "LATIN_SMALL_LETTER_I_WITH_ACUTE" },
		[238]= { 0, 0x00EE, "LATIN_SMALL_LETTER_I_WITH_CIRCUMFLEX" },
		[239]= { 0, 0x00EF, "LATIN_SMALL_LETTER_I_WITH_DIAERESIS" },
		[240]= { 0, 0x00F0, "LATIN_SMALL_LETTER_ETH" },
		[241]= { 0, 0x00F1, "LATIN_SMALL_LETTER_N_WITH_TILDE" },
		[242]= { 0, 0x00F2, "LATIN_SMALL_LETTER_O_WITH_GRAVE" },
		[243]= { 0, 0x00F3, "LATIN_SMALL_LETTER_O_WITH_ACUTE" },
		[244]= { 0, 0x00F4, "LATIN_SMALL_LETTER_O_WITH_CIRCUMFLEX" },
		[245]= { 0, 0x00F5, "LATIN_SMALL_LETTER_O_WITH_TILDE" },
		[246]= { 0, 0x00F6, "LATIN_SMALL_LETTER_O_WITH_DIAERESIS" },
		[247]= { 0, 0x00F7, "DIVISION_SIGN" },
		[248]= { 0, 0x00F8, "LATIN_SMALL_LETTER_O_WITH_STROKE" },
		[249]= { 0, 0x00F9, "LATIN_SMALL_LETTER_U_WITH_GRAVE" },
		[250]= { 0, 0x00FA, "LATIN_SMALL_LETTER_U_WITH_ACUTE" },
		[251]= { 0, 0x00FB, "LATIN_SMALL_LETTER_U_WITH_CIRCUMFLEX" },
		[252]= { 0, 0x00FC, "LATIN_SMALL_LETTER_U_WITH_DIAERESIS" },
		[253]= { 0, 0x00FD, "LATIN_SMALL_LETTER_Y_WITH_ACUTE" },
		[254]= { 0, 0x00FE, "LATIN_SMALL_LETTER_THORN" },
		[255]= { 0, 0x00FF, "LATIN_SMALL_LETTER_Y_WITH_DIAERESIS" }
};

static struct codeset_element iso_8859_2[] = {
		[160]= { 1, 0x00A0, "NO_BREAK_SPACE" },
		[161]= { 0, 0x0104, "LATIN_CAPITAL_LETTER_A_WITH_OGONEK" },
		[162]= { 0, 0x02D8, "BREVE" },
		[163]= { 0, 0x0141, "LATIN_CAPITAL_LETTER_L_WITH_STROKE" },
		[164]= { 1, 0x00A4, "CURRENCY_SIGN" },
		[165]= { 0, 0x013D, "LATIN_CAPITAL_LETTER_L_WITH_CARON" },
		[166]= { 0, 0x015A, "LATIN_CAPITAL_LETTER_S_WITH_ACUTE" },
		[167]= { 1, 0x00A7, "SECTION_SIGN" },
		[168]= { 0, 0x00A8, "DIAERESIS" },
		[169]= { 0, 0x0160, "LATIN_CAPITAL_LETTER_S_WITH_CARON" },
		[170]= { 0, 0x015E, "LATIN_CAPITAL_LETTER_S_WITH_CEDILLA" },
		[171]= { 0, 0x0164, "LATIN_CAPITAL_LETTER_T_WITH_CARON" },
		[172]= { 0, 0x0179, "LATIN_CAPITAL_LETTER_Z_WITH_ACUTE" },
		[173]= { 1, 0x00AD, "SOFT_HYPHEN" },
		[174]= { 0, 0x017D, "LATIN_CAPITAL_LETTER_Z_WITH_CARON" },
		[175]= { 0, 0x017B, "LATIN_CAPITAL_LETTER_Z_WITH_DOT_ABOVE" },
		[176]= { 1, 0x00B0, "DEGREE_SIGN" },
		[177]= { 0, 0x0105, "LATIN_SMALL_LETTER_A_WITH_OGONEK" },
		[178]= { 0, 0x02DB, "OGONEK" },
		[179]= { 0, 0x0142, "LATIN_SMALL_LETTER_L_WITH_STROKE" },
		[180]= { 0, 0x00B4, "ACUTE_ACCENT" },
		[181]= { 0, 0x013E, "LATIN_SMALL_LETTER_L_WITH_CARON" },
		[182]= { 0, 0x015B, "LATIN_SMALL_LETTER_S_WITH_ACUTE" },
		[183]= { 0, 0x02C7, "CARON" },
		[184]= { 0, 0x00B8, "CEDILLA" },
		[185]= { 0, 0x0161, "LATIN_SMALL_LETTER_S_WITH_CARON" },
		[186]= { 0, 0x015F, "LATIN_SMALL_LETTER_S_WITH_CEDILLA" },
		[187]= { 0, 0x0165, "LATIN_SMALL_LETTER_T_WITH_CARON" },
		[188]= { 0, 0x017A, "LATIN_SMALL_LETTER_Z_WITH_ACUTE" },
		[189]= { 0, 0x02DD, "DOUBLE_ACUTE_ACCENT" },
		[190]= { 0, 0x017E, "LATIN_SMALL_LETTER_Z_WITH_CARON" },
		[191]= { 0, 0x017C, "LATIN_SMALL_LETTER_Z_WITH_DOT_ABOVE" },
		[192]= { 0, 0x0154, "LATIN_CAPITAL_LETTER_R_WITH_ACUTE" },
		[193]= { 0, 0x00C1, "LATIN_CAPITAL_LETTER_A_WITH_ACUTE" },
		[194]= { 0, 0x00C2, "LATIN_CAPITAL_LETTER_A_WITH_CIRCUMFLEX" },
		[195]= { 0, 0x0102, "LATIN_CAPITAL_LETTER_A_WITH_BREVE" },
		[196]= { 0, 0x00C4, "LATIN_CAPITAL_LETTER_A_WITH_DIAERESIS" },
		[197]= { 0, 0x0139, "LATIN_CAPITAL_LETTER_L_WITH_ACUTE" },
		[198]= { 0, 0x0106, "LATIN_CAPITAL_LETTER_C_WITH_ACUTE" },
		[199]= { 0, 0x00C7, "LATIN_CAPITAL_LETTER_C_WITH_CEDILLA" },
		[200]= { 0, 0x010C, "LATIN_CAPITAL_LETTER_C_WITH_CARON" },
		[201]= { 0, 0x00C9, "LATIN_CAPITAL_LETTER_E_WITH_ACUTE" },
		[202]= { 0, 0x0118, "LATIN_CAPITAL_LETTER_E_WITH_OGONEK" },
		[203]= { 0, 0x00CB, "LATIN_CAPITAL_LETTER_E_WITH_DIAERESIS" },
		[204]= { 0, 0x011A, "LATIN_CAPITAL_LETTER_E_WITH_CARON" },
		[205]= { 0, 0x00CD, "LATIN_CAPITAL_LETTER_I_WITH_ACUTE" },
		[206]= { 0, 0x00CE, "LATIN_CAPITAL_LETTER_I_WITH_CIRCUMFLEX" },
		[207]= { 0, 0x010E, "LATIN_CAPITAL_LETTER_D_WITH_CARON" },
		[208]= { 0, 0x0110, "LATIN_CAPITAL_LETTER_D_WITH_STROKE" },
		[209]= { 0, 0x0143, "LATIN_CAPITAL_LETTER_N_WITH_ACUTE" },
		[210]= { 0, 0x0147, "LATIN_CAPITAL_LETTER_N_WITH_CARON" },
		[211]= { 0, 0x00D3, "LATIN_CAPITAL_LETTER_O_WITH_ACUTE" },
		[212]= { 0, 0x00D4, "LATIN_CAPITAL_LETTER_O_WITH_CIRCUMFLEX" },
		[213]= { 0, 0x0150, "LATIN_CAPITAL_LETTER_O_WITH_DOUBLE_ACUTE" },
		[214]= { 0, 0x00D6, "LATIN_CAPITAL_LETTER_O_WITH_DIAERESIS" },
		[215]= { 0, 0x00D7, "MULTIPLICATION_SIGN" },
		[216]= { 0, 0x0158, "LATIN_CAPITAL_LETTER_R_WITH_CARON" },
		[217]= { 0, 0x016E, "LATIN_CAPITAL_LETTER_U_WITH_RING_ABOVE" },
		[218]= { 0, 0x00DA, "LATIN_CAPITAL_LETTER_U_WITH_ACUTE" },
		[219]= { 0, 0x0170, "LATIN_CAPITAL_LETTER_U_WITH_DOUBLE_ACUTE" },
		[220]= { 0, 0x00DC, "LATIN_CAPITAL_LETTER_U_WITH_DIAERESIS" },
		[221]= { 0, 0x00DD, "LATIN_CAPITAL_LETTER_Y_WITH_ACUTE" },
		[222]= { 0, 0x0162, "LATIN_CAPITAL_LETTER_T_WITH_CEDILLA" },
		[223]= { 0, 0x00DF, "LATIN_SMALL_LETTER_SHARP_S" },
		[224]= { 0, 0x0155, "LATIN_SMALL_LETTER_R_WITH_ACUTE" },
		[225]= { 0, 0x00E1, "LATIN_SMALL_LETTER_A_WITH_ACUTE" },
		[226]= { 0, 0x00E2, "LATIN_SMALL_LETTER_A_WITH_CIRCUMFLEX" },
		[227]= { 0, 0x0103, "LATIN_SMALL_LETTER_A_WITH_BREVE" },
		[228]= { 0, 0x00E4, "LATIN_SMALL_LETTER_A_WITH_DIAERESIS" },
		[229]= { 0, 0x013A, "LATIN_SMALL_LETTER_L_WITH_ACUTE" },
		[230]= { 0, 0x0107, "LATIN_SMALL_LETTER_C_WITH_ACUTE" },
		[231]= { 0, 0x00E7, "LATIN_SMALL_LETTER_C_WITH_CEDILLA" },
		[232]= { 0, 0x010D, "LATIN_SMALL_LETTER_C_WITH_CARON" },
		[233]= { 0, 0x00E9, "LATIN_SMALL_LETTER_E_WITH_ACUTE" },
		[234]= { 0, 0x0119, "LATIN_SMALL_LETTER_E_WITH_OGONEK" },
		[235]= { 0, 0x00EB, "LATIN_SMALL_LETTER_E_WITH_DIAERESIS" },
		[236]= { 0, 0x011B, "LATIN_SMALL_LETTER_E_WITH_CARON" },
		[237]= { 0, 0x00ED, "LATIN_SMALL_LETTER_I_WITH_ACUTE" },
		[238]= { 0, 0x00EE, "LATIN_SMALL_LETTER_I_WITH_CIRCUMFLEX" },
		[239]= { 0, 0x010F, "LATIN_SMALL_LETTER_D_WITH_CARON" },
		[240]= { 0, 0x0111, "LATIN_SMALL_LETTER_D_WITH_STROKE" },
		[241]= { 0, 0x0144, "LATIN_SMALL_LETTER_N_WITH_ACUTE" },
		[242]= { 0, 0x0148, "LATIN_SMALL_LETTER_N_WITH_CARON" },
		[243]= { 0, 0x00F3, "LATIN_SMALL_LETTER_O_WITH_ACUTE" },
		[244]= { 0, 0x00F4, "LATIN_SMALL_LETTER_O_WITH_CIRCUMFLEX" },
		[245]= { 0, 0x0151, "LATIN_SMALL_LETTER_O_WITH_DOUBLE_ACUTE" },
		[246]= { 0, 0x00F6, "LATIN_SMALL_LETTER_O_WITH_DIAERESIS" },
		[247]= { 0, 0x00F7, "DIVISION_SIGN" },
		[248]= { 0, 0x0159, "LATIN_SMALL_LETTER_R_WITH_CARON" },
		[249]= { 0, 0x016F, "LATIN_SMALL_LETTER_U_WITH_RING_ABOVE" },
		[250]= { 0, 0x00FA, "LATIN_SMALL_LETTER_U_WITH_ACUTE" },
		[251]= { 0, 0x0171, "LATIN_SMALL_LETTER_U_WITH_DOUBLE_ACUTE" },
		[252]= { 0, 0x00FC, "LATIN_SMALL_LETTER_U_WITH_DIAERESIS" },
		[253]= { 0, 0x00FD, "LATIN_SMALL_LETTER_Y_WITH_ACUTE" },
		[254]= { 0, 0x0163, "LATIN_SMALL_LETTER_T_WITH_CEDILLA" },
		[255]= { 1, 0x02D9, "DOT_ABOVE" }
};

static struct codeset_element iso_8859_5[] = {
		[160]= { 1, 0x00A0, "NO_BREAK_SPACE" },
		[161]= { 0, 0x0401, "CYRILLIC_CAPITAL_LETTER_IO" },
		[162]= { 0, 0x0402, "CYRILLIC_CAPITAL_LETTER_DJE" },
		[163]= { 0, 0x0403, "CYRILLIC_CAPITAL_LETTER_GJE" },
		[164]= { 0, 0x0404, "CYRILLIC_CAPITAL_LETTER_UKRAINIAN_IE" },
		[165]= { 0, 0x0405, "CYRILLIC_CAPITAL_LETTER_DZE" },
		[166]= { 0, 0x0406, "CYRILLIC_CAPITAL_LETTER_BYELORUSSIAN_UKRAINIAN_I" },
		[167]= { 0, 0x0407, "CYRILLIC_CAPITAL_LETTER_YI" },
		[168]= { 0, 0x0408, "CYRILLIC_CAPITAL_LETTER_JE" },
		[169]= { 0, 0x0409, "CYRILLIC_CAPITAL_LETTER_LJE" },
		[170]= { 0, 0x040A, "CYRILLIC_CAPITAL_LETTER_NJE" },
		[171]= { 0, 0x040B, "CYRILLIC_CAPITAL_LETTER_TSHE" },
		[172]= { 0, 0x040C, "CYRILLIC_CAPITAL_LETTER_KJE" },
		[173]= { 1, 0x00AD, "SOFT_HYPHEN" },
		[174]= { 0, 0x040E, "CYRILLIC_CAPITAL_LETTER_SHORT_U" },
		[175]= { 0, 0x040F, "CYRILLIC_CAPITAL_LETTER_DZHE" },
		[176]= { 0, 0x0410, "CYRILLIC_CAPITAL_LETTER_A" },
		[177]= { 0, 0x0411, "CYRILLIC_CAPITAL_LETTER_BE" },
		[178]= { 0, 0x0412, "CYRILLIC_CAPITAL_LETTER_VE" },
		[179]= { 0, 0x0413, "CYRILLIC_CAPITAL_LETTER_GHE" },
		[180]= { 0, 0x0414, "CYRILLIC_CAPITAL_LETTER_DE" },
		[181]= { 0, 0x0415, "CYRILLIC_CAPITAL_LETTER_IE" },
		[182]= { 0, 0x0416, "CYRILLIC_CAPITAL_LETTER_ZHE" },
		[183]= { 0, 0x0417, "CYRILLIC_CAPITAL_LETTER_ZE" },
		[184]= { 0, 0x0418, "CYRILLIC_CAPITAL_LETTER_I" },
		[185]= { 0, 0x0419, "CYRILLIC_CAPITAL_LETTER_SHORT_I" },
		[186]= { 0, 0x041A, "CYRILLIC_CAPITAL_LETTER_KA" },
		[187]= { 0, 0x041B, "CYRILLIC_CAPITAL_LETTER_EL" },
		[188]= { 0, 0x041C, "CYRILLIC_CAPITAL_LETTER_EM" },
		[189]= { 0, 0x041D, "CYRILLIC_CAPITAL_LETTER_EN" },
		[190]= { 0, 0x041E, "CYRILLIC_CAPITAL_LETTER_O" },
		[191]= { 0, 0x041F, "CYRILLIC_CAPITAL_LETTER_PE" },
		[192]= { 0, 0x0420, "CYRILLIC_CAPITAL_LETTER_ER" },
		[193]= { 0, 0x0421, "CYRILLIC_CAPITAL_LETTER_ES" },
		[194]= { 0, 0x0422, "CYRILLIC_CAPITAL_LETTER_TE" },
		[195]= { 0, 0x0423, "CYRILLIC_CAPITAL_LETTER_U" },
		[196]= { 0, 0x0424, "CYRILLIC_CAPITAL_LETTER_EF" },
		[197]= { 0, 0x0425, "CYRILLIC_CAPITAL_LETTER_HA" },
		[198]= { 0, 0x0426, "CYRILLIC_CAPITAL_LETTER_TSE" },
		[199]= { 0, 0x0427, "CYRILLIC_CAPITAL_LETTER_CHE" },
		[200]= { 0, 0x0428, "CYRILLIC_CAPITAL_LETTER_SHA" },
		[201]= { 0, 0x0429, "CYRILLIC_CAPITAL_LETTER_SHCHA" },
		[202]= { 0, 0x042A, "CYRILLIC_CAPITAL_LETTER_HARD_SIGN" },
		[203]= { 0, 0x042B, "CYRILLIC_CAPITAL_LETTER_YERU" },
		[204]= { 0, 0x042C, "CYRILLIC_CAPITAL_LETTER_SOFT_SIGN" },
		[205]= { 0, 0x042D, "CYRILLIC_CAPITAL_LETTER_E" },
		[206]= { 0, 0x042E, "CYRILLIC_CAPITAL_LETTER_YU" },
		[207]= { 0, 0x042F, "CYRILLIC_CAPITAL_LETTER_YA" },
		[208]= { 0, 0x0430, "CYRILLIC_SMALL_LETTER_A" },
		[209]= { 0, 0x0431, "CYRILLIC_SMALL_LETTER_BE" },
		[210]= { 0, 0x0432, "CYRILLIC_SMALL_LETTER_VE" },
		[211]= { 0, 0x0433, "CYRILLIC_SMALL_LETTER_GHE" },
		[212]= { 0, 0x0434, "CYRILLIC_SMALL_LETTER_DE" },
		[213]= { 0, 0x0435, "CYRILLIC_SMALL_LETTER_IE" },
		[214]= { 0, 0x0436, "CYRILLIC_SMALL_LETTER_ZHE" },
		[215]= { 0, 0x0437, "CYRILLIC_SMALL_LETTER_ZE" },
		[216]= { 0, 0x0438, "CYRILLIC_SMALL_LETTER_I" },
		[217]= { 0, 0x0439, "CYRILLIC_SMALL_LETTER_SHORT_I" },
		[218]= { 0, 0x043A, "CYRILLIC_SMALL_LETTER_KA" },
		[219]= { 0, 0x043B, "CYRILLIC_SMALL_LETTER_EL" },
		[220]= { 0, 0x043C, "CYRILLIC_SMALL_LETTER_EM" },
		[221]= { 0, 0x043D, "CYRILLIC_SMALL_LETTER_EN" },
		[222]= { 0, 0x043E, "CYRILLIC_SMALL_LETTER_O" },
		[223]= { 0, 0x043F, "CYRILLIC_SMALL_LETTER_PE" },
		[224]= { 0, 0x0440, "CYRILLIC_SMALL_LETTER_ER" },
		[225]= { 0, 0x0441, "CYRILLIC_SMALL_LETTER_ES" },
		[226]= { 0, 0x0442, "CYRILLIC_SMALL_LETTER_TE" },
		[227]= { 0, 0x0443, "CYRILLIC_SMALL_LETTER_U" },
		[228]= { 0, 0x0444, "CYRILLIC_SMALL_LETTER_EF" },
		[229]= { 0, 0x0445, "CYRILLIC_SMALL_LETTER_HA" },
		[230]= { 0, 0x0446, "CYRILLIC_SMALL_LETTER_TSE" },
		[231]= { 0, 0x0447, "CYRILLIC_SMALL_LETTER_CHE" },
		[232]= { 0, 0x0448, "CYRILLIC_SMALL_LETTER_SHA" },
		[233]= { 0, 0x0449, "CYRILLIC_SMALL_LETTER_SHCHA" },
		[234]= { 0, 0x044A, "CYRILLIC_SMALL_LETTER_HARD_SIGN" },
		[235]= { 0, 0x044B, "CYRILLIC_SMALL_LETTER_YERU" },
		[236]= { 0, 0x044C, "CYRILLIC_SMALL_LETTER_SOFT_SIGN" },
		[237]= { 0, 0x044D, "CYRILLIC_SMALL_LETTER_E" },
		[238]= { 0, 0x044E, "CYRILLIC_SMALL_LETTER_YU" },
		[239]= { 0, 0x044F, "CYRILLIC_SMALL_LETTER_YA" },
		[240]= { 0, 0x2116, "NUMERO_SIGN" },
		[241]= { 0, 0x0451, "CYRILLIC_SMALL_LETTER_IO" },
		[242]= { 0, 0x0452, "CYRILLIC_SMALL_LETTER_DJE" },
		[243]= { 0, 0x0453, "CYRILLIC_SMALL_LETTER_GJE" },
		[244]= { 0, 0x0454, "CYRILLIC_SMALL_LETTER_UKRAINIAN_IE" },
		[245]= { 0, 0x0455, "CYRILLIC_SMALL_LETTER_DZE" },
		[246]= { 0, 0x0456, "CYRILLIC_SMALL_LETTER_BYELORUSSIAN_UKRAINIAN_I" },
		[247]= { 0, 0x0457, "CYRILLIC_SMALL_LETTER_YI" },
		[248]= { 0, 0x0458, "CYRILLIC_SMALL_LETTER_JE" },
		[249]= { 0, 0x0459, "CYRILLIC_SMALL_LETTER_LJE" },
		[250]= { 0, 0x045A, "CYRILLIC_SMALL_LETTER_NJE" },
		[251]= { 0, 0x045B, "CYRILLIC_SMALL_LETTER_TSHE" },
		[252]= { 0, 0x045C, "CYRILLIC_SMALL_LETTER_KJE" },
		[253]= { 0, 0x00A7, "SECTION_SIGN" },
		[254]= { 0, 0x045E, "CYRILLIC_SMALL_LETTER_SHORT_U" },
		[255]= { 0, 0x045F, "CYRILLIC_SMALL_LETTER_DZHE" }
};

// List of available codesets

static struct codeset_spec {
	char *name;
	struct codeset_element *codeset;
} codeset_specs[] = {
		{ "ASCII",      ascii        },
		{ "ISO-8859-15", iso_8859_15 },
		{ "ISO-8859-2",  iso_8859_2  },
		{ "ISO-8859-5",  iso_8859_5  }
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
		fprintf(stderr, "%s: Wrong code %u\n", __FUNCTION__, code);
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
		fprintf(stderr, "%s: Wrong code %u\n", __FUNCTION__, code);
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
 * Return the unicode value of a 8-bit 'code' of a given codeset
 */
uint16_t cs_code_to_unicode(unsigned int code, const char *cs)
{
	if (code > 255) {
		fprintf(stderr, "%s: Wrong code %u\n", __FUNCTION__, code);
		exit(1);
	}

	struct codeset_element *codeset = cs_get_codeset(cs);
	if (!codeset) {
		fprintf(stderr, "%s: Unknown codeset %s\n", __FUNCTION__, cs);
		exit(1);
	}

	if (code < 160) {
		return ascii[code].code;
	}
	return codeset[code].code;
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
