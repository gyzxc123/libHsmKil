//! \file


#include "CommonInclude.h"
#include "DebugHelpers.h"
#include "ImageDebugInterface.h"

#define CRT_FINI 1		// until I know a better way
#ifdef CRT_FINI
///////////////////////////////////////////////////////////////////////////////
//! Emergency cleanup.
/*!
 * The class CFini is responsible to do an emergency cleanup if the user of the lib
 * forgets to call any deinit.
 * The special attributes tell the linker to place the ctor/dtor in a table for static objects.
 * So the dtor is called before the lib gets unloaded.
 * Theoretically we could expect the OS does all the resource cleanup, but we want to play nice and save.
 * Background: In initfini.c, the C-Lib defines the _init() and _fini() functions that take care of
 * calling the static objects.
 * */

class CFini_ImageDebug
{
public:
	CFini_ImageDebug() __attribute__((constructor))
	{
//		DBG_FUNC();
		DmesgOpen();
	}
	~CFini_ImageDebug() __attribute__((destructor))
	{
//		DBG_FUNC();
		DestroyImageDebug();
		DmesgClose();
	}
};

static CFini_ImageDebug Cleaner;

#else
HWLAPI void _init()
{
	DmesgOpen();
}
// This function gets called just before the lib is unloaded
// http://linux.die.net/man/3/dlclose
HWLAPI void _fini()
{
	DestroyImageDebug();
	DmesgClose();
}
#endif

