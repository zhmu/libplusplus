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
 * \file database_sqlite.cc
 * \brief SQLite database functionality
 *
 */
#ifdef DB_SQLITE

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <database.h>

/*
 * DBRESULT_SQLITE::DBRESULT_SQLITE(int ncols, int nrows, char** res)
 *
 * This will initialize a new database result with result [res], [ncols] columns
 * and [nrows] rows.
 *
 */
DBRESULT_SQLITE::DBRESULT_SQLITE(int ncols, int nrows, char** res) {
	// store the results
	numfields = ncols; numrows = nrows; result = res; rowno = 1;
}

/*
 * DBRESULT_SQLITE::next()
 *
 * This will attempt to fetch the next row. It will return zero on failure or
 * non-zero on success.
 *
 */
int
DBRESULT_SQLITE::next() {
	return (rowno++ >= numrows) ? 0 : 1;
}

/*
 * DBRESULT_SQLITE::fetchColumnAsString (int colno)
 *
 * This will attempt to fetch the value in column [colno] as a string.
 *
 */
char*
DBRESULT_SQLITE::fetchColumnAsString (int colno) {
	// got a field?
	if (colno > numfields) {
		// no. refuse to fetch
		#ifdef _DEBUG_DB
		fprintf (stderr, "DBRESULT_SQLITE::fetchColumnAsString(): no column %u in %u!\n", colno, numfields);
		#endif /* _DEBUG_DB */
		return "";
	}
	return result[(rowno * numfields) + colno];
}

/*
 * DBRESULT_SQLITE::numRows()
 *
 * This will return the number of rows we got.
 *
 */
int
DBRESULT_SQLITE::numRows() {
	return numrows;
}

/*
 * DBRESULT_SQLITE::~DBRESULT_SQLITE()
 *
 * This will deinitialize the database result.
 *
 */
DBRESULT_SQLITE::~DBRESULT_SQLITE() {
	sqlite3_free_table (result);
}

/*
 * DATABASE_SQLITE::DATABASE_SQLITE()
 *
 * This will initialize the SQLite database driver.
 *
 */
DATABASE_SQLITE::DATABASE_SQLITE() {
	db = NULL; errmsg = NULL;
}

/*
 * DATABASE_SQLITE::~DATABASE_SQLITE()
 *
 * This will deinitialize the SQLite database driver.
 *
 */
DATABASE_SQLITE::~DATABASE_SQLITE() {
}

/*
 * DATABASE_SQLITE::connect (char* hostname, char* username, char* password,
 *                          char* dbname)
 *
 * This will try to make a SQLite connection with file [dbname], all other
 * fields are ignored. It will return zero on failure or non-zero on success.
 */
int
DATABASE_SQLITE::connect (char* hostname, char* username, char* password, char* dbname) {
	return connect (dbname);
}

/*
 * DATABASE_SQLITE::connect (char* fname)
 *
 * This will try to make a SQLite connection with file [fname]. It will return
 * zero on failure or non-zero on success.
 */
int
DATABASE_SQLITE::connect (char* fname) {
	return (sqlite3_open (fname, &db) == SQLITE_OK) ? 1 : 0;
}

/*
 * DATABASE_SQLITE::getErrorMsg()
 *
 * This will retrieve the last error message.
 *
 */
const char*
DATABASE_SQLITE::getErrorMsg() {
	if (errmsg)
		return errmsg;
	return sqlite3_errmsg (db);
}

/*
 * DATABASE_SQLITE::vquery (const char* query, va_list va)
 *
 * This will execute query [query] with list [va]. It will return a
 * DBRESULT_SQLITE* object on success or NULL on failure.
 *
 */
DBRESULT*
DATABASE_SQLITE::vquery (const char* query, va_list va) {
	char realquery[DATABASE_MAX_QUERY_LEN + 1];
	int i, nrows, ncols;
	char** res;

	// build the query
	if (!build_query (realquery, DATABASE_MAX_QUERY_LEN, query, va))
		// this failed. bail out
		return NULL;

	// feed it into the database
#ifdef _DEBUG_DB
	fprintf (stderr, "DATABASE_SQLITE::vquery(): doing query '%s'\n", realquery);
#endif // _DEBUG_DB
	errmsg = NULL;
	i = sqlite3_get_table (db, realquery, &res, &nrows, &ncols, &errmsg);
	if (i != SQLITE_OK) {
		// this failed. complain
		return NULL;
	}

	// fetch the results
	return new DBRESULT_SQLITE (ncols, nrows, res);
}

/*
 * DATABASE_SQLITE::query (const char* query, ...)
 *
 * This will execute query [query] with any parameters needed. It will return a
 * DBRESULT_SQLITE* object on success or NULL on failure.
 *
 */
DBRESULT*
DATABASE_SQLITE::query (const char* query, ...) {
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
 * DATABASE_SQLITE::execute (const char* query, ...)
 *
 * This will execute query [query] with any parameters needed. It will return 
 * zero on failure and non-zero on success.
 *
 */
int
DATABASE_SQLITE::execute (const char* query, ...) {
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
 * This will escape [source] to [dest].
 */
void
DATABASE_SQLITE::escape (const char* source, char* dest, int destlen) {
	const char* ptr;
	int destindex = 0;

	/*
	 * This is crude: we simply copy everything over, and do ' twice.
	 */
	for (ptr = source; *ptr != 0; ptr++) {
		if (*ptr == '\'') {
			dest[destindex++] = *ptr;
			if (destindex >= destlen) break;
			dest[destindex++] = *ptr;
		} else
			dest[destindex++] = *ptr;
		if (destindex >= destlen) break;
	}
	dest[destindex] = 0;
}

/*
 * This will execute query [query] along with any arguments needed.
 */
DBRESULT*
DATABASE_SQLITE::limitQuery (const char* query, unsigned int count, unsigned int offset, ...) {
	char realquery[DATABASE_MAX_QUERY_LEN + 1];
	char realtmp[DATABASE_MAX_QUERY_LEN + 1];
	va_list va;
	int i, nrows, ncols;
	char** res;

	// build the query
	va_start (va, offset);
	if (!build_query (realquery, DATABASE_MAX_QUERY_LEN, query, va)) {
		// this failed. bail out
		va_end (va);
		return NULL;
	}
	va_end (va);

	// append the LIMIT clause
	if (offset != 0)
		sprintf (realtmp, " LIMIT %u,%u", offset, count);
	else
		sprintf (realtmp, " LIMIT %u", count);
	strcat (realquery, realtmp);

	// feed it to the database
	errmsg = NULL;
	i = sqlite3_get_table (db, realquery, &res, &nrows, &ncols, &errmsg);
	if (i != SQLITE_OK) {
		// this failed. complain
		return NULL;
	}

	// fetch the results
	return new DBRESULT_SQLITE (ncols, nrows, res);
}

#endif // DB_SQLITE

/* vim:set ts=2 sw=2: */
