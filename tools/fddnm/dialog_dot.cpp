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
	Update:		2015 Mar 27
*/

#include "dialog_dot.h"
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <utility>
#include <boost/algorithm/string.hpp>	// boost - string manip/upper/lower
#include <boost/tokenizer.hpp>			// boost - char_separator

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif	//BOOST_FILESYSTEM_NO_DEPRECATED

namespace ba = boost::algorithm;

const int WORD_WRAP_SIZE = 25;
CONST_STR STR_STYLE_END("color=\"purple\"");
CONST_STR STR_STYLE_SHOW("color=\"blue\"");
CONST_STR STR_STYLE_SHOWIF("color=\"orange\"");
CONST_STR STR_STYLE_NEXT("color=\"#2fcc2f\"");	// green;
CONST_STR STR_STYLE_HIDE("style=dashed penwidth=\"0.50\" color=\"red\"");

CONST_STR STR_NODE_REPLACE_LABEL("$$$LABEL$$$");
CONST_STR STR_NODE_REPLACE_TEXT("$$$TEXT$$$");

CONST_STR STR_HEAD_DETAIL("<<FONT><B>" + STR_NODE_REPLACE_LABEL + "</B></FONT>>");
CONST_STR STR_NODE_DETAIL("\t<<FONT>\
		\t\t<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"1\">\
		\t\t<TR><TD ALIGN=\"TEXT\"><B>" + STR_NODE_REPLACE_LABEL + "</B></TD></TR>\
		\t\t<TR><TD ALIGN=\"TEXT\">" + STR_NODE_REPLACE_TEXT + "</TD></TR>\
		\t\t</TABLE>\
		\t</FONT>>");
//CONST_STR STR_STYLE_PUSH("color=\"#4f4fff\"");
//CONST_STR STR_STYLE_POP("color=\"#ffff4f\"");


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

std::string dialog_dot::getDotContent(bool withDetail /*= false*/, bool withGrouping /*= false*/) const
{
	std::stringstream dotOut;
	dotOut	<< "digraph fddnm_" << this->characterName << " {" << std::endl
			<< "\trankdir=\"" << drawDirection << "\";" << std::endl
			<< "\tpad=\"0.25\";" << std::endl
			<< "\tnodesep=\"0.25\";" << std::endl
			<< "\tranksep=\"0.25\";" << std::endl
			<< "\tpackMode=\"graph\";" << std::endl
			<< "\tordering=\"in\";" << std::endl
			<< "\tremincross=\"true\";" << std::endl
			<< "\tfontsize=\"12\";" << std::endl
			<< std::endl;

	if (withDetail)
	{
		dotOut << "\tnode[shape=none margin=\"0\"  pad=\"0\" fontsize=\"10\"];" << std::endl;
	}
	else
	{
		dotOut << "\tnode[style=rounded shape=box margin=\"0\" pad=\"0\" fontsize=\"10\"];" << std::endl;
	}

	dotOut	<< "\tedge[style=solid penwidth=\"0.65\" minlen=\"1.5\"];" << std::endl
			<< std::endl;


	dotOut << "\t" << this->nodePrefix << " [label=";
	if (withDetail)
	{
		// insert this->characterLabel into STR_HEAD_DETAIL
		std::string newText(STR_HEAD_DETAIL);
		std::string newLabel(toHTMLStr(this->characterLabel));
		newText = toReplace(newText,STR_NODE_REPLACE_LABEL,newLabel);
		dotOut << newText;
	}
	else
	{
		dotOut  << "\"" << this->characterLabel << "\"";
	}
	 dotOut << " shape=box style=rounded width=\"2.5\" fontsize=\"14\"];" << std::endl;
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

	// output node definitions
	std::string strGroupEnd(" group=\"END\"");
	for (auto& node: nodes)
	{
		if ( node.dotID.empty() ) continue;
		dotOut	<< "\t" << node.dotID << "[";
		if (withDetail)
		{
			// replace label marker with label node.dotLabel
			// replace text marker with text
			// insert this->characterLabel into STR_HEAD_DETAIL
			std::string newText(STR_NODE_DETAIL);
			std::string newLabel(node.dotLabel);
			std::string textdata(node.nodeText);

			newLabel = toHTMLStr(newLabel);
			textdata = textWrap(textdata,WORD_WRAP_SIZE);
			textdata = toHTMLStr(textdata);

			newText = toReplace(newText,STR_NODE_REPLACE_LABEL,newLabel);
			newText = toReplace(newText,STR_NODE_REPLACE_TEXT,textdata);
			dotOut	<< "label=" << newText;
		}
		else
		{
			dotOut	<< "label=\"" << node.dotLabel << "\"";
		}

		if(withGrouping)
		{

			if (node.dotID.compare(dotNodeEnd) == 0)
			{
				// add to "end of dialog" group
				// hint to dot to keep these items close to bottom
				dotOut << strGroupEnd;
			}
			else
			{
				for (auto& edgeitem: relationship)
				{
					if (edgeitem.parentnode.compare(this->nodePrefix) == 0) continue;
					if (edgeitem.parentnode.compare(dotNodeFirst) == 0) continue;
					if (edgeitem.parentnode.compare(dotNodeEvery) == 0) continue;
					if (node.dotID.compare(edgeitem.parentnode) != 0) continue;
					if (edgeitem.childnode.compare(dotNodeEnd) == 0)
					{
						// add to "end of dialog" group
						// hint to dot to keep these items close to bottom
						dotOut << strGroupEnd;
					}
				}
			}
		}
		dotOut << "];" << std::endl;
	}

	// dot language to force [First|Every]Time to be same rank
	if ((!dotNodeFirst.empty()) && (!dotNodeEvery.empty()))
	{
		dotOut	<< "\t{ rank=\"same\" "
				<< dotNodeFirst
				<< ", "
				<< dotNodeEvery
				<< " }" << std::endl;
	}

	// dot language to force end_dialog to be lower
	if (!dotNodeEnd.empty())
	{
		dotOut	<< "\t{ rank=\"max\" "
				<< dotNodeEnd << " }"
				<< std::endl;
	}

	// output node relationships -> edges -> lines between
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

	// output node clusters ( in dialogs - subtopics)
	if(!clusternames.empty())
	{
		for (auto& checkcluster: clusternames)
		{
			bool haveLabel = false;
			dotOut	<< "\tsubgraph cluster_" << checkcluster << " {" << std::endl;
			for (auto& node: nodes)
			{
				if (checkcluster.compare(node.dotCluster) == 0)
				{
					if (!haveLabel)
					{
						dotOut	<< "\t\tlabel =";
						if (withDetail)
						{
							// insert this->characterLabel into STR_HEAD_DETAIL
							std::string newText(STR_HEAD_DETAIL);
							std::string newLabel(toHTMLStr(node.dotClusterLabel));
							newText = toReplace(newText,STR_NODE_REPLACE_LABEL,newLabel);
							dotOut << newText;
						}
						else
						{
							dotOut << "\"" << node.dotClusterLabel << "\"";
						}
						dotOut << ";" << std::endl;
						haveLabel=true;
					}
					dotOut	<< "\t\t"
							<< node.dotID
							<< std::endl;
				}
			}
			dotOut << "\t}" << std::endl;
		}
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

std::string dialog_dot::toHTMLStr(CONST_STR& text)
{
	std::string retText;
	retText = std::regex_replace (text,REGEX_AMP,"&amp;");
	retText = std::regex_replace (retText,REGEX_LT,"&lt;");
	retText = std::regex_replace (retText,REGEX_GT,"&gt;");
	retText = std::regex_replace (retText,REGEX_NEWLINE,"<BR/>");
	retText = std::regex_replace (retText,REGEX_QUOTE,"&quote;");
	retText = std::regex_replace (retText,REGEX_APOSTROPHE,"&#39;");
	retText = std::regex_replace (retText,REGEX_SPACE,"&nbsp;");

	return retText;
}

std::string dialog_dot::toReplace(CONST_STR& text, CONST_STR& search, CONST_STR& replace)
{
	std::string retText(text);
	ba::replace_first(retText,search,replace);
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
		std::string nodeText = theNode.getNodeText();
		node dotnode(	clustername,
						clusterlabel,
						nodeDotID,
						nodelabel,
						nodeText);
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

std::string dialog_dot::textWrap(CONST_STR& input, const int& nWidth)
{
	std::stringstream ss;
	int line_len = 0;
	bool first_word = true;
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep(" ");
	tokenizer tokens(input, sep);
	for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter)
	{
		std::string word(*tok_iter);
		if ((line_len + word.size() + 1) <= nWidth)
		{
			line_len += word.size() + 1;
			if (first_word) first_word = false;
			else ss << ' ';
		}
		else
		{
			ss << std::endl;
			line_len = word.size();
		}
		ss << word;
	}
	ss << std::endl;
	return ss.str();
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
