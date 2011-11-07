/* 
 *
 *
 *   Copyright (c) 2003 Johannes Prix
 *   Copyright (c) 2004-2010 Arthur Huillet 
 *
 *  This file is part of Freedroid
 *
 *  Freedroid is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Freedroid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Freedroid; see the file COPYING. If not, write to the 
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 *
 */

/**
 * This file contains some text code, that is also needed in the 
 * dialog editor.
 */

#define _text_public_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"

#include <zlib.h>

extern int Number_Of_Item_Types;
extern int debug_level;

/** 
 * This function works a malloc, except that it also checks for
 * success and terminates in case of "out of memory", so we don't
 * need to do this always in the code.
 */
void *MyMalloc(long Mamount)
{
	void *Mptr = NULL;

	// make Gnu-compatible even if on a broken system:
	if (Mamount == 0)
		Mamount = 1;

	if ((Mptr = calloc(1, (size_t) Mamount)) == NULL) {
		fprintf(stderr, " MyMalloc(%ld) did not succeed!\n", Mamount);
		fflush(stderr);
		Terminate(EXIT_FAILURE, TRUE);
	}

	return Mptr;

};				// void* MyMalloc ( long Mamount )

/**
 * This function is used for debugging purposes.  It writes the
 * given string on the screen, or simply does
 * nothing according to currently set debug level.
 */
void DebugPrintf(int db_level, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	if (db_level <= debug_level) {
		vfprintf(stderr, fmt, args);
	}

	va_end(args);
};				// void DebugPrintf ( int db_level, char *fmt, ...)

/**
 * This function should help to simplify and standardize the many error
 * messages possible in FreedroidRPG.
 */
void ErrorMessage(const char *FunctionName, const char *fmt, int InformDevelopers, int IsFatal, ...)
{
	va_list args;
	va_start(args, IsFatal);

	fprintf(stderr, "\n----------------------------------------------------------------------\n\
FreedroidRPG has encountered a problem:\n");
	fprintf(stderr, "In function: %s.\n", FunctionName);
	fprintf(stderr, "FreedroidRPG package and version number: %s %s.\n", PACKAGE, VERSION);
	vfprintf(stderr, fmt, args);

	if (InformDevelopers) {
		fprintf(stderr, "\
\n\
\n\
If you encounter this message, please inform the FreedroidRPG developers\n\
about the problem, by either\n\
\n\
  sending e-mail to:\n\
    freedroid-discussion AT lists.sourceforge.net\n\
  mention it to someone of the developers on our IRC channel:\n\
    channel: #freedroid on irc.freenode.net\n\
  post on our forum at:\n\
    https://sourceforge.net/apps/phpbb/freedroid\n\
  report a bug on our tracker at:\n\
    http://bugs.freedroid.org/\n\
\n\
Thanks a lot!\n\n");
	}

	if (IsFatal) {
		fprintf(stderr, "FreedroidRPG will terminate now to draw attention to the problems it could\n\
not resolve.  Sorry if that interrupts a major game of yours...\n");
	} else {
		fprintf(stderr, "The problem mentioned above is not fatal, we continue the execution.\n");
		fprintf(stderr, "----------------------------------------------------------------------\n");
	}

	va_end(args);

	if (IsFatal)
		Terminate(EXIT_FAILURE, TRUE);

};				// void ErrorMessage ( ... )

/**
 * This function does something similar to memmem.  Indeed, it would be
 * best perhaps if it did exactly the same as memmem, but since we do not
 * use gnu extensions for compatibility reasons, we go this way.
 *
 * Be careful with using this function for searching just one byte!!
 *
 * Be careful with using this function for non-string NEEDLE!!
 *
 * Haystack can of course have ANY form!!!
 *
 */
void *MyMemmem(char *haystack, size_t haystacklen, char *needle, size_t needlelen)
{
	char *NextFoundPointer;
	void *MatchPointer;
	size_t SearchPos = 0;

	while (haystacklen - SearchPos > 0) {
		// We search for the first match OF THE FIRST CHARACTER of needle
		//
		NextFoundPointer = memchr(haystack + SearchPos, needle[0], haystacklen - SearchPos);

		// if not even that was found, we can immediately return and report our failure to find it
		//
		if (NextFoundPointer == NULL)
			return (NULL);

		// Otherwise we see, if also the rest of the strings match this time ASSUMING THEY ARE STRINGS!
		// In case of a match, we can return immediately
		//
		MatchPointer = strstr(NextFoundPointer, needle);
		if (MatchPointer != NULL)
			return (MatchPointer);

		// At this point, we know that we had no luck with this one occasion of a first-character-match
		// and must continue after this one occasion with our search
		SearchPos = NextFoundPointer - haystack + 1;
	}

	return (NULL);
};				// void *MyMemmem ( ... );

/**
 * This function looks for a string begin indicator and takes the string
 * from after there up to a string end indicator and mallocs memory for
 * it, copies it there and returns it.
 * The original source string specified should in no way be modified.
 * Returns 0 if the prefix string is not present
 */
char *ReadAndMallocStringFromDataOptional(char *SearchString, const char *StartIndicationString, const char *EndIndicationString)
{
	char *SearchPointer;
	char *EndOfStringPointer;
	char *ReturnString = "";
	int StringLength;

	SearchPointer = strstr(SearchString, StartIndicationString);

	if (!SearchPointer)
		return 0;
	else {
		// Now we move to the beginning
		SearchPointer += strlen(StartIndicationString);

		// Now we move to the end with the end pointer
		EndOfStringPointer = strstr(SearchPointer, EndIndicationString);
		if (!EndOfStringPointer)
			return 0;

		// Now we allocate memory and copy the string...
		// delete_one_dialog_option() doesn't free empty strings so don't
		// malloc those.
		if ((StringLength = (EndOfStringPointer - SearchPointer))) {
			ReturnString = MyMalloc(StringLength + 1);
			strncpy(ReturnString, SearchPointer, StringLength);
			ReturnString[StringLength] = 0;
		} else {
			ReturnString = "";
		}
	}
	return ReturnString;
}

/**
 * This function looks for a string begin indicator and takes the string
 * from after there up to a string end indicator and mallocs memory for
 * it, copies it there and returns it.
 * The original source string specified should in no way be modified.
 * The lack of searched string is fatal.
 */
char *ReadAndMallocStringFromData(char *SearchString, const char *StartIndicationString, const char *EndIndicationString)
{
	char *result = ReadAndMallocStringFromDataOptional(SearchString, StartIndicationString, EndIndicationString);
	if (!result) {
		fprintf(stderr, "\n\nStartIndicationString: '%s'\nEndIndicationString: '%s'\n", StartIndicationString, EndIndicationString);
		ErrorMessage(__FUNCTION__, "\
The string that is supposed to prefix an entry in a text data file\n\
of FreedroidRPG was not found within this text data file.\n\
This indicates some corruption in the data file in question.", PLEASE_INFORM, IS_FATAL);
	}
	return result;
};				// char* ReadAndMallocStringFromData ( ... )

/**
 * This function counts the number of occurences of a string in a given
 * other string.
 */
int CountStringOccurences(char *SearchString, const char *TargetString)
{
	int Counter = 0;
	char *CountPointer;

	CountPointer = SearchString;

	while ((CountPointer = strstr(CountPointer, TargetString)) != NULL) {
		CountPointer += strlen(TargetString);
		Counter++;
	}
	return (Counter);
};				// CountStringOccurences ( char* SearchString , char* TargetString ) 

/**
 * This function read in a file with the specified name, allocated 
 * memory for it of course, looks for the file end string and then
 * terminates the whole read in file with a 0 character, so that it
 * can easily be treated like a common string.
 */
char *ReadAndMallocAndTerminateFile(const char *filename, const char *File_End_String)
{
	FILE *DataFile;
	char *Data;
	char *ReadPointer;
	long MemoryAmount;

	// Read the whole theme data to memory.  We use binary mode, as we
	// don't want to have to deal with any carriage return/line feed 
	// convention mess on win32 or something...
	// 
	if ((DataFile = fopen(filename, "rb")) == NULL) {
		fprintf(stderr, "\n\nfilename: '%s'\n", filename);

		ErrorMessage(__FUNCTION__, "\
FreedroidRPG was unable to open a given text file,\n\
that should be there and should be accessible.\n\
This indicates a serious bug in this installation of FreedroidRPG.", PLEASE_INFORM, IS_FATAL);
	} else {
		DebugPrintf(1, "\nchar* ReadAndMallocAndTerminateFile ( char* filename ) : Opening file succeeded...");
	}

	int filelen = FS_filelength(DataFile);
	MemoryAmount = filelen + 100;
	Data = (char *)MyMalloc(MemoryAmount);

	if (fread(Data, 1, MemoryAmount, DataFile) < filelen && ferror(DataFile)) {
		ErrorMessage(__FUNCTION__, "\
		FreedroidRPG was unable to read a given text file, that should be there and\n\
		should be accessible.\n\
		Filename: %s", PLEASE_INFORM, IS_FATAL, filename);
	} else {
		DebugPrintf(1, "\n%s(): Reading file succeeded...", __FUNCTION__);
	}

	if (fclose(DataFile) == EOF) {
		fprintf(stderr, "\n\nfilename: '%s'\n", filename);
		ErrorMessage(__FUNCTION__, "\
		FreedroidRPG was unable to close a given text file, that should be there and\n\
		should be accessible.\n\
		This indicates a strange bug in this installation of Freedroid, that is\n\
		very likely a problem with the file/directory permissions of the files\n\
		belonging to FreedroidRPG.", PLEASE_INFORM, IS_FATAL);
	} else {
		DebugPrintf(1, "\n%s(): file closed successfully...\n", __FUNCTION__);
	}

	// NOTE: Since we do not assume to always have pure text files here, we switched to
	// MyMemmem, so that we can handle 0 entries in the middle of the file content as well
	if (File_End_String) {
		if ((ReadPointer =
		     MyMemmem(Data, (size_t) MemoryAmount, (char *)File_End_String, (size_t) strlen(File_End_String))) == NULL) {
			fprintf(stderr, "\n\nfilename: '%s'\n", filename);
			fprintf(stderr, "File_End_String: '%s'\n", File_End_String);

			ErrorMessage(__FUNCTION__, "\
		    FreedroidRPG was unable to find the string, that should indicate the end of\n\
		    the given text file within this file.\n\
		    This indicates a corrupt or outdated data or saved game file.", PLEASE_INFORM, IS_FATAL);
		} else {
			ReadPointer[0] = 0;
		}
	} else
		Data[MemoryAmount - 100] = 0;

	return (Data);
}

/**
 * This function tries to locate a string in some given data string.
 * The data string is assumed to be null terminated.  Otherwise SEGFAULTS
 * might happen.
 * 
 * The return value is a pointer to the first instance where the substring
 * we are searching is found in the main text.
 */
char *LocateStringInData(char *SearchBeginPointer, const char *SearchTextPointer)
{
	char *temp;

	if ((temp = strstr(SearchBeginPointer, SearchTextPointer)) == NULL) {
		fprintf(stderr, "\n\nSearchTextPointer: '%s'\n", SearchTextPointer);
		ErrorMessage(__FUNCTION__, "\
The string that was supposed to be in the text data file could not be found.\n\
This indicates a corrupted or seriously outdated game data or saved game file.", PLEASE_INFORM, IS_FATAL);
	}

	return (temp);

};				// char* LocateStringInData ( ... )

/**
 * This function should analyze a given passage of text, locate an 
 * indicator for a value, and read in the value.
 */
void
ReadValueFromStringWithDefault(char *SearchBeginPointer, const char *ValuePreceedText, const char *FormatString,
			       const char *DefaultValueString, void *TargetValue, char *EndOfSearchSectionPointer)
{
	char OldTerminaterCharValue = 0;
	const char *SourceLocation;

	// We shortly make a termination char into the string.
	if (EndOfSearchSectionPointer) {
		OldTerminaterCharValue = EndOfSearchSectionPointer[0];
		EndOfSearchSectionPointer[0] = 0;
	}

	// Now we locate the spot, where we finally will find our value
	SourceLocation = strstr(SearchBeginPointer, ValuePreceedText);
	if (SourceLocation)
		SourceLocation += strlen(ValuePreceedText);
	else
		SourceLocation = DefaultValueString;

	// Attention!!! 
	// Now we try to read in the value!!!
	//
	if (sscanf(SourceLocation, FormatString, TargetValue) == EOF) {
		fprintf(stderr, "\n\nFormatString: '%s'\n", FormatString);
		fprintf(stderr, "ValuePreceedText: '%s'\n", ValuePreceedText);
		ErrorMessage(__FUNCTION__, "\
sscanf using a certain format string failed!\n\
This indicates a corrupted or seriously outdated game data or saved game file.", PLEASE_INFORM, IS_FATAL);
	}
	// Now that we are done, we restore the given SearchArea to former glory
	if (EndOfSearchSectionPointer) {
		EndOfSearchSectionPointer[0] = OldTerminaterCharValue;
	}
}

/**
 * This function should analyze a given passage of text, locate an 
 * indicator for a value, and read in the value.
 */
void
ReadValueFromString(char *SearchBeginPointer, const char *ValuePreceedText, const char *FormatString, void *TargetValue,
		    char *EndOfSearchSectionPointer)
{
	char OldTerminaterCharValue = 0;
	char *SourceLocation;

	// We shortly make a termination char into the string if needed
	if (EndOfSearchSectionPointer) {
		OldTerminaterCharValue = EndOfSearchSectionPointer[0];
		EndOfSearchSectionPointer[0] = 0;
	}
	// Now we locate the spot, where we finally will find our value
	SourceLocation = LocateStringInData(SearchBeginPointer, ValuePreceedText);
	SourceLocation += strlen(ValuePreceedText);

	// Attention!!! 
	// Now we try to read in the value!!!
	//
	if (sscanf(SourceLocation, FormatString, TargetValue) == EOF) {
		fprintf(stderr, "\n\nFormatString: '%s'\n", FormatString);
		fprintf(stderr, "ValuePreceedText: '%s'\n", ValuePreceedText);
		ErrorMessage(__FUNCTION__, "\
sscanf using a certain format string failed!\n\
This indicates a corrupted or seriously outdated game data or saved game file.", PLEASE_INFORM, IS_FATAL);
	}

	if (EndOfSearchSectionPointer) {
		// Now that we are done, we restore the given SearchArea to former glory
		EndOfSearchSectionPointer[0] = OldTerminaterCharValue;
	}

};				// void ReadValueFromString( ... )

/** 
 * This function is for reading in a range of positive values from a string with a default.
 * 
 * If there is no string, the default value is assigned and it returns 1, with no warning.
 *
 * If there is a string with garbled input (negative minimum value, or minimum>maximum), it warns,
 * and assigns the default, and a 2 is returned.
 *
 * A valid string, with no errors means it returns a 0.
*/

int ReadRangeFromString(char *SearchString, const char *StartIndicationString, const char *EndIndicationString, int *min, int *max, int default_val)
{
	char *ptr;
	int return_value = 0;

	// First, see if we have a valid search string
	char *search_ptr = ReadAndMallocStringFromDataOptional(SearchString, StartIndicationString, EndIndicationString);
	if (!search_ptr) {
		//Search string was not valid, set everything to default value.
		*min = *max = default_val;
		return 1;
	}

	*min = strtol(search_ptr, &ptr, 10);
	if (*ptr == '-') {
		ptr++;
		*max = strtol(ptr, NULL, 10);
	} else {
		*max = *min;
	}

	//Handle corrupted values
	if (*min < 0) {
		ErrorMessage(__FUNCTION__, "\
The value read in as a minimum (%d) is a negative number.\n\
This most likely means corrupted data.\n\
Setting both the maximum and minimum to the default value (%d).", NO_NEED_TO_INFORM, IS_WARNING_ONLY, *min, default_val);
		*min = *max = default_val;
		return_value = 2;
	} else if (*max < *min) {
		ErrorMessage(__FUNCTION__, "\
The value read in as a maximum (%d) is less than the minimum (%d).\n\
This most likely means corrupted data.\n\
Setting both the maximum and minimum to the default value (%d).", NO_NEED_TO_INFORM, IS_WARNING_ONLY, *max, *min, default_val);
		*min = *max = default_val;
		return_value = 2;
	}

	free(search_ptr);
	return return_value;
}

/**
 * This function does the rotation of a given vector by a given angle.
 * The vector is a vector.
 * The angle given is in DEGREE MEASURE, i.e. 0-360 or so, maybe more or
 * less than that, but you get what I mean...
 */
void RotateVectorByAngle(moderately_finepoint * vector, float rot_angle)
{
	moderately_finepoint new_vect;
	float rad_angle;

	if (rot_angle == 0)
		return;

	rad_angle = rot_angle * (M_PI / 180.0);

	DebugPrintf(2, "\n RAD_ANGLE : %f ", rad_angle);
	new_vect.x = sin(rad_angle) * vector->y + cos(rad_angle) * vector->x;
	new_vect.y = cos(rad_angle) * vector->y - sin(rad_angle) * vector->x;
	vector->x = new_vect.x;
	vector->y = new_vect.y;

};				// void RotateVectorByAngle ( ... )

/*----------------------------------------------------------------------
 * Copyright (C) 1997-2001 Id Software, Inc., under GPL
 *
 * FS_filelength().. (taken from quake2)
 * 		contrary to stat() this fct is nice and portable, 
 *----------------------------------------------------------------------*/
int FS_filelength(FILE * f)
{
	int pos;
	int end;

	pos = ftell(f);
	fseek(f, 0, SEEK_END);
	end = ftell(f);
	fseek(f, pos, SEEK_SET);

	// DebugPrintf ( -4 , "\n%s(): file length: %d." , __FUNCTION__ , end );

	return end;
};				// int FS_filelength (FILE *f)

/*-------------------------------------------------------------------------
 * Inflate a given stream using zlib
 *
 * Takes DataFile file and points DataBuffer to a buffer containing the 
 * uncompressed data. Sets '*size' to the size of said uncompressed data, if size 
 * is not NULL.
 *
 * Returns nonzero in case of error. 
 ***/
int inflate_stream(FILE * DataFile, unsigned char **DataBuffer, int *size)
{
	int filelen = FS_filelength(DataFile);
	unsigned char *src = MyMalloc(filelen + 1);
	int cursz = 1048576;	//start with 1MB
	unsigned char *temp_dbuffer = malloc(cursz);
	if (fread(src, filelen, 1, DataFile) != 1) {
		ErrorMessage(__FUNCTION__, "Error reading compressed data stream.", PLEASE_INFORM, IS_WARNING_ONLY);
		free(temp_dbuffer);
		free(src);
		return -1;
	}

	int ret;
	z_stream strm;

	/* copy paste of public domain tool "zpipe" of which a copy is in fdRPG source tree */
	/* big thanks to zlib authors */
	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = filelen;
	strm.next_in = src;
	strm.avail_out = cursz;
	strm.next_out = (Bytef *) temp_dbuffer;

	ret = inflateInit2(&strm, 47);
	if (ret != Z_OK) {
		ErrorMessage(__FUNCTION__, "\
		zlib was unable to start decompressing a stream.\n\
		This indicates a serious bug in this installation of FreedroidRPG.", PLEASE_INFORM, IS_WARNING_ONLY);
		free(temp_dbuffer);
		free(src);
		return -1;
	}

	do {
		if (!strm.avail_out) {	//out of memory? increase
			temp_dbuffer = realloc(temp_dbuffer, cursz + 1048576);	//increase size by 1MB
			strm.next_out = temp_dbuffer + cursz;
			cursz += 1048576;
			strm.avail_out += 1048576;
		}
		ret = inflate(&strm, Z_NO_FLUSH);
		switch (ret) {
		case Z_OK:
			break;
		case Z_NEED_DICT:
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			(void)inflateEnd(&strm);

			ErrorMessage(__FUNCTION__, "\
			zlib was unable to decompress a stream\n\
			This indicates a serious bug in this installation of FreedroidRPG.", PLEASE_INFORM, IS_WARNING_ONLY);
			free(temp_dbuffer);
			free(src);
			return -1;
			break;
		}
	} while (ret != Z_STREAM_END);

	(*DataBuffer) = (unsigned char *)malloc(strm.total_out + 1);
	memcpy(*DataBuffer, temp_dbuffer, strm.total_out);
	(*DataBuffer)[strm.total_out] = 0;

	if (size != NULL)
		*size = strm.total_out;

	(void)inflateEnd(&strm);
	free(src);
	free(temp_dbuffer);

	return 0;
}

int deflate_to_stream(unsigned char *source_buffer, int size, FILE *dest)
{
    int ret;
    unsigned have;
    z_stream strm;
#define CHUNK 16384
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
	strm.avail_in = size;
	strm.next_in = source_buffer;

    ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY);
	if (ret != Z_OK) {
		ErrorMessage(__FUNCTION__, "\
		zlib was unable to start compressing a string.", PLEASE_INFORM, IS_WARNING_ONLY);
		return -1;
	}

	/* run deflate() on input until output buffer not full, finish
	   compression if all of source has been read in */
	do {
		strm.avail_out = CHUNK;
		strm.next_out = out;
		ret = deflate(&strm, Z_FINISH);    /* no bad return value */
		have = CHUNK - strm.avail_out;
		if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
			(void)deflateEnd(&strm);
			return -1;
		}
	} while (strm.avail_out == 0);

    /* clean up and return */
    (void)deflateEnd(&strm);

	return 0;
}

#undef _text_public_c
