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
 * \file ipx.cc
 * \brief Core IPX functionality
 *
 */
#ifdef OS_FREEBSD

#include <sys/types.h>
#include <sys/socket.h>
#include <netipx/ipx.h>
#include <string.h>
#include <unistd.h>
#include <network.h>
#include <ipx.h>

/*
 * IPXSERVER::create (int no)
 *
 * This will bind to IPX socket [no]. It will return zero on failure or
 * non-zero on success.
 *
 */
int
IPXSERVER::create (int no) {
	struct sockaddr_ipx sipx;

	// create a socket
	fd = socket (AF_IPX, 0, SOCK_DGRAM);
	if (fd < 0)
		return 0;

	// bind the socket
	memset (&sipx, 0, sizeof (struct sockaddr_ipx));
	sipx.sipx_family = AF_IPX;
	sipx.sipx_len = sizeof (struct sockaddr_ipx);
	sipx.sipx_addr.x_port = htons (no);
	if (bind (fd, (struct sockaddr*)&sipx, sizeof (struct sockaddr_ipx)) < 0) {
		// this failed. close the socket and return
		::close (fd); fd = -1;
		return 0;
	}

	// we accept all packets by default
	int i = 1;
	if (setsockopt (fd, 0, SO_ALL_PACKETS, &i, sizeof (i)) < 0) {
		// this failed. close the socket and return
		::close (fd); fd = -1;
		return 0;
	}

	// IPX is usually all about broadcasts, so enable them too
	i = 1;
	if (setsockopt (fd, 0, SO_BROADCAST, &i, sizeof (i)) < 0) {
		// this failed. close the socket and return
		::close (fd); fd = -1;
		return 0;
	}

	// all done
	return 1;
}

/*
 * IPXADDRESS::IPXADDRESS()
 *
 * This is the constructor.
 *
 */
IPXADDRESS::IPXADDRESS() {
	// build a pointer to the address, and set it up
	sipx = (struct sockaddr_ipx*)&saddr;
	sipx->sipx_len = sizeof (struct sockaddr_ipx);
	sipx->sipx_family = AF_IPX;
}

/*
 * IPXADDRESS::setAddr (char* addr)
 *
 * This will convert [addr] to the internal representation. It will return zero
 * on failure and non-zero on success.
 *
 */
int
IPXADDRESS::setAddr (char* addr) {
	// convert it
	struct ipx_addr iaddr = ipx_addr (addr);

	// copy it over
	bcopy (&iaddr, &sipx->sipx_addr, sizeof (struct ipx_addr));

	// all done
	return 1;
}

/*
 * IPXADDRESS::getAddr()
 *
 * This will return a human-readable IPX address.
 *
 */
char*
IPXADDRESS::getAddr() {
	return ipx_ntoa (sipx->sipx_addr);
}

/*
 * IPXADDRESS::setPort (int port)
 *
 * This will set the port number to [port].
 *
 */
void
IPXADDRESS::setPort (int port) {
	sipx->sipx_port = htons (port);
}

/*
 * IPXADDRESS::getPort ()
 *
 * This will retrieve the port number used.
 *
 */
int
IPXADDRESS::getPort () {
	return ntohs (sipx->sipx_port);
}
#endif // OS_FREEBSD

/* vim:set ts=2 sw=2: */
