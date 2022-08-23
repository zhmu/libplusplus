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
 * \file database_mysql.cc
 * \brief MySQL database functionality
 *
 */
#ifdef DB_MYSQL

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <database.h>

/*
 * DBRESULT_MYSQL::DBRESULT_MYSQL(MYSQL_RES* res)
 *
 * This will initialize a new database result with result [res].
 *
 */
DBRESULT_MYSQL::DBRESULT_MYSQL(MYSQL_RES* res) {
	// store the result and with rows or fields by default
	numfields = -1; numrows = -1; result = res; row = NULL;

	// got a valid result handle?
	if (result != NULL) {
		// yes. query the number of rows and fields
		numfields = mysql_num_fields (result);
		numrows   = mysql_num_rows   (result);
	}

	// fetch the first row
	next(); 
}

/*
 * DBRESULT_MYSQL::next()
 *
 * This will attempt to fetch the next row. It will return zero on failure or
 * non-zero on success.
 *
 */
int
DBRESULT_MYSQL::next() {
	// if we have no result, we auto-fail
	if (result == NULL)
		return 0;

	// fetch the row
	row = mysql_fetch_row (result);

	// row determines our victory
	return (row == NULL) ? 0 : 1;
}

/*
 * DBRESULT_MYSQL::fetchColumnAsString (int colno)
 *
 * This will attempt to fetch the value in column [colno] as a string.
 *
 */
char*
DBRESULT_MYSQL::fetchColumnAsString (int colno) {
	// ensure we have a row
	if (row == NULL) {
		// no. refuse to fetch
		#ifdef _DEBUG_DB
		fprintf (stderr, "DBRESULT_MYSQL::fetchColumnAsString(): no row!!\n");
		#endif /* _DEBUG_DB */
		return 0;
	}

	// got a field?
	if (colno > numfields) {
		// no. refuse to fetch
		#ifdef _DEBUG_DB
		fprintf (stderr, "DBRESULT_MYSQL::fetchColumnAsString(): no column %u in %u!\n", colno, numfields);
		#endif /* _DEBUG_DB */
		return "";
	}
	return row[colno];
}

/*
 * DBRESULT_MYSQL::numRows()
 *
 * This will return the number of rows we got.
 *
 */
int
DBRESULT_MYSQL::numRows() {
	return numrows;
}

/*
 * DBRESULT_MYSQL::~DBRESULT_MYSQL()
 *
 * This will deinitialize the database result.
 *
 */
DBRESULT_MYSQL::~DBRESULT_MYSQL() {
	// if we had a result, kill it
	if (result)
		mysql_free_result (result);
}

/*
 * DATABASE_MYSQL::DATABASE_MYSQL()
 *
 * This will initialize the MySQL database driver.
 *
 */
DATABASE_MYSQL::DATABASE_MYSQL() {
	mysql_init (&mysql);
}

/*
 * DATABASE_MYSQL::~DATABASE_MYSQL()
 *
 * This will deinitialize the MySQL database driver.
 *
 */
DATABASE_MYSQL::~DATABASE_MYSQL() {
}

/*
 * DATABASE_MYSQL::connect (char* hostname, char* username, char* password,
 *                          char* dbname)
 *
 * This will try to make a MySQL connection to [hostname], as user [username]
 * with password [password]. It will use database [dbname]. It will return
 * zero on failure or non-zero on success.
 *
 */
int
DATABASE_MYSQL::connect (char* hostname, char* username, char* password, char* dbname) {
	// just pass through whatever MySQL tells us
	return (mysql_real_connect (&mysql, hostname, username, password, dbname, 0, NULL, 0) == NULL) ? 0 : 1;
}

/*
 * DATABASE_MYSQL::getErrorMsg()
 *
 * This will retrieve the last error message.
 *
 */
const char*
DATABASE_MYSQL::getErrorMsg() {
	return mysql_error (&mysql);
}

/*
 * DATABASE_MYSQL::vquery (const char* query, va_list va)
 *
 * This will execute query [query] with list [va]. It will return a
 * DBRESULT_MYSQL* object on success or NULL on failure.
 *
 */
DBRESULT*
DATABASE_MYSQL::vquery (const char* query, va_list va) {
	char realquery[DATABASE_MAX_QUERY_LEN + 1];
	int i;

	// build the query
	if (!build_query (realquery, DATABASE_MAX_QUERY_LEN, query, va))
		// this failed. bail out
		return NULL;

	// feed it into the database
	#ifdef _DEBUG_DB
	fprintf (stderr, "MYSQL::vquery(): doing query '%s'\n", realquery);
	#endif // _DEBUG_DB
	i = mysql_real_query (&mysql, realquery, strlen (realquery));
	if (i) {
		// this failed. complain
		return NULL;
	}

	// fetch the results
	MYSQL_RES* result = mysql_store_result (&mysql);
	return new DBRESULT_MYSQL (result);
}

/*
 * DATABASE_MYSQL::query (const char* query, ...)
 *
 * This will execute query [query] with any parameters needed. It will return a
 * DBRESULT_MYSQL* object on success or NULL on failure.
 *
 */
DBRESULT*
DATABASE_MYSQL::query (const char* query, ...) {
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
 * DATABASE_MYSQL::execute (const char* query, ...)
 *
 * This will execute query [query] with any parameters needed. It will return 
 * zero on failure and non-zero on success.
 *
 */
int
DATABASE_MYSQL::execute (const char* query, ...) {
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
DATABASE_MYSQL::escape (const char* source, char* dest, int destlen) {
	// escape it
	mysql_real_escape_string (&mysql, dest, source, strlen (source));
}

/*
 * This will execute query [query] along with any arguments needed.
 */
DBRESULT*
DATABASE_MYSQL::limitQuery (const char* query, unsigned int count, unsigned int offset, ...) {
	char realquery[DATABASE_MAX_QUERY_LEN + 1];
	char realtmp[DATABASE_MAX_QUERY_LEN + 1];
	va_list va;
	int i;

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

	// feed it into the database
	#ifdef _DEBUG_DB
	fprintf (stderr, "MYSQL::vquery(): doing query '%s'\n", realquery);
	#endif // _DEBUG_DB
	i = mysql_real_query (&mysql, realquery, strlen (realquery));
	if (i) {
		// this failed. complain
		return NULL;
	}

	// fetch the results
	MYSQL_RES* result = mysql_store_result (&mysql);
	return new DBRESULT_MYSQL (result);
}

#endif // DB_MYSQL

/* vim:set ts=2 sw=2: */
