// commoninclude.h

#ifndef COMMONINCLUDE_H
#define COMMONINCLUDE_H

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define ARRAY_SIZE(a)	(int)(sizeof(a) / sizeof((a)[0]))
#define ARRAY_SIZE_U(a)	(sizeof(a) / sizeof((a)[0]))

#define APIENTRY

#ifndef HWLAPI
  #define HWLAPI extern "C"
#endif

#define KILAPI HWLAPI

// for testing of syncobjs
//typedef 	volatile unsigned int 			HANDLE;

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

#define USE_IMAGEDEBUG 1 // so MSVC shows the right colors
#include "ImageDebugInterface.h"

#endif
