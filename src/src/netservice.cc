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
 * \file netservice.cc
 * \brief Core network functionality, implements the NETSERVICE class.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <network.h>

/*
 * NETSERVICE::NETSERVICE()
 *
 * This is the constructor.
 *
 */
NETSERVICE::NETSERVICE() {
	// no file descriptors nor clients just yet
	fd = -1; clients = new VECTOR(); parent = NULL; clientAddress = NULL;
	filp = NULL;
}

/*
 * NETSERVICE::~NETSERVICE()
 *
 * This is the destructor.
 *
 */
NETSERVICE::~NETSERVICE() {
	// do we have a client address?
	if (clientAddress)
		// yes. get rid of it
		delete clientAddress;

	// close all connections
	close();

	// get rid of the clients
	if (clients)
		delete clients;
}

/*
 * NETSERVICE::getFD()
 *
 * This will return the network service's file descriptor.
 *
 */
int
NETSERVICE::getFD() {
	return fd;
}

/*
 * NETSERVICE::setFD(int no)
 *
 * This will change the network service's file descriptor to [no].
 *
 */
void
NETSERVICE::setFD(int no) {
	fd = no;

	if (fd != -1) {
		/* associate a file structure with the descriptor. no buffering please */
		filp = fdopen (fd, "a+b");
		setvbuf (filp, NULL, _IONBF, 0);
	}
}

/*
 * NETSERVICE::getClients()
 *
 * This will return a vector containing all clients.
 *
 */
VECTOR*
NETSERVICE::getClients() {
	return clients;
}

/*
 * NETSERVICE::getParent()
 *
 * This will retrieve the parent object.
 *
 */
NETSERVICE*
NETSERVICE::getParent() {
	return parent;
}

/*
 * NETSERVICE::getClientAddress ()
 *
 * This will retrieve the client address.
 *
 */
NETADDRESS*
NETSERVICE::getClientAddress () {
	return clientAddress;
}

/*
 * NETSERVICE::setParent(NETSERVICE* p)
 *
 * This will set the parent object to [p].
 *
 */
void
NETSERVICE::setParent(NETSERVICE* p) {
	parent = p;
}

/*
 * NETSERVICE::setClientAddress (NETADDRESS* addr)
 *
 * This will set the client address to [addr].
 *
 */
void
NETSERVICE::setClientAddress (NETADDRESS* addr) {
	clientAddress = addr;
}

/*
 * NETSERVICE::recv (char* buf, int len)
 *
 * This will try to receive up to [len] bytes into [buf]. It will return the
 * number of bytes received.
 *
 */
int
NETSERVICE::recv (char* buf, int len) {
	// got a file descriptor at hand ?
	if (fd == -1)
		// no. refuse to read anything
		return 0;

	// fetch the data
	int i = ::recv (fd, buf, len, 0);

	// return the size
	return (i == -1) ? 0 : i;
}

/*
 * NETSERVICE::send (char* buf, int len)
 *
 * This will try to send up to [len] bytes from [buf]. It will return the
 * number of bytes sent.
 *
 */
int
NETSERVICE::send (char* buf, int len) {
	// got a file descriptor at hand ?
	if (fd == -1)
		// no. refuse to read anything
		return 0;

	// send the data
	int i = ::send (fd, buf, len, 0);

	// return the size
	return (i == -1) ? 0 : i;
}

/*
 * NETSERVICE::sendf (char* fmt, ...)
 *
 * This will try to send printf() formatted [fmt] to the socket. It will return
 * the number of bytes sent.
 *
 */
int
NETSERVICE::sendf (char* fmt, ...) {
	va_list ap;
	int ret;

	// got a file descriptor at hand ?
	if (fd == -1)
		// no. refuse to read anything
		return 0;

	// build the data to send
	va_start (ap, fmt);
	ret = vfprintf (filp, fmt, ap);
	va_end (ap);
	return ret;
}

/*
 * NETSERVICE::close ()
 *
 * This will close the network connection.
 *
 */
void
NETSERVICE::close () {
	NETSERVICE* c;

	// got a file descriptor ?
	if (fd != -1) {
		// yes. close it
		#ifdef _DEBUG_NETWORK
		printf ("NETSERVICE(): closed fd %u for 0x%x\n", fd, (unsigned int)this);
		#endif // _DEBUG_NETWORK
		if (filp != NULL) {
			fflush (filp);
			fclose (filp);
		} else {
			::shutdown (fd, SHUT_RDWR);
			::close (fd);
		}
		fd = -1;
	}

	// do we have a parent object ?
	if (parent) {
		// yes. remove us as a client
		parent->removeClient (this);
	}

	// scan all clients, too
	for (int i = 0; i < clients->count(); i++) {
		// fetch the client
		c = (NETSERVICE*)clients->elementAt (i);

		#ifdef _DEBUG_NETWORK
		printf ("NETSERVICE(): close(): closing 0x%x for 0x%x\n", (unsigned int)c, (unsigned int)this);
		#endif // _DEBUG_NETWORK

		// close the connection of this client
		c->close();

		// get rid of the object, too
		clients->removeElement (c);
		delete c;
	}
}

/*
 * NETSERVICE::removeClient (NETSERVICE* client)
 *
 * This will remove [client] from the list of clients.
 *
 */
void
NETSERVICE::removeClient (NETSERVICE* client) {
	clients->removeElement (client);
}

/*
 * NETSERVICE::addClient (NETSERVICE* client)
 *
 * This will add [client] to list of clients.
 *
 */
void
NETSERVICE::addClient (NETSERVICE* client) {
	clients->addElement (client);
}

/*
 * NETSERVICE::peek ()
 *
 * This will peek whether we have data available. It will return zero if not
 * and non-zero if so.
 *
 */
int
NETSERVICE::peek() {
	char buf;

	// got a file descriptor at hand ?
	if (fd == -1)
		// no. we can never have data ready
		return 0;

	// fetch the data
	int i = ::recv (fd, &buf, 1, MSG_PEEK);

	// return the size
	return (i == -1) ? 0 : i;
}

/*
 * NETSERVICE::isActive()
 *
 * This will return non-zero if the service connection is active, or zero if it
 * is not.
 *
 */
int
NETSERVICE::isActive() {
	return (fd == -1) ? 0 : 1;
}

/* vim:set ts=2 sw=2: */
