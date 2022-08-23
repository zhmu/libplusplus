/*
 * \file vector.h
 * \brief Dynamic vector class
 *
 */
#ifndef __VECTOR_H__
#define __VECTOR_H__

/*!	\class VECTOR
 *  \brief Class for storing anything
 *
 *  A vector can store objects, while dynamically resizing itself to keep up.
 */
class VECTOR {
public:
	//! \brief The constructor of the class.
	VECTOR();

	//! \brief The destructor of the class.
	~VECTOR();

	/*! \brief Adds an element to the vector
	 *  \param e The element to be added
	 */
	void addElement (void* e);

	/*! \brief Removes an element from the vector
	 *  \param e The element to be removed.
	 */
	void removeElement (void* e);

	//! \brief Returns the number of elements in the vector
	int	count ();

	/*! \brief Retrieves a specific element
	 *  \param no The position of the element
	 */
  void* elementAt (int no);

	//! \brief Performs a debugging dump of the contents of the vector
	void dump();
	
private:
	//! \brief The internal list of elements
	void* elements;

	//! \brief The maximum possible elements currently storable
	int	maxElements;

	//! \brief The number of currently stored elements
	int	numElements;
};

#endif // __VECTOR_H__

/* vim:set ts=2 sw=2: */
