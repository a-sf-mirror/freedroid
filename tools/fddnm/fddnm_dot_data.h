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
	File:		fddnm_dot_data.h
	Purpose:	defintion/implementation of dialog_dot_data struct
	Author:		Scott Furry
	Date:		2014 Dec 12
	Update:		2015 Feb 25
*/

#ifndef FDDNM_DOT_DATA_H
#define FDDNM_DOT_DATA_H

#include "dialog_file.h"
#include "dialog_dot.h"

struct dialog_dot_data {

	dialog_dot_data() :
	skipRender(),
	dotName(),
	dialogName(),
	dialogFileName(),
	displayPath(),
	dialogData(),
	dotData()
	{}

	//copy-constructor
	dialog_dot_data(const dialog_dot_data& rhs) :
	skipRender(rhs.skipRender),
	dotName(rhs.dotName),
	dialogName(rhs.dialogName),
	dialogFileName(rhs.dialogFileName),
	displayPath(rhs.displayPath),
	dialogData(rhs.dialogData),
	dotData(rhs.dotData)
	{ }

	//move-cosntructor
	dialog_dot_data(dialog_dot_data&& rhs) noexcept :
	skipRender(rhs.skipRender),
	dotName(rhs.dotName),
	dialogName(rhs.dialogName),
	dialogFileName(rhs.dialogFileName),
	displayPath(rhs.displayPath),
	dialogData(rhs.dialogData),
	dotData(rhs.dotData)
	{}

	virtual ~dialog_dot_data()
	{}

	//copy assignment operator
	dialog_dot_data& operator=(const dialog_dot_data& rhs)
	{
		if(this != &rhs)
		{
			this->skipRender		= rhs.skipRender;
			this->dotName			= rhs.dotName;
			this->dialogName		= rhs.dialogName;
			this->dialogFileName	= rhs.dialogFileName;
			this->displayPath		= rhs.displayPath;
			this->dialogData		= rhs.dialogData;
			this->dotData			= rhs.dotData;
		}
		return *this;
	}

	//move Assignment operator
    dialog_dot_data& operator=(dialog_dot_data&& rhs) noexcept
	{
		this->skipRender		= std::move(rhs.skipRender);
		this->dotName			= std::move(rhs.dotName);
		this->dialogName		= std::move(rhs.dialogName);
		this->dialogFileName	= std::move(rhs.dialogFileName);
		this->displayPath		= std::move(rhs.displayPath);
		this->dialogData		= std::move(rhs.dialogData);
		this->dotData			= std::move(rhs.dotData);
		return *this;
	}

	// is this file skipped during output to graphics?
	// set to yes if another dialog includes the file (and -I flag not set)
	bool			skipRender;
	// dot friendly name (dot language does not like certain chars
	// - see dialog_dot::toDotStr
	std::string		dotName;
	// dialog name (no extension)
	std::string		dialogName;
	// full path to dialog
	std::string		dialogFileName;
	// shortened path to display to console
	std::string		displayPath;
	//	parsed data - class dialog_file
	dialog_file		dialogData;
	//	dot data - class dialog_dot
	dialog_dot		dotData;
};

#endif	// FDDNM_DOT_DATA_H
