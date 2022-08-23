/*
 * \file database.h
 * \brief Database functionality
 *
 */
#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <sys/types.h>
#include <stdarg.h>
#ifdef DB_MYSQL
#include <mysql/mysql.h>
#endif // DB_MYSQL
#ifdef DB_PGSQL
#include <libpq-fe.h>
#endif // DB_PGSQL
#ifdef DB_SQLITE
#include <sqlite3.h>
#endif // DB_SQLITE

#define DATABASE_MAX_QUERY_LEN 65535

/*! \class DBRESULT
 *  \brief Base class for database results
 */
class DBRESULT {
public:
	/*! Destroys the object */
	virtual ~DBRESULT() = 0;

	/*! \brief Fetches a given column as integer
	 *  \return The value of the column as an integer, or 0 if there is no such column
	 *  \param colno The column number to retrieve
 	 */
	unsigned int fetchColumnAsInteger (int colno);

	/*! \brief Fetches a given column as string
	 *  \return The value of the column as string, or NULL if there is no such column
	 *  \param colno The column number to retrieve
 	 */
	virtual char* fetchColumnAsString (int colno) = 0;

	/*!	\brief Returns the number of rows returned by the previous SELECT
	 *
	 *  The return value is undefined if the previous function was not a SELECT.
	 */
	virtual int numRows() = 0;

	/*! \brief Switches to the next row of data
	 *  \return Zero on failure and non-zero on success
	 */
	virtual int next() = 0;
};

/*! \class DATABASE
		\brief Base class for database connectivity
 */
class DATABASE {
public:
	/*! Destroys the object */
	virtual ~DATABASE() = 0;

	/*!* \brief Returns a new database object for a given server
	 *   \param type The desired database type
	 *   \return A new database object on success or NULL if the type is
	 *           unsupported
	 */
	static DATABASE* getDatabase(char* type);

	/*! \brief Makes a connection with a server
	 *  \return Zero on failure and non-zero on success
	 *  \param hostname Hostname or IP address of the server to connect to
	 *  \param username Username to use while connecting
	 *  \param password Password to use while connecting
	 *  \param dbname Databasename to connect to
	 *
	 * Errors can be received using getErrorMsg().
	 */
	virtual int connect (char* hostname, char* username, char* password, char* dbname) = 0;

	//! \brief Returns the last error message
	virtual const char* getErrorMsg() = 0;

	/*! \brief Execute a query with result
	 *  \return A DBRESULT object on success, or NULL on failure
	 *  \param query The query to execute
	 *  \param ... Possible parameters to the query
	 *
	 *  Errors can be received using getErrorMsg().
	 *
	 *  query can include special control charachters to include the parameters:
	 *  # is a number, and will be escaped
	 *  @ is a number, and will not be escaped
	 *  ? is a string, and will be escaped
	 */
	DBRESULT* query (const char* query, ...);

	/*! \brief Execute a query with results limited
	 *  \return A DBRESULT object on success or NULL on failure
	 *  \param query The query to execute
	 *  \param count Number of queries to return
	 *  \param offset Offset from which to fetch the replies, 0 if not needed
	 *  \param ... Possible parameters to the query
	 */
	virtual DBRESULT* limitQuery (const char* query, unsigned int count, unsigned int offset, ...) = 0;

	/*! \brief Execute a query without result
	 *  \return A DBRESULT object on success, or NULL on failure
	 *  \param query The query to execute
	 *  \param ... Possible parameters to the query
	 *
	 *  Errors can be received using getErrorMsg().
	 *
	 *  query can include special control charachters, refer to query() for
	 *  an overview.
	 */
	int execute (const char* query, ...);

protected:
	/*! \brief Escape a string
	 *  \param source The string to escape
	 *  \param dest Buffer in which to store the escaped string
	 *  \param destlen Length of the destination buffer
	 */
	virtual void escape (const char* source, char* dest, int destlen) = 0;

	/*! \brief This will build a query
	 *  \param output Buffer in which to store the output
	 *  \param outlen Length of the output buffer
	 *  \param query Query to build.
	 *  \param va Argument list
	 *  \return Non-zero on success or zero on failure
	 *
	 *  This function will substitute values for ?, # and @ as described in
	 *  query().
	 */
	int build_query (char* output, int outlen, const char* query, va_list va);

	/*! \brief This will build a query and execute it
	 *  \param query Query to execute, including syntax modifiers
	 *  \param va Argument list
	 *  \return NULL on failure or a DBRESULT object on success
	 */
	virtual DBRESULT* vquery (const char* query, va_list va) = 0;
};

#ifdef DB_MYSQL
/*! \class DBRESULT_MYSQL
 *  \brief Class for MySQL database results
 */
class DBRESULT_MYSQL : public DBRESULT {
public:
	DBRESULT_MYSQL(MYSQL_RES* res);
	~DBRESULT_MYSQL();
	char* fetchColumnAsString (int colno);
	int next();
	int numRows();

private:
	MYSQL_ROW row;
	MYSQL_RES* result;

	int numfields;
	int numrows;
};

/*! \class DATABASE_MYSQL
		\brief MySQL database connectivity
 */
class DATABASE_MYSQL : public DATABASE {
public:
	DATABASE_MYSQL();
	~DATABASE_MYSQL();

	int connect (char* hostname, char* username, char* password, char* dbname);

	const char* getErrorMsg();
	DBRESULT* query (const char* query, ...);
	int execute (const char* query, ...);

	void escape (const char* source, char* dest, int destlen);

	DBRESULT* limitQuery (const char* query, unsigned int count, unsigned int offset, ...);

private:
	DBRESULT* vquery (const char* query, va_list va);
	MYSQL mysql;
};
#endif // DB_MYSQL

#ifdef DB_PGSQL
/*! \class DBRESULT_PGSQL
 *  \brief Class for PostgreSQL database results
 */
class DBRESULT_PGSQL : public DBRESULT {
public:
	DBRESULT_PGSQL(PGresult* res);
	~DBRESULT_PGSQL();
	char* fetchColumnAsString (int colno);
	int next();
	int numRows();

private:
	PGresult* result;

	int numfields;
	int numrows, rowno;
};

/*! \class DATABASE_PGSQL
		\brief PostgreSQL database connectivity
 */
class DATABASE_PGSQL : public DATABASE {
public:
	DATABASE_PGSQL();
	~DATABASE_PGSQL();

	int connect (char* hostname, char* username, char* password, char* dbname);

	const char* getErrorMsg();
	DBRESULT* query (const char* query, ...);
	int execute (const char* query, ...);

	void escape (const char* source, char* dest, int destlen);

	DBRESULT* limitQuery (const char* query, unsigned int count, unsigned int offset, ...);

private:
	DBRESULT* vquery (const char* query, va_list va);
	PGconn* conn;
};
#endif // DB_PGSQL

#ifdef DB_SQLITE
/*! \class DBRESULT_SQLITE
 *  \brief Class for SQLite database results
 */
class DBRESULT_SQLITE : public DBRESULT {
public:
	DBRESULT_SQLITE(int ncols, int nrows, char** res);
	~DBRESULT_SQLITE();
	char* fetchColumnAsString (int colno);
	int next();
	int numRows();

private:
	char** result;

	int numfields;
	int numrows, rowno;
};

/*! \class DATABASE_SQLITE
		\brief SQLite database connectivity
 */
class DATABASE_SQLITE : public DATABASE {
public:
	DATABASE_SQLITE();
	~DATABASE_SQLITE();

	int connect (char* hostname, char* username, char* password, char* dbname);

	/*! \brief Makes a database connection to a file
	 *  \return Zero on failure and non-zero on success
	 *  \param filename File to open
	 *
	 * Errors can be received using getErrorMsg().
	 */
	int connect (char* filename);

	const char* getErrorMsg();
	DBRESULT* query (const char* query, ...);
	int execute (const char* query, ...);

	void escape (const char* source, char* dest, int destlen);

	DBRESULT* limitQuery (const char* query, unsigned int count, unsigned int offset, ...);

private:
	DBRESULT* vquery (const char* query, va_list va);
	sqlite3* db;
	char* errmsg;
};
#endif // DB_SQLITE

#endif // __DATABASE_H__

/* vim:set ts=2 sw=2: */
