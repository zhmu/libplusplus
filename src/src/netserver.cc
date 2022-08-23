/*k
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
 * \file netserver.cc
 * \brief Core network functionality, implements the NETSERVER class
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <network.h>

/*
 * NETSERVER::create (int no)
 *
 * This will create a TCP port [no], listening for connections. It will return
 * zero on failure or non-zero on success.
 *
 */
int
NETSERVER::create (int no) {
	struct sockaddr_in sin;
	int on = 1;
	int lfd;

	// create a socket
	lfd = socket (AF_INET, SOCK_STREAM, 0);
	if (lfd < 0)
		return 0;
	
	// ensure we can bind to the socket
	setsockopt (lfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on));

	// set the bind structure up
	memset (&sin, 0, sizeof (struct sockaddr_in));
	#ifdef OS_BSD
	sin.sin_len = sizeof (struct sockaddr_in);
	#endif // OS_BSD
	sin.sin_family = AF_INET;
	sin.sin_port = htons (no);

	// bind the socket
	if (bind (lfd, (struct sockaddr*)&sin, sizeof (struct sockaddr_in)) < 0)
		// this failed. too bad
		goto fail;

	// listen for incoming connections
	if (listen (lfd, 5) < 0)
		// this failed. too bad
		goto fail;

#if 0
	// set the close-on-exec flag. this is required in case exec..() is used,
  // since clients can only exit if no processes occupy the sockets.
	fcntl (lfd, F_SETFD, FD_CLOEXEC);
#endif

	// victory
	setFD (lfd);
	return 1;

fail:
	::close (lfd);
	return 0;
}

/*
 * NETSERVER::accept (SERVICECLIENT* client)
 *
 * This will assign the pending connection to [client]. It will return zero on
 * failure and non-zero on success.
 *
 */
int
NETSERVER::accept (SERVICECLIENT* client) {
	IPV4ADDRESS* addr = new IPV4ADDRESS();
	socklen_t slen = addr->getInternalLength();
	int client_fd;

	// accept the connection
	client_fd = ::accept (fd, addr->getInternalAddress(), &slen);
	if (client_fd < 0) {
		// this failed. get rid of the client and leave
		delete addr; delete client;
		return 0;
	}

	// set the close-on-exec flag. this is required in case exec..() is used,
  // since clients can only exit if no processes occupy the sockets.
	fcntl (client_fd, F_SETFD, FD_CLOEXEC);
	
	// assign the client the correct file descriptor and parent
	client->setFD (client_fd);
	client->setParent (this);
	client->setClientAddress (addr);

	// append the client to the pool of clients
	addClient (client);

	#ifdef _DEBUG_NETWORK
	printf ("NETSERVER::accept(): client 0x%x added\n", (unsigned int)client);
	#endif // _DEBUG_NETWORK

	// all is fine
	return 1;
}

/* vim:set ts=2 sw=2: */
