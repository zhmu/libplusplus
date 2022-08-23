/*
 * libplusplus - A generic C++ library for networking, databases and more
 * Copyright (C) 2002-2005 Rink Springer
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
 * \file database.cc
 * \brief Database functionality
 *
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef DB_MYSQL
#include <mysql/mysql.h>
#endif /* DB_MYSQL */
#include <database.h>

/*
 * This will deinitialize the database driver.
 */
DATABASE::~DATABASE() { }

/*
 * This will return a new database object for type [type],
 * or NULL if the server isn't supported.
 *
 */
DATABASE*
DATABASE::getDatabase(char* type) {
#ifdef DB_MYSQL
	if (!strcasecmp (type, "mysql"))
		return new DATABASE_MYSQL();
#endif /* DB_MYSQL */

#ifdef DB_PGSQL
	if (!strcasecmp (type, "pgsql"))
		return new DATABASE_PGSQL();
#endif /* DB_PGSQL */

#ifdef DB_SQLITE
	if (!strcasecmp (type, "sqlite"))
		return new DATABASE_SQLITE();
#endif /* DB_SQLITE */

	// no such database
	return NULL;
}

/*
 * This will deinitialize the database driver.
 *
 */
DBRESULT::~DBRESULT() { }

/*
 * This will execute query [query] along with any arguments needed.
 */
int
DATABASE::execute (const char* query, ...) {
	va_list va;
	DBRESULT* res;

	// just feed the query in vquery()
	va_start (va, query);
	res = vquery (query, va);
	va_end (va);

	// if we have a result, ditch it
	if (res)
		delete res;

	// the result indicates the return status
	return (res == NULL) ? 0 : 1;
}

/*
 * This will execute query [query] along with any arguments needed.
 */
DBRESULT*
DATABASE::query (const char* query, ...) {
	va_list va;
	DBRESULT* res;

	// just feed the query in vquery()
	va_start (va, query);
	res = vquery (query, va);
	va_end (va);

	// return the result
	return res;
}

/*
 * This will build query [query] with list [va]. The output will be put in
 * [output], up to a maximum of [outlen] bytes.
 */
int
DATABASE::build_query (char* output, int outlen, const char* query, va_list va) {
	char realtmp[DATABASE_MAX_QUERY_LEN + 1];
	char* ptr;
	char* tmp_ptr;
	int i;
	int query_len = 0;

	// clean it out
	memset (output, 0, outlen);

	// walk through the query
	ptr = (char*)query;
	while (*ptr) {
		// got a question mark?
		if (*ptr == '?') {
			// yes. fetch the argument
			tmp_ptr = va_arg (va, char*);

			// escape it
			escape (tmp_ptr, realtmp, DATABASE_MAX_QUERY_LEN);

			// is this add-able?
			if ((query_len + strlen (realtmp)) >= outlen)
				// no. bail out
				return 0;

			// append it
			output[query_len++] = '\'';
			strcat ((output + query_len), realtmp);
			query_len += strlen (realtmp);
			output[query_len++] = '\'';
		} else if (*ptr == '#') {
			// no, but we got a hash char. fetch the argument
			i = va_arg (va, int);

			// convert the integer to a string
			snprintf (realtmp, sizeof (realtmp), "%u", i);

			// is this add-able?
			if ((query_len + strlen (realtmp) + 3) >= outlen)
				// no. bail out
				return 0;

			// append it
			output[query_len++] = '\'';
			strcat ((output + query_len), realtmp);
			query_len += strlen (realtmp);
			output[query_len++] = '\'';
		} else if (*ptr == '@') {
			// no, but we got an at char. fetch the argument
			i = va_arg (va, int);

			// convert the integer to a string
			snprintf (realtmp, sizeof (realtmp), "%u", i);

			// is this add-able?
			if ((query_len + strlen (realtmp)) >= outlen)
				// no. bail out
				return 0;

			// append it
			strcat ((output + query_len), realtmp);
			query_len += strlen (realtmp);
		} else {
			// no. is this add-able?
			if (query_len >= outlen)
				// no. refuse to query
				return 0;

			// append the char
			output[query_len++] = *ptr;
		}

		// next
		ptr++;
	}

	// terminate the output line :)
	output[query_len] = 0;

	// debugging
	#ifdef _DEBUG_DB
	fprintf (stderr, "DATABASE::build_query(): made query '%s'\n", output);
	#endif // _DEBUG_DB

	// all done
	return 1;
}

/*
 * This will attempt to fetch the value in column [colno] as an integer.
 */
unsigned int
DBRESULT::fetchColumnAsInteger (int colno) {
	char* s = fetchColumnAsString (colno);
	return (s == NULL) ? 0 : atoi (s);
}

/* vim:set ts=2 sw=2: */
