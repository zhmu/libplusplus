/*
 * libplusplus - A generic C++ library for networking, databases and more
 * Copyright (C) 2002-2004 Rink Springer
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
 * \file vector.cc
 * \brief Vector class implementation
 *
 */
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector.h>

/*
 * VECTOR::VECTOR()
 *
 * This will construct an empty vector.
 *
 */
VECTOR::VECTOR() {
	// chose a reasonable default
	maxElements = 10;

	// allocate memory
	elements = malloc (maxElements * sizeof (void*));

	// empty now
	numElements = 0;
}

/*
 * VECTOR::~VECTOR()
 *
 * This will destruct the vector.
 *
 */
VECTOR::~VECTOR() {
	// free the memory pool
	free (elements);
}

/*
 * VECTOR::addElement (void* e)
 *
 * This will add element [e] to the vector.
 *
 */
void
VECTOR::addElement (void* e) {
	#ifdef _DEBUG_VECTOR
		printf ("VECTOR::addElement (0x%x): before add:\n", (caddr_t)e);
		dump();
	#endif // _DEBUG_VECTOR

	// do we still have enough space in our vector?
	if ((numElements + 1) >= maxElements) {
		// yes. resize the vector to twice the current size
		maxElements *= 2;
		elements = realloc (elements, maxElements * sizeof (caddr_t));
	}

	// insert the element
	*((caddr_t*)elements + numElements++) = (caddr_t)e;

	#ifdef _DEBUG_VECTOR
		printf ("VECTOR::addElement (0x%x): after add:\n", (caddr_t)e);
		dump();
	#endif // _DEBUG_VECTOR
}

/*
 * VECTOR::removeElement (void* e)
 *
 * This will remove element [e] from the vector.
 *
 */
void
VECTOR::removeElement (void* e) {
	#ifdef _DEBUG_VECTOR
		printf ("VECTOR::removeElement (0x%x): before remove:\n", (caddr_t)e);
		dump();
	#endif // _DEBUG_VECTOR

	// keep scanning the vector until we find it
	for (int i = 0; i < numElements; i++) {
		// got it ?
		if (*((caddr_t*)elements + i) == (caddr_t)e) {
			// yes. is this the last one?
			if (i != (numElements - 1))
				// no. copy the last one over us
				*((caddr_t*)elements + i) = *((caddr_t*)elements + numElements - 1);

			// one element less
			numElements--;

			#ifdef _DEBUG_VECTOR
				printf ("VECTOR::removeElement (0x%x): success, after remove:\n", (caddr_t)e);
				dump();
			#endif // _DEBUG_VECTOR

			// all done
			return;
		}
	}

	// not found
}

/*
 * VECTOR::count()
 *
 * This will return the vector's element count.
 *
 */
int
VECTOR::count() {
	return numElements;
}

/*
 * VECTOR::elementAt (int no)
 *
 * This will return the element at position [no].
 *
 */
void*
VECTOR::elementAt (int no) {
	return (void*)*((caddr_t*)elements + no);
}

/*
 * VECTOR::dump()
 *
 * This will do a debugging dump of a vector.
 *
 */
void
VECTOR::dump() {
#ifdef _DEBUG_VECTOR
	printf ("Vector dump of vector 0x%x\n", (caddr_t)this);
	for (int i = 0; i < numElements; i++) {
		printf ("%u -> 0x%x\n", i, (caddr_t)elementAt (i));
	}
#endif // _DEBUG_VECTOR
}

/* vim:set ts=2 sw=2: */
