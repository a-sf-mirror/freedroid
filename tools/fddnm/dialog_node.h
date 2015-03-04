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
	File:		dialog_node.h
	Purpose:	class defintion for parsing FDRPG dialog nodes
	Author:		Scott Furry
	Date:		2014 Nov 13
	Update:		2015 Feb 25
*/

#ifndef DIALOG_NODE_H
#define DIALOG_NODE_H

#include "dialog_regex.h"
#include <vector>

typedef	std::vector<std::string>			linetext;
typedef linetext::iterator					linetext_IT;
typedef linetext::reverse_iterator			linetext_revIT;
typedef linetext::const_iterator			linetext_const_IT;
typedef linetext::const_reverse_iterator	linetext_const_revIT;

class dialog_node
{
public:
	dialog_node();								//constructor
	dialog_node(const dialog_node& rhs);		//copy-constructor
	dialog_node (dialog_node&& rhs) noexcept;	//move-constructor
	virtual ~dialog_node();						//destructor

	//copy assignment operator
	dialog_node& operator=(const dialog_node& rhs);
	//move Assignment operator
    dialog_node& operator=(dialog_node&& rhs) noexcept;

	std::string		ParseNode(std::stringstream& nodeData,bool ParseForData = true);
	void			setNodeID(CONST_STR& text)		{ nodeID = text; }
	void			setNodeText(CONST_STR& text)	{ nodeText = text; }
	void			setNodeTopic(CONST_STR& text)	{ nodeTopic = text; }

	bool			hasIncludes() const		{ return this->nodeIncludes; }
	std::string		getNodeID() const		{ return this->nodeID; }
	std::string		getNodeText() const		{ return this->nodeText; }
	std::string		getNodeTopic() const	{ return this->nodeTopic; }
	linetext		getInclFiles() const	{ return this->nodeInclFiles; }
	linetext		getNodeCode() const		{ return this->nodeCode; }

	std::string		printNode() const;
	std::string		inclToString() const;
	std::string		codeToString() const;
private:
	bool			nodeIncludes;
	std::string		nodeID;
	std::string		nodeText;
	std::string		nodeTopic;
	linetext		nodeInclFiles;
	linetext		nodeCode;
};

typedef	std::vector<dialog_node>			filenodes;
typedef filenodes::iterator					filenodes_IT;
typedef filenodes::reverse_iterator			filenodes_revIT;
typedef filenodes::const_iterator			filenodes_const_IT;
typedef filenodes::const_reverse_iterator	filenodes_const_revIT;

#endif //DIALOG_NODE_H
