// commoninclude.h

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>

#define ASSERT assert

#define APIENTRY

//! Interface export.
#ifndef IMGDBGAPI
//  #define IMGDBGAPI extern "C" __attribute__ ((visibility("default")))
  #define IMGDBGAPI __attribute__ ((visibility("default")))
#endif	// IMGDBGAPI

#ifndef DBGAPI
  //#define DBGAPI extern "C" __attribute__ ((visibility("default")))
  #define DBGAPI __attribute__ ((visibility("default")))
#endif	// DBGAPI

#define USE_IMAGEDEBUG 1 // always include code for exporting

// for testing of syncobjs
typedef 	volatile unsigned int 			HANDLE;

//-----------------------------------------------------------------------------
//  These are the possible return values for all API functions
//  They indicate either success, or the type of error encountered.
//-----------------------------------------------------------------------------
typedef enum
{
   RESULT_INITIALIZE = -1,
    RESULT_SUCCESS  = 0,                // Operation was successful
    RESULT_ERR_BADFILENAME,
    RESULT_ERR_BADPORT,
    RESULT_ERR_BADREGION,
    RESULT_ERR_DRIVER,              //  Error occurred communicating with engine driver
    RESULT_ERR_ENGINEBUSY,
    RESULT_ERR_FILE,
    RESULT_ERR_FILEINCOMPATIBLE,
    RESULT_ERR_FILEINVALID,
    RESULT_ERR_MEMORY,
    RESULT_ERR_NODECODE,
    RESULT_ERR_NODRIVER,
    RESULT_ERR_NOIMAGE,
    RESULT_ERR_NORESPONSE,
    RESULT_ERR_NOTCONNECTED,
    RESULT_ERR_PARAMETER,           // One of the function parameters was invalid
    RESULT_ERR_UNSUPPORTED,         // The operation was not supported by the engine
    RESULT_ERR_UPGRADE,
    RESULT_ERR_MENUDECODE,
    RESULT_ERR_REFLASH,
    RESULT_ERR_NOTRIGGER,           // During wait for decode the function we call to check a trigger return that it was released
    RESULT_ERR_BADSMARTIMAGE,
    RESULT_ERR_SMARTIMAGETOOLARGE,
    RESULT_ERR_BUFFER_TOO_SMALL,
	RESULT_ERR_TOO_MUCH_INTERPOLATION
}   Result_t;


Result_t ConvertToBitmap(unsigned char* pImg, unsigned short nRows, unsigned short nCols, unsigned long* pdwSize, unsigned short nBits);

