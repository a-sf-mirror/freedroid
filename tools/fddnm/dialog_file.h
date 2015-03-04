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
	File:		dialog_file.h
	Purpose:	class defintion for parsing FDRPG dialogs
	Author:		Scott Furry
	Date:		2014 Nov 13
	Update:		2015 Feb 25
*/

#ifndef DIALOG_FILE_H
#define DIALOG_FILE_H

#include "dialog_regex.h"
#include "dialog_node.h"
#include <vector>

class dialog_file
{
public:

	dialog_file();								//constructor
	dialog_file(const dialog_file& rhs);		//copy-constructor
	dialog_file(dialog_file&& rhs) noexcept;	//move-constructor
	virtual ~dialog_file();						//destructor

	//copy assignment operator
	dialog_file& operator=(const dialog_file& rhs);
	//move Assignment operator
	dialog_file& operator=(dialog_file&& rhs) noexcept;

	bool		parseFile();
	bool		parseNodes(std::string& errormsg);
	std::string	printFile() const;

	void setFileName(CONST_STR& text) { filename = text; }
	void setCharName(CONST_STR& text) { charname = text; }

	bool		hasIncludes()	const	{ return this->nodeIncludes; }
	bool		getEndDlg()		const	{ return this->hasEndDlg; }
	int			getNodeCount()	const	{ return this->nodeCount; }
	int			getNodeSize()	const	{ return this->dlgNodes.size(); }
	CONST_STR	getFileName()	const	{ return this->filename; }
	CONST_STR	getCharName()	const	{ return this->charname; }
	CONST_STR	getRawText()	const	{ return this->textRaw; }
	CONST_STR	getParsedText()	const	{ return this->textParsed; }

	linetext	getInclFiles()	const	{ return this->nodeInclFiles; }
	filenodes	getAllNodes()	const	{ return dlgNodes; }
	void		addNodes(CONST_STR& subtopic, filenodes& addnodes);

private:
	static std::string	lineTrim(CONST_STR& text);

private:
	bool		nodeIncludes;
	bool		hasEndDlg;
	int			nodeCount;
	std::string	filename;
	std::string	charname;
	std::string	textRaw;
	std::string	textParsed;
	linetext	nodeInclFiles;
	filenodes	dlgNodes;
};

#endif //DIALOG_FILE_H
