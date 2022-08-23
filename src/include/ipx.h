/*
 * \file ipx.h
 * \brief Core IPX network functionality
 *
 */
#ifndef __IPX_H__
#define __IPX_H__

#include "network.h"

/*! \class IPXSERVER
		\brief IPX server class

		This class is capable of binding to a IPX socket, to which other clients can
		connect.
 */
class IPXSERVER : public NETSERVICE {
public:
	/*! \brief Creates an IPX server socket
	 *  \return Zero on failure and non-zero on success.
	 *  \param no The port number to open
	 */
	int create (int no);

	/*
	 * IPXSERVER is a client networking service! This is because IPX is
	 *	connectionless and thus nothing needs to be accepted.
   */
	int getType () { return NETSERVICE_CLIENT; };
};

#endif // __IPX_H__

/* vim:set ts=2 sw=2: */
