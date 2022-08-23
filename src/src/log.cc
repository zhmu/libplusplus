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
 * \file log.cc
 * \brief Core logging functionality
 *
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <log.h>

/*
 * LOG::LOG()
 *
 * This is the standard log constructor, which will initialize the logger
 * without identification.
 *
 */
LOG::LOG() {
	// no identication
	ident = NULL;
}

/*
 * LOG::~LOG()
 *
 * This is the standard log constructor, which will deinitialize the logger.
 *
 */
LOG::~LOG() {
	// got identication?
	if (ident)
		// yes. zap it
		free (ident);
}

/*
 * This will return a new logger object for type [type] with name [name],
 * or NULL if the type isn't supported.
 *
 */
LOG*
LOG::getLog(char* type, char* name) {
#ifndef OS_WINDOWS
		if (!strcasecmp(type, "syslog"))
			return new SYSLOG(name);
#endif /* !OS_WINDOWS */

	if (!strcasecmp(type, "stderr"))
		return new STDLOG(name);

	// no such log
	return NULL;
}

#ifndef OS_WINDOWS
/*
 * SYSLOG::SYSLOG (char* namet)
 *
 * This will initialize the system logger with identification [name].
 *
 */
SYSLOG::SYSLOG (char* name) {
	openlog (name, LOG_ERR, LOG_USER);
}

/*
 *
 * SYSLOG::~SYSLOG()
 *
 * This will deinitialize the system logger.
 *
 */
SYSLOG::~SYSLOG() {
	closelog();
}

/*
 * SYSLOG::log (int pri, char* msg, ...)
 *
 * This will log message [msg] at priority [pri].
 *
 */
void
SYSLOG::log (int pri, char* msg, ...) {
	va_list va;

	va_start (va, msg);
	vsyslog (pri, msg, va);
	va_end (va);
}
#endif /* OS_WINDOWS */

/*
 * STDLOG::STDLOG (char* namet)
 *
 * This will initialize the standard logger with identification [name].
 *
 */
STDLOG::STDLOG (char* name) {
	ident = strdup (name);
}

/*
 * STDLOG::log (int pri, char* msg, ...)
 *
 * This will log message [msg] at priority [pri].
 *
 */
void
STDLOG::log (int pri, char* msg, ...) {
	va_list va;
	time_t now;
	
	#ifndef _DEBUG_LOG
	// we are not compiled in debugging mode. got a debug log message?
	if (pri == LOG_DEBUG)
		// yes. discard it
		return;
	#endif
	
	// fetch the time
	time (&now);

	// log the request
	va_start (va, msg);
	fprintf (stderr, "%.24s %s: ", ctime (&now), ident);
#if 0
	// this is useless...
	switch (pri) {
		case LOG_EMERG: fprintf (stderr, "EMERGENCY"); break;
		case LOG_ALERT: fprintf (stderr, "ALERT"); break;
		 case LOG_CRIT: fprintf (stderr, "CRITICAL"); break;
		  case LOG_ERR: fprintf (stderr, "ERROR"); break;
  case LOG_WARNING: fprintf (stderr, "WARNING"); break;
   case LOG_NOTICE: fprintf (stderr, "NOTICE"); break;
		 case LOG_INFO: fprintf (stderr, "INFO"); break;
		case LOG_DEBUG: fprintf (stderr, "DEBUG"); break;
	}
	fprintf (stderr, ": ");
#endif
	vfprintf (stderr, msg, va);

	// add a newline
	fprintf (stderr, "\n");
	va_end (va);
}

/* vim:set ts=2 sw=2: */
