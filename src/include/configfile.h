/*
 * \file configfile.h
 * \brief Configuration file reader
 *
 */
#include <stdio.h>

#ifndef __CONFIGFILE_H__
#define __CONFIGFILE_H__

/* CONFIGFILE_ERROR_xxx are error codes. CONFIGFILE_OK means victory. */
#define CONFIGFILE_OK                 0       /* victory */
#define CONFIGFILE_ERROR_NOFILE       1       /* no config file */
#define CONFIGFILE_ERROR_OPENERR      2       /* can't open file */
#define CONFIGFILE_ERROR_OUTOFMEM     3       /* out of memory */
#define CONFIGFILE_ERROR_READERR      4       /* read error */
#define CONFIGFILE_ERROR_NOSECTION    5       /* no such section */
#define CONFIGFILE_ERROR_NOVALUE      6       /* no such value */
#define CONFIGFILE_ERROR_INVALIDVALUE 7       /* invalid value */

/*! \class CONFIGFILE
 *  \brief CONFIGFILE is reponsible for parsing and understanding
 *		   configuration files.
 */
class CONFIGFILE {
public:
	//! \brief Initializes a new configuration file
	CONFIGFILE();

	//! \brief Deinitializes the configuration file
	virtual ~CONFIGFILE();

	/*! \brief Load a configuration file
	 *  \return CONFIGFILE_OK on success or CONFIGFILE_ERROR_xxx on failure
	 *  \param fname The name of the configuration file to be loaded
	 */
	int   load(char* fname);

	/*! \brief Reloads te configuration file
	 *  \return CONFIGFILE_OK on success or CONFIGFILE_ERROR_xxx on failure
	 */
	int   reload();

	/*! \brief Fetch a numeric value from the configuration file
	 *  \return CONFIGFILE_OK on success or CONFIGFILE_ERROR_xxx on failure
	 *  \param section The section in which the value resides
	 *  \param name The name of the value to be fetched
	 *  \param value Pointer in which the value must be returned
	 */
	int   get_string(char* section, char* name, char** dest);

	/*! \brief Fetch a value from an identifier in a section
	 *  \return CONFIGFILE_OK on success or CONFIGFILE_ERROR_xxx on failure
	 *  \param section The section in which the value resides
	 *  \param name The name of the value to be fetched
	 *  \param dest Pointer which will be replaced by a pointer to the value
	 */
	int   get_value(char* section, char* name, int* value);

	/*! \brief Scan for a section in the configuration file
	 *  \return CONFIGFILE_OK on success or CONFIGFILE_ERROR_xxx on failure
	 *  \param name Pointer in which the name will be returned
 	 *  \param offset Pointer to the source and destination offsets
	 *
	 *  offset is used as the offset start, but will be replaced by the offset
	 *  at which the new section is found.
	 *
	 */
	int   scan_section(char** name, int* offset);

	/*! \brief Scan for a value in the file
	 *  \return CONFIGFILE_OK on success or CONFIGFILE_ERROR_xxx on failure
	 *  \param name Pointer which will contain the name of the value
	 *  \param value Pointer which will contain the actual value
	 *  \param offset Source and destination offset 
	 *
	 *  offset is used as the offset start, but will be replaced by the offset
	 *  at which the new value is found.
	 */
	int   scan_value(char** name, char** value, int* offset);

private:
	char* fname;
	char* buf;
	int   buf_length;

	/*! \brief Parses all settings in the configuration file
	 *
	 * An application should overload this, for it will be called whenever
	 * the configuration is (re)loaded. It should reset everything to the defaults
	 * and parse the file.
	 */
	virtual void parse();

	/*! \brief Scans for a specific section within the configuration file
	 *  \return CONFIGFILE_OK on success or CONFIGFILE_ERROR_xxx on failure
	 *  \param section The section to be found
	 *  \param offset The source and destination offset
	 *
	 *  offset is used as the offset start, but will be replaced by the offset
	 *  at which the new value is found.
	 */
	int   find_section(char* section, int* offset);
};
#endif

/* vim:set ts=2 sw=2: */
