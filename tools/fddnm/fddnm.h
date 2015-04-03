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
	File:		fddnm.h
	Purpose:	class definition for fddnm control
	Author:		Scott Furry
	Date:		2014 Dec 09
	Update:		2015 Mar 27
*/

#ifndef FDDNM_H
#define FDDNM_H

#include <iostream>
#include <vector>
#include <map>
#include "fddnm_dot_data.h"

class fddnm
{
public:
	fddnm();						//constructor
	fddnm(const fddnm& rhs);		//copy-constructor
	fddnm (fddnm&& rhs) noexcept;	//move-constructor
	virtual ~fddnm();				//destructor

	//copy assignment operator
	fddnm& operator=(const fddnm& rhs);
	//move Assignment operator
    fddnm& operator=(fddnm&& rhs) noexcept;

	void	setProgramOptions(int argc, char** argv);
	void	processDialogs();
	void	processVerbosity();

private:
	void	enumaerateDirectory();
	void	reconcileSearch();
	bool	isWanted(const std::string& dlgStemName) const;
	void	reconcileInclude(const std::string& include, dialog_dot_data& theData);
	void	graphivOutput(const std::string& outFileName, const std::string& dotData);

private:
	bool									quiet_output;
	bool									verbose_parse;
	bool									verbose_dot;
	// enable detailed output
	bool									detailed;
	// enable group flag for nodes connected to "end_dialog"
	bool									grouping;
	// no not imported other dialogs (e.g. 614 dialogs)
	bool									individual_files;
	// where to look for dialog files
	std::string								dirSearch;
	// displayed version of search dir
	std::string								dirDisplaySearch;
	// where to write fddnm output
	std::string								dirOutput;
	// dot - 1 of TB[default],BT,RL,LR
	std::string								drawLayout;
	// dot - 1 of dot,jpg,png,svg (more can be added)
	std::string								drawFormat;
	// prefix for output file
	std::string								filePrefix;
	// list of dialog filename paths found in search dir
	std::vector<std::string>				dlgsFound;
	// list of dialog names user wants
	std::vector<std::string>				dlgsWanted;
	// dialog data to be processed
	std::map<std::string, dialog_dot_data>	dlgsDraw;
	// map < stem file name (no ext), struct dialog_dot_data >
};

#endif	//FDDNM_H
