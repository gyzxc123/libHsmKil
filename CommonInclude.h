//! \file
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

#include "debug.h"

#define CAMERA_FOR_SCANNER 1


#ifndef IMGDBGAPI
//  #define IMGDBGAPI extern "C" __attribute__ ((visibility("default")))
#define IMGDBGAPI __attribute__ ((visibility("default")))
#endif // IMGDBGAPI

#ifndef DBGAPI
//#define DBGAPI extern "C" __attribute__ ((visibility("default")))
#define DBGAPI __attribute__ ((visibility("default")))
#endif // DBGAPI

#define ARRAY_SIZE(a)	(int)(sizeof(a) / sizeof((a)[0]))
#define ARRAY_SIZE_U(a)	(sizeof(a) / sizeof((a)[0]))

//! A helper to clear the contensts of structs.
#define CLEAR(x) memset(&(x), 0, sizeof (x))

/* if not using kilhelpers */
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int

#define INVALID_HANDLE -1
