/*
	Copyright (c) 2015 Scott Furry

	This file is part of Freedroid

	Freedroid is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	Freedroid is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Freedroid; see the file COPYING. If not, write to the
	Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
	MA  02111-1307  USA
*/
/*
	File:		dialog_regex.h
	Purpose:	defintions of regex strings used in parsing FDRPG dialogs
	Author:		Scott Furry
	Date:		2014 Nov 29
	Update:		2015 Mar 27
*/

#include <iostream>
#include <string>
#include <regex>

#ifndef DIALOG_REGEX_H
#define DIALOG_REGEX_H

typedef	const std::string	CONST_STR;
typedef const std::regex	CONST_REGEX;

// name assigned to end dialog node
CONST_STR END_DLG("end_dialog");

CONST_STR REGEX_STR_TRIM_LEAD("^[\\s]+");
CONST_STR REGEX_STR_TRIM_TAIL("[\\s]+$");
CONST_STR REGEX_STR_LN_COMMENT("^[\\s]*--");
CONST_STR REGEX_STR_EMBD_COMMENT("([\\S|\\s]*)--[\\S\\s]*");
CONST_STR REGEX_STR_BLK_COMMENT_START("^[\\s]*--\\[\\[");
CONST_STR REGEX_STR_BLK_COMMENT_END("\\]\\]--$");

CONST_STR REGEX_STR_DLG_RETURN_DEFN("return[\\s]*\\{([\\S|\\s]*)\\}");
CONST_STR REGEX_STR_FN("function\\([\\S|\\s]*?\\)");
CONST_STR REGEX_STR_FN_END("\\send,");
CONST_STR REGEX_STR_NODE_FIRSTTIME
	("FirstTime[\\s]*=[\\s]*" + REGEX_STR_FN + "([\\S|\\s]*?)" + REGEX_STR_FN_END);
CONST_STR REGEX_STR_NODE_EVERYTIME
	("EveryTime[\\s]*=[\\s]*" + REGEX_STR_FN + "([\\S|\\s]*?)" + REGEX_STR_FN_END);
CONST_STR REGEX_STR_NODE_DELIMITER("\\{([\\S|\\s]*?)\\},");


CONST_STR REGEX_STR_NODE_ID("id[\\s]*?=[\\s]*?\"([\\S|\\s]*?)\",");
CONST_STR REGEX_STR_NODE_TOPIC("topic[\\s]*?=[\\s]*?\"([\\S|\\s]*?)\",");
CONST_STR REGEX_STR_NODE_TEXT("text[\\s]*?=[\\s]*?[_]*?\"([\\S|\\s]*?)\",");
CONST_STR REGEX_STR_NODE_GEN
	("generator[\\s]*=[\\s]*" + REGEX_STR_FN + "([\\S|\\s]*)end[,]*");
CONST_STR REGEX_STR_NODE_GENINCLD
	("generator[\\s]*?=[\\s]*?include\\(\"([\\S|\\s]*?)\"\\)[,]+?");
CONST_STR REGEX_STR_NODE_CODE
	("code[\\s]*=[\\s]*" + REGEX_STR_FN + "([\\S|\\s]*?)" + REGEX_STR_FN_END);

CONST_STR REGEX_STR_CMD_NODE_LIST("[,]?[\\s]*?\"([\\S\\s]+?)\"");
CONST_STR REGEX_STR_CMD_NEXT("next\\(([\\S\\s]+?)\\)");
CONST_STR REGEX_STR_CMD_SHOW("show\\(([\\S\\s]+?)\\)");
CONST_STR REGEX_STR_CMD_SHOW_IF("show_if\\([\\S|\\s]+?,[\\s]*?\"([\\S\\s]+?)\"\\)");
CONST_STR REGEX_STR_CMD_HIDE("hide\\(([\\S\\s]+?)\\)");
CONST_STR REGEX_STR_CMD_TOPIC_PUSH("push_topic\\(\"([\\S|\\s]*)\"\\)");
CONST_STR REGEX_STR_CMD_TOPIC_POP("pop_topic\\(\"([\\S|\\s]*)\"\\)");
CONST_STR REGEX_STR_CMD_DLG_END( END_DLG + "\\([\\s]*\\)");

CONST_REGEX REGEX_TRIM_LEAD(REGEX_STR_TRIM_LEAD,std::regex::ECMAScript);
CONST_REGEX REGEX_TRIM_TAIL(REGEX_STR_TRIM_TAIL,std::regex::ECMAScript);
CONST_REGEX REGEX_LN_COMMENT(REGEX_STR_LN_COMMENT,std::regex::ECMAScript);
CONST_REGEX REGEX_EMBD_COMMENT(REGEX_STR_EMBD_COMMENT,std::regex::ECMAScript);
CONST_REGEX REGEX_BLK_COMMENT_START(REGEX_STR_BLK_COMMENT_START,std::regex::ECMAScript);
CONST_REGEX REGEX_BLK_COMMENT_END(REGEX_STR_BLK_COMMENT_END,std::regex::ECMAScript);

CONST_REGEX REGEX_DLG_RETURN_DEFN(REGEX_STR_DLG_RETURN_DEFN,std::regex::ECMAScript);
CONST_REGEX REGEX_NODE_FIRSTTIME(REGEX_STR_NODE_FIRSTTIME,std::regex::ECMAScript);
CONST_REGEX REGEX_NODE_EVERYTIME(REGEX_STR_NODE_EVERYTIME,std::regex::ECMAScript);
CONST_REGEX REGEX_NODE_DELIMITER(REGEX_STR_NODE_DELIMITER,std::regex::ECMAScript);

CONST_REGEX REGEX_NODE_ID(REGEX_STR_NODE_ID,std::regex::ECMAScript);
CONST_REGEX REGEX_NODE_TOPIC(REGEX_STR_NODE_TOPIC,std::regex::ECMAScript);
CONST_REGEX REGEX_NODE_TEXT(REGEX_STR_NODE_TEXT,std::regex::ECMAScript);
CONST_REGEX REGEX_NODE_GEN(REGEX_STR_NODE_GEN,std::regex::ECMAScript);
CONST_REGEX REGEX_NODE_GENINCL(REGEX_STR_NODE_GENINCLD,std::regex::ECMAScript);
CONST_REGEX REGEX_NODE_CODE(REGEX_STR_NODE_CODE,std::regex::ECMAScript);

CONST_REGEX REGEX_CMD_NODE_LIST(REGEX_STR_CMD_NODE_LIST,std::regex::ECMAScript);
CONST_REGEX REGEX_CMD_NEXT(REGEX_STR_CMD_NEXT,std::regex::ECMAScript);
CONST_REGEX REGEX_CMD_SHOW(REGEX_STR_CMD_SHOW,std::regex::ECMAScript);
CONST_REGEX REGEX_CMD_SHOW_IF(REGEX_STR_CMD_SHOW_IF,std::regex::ECMAScript);
CONST_REGEX REGEX_CMD_HIDE(REGEX_STR_CMD_HIDE,std::regex::ECMAScript);
CONST_REGEX REGEX_CMD_TOPIC_PUSH(REGEX_STR_CMD_TOPIC_PUSH,std::regex::ECMAScript);
CONST_REGEX REGEX_CMD_TOPIC_POP(REGEX_STR_CMD_TOPIC_POP,std::regex::ECMAScript);
CONST_REGEX REGEX_CMD_DLG_END(REGEX_STR_CMD_DLG_END,std::regex::ECMAScript);

// dot language doesn't like '-' and "." chars
// replace these for the nodeID
CONST_REGEX REGEX_HYPHEN("(\\-)",std::regex::ECMAScript);
CONST_REGEX REGEX_PERIOD("(\\.)",std::regex::ECMAScript);
CONST_REGEX REGEX_APOSTROPHE("(\\\')",std::regex::ECMAScript);
CONST_REGEX REGEX_SPACE("( )",std::regex::ECMAScript);
CONST_REGEX REGEX_FWD_SLASH("(/)",std::regex::ECMAScript);
CONST_REGEX REGEX_NEWLINE("(\\n)",std::regex::ECMAScript);
CONST_REGEX REGEX_AMP("(\\&)",std::regex::ECMAScript);
CONST_REGEX REGEX_LT("(\\<)",std::regex::ECMAScript);
CONST_REGEX REGEX_GT("(\\>)",std::regex::ECMAScript);
CONST_REGEX REGEX_QUOTE("(\\\")",std::regex::ECMAScript);



#endif	// DIALOG_REGEX_H
