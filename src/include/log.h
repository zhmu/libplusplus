/*
 * \file log.h
 * \brief Logging functionality
 *
 */
#ifndef __LOG_H__
#define __LOG_H__

#include <sys/types.h>
#include <syslog.h>

/*! \class LOG
 *  \brief Base class for logging messages.
 */
class LOG {
public:
	//! \brief This will initialize the log class
	LOG();

	/*!* \brief Returns a new logging object for a given type and name
	 *   \param type The desired logging type
	 *   \param name Log name
	 *   \return A new logger object on success or NULL if the type is
	 *           unsupported
	 */
	static LOG* getLog(char* type, char* name);

	//! \brief This will deinitialize the log class
	virtual ~LOG();

	/*! \brief This will log the message in printf() format.
	 *  \param pri Priority of the message
	 *  \param msg Message to log
	 * 	\param ... Extra parameters
	 */
	virtual void log (int pri, char* msg, ...) = 0;

protected:
	//! \brief The identification of the program
	char* ident;
};

/*! \class SYSLOG
 *  \brief Class for logging messages to syslogd(8).
 */
class SYSLOG : public LOG {
public:
	/*! \brief This will initialize the syslog class
	 *  \param name The name of the program or facility that logs
	 */
	SYSLOG(char* name);

	//! \brief This will deinitialize the syslog class
	~SYSLOG();

	/*! \brief This will log the message in printf() format.
	 *  \param pri Priority of the message
	 *	\param msg Message to log
	 * 	\param ... Extra parameters
	 */
	void log (int pri, char* msg, ...);
};

/*! \class STDLOG
 *  \brief Class for logging messages to stderr.
 */
class STDLOG : public LOG {
public:
	/*! \brief This will initialize the stdlog class
	 *  \param name The name of the program or facility that logs
	 */
	STDLOG(char* name);

	/*! \brief This will log the message in printf() format.
	 *  \param pri Priority of the message
	 * 	\param msg Message to log
	 */
	void log (int pri, char* msg, ...);
};

#endif // __LOG_H__

/* vim:set ts=2 sw=2: */
