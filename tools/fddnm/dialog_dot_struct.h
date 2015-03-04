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
	File:		dialog_dot_struct.h
	Purpose:	defintions supporting FDRPG dialog nodes parsing
	Author:		Scott Furry
	Date:		2014 Dec 12
	Update:		2015 Feb 25
*/

#ifndef DIALOG_DOT_STRUCT_H
#define DIALOG_DOT_STRUCT_H

#include "dialog_regex.h"
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>

struct node {
	node(CONST_STR& a, CONST_STR& b, CONST_STR& c, CONST_STR& d) :
	dotCluster(a), dotClusterLabel(b), dotID(c), dotLabel(d)	{}

	bool operator==(const node& rhs) const
	{
		return ((dotCluster.compare(rhs.dotCluster) == 0) &&
				(dotClusterLabel.compare(rhs.dotClusterLabel) == 0) &&
				(dotID.compare(rhs.dotID) == 0) &&
				(dotLabel.compare(rhs.dotLabel) == 0));
	}

	bool operator!=(const node& rhs) const
	{
		return !(*this == rhs);
	}

	std::string printNode() const
	{
		std::stringstream outText;
		outText	<< "dotID: " << std::setw(26) << this->dotID
				<< "\tdotLabel: " << std::setw(26) << this->dotLabel
				<< "\tdotCluster: " << std::setw(40) << this->dotCluster
				<< "\tdotClusterLabel: " << this->dotClusterLabel;
		return outText.str();
	}

	std::string dotCluster;
	std::string dotClusterLabel;
	std::string dotID;
	std::string dotLabel;
};

typedef std::vector<node>					dotNodes;
typedef dotNodes::iterator					dotNodes_IT;
typedef dotNodes::reverse_iterator			dotNodes_revIT;
typedef dotNodes::const_iterator			dotNodes_const_IT;
typedef dotNodes::const_reverse_iterator	dotNodes_const_revIT;

struct edge {
	edge(CONST_STR& a, CONST_STR& b, CONST_STR& c, CONST_STR& d) :
	parentnode(a), childnode(b), label(c), style(d)	{}

	bool operator==(const edge& rhs) const
	{
		return ((parentnode.compare(rhs.parentnode) == 0) &&
				(childnode.compare(rhs.childnode) == 0) &&
				(label.compare(rhs.label) == 0) &&
				(style.compare(rhs.style) == 0));
	}

	bool operator!=(const edge& rhs) const
	{
		return !(*this == rhs);
	}

	std::string printEdge() const
	{
		std::stringstream outText;
		outText	<< "parentnode: " << std::setw(30) << this->parentnode
				<< "\tchildnode: " << std::setw(30) << this->childnode
				<< "\tlabel: " << std::setw(10) << this->label
				<< "\tstyle: " << this->style;
		return outText.str();
	}

	std::string	parentnode;
	std::string childnode;
	std::string label;
	std::string style;
};

typedef std::vector<edge>					edgepath;
typedef edgepath::iterator					edgepath_IT;
typedef edgepath::reverse_iterator			edgepath_revIT;
typedef edgepath::const_iterator			edgepath_const_IT;
typedef edgepath::const_reverse_iterator	edgepath_const_revIT;

#endif	//DIALOG_DOT_STRUCT_H
