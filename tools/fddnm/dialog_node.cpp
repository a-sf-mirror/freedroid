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
	File:		dialog_node.cpp
	Purpose:	class implementation for parsing FDRPG dialog nodes
	Author:		Scott Furry
	Date:		2014 Nov 13
	Update:		2015 Feb 25
*/

#include "dialog_node.h"
#include <sstream>
#include <algorithm>
#include <utility>

//constructor
dialog_node::dialog_node() :
nodeIncludes(false),
nodeID(),
nodeText(),
nodeTopic(),
nodeInclFiles(),
nodeCode()
{}

//copy-constructor
dialog_node::dialog_node(const dialog_node& rhs)
{
	if(this != &rhs)
	{
		this->nodeIncludes	= rhs.nodeIncludes;
		this->nodeID		= rhs.nodeID;
		this->nodeText		= rhs.nodeText;
		this->nodeTopic		= rhs.nodeTopic;
		this->nodeInclFiles.clear();
		copy(rhs.nodeInclFiles.begin(),
			rhs.nodeInclFiles.end(),
			std::back_inserter(this->nodeInclFiles)
		);
		this->nodeCode.clear();
		copy(rhs.nodeCode.begin(),
			rhs.nodeCode.end(),
			std::back_inserter(this->nodeCode)
		);
	}
}

//move-constructor
dialog_node::dialog_node(dialog_node&& rhs) noexcept :
nodeIncludes(rhs.nodeIncludes),
nodeID(rhs.nodeID),
nodeText(rhs.nodeText),
nodeTopic(rhs.nodeTopic),
nodeInclFiles(rhs.nodeInclFiles),
nodeCode(rhs.nodeCode)
{
	rhs.nodeInclFiles.clear();
	rhs.nodeCode.clear();
}

//destructor
dialog_node::~dialog_node()
{
	this->nodeInclFiles.clear();
	this->nodeCode.clear();
}

//copy assignment operator
dialog_node& dialog_node::operator=(const dialog_node& rhs)
{
	if(this != &rhs)
	{
		this->nodeIncludes	= rhs.nodeIncludes;
		this->nodeID		= rhs.nodeID;
		this->nodeText		= rhs.nodeText;
		this->nodeTopic		= rhs.nodeTopic;
		this->nodeInclFiles.clear();
		copy(rhs.nodeInclFiles.begin(),
			rhs.nodeInclFiles.end(),
			std::back_inserter(this->nodeInclFiles)
		);
		this->nodeCode.clear();
		copy(rhs.nodeCode.begin(),
			rhs.nodeCode.end(),
			std::back_inserter(this->nodeCode)
		);
	}
	return *this;
}

//move Assignment operator
dialog_node& dialog_node::operator=(dialog_node&& rhs) noexcept
{
	this->nodeIncludes	= std::move(rhs.nodeIncludes);
	this->nodeID		= std::move(rhs.nodeID);
	this->nodeText		= std::move(rhs.nodeText);
	this->nodeTopic		= std::move(rhs.nodeTopic);
	this->nodeInclFiles	= std::move(rhs.nodeInclFiles);
	this->nodeCode		= std::move(rhs.nodeCode);
	return *this;
}

std::string dialog_node::ParseNode(std::stringstream& nodeData, bool ParseForData/* = true*/)
{
	std::stringstream errorReturn;
	if(!ParseForData)
	{
		// nodes [FirstTime | EveryTime]
		std::string line;
		while ( std::getline(nodeData, line) )
		{
			nodeCode.push_back(line);
		}
	}
	else
	{
		// for each found node - "{ ... },"
		std::string node_text;
		std::stringstream remainderContent;
		while ( std::getline(nodeData, node_text) )
		{
			std::smatch nodeMatchText;
			if (std::regex_search(node_text, nodeMatchText, REGEX_NODE_ID ))
			{
				// extract id
				setNodeID(nodeMatchText.str(1));
			}
			else if (std::regex_search(node_text, nodeMatchText, REGEX_NODE_TOPIC ))
			{
				// extract topic - dialog sub
				setNodeTopic(nodeMatchText.str(1));
			}
			else if (std::regex_search(node_text, nodeMatchText, REGEX_NODE_TEXT ))
			{
				// extract displayed dialog text
				setNodeText(nodeMatchText.str(1));
			}
			else if (std::regex_search(node_text, nodeMatchText, REGEX_NODE_GENINCL ))
			{
				// find node containing "generator = include(...)," - extract included files
				// assume there is only one file to be included
				std::string testText(nodeMatchText.str(1));
				this->nodeIncludes	= true;
				setNodeID("include " + testText);
				setNodeText("include " + testText);
				this->nodeInclFiles.push_back(testText);
			}
			else
			{
				remainderContent << node_text << std::endl;
			}
		}
		node_text = remainderContent.str();
		std::smatch nodeMatchGen;
		std::smatch nodeMatchCode;
		// extract "generator = function() ... end,"
		bool bMatchGen = std::regex_search(node_text, nodeMatchGen, REGEX_NODE_GEN );
		// extract "code = function() ... end,"
		bool bMatchCode = std::regex_search(node_text, nodeMatchCode, REGEX_NODE_CODE );
		if (bMatchGen || bMatchCode)
		{
			if(bMatchGen) setNodeID("Generator");
			std::string codeline;
			std::stringstream nodestream;
			if(bMatchGen) nodestream << nodeMatchGen.str(1);
			if(bMatchCode) nodestream << nodeMatchCode.str(1);
			while ( getline(nodestream, codeline) )
			{
				nodeCode.push_back(codeline);
			}
		}
		else
		{
			std::string codeline;
			std::stringstream nodestream;
			nodestream << node_text;
			while ( getline(nodestream, codeline) )
			{
				nodeCode.push_back(codeline);
			}
		}

		if ((!getNodeTopic().empty()) && ( getNodeID().empty()))
		{
			setNodeID("Topic");
		}

		if (getNodeTopic().empty() && getNodeID().empty())
		{
			errorReturn	<< std::endl
						<< "\tTopic(" << getNodeTopic().length()
						<< "): " << getNodeTopic()
						<< "\tID(" << getNodeID().length()
						<< "): " << getNodeID()
						<< std::endl;
		}
	}
	return errorReturn.str();
}

std::string	dialog_node::printNode() const
{
	std::stringstream outtext;
	outtext	<< "\t" << "NodeID: ";

	if (nodeID.length() == 0) outtext << "##### EMPTY ID #####" << std::endl;
	else outtext << nodeID << std::endl;

	outtext	<< "\t" << "NodeText: " << this->nodeText << std::endl
			<< "\t" << "NodeTopic: " << this->nodeTopic << std::endl;
	if(hasIncludes())
	{
		outtext << "\t" << "Incl File: "
				<< std::boolalpha << this->nodeIncludes
				<< "\t" << inclToString() << std::endl;
	}

	outtext	<< "\t" << "NodeCode: " << std::endl;
	for(auto& text : this->nodeCode)
	{
		if(!text.empty()) outtext << "\t\t" << text << std::endl;
	}
	outtext << std::endl;
	return outtext.str();
}

std::string dialog_node::inclToString() const
{
	std::stringstream retText;
	for(auto& text : this->nodeInclFiles)
	{
		retText	<< "\t" << text;
	}
	return retText.str();
}

std::string dialog_node::codeToString() const
{
	std::stringstream retText;
	for(auto& text : this->nodeCode)
	{
		retText	<< text << std::endl;
	}
	return retText.str();
}
