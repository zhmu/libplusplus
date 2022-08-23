/*
 * libplusplus - A generic C++ library for networking, databases and more
 * Copyright (C) 2002-2004 Rink Springer
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
 * \file database_pgsql.cc
 * \brief PostgreSQL database functionality
 *
 */
#ifdef DB_PGSQL

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>
#include <database.h>

/*
 * This will initialize our object.
 */
DATABASE_PGSQL::DATABASE_PGSQL() {
	// no connection just yet
	conn = NULL;
}

/*
 * This will clean our object up.
 */
DATABASE_PGSQL::~DATABASE_PGSQL() {
	// got a connection?
	if (conn != NULL) {
		// yes. clean it up
		PQfinish (conn);
		conn = NULL;
	}
}

/*
 * This will try to make a PostgreSQL connection to [hostname], as user
 * [username] with password [password]. It will use database [dbname]. It will
 * return zero on failure or non-zero on success.
 */
int
DATABASE_PGSQL::connect (char* hostname, char* username, char* password, char* dbname) {
	char connectstring[DATABASE_MAX_QUERY_LEN];

	/* construct the connection string */
	snprintf (connectstring, DATABASE_MAX_QUERY_LEN - 1, "host='%s' user='%s' password='%s' dbname='%s'", hostname, username, password, dbname);

	/* try to connect */
	conn = PQconnectdb (connectstring);

	/* success depends on the connection status */
	return (PQstatus (conn) != CONNECTION_OK) ? 0 : 1;
}

/*
 * This will return the last error message in a human-readable form.
 */
const char*
DATABASE_PGSQL::getErrorMsg() {
	if (conn == NULL)
		return NULL;
	return PQerrorMessage (conn);
}

/*
 * This will execute query [query] with list [va]. It will return a
 * DBRESULT_PGSQL* object on success or NULL on failure.
 */
DBRESULT*
DATABASE_PGSQL::vquery (const char* query, va_list va) {
	char realquery[DATABASE_MAX_QUERY_LEN + 1];
	PGresult* res;

	// build the query
	if (!build_query (realquery, DATABASE_MAX_QUERY_LEN, query, va))
		// this failed. bail out
		return NULL;

	// feed it into the database
	#ifdef _DEBUG_DB
	fprintf (stderr, "PGSQL::vquery(): doing query '%s'\n", realquery);
	#endif // _DEBUG_DB
	res = PQexec (conn, realquery);
	if ((PQresultStatus (res) != PGRES_COMMAND_OK) && (PQresultStatus (res) != PGRES_TUPLES_OK)) {
		// this failed. complain
		#ifdef _DEBUG_DB
		fprintf (stderr, "PGSQL::vquery(): query not okay: %s\n", PQerrorMessage (conn));
		#endif // _DEBUG_DB
		PQclear (res);
		return NULL;
	}

	// fetch the results
	return new DBRESULT_PGSQL (res);
}

/*
 * This will initialize a PostgreSQL result.
 */
DBRESULT_PGSQL::DBRESULT_PGSQL(PGresult* res) {
	// store the result, first row
	result = res; rowno = 0;

	// fetch the information
	numrows = PQntuples (result);
	numfields = PQnfields (result);
}

/*
 * This will deinitialize a PostgreSQL result.
 */
DBRESULT_PGSQL::~DBRESULT_PGSQL() {
	// free the result
	PQclear (result);
}

/*
 * This will attempt to fetch the value in column [colno] as a string.
 */
char*
DBRESULT_PGSQL::fetchColumnAsString (int colno) {
	return PQgetvalue (result, rowno, colno);
}

/*
 * This will attempt to fetch the next row. It will return zero on failure or
 * non-zero on success.
 *
 */
int
DBRESULT_PGSQL::next() {
	/* next */
	rowno++;

	/* if we have run out of rows, we failed */
	return (rowno == numrows) ? 0 : 1;
}

/*
 * This will return the number of rows we got.
 */
int
DBRESULT_PGSQL::numRows() {
	return numrows;
}

/*
 * This will escape [source] to [dest].
 */
void
DATABASE_PGSQL::escape (const char* source, char* dest, int destlen) {
	// escape it
	PQescapeString (dest, source, strlen (source));
}

/*
 * This will execute query [query] along with any arguments needed.
 */
DBRESULT*
DATABASE_PGSQL::limitQuery (const char* query, unsigned int count, unsigned int offset, ...) {
	char realquery[DATABASE_MAX_QUERY_LEN + 1];
	char realtmp[DATABASE_MAX_QUERY_LEN + 1];
	PGresult* res;
	va_list va;

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
		sprintf (realtmp, " LIMIT %u OFFSET %u", count, offset);
	else
		sprintf (realtmp, " LIMIT %u", count);
	strcat (realquery, realtmp);

	// feed it into the database
	#ifdef _DEBUG_DB
	fprintf (stderr, "PGSQL::limitQuery(): doing query '%s'\n", realquery);
	#endif // _DEBUG_DB
	res = PQexec (conn, realquery);
	if ((PQresultStatus (res) != PGRES_COMMAND_OK) && (PQresultStatus (res) != PGRES_TUPLES_OK)) {
		// this failed. complain
		#ifdef _DEBUG_DB
		fprintf (stderr, "PGSQL::vquery(): query not okay: %s\n", PQerrorMessage (conn));
		#endif // _DEBUG_DB
		PQclear (res);
		return NULL;
	}

	// fetch the results
	return new DBRESULT_PGSQL (res);
}

#endif // DB_PGSQL

/* vim:set ts=2 sw=2: */
