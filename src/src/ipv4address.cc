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
 * \file ipv4address.cc
 * \brief Core network functionality, implements the IPV4ADDRESS class
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
 * IPV4ADDRESS::IPV4ADDRESS()
 *
 * This is the constructor.
 *
 */
IPV4ADDRESS::IPV4ADDRESS() {
	// build a pointer to the address, and set it up
	sin = (struct sockaddr_in*)&saddr;
	#ifdef OS_BSD
	sin->sin_len = sizeof (struct sockaddr_in);
	#endif // OS_BSD
	sin->sin_family = AF_INET;
}

/*
 * IPV4ADDRESS::setAddr (char* addr)
 *
 * This will convert [addr] to the internal representation. It accepts hostnames
 * and IPv4 addresses. It will return zero on failure and non-zero on success.
 *
 */
int
IPV4ADDRESS::setAddr (char* addr) {
	struct hostent* hent;

	// try to resolve this as an ip address
	if (!inet_aton (addr, &sin->sin_addr)) {
		// this did not work. try to resolve it
		hent = gethostbyname (addr);
		if (hent == NULL) {
			// this failed too. bail out
			return 0;
		}

		// copy the address over
		memcpy (&sin->sin_addr, hent->h_addr_list[0], sizeof (sin->sin_addr));
	}

	// all done
	return 1;
}

/*
 * IPV4ADDRESS::getAddr()
 *
 * This will return a human-readable IPv4 address.
 *
 */
char*
IPV4ADDRESS::getAddr() {
	//return addr2ascii (AF_INET, (void*)&saddr, sizeof (struct sockaddr_in), NULL);
	return inet_ntoa (sin->sin_addr);
}

/*
 * IPV4ADDRESS::setPort (int port)
 *
 * This will set the port number to [port].
 *
 */
void
IPV4ADDRESS::setPort (int port) {
	sin->sin_port = htons (port);
}

/*
 * IPV4ADDRESS::getPort ()
 *
 * This will retrieve the port number used.
 *
 */
int
IPV4ADDRESS::getPort () {
	return ntohs (sin->sin_port);
}

/*
 * IPV4ADDRESS::compareAddr (char* addr)
 *
 * This will check whether the address stored matches [addr]. It will return zero
 * if not an non-zero if it does.
 *
 */
int
IPV4ADDRESS::compareAddr (char* addr) {
	struct sockaddr_in stmp;

	// convert the supplied address to network order
	if (!inet_aton (addr, &stmp.sin_addr))
		// this did not work. bail out
		return 0;

	// it's all up to the match now
	return (!memcmp (&stmp.sin_addr, &sin->sin_addr, sizeof (sin->sin_addr))) ? 1 : 0;
}

/* vim:set ts=2 sw=2: */
