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
	File:		fddnm.cpp
	Purpose:	class implementation for fddnm control
	Author:		Scott Furry
	Date:		2014 Dec 09
	Update:		2015 Mar 27
*/

#include "fddnm.h"
#include <exception>
#include <stdexcept>
#include <iomanip>
#include <utility>
#include <algorithm>
#include <fstream>
#include <boost/filesystem.hpp>			// boost - file system - path
#include <boost/program_options.hpp>	// boost - command line parsing
#include <boost/algorithm/string.hpp>	// boost - string manip/upper/lower
#include <graphviz/gvc.h>				// graphviz library

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif	//BOOST_FILESYSTEM_NO_DEPRECATED

namespace bfs = boost::filesystem;
namespace bpo = boost::program_options;
namespace ba = boost::algorithm;


const std::string DESC_VERBOSE_PRINT
	("print parsed dialog data to text file\ntext file placed in output directory\n");
const std::string DESC_VERBOSE_DOT
	("print analyzed dialog data used to create graphic to text file\ntext file placed in output directory\n");
const std::string DESC_GROUPING
	("enable \"end_dialog\" group hint in dot output\n");
const std::string DESC_DETAILED
	("enable HTML-like dot output - includeds extra node information\n");
const std::string DESC_INDIVIDUAL
	("program is to parse and process all dialog files individually\n");
const std::string DESC_DIR_SEARCH
	("search directory for dialog files\nDEFAULT: [FDRPG root]/dialogs\n");
const std::string DESC_DIR_OUTPUT
	("output directory for resulting graphic files\nDEFAULT: current directory\n");
const std::string DESC_DRAW_DIR
	("direction of graph layout\nOne of [ TB (DEFAULT) | LR | RL | BT ]\n");
const std::string DESC_DRAW_FORMAT
	("graphic format of output\nOne of [ none | dot | jpg | png | svg (DEFAULT) ]\n");
const std::string DESC_PREFIX
	("prefix to use for dot output file names\n");
const std::string DESC_DIALOG
	("dialogs to be parsed\nNOTE: no values imply all dialogs in search directory\n");

enum FORMAT_ID { ID_NONE, ID_DOT, ID_JPG, ID_PNG, ID_SVG, ID_SIZE};
const std::vector<std::string> FORMATS = { "none","dot","jpg","png","svg" };

enum LAYOUT_ID { ID_TB, ID_LR, ID_RL, ID_BT };
const std::vector<std::string> LAYOUTS = { "TB","LR","RL","BT" };

const std::string VDOT("_DOT");
const std::string VPARSE("_PARSE");
const std::string EXT_VERBOSE(".txt");
const std::string EXT_LUA(".lua");

//constructor
fddnm::fddnm() :
quiet_output(false),
verbose_parse(false),
verbose_dot(false),
detailed(false),
grouping(false),
individual_files(false),
dirSearch("../../dialogs"),
dirDisplaySearch(),
dirOutput(),
drawLayout(LAYOUTS[ID_TB]),
drawFormat(FORMATS[ID_SVG]),
filePrefix(),
dlgsFound(),
dlgsWanted(),
dlgsDraw()
{}

//copy-constructor
fddnm::fddnm(const fddnm& rhs)
{
	if(this != &rhs)
	{
		this->quiet_output		= rhs.quiet_output;
		this->verbose_parse		= rhs.verbose_parse;
		this->verbose_dot		= rhs.verbose_dot;
		this->detailed			= rhs.detailed;
		this->grouping			= rhs.grouping;
		this->individual_files	= rhs.individual_files;
		this->dirSearch			= rhs.dirSearch;
		this->dirDisplaySearch	= rhs.dirDisplaySearch;
		this->dirOutput			= rhs.dirOutput;
		this->drawLayout		= rhs.drawLayout;
		this->drawFormat		= rhs.drawFormat;
		this->filePrefix		= rhs.filePrefix;
		std::copy(	rhs.dlgsFound.begin(),
					rhs.dlgsFound.end(),
					std::back_inserter(this->dlgsFound)
		);
		std::copy(	rhs.dlgsWanted.begin(),
					rhs.dlgsWanted.end(),
					std::back_inserter(this->dlgsWanted)
		);
		std::copy(	rhs.dlgsDraw.begin(),
					rhs.dlgsDraw.end(),
					std::inserter(this->dlgsDraw, this->dlgsDraw.end())
		);
	}
}

//move-constructor
fddnm::fddnm (fddnm&& rhs) noexcept :
quiet_output(rhs.quiet_output),
verbose_parse(rhs.verbose_parse),
verbose_dot(rhs.verbose_dot),
detailed(rhs.detailed),
grouping(rhs.grouping),
individual_files(rhs.individual_files),
dirSearch(rhs.dirSearch),
dirDisplaySearch(rhs.dirDisplaySearch),
dirOutput(rhs.dirOutput),
drawLayout(rhs.drawLayout),
drawFormat(rhs.drawFormat),
filePrefix(rhs.filePrefix),
dlgsFound(rhs.dlgsFound),
dlgsWanted(rhs.dlgsWanted),
dlgsDraw(rhs.dlgsDraw)
{
	rhs.dlgsFound.clear();
	rhs.dlgsWanted.clear();
	rhs.dlgsDraw.clear();
}

//destructor
fddnm::~fddnm()
{
	this->dlgsFound.clear();
	this->dlgsWanted.clear();
	this->dlgsDraw.clear();
}

//copy assignment operator
fddnm& fddnm::operator=(const fddnm& rhs)
{
	if(this != &rhs)
	{
		this->quiet_output		= rhs.quiet_output;
		this->verbose_parse		= rhs.verbose_parse;
		this->verbose_dot		= rhs.verbose_dot;
		this->detailed			= rhs.detailed;
		this->grouping			= rhs.grouping;
		this->individual_files	= rhs.individual_files;
		this->dirSearch			= rhs.dirSearch;
		this->dirDisplaySearch	= rhs.dirDisplaySearch;
		this->dirOutput			= rhs.dirOutput;
		this->drawLayout		= rhs.drawLayout;
		this->drawFormat		= rhs.drawFormat;
		this->filePrefix		= rhs.filePrefix;
		std::copy(	rhs.dlgsFound.begin(),
					rhs.dlgsFound.end(),
					std::back_inserter(this->dlgsFound)
		);
		std::copy(	rhs.dlgsWanted.begin(),
					rhs.dlgsWanted.end(),
					std::back_inserter(this->dlgsWanted)
		);
		std::copy(	rhs.dlgsDraw.begin(),
					rhs.dlgsDraw.end(),
					std::inserter(this->dlgsDraw, this->dlgsDraw.end())
		);
	}
	return *this;
}

//move Assignment operator
fddnm& fddnm::operator=(fddnm&& rhs) noexcept
{
	this->quiet_output		= std::move(rhs.quiet_output);
	this->verbose_parse		= std::move(rhs.verbose_parse);
	this->verbose_dot		= std::move(rhs.verbose_dot);
	this->detailed			= std::move(rhs.detailed);
	this->grouping			= std::move(rhs.grouping);
	this->individual_files	= std::move(rhs.individual_files);
	this->dirSearch			= std::move(rhs.dirSearch);
	this->dirDisplaySearch	= std::move(rhs.dirDisplaySearch);
	this->dirOutput			= std::move(rhs.dirOutput);
	this->drawLayout		= std::move(rhs.drawLayout);
	this->drawFormat		= std::move(rhs.drawFormat);
	this->filePrefix		= std::move(rhs.filePrefix);
	this->dlgsFound			= std::move(rhs.dlgsFound);
	this->dlgsWanted		= std::move(rhs.dlgsWanted);
	this->dlgsDraw			= std::move(rhs.dlgsDraw);
	return *this;
}

void fddnm::setProgramOptions(int argc, char** argv)
{
	try
	{
		bpo::options_description desc("Usage", 80, 72);
		desc.add_options()
			("help,h", "show help message")
			(",q","no output to std::cout")
			("dirsearch,s",
				bpo::value<std::string>(&dirSearch),
				DESC_DIR_SEARCH.c_str())
			("diroutput,o",
				bpo::value<std::string>(&dirOutput),
				DESC_DIR_OUTPUT.c_str())
			("layout,L",
				bpo::value<std::string>(&drawLayout),
				DESC_DRAW_DIR.c_str())
			("format,T",
				bpo::value<std::string>(&drawFormat),
				DESC_DRAW_FORMAT.c_str())
			("fileprefix,F",
				bpo::value<std::string>(&filePrefix),
				DESC_PREFIX.c_str())
			("dialog,d",
				bpo::value< std::vector<std::string> >(&dlgsWanted),
				DESC_DIALOG.c_str())
			("verboseprint,P", DESC_VERBOSE_PRINT.c_str())
			("verbosedot,D", DESC_VERBOSE_DOT.c_str())
			("group,G", DESC_GROUPING.c_str())
			("extrainfo,H", DESC_DETAILED.c_str())
			("individual,I", DESC_INDIVIDUAL.c_str())

		;
		//dlgsWanted
		bpo::positional_options_description posOpt;
		posOpt.add("dialog", -1);
		bpo::variables_map vm;
		bpo::store(bpo::command_line_parser(argc, argv).options(desc).positional(posOpt).run(), vm);

		if (vm.count("help"))
		{
			std::cout << desc << std::endl;
			return;
		}
		// defer bpo::notify until after verions/help options parsed as notify throws
		// exception over missing values regardless of help/ver [returning] options
		bpo::notify(vm);
		if (vm.count("-q"))				quiet_output		= true;
		if (vm.count("verboseprint"))	verbose_parse		= true;
		if (vm.count("verbosedot"))		verbose_dot			= true;
		if (vm.count("group"))			grouping			= true;
		if (vm.count("extrainfo"))		detailed			= true;
		if (vm.count("individual"))		individual_files	= true;

		// validate layout requested
		bool goodLayout = false;
		std::string drawLayoutOrig(drawLayout);
		ba::to_upper(drawLayout);
		for (auto& layout: LAYOUTS)
		{
			if (layout.compare(drawLayout) != 0) continue;
			drawLayout = layout;
			goodLayout = true;
			break;
		}
		if(!goodLayout)
			throw std::runtime_error(drawLayoutOrig + " is not a recognized layout direction");

		// validate format requested
		bool goodFormat = false;
		std::string drawFormatOrig(drawFormat);
		ba::to_lower(drawFormat);
		for (auto& format: FORMATS)
		{
			if (format.compare(drawFormat) != 0) continue;
			drawFormat = format;
			goodFormat = true;
			break;
		}
		if(!goodFormat)
			throw std::runtime_error(drawFormatOrig + " is not a recognized output format");

		// check dirSearch and change to absolute path
		if(dirSearch.empty()) dirSearch = "../../dialogs";
		bfs::path localpath(dirSearch);
		if(localpath.is_relative())		localpath = bfs::canonical(localpath);
		dirSearch = localpath.string();
		if(!bfs::exists(localpath) && (!bfs::is_directory(localpath)))
			throw std::runtime_error(dirSearch + " is not valid path");
		dirDisplaySearch = localpath.filename().string();

		// check dirOutput and change to absolute path
		if(dirOutput.empty()) dirOutput = "./";
		bfs::path outpath(dirOutput);
		if(outpath.is_relative())		outpath = bfs::canonical(outpath);
		dirOutput = outpath.string();
		if (!(bfs::is_directory(outpath)))
			throw std::runtime_error(dirOutput + " is not a directory");

		// completed cmd line parsing - can user actions be completed?
		enumaerateDirectory();
		// if dlgsFound is empty -> no files found
		if ( dlgsFound.empty() )
			throw std::runtime_error("No Dialog Files Found in " + dirSearch);

		reconcileSearch();
		// if dlgsDraw is empty -> no files found to be processed
		if ( dlgsDraw.empty() )
			throw std::runtime_error("Unable to determine dialogs to parse. Please check input");

		// prefixfile - check contains valid filesystem chars
		if ((!filePrefix.empty()) && (!bfs::portable_name(filePrefix)))
			throw std::runtime_error("File prefix contains characters that may not be appropriate for a file name");

		if (!quiet_output)
		{
			std::cout	<< "Search Path: " << dirSearch << std::endl
						<< "Displayed Search Path: " << dirDisplaySearch << std::endl
						<< "Output Path: " << dirOutput << std::endl
						<< "Layout: " << drawLayout
						<< "\tFormat: " << drawFormat
						<< "\tIndiviual Files: " << std::boolalpha << individual_files
						<< std::endl << std::endl;
		}
	}
	catch(...)
	{
		throw;
	}
}

void fddnm::enumaerateDirectory()
{
	bfs::path p(dirSearch);
	// get paths of all files/directories under dirSearch;
	std::vector<bfs::path> tempfiles;
	std::copy(	bfs::directory_iterator(p),
				bfs::directory_iterator(),
				std::back_inserter(tempfiles)
	);
	for (auto& localpath: tempfiles)
	{
		// filter list of files in directory
		// do not want directories or files without *.lua extentions
		if(bfs::is_directory(localpath)) continue;
		if(EXT_LUA.compare(localpath.extension().string()) != 0) continue;
		dlgsFound.push_back(localpath.string());
	}
	std::sort(dlgsFound.begin(), dlgsFound.end());
}

void fddnm::reconcileSearch()
{
	for (auto& strDlgPath: dlgsFound)
	{
		// leverage boost::filesystem path decomposition
		bfs::path localpath(strDlgPath);
		bfs::path pathDisplay(dirDisplaySearch);
		std::string stemName(localpath.stem().string());
		// dlgsWanted.empty() == true - All Dialogs wanted
		if ((dlgsWanted.empty()) || (isWanted(stemName)))
		{
			dialog_dot_data theData;
			theData.dialogFileName = localpath.string();

			// setup for shorten display of file path
			pathDisplay /= localpath.filename().string();
			theData.displayPath = pathDisplay.string();

			theData.dialogName = stemName;
			theData.dotName = dialog_dot::toDotStr(stemName);
			dlgsDraw[theData.dialogName] = theData;
		}
	}
}

bool fddnm::isWanted(const std::string& dlgStemName) const
{
	bool bReturn = false;
	for (auto& wantedDialogName: dlgsWanted)
	{
		if (wantedDialogName.compare(dlgStemName) != 0) continue;
		bReturn = true;
		break;
	}
	return bReturn;
}

void fddnm::reconcileInclude(const std::string& include, dialog_dot_data& theData)
{
	// re-run process to "reconcileSearch" and "processDialog"
	// append node data to existing dlgsDraw Object
	dialog_dot_data workingData;
	for (auto& strDlgPath: dlgsFound)
	{
		// leverage boost::filesystem path decomposition
		bfs::path localpath(strDlgPath);
		bfs::path pathDisplay(dirDisplaySearch);
		std::string stemName(localpath.stem().string());
		if (include.compare(stemName) == 0)
		{
			// found included dialog
			workingData.dialogFileName = localpath.string();
			// setup for shorten display of file path
			pathDisplay /= localpath.filename().string();
			workingData.displayPath = pathDisplay.string();

			workingData.dialogName = stemName;
			workingData.dotName = dialog_dot::toDotStr(stemName);
			workingData.dialogData.setFileName(workingData.dialogFileName);
			workingData.dialogData.setCharName(workingData.dialogName);
			if (!quiet_output)
			{
				std::cout.flags ( std::ios::left);
				std::cout	<< "Parsing\t.."
							<< std::setw(30)
							<< workingData.displayPath
							<< std::endl;
			}
			if(workingData.dialogData.parseFile())
			{
				std::string errMsg;
				workingData.dialogData.parseNodes(errMsg);
				if ((!quiet_output) && (!errMsg.empty()))
				{
					std::cout	<< workingData.dialogName
								<< " node parse errors:"
								<< std::endl
								<< errMsg
								<< std::endl;
				}
				filenodes trxfNodes = workingData.dialogData.getAllNodes();
				theData.dialogData.addNodes(include,trxfNodes);
				if (!quiet_output)
				{
					std::cout	<< std::setw(30)
								<< stemName
								<< "\tDetected "
								<< workingData.dialogData.getNodeCount()
								<< " nodes"
								<< std::endl;
				}
			}
		}
	}
}

void fddnm::processDialogs()
{
	// assumptions
	// 1) passed in values have been parsed and are correct
	// 2) dlgsDraw has been populated completely
	//
	// dlgsDraw == map < stem file name (no ext), struct dialog_dot_data >
	//
	for (std::pair<const std::string, dialog_dot_data>& x : dlgsDraw)
	{
		std::string filestem = x.first;
		dialog_dot_data item = x.second;

		if((!individual_files) and (item.skipRender)) continue;

		item.dialogData.setFileName(item.dialogFileName);
		item.dialogData.setCharName(item.dialogName);
		if (!quiet_output)
		{
			std::cout.flags ( std::ios::left);
			std::cout	<< "Parsing\t.."
						<< std::setw(30)
						<< item.displayPath
						<< std::endl;
		}
		if(item.dialogData.parseFile())
		{
			std::string errMsg;
			item.dialogData.parseNodes(errMsg);
			if((!individual_files) && (item.dialogData.hasIncludes()))
			{
				linetext inclFiles = item.dialogData.getInclFiles();
				for(auto& inclfile : inclFiles)
				{
					// append parsing to existing object
					reconcileInclude(inclfile, item);
					dlgsDraw[inclfile].skipRender = true;
				}
			}
			if ((!quiet_output) && (!errMsg.empty()))
			{
				std::cout	<< item.dialogName
							<< " node parse errors:"
							<< std::endl
							<< errMsg
							<< std::endl;
			}
			if (!quiet_output)
			{
				std::cout	<< std::setw(30)
							<< filestem
							<< "\tDetected "
							<< item.dialogData.getNodeCount()
							<< " nodes"
							<< "\tParsed "
							<< item.dialogData.getNodeSize()
							<< " nodes"
							<< std::endl
							<< std::endl;
			}
		}
		else
		{
			throw std::runtime_error("Unable to parse: " + item.displayPath);
		}
		// update saved data
		x.second = item;
	}

	// process dialogs for dot data
	for (std::pair<const std::string,dialog_dot_data>& x: dlgsDraw)
	{
		//std::string filestem = x.first;
		dialog_dot_data item = x.second;

		if((!individual_files) and (item.skipRender)) continue;

		item.dotData.setNodePrefix("a" + item.dotName + "_");
		item.dotData.setDrawDirection(drawLayout);

		std::string strError = item.dotData.setFile(item.dialogData);
		if (!strError.empty())
		{
			std::cerr	<< "Dot Transform Error(" << item.dialogName << ")"
						<< std::endl
						<< strError
						<< std::endl;
		}
		// update object data
		x.second = item;

		// build up output file name for diagram
		bfs::path outFilePath(dirOutput);
		std::string ext("");

		if(drawFormat.compare(FORMATS[ID_NONE]) == 0)
		{
			ext = EXT_VERBOSE;
		}
		else
		{
			ext = "." + drawFormat;
		}
		bfs::path outFileName(filePrefix + item.dotName + ext);
		outFilePath /= outFileName;
		if ( FORMATS[ID_NONE].compare(drawFormat) != 0)
		{
			if (!quiet_output)
			{
				std::cout << "Rendering to file: " << outFileName.string() << std::endl;
			}
			graphivOutput(outFilePath.string(), item.dotData.getDotContent(this->detailed, this->grouping));
		}
	}
}

void fddnm::processVerbosity()
{
	try
	{
		for (const std::pair<const std::string,dialog_dot_data>& x: dlgsDraw)
		{
			std::string filestem = x.first;
			dialog_dot_data item = x.second;
			if (verbose_parse)
			{
				// build up output file name for verbose parse output
				bfs::path outFileName(dirOutput);
				outFileName = outFileName / bfs::path(filePrefix + item.dotName + VPARSE + EXT_VERBOSE);
				// dump to file
				std::ofstream output(outFileName.string());
				if (output.is_open())
				{
					output	<< "=====" << std::endl
							<< "parse data: " << filestem << std::endl
							<< item.dialogData.printFile() << std::endl
							<< "=====" << std::endl;
					output.close();
				}
			}
			if (verbose_dot)
			{
				// build up output file name for verbose dot output
				bfs::path outFileName(dirOutput);
				outFileName = outFileName / bfs::path(filePrefix + item.dotName + VDOT + EXT_VERBOSE);
				// dump to file
				std::ofstream output(outFileName.string());
				if (output.is_open())
				{
					output	<< "=====" << std::endl
							<< "dot data: " << filestem << std::endl
							<< item.dotData.printDotData() << std::endl
							<< "=====" << std::endl;
					output.close();
				}
			}
		}
	}
	catch(...)
	{
		// some error occurred during conversion
		throw;
	}
}

void fddnm::graphivOutput(const std::string& outFileName, const std::string& dotData)
{
	if ( 0 == drawFormat.compare(FORMATS[ID_NONE]) ) return;
	else if ( 0 == drawFormat.compare(FORMATS[ID_DOT]) )
	{
		// user wants dot format output
		// dump dotData to file
		std::ofstream output(outFileName);
		if (output.is_open())
		{
			output << dotData << std::endl;
			output.close();
		}
		else
		{
			throw std::runtime_error("A problem occurred creating file: " + outFileName);
		}
	}
	else
	{
		// user wants other image format output. dot-language data converted
		// by graphviz library functions to user disired graphics format
		// see "Graphviz Library Manual" for details
		// www.graphviz.org/doc/libguide/libguide.pdf
		//
		// Also see Graphviz API documentation at
		// http://www.graphviz.org/pub/graphviz/development/doxygen/html/globals.html
		//
		// create graphviz context - initialize graphviz
		GVC_t* gvc = gvContext();
		if( gvc == nullptr )
		{
			throw std::runtime_error("A problem creating gvContext occurred. File: " + outFileName);
		}
		// to avoid valgrind errors
		// const char* from std::string().c_str()
		const char* dotFormatLabel = FORMATS[ID_DOT].c_str();
		const char* userDotData = dotData.c_str();
		const char* userFormat = drawFormat.c_str();
		const char* userFileName = outFileName.c_str();
		// read graph (dot) data from memory
		Agraph_t* G = agmemread(userDotData);
		if( G == nullptr )
		{
			gvFreeContext(gvc);
			throw std::runtime_error("A problem occurred importing dot file data: " + outFileName);
		}

		// layout dot-language data into a graph
		if(0 != gvLayout(gvc, G, dotFormatLabel) )
		{
			gvFreeLayout(gvc, G);
			agclose(G);
			gvFreeContext(gvc);
			throw std::runtime_error("A problem occurred with graphic layout of file: " + outFileName);
		}

		// render the graph(dot) as user specified format into file
		if( 0 != gvRenderFilename(gvc, G, userFormat, userFileName) )
		{
			gvFreeLayout(gvc, G);
			agclose(G);
			gvFreeContext(gvc);
			throw std::runtime_error("A problem occurred rendering to file: " + outFileName);
		}
		// cleanup and exit
		gvFreeLayout(gvc, G);
		agclose(G);
		gvFreeContext(gvc);
	}
}
