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
	File:		main.cpp
	Purpose:	initiate control of FDRPG dialog parsing
	Author:		Scott Furry
	Date:		2014 Nov 10
	Update:		2015 Feb 25
*/

#include <iostream>
#include "fddnm.h"

int main(int argc, char* argv[])
{
	try {
		fddnm fddnmcontrol;
		fddnmcontrol.setProgramOptions(argc, argv);
		fddnmcontrol.processDialogs();
		fddnmcontrol.processVerbosity();
	}
    catch(std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        std::cerr << "Exception of unknown type!\n";
        return 1;
    }
	return 0;
}
