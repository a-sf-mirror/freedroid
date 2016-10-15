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

#define _text_public_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"

#include <zlib.h>

extern int debug_level;

// A store for alert window or console error messages.
// Used in alert_once_window() and error_once_window() to avoid to display
// several the same message.

struct msg_store_item {
	int when;                 // ONCE_PER_RUN / ONCE_PER_GAME - see defs.h
	struct auto_string *msg;  // The message already displayed.
};

static struct dynarray *msg_store = NULL;

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
		Terminate(EXIT_FAILURE);
	}

	return Mptr;

};				// void* MyMalloc ( long Mamount )

/**
 * This function works as strdup, except that it also checks for
 * success and terminates in case of "out of memory", so we don't
 * need to do this always in the code.
 */
char *my_strdup(const char *src)
{
	if (src == NULL)
		return NULL;

	char *dst = strdup(src);
	if (dst == NULL) {
		fprintf(stderr, " my_strdup() did not succeed!\n");
		fflush(stderr);
		Terminate(EXIT_FAILURE);
	}

	return dst;
}

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

/*
 * Check if a msg is already in the error msg store. If not, register it.
 * Return TRUE if the msg was not yet in the store, FALSE otherwise.
 */
static int _first_use_of_msg(int when, struct auto_string *msg)
{
	// A dynarray is used to store past alerts/errors, to remember which message was
	// already displayed.

	// On first use, initialize the msg store.

	if (!msg_store)
		msg_store = dynarray_alloc(10, sizeof(struct msg_store_item));

	// If the message is already in the store, immediately return FALSE

	int i;
	for (i = 0; i < msg_store->size; i++) {
		struct msg_store_item *stored_item = (struct msg_store_item *)dynarray_member(msg_store, i, sizeof(struct msg_store_item));
		if (!strcmp(msg->value, stored_item->msg->value)) {
			return FALSE;
		}
	}

	// The message was not yet registered. Store it and return TRUE

	struct msg_store_item *stored_item = (struct msg_store_item *)MyMalloc(sizeof(struct msg_store_item));
	stored_item->when = when;
	stored_item->msg = msg;
	dynarray_add(msg_store, stored_item, sizeof(struct msg_store_item));
	free(stored_item);

	return TRUE;
}

/**
 * Clean the alert/error message store, removing messages which are flagged as ONCE_PER_GAME
 * so that they can be redisplayed
 */
void clean_error_msg_store()
{
	if (!msg_store) // msg store not yet used
		return;

	// We have to remove the item flagged as ONCE_PER_GAME, but we have to keep
	// the others. We can not delete a dynarray member inside a loop over the
	// dynarray. We thus create a new dynarray to replace the current one,
	// containing the kept items.

	struct dynarray *new_store = dynarray_alloc(10, sizeof(struct msg_store_item));

	int i;
	for (i = 0; i < msg_store->size; i++) {
		struct msg_store_item *stored_item = (struct msg_store_item *)dynarray_member(msg_store, i, sizeof(struct msg_store_item));
		if (stored_item->when == ONCE_PER_GAME) {
			// free memory
			free_autostr(stored_item->msg);
			stored_item->msg = NULL;
		} else {
			// copy the item in the new store
			dynarray_add(new_store, stored_item, sizeof(struct msg_store_item));
		}
	}
	dynarray_free(msg_store);
	free(msg_store);
	msg_store = new_store;
}

/**
 * Free all the memory slots used by the msg_store
 */
void free_error_msg_store()
{
	if (!msg_store) // msg store was not used
		return;

	int i;
	for (i = 0; i < msg_store->size; i++) {
		struct msg_store_item *stored_item = (struct msg_store_item *)dynarray_member(msg_store, i, sizeof(struct msg_store_item));
		free_autostr(stored_item->msg);
	}
	dynarray_free(msg_store);
	free(msg_store);
}

/**
 * This function should help to simplify and standardize the many error
 * messages possible in FreedroidRPG.
 */
void error_message(const char *fn, const char *fmt, int error_type, ...)
{
	if (error_type & SILENT)
		return;

	va_list args;
	va_start(args, error_type);

	fprintf(stderr, "\n---------------------------------------------------------------------------------\n"
	                "FreedroidRPG %s encountered a problem in function: %s\n", freedroid_version, fn);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");

	va_end(args);

	if (error_type & PLEASE_INFORM) {
		fprintf(stderr, "\n\n"
		                "If you encounter this message, please inform the FreedroidRPG developers about it!\n"
		                "You can\n"
		                "  send an e-mail to                    freedroid-discussion AT lists.sourceforge.net\n"""
		                "  mention it on our IRC channel        #freedroid on irc.freenode.net\n"
		                "  or report the bug on our tracker at  http://bugs.freedroid.org/\n"
		                "\n"
		                "Thank you!\n");
	}

	if (error_type & IS_FATAL) {
		fprintf(stderr, "\nFreedroidRPG will terminate now to draw attention to the problems it could\n"
		                "not resolve. We are sorry if that interrupts a major game of yours.\n"
		                "---------------------------------------------------------------------------------\n");
		print_trace(0);
	} else {
		fprintf(stderr, "---------------------------------------------------------------------------------\n");
	}

	if ((error_type & IS_FATAL) && !(error_type & NO_TERMINATE))
		Terminate(EXIT_FAILURE);
}

/**
 * Wrapper on error_message() that avoids to redisplay several times the same error.
 * The first parameter defines if an error should never be redisplayed at
 * all (ONCE_PER_RUN), or if it has to be redisplayed once if a new savegame was
 * loaded (ONCE_PER_GAME).
 */
void error_once_message(int when, const char *fn, const char *fmt, int error_type, ...)
{
	if (error_type & SILENT)
		return;

	// Compute the error message

	va_list args;
	struct auto_string *buffer = alloc_autostr(256);
	va_start(args, error_type);
	autostr_vappend(buffer, fmt, args);
	va_end(args);

	// If the message is already in the store, immediately return

	if (!_first_use_of_msg(when, buffer)) {
		free_autostr(buffer);
		return;
	}

	// This error was not yet displayed.

	error_message(fn, "%s", error_type, buffer->value);
}

/**
 * In some cases it will be necessary to inform the user of something in
 * a big important style.  Then a popup window is suitable, with a mouse
 * click to confirm and make it go away again.
 */
void alert_window(const char *text, ...)
{
	if (!SDL_WasInit(SDL_INIT_VIDEO))
		return;

	va_list args;
	struct auto_string *buffer = alloc_autostr(256);
	va_start(args, text);
	autostr_vappend(buffer, text, args);
	va_end(args);

	Activate_Conservative_Frame_Computation();
	StoreMenuBackground(1);

	int w = 440;   // arbitrary
	int h = 60;    // arbitrary
	int x = (GameConfig.screen_width  - w) / 2;	// center of screen
	int y = (GameConfig.screen_height - h) / 5 * 2; // 2/5 of screen from top

	SDL_Event e;
	while (1) {
		SDL_Delay(1);
		RestoreMenuBackground(1);

		int r_height = show_backgrounded_text_rectangle(buffer->value, FPS_Display_Font, x, y, w, h);
		show_backgrounded_text_rectangle(_("Click to continue..."), Red_Font, x, y + r_height, w, 10);

		blit_mouse_cursor();
		our_SDL_flip_wrapper();
		save_mouse_state();

		SDL_WaitEvent(&e);

		switch (e.type) {
		case SDL_QUIT:
			Terminate(EXIT_SUCCESS);
			break;
		case SDL_KEYDOWN:
			if (e.key.keysym.sym == SDLK_SPACE || e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_ESCAPE)
				goto wait_click_and_out;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (e.button.button == 1)
				goto wait_click_and_out;
			break;
		}
	}

wait_click_and_out:
	while (SpacePressed() || EnterPressed() || EscapePressed() || MouseLeftPressed());

	our_SDL_flip_wrapper();
	RestoreMenuBackground(1);

	free_autostr(buffer);
}

/**
 * Wrapper on alert_window() that avoids to redisplay several times the same alert.
 * The first parameter defines if an alert should never be redisplayed at
 * all (ONCE_PER_RUN), or if it has to be redisplayed once if a new savegame was
 * loaded (ONCE_PER_GAME).
 */
void alert_once_window(int when, const char *text, ...)
{
	if (!SDL_WasInit(SDL_INIT_VIDEO))
		return;

	// Compute the alert message

	va_list args;
	struct auto_string *buffer = alloc_autostr(256);
	va_start(args, text);
	autostr_vappend(buffer, text, args);
	va_end(args);

	// If the message is already in the store, immediately return

	if (!_first_use_of_msg(when, buffer)) {
		free_autostr(buffer);
		return;
	}

	// This alert was not yet displayed.

	alert_window("%s", buffer->value);
}

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
void *my_memmem(char *haystack, size_t haystacklen, char *needle, size_t needlelen)
{
	size_t search_pos = 0;

	while (haystacklen - search_pos > 0) {
		// We search for the first match OF THE FIRST CHARACTER of needle

		char *next_found_pointer = memchr(haystack + search_pos, needle[0], haystacklen - search_pos);

		// if not even that was found, we can immediately return and report our failure to find it

		if (next_found_pointer == NULL)
			return (NULL);

		// Otherwise we see, if also the rest of the strings match this time ASSUMING THEY ARE STRINGS!
		// In case of a match, we can return immediately

		char *match_pointer = strstr(next_found_pointer, needle);
		if (match_pointer != NULL)
			return (void *)match_pointer;

		// At this point, we know that we had no luck with this one occasion of a first-character-match
		// and must continue after this one occasion with our search
		search_pos = next_found_pointer - haystack + 1;
	}

	return NULL;
}

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
	char *ReturnString = NULL;
	int StringLength;

	SearchPointer = strstr(SearchString, StartIndicationString);

	if (!SearchPointer)
		return 0;

	// Now we move to the beginning
	SearchPointer += strlen(StartIndicationString);

	// Now we move to the end with the end pointer
	EndOfStringPointer = strstr(SearchPointer, EndIndicationString);
	if (!EndOfStringPointer)
		return 0;

	// Now we allocate memory and copy the string (even empty string)...
	// Note: in order to have clang-analyzer correctly analyze this code,
	// the MyMalloc argument has to be the same than the length used
	// in strncpy. Using MyMalloc(xxx + 1), as it was done before,
	// generates a false clang-analyzer report.
	StringLength = EndOfStringPointer - SearchPointer + 1;
	ReturnString = MyMalloc(StringLength);
	strncpy(ReturnString, SearchPointer, StringLength);
	ReturnString[StringLength - 1] = 0;

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
		error_message(__FUNCTION__, "\
The string that is supposed to prefix an entry in a text data file\n\
of FreedroidRPG was not found within this text data file.\n\
This indicates some corruption in the data file in question.", PLEASE_INFORM | IS_FATAL);
	}
	return result;
};				// char* ReadAndMallocStringFromData ( ... )

/**
 * This function counts the number of occurrences of a string in a given
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
char *read_and_malloc_and_terminate_file(const char *filename, const char *file_end_string)
{
	// Read the whole theme data to memory.  We use binary mode, as we
	// don't want to have to deal with any carriage return/line feed 
	// convention mess on win32 or something...

	FILE *data_file = fopen(filename, "rb");
	if (!data_file) {
		fprintf(stderr, "\n\nfilename: '%s'\n", filename);

		error_message(__FUNCTION__,
		              "FreedroidRPG was unable to open a given text file,\n"
		              "that should be there and should be accessible.\n"
		              "This indicates a serious bug in this installation of FreedroidRPG.",
		              PLEASE_INFORM | IS_FATAL);
	}

	int filelen = FS_filelength(data_file);
	long memory_amount = filelen + 100;
	char *data = (char *)MyMalloc(memory_amount);

	if (fread(data, 1, memory_amount, data_file) < filelen && ferror(data_file)) {
		error_message(__FUNCTION__,
		              "FreedroidRPG was unable to read a given text file, that should be there and\n"
		              "should be accessible.\n"
		              "Filename: %s",
		              PLEASE_INFORM | IS_FATAL, filename);
	}

	if (fclose(data_file) == EOF) {
		fprintf(stderr, "\n\nfilename: '%s'\n", filename);
		error_message(__FUNCTION__,
		              "FreedroidRPG was unable to close a given text file, that should be there and\n"
		              "should be accessible.\n"
		              "This indicates a strange bug in this installation of Freedroid, that is\n"
		              "very likely a problem with the file/directory permissions of the files\n"
		              "belonging to FreedroidRPG.",
		              PLEASE_INFORM | IS_FATAL);
	}

	// NOTE: Since we do not assume to always have pure text files here, we switched to
	// MyMemmem, so that we can handle 0 entries in the middle of the file content as well
	if (file_end_string) {
		char *read_pointer = my_memmem(data, (size_t) memory_amount, (char *)file_end_string, (size_t) strlen(file_end_string));
		if (!read_pointer) {
			error_message(__FUNCTION__, "FreedroidRPG was unable to find the string, that should indicate the end of\n"
			                            "the given text file within this file.\n"
			                            "This indicates a corrupt or outdated data or saved game file."
			                            "  filename: '%s' - File_End_String: '%s'",
			              PLEASE_INFORM | IS_FATAL, filename, file_end_string);
		} else {
			read_pointer[0] = 0;
		}
	} else
		data[memory_amount - 100] = 0;

	return (data);
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
		error_message(__FUNCTION__, "\
The string that was supposed to be in the text data file could not be found.\n\
This indicates a corrupted or seriously outdated game data or saved game file.", PLEASE_INFORM | IS_FATAL);
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
		error_message(__FUNCTION__, "sscanf using a certain format string failed!\n"
		                            "This indicates a corrupted or seriously outdated game data or saved game file.\n"
		                            "  FormatString: '%s' - ValuePreceedText: '%s'",
		              PLEASE_INFORM | IS_FATAL, FormatString, ValuePreceedText);
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
		error_message(__FUNCTION__, "sscanf using a certain format string failed!\n"
		                            "This indicates a corrupted or seriously outdated game data or saved game file.\n"
		                            "  FormatString: '%s' - ValuePreceedText: '%s'",
		              PLEASE_INFORM | IS_FATAL, FormatString, ValuePreceedText);
	}

	if (EndOfSearchSectionPointer) {
		// Now that we are done, we restore the given SearchArea to former glory
		EndOfSearchSectionPointer[0] = OldTerminaterCharValue;
	}

};				// void ReadValueFromString( ... )

/** 
 * \deprecated Use get_range_from_string.
 * \brief Read a range of integer values from a string with a default.
 *
 * Read the range between two indications. The function don't accept negative value.
 *
 * \param SearchString The global string to search the range.
 * \param StartIndicationString The start indication.
 * \param EndIndicationString The end indication.
 * \param min,max The result of the conversion.
 * \param default_val The value used as default.
 * \return FALSE if the conversion failed and set min and max to default_val, else return TRUE.
 */
int ReadRangeFromString(char *SearchString, const char *StartIndicationString, const char *EndIndicationString, int *min, int *max, int default_val)
{
	int result;

	// First, we have to get the range string.
	char *search_ptr = ReadAndMallocStringFromDataOptional(SearchString, StartIndicationString, EndIndicationString);

	result = get_range_from_string(search_ptr, min, max, default_val);
	free(search_ptr);

	// Handle corrupted values.
	if (*min < 0) {
		error_message(__FUNCTION__, "\
The value read in as a minimum (%d) is a negative number.\n\
This most likely means corrupted data.\n\
Setting both the maximum and minimum to the default value (%d).", NO_REPORT, *min, default_val);
		*min = *max = default_val;
		return FALSE;
	}

	return result;
}

/**
 * \brief Read a range of integer values from a string with a default.
 * \param str The constant string to convert.
 * \param min,max The result of the conversion.
 * \param default_value The value used in case of error or NULL.
 * \return FALSE if the conversion failed and set min and max to default_val, else return TRUE.
 */
int get_range_from_string(const char *str, int *min, int *max, int default_value)
{
	char *ptr;

	if (!str) {
		// String is NULL, set everything to default value.
		*min = *max = default_value;
		return TRUE;
	}

	*min = strtol(str, &ptr, 10);

	if (str == ptr) {
		goto read_error;
	}

	if (*ptr == '\0') {
		// Conversion succeeded and the string contains only one value.
		*max = *min;
		return TRUE;
	}

	if (*ptr != ':') {
		goto read_error;
	}

	// The string contains potentially another value.
	str = ++ptr;
	*max = strtol(str, &ptr, 10);

	if (str == ptr) {
		goto read_error;
	}
	// Conversion of max value succeeded

	if (*max >= *min) {
		return TRUE; // Values are in the good order.
	}

	error_message(__FUNCTION__, "The value read in as a maximum (%d) is less than the minimum (%d).\n"
	                           "This most likely means corrupted data.\n"
	                           "Setting both the maximum and minimum to the default value (%d).",
	                           NO_REPORT, *max, *min, default_value);
	*min = *max = default_value;
	return FALSE;

read_error:
	// Failed to read the string.
	error_message(__FUNCTION__, "The string (\"%s\") cannot be legaly converted to a range of integers.\n"
	                           "This most likely means corrupted data.\n"
	                           "Setting both the maximum and minimum to the default value (%d).",
	                           NO_REPORT, str, default_value);
	*min = *max = default_value;
	return FALSE;
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
		error_message(__FUNCTION__, "Error reading compressed data stream.", PLEASE_INFORM);
		free(temp_dbuffer);
		free(src);
		return -1;
	}

	int ret;
	z_stream strm;

	/* copy paste of public domain tool "zpipe" */
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
		error_message(__FUNCTION__, "\
		zlib was unable to start decompressing a stream.\n\
		This indicates a serious bug in this installation of FreedroidRPG.", PLEASE_INFORM);
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

			error_message(__FUNCTION__, "\
			zlib was unable to decompress a stream\n\
			This indicates a serious bug in this installation of FreedroidRPG.", PLEASE_INFORM);
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
#define CHUNK 16384
	unsigned char out[CHUNK];

	/* allocate deflate state */
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = size;
	strm.next_in = source_buffer;

	int ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY);
	if (ret != Z_OK) {
		error_message(__FUNCTION__, "\
		zlib was unable to start compressing a string.", PLEASE_INFORM);
		return -1;
	}

	/* run deflate() on input until output buffer not full, finish
	   compression if all of source has been read in */
	do {
		strm.avail_out = CHUNK;
		strm.next_out = out;
		ret = deflate(&strm, Z_FINISH);
		if (ret == Z_STREAM_ERROR) {
			error_message(__FUNCTION__, "Stream error while deflating a buffer", IS_FATAL | PLEASE_INFORM);
		}
		unsigned int have = CHUNK - strm.avail_out;
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
