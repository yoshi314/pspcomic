/** \file
	RAR-related functions header

    Copyright (C) 2007 - 2008 Jeffrey P.

	This file is licensed under the UnRAR license. You may not use it to
	recreate the compression algorithm used in the Roshal Archive (RAR)
	format. For more information, see the file rar/license.txt.
**/
#ifndef PSPComic_rar_h
#define PSPComic_rar_h


#ifndef PSPComic_h
#include "rar/rar.hpp"
#include "rar/dll.hpp"
#else
/** Taken from rar/dll.hpp **/
#define ERAR_END_ARCHIVE        10
#define ERAR_NO_MEMORY          11
#define ERAR_BAD_DATA           12
#define ERAR_BAD_ARCHIVE        13
#define ERAR_UNKNOWN_FORMAT     14
#define ERAR_EOPEN              15
#define ERAR_ECREATE            16
#define ERAR_ECLOSE             17
#define ERAR_EREAD              18
#define ERAR_EWRITE             19
#define ERAR_SMALL_BUF          20
#define ERAR_UNKNOWN            21
#define ERAR_MISSING_PASSWORD   22

#define RAR_OM_LIST           0
#define RAR_OM_EXTRACT        1

#define RAR_SKIP              0
#define RAR_TEST              1
#define RAR_EXTRACT           2

#define RAR_VOL_ASK           0
#define RAR_VOL_NOTIFY        1

struct RARHeaderData
{
  char         ArcName[260];
  char         FileName[260];
  unsigned int Flags;
  unsigned int PackSize;
  unsigned int UnpSize;
  unsigned int HostOS;
  unsigned int FileCRC;
  unsigned int FileTime;
  unsigned int UnpVer;
  unsigned int Method;
  unsigned int FileAttr;
  char         *CmtBuf;
  unsigned int CmtBufSize;
  unsigned int CmtSize;
  unsigned int CmtState;
};
/** End **/
#endif

///This is used in the callback for extracting RARs to memory
typedef struct {
	char **buffer;
	unsigned long size;
} rar_callback_info;

#ifdef __cplusplus
extern "C" { 
#endif

/**
 * Open a RAR file
 *
 * \param filename The file to open
 * \param om The open mode
 * \return The RAR file, or 0 on error
 */
void* rar_open(char *filename, unsigned int om);

/**
 * Close a RAR file
 *
 * \param rar_file The file to close
 * \return 0 on success, something else on error. (See dll.hpp defines.)
 */
int rar_close(void *rar_file);

/**
 * Get the header for the current file in a RAR file
 *
 * \param rar_file The RAR file
 * \param HeaderData A pointer to the RARHeaderData struct in which to store the
 * header data
 * \return 0 on success, something else on error. (See dll.hpp defines.)
 */
int rar_get_header(void *rar_file, struct RARHeaderData *HeaderData);

/**
 * Move to the next file in a RAR file
 *
 * \param rar_file The RAR file
 * \return 0 on success, something else on error. (See dll.hpp defines.)
 */
int rar_next_file(void *rar_file);

/**
 * Get the full contents of a file from a RAR file given the filename
 * \param arc_file The name of the archive to search
 * \param filename The file to read
 * \param data A pointer to a pointer that will get set to the allocated block
 * of memory for the archive
 * \param size A pointer to a size_t that gets set to the file's size
 * \return 0 on success, something else on error. (See dll.hpp defines.)
 */
int rar_get_file_by_name(char *arc_file, char *filename, char **data, size_t *size);

#ifdef PSPComic_rar_cpp
int CALLBACK rar_file_callback(UINT msg,LONG UserData,LONG P1,LONG P2);
#endif

#ifdef __cplusplus
} 
#endif

#include "pspcomic.h"

#endif
