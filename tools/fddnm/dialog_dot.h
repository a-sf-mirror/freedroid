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
	File:		dialog_dot.h
	Purpose:	class defintion for producing dot diagram of FDRPG dialog nodes
	Author:		Scott Furry
	Date:		2014 Nov 26
	Update:		2015 Mar 27
*/

#ifndef DIALOG_DOT_H
#define DIALOG_DOT_H

#include "dialog_regex.h"
#include "dialog_dot_struct.h"
#include "dialog_file.h"

class dialog_dot
{
public:
	dialog_dot();								//constructor
	dialog_dot(const dialog_dot& rhs);			//copy-constructor
	dialog_dot (dialog_dot&& rhs) noexcept;		//move-constructor
	virtual ~dialog_dot();						//destructor

	//copy assignment operator
	dialog_dot& operator=(const dialog_dot& rhs);
	//move Assignment operator
    dialog_dot& operator=(dialog_dot&& rhs) noexcept;

	std::string setFile(const dialog_file& dlgFile);
	void		setDrawDirection(CONST_STR& DrawDir)	{ drawDirection = DrawDir; }
	void		setNodePrefix(CONST_STR& prefix)		{ nodePrefix = prefix; }
	std::string	getDotContent(bool withDetail = false, bool withGrouping = false) const;
	std::string printDotData() const;

	static std::string toDotStr(CONST_STR& text);
	static std::string toHTMLStr(CONST_STR& text);
	static std::string toReplace(CONST_STR& text, CONST_STR& search, CONST_STR& replace);
	static std::string textWrap(CONST_STR& input, const int& nWidth);
private:
	void		buildNodes(const dialog_file& dlgFile);
	std::string	buildRelationships(const dialog_file& dlgFile);

	std::string	findCharNodeID(CONST_STR& character) const;
	std::string	findNodeFromRaw(CONST_STR& rawtext) const;
	bool		testExistRelationship(const edge& testItem) const;

	// parse node code block for commands
	void	nodeShowIfParse( CONST_STR& ParentNode, CONST_STR& NodeCode, std::string& strMsg);
	void	nodeShowParse( CONST_STR& ParentNode, CONST_STR& NodeCode, std::string& strMsg);
	void	nodeNextParse( CONST_STR& ParentNode, CONST_STR& NodeCode, std::string& strMsg);
	void	nodeHideParse( CONST_STR& ParentNode, CONST_STR& NodeCode, std::string& strMsg);
	std::string	nodeListParse(	CONST_STR& ParentNode, CONST_STR& NodeCode,
								CONST_STR& cmd, CONST_STR& edgeStyle,
								std::string& strMsg);
//	void	nodePushTopicParse(CONST_STR& ParentNode, CONST_STR& NodeCode, std::string& strMsg);
//	void	nodePopTopicParse(CONST_STR& ParentNode, CONST_STR& NodeCode, std::string& strMsg);

private:
	std::string	drawDirection;
	std::string	dotNodeFirst;
	std::string dotNodeEvery;
	std::string	dotNodeEnd;
	std::string characterName;
	std::string characterLabel;
	std::string	nodePrefix;
	dotNodes	nodes;
	edgepath	relationship;
};


#endif //DIALOG_DOT_H
