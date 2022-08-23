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
 * \file netclient.cc
 * \brief Core network functionality, implements the NETCLIENT class
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <network.h>

/*
 * NETCLIENT::connect (NETADDRESS* addr)
 *
 * This will try to make a connection to address [addr]. It will return zero on
 * failure or non-zero on success.
 *
 */
int
NETCLIENT::connect (NETADDRESS* addr) {
	int lfd;

	// create a socket
	lfd = socket (AF_INET, SOCK_STREAM, 0);
	if (lfd < 0)
		return 0;

	// connect to the host
	if (::connect (lfd, addr->getInternalAddress(), addr->getInternalLength()) < 0) {
		// this failed. complain
		#ifdef _DEBUG_NETWORK
		perror ("NETCLIENT::connect(): connect() failed");
		#endif // _DEBUG_NETWORK
		::close (lfd);
		return 0;
	}

	// set the close-on-exec flag. this is required in case exec..() is used,
  // since clients can only exit if no processes occupy the sockets.
	fcntl (lfd, F_SETFD, FD_CLOEXEC);

	// victory
	setFD (lfd);
	return 1;
}

/* vim:set ts=2 sw=2: */
