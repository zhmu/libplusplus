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
 * \file network.cc
 * \brief Core network functionality
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
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
 * NETWORK::NETWORK()
 *
 * This is the constructor.
 *
 */
NETWORK::NETWORK() {
	// no services just yet
	services = new VECTOR();
}

/*
 * NETWORK::addService (NETSERVICE* service)
 *
 * This will add service [service] to the chain of services.
 *
 */
void
NETWORK::addService (NETSERVICE* service) {
	// add the service to the vector
	services->addElement (service);

	#ifdef _DEBUG_NETWORK
	printf ("NETWORK::addService(): service 0x%p added\n", service);
	#endif // _DEBUG_NETWORK
}

/*
 * NETWORK::removeService (NETSERVICE* service)
 *
 * This will remove service [service] from the chain of services.
 *
 */
void
NETWORK::removeService (NETSERVICE* service) {
	// remove the service from the vector
	services->removeElement (service);

	#ifdef _DEBUG_NETWORK
	printf ("NETWORK::removeService(): service 0x%p removed\n", service);
	#endif // _DEBUG_NETWORK
}

/*
 * NETWORK::run()
 *
 * This will monitor the network.
 *
 */
void
NETWORK::run() {
	fd_set fds;
	int		 i, j, fd, fdmax;
	NETSERVICE* service;
	NETSERVICE* subservice;

	// construct the set of file descriptors to monitor
	FD_ZERO (&fds); fdmax = -1;
	for (i = 0; i < services->count(); i++) {
		// fetch the service
		service = (NETSERVICE*)services->elementAt (i);

		// fetch the file descriptor
		fd = service->getFD();

		// valid descriptor ?
		if (fd != -1) {
			// yes. append it to the list
			FD_SET (fd, &fds);

			#ifdef _DEBUG_NETWORK
			printf ("NETWORK::run(): added fd %u for service 0x%p\n", fd, service);
			#endif // _DEBUG_NETWORK

			// if we have a new maximum, use it
			if (fdmax < fd)
				fdmax = fd;
		}

		// check for service's clients
		for (j = 0; j < service->getClients()->count(); j++) {
			// fetch the service
			subservice = (NETSERVICE*)service->getClients()->elementAt (j);

			// fetch the file descriptor
			fd = subservice->getFD();

			// valid descriptor ?
			if (fd != -1) {
				// yes. append it to the list
				FD_SET (fd, &fds);

				#ifdef _DEBUG_NETWORK
				printf ("NETWORK::run(): added fd %u for client 0x%p for service 0x%p\n", fd, subservice, service);
				#endif // _DEBUG_NETWORK


				// if we have a new maximum, use it
				if (fdmax < fd)
					fdmax = fd;
			}
		}
	}

	// await a connection
	if (select (fdmax + 1, &fds, (fd_set*)NULL, (fd_set*)NULL, (struct timeval*)NULL) < 0) {
		// this failed. return
		#ifdef _DEBUG_NETWORK
		perror ("NETWORK::run(): select() ended unsuccessfully");
		#endif // _DEBUG_NETWORK
		return;
	}

	// if this is reached, we got a connection. browse the service list again
	// to figure out who should get this event
	for (i = 0; i < services->count(); i++) {
		// fetch the service
		service = (NETSERVICE*)services->elementAt (i);

		// fetch the file descriptor
		fd = service->getFD();

		// do we have a valid file descriptor ?
		if (fd != -1) {
			// yes. did this service generate the event ?
			if (FD_ISSET (fd, &fds)) {
				// yes. is this a server socket ?
				if (service->getType() == NETSERVICE_SERVER) {
					// yes. handle the incoming connection
					#ifdef _DEBUG_NETWORK
					printf ("NETWORK::run(): calling incoming() for server service 0x%p\n", service);
					#endif // _DEBUG_NETWORK
					service->incoming();
				} else {
					// no. is there actual data available ?
					if (service->peek()) {
						// yes. handle the incoming connection
						#ifdef _DEBUG_NETWORK
						printf ("NETWORK::run(): calling incoming() for client service 0x%p\n", service);
						#endif // _DEBUG_NETWORK
						service->incoming();
					} else {
						// no. drop the connection
						#ifdef _DEBUG_NETWORK
						printf ("NETWORK::run(): dropping service 0x%p\n", service);
						#endif // _DEBUG_NETWORK

						// mark the service as removed
						service->setFD (-1);
						services->removeElement (service);
						service = NULL;
					}
				}
			}

			// still have a service left ?
			if (service) {
				// yes. check for service's clients
				for (j = 0; j < service->getClients()->count(); j++) {
					// fetch the service
					subservice = (NETSERVICE*)service->getClients()->elementAt (j);

					// fetch the file descriptor
					fd = subservice->getFD();

					// valid descriptor ?
					if (fd != -1) {
						// yes. did this service generate the event ?
						if (FD_ISSET (fd, &fds)) {
							// yes. do we really have data available ?
							if (subservice->peek()) {
								// yes. call the service's incoming() function
								#ifdef _DEBUG_NETWORK
								printf ("NETWORK::run(): calling incoming() for client 0x%p for service 0x%p\n", subservice, service);
								#endif // _DEBUG_NETWORK
								subservice->incoming();
							} else {
								// no. drop the connection
								#ifdef _DEBUG_NETWORK
								printf ("NETWORK::run(): dropping connection for client 0x%p for service 0x%p\n", subservice, service);
								#endif // _DEBUG_NETWORK
								delete subservice;
								service->getClients()->removeElement (subservice);

								// ensure we don't skip the next client in the for loop
								j--;
							}
						}
					}
				}
			}
		}
	}
}

/* vim:set ts=2 sw=2: */
