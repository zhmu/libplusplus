/*
 * libplusplus - A generic C++ library for networking, databases and more
 * Copyright (C) 2002, 2003 Rink Springer
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * \file configfile.cc
 * \brief Configuration file functionality
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <configfile.h>

/*
 * CONFIGFILE::CONFIGFILE()
 *
 * This will create an empty, unitialized configuration file.
 */
CONFIGFILE::CONFIGFILE() {
	// no config file now
	fname = NULL; buf = NULL; buf_length = 0;
}

/*
 * CONFIGFILE::~CONFIGFILE()
 *
 * This will deinitialize the configuration file, free-ing any memory as needed.
 */
CONFIGFILE::~CONFIGFILE() {
	// is there memory allocated for the configuration file ?
	if (buf) {
		// yes. free it
		free (buf); buf_length = 0;
	}
}

/*!
 * CONFIGFILE::load (char* name)
 *
 * This will load the configuration file [name]. It will return CONFIGFILE_OK
 * on success or CONFIGFILE_ERROR_xxx on failure.
 */
int
CONFIGFILE::load (char* name) {
	// store the file name
	fname = name;

	// pass it over to reload()
	return reload();
}

/*
 * CONFIGFILE::reload()
 *
 * This will reload the configuration file. It will return CONFIGFILE_OK on
 * success or CONFIGFILE_ERROR_xxx on failure.
 */
int
CONFIGFILE::reload() {
	FILE* f;
	int i;

	// is a configuration file name set?
	if (fname == NULL)
		// no. complain
		return CONFIGFILE_ERROR_NOFILE;

	// open the configuration file
	if ((f = fopen (fname, "rt")) == NULL)
		// this failed. complain
		return CONFIGFILE_ERROR_OPENERR;

	// fetch the file length
	fseek (f, 0, SEEK_END); buf_length = ftell (f); rewind (f);

	// allocate memory
	if ((buf = (char*)malloc (buf_length + 1)) == NULL) {
		// this failed. complain
		fclose (f);
		return CONFIGFILE_ERROR_OUTOFMEM;
	}

	// read the contents
	if (!fread (buf, buf_length, 1, f)) {
		// this failed. complain
		fclose (f);
		return CONFIGFILE_ERROR_READERR;
	}

	// nul-terminate it
	buf[buf_length] = 0;

	// close the file
	fclose (f);

	// now, replace all newlines by nuls
	for (i = 0; i < buf_length; i++)
		if (buf[i] == 0xa) buf[i] = 0;

	// give the user a chance to parse this file
	parse();

	// this worked
	return CONFIGFILE_OK;
}

/*
 * CONFIGFILE::scan_section (char** name, int* offset)
 *
 * This will scan the file for the next section, starting at the supplied
 * offset. If found, CONFIGFILE_OK will be returned and the offset will be
 * copied to the variable which held the source offset. On failure,
 * CONFIGFILE_ERROR_xxx will be returned.
 */
int
CONFIGFILE::scan_section (char** name, int* offset) {
	int   ofs = *offset;
	char* temp_line;
	char* ptr;

	// walk through the entire file
	while (1) {
		// have we reached the end of the file?
		if (ofs >= buf_length)
			// yes. get out of here
			break;

		// fetch the line
		temp_line = buf + ofs;

		// is the first charachter a '[' thingy?
 		if (*temp_line == '[') {
			// yes. is there also a ']' at the end?
			ptr = strchr (temp_line, ']');
			if ((ptr != NULL) && (!*(ptr + 1))) {
				// yes. we got it!
				*name = temp_line + 1;
				*offset = ofs + strlen (temp_line) + 1;
				return CONFIGFILE_OK;
			}
		}

		// next line
		ofs += strlen (temp_line) + 1;
	}

	// no such section
	return CONFIGFILE_ERROR_NOSECTION;
}

/*
 * CONFIGFILE::scan_value(char** name, char** value, int* offset)
 *
 * This will scan the file for the next value. If found, it return the name
 * and the value itself, as well as the offset in the supplied pointers, and
 * the offset pointer will be updated. On failure, CONFIGFILE_ERROR_xxx will be
 * returned, otherwise CONFIGFILE_OK.
 */
int
CONFIGFILE::scan_value(char** name, char** value, int* offset) {
	int ofs = *offset;
	char* ptr;
	char* ptr2;

	while (1) {
		// end of the buffer?
		if (ofs >= buf_length)
			// yes. bail out
			break;

		// get the line
		ptr = buf + ofs; ofs += strlen (ptr) + 1;

		// skip any spaces
		while ((*ptr == ' ') || (*ptr == '\t')) ptr++;

		// does the line begin with a '[' (section)?
		if (*ptr == '[')
			// yes. bail out
			break;

		// does the line begin with a '#' (comment)?
		if (*ptr != '#') {
			// no. scan for a '='
			ptr2 = strchr (ptr, '=');

			if (ptr2 != NULL) {
				// we got it. skip any spaces after the '='
				ptr2++;
				while ((*ptr2 == ' ') || (*ptr2 == '\t')) ptr2++;

				// return the pointers and new offset
				*name = ptr; *value = ptr2; *offset = ofs;
				return CONFIGFILE_OK;
			}
		}
	}

	// no more values
	return CONFIGFILE_ERROR_NOVALUE;
}

/*
 * CONFIGFILE::find_section(char* section, int* offset)
 *
 * This will scan the configuration file for the section supplied. If it is
 * found, the offset will be replaced by the section offset and CONFIGFILE_OK
 * will be returned.  On failure, one of the CONFIGFILE_ERROR_xxx values will
 * be returned.
 */
int
CONFIGFILE::find_section(char* section, int* offset) {
	int   i, j;
	char* ptr;
	char* ptr2;

	// fetch the section length
	j = strlen (section);
	if (strchr (section, ']')) j--;

	// keep looking
	while (1) {
		// get the next section
		i = scan_section (&ptr, offset);

		// got one?
		if (i != CONFIGFILE_OK)
			// no. bail out
			break;

		// match?
		ptr2 = strchr (ptr, ']');
		if ((ptr2 != NULL) && ((ptr2 - ptr) == j))
			if (!strncasecmp (ptr, section, j))
				// yes. we got it
				return CONFIGFILE_OK;
	}

	return i;
}

/*
 * CONFIGFILE::get_string (char* section, char* name, char** dest)
 *
 * This will fetch the value of a named identified in the supplied section. On
 * success, CONFIGFILE_OK will be returned, otherwise CONFIGFILE_ERROR_xxx will
 * be returned.
 */
int
CONFIGFILE::get_string (char* section, char* name, char** dest) {
	int i, offset = 0;
	char* ptr;
	char* x_name;
	char* x_val;

	// scan the entire file for the section
	while (1) {
		// fetch the section offset
		i = find_section (section, &offset);
		if (i != CONFIGFILE_OK)
			// no such section. too bad, so sad
			return (!offset) ? CONFIGFILE_ERROR_NOSECTION :
							   CONFIGFILE_ERROR_NOVALUE;

		// now, keep scanning [offset] until we either find the value, we reach
		// a [ or we reach the end of the file
		while (1) {
			// scan for the next value
			i = scan_value (&x_name, &x_val, &offset);
			if (i != CONFIGFILE_OK)
				// this failed. bail out
				break;

			// figure out the length of the name
			ptr = x_name;
			while ((*ptr != ' ') && (*ptr != '\t') && (*ptr != '=')) ptr++;

			// does the name match?
			i = strlen (name);
			if ((!strncasecmp (x_name, name, i)) &&
				((ptr - x_name) == i)) {
				// yes. we got it. skip any spaces and go
				*dest = x_val;
				return CONFIGFILE_OK;
			}
		}

		// fatal error?
		if (i != CONFIGFILE_ERROR_NOVALUE)
			// yes. bomb out
			return i;
	}

	/* NOTREACHED */
}

/*
 * CONFIGFILE::get_value (char* section, char* name, int* value)
 *
 * This will fetch the numeric value of a named identified in the supplied
 * section. On success, CONFIGFILE_OK will be returned, otherwise
 * CONFIGFILE_ERROR_xxx will be returned.
 *
 * If the value is not numeric, CONFIGFILE_ERROR_INVALIDVALUE will be returned.
 *
 */
int
CONFIGFILE::get_value (char* section, char* name, int* value) {
	char* ptr;
	char* ptr2;
	int   i = get_string (section, name, &ptr);

	// did this work?
	if (i != CONFIGFILE_OK) {
		// no. return that value
		return i;
	}

	// resolve the number
	i = strtol (ptr, &ptr2, 0);

	// did this work?
	if ((!*ptr) || (*ptr2))
		// no. complain
		return CONFIGFILE_ERROR_INVALIDVALUE;

	// it worked. set the number and return success
	*value = i;
	return CONFIGFILE_OK;
}

/*
 * CONFIGFILE::parse()
 *
 * An application should overload this, for it will be called whenever
 * the configuration is (re)loaded. It should reset everything to the defaults
 * and parse the file.
 *
 */
void
CONFIGFILE::parse() {
	/* ... */
}

/* vim:set ts=2 sw=2: */
