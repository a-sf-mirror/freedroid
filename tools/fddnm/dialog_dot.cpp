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
	File:		dialog_dot.cpp
	Purpose:	class implementation for producing dot diagram of FDRPG dialog nodes
	Author:		Scott Furry
	Date:		2014 Nov 26
	Update:		2015 Feb 25
*/

#include "dialog_dot.h"
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <utility>

//constructor
dialog_dot::dialog_dot() :
drawDirection(),
dotNodeFirst(),
dotNodeEvery(),
dotNodeEnd(),
characterName(),
characterLabel(),
nodePrefix(),
nodes(),
relationship()
{
}

//copy-constructor
dialog_dot::dialog_dot(const dialog_dot& rhs)
{
	if(this != &rhs)
	{
		this->drawDirection		= rhs.drawDirection;
		this->dotNodeFirst		= rhs.dotNodeFirst;
		this->dotNodeEvery		= rhs.dotNodeEvery;
		this->dotNodeEnd		= rhs.dotNodeEnd;
		this->characterName		= rhs.characterName;
		this->characterLabel	= rhs.characterLabel;
		this->nodePrefix		= rhs.nodePrefix;
		this->nodes.clear();
		this->nodes				= rhs.nodes;
		this->relationship.clear();
		this->relationship		= rhs.relationship;
	}
}

//move-constructor
dialog_dot::dialog_dot(dialog_dot&& rhs) noexcept :
drawDirection(rhs.drawDirection),
dotNodeFirst(rhs.dotNodeFirst),
dotNodeEvery(rhs.dotNodeEvery),
dotNodeEnd(rhs.dotNodeEnd),
characterName(rhs.characterName),
characterLabel(rhs.characterLabel),
nodePrefix(rhs.nodePrefix),
nodes(rhs.nodes),
relationship(rhs.relationship)
{
	rhs.nodes.clear();
	rhs.relationship.clear();
}

//destructor
dialog_dot::~dialog_dot()
{
	nodes.clear();
	relationship.clear();
}

//copy assignment operator
dialog_dot& dialog_dot::operator=(const dialog_dot& rhs)
{
	if(this != &rhs)
	{
		this->drawDirection		= rhs.drawDirection;
		this->dotNodeFirst		= rhs.dotNodeFirst;
		this->dotNodeEvery		= rhs.dotNodeEvery;
		this->dotNodeEnd		= rhs.dotNodeEnd;
		this->characterName		= rhs.characterName;
		this->characterLabel	= rhs.characterLabel;
		this->nodePrefix		= rhs.nodePrefix;
		this->nodes.clear();
		this->nodes				= rhs.nodes;
		this->relationship.clear();
		this->relationship		= rhs.relationship;
	}
	return *this;
}

//move Assignment operator
dialog_dot& dialog_dot::operator=(dialog_dot&& rhs) noexcept
{
	this->drawDirection		= std::move(rhs.drawDirection);
	this->dotNodeFirst		= std::move(rhs.dotNodeFirst);
	this->dotNodeEvery		= std::move(rhs.dotNodeEvery);
	this->dotNodeEnd		= std::move(rhs.dotNodeEnd);
	this->characterName		= std::move(rhs.characterName);
	this->characterLabel	= std::move(rhs.characterLabel);
	this->nodePrefix		= std::move(rhs.nodePrefix);
	this->nodes				= std::move(rhs.nodes);
	this->relationship		= std::move(rhs.relationship);
	return *this;
}

std::string dialog_dot::setFile(const dialog_file& dlgFile)
{
	// populate nodes and edges here
	buildNodes(dlgFile);
	std::string strRet = buildRelationships(dlgFile);
	return strRet;
}

std::string dialog_dot::getDotContent() const
{
	std::stringstream dotOut;
	dotOut	<< "digraph fddnm_" << this->characterName << " {" << std::endl
			<< "\trankdir=\"" << drawDirection << "\";" << std::endl
			<< "\tresolution=72;" << std::endl
			<< "\tfontsize=12;" << std::endl
			<< "\tedge[style=solid, penwidth=1.25];" << std::endl
			<< "\tsep=\"+50,50\";" << std::endl
			<< "\toverlap=scalexy;" << std::endl
			<< "\t" << this->nodePrefix << " [label=\"" << this->characterLabel
					<< "\", shape=Mrecord, width=2.5];" << std::endl;

	// cluster names
	std::vector<std::string> clusternames;
	for (auto& node: nodes)
	{
		if (!node.dotCluster.empty())
		{
			bool bFound = false;
			for (auto& checkcluster: clusternames)
			{
				if(checkcluster.compare(node.dotCluster) == 0)
				{
					bFound = true;
					break;
				}
			}
			if(!bFound) clusternames.push_back(node.dotCluster);
		}
	}

	for (auto& node: nodes)
	{
		if (node.dotCluster.empty())
		{
			dotOut	<< "\t" << node.dotID
					<< "[label=\"" << node.dotLabel << "\", style=rounded, shape=box];"
					<< std::endl;
		}
	}

	if(!dotNodeFirst.empty() && !dotNodeEvery.empty())
	{
		dotOut	<< "{ rank=\"same\" "
				<< dotNodeFirst << " "
				<< dotNodeEvery << " }"
				<< std::endl;
	}

	if(!dotNodeEnd.empty())
	{
		dotOut	<< "{ rank=\"sink\" "
				<< dotNodeEnd << " }"
				<< std::endl;
	}

	if(!clusternames.empty())
	{
		for (auto& checkcluster: clusternames)
		{
			std::string label;
			dotOut << "\tsubgraph cluster_" << checkcluster << " {" << std::endl;
			for (auto& node: nodes)
			{
				if (checkcluster.compare(node.dotCluster) == 0)
				{
					if (label.empty())
					{
						label = node.dotClusterLabel;
						dotOut << "\t\tlabel =\"" << label << "\";" << std::endl;
					}

					dotOut	<< "\t\t" << node.dotID
							<< "[label=\"" << node.dotLabel
							<< "\", style=rounded, shape=box];"
							<< std::endl;
				}
			}
			dotOut << "\t}" << std::endl;
		}
	}
	for (auto& edgeitem: relationship)
	{
		dotOut	<< "\t" << edgeitem.parentnode
				<< " -> "
				<< edgeitem.childnode
				<< " [";
		if(!edgeitem.label.empty())
		{
			dotOut	<< "label=\""
					<< edgeitem.label
					<< "\",";
		}
		dotOut	<< edgeitem.style
				<< "];" << std::endl;
	}
	dotOut << "}" << std::endl;
	return dotOut.str();
}

std::string dialog_dot::printDotData() const
{
	std::stringstream dotOut;
	dotOut	<< "---------- " << std::endl
			<< "- Dot Data" << std::endl
			<< "---------- " << std::endl
			<< "Direction: " << drawDirection << std::endl
			<< "---------- " << std::endl
			<< "- Character" << std::endl
			<< "NodeID: " << std::setw(20) << this->nodePrefix
			<< "\tCharacter(dot): " << std::setw(26) << this->characterName
			<< "\tCharacter(raw): " << std::setw(26) << this->characterLabel
			<< std::endl
			<< "---------- " << std::endl
			<< "- nodes" << std::endl;
	for (auto& nodeitem: nodes)
	{
		dotOut << nodeitem.printNode() << std::endl;
	}
	dotOut	<< "---------- " << std::endl
			<< "- edges" << std::endl;
	for (auto& edgeitem: relationship)
	{
		dotOut << edgeitem.printEdge() << std::endl;
	}
	dotOut	<< "---------- " << std::endl;
	return dotOut.str();
}

std::string dialog_dot::toDotStr(CONST_STR& text)
{
	std::string retText;
	retText = std::regex_replace (text,REGEX_HYPHEN,"_");
	retText = std::regex_replace (retText,REGEX_PERIOD,"_");
	retText = std::regex_replace (retText,REGEX_APOSTROPHE,"_");
	retText = std::regex_replace (retText,REGEX_SPACE,"_");
	retText = std::regex_replace (retText,REGEX_FWD_SLASH,"_");
	return retText;
}

void dialog_dot::buildNodes(const dialog_file& dlgFile)
{
	this->characterLabel = dlgFile.getCharName();
	this->characterName = toDotStr(this->characterLabel);
	for(auto& theNode: dlgFile.getAllNodes())
	{
		if(theNode.hasIncludes()) continue;
		std::string clusterlabel = theNode.getNodeTopic();
		std::string clustername = toDotStr(clusterlabel);
		std::string nodelabel = theNode.getNodeID();
		std::string nodeDotID = nodePrefix + toDotStr(nodelabel);
		node dotnode(	clustername,
						clusterlabel,
						nodeDotID,
						nodelabel);
		nodes.push_back(dotnode);
		if (nodelabel.find("FirstTime") != std::string::npos)
		{
			dotNodeFirst = nodeDotID;
		}
		else if (nodelabel.find("EveryTime") != std::string::npos)
		{
			dotNodeEvery = nodeDotID;
		}
		else if (nodelabel.find(END_DLG) != std::string::npos)
		{
			dotNodeEnd = nodeDotID;
		}
	}
}

std::string dialog_dot::buildRelationships(const dialog_file& dlgFile)
{
	std::stringstream errStream;
	filenodes currentNodes = dlgFile.getAllNodes();
	for (auto& node: currentNodes)
	{
		std::string parentlabel(node.getNodeID());
		std::string parentDotID(nodePrefix + toDotStr(parentlabel));

		linetext code(node.getNodeCode());
		std::string AllCode(node.codeToString());
		if (parentlabel.find("EveryTime") != std::string::npos)
		{
			relationship.push_back(edge(nodePrefix,parentDotID,"",""));
		}

		if (parentlabel.find("FirstTime") != std::string::npos)
		{
			relationship.push_back(edge(nodePrefix,parentDotID,"",""));
		}

		// multiline checks of node code
		std::string errmsg;
//		nodePopTopicParse(parentDotID, AllCode, errmsg);
//		if(!errmsg.empty()) errStream << errmsg << std::endl;
//		errmsg.clear();

		// search dialog node code for "showif(...)"
		nodeShowIfParse(parentDotID, AllCode, errmsg);
		if(!errmsg.empty()) errStream << errmsg << std::endl;
		errmsg.clear();

		// search dialog node code for "show(...)"
		nodeShowParse(parentDotID, AllCode, errmsg);
		if(!errmsg.empty()) errStream << errmsg << std::endl;
		errmsg.clear();

		// search dialog node code for "next(...)"
		nodeNextParse(parentDotID, AllCode, errmsg);
		if(!errmsg.empty()) errStream << errmsg << std::endl;
		errmsg.clear();

		// search dialog node code for "hide(...)"
		nodeHideParse(parentDotID, AllCode, errmsg);
		if(!errmsg.empty()) errStream << errmsg << std::endl;
		errmsg.clear();

		// parse node for its code
		for (auto& text: code)
		{
			std::smatch matchEnd;
			bool bEndDlg = std::regex_search(text, matchEnd, REGEX_CMD_DLG_END);
			if (bEndDlg)
			{
				std::string IDNode(findNodeFromRaw(END_DLG));
				std::string childnode(IDNode);
				edge newEdge(parentDotID,childnode,"",STR_STYLE_END);
				if(!testExistRelationship(newEdge))	relationship.push_back(newEdge);
			}
		} // cycle through each line of code in each dialog node
	} // cycle through each dialog node
	return errStream.str();
}

std::string dialog_dot::findNodeFromRaw(CONST_STR& rawtext) const
{
	std::string retText("");
	for (auto& testnode: nodes)
	{
		if(testnode.dotLabel.compare(rawtext) == 0)
		{
			retText = testnode.dotID;
			break;
		}
	}
	return retText;
}

bool dialog_dot::testExistRelationship(const edge& testItem) const
{
	bool bReturn = false;
	for (auto& examrelation: relationship)
	{
		bReturn = (examrelation == testItem);
		if(bReturn) break;
	}
	return bReturn;
}

// parsing for "show_if(...)" commands
void dialog_dot::nodeShowIfParse(CONST_STR& ParentNode, CONST_STR& NodeCode, std::string& strMsg)
{
	std::stringstream errMsg;
	std::sregex_iterator nextShowIf(NodeCode.begin(), NodeCode.end(), REGEX_CMD_SHOW_IF);
	std::sregex_iterator end;
	while (nextShowIf != end)
	{
		std::string foundText((*nextShowIf).str(1));
		std::string tempNodeID(findNodeFromRaw(foundText));
		if(!tempNodeID.empty())
		{
			edge newEdge( ParentNode, tempNodeID, "", STR_STYLE_SHOWIF);
			if(!testExistRelationship(newEdge))	relationship.push_back(newEdge);
		}
		else
		{
			errMsg	<< "----- showif -----\t"
					<< ParentNode << "\t"
					<< foundText << "\t"
					<< tempNodeID;
		}
		nextShowIf++;
	}
	strMsg = errMsg.str();
}

// parsing for "show(...)" commands
void dialog_dot::nodeShowParse( CONST_STR& ParentNode, CONST_STR& NodeCode, std::string& strMsg)
{
	std::stringstream errStream;
	std::sregex_iterator nextShow(NodeCode.begin(), NodeCode.end(), REGEX_CMD_SHOW);
	std::sregex_iterator end;
	while (nextShow != end)
	{
		std::string nextText((*nextShow).str(1));
		while(nextText.length() > 0)
		{
			std::string txtErr;
			nextText = nodeListParse(ParentNode, nextText, "Show", STR_STYLE_SHOW, txtErr);
			if(!txtErr.empty()) errStream << txtErr << "\t" << nextText;
		} // look for other nodes in show() match
		nextShow++;
	}
	strMsg = errStream.str();
}

// parsing for "next(...)" commands
void dialog_dot::nodeNextParse( CONST_STR& ParentNode, CONST_STR& NodeCode, std::string& strMsg)
{
	std::stringstream errStream;
	std::sregex_iterator nextNext(NodeCode.begin(), NodeCode.end(), REGEX_CMD_NEXT);
	std::sregex_iterator end;
	while (nextNext != end)
	{
		std::string nextText((*nextNext).str(1));
		while(nextText.length() > 0)
		{
			std::string txtErr;
			nextText = nodeListParse(ParentNode, nextText, "Next", STR_STYLE_NEXT, txtErr);
			if(!txtErr.empty()) errStream << txtErr << "\t" << nextText;
		} // look for other nodes in next() match
		nextNext++;
	}
	strMsg = errStream.str();
}

// parsing for "hide(...)" commands
void dialog_dot::nodeHideParse( CONST_STR& ParentNode, CONST_STR& NodeCode, std::string& strMsg)
{
	std::stringstream errStream;
	std::sregex_iterator nextHide(NodeCode.begin(), NodeCode.end(), REGEX_CMD_HIDE);
	std::sregex_iterator end;
	while (nextHide != end)
	{
		std::string nextText((*nextHide).str(1));
		while(nextText.length() > 0)
		{
			std::string txtErr;
			nextText = nodeListParse(ParentNode, nextText, "Hide", STR_STYLE_HIDE, txtErr);
			if(!txtErr.empty()) errStream << txtErr << "\t" << nextText;
		} // look for other nodes in hide() match
		nextHide++;
	}
	strMsg = errStream.str();
}

// parsing for matches to array [ "stuff","stuff",...,"more stuff" ]
std::string dialog_dot::nodeListParse(	CONST_STR& ParentNode,
										CONST_STR& NodeCode,
										CONST_STR& cmd,
										CONST_STR& edgeStyle,
										std::string& strMsg)
{
	std::stringstream errMsg;
	std::string testText(NodeCode);
	std::smatch match;
	if (std::regex_search(testText, match, REGEX_CMD_NODE_LIST))
	{
		std::string foundText(match.str(1));
		std::string tempNodeID(findNodeFromRaw(foundText));
		if(!tempNodeID.empty())
		{
			edge newEdge( ParentNode, tempNodeID, "", edgeStyle);
			if(!testExistRelationship(newEdge))	relationship.push_back(newEdge);
		}
		else
		{
			errMsg	<< "--- " << cmd << " ---\t"
					<< ParentNode << "\t"
					<< foundText << "\t"
					<< tempNodeID;
		}
		testText = testText.substr(match.length());
	}
	else
	{
		testText = "";
	}
	strMsg = errMsg.str();
	return testText;
}

// parsing for "push_topics()" commands
/*
void dialog_dot::nodePushTopicParse(CONST_STR& ParentNode, CONST_STR& NodeCode, std::string& strMsg)
{
	std::stringstream errMsg;
	std::sregex_iterator nextShowIf(NodeCode.begin(), NodeCode.end(), REGEX_CMD_TOPIC_PUSH);
	std::sregex_iterator end;
	while (nextShowIf != end)
	{
		edge newEdge( ParentNode, dotNodeEvery, "", STR_STYLE_PUSH);
		if(!testExistRelationship(newEdge))	relationship.push_back(newEdge);
		else
		{
			errMsg	<< "----- push_topic -----\t"
					<< ParentNode << "\t"
					<< dotNodeEvery;
		}
		nextShowIf++;
	}
	strMsg = errMsg.str();
}
*/

// parsing for "pop_topics()" commands
/*
void dialog_dot::nodePopTopicParse(CONST_STR& ParentNode, CONST_STR& NodeCode, std::string& strMsg)
{
	std::stringstream errMsg;
	std::sregex_iterator nextShowIf(NodeCode.begin(), NodeCode.end(), REGEX_CMD_TOPIC_POP);
	std::sregex_iterator end;
	while (nextShowIf != end)
	{
		edge newEdge( ParentNode, dotNodeEvery, "", STR_STYLE_POP);
		if(!testExistRelationship(newEdge))	relationship.push_back(newEdge);
		else
		{
			errMsg	<< "----- pop_topic -----\t"
					<< ParentNode << "\t"
					<< dotNodeEvery;
		}
		nextShowIf++;
	}
	strMsg = errMsg.str();
}
*/
