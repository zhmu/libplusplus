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
 * \file netaddress.cc
 * \brief Core network functionality, implements the NETADDRESS class
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <network.h>

/*
 * NETADDRESS::NETADDRESS()
 *
 * This is the constructor.
 *
 */
NETADDRESS::NETADDRESS() {
	// wipe the internal representation
	memset (&saddr, 0, sizeof (struct sockaddr));
}

/*
 * NETADDRESS::getInternalAddress() {
 *
 * This will return the internal OS-specific representation of the IPX
 * address.
 *
 */
struct sockaddr*
NETADDRESS::getInternalAddress() {
	return &saddr;
}

/* vim:set ts=2 sw=2: */
