#define LOG_TAG "Kil"

#include "CommonInclude.h"
#include <stdio.h>
#include "DebugHelpers.h"
#include "mscgen.h"

#ifndef MSCAPI
  #define MSCAPI __attribute__ ((visibility("default")))
#endif	// MSCAPI

MSCAPI void vmscc(  const char * top_level, const char * op, const char * bottom_level,
		   unsigned color, const char * msg, va_list argptr                 )
{
#ifdef HHPOS
	if ((GET_SETTING(DebugScanDriver) & 128) == 0)
		return;
#endif

	char buffer[257];
	char * p = buffer;
	char * e = buffer + sizeof(buffer);
	int nchars = 0;

#define P(func,args)							\
	do											\
	{											\
		if (nchars != -1 && p < e)				\
		{										\
			nchars = func args;					\
			if (nchars != -1)					\
				p += nchars;					\
		}										\
	} while(0)


	P(snprintf, (p, e-p, "[MSCGEN] %s%s%s [ label=\"",
				  top_level, op, bottom_level));

	P(vsnprintf, (p, e-p, msg, argptr));

	P(snprintf, (p, e-p, " @%s", now_str()));
	P(snprintf, (p, e-p, "\""));
	if (color != ~0U)
		P(snprintf, (p, e-p, ", textcolor=\"#%06X\"", color));
	P(snprintf, (p, e-p, "] ;"));

#ifdef USING_PRINTF
	DbgOut(PRINT_LEVEL, LOG_TAG, buffer);	// adds a \n
#else
	DbgOut1(PRINT_LEVEL, LOG_TAG, "%s", buffer);	// no \n for logcat
#endif
#undef P
}

MSCAPI void mscc(const char * top_level, const char * op, const char * bottom_level,
		 unsigned color, const char * msg, ...            )
{
	va_list argptr;
	va_start(argptr, msg);
	vmscc(top_level, op, bottom_level, color, msg, argptr);
	va_end(argptr);
}

MSCAPI void vmsc(const char * top_level, const char * op, const char * bottom_level,
		 const char * msg, va_list argptr )
{
	vmscc(top_level, op, bottom_level, ~0U, msg, argptr);
}

MSCAPI void msc(const char * top_level, const char * op, const char * bottom_level,
		 const char * msg, ...            )
{
	va_list argptr;
	va_start(argptr, msg);
	vmscc(top_level, op, bottom_level, ~0U, msg, argptr);
	va_end(argptr);
}

MSCAPI void msc_warning(const char * msg, ...)
{
	va_list argptr;
	va_start(argptr, msg);
	vmscc("", "---", "", 0xC0C000, msg,  argptr);
	va_end(argptr);
}

MSCAPI void msc_error(const char * msg, ...)
{
	va_list argptr;
	va_start(argptr, msg);
	vmscc("", "---", "", 0xFF0000, msg,  argptr);
	va_end(argptr);
}

MSCAPI void msc_note(const char * msg, ...)
{
	va_list argptr;
	va_start(argptr, msg);
	vmsc("", "---", "", msg,  argptr);
	va_end(argptr);
}


