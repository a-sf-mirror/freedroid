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
	File:		dialog_file.cpp
	Purpose:	class implementation for parsing FDRPG dialogs
	Author:		Scott Furry
	Date:		2014 Nov 13
	Update:		2015 Feb 25
*/

#include "dialog_file.h"
#include <exception>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <utility>

//constructor
dialog_file::dialog_file() :
nodeIncludes(false),
hasEndDlg(false),
nodeCount(0),
filename(),
charname(),
textRaw(),
textParsed(),
nodeInclFiles(),
dlgNodes()
{
}
//copy-constructor
dialog_file::dialog_file(const dialog_file& rhs)
{
	if(this != &rhs)
	{
		this->nodeIncludes	= rhs.nodeIncludes;
		this->hasEndDlg		= rhs.hasEndDlg;
		this->nodeCount		= rhs.nodeCount;
		this->filename		= rhs.filename;
		this->charname		= rhs.charname;
		this->textRaw		= rhs.textRaw;
		this->textParsed 	= rhs.textParsed;
		this->nodeInclFiles.clear();
		copy(rhs.nodeInclFiles.begin(),
			rhs.nodeInclFiles.end(),
			std::back_inserter(this->nodeInclFiles)
		);
		this->dlgNodes.clear();
		copy(rhs.dlgNodes.begin(),
			rhs.dlgNodes.end(),
			std::back_inserter(this->dlgNodes)
		);
	}
}

//move-constructor
dialog_file::dialog_file(dialog_file&& rhs) noexcept :
nodeIncludes(rhs.nodeIncludes),
hasEndDlg(rhs.hasEndDlg),
nodeCount(rhs.nodeCount),
filename(rhs.filename),
charname(rhs.charname),
textRaw(rhs.textRaw),
textParsed(rhs.textParsed),
nodeInclFiles(rhs.nodeInclFiles),
dlgNodes(rhs.dlgNodes)
{
	rhs.nodeInclFiles.clear();
	rhs.dlgNodes.clear();
}

//destructor
dialog_file::~dialog_file()
{
	nodeInclFiles.clear();
	dlgNodes.clear();
}

//copy assignment operator
dialog_file& dialog_file::operator=(const dialog_file& rhs)
{
	if(this != &rhs)
	{
		this->nodeIncludes	= rhs.nodeIncludes;
		this->hasEndDlg		= rhs.hasEndDlg;
		this->nodeCount		= rhs.nodeCount;
		this->filename		= rhs.filename;
		this->charname		= rhs.charname;
		this->textRaw		= rhs.textRaw;
		this->textParsed 	= rhs.textParsed;
		this->nodeInclFiles.clear();
		copy(rhs.nodeInclFiles.begin(),
			rhs.nodeInclFiles.end(),
			std::back_inserter(this->nodeInclFiles)
		);
		this->dlgNodes.clear();
		copy(rhs.dlgNodes.begin(),
			rhs.dlgNodes.end(),
			std::back_inserter(this->dlgNodes)
		);
	}
	return *this;
}

//move Assignment operator
dialog_file& dialog_file::operator=(dialog_file&& rhs) noexcept
{
	this->nodeIncludes	= std::move(rhs.nodeIncludes);
	this->hasEndDlg		= std::move(rhs.hasEndDlg);
	this->nodeCount		= std::move(rhs.nodeCount);
	this->filename		= std::move(rhs.filename);
	this->charname		= std::move(rhs.charname);
	this->textRaw		= std::move(rhs.textRaw);
	this->textParsed	= std::move(rhs.textParsed);
	this->nodeInclFiles	= std::move(rhs.nodeInclFiles);
	this->dlgNodes		= std::move(rhs.dlgNodes);
	return *this;
}

bool dialog_file::parseFile()
{
	std::stringstream rawRead;
	std::stringstream parsedRead;
	bool bReturn = false;
	std::ifstream myfile(filename,std::ios::binary);
	if (myfile.is_open())
	{
		std::string str;
		bool bInMulti = false;
		while (getline(myfile, str))
		{
			rawRead << str << std::endl;
			std::smatch matchEmbed;
			// line of text starts with a comment
			bool bREGEX_LN_COMMENT = std::regex_search(str.begin(),str.end(),REGEX_LN_COMMENT);
			// line of text starts multiline entry
			bool bmultiline = std::regex_search(str.begin(),str.end(),REGEX_BLK_COMMENT_START);
			if (!bInMulti)
			{
				if (str.length() >= 1)
				{
					// not in a multi-line comment block
					if ( bREGEX_LN_COMMENT && !bmultiline )
					{
						continue;
					}
					else if (bREGEX_LN_COMMENT && bmultiline)
					{
						 bInMulti = true;
						 continue;
					}
					else if (std::regex_search(str, matchEmbed, REGEX_EMBD_COMMENT))
					{
						// line of text has an embedded comment
						parsedRead << lineTrim(matchEmbed.str(1)) << std::endl;
					} else {
						// just line of text
						parsedRead << lineTrim(str) << std::endl;
					}
				}
			} else {
				// stop searching for multiline comments if multi line comment end found
				bInMulti = !std::regex_search(str.begin(),str.end(),REGEX_BLK_COMMENT_END);
			}
		}
		bReturn = true;
		textParsed = parsedRead.str();
		textRaw = rawRead.str();
	}
	else
	{
		throw std::runtime_error("Unable to open file " + filename);
	}
	return bReturn;
}

bool dialog_file::parseNodes(std::string& errormsg)
{
	std::stringstream errorReturn;
	bool bReturn = false;
	// get all text parsed from file
	std::string filetext(textParsed);
	// find beginning/end - nodes are in between
	std::string result;
	std::smatch strMatch;
	if (std::regex_search(filetext, strMatch, REGEX_DLG_RETURN_DEFN))
	{
		// have all text between "return { ... }" from dialog file
		result = strMatch.str(1);
		std::smatch matchFirstTime;
		if(std::regex_search(result, matchFirstTime, REGEX_NODE_FIRSTTIME))
		{
			// found dialog node: "FirstTime = function()...end,
			std::stringstream nodetext;
			dialog_node newnode;
			newnode.setNodeID("FirstTime");
			newnode.setNodeText("FirstTime");
			nodetext << matchFirstTime.str(1);
			std::string retMsg = newnode.ParseNode(nodetext,false);
			if(!retMsg.empty()) errorReturn << retMsg << std::endl;
			dlgNodes.push_back(newnode);
			nodeCount++;
		}
		std::smatch matchEveryTime;
		if (std::regex_search(result, matchEveryTime, REGEX_NODE_EVERYTIME))
		{
			// found dialog node: "EveryTime = function()...end,
			std::stringstream nodetext;
			dialog_node newnode;
			newnode.setNodeID("EveryTime");
			newnode.setNodeText("EveryTime");
			nodetext << matchEveryTime.str(1);
			std::string retMsg = newnode.ParseNode(nodetext,false);
			if(!retMsg.empty()) errorReturn << retMsg << std::endl;
			dlgNodes.push_back(newnode);
			nodeCount++;
		}
		// for each found node - "{ ... },"
		std::sregex_iterator next(result.begin(), result.end(), REGEX_NODE_DELIMITER);
		std::sregex_iterator end;
		while (next != end) {
			dialog_node newnode;
			std::stringstream nodeData;
			nodeData << (*next).str(1);
			std::string retMsg = newnode.ParseNode(nodeData);
			if(!retMsg.empty()) errorReturn << retMsg << std::endl;
			dlgNodes.push_back(newnode);

			if(newnode.hasIncludes())
			{
				linetext includes = newnode.getInclFiles();
				copy(includes.begin(),includes.end(), std::back_inserter(this->nodeInclFiles));
				this->nodeIncludes = true;
			}
			nodeCount++;
			next++;
		}

		if(!hasEndDlg)
		{
			hasEndDlg = std::regex_search(result.begin(), result.end(), REGEX_CMD_DLG_END);
		}
		bReturn = true;
	}

	// add end_dialog node if called from dialog file
	if(hasEndDlg)
	{
		dialog_node endnode;
		endnode.setNodeID(END_DLG);
		endnode.setNodeText(END_DLG);
		dlgNodes.push_back(endnode);
	}
	errormsg = errorReturn.str();
	return bReturn;
}

// remove all leading and trailing whitespaces from text
std::string dialog_file::lineTrim(CONST_STR& text)
{
	std::string strReturn(text);
	std::smatch matchTrimLead;
	std::smatch matchTrimTail;
	if (std::regex_search(strReturn, matchTrimLead, REGEX_TRIM_LEAD))
	{
		strReturn = strReturn.substr(matchTrimLead.length());
	}
	if (std::regex_search(strReturn, matchTrimTail, REGEX_TRIM_TAIL))
	{
		strReturn = strReturn.substr(0, matchTrimTail.position());
	}
	return strReturn;
}

std::string dialog_file::printFile() const
{
	std::stringstream outtext;
	outtext	<< "File: " << filename << std::endl
			<< "CharName: " << charname
			<< "\t\t" << "Included Dialogs: ";
	if(!nodeInclFiles.empty())
	{
		for(auto& filename : nodeInclFiles)
		{
			outtext << "\t" << filename;
		}
	}
	else
	{
		outtext << std::boolalpha << nodeIncludes;
	}
	outtext << std::endl
			<< "Node Count: " << getNodeSize()
			<< std::endl << std::endl;
	for (auto& nodedata: dlgNodes)
	{
		outtext	<< nodedata.printNode();
	}
	outtext << std::endl;
	return outtext.str();
}

void dialog_file::addNodes(CONST_STR& subtopic, filenodes& addnodes)
{
	for(auto& nodeitem: addnodes)
	{
		nodeitem.setNodeTopic(subtopic);
		dlgNodes.push_back(nodeitem);
	}
}
