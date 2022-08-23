/*
 * \file network.h
 * \brief Core network functionality
 *
 */
#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#ifdef OS_FREEBSD
#include <netipx/ipx.h>
#endif /* OS_FREEBSD */
#include "vector.h"

// NETSERVICE is yet to come
class NETSERVICE;

//! \brief NETSERVICE_SERVER identifies a server class
#define NETSERVICE_SERVER 0

//! \brief NETSERVICE_CLIENT identifies a client class
#define NETSERVICE_CLIENT 1

/*! \class NETADDRESS
 *  \brief Holder of a protocol independant network address
 *
 * NETADDRESS can hold any network address, in a protocol independant way.
 */
class NETADDRESS {
public:
	//! \brief The constructor of this class
	NETADDRESS();

	//! \brief Returns the internal representation of te address
	struct sockaddr* getInternalAddress();

	/*! \brief Sets the address
	 *  \return Zero on failure or non-zero on success
	 *	\param addr The human-readable address to convert
	 *
   *  This will set the passed string into the protocol's specific address type.
	 */
	virtual int setAddr(char* addr) = 0;

	//! \brief This will return a human-readable address string
	virtual char* getAddr() = 0;

	/*! This will set the port number
			\arg port The port number to use */
	virtual void setPort (int port) = 0;

	//! This will retrieve the port number
	virtual int getPort () = 0;

	//! This will retrieve the length of the internal representation
	virtual int getInternalLength() { return sizeof (struct sockaddr); };

	/*! \brief Compares the supplied address with the address stored
	 *  \return Non-zero on a match, zero if no match
	 *  \param addr The adress to match
	 */
	inline virtual int compareAddr (char* addr) { return 0; }

protected:
	//! \brief Internal representation of the address
	struct sockaddr saddr;
};

/*! \class IPV4ADDRESS
 *  \brief Holder of an IPv4 network address
 */
class IPV4ADDRESS : public NETADDRESS {
public:
	//! \brief This is the constructor
	IPV4ADDRESS();

	/*! \brief Sets an IPv4 address
	 *  \return Zero on failure or non-zero on success
	 *  \param addr The address to use
	 *
	 *  This function understands both hosts as IPv4 addresses.
	 */
	int setAddr(char* addr);

	//! \brief This will return the IPv4 address stored as human-readable text
	char* getAddr();

	/*! \brief This will set the port number
	 *  \arg port The port number to use
	 */
	void setPort (int port);

	//! \brief Returns the port number
	int getPort ();

	//! \brief Retrieves the length of the internal representation
	virtual int getInternalLength() { return sizeof (struct sockaddr_in); };

	/*! \brief Compares the supplied IPv4 address with the address stored
	 *  \return Non-zero on a match, zero if no match
	 *  \param addr The adress to match
	 *
	 *  The current implementation only supports dotted IPv4 addresses.
	 */
	int compareAddr (char* addr);

private:
	// This is just the cast we need to correctly access the internal address
	struct sockaddr_in* sin;
};

/*! \class IPXADDRESS
 *  \brief Holder of an IPX network address
 */
class IPXADDRESS : public NETADDRESS {
public:
	//! \brief This is the constructor
	IPXADDRESS();

	/*! \brief Sets an IPX address
	 *  \return Zero on failure or non-zero on success
	 *  \param addr The address to use
	 */
	int setAddr(char* addr);

	//! \brief Returns the IPX address stored as human-readable text
	char* getAddr();

	/*! \brief Sets the port number
	 *  \param port The port number to use
	 */
	void setPort (int port);

	//! \brief Returns the port number
	int getPort ();

	//! \brief Retrieves the length of the internal representation
	virtual int getInternalLength() { return sizeof (struct sockaddr_in); };

private:
	// This is just the cast we need to correctly access the internal address
	struct sockaddr_ipx* sipx;
};

/*!	\class NETWORK
		\brief The core network class

		The network keeps all NETSERVICE-es in control and ensures their functions
		are called when needed.
 */
class NETWORK {
public:
	//! \brief The constructor of the class.
	NETWORK();

	/*!	\brief Adds a service to the network for monitoring.
	 *  \param service The service to be monitored.
	 */
	void addService (NETSERVICE* service);

	/*! \brief Removes a service from the network.
	 *  \param service The service to be removed.
	 */
	void removeService (NETSERVICE* service);

	/*! \brief Launches the network and handles events
	 *
	 *  This will return whenever an event has been handeled.
	 */
	void run ();

private:
	// \brief The internal list of services to be monitored
	VECTOR* services;
};

/*! \class NETSERVICE
 *  \brief A prototype of a network service
 *
 *  This is the protoype of a network service. Services attach themselves to the
 * 	NETWORK.
 */
class NETSERVICE {
	// everybody loves somebody ... ;-)
	friend class NETWORK;

public:
	//! \brief The constructor of the class.
	NETSERVICE();

	//! \brief The destructor of the class.
	virtual	~NETSERVICE();

	//! \brief Returns the associated file descriptor
	int	getFD();

	/*! \brief Change the associated file descriptor
	 *  \param no The new file descriptor to use
	 */
	void setFD(int no);

	/*! \brief Set the associated parent object
	 *  \param p The new parent object
	 */
	void setParent (NETSERVICE* p);

	/*! \brief Set the address of the client
	 *  \param addr The new client address
	 */
	void setClientAddress (NETADDRESS* addr);

	/*! \brief Sends data to the socket
	 *	\return The number of bytes sent
	 *  \param buf Buffer of data to send
	 *  \param len Size of the buffer
   */
	int	send (char* buf, int len);

	/*! \brief Sends printf()-formatted data to the socket
	 *  \return The number of bytes sent
	 *  \param fmt Format specifier for printf()
	 *  \param ... Parameters for printf()
	 */
	int	sendf (char* fmt, ...);

	/*! \brief Determines the service type
	 *  \return The service type
	 *
	 * This must return NETSERVICE_SERVER or NETSERVICE_CLIENT. The difference
	 * between the two is that a NETSERVICE_CLIENT will always be checked to
	 * have data available when it is executed, and the other will not.
	 */
	virtual int getType () = 0;

	/*! \brief Determine whether the connection is active
	 *  \return Non-zero if the connection is active, or zero if not
	 */
	virtual int isActive();

	//! \brief Retrieves the parent object
	NETSERVICE* getParent ();

	//! \brief Retrieves the client address
	NETADDRESS* getClientAddress ();

protected:
	/*! \brief Callback function to handle events
	 *
	 *  This will be called whenever NETWORK::run() notices an event for the file
	 *  descriptor used by the service.
	 */
	virtual void incoming() = 0;

	/*! \brief Remove a client from the client list
	 *  \param client The client to remove
	 */
	void removeClient (NETSERVICE* client);

	/*! \brief Add a client to the client list
	 *  \param client The client to add
	 */
	void addClient (NETSERVICE* client);

	//! \brief Returns a vector containing all clients
	VECTOR* getClients();

	//! \brief Holds the file descriptor used by the service
	int	fd;

	//! \brief Contains a file reference to the descriptor
	FILE* filp;

	/*! \brief Reads data from the socket
	 *  \return The number of bytes retrieved
	 *  \param buf Buffer to handle the data
	 *  \param len Size of the buffer
	 */
	virtual int recv (char* buf, int len);

	/*! \briefs Peeks whether there is available data
	 *  \return Non-zero if there is data available, zero if there is none
	 */
	int peek ();

	//! \brief Closes the connection
	void close ();

private:
	//! \brief Holds all attached clients
	VECTOR*	clients;

	//! \brief Holds the parent class
	NETSERVICE* parent;

	//! \brief Holds the address of whoever connected to us
	NETADDRESS* clientAddress;
};

/*! \class SERVICECLIENT
 *  \brief Client connected to a server
 *
 *  This class is capable of maintaining connections made to a server class.
 */
class SERVICECLIENT : public NETSERVICE {
public:
	/*! \brief Callback handler for an event
	 *
	 * This will be called whenever NETWORK::run() notices an event for the file
	 * descriptor used by the service */
	virtual void incoming() = 0;

	// SERVICECLIENT is a client networking service
	inline int getType () { return NETSERVICE_CLIENT; };
};

/*! \class NETSERVER
 *  \brief TCP server class
 *
 *  This class is capable of binding to a TCP socket, to which other clients can
 *	connect.
 */
class NETSERVER : public NETSERVICE {
public:
	/*! \brief Creates a server TCP socket
	 *  \return Zero on failure and non-zero on failure
	 *  \param no The port number to open
	 */
	int create (int no);

	// NETSERVER is a server networking service
	inline int getType () { return NETSERVICE_SERVER; };

protected:
	/*! \brief Accepts a new connection
	 *  \returns Non-zero on success and non-zero on failure
	 *
	 *  If anything fails, client will automatically be deleted.
	 */
	int accept (SERVICECLIENT* client);
};

/*! \class NETCLIENT
 *  \brief TCP client class
 *
 *  This class is capable of connecting to a TCP socket.
 */
class NETCLIENT : public NETSERVICE {
public:
	/*! \brief Creates a new connection to a server
	 *  \returns Non-zero on success and non-zero on failure
	 *  \param addr The address to connect to.
	 */
	int connect (NETADDRESS* addr);

	// NETCLIENT is a client networking service
	inline int getType () { return NETSERVICE_CLIENT; };

protected:
	/*! \brief Callback handler for an event
	 *
	 * This will be called whenever NETWORK::run() notices an event for the file
	 * descriptor used by the service, such as new data having arrived */
	virtual void incoming() = 0;
};

#endif // __NETWORK_H__

/* vim:set ts=2 sw=2: */
